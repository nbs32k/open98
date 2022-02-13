#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include "ntdef.h"

#include "hal/hal.h"
#include "drv/drv.h"

#include "lib/utill.h"

#include "ke/ki.h"
#include "mm/mm.h"
#include "ntuser/ntuser.h"

#include "hal/acpi/acpi.h"
#include "hal/acpi/shutdown.h"
#include "ps/ps.h"

#include "drv/filesystem/storage.h"

#include "drv/filesystem/fat32/fat32.h"

#include "fsrtl/fsrtl.h"

#include "ps/ps.h"

#include "mod/module.h"


VOID
KiKernelThread(

)
{


	
	for ( ;;)
	{
		/*Critical thread, as it is a kernel thread
		it will crash the whole os if terminated*/
	}
}


VOID
KiSystemStartup(
	struct stivale2_struct *KBOOT_STRUCT
)
{
	//Initialize VBE Struct and Driver
	HalInitVBE( KBOOT_STRUCT );
	KiDrawFilled( 0, KiVBEData.Width, 0, KiVBEData.Height, 0x00000000 );
	KiPrintToScreen( "[ OK ] Initialized VBE Structure and Variables" );

    //Initialize HAL (ALWAYS FIRST!)
	HalInitModule( );
	KiPrintToScreen("[ OK ] Initialized HAL Module");



	//MmPhysInit( KiGetStivale2Tag( KBOOT_STRUCT, STIVALE2_STRUCT_TAG_MEMMAP_ID ) );
	KiPrintToScreen( "[ OK ] Initialized Physical Memory Manager" );


	HalInitACPI( ( PVOID )KiGetStivale2Tag( KBOOT_STRUCT, STIVALE2_STRUCT_TAG_RSDP_ID ) );
	KiPrintToScreen( "[ OK ] Initialized ACPI" );

	MmInitializePmm( KiGetStivale2Tag( KBOOT_STRUCT, STIVALE2_STRUCT_TAG_MEMMAP_ID ) );
	KiPrintToScreen( "[ OK ] Initialized Physical Memory Manager" );

	MmInitializeVmm( );
	KiPrintToScreen( "[ OK ] Initialized Virtual Memory Manager" );

	

	HalPCIInit( );
	KiPrintToScreen( "[ OK ] Initialized PCI" );

	

	//Initialize Processes
	/*PsThreadsInit( );
	KiPrintToScreen( "[ OK ] Initialized Processes and Threads" );*/

	//Initialize drivers
	KiInitKeyboard( );
	KiPrintToScreen( "[ OK ] Initialized Keyboard Driver" );

	KiInitMouse( );
	KiPrintToScreen( "[ OK ] Initialized Mouse Driver" );
	

	KiATAIdentify( );
	KiPrintToScreen( "[ OK ] Initialized ATA Driver" );

	BOOLEAN failBoot = TRUE;
	for ( int i = 0; i < MAX_PARTITION; i++ )
	{
		Partition[ i ] = FatInitialize( i );

		if ( Partition[ i ] == NULL )
			DbgPrintFmt( "Failed to create FAT32 Filesystem, partition error on part %d\n", i  );
		else
		{
			DbgPrintFmt( "Partition %s is now Online.\n", Partition[ i ]->RootPath );
			failBoot = FALSE;
		}
	}
	if ( failBoot )
	{
		KiPrintToScreen( "System halted because it couldn't connect to any partitions!" );
		DbgPrintFmt( "System halted because it couldn't connect to any partitions!" );
		for ( ;;) asm( "hlt" );
	}

	PCHAR pcBuffer;
	ULONG dwBytes;
	PHANDLE lol;
	BOOLEAN bSuccess;

	pcBuffer = ( PCHAR )malloc( 512 );

	lol = NtCreateFile( "C:\\SYSTEM\\POG.TXT", GENERIC_ALL, 0, 0, OPEN_EXISTING, 0, 0 );
	bSuccess = NtReadFile( lol, pcBuffer, 512, &dwBytes, NULL );

	NtClose( lol );

	if(bSuccess) DbgPrintFmt( "File has been read successfully!" );

	free( pcBuffer );

	/*PsThreadsInit( );
	PspCreateThread( ( LPTHREAD_START_ROUTINE )KiKernelThread );*/

	DbgPrintFmt( "%s", pcBuffer );

	/*DwmInitialize( );
	
	NtCreateWindow( NULL, "SEX", NULL, 10, 10, 300, 100, NULL, NULL, NULL, NULL );*/
	DwmInitialize( );
	for ( ;;)
	{
		
		//asm( "hlt" ); <- dont be idiot as me to try to find out why the interrupts werent firing
	}
}
