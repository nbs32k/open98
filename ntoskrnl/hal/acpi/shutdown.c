#include "../io/io.h"
#include "shutdown.h"
#include "../../drv/drv.h"
#include "../../lib/utill.h"

#include "../../ke/ki.h"


#include "acpi.h"
INT 
PopTransitionSystemPowerStateEx(
	PUINT a1
)
{
	HalACPIShutdown( );

	return 0;
}

INT
NtSetSystemPowerState(
	INT a1,
	INT a2,
	INT a3
)
{
	return PopTransitionSystemPowerStateEx( NULL );
}

INT
NtShutdownSystem(
	SHUTDOWN_ACTION Action
)
{
	DbgPrintFmt( "(NtShutdownSystem) NtShutdownSystem called." );

	if ( Action == 0 )
		if ( !NtSetSystemPowerState( NULL, 4, -1073741820 ) ) goto halt;
	else if ( Action == 1 )
		return 0;//NtSetSystemPowerState( NULL, 4, -1073741820 );
	else if ( Action == 2 )
		return 0;//NtSetSystemPowerState( NULL, 4, -1073741820 );
	
halt:
	DbgPrintFmt( "(NtShutdownSystem) NtSetSystemPowerState failed, jumping to manual screen." );
	KiDrawFilled( 0, KiVBEData.Width, 0, KiVBEData.Height, 0x00000000 );

	PCHAR* Text;// = malloc( 42 );
	Text = "It is now safe to shutdown your computer!";

	INT iCharWidth = 8;
	INT iCharHeight = 16;

	INT iMiddleX = ( KiVBEData.Width / 2 ) - //cut the screen in half, minus
		( RtlStringLength( Text ) * ( iCharWidth / 2 ) );//letter lenght, multiplied with
														//letter font width devided by 2 (in half)

	INT iMiddleY = ( KiVBEData.Height / 2 ) -
		( RtlStringLength( Text ) / ( iCharHeight / 2 ) );

	KiDisplayString( Text, iMiddleX, iMiddleY, 0xFFFFFFFF );

	KiDisableInt( );

	for ( ;;)asm( "hlt" );
		
}