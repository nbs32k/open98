#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stivale2.h>
#include "../lib/utill.h"
#include "../ke/ki.h"

//Physical Memory Manager

#define PAGE_SIZE 4096
#define CHECK_PAGE(page) (KiPhysBitmap[page / 8] & (1 << (page % 8)))

extern UCHAR *KiPhysBitmap;

VOID MiPhysicalInit( struct stivale2_struct_tag_memmap *mem_tag );

//Virtual Memory Manager

#define HIGHER_HALF 0xffff800000000000
#define KERNEL_OFFS 0xffffffff80000000

extern ULONG64 *MiKernelPage;

VOID
MmLoadVirtualPage(
	ULONG64 pml4
);

VOID
MmMapMultiplePage(
	ULONG64 *page_map,
	ULONG64 base,
	ULONG64 end,
	ULONG64 offset,
	ULONG64 flags
);

VOID
MiVirtualizeInit(

);

//Heap

VOID*
kmalloc(
	INT size
);