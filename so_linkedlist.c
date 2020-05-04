#include "so_linkedlist.h"
#include "utils.h"

LinkedList createList(void)
{
	LinkedList list = malloc(sizeof(struct linkedList));

	DIE(list == NULL, "ENOMEM");

	list->size = 0;
	list->head = NULL;
	return list;
}

int addElement(LinkedList list, void *data)
{
	NodeList newNode, node;

	if (list == NULL || data == NULL)
		return 0;

	newNode = calloc(1, sizeof(struct node));

	newNode->next = NULL;
	newNode->value = data;

	node = list->head;

	if (node == NULL) {
		list->head = newNode;
		list->size = 1;
		return 0;
	}

	while (node->next != NULL)
		node = node->next;

	node->next = newNode;
	list->size++;

	return 0;
}

void *popElement(LinkedList list)
{
	void *value;
	NodeList auxNode;

	DIE(list == NULL || list->size == 0, "empty list");

	auxNode = list->head;
	list->size--;
	value = auxNode->value;
	list->head = auxNode->next;
	free(auxNode);

	return value;
}

int getSize(LinkedList list)
{
	if (list == NULL)
		return -1;

	return list->size;
}

void deleteList(LinkedList list)
{
	NodeList auxNode;

	if (list == NULL)
		return;

	while (list->size > 0) {
		auxNode = list->head;
		list->size--;
		list->head = auxNode->next;
		free(auxNode);
	}
}

void destructList(LinkedList list)
{
	if (list->size > 0)
		deleteList(list);

	free(list);
	list = NULL;
}
