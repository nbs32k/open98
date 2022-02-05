#include "mm.h"
#include "../ke/ki.h"


// Bit functions. Provided by AtieP
#define ALIGN_UP(__number) (((__number) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define ALIGN_DOWN(__number) ((__number) & ~(PAGE_SIZE - 1))

#define BIT_SET(__bit) (pmm_bitmap[(__bit) / 8] |= (1 << ((__bit) % 8)))
#define BIT_CLEAR(__bit) (pmm_bitmap[(__bit) / 8] &= ~(1 << ((__bit) % 8)))
#define BIT_TEST(__bit) ((pmm_bitmap[(__bit) / 8] >> ((__bit) % 8)) & 1)

DEFINE_LOCK( pmm_lock );

static uint8_t *pmm_bitmap = 0;
static uintptr_t highest_page = 0;

void pmm_free_page( void *adr )
{
	KeTryAcquireMutex( pmm_lock );
	BIT_CLEAR( ( size_t )adr / PAGE_SIZE );
	KeReleaseMutex( pmm_lock );
}

void pmm_alloc_page( void *adr )
{
	DEFINE_LOCK( pmm_lock );
	BIT_SET( ( size_t )adr / PAGE_SIZE );
	KeReleaseMutex( pmm_lock );
}

void pmm_free_pages( void *adr, size_t page_count )
{
	for ( size_t i = 0; i < page_count; i++ )
		pmm_free_page( ( void * )( adr + ( i * PAGE_SIZE ) ) );
}

void pmm_alloc_pages( void *adr, size_t page_count )
{
	for ( size_t i = 0; i < page_count; i++ )
		pmm_alloc_page( ( void * )( adr + ( i * PAGE_SIZE ) ) );
}

void *pmalloc( size_t pages )
{
	KeTryAcquireMutex( pmm_lock );

	for ( size_t i = 0; i < highest_page / PAGE_SIZE; i++ )
		for ( size_t j = 0; j < pages; j++ )
		{
			if ( BIT_TEST( i ) )
				break;
			else if ( j == pages - 1 )
			{
				KeReleaseMutex( pmm_lock );
				pmm_alloc_pages( ( void * )( i * PAGE_SIZE ), pages );
				return ( void * )( i * PAGE_SIZE );
			}
		}

	KeReleaseMutex( pmm_lock );
	DbgPrintFmt( 1, "Ran out of memory! Halting!" );
	while ( 1 )
		;
	return NULL;
}

void *pcalloc( size_t pages )
{
	char *ret = ( char * )pmalloc( pages );

	if ( ret == NULL )
		return NULL;

	memset( ( void * )( ( uint64_t )ret + PHYS_MEM_OFFSET ), 0, pages * PAGE_SIZE );

	return ret;
}

int MmInitializePmm( struct stivale2_struct_tag_memmap *memory_info )
{
	uintptr_t top;

	for ( size_t i = 0; i < memory_info->entries; i++ )
	{
		struct stivale2_mmap_entry *entry = &memory_info->memmap[ i ];

		if ( entry->type != STIVALE2_MMAP_USABLE &&
			entry->type != STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE &&
			entry->type != STIVALE2_MMAP_KERNEL_AND_MODULES )
			continue;

		top = entry->base + entry->length;

		if ( top > highest_page )
			highest_page = top;
	}

	size_t bitmap_size = ALIGN_UP( ALIGN_DOWN( highest_page ) / PAGE_SIZE / 8 );

	for ( size_t i = 0; i < memory_info->entries; i++ )
	{
		struct stivale2_mmap_entry *entry = &memory_info->memmap[ i ];

		if ( entry->type == STIVALE2_MMAP_USABLE && entry->length >= bitmap_size )
		{
			pmm_bitmap = ( uint8_t * )( entry->base + PHYS_MEM_OFFSET );
			entry->base += bitmap_size;
			entry->length -= bitmap_size;
			break;
		}
	}

	memset( pmm_bitmap, 0xff, bitmap_size );

	for ( size_t i = 0; i < memory_info->entries; i++ )
		if ( memory_info->memmap[ i ].type == STIVALE2_MMAP_USABLE )
			pmm_free_pages( ( void * )memory_info->memmap[ i ].base,
						   memory_info->memmap[ i ].length / PAGE_SIZE );

	return 0;
}