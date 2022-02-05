#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include "windef.h"

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



	FatInitialize( ); 



	for ( ;;)
	{
		//asm( "hlt" ); <- dont be idiot as me to try to find out why the interrupts werent firing
	}
}
