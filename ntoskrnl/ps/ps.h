#pragma once
#include <ntdef.h>
#include "../hal/idt/idt.h"
#include "../mm/mm.h"

typedef struct TASK_STRUCT
{
	int pid;
	int status;

	struct InterruptRegisters regs;
	pagemap_t* page_map;
	UCHAR flags;
} KSYSTEM_TASK;


//scheduler.c
//////

VOID
PsThreadsInit(

);

VOID PsLoadTask(
	KSYSTEM_TASK *task
);

VOID
PsScheduleThreads(
	struct InterruptRegisters *regs
);



//task.c
////

#define TASK_STOPPED 1
#define TASK_RUNNING 2
#define TASK_NO_INIT 3

#define FLAG_USER 0b01
#define FLAG_KERN 0b10

#define STACK_SIZE 0x10000

/* a task is a process, for now, we are not going to differentiate
 * between task and thread
 *
 * each task ought to have its own pid (process id), its own registers
 * (each time we change tasks, we need to load their new registers) and
 * its own pagemap in order to have its own virtual space
 *
 * so as to get several tasks running (ostensibly), what we are going to do is give
 * them a CPU time, that is, one task will run for some milliseconds, and
 * then we will change to the next one */


VOID
PspCreateThread(
	LPTHREAD_START_ROUTINE lpThread
);