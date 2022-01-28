#include "ki.h"
#include "../hal/idt/idt.h"
#include "../drv/drv.h"
#include "../lib/printf.h"


DEFINE_LOCK( BlueScreen );

VOID
KeBugCheck(
	struct InterruptRegisters* Reg
)
{
	KeTryAcquireMutex( BlueScreen ); //mutex lock

	KiDrawFilled( 0, KiVBEData.Width, 0, KiVBEData.Height, 0x000080 );

	PCHAR title = "A problem has been detected and noobxp has been shutdown to prevent damage to your computer.\r\n\r\nIf this is the first time you've seen this Stop error screen\r\nrestart your computer. If this screen appears again, follow\r\nthese steps:\r\n\r\nCheck for viruses on your computer. Remove any newly installed\r\nhard drives or hard drive controllers. Check your hard drive\r\nto make sure it is properly configured and terminated.\r\nRun CHKDSK /F to check for hard drive corruption, and then\r\nrestart your computer.\r\n\r\nTechnical information:\r\n\r\n*** STOP: %p (%p, %p, %p, %p)";
	PCHAR StopCode[ 1024 ];
	sprintf(
		StopCode,
		title,
		Reg->int_no,
		Reg->error_code,
		Reg->rip,
		Reg->cs,
		Reg->rflags );
	
	KiDisplayString( StopCode, 1, 1, 0xffffff );



	KeReleaseMutex( BlueScreen );
}