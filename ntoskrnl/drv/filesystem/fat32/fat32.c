#include "fat32.h"
#include <mm/mm.h>
#include <drv/filesystem/storage.h>

BPBTable* FatDiskBPB;

UINT FatBeginSector;
UINT FatClusterBeginSector;

UCHAR* BootSector;

UINT* FAT;
INT iClusterSize;
UINT uiClusterAllocHint;
UINT uiFirstFatSector;

#define EOC 0x0FFFFFF8 //End of Chain

UCHAR
readi32(
	UCHAR *buff,
	INT offset
)
{
	UCHAR *ubuff = buff + offset;
	return
		( ( ubuff[ 3 ] << 24 ) & 0xFF000000 ) |
		( ( ubuff[ 2 ] << 16 ) & 0x00FF0000 ) |
		( ( ubuff[ 1 ] << 8 ) & 0x0000FF00 ) |
		( ubuff[ 0 ] & 0x000000FF );
}


USHORT
readi16(
	UCHAR *buff,
	INT offset
)
{
	UCHAR *ubuff = buff + offset;
	return ubuff[ 1 ] << 8 | ubuff[ 0 ];
}

VOID
trim_spaces(
	char *c,
	int max
)
{
	int i = 0;
	while ( *c != ' ' && i++ < max )
	{
		c++;
	}
	if ( *c == ' ' ) *c = 0;
}

VOID
FatInitialize(

)
{
	DbgPrintFmt( "Initializing Fat Partition" );
	

	


	//Alloc memory for data
	BootSector = ( UCHAR* )malloc( 512 );
	RtlZeroMemory( BootSector, 512 );
	//Get partition's sector
	UINT LBAPartition = FatAcquirePartitionSector( NULL );

	DbgPrintFmt( "Found FAT Partition at Sector: %d", LBAPartition );

	//Read partition BPB
	KiATAReadDiskEx( LBAPartition, 1, BootSector );
	
	FatDiskBPB = ( BPBTable* )BootSector; //Set the data from Harddisk to Struct

	FatBeginSector = LBAPartition + FatDiskBPB->ReservedSectorCount;
	FatClusterBeginSector = FatBeginSector + ( ( UINT )FatDiskBPB->NumberOfFAT * ( UINT )FatDiskBPB->ExtendedBootRecord.SectorsPerFat );

	iClusterSize = 512 * ( UINT )FatDiskBPB->SectorsPerCluster;
	UINT bPerFat = 512 * ( UINT )FatDiskBPB->ExtendedBootRecord.SectorsPerFat;
	FAT = malloc( bPerFat );


	UINT iSector;
	for ( iSector = 0; iSector < FatDiskBPB->ExtendedBootRecord.SectorsPerFat; iSector++ )
	{
		DbgPrintFmt( "iSector: %d\nFatDiskBPB->ExtendedBootRecord.SectorsPerFat: %d", iSector, FatDiskBPB->ExtendedBootRecord.SectorsPerFat );
		UCHAR sector[ 512 ];
		KiATAReadDiskEx( sector, FatBeginSector + iSector, 1 );
		UINT integer_j;
		for ( integer_j = 0; integer_j < 512 / 4; integer_j++ )
		{
			FAT[ iSector * ( 512 / 4 ) + integer_j ]
				= readi32( sector, integer_j * 4 );
		}
	}


	//null-terminate
	RtlSetMemory( FatDiskBPB->OEMIdentifier + 7, '\0', 1 );
	RtlSetMemory( FatDiskBPB->ExtendedBootRecord.VolumeLabel + 10, '\0', 1 );
	RtlSetMemory( FatDiskBPB->ExtendedBootRecord.SystemIdentifier + 7, '\0', 1 );
 

	DbgPrintFmt( "Partition Label: %s", FatDiskBPB->ExtendedBootRecord.VolumeLabel );
	DbgPrintFmt( "System Identifier: %s", FatDiskBPB->ExtendedBootRecord.SystemIdentifier );
	DbgPrintFmt( "Serial Number: %d", (UINT)FatDiskBPB->ExtendedBootRecord.VolumeIDSerial );


	uiFirstFatSector = FatDiskBPB->ReservedSectorCount;

	if ( RtlCompareMemory( FatDiskBPB->ExtendedBootRecord.SystemIdentifier, "FAT32", 5 ) != 0 )
		DbgPrintFmt( "Current Partition is NOT a FAT32 one!" );

}

