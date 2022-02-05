#include "io.h"

VOID outb( USHORT port, UCHAR data )
{
	asm volatile( "outb %0, %1"
				  :
	: "a"( data ), "Nd"( port ) );
}

VOID outw( USHORT port, USHORT data )
{
	asm volatile( "outw %0, %1"
				  :
	: "a"( data ), "Nd"( port ) );
}

VOID outl( USHORT port, UINT data )
{
	asm volatile( "outl %0, %1"
				  :
	: "a"( data ), "Nd"( port ) );
}

VOID outsl( USHORT port, ULONG64 buffer, UINT count )
{
	asm volatile( "cld; rep; outsl"
				  :
	: "D"( buffer ), "d"( port ), "c"( count ) );
}

UCHAR inb( USHORT port )
{
	UCHAR ret;
	asm volatile( "inb %1, %0"
				  : "=a"( ret )
				  : "Nd"( port ) );
	return ret;
}

USHORT inw( USHORT port )
{
	USHORT ret;
	asm volatile( "inw %1, %0"
				  : "=a"( ret )
				  : "Nd"( port ) );
	return ret;
}

UINT inl( USHORT port )
{
	UINT ret;
	asm volatile( "inl %1, %0"
				  : "=a"( ret )
				  : "Nd"( port ) );
	return ret;
}

VOID insl( USHORT port, ULONG64 buffer, UINT count )
{
	asm volatile( "cld; rep; insl"
				  :
	: "D"( buffer ), "d"( port ), "c"( count ) );
}