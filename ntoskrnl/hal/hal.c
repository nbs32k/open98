#include "hal.h"

VOID
HalInitModule(

)
{
	
	void( *term_write )( const char *string, UINT length ) = term_write_ptr;
	term_write( "Initialized SSE\n", 17 );

	HalInitGDT( ); //Initialize GDT
	HalInitIDT( );

	HalInitPIT( );

	KeInitializeKernelClock( );
}