#include "../io/io.h"
#include "shutdown.h"
#include "../../drv/drv.h"
#include "../../lib/utill.h"
#include "../../mm/mm.h"
#include "../../ke/ki.h"

struct facp
{
	CHAR     signature[ 4 ];
	UINT length;
	UCHAR  unneeded1[ 40 - 8 ];
	UINT dsdt;
	UCHAR  unneeded2[ 48 - 44 ];
	UINT SMI_CMD;
	UCHAR  ACPI_ENABLE;
	UCHAR  ACPI_DISABLE;
	UCHAR  unneeded3[ 64 - 54 ];
	UINT PM1a_CNT_BLK;
	UINT PM1b_CNT_BLK;
	UCHAR  unneeded4[ 89 - 72 ];
	UCHAR  PM1_CNT_LEN;
};

STATIC inline
UCHAR KiParseInt(
	UCHAR* s5_addr,
	ULONG64* value
)
{
	UCHAR op = *s5_addr++;
	if ( op == 0x0 )
	{ // ZeroOp
		*value = 0;
		return 1; // 1 Op Byte
	}
	else if ( op == 0x1 )
	{ // OneOp
		*value = 1;
		return 1; // 1 Op Byte
	}
	else if ( op == 0xFF )
	{ // OnesOp
		*value = ~0;
		return 1; // 1 Op Byte
	}
	else if ( op == 0xA )
	{ // ByteConst
		*value = s5_addr[ 0 ];
		return 2; // 1 Type Byte, 1 Data Byte
	}
	else if ( op == 0xB )
	{ // WordConst
		*value = s5_addr[ 0 ] | ( ( USHORT )s5_addr[ 1 ] << 8 );
		return 3; // 1 Type Byte, 3 Data Bytes
	}
	else if ( op == 0xC )
	{ // DWordConst
		*value = s5_addr[ 0 ] | ( ( UINT )s5_addr[ 1 ] << 8 ) | ( ( UINT )s5_addr[ 2 ] << 16 ) | ( ( UINT )s5_addr[ 3 ] << 24 );
		return 5; // 1 Type Byte, 4 Data Bytes
	}
	else if ( op == 0xE )
	{ // QWordConst
		*value = s5_addr[ 0 ] | ( ( ULONG64 )s5_addr[ 1 ] << 8 ) | ( ( ULONG64 )s5_addr[ 2 ] << 16 ) | ( ( ULONG64 )s5_addr[ 3 ] << 24 ) \
			| ( ( ULONG64 )s5_addr[ 4 ] << 32 ) | ( ( ULONG64 )s5_addr[ 5 ] << 40 ) | ( ( ULONG64 )s5_addr[ 6 ] << 48 ) | ( ( ULONG64 )s5_addr[ 7 ] << 56 );
		return 9; // 1 Type Byte, 8 Data Bytes
	}
	else
	{
		return 0; // No Integer, so something weird
	}
}

#include "acpi.h"
INT 
PopTransitionSystemPowerStateEx(
	PUINT a1
)
{
	struct facp *facp = acpi_find_sdt( "FACP", 0 );

	UCHAR *dsdt_ptr = ( UCHAR * )( uintptr_t )facp->dsdt + 36;
	INT   dsdt_len = *( ( UINT * )( ( uintptr_t )facp->dsdt + 4 ) ) - 36;

	UCHAR *s5_addr = 0;
	for ( INT i = 0; i < dsdt_len; i++ )
	{
		if ( ( dsdt_ptr + i )[ 0 ] == '_'
		 && ( dsdt_ptr + i )[ 1 ] == 'S'
		 && ( dsdt_ptr + i )[ 2 ] == '5'
		 && ( dsdt_ptr + i )[ 3 ] == '_' )
		{
			s5_addr = dsdt_ptr + i;
			goto s5_found;
		}
	}
	return 0;

s5_found:
	s5_addr += 4; // Skip last part of NameSeg, the earlier segments of the NameString were already tackled by the search loop
	if ( *s5_addr++ != 0x12 ) // Assert that it is a PackageOp, if its a Method or something there's not much we can do with such a basic parser
		return 0;
	s5_addr += ( ( *s5_addr & 0xc0 ) >> 6 ) + 1; // Skip PkgLength
	if ( *s5_addr++ < 2 ) // Make sure there are at least 2 elements, which we need, normally there are 4
		return 0;

	ULONG64 value = 0;
	UCHAR size = KiParseInt( s5_addr, &value );
	if ( size == 0 ) // Wasn't able to parse it
		return 0;

	USHORT SLP_TYPa = value << 10;
	s5_addr += size;


	size = KiParseInt( s5_addr, &value );
	if ( size == 0 ) // Wasn't able to parse it
		return 0;

	USHORT SLP_TYPb = value << 10;
	s5_addr += size;

	if ( facp->SMI_CMD != 0 && facp->ACPI_ENABLE != 0 )
	{ // This PC has SMM and we need to enable ACPI mode first
		outb( facp->SMI_CMD, facp->ACPI_ENABLE );
		for ( int i = 0; i < 100; i++ )
			inb( 0x80 );

		while ( !inw( facp->PM1a_CNT_BLK ) & ( 1 << 0 ) )
			;
	}


	outw( facp->PM1a_CNT_BLK, SLP_TYPa | ( 1 << 13 ) );
	if ( facp->PM1b_CNT_BLK )
		outw( facp->PM1b_CNT_BLK, SLP_TYPb | ( 1 << 13 ) );

	for ( int i = 0; i < 100; i++ )
		inb( 0x80 );

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

	PCHAR* Text = malloc( 42 );
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