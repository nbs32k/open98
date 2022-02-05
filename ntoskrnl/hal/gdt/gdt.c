#include "gdt.h"
#include "../../lib/printf.h"

__attribute__( ( aligned( 0x1000 ) ) )
struct GDT GDTtable = {
	{0, 0, 0, 0x00, 0x00, 0},
	{0xffff, 0, 0, 0x9a, 0x80, 0},
	{0xffff, 0, 0, 0x92, 0x80, 0},
	{0xffff, 0, 0, 0x9a, 0xcf, 0},
	{0xffff, 0, 0, 0x92, 0xcf, 0},
	{0, 0, 0, 0x9a, 0xa2, 0},
	{0, 0, 0, 0x92, 0xa0, 0},
};

VOID
HalInitGDT()
{
	struct GDTDescriptor descriptor;
	descriptor.Size = sizeof( struct GDT ) - 1;
	descriptor.Offset = ( ULONG64 )&GDTtable;
	HalLoadGDT( &descriptor );

	
}