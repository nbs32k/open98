#pragma once
#include <ntdef.h>
#include <lib/utill.h>
#define HEADER_SIZE 128

enum ustar_filetypes
{
	REGULAR_FILE = '0',
	HARDLINK = '1',
	SYMLINK = '2',
	CHARDEV = '3',
	BLOCKDEV = '4',
	DIRECTORY_VFS = '5',
	FIFO = '6'
};

struct ustar_file_header_t
{
	CHAR name[ 100 ];
	CHAR mode[ 8 ];
	CHAR uid[ 8 ];
	CHAR gid[ 8 ];
	CHAR size[ 12 ];
	CHAR mtime[ 12 ];
	CHAR chksum[ 8 ];
	CHAR typeflag[ 1 ];
	CHAR link[ 100 ];
	CHAR signature[ 6 ];
	CHAR version[ 2 ];
	CHAR owner[ 32 ];
	CHAR group[ 32 ];
	CHAR dev_maj[ 8 ];
	CHAR dev_min[ 8 ];
	CHAR prefix[ 155 ];
};

struct ustar_header_t
{
	struct ustar_file_header_t *headers[ HEADER_SIZE ];
	UINT address[ HEADER_SIZE ];
	int count;
};

BOOLEAN initrd;

struct ustar_header_t *ustar_headers;

UINT getsize( const char *s );

VOID ustar_list( );

PCHAR ustar_cat( PCHAR name );

int ustar_getid( PCHAR name );

int ustar_search( PCHAR filename, PCHAR *contents );

VOID KdInitRAMFS( UINT address );