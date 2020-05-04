CC=gcc
CFLAGS=-Wall -g -Wno-unused -fPIC
LIBS=-lpthread
EXE=libscheduler.so

.PHONY: all clean

build: $(EXE)

$(EXE): so_scheduler.o so_priorityqueue.o so_linkedlist.o
	$(CC) -shared so_scheduler.o so_priorityqueue.o so_linkedlist.o -o $(EXE)

so_scheduler.o: so_scheduler.h scheduler_struct.h so_scheduler.c utils.h
	$(CC) $(CFLAGS) -c so_scheduler.c $(LIBS)

so_linkedlist.o: so_linkedlist.c so_linkedlist.h utils.h
	$(CC) $(CFLAGS) -c so_linkedlist.c $(LIBS)

so_priorityqueue.o: so_priorityqueue.h so_priorityqueue.c utils.h
	$(CC) $(CFLAGS) -c  so_priorityqueue.c $(LIBS)

clean:
	rm -f *~ *.o *.so