#pragma once

#include "../windef.h"
#include "bitmap.h"





#define PAGE_PRESENT 1 << 0
#define PAGE_WRITABLE 1 << 1
#define PAGE_USER 1 << 2
#define PAGE_WRITE_THROUGH 1 << 3
#define PAGE_CACHE_DISABLE 1 << 4
#define PAGE_ACCESSED 1 << 5
#define PAGE_DIRTY 1 << 6
#define PAGE_GLOBAL 1 << 8
#define PAGE_NO_EXECUTE 1 << 63
#define PAGE_ADDRESS_MASK 0x000FFFFFFFFFF000
typedef struct {
    USHORT page_offset : 12;
    USHORT pt_offset : 9;
    USHORT pd_offset : 9;
    USHORT pdp_offset : 9;
    USHORT pml4_offset : 9;
    USHORT sign_extend : 16;
} __attribute__((packed)) parsed_address_t;

typedef union {
    ULONG64 int_address;
    parsed_address_t parsed_address;
} address_t;

static ULONG64 PML4[512] __attribute__((aligned(4096)));

static bitmap_t *page_bitmap;

void setup_paging(ULONG64 total_memory);
void map_page(ULONG64 physical, ULONG64 virtual);