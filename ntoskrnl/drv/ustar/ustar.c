#include "ustar.h"


BOOLEAN initrd = FALSE;

struct ustar_header_t *ustar_headers;

UINT getsize( const char *s )
{
	ULONG64 ret = 0;
	while ( *s )
	{
		ret *= 8;
		ret += *s - '0';
		s++;
	}
	return ret;
}

INT ustar_parse( UINT address )
{
	UINT i;
	for ( i = 0;; i++ )
	{
		struct ustar_file_header_t *header = ( struct ustar_file_header_t * )address;

		if ( strcmp( header->signature, "ustar" ) )
		{
			break;
		}

		uintptr_t size = getsize( header->size );
		ustar_headers->headers[ i ] = header;
		ustar_headers->address[ i ] = address + 512;
		ustar_headers->count++;

		address += ( ( ( size + 511 ) / 512 ) + 1 ) * 512;
	}
	for ( int g = 1; g < ustar_headers->count; g++ )
	{
		memmove( ustar_headers->headers[ g ]->name, ustar_headers->headers[ g ]->name + 1, strlen( ustar_headers->headers[ g ]->name ) );
	}
	ustar_headers->count--;
	i--;
	return i;
}

void not_initialized( )
{
	DbgPrintFmt( "[NT-Kernel] Initrd has not been initialized!" );
}
PCHAR humanify( int bytes, PCHAR buf )
{
	int i = 0;
	const PCHAR units[ ] = { "B", "KB", "MB", "GB", "TB" };
	for ( i = 0; bytes > 1024; i++ )
	{
		bytes /= 1024;
	}
	sprintf( buf, "%d%s", bytes, units[ i ] );
	return buf;
}
void ustar_list( )
{
	if ( !initrd )
	{
		not_initialized( );
		return;
	}
	int size = 0;
	//DbgPrintFmt("Total %d items:\n--------------------\n", ustar_headers->count);
	for ( int i = 1; i < ustar_headers->count + 1; i++ )
	{
		char buf[ 10 ];
		switch ( ustar_headers->headers[ i ]->typeflag[ 0 ] )
		{
			case REGULAR_FILE:

				DbgPrintFmt( "File --> %s \n", ustar_headers->headers[ i ]->name, humanify( oct_to_dec( string_to_int( ustar_headers->headers[ i ]->size ) ), buf ) );
				size += oct_to_dec( string_to_int( ustar_headers->headers[ i ]->size ) );
				break;
			case SYMLINK:
				DbgPrintFmt( "Linkage --> %s --> %s\n", ustar_headers->headers[ i ]->name, ustar_headers->headers[ i ]->link );
				break;
			case DIRECTORY_VFS:
				DbgPrintFmt( "Dir --> %s\n", ustar_headers->headers[ i ]->name );
				break;
			default:
				DbgPrintFmt( "Unknown --> %s\n", ustar_headers->headers[ i ]->name );
				break;
		}
	}
	char buf1[ 10 ];
	DbgPrintFmt( "%d Items Found!", ustar_headers->count );
	//printf("--------------------\nTotal size: %s\n", humanify(size, buf1));
}

PCHAR ustar_cat( PCHAR name )
{
	if ( !initrd )
	{
		not_initialized( );
		return 0;
	}
	PCHAR contents;
	int i = 0;
	i = ustar_getid( name );
	switch ( ustar_headers->headers[ i ]->typeflag[ 0 ] )
	{
		case REGULAR_FILE:
			if ( ustar_search( name, &contents ) != 0 )
			{
				DbgPrintFmt( "--BEGIN-- %s\n", name );
				DbgPrintFmt( "%s", contents );
				DbgPrintFmt( "--END-- %s\n", name );
			}
			else
			{
				goto Error;
			}
			break;
		default:
			contents = "";
			// printf("\033[31m\"%s\" is not a regular file!\033[0m\n", name);
			break;
	}
	return contents;
Error:
	// printf("Invalid file name!");
	return "Invalid file name!";
}

int ustar_getid( PCHAR name )
{
	if ( !initrd )
	{
		not_initialized( );
		return 0;
	}
	for ( int i = 0; i < ustar_headers->count; ++i )
	{
		if ( !strcmp( ustar_headers->headers[ i ]->name, name ) )
		{
			return i;
		}
	}
	return 0;
}

int ustar_search( PCHAR filename, PCHAR *contents )
{
	if ( !initrd )
	{
		not_initialized( );
		return 0;
	}
	for ( int i = 1; i < ustar_headers->count + 1; i++ )
	{
		if ( !strcmp( ustar_headers->headers[ i ]->name, filename ) )
		{
			*contents = ( PCHAR )ustar_headers->address[ i ];
			return 1;
		}
	}
	return 0;
}

void KdInitRAMFS( UINT address )
{
	DbgPrintFmt("Initializing initrd");

	ustar_parse( address );
	initrd = TRUE;

	// DbgPrintString("Initialized initrd\n");
}