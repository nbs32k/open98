#include "idt.h"
#include <stdint.h>
int_handler_t interrupt_handlers[ 256 ];

VOID HalSetIDTDescriptor( UCHAR vector, PVOID isr, UCHAR type_attr )
{
	struct idt_desc_t *descriptor = ( struct idt_desc_t * )&idt[ vector ];

	descriptor->offset_1 = ( ULONG64 )isr & 0xFFFF;
	descriptor->selector = 0x28;
	descriptor->ist = 0;
	descriptor->type_attr = type_attr;
	descriptor->offset_2 = ( ( ULONG64 )isr >> 16 ) & 0xFFFF;
	descriptor->offset_3 = ( ( ULONG64 )isr >> 32 ) & 0xFFFFFFFF;
	descriptor->zero = 0;
}

STATIC VOID not_implemented( struct interrupt_registers * );
VOID HalInstallIRS();

VOID HalInitIDT()
{
	idtr.base = ( uintptr_t )&idt[ 0 ];
	idtr.limit = ( USHORT )sizeof( struct idt_desc_t ) * 256 - 1;

	for ( INT i = 32; i < 48; i++ )
	{
		HalRegisterInterrupt( i, not_implemented );
	}

	HalInstallIRS();

	__asm__ volatile( "lidt %0"
					  :
	: "memory"( idtr ) );
	__asm__ volatile( "sti" );

	void( *term_write )( const char *string, UINT length ) = term_write_ptr;
	term_write( "Initialized IDT\n", 17 );
}

VOID HalRegisterInterrupt( UCHAR n, int_handler_t handler )
{
	interrupt_handlers[ n ] = handler;
}
#include "../../ke/ki.h"
VOID isr_handler( struct interrupt_registers *regs )
{
	//RtlRaiseException( regs );
	PCHAR shet[ 512 ];
	sprintf( shet, "Raise Exception: %s", exception_messages[regs->int_no] );
	DbgPrintFmt( shet ); //autism 
	asm( "cli" );
	asm( "hlt" );
}

VOID irq_handler( struct interrupt_registers *regs )
{
	if ( interrupt_handlers[ regs->int_no ] != 0 )
	{
		int_handler_t handler = interrupt_handlers[ regs->int_no ];
		handler( regs );
	}

	if ( regs->int_no >= IRQ8 )
	{
		outb( PIC2_COMMAND, PIC_EOI );
	}
	outb( PIC1_COMMAND, PIC_EOI );
}

VOID KeRaiseIrql(
  KIRQL  NewIrql,
  PKIRQL OldIrql
)
{
	KIRQL pog;
	pog.Address = interrupt_handlers[ NewIrql.IRQ ];
	pog.IRQ = NewIrql.IRQ;

	*OldIrql = pog;

	interrupt_handlers[ NewIrql.IRQ ] = NewIrql.Address;
}

VOID KeLowerIrql(
  KIRQL NewIrql
)
{
	interrupt_handlers[ NewIrql.IRQ ] = NewIrql.Address;
	if( NewIrql.Address == IRQ_ADDR_NONE ) interrupt_handlers[ NewIrql.IRQ ] = not_implemented;
}

STATIC VOID not_implemented( struct interrupt_registers *testing )
{
}