VOID
FatFlushFAT( 

)
{
	KiATAWriteDisk( FatBeginSector, FatDiskBPB->ExtendedBootRecord.SectorsPerFat, ( UCHAR * )FAT );
}

UINT
FatGetClusterSector(
	UINT uiCluster
)
{
	return FatClusterBeginSector + ( ( uiCluster - 2 ) * FatDiskBPB->SectorsPerCluster );
}

VOID
FatGetCluster( 
	UCHAR *ucBuffer,
	UINT uiClusterNum
)
{ // static
	if ( uiClusterNum >= EOC )
	{
		DbgPrintFmt( "Can't get cluster. Hit End Of Chain." );
	}
	UINT uiSector = FatGetClusterSector( uiClusterNum );
	UINT uiSectorCount = FatDiskBPB->SectorsPerCluster;

	KiATAReadDiskEx( uiSector, uiSectorCount, ucBuffer );
}

VOID
FatPutCluster(
	UCHAR *ucBuffer,
	UINT uiClusterNum
)
{
	UINT uiSector = FatGetClusterSector( uiClusterNum );
	UINT uiSectorCount = FatDiskBPB->SectorsPerCluster;

	KiATAWriteDisk( uiSector, uiSectorCount, ucBuffer );
}

UINT
FatGetNextClusterID(
	UINT uiCluster
)
{ // static
	return FAT[ uiCluster ] & 0x0FFFFFFF;
}

CHAR*
FatParseLongName(
	UCHAR* ucEntries,
	UCHAR ucEntryCount
)
{
	// each entry can hold 13 characters.
	CHAR *name = malloc( ucEntryCount * 13 );
	int i, j;
	for ( i = 0; i < ucEntryCount; i++ )
	{
		UCHAR *entry = ucEntries + ( i * 32 );
		UCHAR entry_no = ( UCHAR )entry[ 0 ] & 0x0F;
		char *name_offset = name + ( ( entry_no - 1 ) * 13 );

		for ( j = 1; j < 10; j += 2 )
		{
			if ( entry[ j ] >= 32 && entry[ j ] <= 127 )
			{
				*name_offset = entry[ j ];
			}
			else
			{
				*name_offset = 0;
			}
			name_offset++;
		}
		for ( j = 14; j < 25; j += 2 )
		{
			if ( entry[ j ] >= 32 && entry[ j ] <= 127 )
			{
				*name_offset = entry[ j ];
			}
			else
			{
				*name_offset = 0;
			}
			name_offset++;
		}
		for ( j = 28; j < 31; j += 2 )
		{
			if ( entry[ j ] >= 32 && entry[ j ] <= 127 )
			{
				*name_offset = entry[ j ];
			}
			else
			{
				*name_offset = 0;
			}
			name_offset++;
		}
	}
	return name;
}

VOID
FatClearCluster( 
	UINT uiCluster
)
{
	UCHAR ucBuffer[ iClusterSize ];
	RtlZeroMemory( ucBuffer, iClusterSize );
	FatPutCluster( ucBuffer, uiCluster );
}

UINT
FatAllocateCluster(

)
{
	UINT i, ints_per_fat = ( 512 * FatDiskBPB->SectorsPerCluster ) / 4;
	for ( i = uiClusterAllocHint; i < ints_per_fat; i++ )
	{
		if ( FAT[ i ] == 0 )
		{
			FAT[ i ] = 0x0FFFFFFF;
			FatClearCluster( i );
			uiClusterAllocHint = i + 1;
			return i;
		}
	}
	for ( i = 0; i < uiClusterAllocHint; i++ )
	{
		if ( FAT[ i ] == 0 )
		{
			FAT[ i ] = 0x0FFFFFFF;
			FatClearCluster( i );
			uiClusterAllocHint = i + 1;
			return i;
		}
	}
	return 0;
}

