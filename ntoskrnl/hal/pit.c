#include "hal.h"
#include "../ps/ps.h"
ULONG64 ulTick = 0;

VOID
Sleep(
	LONG Seconds
)
{
	UINT lTick;
	lTick = ulTick + ( Seconds * 100 );

	while ( ulTick < lTick )
		;
}

ULONG64
KiGetTickCount(

)
{
	return ulTick;
}

STATIC VOID
KiUpdateTick(
	struct InterruptRegisters *reg
)
{
	//PsScheduleThreads( reg );
	ulTick++;
}

VOID
HalInitPIT(

)
{
	HalRegisterInterrupt(
		IRQ0,
		KiUpdateTick
	);
	

	ULONG64 ulDivisor = ( ULONG64 )( 1193180 / 50 );

	UCHAR ucLow = ( UCHAR )( ulDivisor & 0xFF );
	UCHAR ucHigh = ( UCHAR )( ( ulDivisor >> 8 ) & 0xFF );

	outb( 0x43, 0x36 );

	outb( 0x40, ucLow );
	outb( 0x40, ucHigh );

	KiPrintToScreen( "[ OK ] Initialized PIT" );

}