#include "mm.h"
#include "../ke/ki.h"
#include "../drv/drv.h"

#define ALIGN_DOWN(__addr, __align) ((__addr) & ~((__align)-1))


DEFINE_LOCK( vmm_lock );
DEFINE_LOCK( pagemap_lock );


pagemap_t MiKernelPage;

static ULONG64 *vmm_get_next_level( ULONG64 *table, size_t index,
									ULONG64 flags )
{
	ULONG64 *ret = 0;
	ULONG64 *entry = ( void * )( ( ULONG64 )table + PHYS_MEM_OFFSET ) + index * 8;

	if ( ( entry[ 0 ] & 1 ) != 0 )
		ret = ( ULONG64 * )( entry[ 0 ] & ( ULONG64 )~0xfff );
	else
	{
		ret = pmalloc( 1 );
		entry[ 0 ] = ( ULONG64 )ret | flags;
	}

	return ret;
}

void vmm_invalidate_tlb( pagemap_t *pagemap, uintptr_t virtual_address )
{
	ULONG64 cr3;
	asm volatile( "mov %%cr3, %0" : "=r"( cr3 ) : : "memory" );
	if ( cr3 == ( ULONG64 )pagemap->top_level )
		asm volatile( "invlpg (%0)" : : "r"( virtual_address ) );
}

void vmm_map_page( pagemap_t *pagemap, uintptr_t physical_address,
				  uintptr_t virtual_address, ULONG64 flags )
{
	KeTryAcquireMutex( pagemap_lock );

	size_t pml_entry4 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 39 ) ) >> 39;
	size_t pml_entry3 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 30 ) ) >> 30;
	size_t pml_entry2 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 21 ) ) >> 21;
	size_t pml_entry1 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 12 ) ) >> 12;

	ULONG64 *pml3 = vmm_get_next_level( pagemap->top_level, pml_entry4, flags );
	ULONG64 *pml2 = vmm_get_next_level( pml3, pml_entry3, flags );
	ULONG64 *pml1 = vmm_get_next_level( pml2, pml_entry2, flags );

	*( ULONG64 * )( ( ULONG64 )pml1 + PHYS_MEM_OFFSET + pml_entry1 * 8 ) =
		physical_address | flags;

	vmm_invalidate_tlb( pagemap, virtual_address );

	KeReleaseMutex( pagemap_lock );
}

void vmm_unmap_page( pagemap_t *pagemap, uintptr_t virtual_address )
{
	KeTryAcquireMutex( pagemap_lock );

	size_t pml_entry4 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 39 ) ) >> 39;
	size_t pml_entry3 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 30 ) ) >> 30;
	size_t pml_entry2 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 21 ) ) >> 21;
	size_t pml_entry1 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 12 ) ) >> 12;

	ULONG64 *pml3 = vmm_get_next_level( pagemap->top_level, pml_entry4, 0b111 );
	ULONG64 *pml2 = vmm_get_next_level( pml3, pml_entry3, 0b111 );
	ULONG64 *pml1 = vmm_get_next_level( pml2, pml_entry2, 0b111 );

	*( ULONG64 * )( ( ULONG64 )pml1 + PHYS_MEM_OFFSET + pml_entry1 * 8 ) = 0;

	vmm_invalidate_tlb( pagemap, virtual_address );

	KeReleaseMutex( pagemap_lock );
}

uintptr_t vmm_virt_to_phys( pagemap_t *pagemap, uintptr_t virtual_address )
{
	size_t pml_entry4 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 39 ) ) >> 39;
	size_t pml_entry3 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 30 ) ) >> 30;
	size_t pml_entry2 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 21 ) ) >> 21;
	size_t pml_entry1 = ( size_t )( virtual_address & ( ( size_t )0x1ff << 12 ) ) >> 12;

	ULONG64 *pml3 = vmm_get_next_level( pagemap->top_level, pml_entry4, 0b111 );
	ULONG64 *pml2 = vmm_get_next_level( pml3, pml_entry3, 0b111 );
	ULONG64 *pml1 = vmm_get_next_level( pml2, pml_entry2, 0b111 );

	if ( !( pml1[ pml_entry1 ] & 1 ) )
		return 0;

	return ( pml1[ pml_entry1 ] ) & ~( ( uintptr_t )0xfff );
}

uintptr_t vmm_get_kernel_address( pagemap_t *pagemap,
								 uintptr_t virtual_address )
{
	uintptr_t aligned_virtual_address = ALIGN_DOWN( virtual_address, PAGE_SIZE );
	uintptr_t phys_addr = vmm_virt_to_phys( pagemap, virtual_address );
	return ( phys_addr + PHYS_MEM_OFFSET + virtual_address -
			aligned_virtual_address );
}