UCHAR
FatCheckSumFName(
	CHAR* fname
)
{
	UINT i;
	UCHAR checksum = 0;
	for ( i = 0; i < 11; i++ )
	{
		UCHAR highbit = ( checksum & 0x1 ) << 7;
		checksum = ( ( checksum >> 1 ) & 0x7F ) | highbit;
		checksum = checksum + fname[ i ];
	}
	return checksum;
}

VOID
FatWrite83FileName(
	CHAR* fname,
	UCHAR* buffer
)
{
	memset( buffer, ' ', 11 );
	UINT namelen = strlen( fname );
	// find the extension
	int i;
	int dot_index = -1;
	for ( i = namelen - 1; i >= 0; i-- )
	{
		if ( fname[ i ] == '.' )
		{
			// Found it!
			dot_index = i;
			break;
		}
	}

	// Write the extension
	if ( dot_index >= 0 )
	{
		for ( i = 0; i < 3; i++ )
		{
			UINT c_index = dot_index + 1 + i;
			UCHAR c = c_index >= namelen ? ' ' : k_toupper( fname[ c_index ] );
			buffer[ 8 + i ] = c;
		}
	}
	else
	{
		for ( i = 0; i < 3; i++ )
		{
			buffer[ 8 + i ] = ' ';
		}
	}

	// Write the filename.
	UINT firstpart_len = namelen;
	if ( dot_index >= 0 )
	{
		firstpart_len = dot_index;
	}
	if ( firstpart_len > 8 )
	{
		// Write the weird tilde thing.
		for ( i = 0; i < 6; i++ )
		{
			buffer[ i ] = k_toupper( fname[ i ] );
		}
		buffer[ 6 ] = '~';
		buffer[ 7 ] = '1'; // probably need to enumerate like files and increment.
	}
	else
	{
		// Just write the file name.
		UINT j;
		for ( j = 0; j < firstpart_len; j++ )
		{
			buffer[ j ] = k_toupper( fname[ j ] );
		}
	}
}


UCHAR*
FatLocateEntries(
	UCHAR* cluster_buffer,
	struct directory *dir,
	UINT count,
	UINT *found_cluster
)
{
	UINT dirs_per_cluster = iClusterSize / 32;

	UINT i;
	LONG64 index = -1;
	UINT cluster = dir->cluster;
	while ( 1 )
	{
		FatGetCluster( cluster_buffer, cluster );

		UINT in_a_row = 0;
		for ( i = 0; i < dirs_per_cluster; i++ )
		{
			UCHAR *entry = cluster_buffer + ( i * 32 );
			UCHAR first_byte = entry[ 0 ];
			if ( first_byte == 0x00 || first_byte == 0xE5 )
			{
				in_a_row++;
			}
			else
			{
				in_a_row = 0;
			}

			if ( in_a_row == count )
			{
				index = i - ( in_a_row - 1 );
				break;
			}
		}
		if ( index >= 0 )
		{
			
			break;
		}

		UINT next_cluster = FAT[ cluster ];
		if ( next_cluster >= EOC )
		{
			next_cluster = FatAllocateCluster(  );
			if ( !next_cluster )
			{
				return 0;
			}
			FAT[ cluster ] = next_cluster;
		}
		cluster = next_cluster;
	}
	*found_cluster = cluster;
	return cluster_buffer + ( index * 32 );
}


