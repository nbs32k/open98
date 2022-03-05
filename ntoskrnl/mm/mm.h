#pragma once
#include "../ntdef.h"

#include <stdint.h>
#include "../lib/utill.h"
#include <stivale2.h>
#include "liballoc.h"

//vmm.c


#define KERNEL_MEM_OFFSET 0xffffffff80000000

typedef struct pagemap
{
	ULONG64 *top_level;
} pagemap_t;

extern pagemap_t MiKernelPage;

void MmLoadVirtualPage( pagemap_t *pagemap );
pagemap_t *vmm_create_new_pagemap( );
void vmm_map_page( pagemap_t *pagemap, uintptr_t physical_address,
				  uintptr_t virtual_address, ULONG64 flags );
void vmm_unmap_page( pagemap_t *pagemap, uintptr_t virtual_address );
void vmm_memcpy( pagemap_t *pagemap_1, uintptr_t virtual_address_1,
				pagemap_t *pagemap_2, uintptr_t virtual_address_2,
				size_t count );
uintptr_t vmm_virt_to_phys( pagemap_t *pagemap, uintptr_t virtual_address );
uintptr_t vmm_get_kernel_address( pagemap_t *pagemap, uintptr_t virtual_address );
int MmInitializeVmm( );

void MmLoadVirtualPage2( ULONG64 pml4 );



//pmm.c

#define PAGE_SIZE 0x1000
#define PHYS_MEM_OFFSET 0xffff800000000000

void *pmalloc( size_t pages );
void *pcalloc( size_t pages );
void pmm_free_pages( void *adr, size_t page_count );
int MmInitializePmm( struct stivale2_struct_tag_memmap *memory_info );

//heap.c

void *malloc( size_t size );
void free( void *ptr );
void *krealloc( void *ptr, size_t size );
void *kcalloc( size_t size );
int kheap_init( );



void *liballoc_alloc( int size );