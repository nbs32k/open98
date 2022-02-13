#include "../ntdef.h"
#include "ki.h"
#include "../lib/printf.h"

VOID
DbgPrintString(
	CHAR CONST *val
)
{
	while ( *val )
		outb( 0xE9, *val++ );
}
VOID
DBGHexImplementation(
	ULONG64 num,
	INT nibbles
)
{
	for ( INT i = nibbles - 1; i >= 0; --i )
		outb( 0xE9, "0123456789ABCDEF"[ ( num >> ( i * 4 ) ) & 0xF ] );
}

INT
is_transmit_empty(
	VOID
)
{
	return inb( 0xE9 + 5 ) & 0x20;
}

VOID
serial_printc(
	char c,
	VOID *arg
)
{
	while ( is_transmit_empty( ) == 0 )
		;

	outb( 0xE9, c );
}

VOID
DbgPrintFmt(
	CONST CHAR *fmt, ...
)
{
	va_list args;
	va_start( args, fmt );
	vfctprintf( &serial_printc, NULL, "[KRNL] ", args );
	vfctprintf( &serial_printc, NULL, fmt, args );
	vfctprintf( &serial_printc, NULL, "\n", args );
	va_end( args );
}