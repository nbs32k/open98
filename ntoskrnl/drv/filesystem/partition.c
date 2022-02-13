#include "storage.h"
#include "fat32/fat32.h"

MasterBootRecord MBRTable;

UINT
FatAcquirePartitionSector( 
	INT iPartitionNum
)
{
	KiATAReadDiskEx( 0, 1, ( UCHAR* )&MBRTable );

	if ( iPartitionNum > MAX_PARTITION - 1 ) return -1;

	if ( MBRTable.Signature[ 0 ] == 0x55 && MBRTable.Signature[ 1 ] == 0xAA ) //check for bios sig
		if ( MBRTable.PartitionTable[ iPartitionNum ].StartingSector == NULL )
			return -1;
		else return MBRTable.PartitionTable[ iPartitionNum ].StartingSector;

	return -1;
}