VOID
FatWriteLongFileNameEntries(
	UCHAR *start,
	UINT num_entries,
	CHAR *fname
)
{
	// Create a short filename to use for the checksum.
	char shortfname[ 12 ];
	shortfname[ 11 ] = 0;
	FatWrite83FileName( fname, ( UCHAR * )shortfname );
	UCHAR checksum = FatCheckSumFName( shortfname );

	/* Write the long-filename entries */
	// tracks the number of characters we've written into
	// the long-filename entries.
	UINT writtenchars = 0;
	char *nameptr = fname;
	UINT namelen = strlen( fname );
	UCHAR *entry = NULL;
	UINT i;
	for ( i = 0; i < num_entries; i++ )
	{
		// reverse the entry order
		entry = start + ( ( num_entries - 1 - i ) * 32 );
		// Set the entry number
		entry[ 0 ] = i + 1;
		entry[ 13 ] = checksum;

		// Characters are 16 bytes in long-filename entries (j+=2)
		// And they only go in certain areas in the 32-byte
		// block. (why we have three loops)
		uint32_t j;
		for ( j = 1; j < 10; j += 2 )
		{
			if ( writtenchars < namelen )
			{
				entry[ j ] = *nameptr;
			}
			else
			{
				entry[ j ] = 0;
			}
			nameptr++;
			writtenchars++;
		}
		for ( j = 14; j < 25; j += 2 )
		{
			if ( writtenchars < namelen )
			{
				entry[ j ] = *nameptr;
			}
			else
			{
				entry[ j ] = 0;
			}
			nameptr++;
			writtenchars++;
		}
		for ( j = 28; j < 31; j += 2 )
		{
			if ( writtenchars < namelen )
			{
				entry[ j ] = *nameptr;
			}
			else
			{
				entry[ j ] = 0;
			}
			nameptr++;
			writtenchars++;
		}
		// Mark the attributes byte as LFN (Long File Name)
		entry[ 11 ] = LFN;
	}
	// Mark the last(first) entry with the end-of-long-filename bit
	entry[ 0 ] |= 0x40;
}

VOID
FatDestroyFAT32(

)
{
	DbgPrintFmt( "Destroying filesystem.\n" );
	FatFlushFAT( );
	free( FAT );
}

//void populate_root_dir( f32 *fs, struct directory *dir )
//{
//	populate_dir( fs, dir, 2 );
//}


UCHAR*
FatReadDirectoryEntry(
	UCHAR *start,
	UCHAR *end,
	struct dir_entry *dirent
)
{
	UCHAR first_byte = start[ 0 ];
	UCHAR *entry = start;
	if ( first_byte == 0x00 || first_byte == 0xE5 )
	{
		// NOT A VALID ENTRY!
		return NULL;
	}

	UINT LFNCount = 0;
	while ( entry[ 11 ] == LFN )
	{
		LFNCount++;
		entry += 32;
		if ( entry == end )
		{
			return NULL;
		}
	}
	if ( LFNCount > 0 )
	{
		dirent->name = FatParseLongName( start, LFNCount );
	}
	else
	{
		// There's no long file name.
		// Trim up the short filename.
		dirent->name = malloc( 13 );
		memcpy( dirent->name, entry, 11 );
		dirent->name[ 11 ] = 0;
		char extension[ 4 ];
		memcpy( extension, dirent->name + 8, 3 );
		extension[ 3 ] = 0;
		trim_spaces( extension, 3 );

		dirent->name[ 8 ] = 0;
		trim_spaces( dirent->name, 8 );

		if ( strlen( extension ) > 0 )
		{
			UINT len = strlen( dirent->name );
			dirent->name[ len++ ] = '.';
			memcpy( dirent->name + len, extension, 4 );
		}
	}

	dirent->dir_attrs = entry[ 11 ];;
	USHORT first_cluster_high = readi16( entry, 20 );
	USHORT first_cluster_low = readi16( entry, 26 );
	dirent->first_cluster = first_cluster_high << 16 | first_cluster_low;
	dirent->file_size = readi32( entry, 28 );
	return entry + 32;
}



VOID
FatNextDirectoryEntry(
	UCHAR *root_cluster,
	UCHAR *entry,
	UCHAR **nextentry,
	struct dir_entry *target_dirent,
	UINT cluster,
	UINT *secondcluster
)
{

	UCHAR *end_of_cluster = root_cluster + iClusterSize;
	*nextentry = FatReadDirectoryEntry( entry, end_of_cluster, target_dirent );
	if ( !*nextentry )
	{
		// Something went wrong!
		// Either the directory entry spans the bounds of a cluster,
		// or the directory entry is invalid.
		// Load the next cluster and retry.

		// Figure out how much of the last cluster to "replay"
		UINT bytes_from_prev_chunk = end_of_cluster - entry;

		*secondcluster = FatGetNextClusterID( cluster );
		if ( *secondcluster >= EOC )
		{
			// There's not another directory cluster to load
			// and the previous entry was invalid!
			// It's possible the filesystem is corrupt or... you know...
			// my software could have bugs.
			DbgPrintFmt( "FOUND BAD DIRECTORY ENTRY!" );
		}
		// Load the cluster after the previous saved entries.
		FatGetCluster( root_cluster + iClusterSize, *secondcluster );
		// Set entry to its new location at the beginning of root_cluster.
		entry = root_cluster + iClusterSize - bytes_from_prev_chunk;

		// Retry reading the entry.
		*nextentry = FatReadDirectoryEntry( entry, end_of_cluster + iClusterSize, target_dirent );
		if ( !*nextentry )
		{
			// Still can't parse the directory entry.
			// Something is very wrong.
			DbgPrintFmt( "FAILED TO READ DIRECTORY ENTRY! THE SOFTWARE IS BUGGY!\n" );
		}
	}
}


