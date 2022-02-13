#include "ps.h"

#define MAX_TASKS 64

/* in order to keep the count of the different tasks that are running, we will
 * store them in a queue */
KSYSTEM_TASK *tasks;
static INT iTaskCount = 0;

/* this function is defined in __task.asm */
extern VOID
pop_regs(
	struct InterruptRegisters *regs
);

/* as you can read in task.h, we need to change tasks quickly, so it looks like
 * there are several tasks running at the same time
 *
 * in order to achieve that, we will use the PIT (timer.c), each tick, it will
 * call our PsScheduleThreads function, that will take care of changing tasks */
VOID
PsScheduleThreads(
	struct InterruptRegisters *regs
)
{
	if ( iTaskCount == 0 ) /* if the PsScheduleThreadsr has not been initialized */
		return;

	/* each time our PsScheduleThreads function gets called, it will search for the last
	 * running task in the queue, the queue has the followning structure (where
	 * S is stopped, R is running, and X is no init)
	 *
	 *                 | S | S | X | S | R | X | S | S | X | X |
	 *                  (3)             (1)     (2)
	 *
	 * we will iterate over this, find the running task (1) and set it as stopped,
	 * then, we will change to the iNext stopped task (2) (no init ones will be ignored)
	 *
	 * in case we get to the last initialized task in the queue, we will jump again to
	 * the first initialized one (3)
	 */

	INT iNext = 0;
	if ( iTaskCount > 1 )
	{
		for ( INT i = 0; i < MAX_TASKS; i++ )
		{
			if ( tasks[ i ].status == TASK_NO_INIT && i == MAX_TASKS - 1 ) /* finished the iteration without finding anything */
			{
				DbgPrintFmt( "[SCHED] Cannot make new threads, halting!" );
			}/* panic because this should not happen            */
			else if ( tasks[ i ].status == TASK_NO_INIT )
				continue;
			else if ( tasks[ i ].status == TASK_RUNNING )
			{
				tasks[ i ].status = TASK_STOPPED;                        /* set current task as stopped                      */
				RtlCopyMemory( ( VOID * )&tasks[ i ].regs, regs, sizeof( struct InterruptRegisters ) ); /* save current task registers INTo the task struct */

				/* from that position, we are going to find the iNext task that we can
				 * set as running, in order change to that new one */
				for ( INT j = i + 1; j < MAX_TASKS; j++ )
				{
					if ( tasks[ j ].status == TASK_STOPPED )
					{
						iNext = j;
						goto end;
					}
					else if ( j == MAX_TASKS - 1 )
					{
						/* if there are not any stopped tasks after that one, we completed a cycle
						 * and we have to start again */
						for ( INT k = 0; k < i + 1; k++ )
						{
							if ( tasks[ k ].status == TASK_STOPPED )
							{
								iNext = k;
								goto end;
							}
							else if ( k == i )
								/* if that didn't work, it means that the PsScheduleThreadsr did something wrong, or something
								 * external broke our queue */
								DbgPrintFmt( "No available tasks to switch to [k]\n" );
						}
					}
				}
			}
		}
	}
end:
	if ( tasks[ iNext ].status != TASK_RUNNING )
	{
		 //kprINTf("[SCHED] changing to [number = %u, status = %u, rip = 0x%lx, page_map = 0x%lx]\n",
		       // iNext, tasks[iNext].status, tasks[iNext].regs.rip, tasks[iNext].page_map);
		tasks[ iNext ].status = TASK_RUNNING;

		/* remember that the PsScheduleThreads function was called because an IRQ0 was issued, then, we need to
		 * mark the INTerrupt as correctly handled */
		outb( PIC1, PIC_EOI );

		/* now that we marked the task as running, we need to actually make the switch, before that,
		 * we need to load the task's pagemap INTo CR3 */
		//MmLoadVirtualPage( ( ULONG64 )tasks[ iNext ].page_map );
		pop_regs( &tasks[ iNext ].regs ); /* restore task registers and make the switch */
	}
}

VOID PsLoadTask(
	KSYSTEM_TASK *task
)
{
	for ( INT i = 0; i < MAX_TASKS; i++ )
	{
		if ( tasks[ i ].status == TASK_NO_INIT ) /* if we find an empty task */
		{
			RtlCopyMemory( &tasks[ i ], task, sizeof( KSYSTEM_TASK ) ); /* load it INTo the queue */
			tasks[ i ].status = TASK_STOPPED;
			iTaskCount++;
			DbgPrintFmt( "[SCHD] task[%u] loaded INTo the queue\n", task->pid );
			break;
		}
		else if ( i == MAX_TASKS - 1 )
			DbgPrintFmt( "Cannot load a new task\n" );
	}
}

VOID
PsThreadsInit(

)
{
	tasks = ( KSYSTEM_TASK * )malloc( sizeof( KSYSTEM_TASK ) * MAX_TASKS );
	for ( INT i = 0; i < MAX_TASKS; i++ ) /* set all tasks as uninitialized */
		tasks[ i ].status = TASK_NO_INIT;
	DbgPrintFmt( "[SCHD] tasks queue initialized\n" );

	
}