void vmm_memcpy( pagemap_t *pagemap_1, uintptr_t virtual_address_1,
				pagemap_t *pagemap_2, uintptr_t virtual_address_2,
				size_t count )
{
	uintptr_t aligned_virtual_address_1 =
		ALIGN_DOWN( virtual_address_1, PAGE_SIZE );
	uintptr_t aligned_virtual_address_2 =
		ALIGN_DOWN( virtual_address_2, PAGE_SIZE );

	uint8_t *phys_addr_1 =
		( uint8_t * )vmm_virt_to_phys( pagemap_1, aligned_virtual_address_1 );
	uint8_t *phys_addr_2 =
		( uint8_t * )vmm_virt_to_phys( pagemap_2, aligned_virtual_address_2 );

	size_t align_difference_1 = virtual_address_1 - aligned_virtual_address_1;

	size_t align_difference_2 = virtual_address_2 - aligned_virtual_address_2;
	for ( size_t i = 0; i < count; i++ )
	{
		*( phys_addr_1 + PHYS_MEM_OFFSET + align_difference_1 ) =
			*( phys_addr_2 + PHYS_MEM_OFFSET + align_difference_2 );

		if ( !( ( ++align_difference_1 + 1 ) % PAGE_SIZE ) )
		{
			align_difference_1 = 0;

			virtual_address_1 += PAGE_SIZE;

			aligned_virtual_address_1 = ALIGN_DOWN( virtual_address_1, PAGE_SIZE );
			phys_addr_1 =
				( uint8_t * )vmm_virt_to_phys( pagemap_1, aligned_virtual_address_1 );
		}

		if ( !( ( ++align_difference_2 + 1 ) % PAGE_SIZE ) )
		{
			align_difference_2 = 0;

			virtual_address_2 += PAGE_SIZE;

			aligned_virtual_address_2 = ALIGN_DOWN( virtual_address_2, PAGE_SIZE );
			phys_addr_2 =
				( uint8_t * )vmm_virt_to_phys( pagemap_2, aligned_virtual_address_2 );
		}
	}
}
void MmLoadVirtualPage2( ULONG64 pml4 )
{
	/* in long mode, the CR3 register is used to point to the PML4 base address */
	asm( "mov %0, %%cr3" ::"a"( pml4 ) );
}
void MmLoadVirtualPage( pagemap_t *pagemap )
{
	asm volatile( "mov %0, %%cr3" : : "a"( pagemap->top_level ) );
}

pagemap_t *vmm_create_new_pagemap( )
{
	pagemap_t *new_map = kcalloc( sizeof( pagemap_t ) );
	new_map->top_level = pcalloc( 1 );

	ULONG64 *kernel_top =
		( ULONG64 * )( ( void * )MiKernelPage.top_level + PHYS_MEM_OFFSET );
	ULONG64 *user_top =
		( ULONG64 * )( ( void * )new_map->top_level + PHYS_MEM_OFFSET );

	for ( uintptr_t i = 256; i < 512; i++ )
		user_top[ i ] = kernel_top[ i ];

	for ( uintptr_t i = 0;
		 i < ( uintptr_t )( KiVBEData.Width * KiVBEData.Height * sizeof( uint32_t ) ); i += PAGE_SIZE )
		vmm_map_page(
			new_map, vmm_virt_to_phys( &MiKernelPage, ( uintptr_t )KiVBEData.Address ) + i,
			vmm_virt_to_phys( &MiKernelPage, ( uintptr_t )KiVBEData.Address ) + i, 0b111 );

	return new_map;
}

int MmInitializeVmm( )
{
	MiKernelPage.top_level = ( ULONG64 * )pcalloc( 1 );

	for ( ULONG64 i = 256; i < 512; i++ )
		vmm_get_next_level( MiKernelPage.top_level, i, 0b111 );

	for ( uintptr_t i = PAGE_SIZE; i < 0x100000000; i += PAGE_SIZE )
	{
		vmm_map_page( &MiKernelPage, i, i, 0b11 );
		vmm_map_page( &MiKernelPage, i, i + PHYS_MEM_OFFSET, 0b11 );
	}

	for ( uintptr_t i = 0; i < 0x80000000; i += PAGE_SIZE )
		vmm_map_page( &MiKernelPage, i, i + KERNEL_MEM_OFFSET, 0b111 );

	MmLoadVirtualPage( &MiKernelPage );

	return 0;
}