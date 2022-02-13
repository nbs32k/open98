#include <ntdef.h>
#include "../drv/filesystem/fat32/fat32.h"

typedef struct NtCreateFileStruct
{
	PCHAR	lpFileName;
	UINT	dwDesiredAccess;
	ULONG	dwShareMode;
	ULONG	lpSecurityAttributes;
	ULONG	dwCreationDisposition;
	ULONG	dwFlagsAndAttributes;
	HANDLE	hTemplateFile;
	FAT32*  Stream;
}IO_FILE, *PIO_FILE;

//dwDesiredAccess
#define GENERIC_READ	0x00000001
#define GENERIC_WRITE	0x00000002
#define GENERIC_ALL		0x00000003

//dwShareMode
#define FILE_SHARE_DELETE	0x00000004
#define FILE_SHARE_READ		0x00000001
#define FILE_SHARE_WRITE	0x00000002

//lpSecurityAttributes

//dwCreationDisposition
#define CREATE_ALWAYS		2
#define CREATE_NEW			1
#define OPEN_ALWAYS			4
#define OPEN_EXISTING		3
#define TRUNCATE_EXISTING	5


HANDLE*
NtCreateFile(
	PCHAR	lpFileName,
	UINT	dwDesiredAccess,
	ULONG	dwShareMode,
	ULONG	lpSecurityAttributes,
	UINT	dwCreationDisposition,
	ULONG	dwFlagsAndAttributes,
	HANDLE	hTemplateFile
);

VOID
NtClose(
	PHANDLE hHandle
);

BOOLEAN
NtWriteFile(
	PHANDLE hFile,
	PVOID	lpBuffer,
	ULONG	nNumberOfBytesToWrite,
	ULONG	*nNumberOfBytesWritten,
	ULONG	lpOverlapped //what the fuck.
);

BOOLEAN
NtReadFile(
	PHANDLE hFile,
	UCHAR*	lpBuffer,
	ULONG	nNumberOfBytesToRead,
	ULONG	*nNumberOfBytesRead,
	ULONG	lpOverlapped //what the fuck.
);