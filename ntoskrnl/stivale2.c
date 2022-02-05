#include "stivale2.h"


static uint8_t stack[ 4096 ];

static struct stivale2_header_tag_framebuffer fb_tag = {
	.tag = {
		.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
		.next = 0},          /* there's no tag after this one, so next = 0  */
	.framebuffer_width = 0,  /* let the bootloader decide the fb resolution */
	.framebuffer_height = 0, /*                                             */
	.framebuffer_bpp = 0 };   /* bits per pixel, see color depth             */

__attribute__( ( section( ".stivale2hdr" ), used ) )

/* the stivale_hdr or stivale_header, refer to the official documentation
 * https://github.com/stivale/stivale/blob/master/STIVALE2.md#stivale2-header */
	static struct stivale2_header stivale_hdr = {
		.entry_point = ( uint64_t )KiSystemStartup,           /* tells the bootloader that the entry point is _start         */
		.stack = ( uintptr_t )stack + sizeof( stack ), /* above, static uint8_t stack[4096];                          */
		.flags = ( 1 << 1 ),                         /* bit 1 set, all pointers are to be offset to the higher half */
		.tags = ( uintptr_t )&fb_tag };               /* points to the first tag of the linked list of header tags   */

PVOID
KiGetStivale2Tag(
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