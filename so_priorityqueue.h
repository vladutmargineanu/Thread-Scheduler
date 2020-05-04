#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef struct priorityqueue *PriorityQueue;
typedef struct qnode *QNode;

struct qnode {
	int key;		/* prioritatea din coada */
	void *data;		/* informatia unui thread */
	QNode next;
};

struct priorityqueue {
	int size;
	QNode head;
};

/* Functia creeaza o coada si aloca memoria necesara */
PriorityQueue initPQueue(void);

/* Se verifica daca coada este goala */
int emptyPQueue(PriorityQueue queue);

/* Functia intoarce numarul de elemente din coada */
int sizePQueue(PriorityQueue queue);

/* Functia adauga un element nou in coada */
int pushPQueue(PriorityQueue queue, int key, void *data);

/* Functia intoarce valoarea din primul nod din coada */
void *topPQueue(PriorityQueue queue);

/* Functia sterge primul nod din coada si intoarce valoarea din nod */
void *popPQueue(PriorityQueue queue);

/* Functia sterge elementele din coada si dezaloca memoria */
void clearPQueue(PriorityQueue queue);

#endif
