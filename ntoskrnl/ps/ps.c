#include "ps.h"
INT iPidCount = 0;

VOID
PspCreateThread(
	LPTHREAD_START_ROUTINE lpThread
)
{
	KSYSTEM_TASK *Task = ( KSYSTEM_TASK * )malloc( sizeof( KSYSTEM_TASK ) );
	Task->flags = 0b10; //krnl

	Task->pid = iPidCount++;
	Task->regs.rip = lpThread;                     /* the ip is the instruction poINTer, and will be loaded INTo the rip register */
	Task->regs.rflags = ( 1 << 9 ) | ( 1 << 1 ); /* rflags reg contains the current CPU state, INTerrupt enable flag | reserved */

	Task->regs.cs = 0x28; /* [8]  -> gdt_entry kernel_code */
	Task->regs.ss = 0x30; /* [16] -> gdt_entry kernel_data */

	Task->regs.rsp = ( UINT )malloc( STACK_SIZE ) + STACK_SIZE;

	Task->page_map = vmm_create_new_pagemap( );

	PsLoadTask( Task ); /* automatically enqueue the new Task */
}