VOID
FatPopulateDirectory(
	struct directory *dir,
	UINT cluster
)
{
	dir->cluster = cluster;
	UINT dirs_per_cluster = iClusterSize / 32;
	UINT max_dirs = 0;
	dir->entries = 0;
	UINT entry_count = 0;

	while ( 1 )
	{
		max_dirs += dirs_per_cluster;
		dir->entries = krealloc( dir->entries, max_dirs * sizeof( struct dir_entry ) );
		// Double the size in case we need to read a directory entry that
		// spans the bounds of a cluster.
		UCHAR root_cluster[ iClusterSize * 2 ];
		FatGetCluster( root_cluster, cluster );

		UCHAR *entry = root_cluster;
		while ( ( UINT )( entry - root_cluster ) < iClusterSize )
		{
			UCHAR first_byte = *entry;
			if ( first_byte == 0x00 || first_byte == 0xE5 )
			{
				// This directory entry has never been written
				// or it has been deleted.
				entry += 32;
				continue;
			}

			UINT secondcluster = 0;
			UCHAR *nextentry = NULL;
			struct dir_entry *target_dirent = dir->entries + entry_count;
			FatNextDirectoryEntry(  root_cluster, entry, &nextentry, target_dirent, cluster, &secondcluster );
			entry = nextentry;
			if ( secondcluster )
			{
				cluster = secondcluster;
			}

			entry_count++;
		}
		cluster = FatGetNextClusterID( cluster );
		if ( cluster >= EOC ) break;
	}
	dir->num_entries = entry_count;
}

VOID
FatZeroFatChain(
	UINT start_cluster
)
{
	UINT cluster = start_cluster;
	while ( cluster < EOC && cluster != 0 )
	{
		UINT next_cluster = FAT[ cluster ];
		FAT[ cluster ] = 0;
		cluster = next_cluster;
	}
	FatFlushFAT( );
}


VOID 
FatDeleteFile( 
	struct directory *dir,
	CHAR *filename
)
{ //struct dir_entry *dirent) {
	UINT cluster = dir->cluster;

	// Double the size in case we need to read a directory entry that
	// spans the bounds of a cluster.
	UCHAR root_cluster[ iClusterSize * 2 ];
	struct dir_entry target_dirent;

	// Try to locate and invalidate the directory entries corresponding to the
	// filename in dirent.
	while ( 1 )
	{
		FatGetCluster( root_cluster, cluster );

		UCHAR *entry = root_cluster;
		while ( ( UINT )( entry - root_cluster ) < iClusterSize )
		{
			UCHAR first_byte = *entry;
			if ( first_byte == 0x00 || first_byte == 0xE5 )
			{
				// This directory entry has never been written
				// or it has been deleted.
				entry += 32;
				continue;
			}

			UINT secondcluster = 0;
			UCHAR *nextentry = NULL;
			FatNextDirectoryEntry( root_cluster, entry, &nextentry, &target_dirent, cluster, &secondcluster );

			// We have a target dirent! see if it's the one we want!
			if ( strcmp( target_dirent.name, filename ) == 0 )
			{
				// We found it! Invalidate all the entries.
				memset( entry, 0, nextentry - entry );
				FatPutCluster( root_cluster, cluster );
				if ( secondcluster )
				{
					FatPutCluster( root_cluster +  iClusterSize, secondcluster );
				}
				FatZeroFatChain( target_dirent.first_cluster );
				free( target_dirent.name );
				return;
			}
			else
			{
				// We didn't find it. Continue.
				entry = nextentry;
				if ( secondcluster )
				{
					cluster = secondcluster;
				}
			}
			free( target_dirent.name );

		}
		cluster = FatGetNextClusterID( cluster );
		if ( cluster >= EOC ) return;
	}
}

