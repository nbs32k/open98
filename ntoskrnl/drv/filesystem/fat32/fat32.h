#pragma once
#include <ntdef.h>
#include <stdint.h>
#include <lib/utill.h>
#include <drv/filesystem/storage.h>


STATIC UCHAR LetterAssignment[ 4 ] = { 'C', 'D', 'E', 'F' };

struct BiosParameterBlockStruct
{
	USHORT BytesPerSector;          // IMPORTANT
	UCHAR SectorsPerCluster;        // IMPORTANT
	USHORT ReservedSectors;          // IMPORTANT
	UCHAR FatCount;                  // IMPORTANT
	USHORT DirectoryEntries;
	USHORT TotalSectors;
	UCHAR MediaDescriptorType;
	USHORT SectorsPerFat; // FAT12/FAT16 only.
	USHORT SectorsPerTrack;
	USHORT HeadsOrSizesOnMedia;
	UINT HiddenSectors;
	UINT LargeSectorsOnMedia;  // This is set instead of TotalSectors if it's > 65535

	// Extended Boot Record
	UINT SectorsPerFat32;   // IMPORTANT
	USHORT flags;
	USHORT FatVersion;
	UINT ClusterRootDirectory;   // IMPORTANT
	USHORT FSInfoSectorNumber;
	USHORT BackupBootSectorNumber;
	UCHAR DriveNumber;
	UCHAR WindowsFlags;
	UCHAR Signature;                  // IMPORTANT
	UINT VolumeSerial;
	CHAR VolumeLabel[ 12 ];
	CHAR SystemID[ 9 ];
};

#define READONLY  1
#define HIDDEN    (1 << 1)
#define SYSTEM    (1 << 2)
#define VolumeID  (1 << 3)
#define DIRECTORY (1 << 4)
#define ARCHIVE   (1 << 5)
#define LFN (READONLY | HIDDEN | SYSTEM | VolumeID)

struct DirectoryEntry
{
	CHAR *name;
	UCHAR dir_attrs;
	UINT first_cluster;
	UINT file_size;
};

struct Directory
{
	UINT cluster;
	struct DirectoryEntry *entries;
	UINT num_entries;
};

// REFACTOR
// I want to get rid of this from the header. This should be internal
// implementation, but for now, it's too convenient for stdio.c impl.

// EOC = End Of Chain
#define EOC 0x0FFFFFF8

typedef struct FAT32
{
	//FILE *f;
	UCHAR LetterAssigned; //C
	UCHAR RootPath[ 2 ];
	UINT *FAT;
	struct BiosParameterBlockStruct BiosParameterBlock;
	UINT PartitionSector; //Where does the partition start from (sector number)
	UINT FatSector;
	UINT ClusterSector;
	UINT ClusterSize;
	UINT ClusterAllocationHint;
}FAT32;

INT iLastDriveCount; //extern

VOID
FatGetCluster(
	FAT32 *fs,
	UCHAR *buff,
	UINT cluster_number
);

UINT
FatGetNextClusterID(
	FAT32 *fs,
	UINT cluster
);

// END REFACTOR

FAT32*
FatInitialize(
	INT iPartitionNumber
);
VOID
FatDestroy(
	FAT32 *fs
);



CONST struct BiosParameterBlockStruct*
FatGetBiosParameterBlock(
	FAT32 *fs
);

VOID
FatPopulateRootDir(
	FAT32 *fs,
	struct Directory *dir
);

VOID
FatPopulateDir(
	FAT32 *fs,
	struct Directory *dir,
	UINT cluster
);

VOID
FatFreeDirectory(
	FAT32 *fs,
	struct Directory *dir
);


UCHAR*
FatReadFile(
	FAT32 *fs,
	struct DirectoryEntry *dirent
);

VOID
FatWriteFile(
	FAT32 *fs,
	struct Directory *dir,
	UCHAR *file,
	CHAR *fname,
	UINT flen
);
VOID
FatMakeDirectory(
	FAT32 *fs,
	struct Directory *dir,
	CHAR *dirname
);

VOID
FatDeleteFile(
	FAT32 *fs,
	struct Directory *dir,
	CHAR *filename
);


VOID
FatPrintDirectory(
	FAT32 *fs,
	struct Directory *dir
);

UINT
FatGetFreeClusters(
	FAT32 *fs
);

extern FAT32 *Partition[4];


FAT32*
FatResolveByPrefix(
	CHAR* pcPath
);

#define MAX_PARTITION 4

/////
typedef struct FILE FILE;

FILE*
fopen(
	const CHAR *pathname,
	const CHAR *mode
);

static inline INT
FileEntryForPath(
	const CHAR *path,
	struct DirectoryEntry *entry
);

INT
fclose(
	FILE *stream
);

INT
fread(
	void *ptr,
	INT size,
	INT nmemb,
	FILE *stream
);