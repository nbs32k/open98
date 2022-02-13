#include "fsrtl.h"


HANDLE*
NtCreateFile(
	PCHAR	lpFileName,
	UINT	dwDesiredAccess,
	ULONG	dwShareMode,
	ULONG	lpSecurityAttributes,
	UINT	dwCreationDisposition,
	ULONG	dwFlagsAndAttributes,
	HANDLE	hTemplateFile
)
{
	if ( dwCreationDisposition == NULL ) return NULL;

	PIO_FILE pioFile = ( PIO_FILE )malloc(sizeof( IO_FILE ));


	pioFile->lpFileName = lpFileName;
	pioFile->dwDesiredAccess = dwDesiredAccess;
	pioFile->dwShareMode = dwShareMode;
	pioFile->lpSecurityAttributes = lpSecurityAttributes;
	pioFile->dwCreationDisposition = dwCreationDisposition;
	pioFile->dwFlagsAndAttributes = dwFlagsAndAttributes;
	pioFile->hTemplateFile = hTemplateFile;


	//drill
	//UCHAR* PhysicalDrive = ( UCHAR* )malloc( 18 );
	//RtlCopyMemory( PhysicalDrive, pioFile->lpFileName, 17 );
	////PhysicalDriveX -> where X is an integer
	//if ( strcmp( PhysicalDrive, "\\\\.\\PhysicalDrive" ) != 0 )
		pioFile->Stream = fopen( lpFileName, NULL );

	//free( PhysicalDrive );

	/*if ( dwCreationDisposition == CREATE_NEW && pioFile->FileSystem == NULL )
		NtWriteFile( pioFile, ( UCHAR* )"", NULL, NULL, NULL );*/

	
	
	free( pioFile );

	return pioFile;
}


BOOLEAN
NtWriteFile( 
	PHANDLE hFile,
	PVOID	lpBuffer,
	ULONG	nNumberOfBytesToWrite,
	ULONG	*nNumberOfBytesWritten,
	ULONG	lpOverlapped //what the fuck.
)
{
	if ( hFile == NULL )
		return FALSE;

	PIO_FILE pioFile = hFile;
													 
	//UCHAR* PhysicalDrive = ( UCHAR* )malloc( 18 );
	//RtlCopyMemory( PhysicalDrive, pioFile->lpFileName, 17 );
	//								  //PhysicalDriveX -> where X is an integer
	//if ( strcmp( PhysicalDrive, "\\\\.\\PhysicalDrive" ) == 0 );

	//free( PhysicalDrive );

	FAT32* lol = FatResolveByPrefix( pioFile->lpFileName );
	if ( pioFile->dwDesiredAccess != GENERIC_WRITE &&
		pioFile->dwDesiredAccess != GENERIC_ALL )
		return FALSE;

	

	//Get directory FAT by where the file is. (partition)

	/*struct Directory dir;
	FatPopulateDir( , &dir, 2 );

	FatWriteFile( pioFile->FileSystem, dir, ( UCHAR* )lpBuffer, pioFile->lpFileName, 0 );*/
	//( *nNumberOfBytesWritten ) = sizeof( lpBuffer );
	return TRUE;

}

BOOLEAN
NtReadFile(
	PHANDLE hFile,
	UCHAR*	lpBuffer,
	ULONG	nNumberOfBytesToRead,
	ULONG	*nNumberOfBytesRead,
	ULONG	lpOverlapped //what the fuck.
)
{
	if ( hFile == NULL )
		return FALSE;

	PIO_FILE pioFile = hFile;

	if ( pioFile->dwDesiredAccess != GENERIC_READ &&
		pioFile->dwDesiredAccess != GENERIC_ALL )
		return FALSE;

	if ( pioFile->dwCreationDisposition != OPEN_EXISTING )
		return FALSE;

	//DbgPrintFmt( "test -- %s", pioFile->lpFileName );

	//UCHAR* PhysicalDrive = ( UCHAR* )malloc( 18 );
	//RtlCopyMemory( PhysicalDrive, pioFile->lpFileName, 17 );

	//if ( strcmp( PhysicalDrive, "\\\\.\\PhysicalDrive" ) == 0 )
	//{
	//	DbgPrintFmt( "Success" );
	//	KiATAReadDiskEx( 0, 1, lpBuffer );
	//	( *nNumberOfBytesRead ) = ( INT )nNumberOfBytesToRead;

	//	free( PhysicalDrive );
	//	return TRUE;
	//}

	if ( pioFile->Stream == NULL ) return FALSE;

	INT iBytes = fread( lpBuffer, nNumberOfBytesToRead, 1, pioFile->Stream );

	( *nNumberOfBytesRead ) = iBytes;
	return TRUE;
}

VOID
NtClose( 
	PHANDLE hHandle
)
{
	PIO_FILE pioFile = hHandle;
	//DbgPrintFmt( "lpFileName: %s", pioFile->lpFileName );
	free( pioFile->Stream );
}