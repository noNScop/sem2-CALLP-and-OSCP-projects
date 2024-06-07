#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include "list.h"

// PTHREAD_MUTEX_INITIALIZER works equivalently to creating a pthread_mutex_t variable and then calling pthread_mutex_init with NULL as the second argument
static pthread_mutex_t id_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
static int ID_COUNTER = 0;

TList* createList(int s) {
    TList *list = malloc(sizeof(TList));
    if (list == NULL) {
        printf("Failed to allocate memory for list\n");
        return NULL;
    }

    list->first = NULL;
    list->last = NULL;
    list->capacity = s;
    list->num_of_elements = 0;

    pthread_mutex_lock(&id_counter_mutex);
    list->id = ID_COUNTER++;
    pthread_mutex_unlock(&id_counter_mutex);

    if (pthread_mutex_init(&list->mutex, NULL) != 0) {
        printf("Failed to initialize mutex\n");
        free(list);
        return NULL;
    }

    if (pthread_cond_init(&list->cfull, NULL) != 0) {
        printf("Failed to initialize conditional variable cfull\n");
        pthread_mutex_destroy(&list->mutex);
        free(list);
        return NULL;
    }

    if (pthread_cond_init(&list->cempty, NULL) != 0) {
        printf("Failed to initialize conditional variable cempty\n");
        pthread_cond_destroy(&list->cfull);
        pthread_mutex_destroy(&list->mutex);
        free(list);
        return NULL;
    }

    return list;
}

void putItem(TList *lst, void *itm) {
    pthread_mutex_lock(&lst->mutex);

    while (lst->capacity <= lst->num_of_elements) {
        pthread_cond_wait(&lst->cfull, &lst->mutex);
    }
    
    Node *node = malloc(sizeof(Node));
    if (node == NULL) {
        printf("Failed to allocate memory for node\n");
        pthread_mutex_unlock(&lst->mutex);
        return;
    }

    node->prev = lst->last;
    node->next = NULL;
    node->mem_ptr = itm;

    if (lst->last != NULL) {
        lst->last->next = node;
    } else {
        lst->first = node;
    }

    lst->last = node;
    lst->num_of_elements++;

    pthread_cond_signal(&lst->cempty);
    pthread_mutex_unlock(&lst->mutex);
}

void showList(TList *lst) {
    pthread_mutex_lock(&lst->mutex);

    Node *node = lst->first;
    while (node != NULL) {
        printf("%p\n", node->mem_ptr);
        node = node->next;
    }

    pthread_mutex_unlock(&lst->mutex);
}

void setMaxSize(TList *lst, int s) {
    pthread_mutex_lock(&lst->mutex);

    int prev = lst->capacity;
    lst->capacity = s;

    if (prev < s) {
        pthread_cond_broadcast(&lst->cfull);
    }
    
    pthread_mutex_unlock(&lst->mutex);
}

int getCount(TList *lst) {
    pthread_mutex_lock(&lst->mutex);

    int out = lst->num_of_elements;

    pthread_mutex_unlock(&lst->mutex);

    return out;
}

void destroyList(TList *lst) {
    pthread_mutex_lock(&lst->mutex);
    
    Node *node = lst->last;
    Node *prev;
    while (node != NULL) {
        prev = node->prev;
        free(node->mem_ptr);
        free(node);
        node = prev;
    }

    pthread_mutex_unlock(&lst->mutex);

    pthread_mutex_destroy(&lst->mutex);
    pthread_cond_destroy(&lst->cempty);
    pthread_cond_destroy(&lst->cfull);
    free(lst);
}

void *getItem(TList *lst) {
    pthread_mutex_lock(&lst->mutex);

    while (lst->num_of_elements == 0) {
        pthread_cond_wait(&lst->cempty, &lst->mutex);;
    }

    Node *node = lst->first;
    if (node->next != NULL) {
        node->next->prev = NULL;
        lst->first = node->next;
    } else {
        lst->first = NULL;
        lst->last = NULL;
    }

    lst->num_of_elements--;

    pthread_cond_signal(&lst->cfull);
    pthread_mutex_unlock(&lst->mutex);

    void *item = node->mem_ptr;
    free(node);
    return item;
}

void *popItem(TList *lst) {
    pthread_mutex_lock(&lst->mutex);

    while (lst->num_of_elements == 0) {
        pthread_cond_wait(&lst->cempty, &lst->mutex);;
    }

    Node *node = lst->last;
    if (node->prev != NULL) {
        node->prev->next = NULL;
        lst->last = node->prev;
    } else {
        lst->first = NULL;
        lst->last = NULL;
    }

    lst->num_of_elements--;

    pthread_cond_signal(&lst->cfull);
    pthread_mutex_unlock(&lst->mutex);

    void *item = node->mem_ptr;
    free(node);
    return item;
}

int removeItem(TList *lst, void *itm) {
    pthread_mutex_lock(&lst->mutex);

    Node *node = lst->first;
    while (node != NULL && node->mem_ptr != itm) {
        node = node->next;
    }

    if (node != NULL) {
        if (node->prev != NULL && node->next != NULL) {
            // convert "a <-> b <-> c" into "a <-> c"
            node->prev->next = node->next;
            node->next->prev = node->prev;
        } else if (node->prev != NULL) {
            node->prev->next = NULL;
            lst->last = node->prev;
        } else if (node->next != NULL) {
            node->next->prev = NULL;
            lst->first = node->next;
        } else {
            lst->first = NULL;
            lst->last = NULL;
        }
        
        lst->num_of_elements--;

        pthread_cond_signal(&lst->cfull);
        pthread_mutex_unlock(&lst->mutex);

        free(node->mem_ptr);
        free(node);
        return 1;
    }

    pthread_mutex_unlock(&lst->mutex);
    return 0;
}

void appendItems(TList *lst, TList *lst2) {
    // lists have ids to avoid deadlock when 2 threads simultanously want to append list1 to list2 and list2 to list1, thanks to id's I can always lock them in the same order, hence avoiding the deadlock
    TList *firstList = (lst->id < lst2->id) ? lst : lst2;
    TList *secondList = (lst->id < lst2->id) ? lst2 : lst;

    pthread_mutex_lock(&firstList->mutex);
    pthread_mutex_lock(&secondList->mutex);

    if (lst->last != NULL && lst2->first != NULL) {
        // both lists contain something
        lst->num_of_elements += lst2->num_of_elements;
        lst->last->next = lst2->first;
        lst2->first->prev = lst->last;
        lst->last = lst2->last;
        lst2->first = NULL;
        lst2->last = NULL;
    } else if (lst2->first != NULL) {
        // only lst2 contains something
        lst->num_of_elements = lst2->num_of_elements;
        lst->first = lst2->first;
        lst->last = lst2->last;
        lst2->first = NULL;
        lst2->last = NULL;
    }

    lst2->num_of_elements = 0;

    pthread_cond_broadcast(&lst->cempty);
    pthread_cond_broadcast(&lst2->cfull);
    pthread_mutex_unlock(&secondList->mutex);
    pthread_mutex_unlock(&firstList->mutex);
}
