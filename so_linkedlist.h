#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

typedef struct linkedList *LinkedList;
typedef struct node *NodeList;

struct node {
	NodeList next;
	void *value;	/* informatia pentru thread */
};

struct linkedList {
	NodeList head;
	int size;
};
/* Se creeaza o lista goala si se aloca memorie */
LinkedList createList(void);

/* Functia adauga un nou nod in lista */
int addElement(LinkedList list, void *value);

/* Functia intoarce numarul de elemente din lista */
int getSize(LinkedList list);

/* Functia sterge primul element din lista, intorcand valoarea */
void *popElement(LinkedList list);

/* Functia sterge elementele din lista, lasand lista goala */
void deleteList(LinkedList list);

/* Functia sterge elementele din lista daca exista, si dezaloca memoria */
void destructList(LinkedList list);

#endif
