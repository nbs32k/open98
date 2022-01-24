#include "../windef.h"
#include "../stivale2.h"


#define MEM_BLOCK_LIST_SIZE 8192
#define MEM_BLOCK_INCREASE_SIZE 4
struct mem_block
{
	BOOLEAN is_used;
	BOOLEAN is_end;
	INT size;
	INT addr;
};

void* malloc( INT size );
VOID MmPopulateBlock( INT iStartAddr );
UINT MmFindAvailableBlock( INT iStartIndex, INT iSize );
PVOID malloc( INT iSize );
VOID free( void* Mem );

VOID MmInit( struct stivale2_struct_tag_memmap* MemMap );