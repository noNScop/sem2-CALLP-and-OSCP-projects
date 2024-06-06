#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "list.h"

// Brief demonstration of list.h library functions

void *thread1(void *arg) {
    TList *lst = (TList *)arg;
    int flag;

    for (int i = 0; i < 10; i++) {
        int *item = malloc(sizeof(int));
        if (item == NULL) {
            printf("Failed to allocate memory for item\n");
            continue;
        }

        *item = i;
        putItem(lst, item);
        if (i == 8) {
            int *num = (int *)popItem(lst);
            printf("\nShould have popped 8, popped: %d\n", *num);

            flag = removeItem(lst, item);
            if (flag) {
                printf("Removed item successfully\n");
            } else {
                printf("Failed to remove an item\n");
            }
            
            free(num);
        }

        if (i % 3 == 0) {
            flag = removeItem(lst, item);
            if (flag) {
                printf("Removed item successfully\n");
            } else {
                printf("Failed to remove an item\n");
            }
        }
    }

    setMaxSize(lst, 50);
    printf("\nCapacity: %d\n", lst->capacity);
    setMaxSize(lst, 1);
    printf("Capacity: %d\n", lst->capacity);

    return NULL;
}

void *thread2(void *arg) {
    TList *lst = (TList *)arg;

    for (int i = 0; i < 10; i++) {
        int *item = malloc(sizeof(int));
        if (item == NULL) {
            printf("Failed to allocate memory for item\n");
            continue;
        }

        *item = i;
        putItem(lst, item);

        if (i % 2 == 0) {
            int *num = (int *)getItem(lst);
            printf("\nShould have popped %d, popped: %d\n", i/2, *num);
            free(num);
        }
    }

    int c = getCount(lst);
    printf("\nNum of items: %d\n", c);
    showList(lst);

    return NULL;
}

int main() {
    pthread_t threads[3];
    TList *lists[2];
    int rc;

    for (int i = 0; i < 2; i++) {
        lists[i] = createList(10);
    }

    rc = pthread_create(&threads[0], NULL, thread1, lists[0]);
    if (rc) {
        printf("Failed to create thread 1, return code: %d\n", rc);
        return 1;
    }

    rc = pthread_create(&threads[1], NULL, thread2, lists[1]);
    if (rc) {
        printf("Failed to create thread 2, return code: %d\n", rc);
        return 1;
    }

    for (int i = 0; i < 2; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            printf("Failed to join thread %d, return code: %d\n", i, rc);
            return 1;
        }
    }

    appendItems(lists[0], lists[1]);

    printf("\nSummary:\n");
    printf("Size of list1: %d\n", getCount(lists[0]));
    printf("Size of list2: %d\n", getCount(lists[1]));
    
    printf("\nList1:\n");
    showList(lists[0]);

    printf("\nList2:\n");
    showList(lists[1]);

    for (int i = 0; i < 2; i++) {
        destroyList(lists[i]);
    }

    return 0;
}