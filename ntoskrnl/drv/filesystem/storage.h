#pragma once

#include <ntdef.h>
#include "../../ke/ki.h"
#include "../../lib/utill.h"
#include <lib/vec.h>

//ata.c
///////////////
/////////////
///////////
UCHAR
KiATAIdentify(

);

VOID
KiATAReadDiskEx(
	UINT LBA,
	UCHAR sectorcount,
	UCHAR *target
);

VOID
KiATAReadDisk(
	ULONG64 LBA,
	USHORT sectorcount,
	UCHAR *target
);

VOID
KiATAWriteDisk(
	ULONG64 LBA,
	USHORT sectorcount,
	UCHAR *target
);

//parttable.c
///////////////
/////////////
///////////
typedef struct
{
	UCHAR  Status;
	UCHAR  FirstSector[ 3 ];
	UCHAR  Type;
	UCHAR  LastSector[ 3 ];
	UINT   StartingSector;
	UINT   SectorCount;
} PartitionTable;

typedef struct
{
	CHAR BootStrap[ 446 ];
	PartitionTable PartitionTable[ 4 ];
	UCHAR Signature[ 2 ];
}
__attribute__( ( packed ) )
MasterBootRecord;



UINT
FatAcquirePartitionSector(
	INT iPartitionNum
);


