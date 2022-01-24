#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include "windef.h"

static UCHAR stack[8192];

static struct stivale2_header_tag_terminal terminal_hdr_tag = {
	.tag = {

		.identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,

		.next = 0
	},

	.flags = 0
};


static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {

	.tag = {
		.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,

		.next = ( ULONG64 )&terminal_hdr_tag
	},

    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};

__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {

    .entry_point = 0,

    .stack = (uintptr_t)stack + sizeof(stack),

    .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),

    .tags = (uintptr_t)&framebuffer_hdr_tag
};


PVOID KiGetStivale2Tag(
  struct stivale2_struct *stivale2_struct,
  ULONG64 id
)
{
	
	struct stivale2_tag *CurrentTag = ( PVOID )stivale2_struct->tags;
	for ( ;;)
	{

		if ( CurrentTag == NULL )
		{
			return NULL;
		}


		if ( CurrentTag->identifier == id )
		{
			return CurrentTag;
		}


		CurrentTag = ( PVOID )CurrentTag->next;
	}
}

#include "hal/hal.h"
#include "drv/drv.h"

// Kernel Load
///////////////

void *term_write_ptr = 0;


#include "lib/utill.h"
VOID Plasma( )
{
	ULONG64 ulOffset;

	
	for ( ;; )
	{
		for ( int time = 0; time < 50; time++ )
			for ( int i = 0, y = -720; y < 720; y++ )
				for ( int x = -1280; x < 1280; x++, i++ )
				{

					*( UCHAR* )( KiVBEData.Address + i ) = 0xFFFF8888 + ( ( time + x * x + y * y ) * 5 / 6000 ) ;
				}
	}

	for ( int time = 0; time < 10000; time++ )
	{
		for ( int y = 0; y < KiVBEData.Height; y++ )
		{
			for ( int x = 0; x < KiVBEData.Width; x++ )
			{
				//the plasma buffer is a sum of sines
				ULONG64 color = ( 0xFF + time + ( 128.0 + ( 128.0 * sin( x / 16.0 ) )
								 + 128.0 + ( 128.0 * sin( y / 8.0 ) )
								 + 128.0 + ( 128.0 * sin( ( x - y ) / 16.0 ) )
								 + 128.0 + ( 128.0 * sin( sqrt( ( x * x + y * y ) ) / 8.0 ) )
								 ) / 0xA3 );

				
				

				ulOffset = ( ULONG64 )( ( x * 4 ) + ( y * KiVBEData.Pitch ) );
				*(UCHAR*)(KiVBEData.Address + ulOffset ) = 0x20 + ( UCHAR )( color ) % 0xF0;
			}
		}
	}
}

#include "ke/ki.h"
#include "mm/mm.h"
#include "ntuser/ntuser.h"

#include "hal/acpi/acpi.h"
#include "hal/acpi/shutdown.h"
VOID
KiSystemStartup(
	struct stivale2_struct *stivale2_struct
)
{
    
	
	



	struct stivale2_struct_tag_terminal *term_str_tag;
	term_str_tag = KiGetStivale2Tag( stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID );
	term_write_ptr = ( void * )term_str_tag->term_write;
	void( *term_write )( const char *string, size_t length ) = term_write_ptr;

	struct stivale2_struct_tag_memmap *mmap = KiGetStivale2Tag( stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID );


	MmInit( mmap );

	HalInitModule( );


	HalInitVBE( stivale2_struct );

	KiInitSinTable( );
	
	KiDrawFilled( 0, KiVBEData.Width, 0, KiVBEData.Height, 0x008080 );

	//init drivers
	KiInitKeyboard( );
	KiInitMouse( );
	//Plasma( );
	struct stivale2_struct_tag_rsdp *rsdp_hdr_tag;
	rsdp_hdr_tag = KiGetStivale2Tag( stivale2_struct, STIVALE2_STRUCT_TAG_RSDP_ID );
	HalInitACPI( ( void * )rsdp_hdr_tag->rsdp );
	

	/*KiDisplayString(
		"debug text 123@#$%^&*(!~\nnew line test\r\nnew line reset test",
		10,
		10,
		0xFFFFFFFF
	);

	KiDrawFilled( 90, 300, 200, 400, 0xFFFFFFFF );*/
	PCHAR TextString[ 512 ];
	KSYSTEM_TIME SystemTime;
	KeQuerySystemTime( &SystemTime );

	sprintf( TextString, "Boot at: %d:%d:%d", SystemTime.Hours, SystemTime.Minutes, SystemTime.Seconds );

	KiDisplayString(
		TextString,
		10,
		10,
		0xffffffff
	);

	NtSetCursorPos( 0, 0 );

	

	

	

	//for ( int i = 0; i < 300000; ++i )
		//io_wait(  );
	
	//NtShutdownSystem( 0 );

    for (;;) {
		
		

		
		//asm( "hlt" ); <- dont be idiot as me to try to find out why the interrupts werent firing
    }
}
