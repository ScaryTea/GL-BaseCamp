#include "queue.h"

struct Queue *constructQueue() {
	struct Queue *queue = malloc(sizeof(*queue));
	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;
	return queue;
}

void destructQueue(struct Queue *pQueue)
{
	struct Node *pN;
	while (!isEmpty(pQueue)) {
		pN = dequeue(pQueue);
		free(pN);
	}
	free(pQueue);
}

bool enqueue(struct Queue *pQueue, struct Cell *cell) {
	if (pQueue == NULL) {
        	return false;
	}
	struct Node *item = malloc(sizeof(*item));
	item->cell = cell;
	item->prev = NULL;
	if (pQueue->size == 0) {
		pQueue->head = item;
		pQueue->tail = item;
	} else {
		pQueue->tail->prev = item;
		pQueue->tail = item;
	}
	pQueue->size++;
	return true;
}

struct Node *dequeue(struct Queue *pQueue) {
	struct Node *item;
	if (isEmpty(pQueue))
		return NULL;
	item = pQueue->head;
	pQueue->head = (pQueue->head)->prev;
	pQueue->size--;
	return item;
}

bool isEmpty(struct Queue* pQueue) {
	if (pQueue == NULL)
		return false;
	if (pQueue->size == 0)
		return true;
	return false;
}