VOID HalInstallIRS()
{
	HalSetIDTDescriptor( 0, ( PVOID )isr0, 0x8E );
	HalSetIDTDescriptor( 1, ( PVOID )isr1, 0x8E );
	HalSetIDTDescriptor( 2, ( PVOID )isr2, 0x8E );
	HalSetIDTDescriptor( 3, ( PVOID )isr3, 0x8E );
	HalSetIDTDescriptor( 4, ( PVOID )isr4, 0x8E );
	HalSetIDTDescriptor( 5, ( PVOID )isr5, 0x8E );
	HalSetIDTDescriptor( 6, ( PVOID )isr6, 0x8E );
	HalSetIDTDescriptor( 7, ( PVOID )isr7, 0x8E );
	HalSetIDTDescriptor( 8, ( PVOID )isr8, 0x8E );
	HalSetIDTDescriptor( 9, ( PVOID )isr9, 0x8E );
	HalSetIDTDescriptor( 10, ( PVOID )isr10, 0x8E );
	HalSetIDTDescriptor( 11, ( PVOID )isr11, 0x8E );
	HalSetIDTDescriptor( 12, ( PVOID )isr12, 0x8E );
	HalSetIDTDescriptor( 13, ( PVOID )isr13, 0x8E );
	HalSetIDTDescriptor( 14, ( PVOID )isr14, 0x8E );
	HalSetIDTDescriptor( 15, ( PVOID )isr15, 0x8E );
	HalSetIDTDescriptor( 16, ( PVOID )isr16, 0x8E );
	HalSetIDTDescriptor( 17, ( PVOID )isr17, 0x8E );
	HalSetIDTDescriptor( 18, ( PVOID )isr18, 0x8E );
	HalSetIDTDescriptor( 19, ( PVOID )isr19, 0x8E );
	HalSetIDTDescriptor( 20, ( PVOID )isr20, 0x8E );
	HalSetIDTDescriptor( 21, ( PVOID )isr21, 0x8E );
	HalSetIDTDescriptor( 22, ( PVOID )isr22, 0x8E );
	HalSetIDTDescriptor( 23, ( PVOID )isr23, 0x8E );
	HalSetIDTDescriptor( 24, ( PVOID )isr24, 0x8E );
	HalSetIDTDescriptor( 25, ( PVOID )isr25, 0x8E );
	HalSetIDTDescriptor( 26, ( PVOID )isr26, 0x8E );
	HalSetIDTDescriptor( 27, ( PVOID )isr27, 0x8E );
	HalSetIDTDescriptor( 28, ( PVOID )isr28, 0x8E );
	HalSetIDTDescriptor( 29, ( PVOID )isr29, 0x8E );
	HalSetIDTDescriptor( 30, ( PVOID )isr30, 0x8E );
	HalSetIDTDescriptor( 31, ( PVOID )isr31, 0x8E );

	outb( 0x20, 0x11 );
	outb( 0xA0, 0x11 );
	outb( 0x21, 0x20 );
	outb( 0xA1, 0x28 );
	outb( 0x21, 0x04 );
	outb( 0xA1, 0x02 );
	outb( 0x21, 0x01 );
	outb( 0xA1, 0x01 );
	outb( 0x21, 0x0 );
	outb( 0xA1, 0x0 );


	

	HalSetIDTDescriptor( IRQ0, ( PVOID )irq0, 0x8E );
	HalSetIDTDescriptor( IRQ1, ( PVOID )irq1, 0x8E );
	HalSetIDTDescriptor( IRQ2, ( PVOID )irq2, 0x8E );
	HalSetIDTDescriptor( IRQ3, ( PVOID )irq3, 0x8E );
	HalSetIDTDescriptor( IRQ4, ( PVOID )irq4, 0x8E );
	HalSetIDTDescriptor( IRQ5, ( PVOID )irq5, 0x8E );
	HalSetIDTDescriptor( IRQ6, ( PVOID )irq6, 0x8E );
	HalSetIDTDescriptor( IRQ7, ( PVOID )irq7, 0x8E );
	HalSetIDTDescriptor( IRQ8, ( PVOID )irq8, 0x8E );
	HalSetIDTDescriptor( IRQ9, ( PVOID )irq9, 0x8E );
	HalSetIDTDescriptor( IRQ10, ( PVOID )irq10, 0x8E );
	HalSetIDTDescriptor( IRQ11, ( PVOID )irq11, 0x8E );
	HalSetIDTDescriptor( IRQ12, ( PVOID )irq12, 0x8E );
	HalSetIDTDescriptor( IRQ13, ( PVOID )irq13, 0x8E );
	HalSetIDTDescriptor( IRQ14, ( PVOID )irq14, 0x8E );
	HalSetIDTDescriptor( IRQ15, ( PVOID )irq15, 0x8E );
}