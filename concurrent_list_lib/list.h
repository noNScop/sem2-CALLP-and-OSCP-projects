#include <pthread.h>

typedef struct Node {
    struct Node *prev;
    struct Node *next;
    void *mem_ptr;
} Node;

struct TList {
    int id;
    int capacity;
    int num_of_elements;
    Node *first;
    Node *last;
    pthread_mutex_t mutex;
    pthread_cond_t cfull;
    pthread_cond_t cempty;
};
typedef struct TList TList;

TList* createList(int s);
void destroyList(TList *lst);
void putItem(TList *lst, void *itm);
void* getItem(TList *lst);
void* popItem(TList *lst);
int removeItem(TList *lst, void *itm);
int getCount(TList *lst);
void setMaxSize(TList *lst, int s);
void appendItems(TList *lst, TList *lst2);
void showList(TList *lst);