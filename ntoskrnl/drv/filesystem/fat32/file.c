#include "fat32.h"

typedef struct FILE
{
	//struct DirectoryEntry file_ent;
	UINT curr_cluster;
	UINT file_size; // total file size
	UINT fptr; // index INTo the file
	UINT buffptr; // index INTo currbuf
	UCHAR path[ 3 ]; //mostly needed the C://
	UINT currbuf[ ]; // flexible member for current cluster
} FILE;


static inline INT
FileEntryForPath(
	const CHAR *path,
	struct DirectoryEntry *entry
)
{
	struct Directory dir;

	UCHAR* RemainingPath = malloc( sizeof( path ) );
	RtlCopyMemory( RemainingPath, path + 2, RtlStringLength( path ) - 2 );
	DbgPrintFmt( "Drive prefix: %s\nRemaining Path: %s", 0, RemainingPath );


	FAT32* filesystem = FatResolveByPrefix( path );
	if ( filesystem == NULL ) return 0;

	DbgPrintFmt( "DAWG1" );
	FatPopulateRootDir( filesystem, &dir );
	INT found_file = 0;
	/*if ( path[ 0 ] != '/' )*/

	DbgPrintFmt( "DAWG2" );
	if ( RemainingPath[ 0 ] != '\\' )
	{
		return found_file;
	}
	DbgPrintFmt( "DAWG3" );
	CHAR *cutpath = strdup( RemainingPath );
	CHAR *tokstate = NULL;
	DbgPrintFmt( "DAWG4" );

	//CHAR *next_dir = strtok_r( cutpath, "/", &tokstate );
	CHAR *next_dir = strtok_r( cutpath, "\\", &tokstate );
	struct DirectoryEntry *currentry = NULL;
	entry->name = NULL;
	DbgPrintFmt( "DAWG5" );
	while ( next_dir )
	{
		INT found = 0;
		for ( INT entryi = 0; entryi < dir.num_entries; entryi++ )
		{
			currentry = &dir.entries[ entryi ];
			if ( strcmp( currentry->name, next_dir ) == 0 )
			{
				if ( entry->name ) free( entry->name );
				*entry = *currentry;
				// TODO: Make sure this doesn't leak. Very dangerous:
				entry->name = strdup( currentry->name );

				INT cluster = currentry->first_cluster;
				FatFreeDirectory( filesystem, &dir );
				DbgPrintFmt( "DAWG6" );
				FatPopulateDir( filesystem, &dir, cluster );
				DbgPrintFmt( "DAWG7" );
				found = 1;
				break;

			}
		}
		if ( !found )
		{
			free( cutpath );
			FatFreeDirectory( FatResolveByPrefix( path ), &dir );
			DbgPrintFmt( "DAWG8" );
			return 0;
		}
		/*next_dir = strtok_r( NULL, "/", &tokstate );*/
		next_dir = strtok_r( NULL, "\\", &tokstate );
		DbgPrintFmt( "DAWG9" );
	}
	FatFreeDirectory( FatResolveByPrefix( path ), &dir );
	DbgPrintFmt( "DAWG10" );
	free( cutpath );
	return 1;
}

FILE*
fopen(
	const CHAR *pathname,
	const CHAR *mode
)
{
	struct DirectoryEntry entry;
	if ( !FileEntryForPath( pathname, &entry ) )
	{
		free( entry.name );
		return NULL;
	}
	    DbgPrintFmt("Got entry: %s [%d]\n", entry.name, entry.first_cluster);
	free( entry.name );

	FAT32* filesystem = FatResolveByPrefix( pathname );

	FILE *f = malloc( sizeof( FILE ) + filesystem->ClusterSize );
	f->curr_cluster = entry.first_cluster;
	f->file_size = entry.file_size;
	f->fptr = 0;
	f->buffptr = 0;
	RtlCopyMemory( f->path, pathname, 3 );
	
	FatGetCluster( filesystem, f->currbuf, f->curr_cluster );
	
	return f;
}

FILE *fdopen( INT fd, const CHAR *mode );
FILE *freopen( const CHAR *pathname, const CHAR *mode, FILE *stream );

INT
fclose(
	FILE *stream
)
{
	free( stream );
}

INT
fread(
	void *ptr,
	INT size,
	INT nmemb,
	FILE *stream
)
{
	INT bytes_to_read = size * nmemb;
	INT bytes_read = 0;

	FAT32* filesystem = FatResolveByPrefix( stream->path );
	if ( filesystem == NULL ) return 0;

	if ( stream->fptr + bytes_to_read > stream->file_size )
	{
		bytes_to_read = stream->file_size - stream->fptr;
	}
	//prINTf("Reading %d bytes.\n", bytes_to_read);
	while ( bytes_to_read > 0 )
	{
		DbgPrintFmt( "%s", stream->path );
		if ( stream->buffptr + bytes_to_read > filesystem->ClusterSize )
		{
			// Need to read at least 1 more cluster
			INT to_read_in_this_cluster = filesystem->ClusterSize - stream->buffptr;
			memcpy( ptr + bytes_read, stream->currbuf + stream->buffptr, to_read_in_this_cluster );
			bytes_read += to_read_in_this_cluster;
			//prINTf("buffptr = 0\n");
			stream->buffptr = 0;
			//prINTf("Getting next cluster.\n");
			stream->curr_cluster = FatGetNextClusterID( filesystem, stream->curr_cluster );
			//prINTf("Next cluster: %x\n", stream->curr_cluster);
			if ( stream->curr_cluster >= EOC )
			{
				//prINTf("Returning.\n");
				stream->fptr += bytes_read;
				return bytes_read;
			}
			//prINTf("getting next cluster.\n");
			FatGetCluster( filesystem, stream->currbuf, stream->curr_cluster );
			bytes_to_read -= to_read_in_this_cluster;
		}
		else
		{
			//prINTf("buffptr: %d\n", stream->buffptr);
			memcpy( ptr + bytes_read, stream->currbuf + stream->buffptr, bytes_to_read );
			bytes_read += bytes_to_read;
			stream->buffptr += bytes_to_read;
			bytes_to_read = 0;
		}
	}
	//prINTf("Returning.\n");
	stream->fptr += bytes_read;
	return bytes_read;
}

INT fwrite( const void *ptr, INT size, INT nmemb, FILE *stream );