VOID
FatFreeDirectory( 
	struct directory *dir
)
{
	UINT i;
	for ( i = 0; i < dir->num_entries; i++ )
	{
		free( dir->entries[ i ].name );
	}
	free( dir->entries );
}

UCHAR*
FatReadFile( 
	struct dir_entry *dirent
)
{
	uint8_t *file = malloc( dirent->file_size );
	uint8_t *filecurrptr = file;
	uint32_t cluster = dirent->first_cluster;
	uint32_t copiedbytes = 0;
	while ( 1 )
	{
		uint8_t cbytes[ iClusterSize ];
		FatGetCluster( cbytes, cluster );

		uint32_t remaining = dirent->file_size - copiedbytes;
		uint32_t to_copy = remaining > iClusterSize ? iClusterSize : remaining;

		memcpy( filecurrptr, cbytes, to_copy );

		filecurrptr += iClusterSize;
		copiedbytes += to_copy;

		cluster = FatGetNextClusterID( cluster );
		if ( cluster >= EOC ) break;
	}
	return file;
}


VOID
FatWriteFileImplementation(
	struct directory *dir,
	UCHAR *file,
	CHAR *fname,
	UINT flen,
	UCHAR attrs,
	UINT setcluster
)
{
	UINT required_clusters = flen / iClusterSize;
	if ( flen % iClusterSize != 0 ) required_clusters++;
	if ( required_clusters == 0 ) required_clusters++; // Allocate at least one cluster.
	// One for the traditional 8.3 name, one for each 13 charaters in the extended name.
	// Int division truncates, so if there's a remainder from length / 13, add another entry.
	UINT required_entries_long_fname = ( strlen( fname ) / 13 );
	if ( strlen( fname ) % 13 > 0 )
	{
		required_entries_long_fname++;
	}

	UINT required_entries_total = required_entries_long_fname + 1;

	UINT cluster; // The cluster number that the entries are found in
	UCHAR root_cluster[ iClusterSize ];
	UCHAR *start_entries = FatLocateEntries( root_cluster, dir, required_entries_total, &cluster );
	FatWriteLongFileNameEntries( start_entries, required_entries_long_fname, fname );

	// Write the actual file entry;
	UCHAR *actual_entry = start_entries + ( required_entries_long_fname * 32 );
	FatWrite83FileName( fname, actual_entry );

	// Actually write the file!
	UINT writtenbytes = 0;
	UINT prevcluster = 0;
	UINT firstcluster = 0;
	UINT i;
	if ( setcluster )
	{
		// Caller knows where the first cluster is.
		// Don't allocate or write anything.
		firstcluster = setcluster;
	}
	else
	{
		for ( i = 0; i < required_clusters; i++ )
		{
			UINT currcluster = FatAllocateCluster( );
			if ( !firstcluster )
			{
				firstcluster = currcluster;
			}
			UCHAR cluster_buffer[ iClusterSize ];
			memset( cluster_buffer, 0, iClusterSize );
			uint32_t bytes_to_write = flen - writtenbytes;
			if ( bytes_to_write > iClusterSize )
			{
				bytes_to_write = iClusterSize;
			}
			memcpy( cluster_buffer, file + writtenbytes, bytes_to_write );
			writtenbytes += bytes_to_write;
			FatPutCluster( cluster_buffer, currcluster );
			if ( prevcluster )
			{
				FAT[ prevcluster ] = currcluster;
			}
			prevcluster = currcluster;
		}
	}

	// Write the other fields of the actual entry
	// We do it down here because we need the first cluster
	// number.

	// attrs
	actual_entry[ 11 ] = attrs;

	// high cluster bits
	actual_entry[ 20 ] = ( firstcluster >> 16 ) & 0xFF;
	actual_entry[ 21 ] = ( firstcluster >> 24 ) & 0xFF;

	// low cluster bits
	actual_entry[ 26 ] = ( firstcluster ) & 0xFF;
	actual_entry[ 27 ] = ( firstcluster >> 8 ) & 0xFF;

	// file size
	actual_entry[ 28 ] = flen & 0xFF;
	actual_entry[ 29 ] = ( flen >> 8 ) & 0xFF;
	actual_entry[ 30 ] = ( flen >> 16 ) & 0xFF;
	actual_entry[ 31 ] = ( flen >> 24 ) & 0xFF;

	// Write the cluster back to disk
	FatPutCluster( root_cluster, cluster );
	FatFlushFAT( );
}

