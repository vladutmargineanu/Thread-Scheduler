#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler_struct.h"

/* strcuctura pentru planificarea thread-urilor */
static SO_SCHEDULER scheduler;

/* functia care ruleaza algoritmul de planificare */
static void algorithm_scheduler(SO_SCHEDULER scheduler,
						THREAD_STATE futureState,
						int io)
{
	DWORD rc = -1;

	/*
	 * Cazul in care un thread se blocheaza si asteapta dispozitivul io
	 */
	if (futureState == WAITING) {

		SO_THREAD wait_thread = NULL;
		/* Thread-ul care executa instructiunea curenta */
		SO_THREAD currentThread = scheduler->running_thread;
		/* Thread-ul care urmeaza a fi planificat */
		SO_THREAD futureThread = topPQueue(scheduler->ready_threads);

		/* Verificam daca exista un thread in sistem */
		if (currentThread == NULL || futureThread == NULL)
			return;
		/* se reinitializeaza timpul de rulare al thread-ului curent */
		currentThread->time_task = 0;

		/* Verificam daca exista thread-uri pentru dispozitivul io */
		if (scheduler->wait_threads[io] == NULL)
			scheduler->wait_threads[io] = createList();
		/* Thread-ul curent trece in starea de wait */
		currentThread->state = WAITING;
		/* Se adauga thread-ul curent in lista de waiting pentru io */
		rc = addElement(scheduler->wait_threads[io], currentThread);

		/* Se alege un thread pentru rulare */
		wait_thread = (SO_THREAD) popPQueue(scheduler->ready_threads);
		if (wait_thread == NULL)
			return;
		/* Se initializeaza scheduler cu thread-ul curent */
		scheduler->running_thread = futureThread;
		/* Se trece in starea running pentru thread-ul curent */
		scheduler->running_thread->state = RUNNING;

		/* Se elibereaza thread-ul care va rula */
		rc = ReleaseSemaphore(futureThread->sem_wait, 1, NULL);
		DIE(rc == FALSE, "error sem_post");

		/* Thread-ulcurent este pus in asteptare */
		rc = WaitForSingleObject(currentThread->sem_wait, INFINITE);
		DIE(rc == WAIT_FAILED, "error sem_wait");

	} else if (futureState == READY) {
		/*
		 * Cazul in care un thread este in starea ready, se verifica
		 * daca exista un thread cu prioritate mai mare pentru inlocuire
		 */

		/* Thread-ul care executa instructiunea curenta */
		SO_THREAD currentThread = scheduler->running_thread;
		/* Thread-ul care urmeaza a fi planificat */
		SO_THREAD futureThread = topPQueue(scheduler->ready_threads);

		DIE(currentThread == NULL && futureThread == NULL,
										 "error call ready");
		/* Daca este la apelul primului fork */
		if (currentThread == NULL) {
			/* Se alege un thread pentru rulare */
			popPQueue(scheduler->ready_threads);
			scheduler->running_thread = futureThread;
			/* Se elibereaza thread-ul care va rula */
			rc = ReleaseSemaphore(futureThread->sem_wait, 1, NULL);
			DIE(rc == FALSE, "error sem_post");

			return;
		}
		/* Timpul thread-ului curent creste cu un proces */
		currentThread->time_task++;
		/* Daca nu exista un alt thread in sistem */
		if (futureThread == NULL) {
			if (currentThread->time_task == scheduler->time_quantum)
				currentThread->time_task = 0;
			return;
		}
		/*
		 * Thread-ul curent are prioritatea mai mare,
		 * isi continua executia
		 */
		if (currentThread->priority > futureThread->priority) {
			if (currentThread->time_task == scheduler->time_quantum)
				currentThread->time_task = 0;
			return;
		}
		/*
		 * Thread-ul curent are aceeasi
		 * prioritate cu thread-ul planificat
		 */
		if (currentThread->priority == futureThread->priority)
			if (currentThread->time_task < scheduler->time_quantum)
				return;
		/* Thread-ul curent isi reinitializeaza timpul de executie */
		currentThread->time_task = 0;
		/* Se adauga threadul curent in lista ready */
		rc = pushPQueue(scheduler->ready_threads,
						currentThread->priority,
						currentThread);
		DIE(rc != 0, "error add element");
		/* Se scoate thread-ul cu prioritate din coada */
		popPQueue(scheduler->ready_threads);
		/* Se alege thread-ul care ruleaza */
		scheduler->running_thread = futureThread;

		/* Se elibereaza thread-ul care va rula */
		rc = ReleaseSemaphore(futureThread->sem_wait, 1, NULL);
		DIE(rc == FALSE, "error sem_post");

		/* Se pune in asteptare thread-ul curent */
		rc = WaitForSingleObject(currentThread->sem_wait, INFINITE);
		DIE(rc == WAIT_FAILED, "error sem_wait");

	} else if (futureState == TERMINATED) {
		/*
		 * Cazul in care thread-ul curent este adaugat in lista
		 * thread-urilor care si-au terminat executia si se
		 * verifica daca exista thread-uri in sistem pentru a
		 * astepta terminarea acestora.
		 */

		/* Thread-ul care executa instructiunea curenta */
		SO_THREAD currentThread = scheduler->running_thread;
		/* Thread-ul care urmeaza a fi planificat */
		SO_THREAD futureThread = topPQueue(scheduler->ready_threads);

		/* Thread-ul curent se adauga in lista de terminare */
		rc = addElement(scheduler->finish_threads, currentThread);
		DIE(rc != 0, "error add element");

		/* Se verifica daca exista alte thread-uri pentru terminare */
		if (futureThread == NULL) {
			rc = ReleaseSemaphore(scheduler->sem_finish, 1, NULL);
			DIE(rc == FALSE, "error sem_post");

			return;
		}
		/* Se alege un thread pentru rulare */
		popPQueue(scheduler->ready_threads);
		/* Se planifica urmatorul thread in scheduler */
		scheduler->running_thread = futureThread;
		/* Se elibereaza thread-ul care va rula */
		rc = ReleaseSemaphore(futureThread->sem_wait, 1, NULL);
		DIE(rc == FALSE, "error sem_post");
	}
}
/*
 * Functia pe care o executa thread-urile la planificare
 */
