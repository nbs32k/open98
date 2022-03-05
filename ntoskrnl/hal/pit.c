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

#include "../mod/module.h"

INT BaseFrequency = 1193180;
INT div = 50;
CHAR timepre = 0;
STATIC VOID
KiUpdateTick(
	struct InterruptRegisters *reg
)
{
	ulTick++;
	PsScheduleThreads( reg );
	
	if ( !( ulTick % ( div / 30 ) ) )
	{
		DwmRefreshBuffer( );
		
	}
}

VOID
HalInitPIT(

)
{
	HalRegisterInterrupt(
		IRQ0,
		KiUpdateTick
	);
	

	ULONG64 ulDivisor = ( ULONG64 )( BaseFrequency / div );

	UCHAR ucLow = ( UCHAR )( ulDivisor & 0xFF );
	UCHAR ucHigh = ( UCHAR )( ( ulDivisor >> 8 ) & 0xFF );

	outb( 0x43, 0x36 );

	outb( 0x40, ucLow );
	outb( 0x40, ucHigh );

	KiPrintToScreen( "[ OK ] Initialized PIT" );

}