#include "paging.h"


void map_page(ULONG64 physical, ULONG64 virtual) {
    address_t a;
    a.int_address = virtual;
    ULONG64 *pdp = (PML4[a.parsed_address.pml4_offset] & PAGE_ADDRESS_MASK) >> 12;
    ULONG64 *pd = (pdp[a.parsed_address.pdp_offset] & PAGE_ADDRESS_MASK) >> 12;
    ULONG64 *pt = (pd[a.parsed_address.pd_offset] & PAGE_ADDRESS_MASK) >> 12;
    pt[a.parsed_address.pt_offset] = (physical & (PAGE_ADDRESS_MASK >> 12));
}

void setup_paging(ULONG64 total_memory) {
    page_bitmap->size = (total_memory / 4096 + 1) * 8;
    memset(page_bitmap->contents, 0, page_bitmap->size / 8);
}