static void *thread_start(void *thread_info)
{
	DWORD rc;
	SO_THREAD thread = (SO_THREAD) thread_info;

	thread->id_thread = GetCurrentThreadId();

	/* Ne indica ca thread-ul respectiv este gata */
	rc = ReleaseSemaphore(thread->sem_create, 1, 0);
	DIE(rc == FALSE, "error sem_post");

	/* Se pune in asteptare pentru a fi planificat */
	rc = WaitForSingleObject(thread->sem_wait, INFINITE);
	DIE(rc == WAIT_FAILED, "sem_wait");

	/* Se ruleaza functia care consuma timp */
	thread->handler(thread->priority);

	/*
	 * Se apeleaza algoritmul pentru un thread
	 * care si-a terminat executia
	 */
	algorithm_scheduler(scheduler, TERMINATED, -1);

	return NULL;
}

int so_init(unsigned int time_quantum, unsigned int io)
{
	unsigned int i;

	/* Se verifica parametrii daca sunt valizi */
	if (scheduler != NULL || io > SO_MAX_NUM_EVENTS || time_quantum <= 0)
		return -1;

	/* Se aloca memorie pentru structura scheduler */
	scheduler = malloc(sizeof(struct so_scheduler));
	DIE(scheduler == NULL, "ENOMEM");

	/*
	 * Se aloca memorie pentru vectorul liste
	 * cu thread-urile in asteptare
	 */
	scheduler->wait_threads = malloc(io  * sizeof(struct linkedList));
	DIE(scheduler->wait_threads == NULL, "ENOMEM");

	/* Se initializeaza fiecare lista la NULL */
	for (i = 0; i < io; i++)
		scheduler->wait_threads[i] = NULL;

	/* Se initializeaza semaforul care indica terminarea programului */
	scheduler->sem_finish = CreateSemaphore(NULL, 0, 1, NULL);
	DIE(scheduler->sem_finish == NULL, "error sem_init");

	scheduler->maximum_io = io;
	scheduler->time_quantum = time_quantum;
	scheduler->running_thread = NULL;
	scheduler->active_thread = 0;
	/* Se creaza coada de prioritati pentru thread-urile din starea ready */
	scheduler->ready_threads = initPQueue();
	/* Se creaza lista cu thread-urile din starea finish */
	scheduler->finish_threads = createList();

	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	tid_t id_current;
	SO_THREAD thread = NULL;
	HANDLE createThread;
	DWORD id_createThread;
	int rc = -1;

	/* Se verifica daca parametrii sunt valizi */
	if (func == NULL || priority > SO_MAX_PRIO)
		return INVALID_TID;

	/* Se aloca memorie pentru noul thread */
	thread = malloc(sizeof(struct so_thread));
	DIE(thread == NULL, "ENOMEM");

	thread->priority = priority;
	thread->handler = func;
	thread->time_task = 0;
	thread->state = NEW;
	scheduler->active_thread = 1;

	/* Se initializeaza semaforul pentru thread-ul creat */
	thread->sem_create = CreateSemaphore(NULL, 0, 1, NULL);
	DIE(thread->sem_create == NULL, "error sem_init");

	thread->sem_wait = CreateSemaphore(NULL, 0, 1, NULL);
	DIE(thread->sem_wait == NULL, "error sem_init");

	/* Se creaza thread-ul cu rutina data ca parametru */
	createThread = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE) thread_start,
				(LPVOID) thread,
				0,
				&id_createThread);

	if (createThread == NULL)
		return INVALID_TID;

	/* Se adauga thread-ul curent in coada ready pentru planificare */
	rc = pushPQueue(scheduler->ready_threads, thread->priority, thread);
	DIE(rc != 0, "error add element");

	/* Se asteapta ca thread-ul curent sa fie in starea ready */
	rc = WaitForSingleObject(thread->sem_create, INFINITE);
	DIE(rc == WAIT_FAILED, "error sem_wait");

	id_current = thread->id_thread;
	thread->handler_thread = CreateThread;

	/* Se apeleaza algoritm-ul pentru cazul READY pentru planificare */
	algorithm_scheduler(scheduler, READY, -1);

	return id_current;
}


