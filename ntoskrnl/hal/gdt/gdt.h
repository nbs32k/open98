#pragma once
#include "../../ntdef.h"
struct GDTDescriptor
{
	USHORT Size;
	ULONG64 Offset;
} __attribute__( ( packed ) );

struct GDTEntry
{
	USHORT Limit0;
	USHORT Base0;
	UCHAR Base1;
	UCHAR AccessByte;
	UCHAR Limit1_Flags;
	UCHAR Base2;
} __attribute__( ( packed ) );

struct GDT
{
	struct GDTEntry Null;
	struct GDTEntry KernelCode;
	struct GDTEntry KernelData;
	struct GDTEntry UserNull;
	struct GDTEntry UserCode;
	struct GDTEntry UserData;
	struct GDTEntry Reserved;
} __attribute__( ( packed ) )
__attribute( ( aligned( 0x1000 ) ) );

extern struct GDT GDTtable;

VOID HalLoadGDT( struct GDTDescriptor *gdtDescriptor );

VOID HalInitGDT();