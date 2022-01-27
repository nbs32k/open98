#include "mm.h"

struct mem_block mem_blocks[ MEM_BLOCK_LIST_SIZE ];

VOID MmPopulateBlock(
	INT iStartAddr
)
{
	INT iLastAddr = iStartAddr;
	UINT uiLastSize = 1;

	for ( int i = 0; i < MEM_BLOCK_LIST_SIZE; i++ )
	{
		mem_blocks[ i ].addr = iLastAddr;
		mem_blocks[ i ].size = uiLastSize;
		iLastAddr += mem_blocks[ i ].size;
		uiLastSize += MEM_BLOCK_INCREASE_SIZE;
	}
}

UINT MmFindAvailableBlock(
	INT iStartIndex,
	INT iSize
)
{
	struct mem_block block = mem_blocks[ iStartIndex ];
	if ( !block.is_used && block.size > iSize )
	{
		return iStartIndex; /* we found a free block */
	}
	else
	{
		return MmFindAvailableBlock( iStartIndex + 1, iSize );
	}
}

PVOID malloc(
	INT iSize
)
{
	UINT uiBlock = MmFindAvailableBlock( 0, iSize );
	mem_blocks[ uiBlock ].is_used = TRUE;

	return mem_blocks[ uiBlock ].addr;
}

VOID free(
	void* Mem
)
{
	for ( int i = 0; i < MEM_BLOCK_LIST_SIZE; i++ )
	{
		if ( mem_blocks[ i ].addr == ( INT )Mem )
		{
			mem_blocks[ i ].is_used = FALSE;
			return;
		}
	}
}


VOID MmInit(
	struct stivale2_struct_tag_memmap* MemMap
)
{
	ULONG64 ulBase, ulSize;

	for ( INT i = 0; i < MemMap->entries; i++ )
	{
		struct stivale2_mmap_entry entry = MemMap->memmap[ i ];

		if ( entry.type == STIVALE2_MMAP_USABLE )
		{
			if ( entry.length > ulSize )
			{
				ulBase = entry.base;
				ulSize = entry.length;
			}
		}
	}

	MmPopulateBlock( ulBase );


}