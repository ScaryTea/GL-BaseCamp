#include "lee.h"

#ifndef QUEUE_H 
#define QUEUE_H 

struct Node {
    struct Cell *cell;
    struct Node *prev;
};

struct Queue {
    struct Node *head;
    struct Node *tail;
    int size;
};

extern struct Queue *constructQueue();
extern void destructQueue(struct Queue *pQueue);
extern bool enqueue(struct Queue *pQueue, struct Cell *item);
extern struct Node *dequeue(struct Queue *pQueue);
extern bool isEmpty(struct Queue* pQueue);

#endif