int so_wait(unsigned int io)
{
	/* Se verifica daca parametrii sunt valizi */
	if (io >= scheduler->maximum_io)
		return -1;
	/* Se apeleaza algoritmul de planificare pentru cazul WAIT */
	algorithm_scheduler(scheduler, WAITING, io);

	return 0;
}

int so_signal(unsigned int io)
{
	int ret = -1;
	int number_threads = 0;
	int maximum_prio = -1;
	LinkedList wait_threads = NULL;
	SO_THREAD wait_thread = NULL;

	/* Se verifica daca sunt valizi parametrii */
	if (io >= scheduler->maximum_io)
		return -1;
	/* Se verifica daca lista din wait este valida */
	if (scheduler->wait_threads == NULL ||
		scheduler->wait_threads[io] == NULL)
		return 0;

	number_threads = getSize(scheduler->wait_threads[io]);
	if (number_threads < 0)
		return 0;
	/*
	 * Adaugam toate thread-urile care asteapta,
	 * trezite de dispozitivul io
	 */
	wait_threads = scheduler->wait_threads[io];
	while (number_threads > 0) {
		wait_thread = (SO_THREAD) popElement(wait_threads);
		if (maximum_prio < wait_thread->priority)
			maximum_prio = wait_thread->priority;

		ret = pushPQueue(scheduler->ready_threads,
						wait_thread->priority,
						wait_thread);
		if (ret != 0)
			return -1;
		number_threads--;
	}
	/* Daca exista un thread cu prioritate mai mare pentru replanificare */
	if (maximum_prio > scheduler->running_thread->priority)
		algorithm_scheduler(scheduler, READY, -1);
	else if (scheduler->running_thread->time_task ==
			scheduler->time_quantum) {
		scheduler->running_thread->time_task = 0;
		algorithm_scheduler(scheduler, READY, -1);
	} else
		algorithm_scheduler(scheduler, READY, -1);

	return 0;
}

void so_exec(void)
{
	/* Se trimite spre planificare thread-ul curent pentru executie */
	if (scheduler->running_thread->time_task == scheduler->time_quantum) {
		scheduler->running_thread->time_task++;
		algorithm_scheduler(scheduler, READY, -1);
		return;
	}
	algorithm_scheduler(scheduler, READY, -1);
}

void so_end(void)
{
	int rc, i;
	NodeList finish_thread = NULL;
	QNode ready_thread = NULL;
	SO_THREAD thread = NULL;

	if (scheduler == NULL)
		return;

	/* Se asteapta sa termine toate thread-urile create */
	if (scheduler->active_thread == 1) {
		rc = WaitForSingleObject(scheduler->sem_finish, INFINITE);
		DIE(rc == WAIT_FAILED, "error sem_wait");
	}

	finish_thread = scheduler->finish_threads->head;
	/* Facem join pe toate thread-urile care nu si-au terminat executia */
	while (finish_thread != NULL) {
		thread = (SO_THREAD) finish_thread->value;

		rc = WaitForSingleObject(thread->handler_thread, INFINITE);
		DIE(rc == WAIT_FAILED, "error pthread_join");

		finish_thread = finish_thread->next;
	}

	/* Eliberam memoria pentru toate thread-urile din lista finish */
	finish_thread = scheduler->finish_threads->head;
	while (finish_thread != NULL) {
		thread = (SO_THREAD) finish_thread->value;

		rc = CloseHandle(thread->sem_wait);
		DIE(rc == FALSE, "sem_destroy");

		rc = CloseHandle(thread->sem_create);
		DIE(rc == FALSE, "sem_destroy");

		free(thread);

		finish_thread = finish_thread->next;
	}
	destructList(scheduler->finish_threads);

	/* Eliberam memoria pentru toate threadu-urile din lista ready */
	ready_thread = scheduler->ready_threads->head;
	while (ready_thread != NULL) {
		thread = (SO_THREAD) ready_thread->data;

		rc = CloseHandle(thread->sem_wait);
		DIE(rc == FALSE, "sem_destroy");

		rc = CloseHandle(thread->sem_create);
		DIE(rc == FALSE, "sem_destroy");

		free(thread);

		ready_thread = ready_thread->next;
	}
	clearPQueue(scheduler->ready_threads);

	rc = CloseHandle(scheduler->sem_finish);
	DIE(rc == FALSE, "sem_destroy");

	for (i = 0; i < scheduler->maximum_io; i++) {
		if (scheduler->wait_threads[i] != NULL)
			destructList(scheduler->wait_threads[i]);
	}
	free(scheduler->wait_threads);
	free(scheduler);
	scheduler = NULL;
}
