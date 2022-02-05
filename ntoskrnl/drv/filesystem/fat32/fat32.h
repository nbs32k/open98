#pragma once
#include <windef.h>

typedef struct 
{

	UCHAR SectorsPerFat[ 4 ]; // 4
	USHORT Flags; //2
	USHORT FatVerNumber; //2
	UCHAR ClusterNumberOfRoot[ 4 ]; //4
	USHORT SectorNumberFSInfo; //2
	USHORT SectorBackupBoot; //2
	UCHAR Reserved[ 12 ]; //12
	UCHAR DriveNumber;//1
	UCHAR FlagsNT;//1
	UCHAR Signature;//1
	UCHAR VolumeIDSerial[ 4 ];//4
	UCHAR VolumeLabel[ 11 ];//11
	UCHAR SystemIdentifier[ 8 ];//8
	UCHAR BootCode[ 420 ];//420
	USHORT BiosSignature;//2
	//476 Bytes

}__attribute__( ( packed ) ) ExtendedFat32;


typedef struct BiosParameterBlock
{

	UCHAR JumpCode[ 3 ]; //3
	UCHAR OEMIdentifier[ 8 ]; //8
	USHORT BytesPerSector; //2
	UCHAR SectorsPerCluster; //1
	USHORT ReservedSectorCount; //2
	UCHAR NumberOfFAT; //1
	USHORT NumberDirectories; //2
	USHORT TotalSectors; //2
	UCHAR MediaDescriptorType; //1
	USHORT NumberOfSectors;	//2	//only FAT12/16
	USHORT NumberOfSPT;	//2		//nSectors Per Track
	USHORT NumberOfHeads; //2
	UCHAR NumbersOfHiddenSectors[ 4 ];//4
	UCHAR LargeSectorCount[ 4 ];//4
	//36 Bytes

	ExtendedFat32 ExtendedBootRecord;

}__attribute__( ( packed ) ) BPBTable;


#define READONLY  1
#define HIDDEN    (1 << 1)
#define SYSTEM    (1 << 2)
#define VolumeID  (1 << 3)
#define DIRECTORY (1 << 4)
#define ARCHIVE   (1 << 5)
#define LFN (READONLY | HIDDEN | SYSTEM | VolumeID)

struct dir_entry
{
	CHAR *name;
	UCHAR dir_attrs;
	UINT first_cluster;
	UINT file_size;
};

struct directory
{
	UINT cluster;
	struct dir_entry *entries;
	UINT num_entries;
};



VOID
FatInitialize(

);

