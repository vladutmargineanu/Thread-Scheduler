#ifndef SCHEDULER_STRUCT_H_
#define SCHEDULER_STRUCT_H_

#include "so_priorityqueue.h"
#include "so_linkedlist.h"
#include "so_scheduler.h"
#include "utils.h"

typedef enum {
	NEW,
	READY,
	RUNNING,
	WAITING,
	TERMINATED
} THREAD_STATE;

typedef struct so_thread *SO_THREAD;
typedef struct so_scheduler *SO_SCHEDULER;

struct so_thread {
	int priority;			/* prioritatea thread-ului */
	int time_task;			/* timpul de rulare al thread-ului */
	tid_t id_thread;	/* id-ul thread-ului */
	HANDLE sem_wait;		/* blocarea thread-ului curent */
	HANDLE sem_create;		/* blocarea procesului parinte */
	THREAD_STATE state;		/* starea thread-ului curent */
	HANDLE handler_thread;	/* thread-ul care ruleaza functia */
	so_handler *handler;	/* functia pe care ruleaza thread-ul */
};

struct so_scheduler {
	/* timpul unui thread pentru executie */
	int time_quantum;
	/* numarul maxim de dispozitive */
	unsigned int maximum_io;
	/* exista un thread care ruleaza */
	int active_thread;
	/* coada cu thread-urile din ready */
	PriorityQueue ready_threads;
	/* vector de liste in starea waiting */
	LinkedList *wait_threads;
	/* lista cu thread-urile din finish */
	LinkedList finish_threads;
	/* thread-ul activ */
	SO_THREAD running_thread;
	/* indica daca thread-urile au terminat */
	HANDLE sem_finish;
};
#endif
