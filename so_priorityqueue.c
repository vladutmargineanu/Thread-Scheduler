#include "so_priorityqueue.h"
#include "utils.h"


static int biggerPriority(QNode task1, QNode task2)
{
	return task1->key - task2->key;
}

PriorityQueue initPQueue(void)
{
	PriorityQueue queue = malloc(sizeof(struct priorityqueue));

	DIE(queue == NULL, "ENOMEM");

	queue->size = 0;
	queue->head = NULL;

	return queue;
}

int emptyPQueue(PriorityQueue queue)
{
	if (queue == NULL)
		return -1;

	return queue->size == 0;
}

int sizePQueue(PriorityQueue queue)
{
	if (queue == NULL)
		return 0;

	return queue->size;
}

int pushPQueue(PriorityQueue queue, int key, void *data)
{
	QNode newNode, auxNode, prevNode;
	int priority;

	if (queue == NULL || data == NULL)
		return	-1;

	if (queue->size == 0) {
		queue->size++;

		queue->head = malloc(sizeof(struct qnode));
		DIE(queue->head == NULL, "ENOMEM");

		queue->head->data = data;
		queue->head->key = key;
		queue->head->next = NULL;

		return 0;
	}

	queue->size++;

	newNode = malloc(sizeof(struct qnode));
	DIE(queue->head == NULL, "ENOMEM");

	newNode->data = data;
	newNode->key = key;

	auxNode = queue->head;
	priority = biggerPriority(auxNode, newNode);

	if (priority == -1) {
		newNode->next = auxNode;
		queue->head = newNode;

		return 0;
	}

	prevNode = auxNode;
	auxNode = auxNode->next;

	while ((auxNode != NULL) && (auxNode->key >= key)) {
		prevNode = auxNode;
		auxNode = auxNode->next;
	}

	newNode->next = auxNode;
	prevNode->next = newNode;

	return 0;
}

void *topPQueue(PriorityQueue queue)
{
	if (queue->head == NULL || queue->size == 0)
		return NULL;

	return queue->head->data;
}

void *popPQueue(PriorityQueue queue)
{
	QNode auxNode;
	void *data;

	DIE(queue == NULL || queue->size == 0, "empty queue");

	auxNode = queue->head;
	queue->size--;
	data = auxNode->data;
	queue->head = auxNode->next;
	free(auxNode);

	return data;
}

void clearPQueue(PriorityQueue queue)
{
	QNode auxNode;

	DIE(queue == NULL, "empty queue");

	while (queue->size > 0) {
		auxNode = queue->head;
		queue->size--;
		queue->head = auxNode->next;
		free(auxNode);
	}
	free(queue);
	queue = NULL;
}
