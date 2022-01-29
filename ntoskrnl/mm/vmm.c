#include <stddef.h>
#include <stdint.h>
#include "mm.h"

/*                                BASIC EXPLANATION
 *
 * virtual memory is a memory management technique that allows us to map virtual
 * addresses to physical addresses
 *
 * for example, a program wants two bytes of memory; our virtual allocator will give the
 * program a memory area that goes from 0x00 to 0x02
 *
 *                               0x00   0x01   0x02
 *                                  |      |      |
 *
 * but usually, there are several programs; what if one of them also wants two bytes?
 * well, we will also give it another memory area that goes from 0x00 to 0x02
 *
 * physically, that is totally impossible, but not virtually, what is really
 * happening is that the programs are receiving 0x00 - 0x02 and 0x02 - 0x04, respectively,
 * but both think that their memory space start at 0x00, something that simplifies everything
 *
 * that means that both memory areas, from the program perspective, are kind of isolated,
 * the program gets its own address space
 *
 * anyways, that is for the userspace, but the basic concept would be that a virtual address
 * can be mapped to a physical address [0x0 -> 0xFFFF], in fact several virtual memory spaces
 * can be mapped to a single physical address (memory sharing)
 */

 /*                                   REFERENCES
  *
  * from https://www.amd.com/system/files/TechDocs/24593.pdf
  * - 5.3 Long-Mode Page Translation
  *      - 5.3.1 CR3 register
  *      - 5.3.3 4-Kbyte Page Translation
  * - 5.4 Page-Translation-Table Entry Fields
  *      - 5.4.1 Field Definitions
  */

ULONG64 *MiKernelPage = 0; /* MiKernelPage is a pointer to PML4, this address will be stored in CR3 */

VOID
MmLoadVirtualPage(
	ULONG64 pml4
)
{
	/* in long mode, the CR3 register is used to point to the PML4 base address */
	asm( "mov %0, %%cr3" ::"a"( pml4 ) );
}

VOID
MiVirtualInvlpg(
	ULONG64 virt_addr
)
{
	/* TLB stands for translation lookaside buffer */
	asm( "invlpg (%0)" ::"r"( virt_addr ) ); /* invlpg flushes/invalidates the specified TLB entry */
}

ULONG64*
MiAcquireVirtualTable(
	ULONG64 *table,
	USHORT offset,
	ULONG64 flags
)
{
	/* first of all, we need to check if that offset (entry) has any value,
	 * if it doesn't, we will point it to some new memory allocated with our kheap */
	if ( !( table[ offset ] & 0x01 ) )
	{
		/* one table has 512 entries, 64 bits each one, we need to alloc 512*sizeof(64) = 4096 */
		table[ offset ] = ( ULONG64 )kmalloc( PAGE_SIZE );
		table[ offset ] |= flags;
	}
	/* according to the AMD specification, each entry has the following structure (simplified)
	 *
	 *             | Other | Table base address field | Flags / Other |
	 *             |63   52|51                      12|11            0|
	 *
	 * a base address size is a 52-bit value, the [Other usage] part ought to be zeros (that's
	 * why we are using AND, in case there is some bit set there)
	 *
	 * NOTE 0xFFF is equivalent to 12 bits (flags size), ~0xFFF, everything else
	 */
	return ( ULONG64 * )( ( table[ offset ] & ~0xFFF ) + HIGHER_HALF );
}

VOID
MmMapVirtualPage(
	ULONG64 *page_map,
	ULONG64 phys_addr,
	ULONG64 virt_addr,
	ULONG64 flags
)
{
	/* from the virtual address, we can get the different tables offsets
	 *
	 *                            VIRTUAL / LINEAR ADDRESS
	 *
	 *    | Sign extend | PML4 offset | PDP offset | PD offset | PT offset | Phys page offset  |
	 *    |63         48|47         39|38        30|29       21|20       12|11                0|
	 *
	 * NOTE 0x1FF is equivalent to 9 bits, the max offset is 512 (2^9), so it fits perfectly */
	USHORT pml4_offset = ( virt_addr >> 39 ) & 0x1FF;
	USHORT pdp_offset = ( virt_addr >> 30 ) & 0x1FF;
	USHORT pd_offset = ( virt_addr >> 21 ) & 0x1FF;
	USHORT pt_offset = ( virt_addr >> 12 ) & 0x1FF;

	/* now, attending to the following pointer to pointer chain, we can access the different
	 * tables
	 *
	 *       CR3 ---> | PML4 |
	 *                     |---> | PDP |
	 *                               |---> | PD |
	 *                                        |---> | PT |
	 *                                                 |---> Phys page
	 *                                                       offset (pp)
	 *
	 * for example, if we want to know where the PDP table is, we just need to look
	 * for its address at its offset in the PML4 table, we can do that with something similar
	 * to pdp = pml4[pml4_offset]
	 *
	 * each table has 512 entries, so the offset is pointing to one of these entries
	 */

	ULONG64 *pdp = MiAcquireVirtualTable( page_map, pml4_offset, flags );
	ULONG64 *pd = MiAcquireVirtualTable( pdp, pdp_offset, flags );
	ULONG64 *pt = MiAcquireVirtualTable( pd, pd_offset, flags );

	pt[ pt_offset ] |= phys_addr | flags | 0x01; /* the physical page offset that we want */
	/* NOTE 0x01 is there to set the page present even if we didn't specify any flags   */

	MiVirtualInvlpg( virt_addr ); /* flush the TLB */
}

VOID
MmMapMultiplePage(
	ULONG64 *page_map,
	ULONG64 base,
	ULONG64 end,
	ULONG64 offset,
	ULONG64 flags
)
{
	for ( ULONG64 i = base; i < end; i += PAGE_SIZE )
		MmMapVirtualPage( page_map, i, i + offset, flags );
}

VOID
MiVirtualizeInit(

)
{
	/* according to the AMD documentation [5.4.1 Field Definitions], the [Flags / Other] bits
	 * are the following ones
	 *
	 *            11     10     9     8     7     6     5     4     3     2     1     0
	 *            x      x      x     G     PS    D     A     PCD   PWT   U/S   R/W   P
	 *
	 * main ones are described as following
	 *
	 *  P (present) this bit indicates whether the page-translation table or physical page is loaded
	 *              in physical memory; a page fault will be raised if an attempt is made to access
	 *              a table or page when the bit is 0
	 *  R/W (read/write) controls read/write access
	 *  U/S (user/supervisor) when the U/S bit is 0, access is restricted to supervisor level; when
	 *                        the U/S bit is set to 1, access is for both
	 */
	MiKernelPage = ( ULONG64* )kmalloc( PAGE_SIZE );
	memset( MiKernelPage, 0, PAGE_SIZE );
	DbgPrintFmt( "[VIRT] MiKernelPage = 0x%x, remapping memory: ", MiKernelPage );

	/* limine mapped the kernel for us, but let's do it again */
	MmMapMultiplePage( MiKernelPage, 0, 0x8000000, 0, 0b11 );
	DbgPrintFmt( "[id map]" );
	MmMapMultiplePage( MiKernelPage, 0, 0x100000000, HIGHER_HALF, 0b11 );
	DbgPrintFmt( "[data]" );
	MmMapMultiplePage( MiKernelPage, 0, 0x8000000, KERNEL_OFFS, 0b11 );
	DbgPrintFmt( "[kernel]" );

	MmLoadVirtualPage( ( ULONG64 )MiKernelPage );

	DbgPrintFmt( "[done]\n" );
}