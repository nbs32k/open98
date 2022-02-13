#pragma once
#include "../../ntdef.h"

VOID outb( USHORT port, UCHAR data );

VOID outw( USHORT port, USHORT data );

VOID outl( USHORT port, UINT data );

VOID outsl( USHORT port, ULONG64 buffer, UINT count );

UCHAR inb( USHORT port );

USHORT inw( USHORT port );

UINT inl( USHORT port );

VOID insl( USHORT port, ULONG64 buffer, UINT count );

inline void io_wait( void )
{
	inb( 0x80 );
}