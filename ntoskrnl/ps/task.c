#include "ps.h"

//INT iPidCount = 0;
//
//VOID
//PspCreateThread(
//	ULONG64 ip,
//	UCHAR flags
//)
//{
//	KSYSTEM_TASK *task = ( KSYSTEM_TASK * )kmalloc( sizeof( KSYSTEM_TASK ) );
//	task->flags = flags;
//
//	task->pid = iPidCount++;
//	task->regs.rip = ip;                     /* the ip is the instruction poINTer, and will be loaded INTo the rip register */
//	task->regs.rflags = ( 1 << 9 ) | ( 1 << 1 ); /* rflags reg contains the current CPU state, INTerrupt enable flag | reserved */
//
//	if ( task->flags & FLAG_USER ) /* this task is for a user program */
//	{
//		task->regs.cs = 0x23; /* gdt_entry user_code  */
//		task->regs.ss = 0x1B; /* gdt_entry user_data  */
//
//		task->page_map = ( ULONG64 * )kmalloc( PAGE_SIZE );
//
//		/* the pagemap has 4096/8 = 512 entries (each one 64 bits), so, if the task is for
//		 * a user program, we will copy the higher half kernel entries INTo the task pagemap */
//		for ( size_t i = 256; i < 512; i++ )
//			task->page_map[ i ] = MiKernelPage[ i ];
//
//		DbgPrintFmt( "User tasks are not implemented\n" );
//	}
//	else /* this task is for the kernel */
//	{
//		task->regs.cs = 0x28; /* [8]  -> gdt_entry kernel_code */
//		task->regs.ss = 0x30; /* [16] -> gdt_entry kernel_data */
//
//		task->page_map = MiKernelPage;
//
//		/* the rsp register poINTs to the top of the stack (since the stack grows downwards, we add its size) */
//		task->regs.rsp = ( ULONG64 )kmalloc( STACK_SIZE ) + HIGHER_HALF + STACK_SIZE;
//	}
//
//	PsLoadTask( task ); /* automatically enqueue the new task */
//}