#include "hal.h"
#include "../drv/drv.h"

VOID
HalInitModule(

)
{
	

	HalInitGDT( ); //Initialize GDT
	

	HalInitIDT( );

	HalInitPIT( );

	KeInitializeKernelClock( );
}