build: so_linkedlist.obj so_priorityqueue.obj so_scheduler.obj
	link /nologo /dll /out:libscheduler.dll /implib:libscheduler.lib so_scheduler.obj so_linkedlist.obj so_priorityqueue.obj

so_linkedlist.obj : utils.h so_linkedlist.h so_linkedlist.c
	cl /nologo /W3 /EHsc /Fo$@ /c so_linkedlist.c

so_priorityqueue.obj : utils.h so_priorityqueue.h so_priorityqueue.c
	cl /nologo /W3 /EHsc /Fo$@ /c so_priorityqueue.c

so_scheduler.obj : utils.h so_scheduler.h scheduler_struct.h so_scheduler.c
	cl /nologo /W3 /EHsc /Fo$@ /c so_scheduler.c

.PHONY : clean

clean:
	del *.obj *.dll *.lib *.exe *.exp