VOID
FatWriteFile( 
	struct directory *dir,
	UCHAR *file,
	CHAR *fname,
	UINT flen
)
{
	FatWriteFileImplementation( dir, file, fname, flen, 0, 0 );
}

VOID
FatMakeDirSubDirs(
	struct directory *dir,
	UCHAR parentcluster
)
{
	FatWriteFileImplementation( dir, NULL, ".", 0, DIRECTORY, dir->cluster );
	FatWriteFileImplementation( dir, NULL, "..", 0, DIRECTORY, parentcluster );
}


VOID
FatMakeDir( 
	struct directory *dir,
	CHAR *dirname
)
{
	FatWriteFileImplementation( dir, NULL, dirname, 0, DIRECTORY, 0 );

	// We need to add the subdirectories '.' and '..'
	struct directory subdir;
	FatPopulateDirectory( &subdir, dir->cluster );
	UINT i;
	for ( i = 0; i < subdir.num_entries; i++ )
	{
		if ( strcmp( subdir.entries[ i ].name, dirname ) == 0 )
		{
			struct directory newsubdir;
			FatPopulateDirectory( &newsubdir, subdir.entries[ i ].first_cluster );
			FatMakeDirSubDirs( &newsubdir, subdir.cluster );
			FatFreeDirectory( &newsubdir );
		}
	}
	FatFreeDirectory( &subdir );
}

VOID
FatPrintDirectory(
	struct directory *dir
)
{
	UINT i;
	UINT max_name = 0;
	for ( i = 0; i < dir->num_entries; i++ )
	{
		UINT namelen = strlen( dir->entries[ i ].name );
		max_name = namelen > max_name ? namelen : max_name;
	}

	char *namebuff = malloc( max_name + 1 );
	for ( i = 0; i < dir->num_entries; i++ )
	{
		//        printf("[%d] %*s %c %8d bytes ",
		//               i,
		//               -max_name,
		//               dir->entries[i].name,
		//               dir->entries[i].dir_attrs & DIRECTORY?'D':' ',
		//               dir->entries[i].file_size, dir->entries[i].first_cluster);
		DbgPrintFmt( "[%d] ", i );


		UINT j;
		for ( j = 0; j < max_name; j++ )
		{
			namebuff[ j ] = ' ';
		}
		namebuff[ max_name ] = 0;
		for ( j = 0; j < strlen( dir->entries[ i ].name ); j++ )
		{
			namebuff[ j ] = dir->entries[ i ].name[ j ];
		}

		DbgPrintFmt( "%s %c %d ",
			   namebuff,
			   dir->entries[ i ].dir_attrs & DIRECTORY ? 'D' : ' ',
			   dir->entries[ i ].file_size );

		UINT cluster = dir->entries[ i ].first_cluster;
		UINT cluster_count = 1;
		while ( 1 )
		{
			cluster = FAT[ cluster ];
			if ( cluster >= EOC ) break;
			if ( cluster == 0 )
			{
				DbgPrintFmt( "BAD CLUSTER CHAIN! FS IS CORRUPT!" );
			}
			cluster_count++;
		}
		DbgPrintFmt( "clusters: [%d]\n", cluster_count );
	}
	free( namebuff );
}

UINT
FatCountFreeClusters(

)
{
	UINT clusters_in_fat = ( (UINT)FatDiskBPB->ExtendedBootRecord.SectorsPerFat * 512 ) / 4;
	UINT i;
	UINT count = 0;
	for ( i = 0; i < clusters_in_fat; i++ )
	{
		if ( FAT[ i ] == 0 )
		{
			count++;
		}
	}
	return count;
}