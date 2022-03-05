#include "fat32.h"
FAT32 *Partition[ 4 ];

STATIC VOID
FatReadSectorBiosParameterBlock(
	FAT32 *fs,
	struct BiosParameterBlockStruct *BiosParameterBlock
);

STATIC UINT
FatAcquireSectorForCluster(
	FAT32 *fs,
	UINT cluster
);


STATIC VOID
FatGetSector(
	FAT32 *fs,
	UCHAR *buff,
	UINT sector,
	UINT count
)
{
	KiATAReadDiskEx( sector, count, buff );
}

STATIC VOID
FatPutSector(
	FAT32 *fs,
	UCHAR *buff,
	UINT sector,
	UINT count
)
{
	UINT i;
	for ( i = 0; i < count; i++ )
	{
		KiATAWriteDisk( sector + i, 1, buff + ( i * 512 ) );
	}
}

STATIC VOID
FatFlushFAT(
	FAT32 *fs
)
{
	// TODO: This is not endian-safe. Must marshal the integers into a byte buffer.
	FatPutSector( fs, ( UCHAR * )fs->FAT, fs->FatSector, fs->BiosParameterBlock.SectorsPerFat32 );
}

STATIC VOID
UtilTrimSpaces(
	CHAR *c,
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

STATIC USHORT
UtilReadInteger16(
	UCHAR *buff,
	INT offset
)
{
	UCHAR *ubuff = buff + offset;
	return ubuff[ 1 ] << 8 | ubuff[ 0 ];
}

STATIC UINT
UtilReadInteger32(
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

INT iLastDriveCount = 0;

STATIC VOID
FatReadSectorBiosParameterBlock(
	FAT32 *fs,
	struct BiosParameterBlockStruct *BiosParameterBlock
)
{
	UCHAR* BootSector = ( UCHAR* )malloc( 512 );
	FatGetSector( fs, BootSector, FatAcquirePartitionSector( NULL ), 1 );

	BiosParameterBlock->BytesPerSector = UtilReadInteger16( BootSector, 11 );;
	BiosParameterBlock->SectorsPerCluster = BootSector[ 13 ];
	BiosParameterBlock->ReservedSectors = UtilReadInteger16( BootSector, 14 );
	BiosParameterBlock->FatCount = BootSector[ 16 ];
	BiosParameterBlock->DirectoryEntries = UtilReadInteger16( BootSector, 17 );
	BiosParameterBlock->TotalSectors = UtilReadInteger16( BootSector, 19 );
	BiosParameterBlock->MediaDescriptorType = BootSector[ 21 ];
	BiosParameterBlock->SectorsPerFat = UtilReadInteger16( BootSector, 22 );
	BiosParameterBlock->SectorsPerTrack = UtilReadInteger16( BootSector, 24 );
	BiosParameterBlock->HeadsOrSizesOnMedia = UtilReadInteger16( BootSector, 26 );
	BiosParameterBlock->HiddenSectors = UtilReadInteger32( BootSector, 28 );
	BiosParameterBlock->LargeSectorsOnMedia = UtilReadInteger32( BootSector, 32 );
	// EBR
	BiosParameterBlock->SectorsPerFat32 = UtilReadInteger32( BootSector, 36 );
	BiosParameterBlock->flags = UtilReadInteger16( BootSector, 40 );
	BiosParameterBlock->FatVersion = UtilReadInteger16( BootSector, 42 );
	BiosParameterBlock->ClusterRootDirectory = UtilReadInteger32( BootSector, 44 );
	BiosParameterBlock->FSInfoSectorNumber = UtilReadInteger16( BootSector, 48 );
	BiosParameterBlock->BackupBootSectorNumber = UtilReadInteger16( BootSector, 50 );
	// Skip 12 bytes
	BiosParameterBlock->DriveNumber = BootSector[ 64 ];
	BiosParameterBlock->WindowsFlags = BootSector[ 65 ];
	BiosParameterBlock->Signature = BootSector[ 66 ];
	BiosParameterBlock->VolumeSerial = UtilReadInteger32( BootSector, 67 );

	//Null-Terminate Strings
	RtlCopyMemory( &BiosParameterBlock->VolumeLabel, BootSector + 71, 11 ); BiosParameterBlock->VolumeLabel[ 11 ] = 0;
	RtlCopyMemory( &BiosParameterBlock->SystemID, BootSector + 82, 8 ); BiosParameterBlock->SystemID[ 8 ] = 0;

	free( BootSector );
}

STATIC UINT
FatAcquireSectorForCluster(
	FAT32 *fs,
	UINT cluster
)
{
	return fs->ClusterSector + ( ( cluster - 2 ) * fs->BiosParameterBlock.SectorsPerCluster );
}

// CLUSTER NUMBERS START AT 2 (for some reason...)
VOID
FatGetCluster(
	FAT32 *fs,
	UCHAR *buff,
	UINT cluster_number
)
{ // STATIC
	if ( cluster_number >= EOC )
	{
		DbgPrintFmt( "Can't get cluster. Hit End Of Chain." );
	}
	UINT sector = FatAcquireSectorForCluster( fs, cluster_number );
	UINT sector_count = fs->BiosParameterBlock.SectorsPerCluster;
	FatGetSector( fs, buff, sector, sector_count );
}

STATIC VOID
FatPutCluster(
	FAT32 *fs,
	UCHAR *buff,
	UINT cluster_number
)
{
	UINT sector = FatAcquireSectorForCluster( fs, cluster_number );
	UINT sector_count = fs->BiosParameterBlock.SectorsPerCluster;
	FatPutSector( fs, buff, sector, sector_count );
}

UINT
FatGetNextClusterID(
	FAT32 *fs,
	UINT cluster
)
{ // STATIC
	return fs->FAT[ cluster ] & 0x0FFFFFFF;
}

STATIC CHAR* 
FatParseLongName(
	UCHAR *entries,
	UCHAR entry_count
)
{
	// each entry can hold 13 characters.
	CHAR *name = malloc( entry_count * 13 );
	int i, j;
	for ( i = 0; i < entry_count; i++ )
	{
		UCHAR *entry = entries + ( i * 32 );
		UCHAR entry_no = ( UCHAR )entry[ 0 ] & 0x0F;
		CHAR *name_offset = name + ( ( entry_no - 1 ) * 13 );

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

STATIC VOID
FatClearCluster(
	FAT32 *fs,
	UINT cluster
)
{
	UCHAR buffer[ fs->ClusterSize ];
	RtlSetMemory( buffer, 0, fs->ClusterSize );
	FatPutCluster( fs, buffer, cluster );
}

STATIC UINT
FatAllocCluster(
	FAT32 *fs
)
{
	UINT i, ints_per_fat = ( 512 * fs->BiosParameterBlock.SectorsPerFat32 ) / 4;
	for ( i = fs->ClusterAllocationHint; i < ints_per_fat; i++ )
	{
		if ( fs->FAT[ i ] == 0 )
		{
			fs->FAT[ i ] = 0x0FFFFFFF;
			FatClearCluster( fs, i );
			fs->ClusterAllocationHint = i + 1;
			return i;
		}
	}
	for ( i = 0; i < fs->ClusterAllocationHint; i++ )
	{
		if ( fs->FAT[ i ] == 0 )
		{
			fs->FAT[ i ] = 0x0FFFFFFF;
			FatClearCluster( fs, i );
			fs->ClusterAllocationHint = i + 1;
			return i;
		}
	}
	return 0;
}

// Creates a checksum for an 8.3 filename
// must be in Directory-entry format, i.e.
// fat32.c -> "FAT32   C  "
STATIC UCHAR
FatCheckSumFNAME(
	CHAR *fname
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

STATIC VOID
FatWrite83FileName(
	CHAR *fname,
	UCHAR *buffer
)
{
	RtlSetMemory( buffer, ' ', 11 );
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

STATIC UCHAR*
FatLocateEntries(
	FAT32 *fs,
	UCHAR *cluster_buffer,
	struct Directory *dir,
	UINT count,
	UINT *found_cluster
)
{
	UINT dirs_per_cluster = fs->ClusterSize / 32;

	UINT i;
	LONG64 index = -1;
	UINT cluster = dir->cluster;
	while ( 1 )
	{
		FatGetCluster( fs, cluster_buffer, cluster );

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
			// We found a spot to put our crap!
			break;
		}

		UINT next_cluster = fs->FAT[ cluster ];
		if ( next_cluster >= EOC )
		{
			next_cluster = FatAllocCluster( fs );
			if ( !next_cluster )
			{
				return 0;
			}
			fs->FAT[ cluster ] = next_cluster;
		}
		cluster = next_cluster;
	}
	*found_cluster = cluster;
	return cluster_buffer + ( index * 32 );
}

STATIC VOID
FatWriteLongFileNameEntries(
	UCHAR *start,
	UINT num_entries,
	CHAR *fname
)
{
	// Create a short filename to use for the checksum.
	CHAR shortfname[ 12 ];
	shortfname[ 11 ] = 0;
	FatWrite83FileName( fname, ( UCHAR * )shortfname );
	UCHAR checksum = FatCheckSumFNAME( shortfname );

	/* Write the long-filename entries */
	// tracks the number of characters we've written into
	// the long-filename entries.
	UINT writtenchars = 0;
	CHAR *nameptr = fname;
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
		UINT j;
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


FAT32*
FatResolveByPrefix(
	CHAR* pcPath
)
{
	for ( int i = 0; i < MAX_PARTITION; i++ )
	{
		if ( Partition[ i ] != NULL ) //check if partition exists
			if ( memcmp( pcPath, Partition[ i ]->RootPath, 1 ) == 0 )// check for C:
				return Partition[ i ];
	}
	return NULL;
}

FAT32*
FatInitialize(
	INT iPartitionNumber
)
{
	INT PartitionSector = FatAcquirePartitionSector( iPartitionNumber );
	if ( PartitionSector == -1 ) // not a partition :D
	{
		return NULL;
	}

	FAT32 *fs = malloc( sizeof( struct FAT32 ) );
	/*if ( !KiATAIdentify( ) )
	{
		return NULL;
	}*/
	FatReadSectorBiosParameterBlock( fs, &fs->BiosParameterBlock );

	UtilTrimSpaces( fs->BiosParameterBlock.SystemID, 8 );
	if ( strcmp( fs->BiosParameterBlock.SystemID, "FAT32" ) != 0 )
	{
		free( fs );
		return NULL;
	}


	

	DbgPrintFmt( "Sectors per cluster: %d\n", fs->BiosParameterBlock.SectorsPerCluster );


	
	

	

	if ( iLastDriveCount == 4 )
	{
		DbgPrintFmt( "Cannot mount more than 4 partitions!" );
		return NULL;
	}

	fs->LetterAssigned = LetterAssignment[ iLastDriveCount ];


	fs->RootPath[ 0 ] = LetterAssignment[ iLastDriveCount ];
	fs->RootPath[ 1 ] = ':';
	iLastDriveCount++;

	DbgPrintFmt( "Mounted partition: %s", fs->RootPath );
	fs->PartitionSector = PartitionSector;
	fs->FatSector = fs->PartitionSector + fs->BiosParameterBlock.ReservedSectors;
	fs->ClusterSector = fs->FatSector + ( fs->BiosParameterBlock.FatCount * fs->BiosParameterBlock.SectorsPerFat32 );
	fs->ClusterSize = 512 * fs->BiosParameterBlock.SectorsPerCluster;
	fs->ClusterAllocationHint = 0;

	// Load the FAT
	UINT bytes_per_fat = 512 * fs->BiosParameterBlock.SectorsPerFat32;
	fs->FAT = malloc( bytes_per_fat );
	UINT sector_i;
	for ( sector_i = 0; sector_i < fs->BiosParameterBlock.SectorsPerFat32; sector_i++ )
	{
		UCHAR sector[ 512 ];
		FatGetSector( fs, sector, fs->FatSector + sector_i, 1 );
		UINT integer_j;
		for ( integer_j = 0; integer_j < 512 / 4; integer_j++ )
		{
			fs->FAT[ sector_i * ( 512 / 4 ) + integer_j ]
				= UtilReadInteger32( sector, integer_j * 4 );
		}
	}
	free( fs );
	return fs;
}

VOID
FatDestroy(
	FAT32 *fs
)
{
	DbgPrintFmt( "Destroying filesystem.\n" );
	FatFlushFAT( fs );
	free( fs->FAT );
	free( fs );
}

CONST struct BiosParameterBlockStruct*
FatGetBiosParameterBlock(
	FAT32 *fs
)
{
	return &fs->BiosParameterBlock;
}

VOID
FatPopulateRootDir(
	FAT32 *fs,
	struct Directory *dir
)
{
	FatPopulateDir( fs, dir, 2 );
}

// Populates dirent with the Directory entry starting at start
// Returns a pointer to the next 32-byte chunk after the entry
// or NULL if either start does not point to a valid entry, or
// there are not enough entries to build a struct DirectoryEntry
STATIC UCHAR*
FatReadDirectoryEntry(
	FAT32 *fs,
	UCHAR *start,
	UCHAR *end,
	struct DirectoryEntry *dirent
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
		RtlCopyMemory( dirent->name, entry, 11 );
		dirent->name[ 11 ] = 0;
		CHAR extension[ 4 ];
		RtlCopyMemory( extension, dirent->name + 8, 3 );
		extension[ 3 ] = 0;
		UtilTrimSpaces( extension, 3 );

		dirent->name[ 8 ] = 0;
		UtilTrimSpaces( dirent->name, 8 );

		if ( strlen( extension ) > 0 )
		{
			UINT len = strlen( dirent->name );
			dirent->name[ len++ ] = '.';
			RtlCopyMemory( dirent->name + len, extension, 4 );
		}
	}

	dirent->dir_attrs = entry[ 11 ];;
	USHORT first_cluster_high = UtilReadInteger16( entry, 20 );
	USHORT first_cluster_low = UtilReadInteger16( entry, 26 );
	dirent->first_cluster = first_cluster_high << 16 | first_cluster_low;
	dirent->file_size = UtilReadInteger32( entry, 28 );
	return entry + 32;
}

// This is a complicated one. It parses a Directory entry into the DirectoryEntry pointed to by target_dirent.
// root_cluster  must point to a buffer big enough for two clusters.
// entry         points to the entry the caller wants to parse, and must point to a spot within root_cluster.
// nextentry     will be modified to hold the next spot within root_entry to begin looking for entries.
// cluster       is the cluster number of the cluster loaded into root_cluster.
// secondcluster will be modified IF this function needs to load another cluster to continue parsing
//               the entry, in which case, it will be set to the value of the cluster loaded.
//
VOID
FatNextDirectoryEntry(
	FAT32 *fs,
	UCHAR *root_cluster,
	UCHAR *entry,
	UCHAR **nextentry,
	struct DirectoryEntry *target_dirent,
	UINT cluster,
	UINT *secondcluster
)
{

	UCHAR *end_of_cluster = root_cluster + fs->ClusterSize;
	*nextentry = FatReadDirectoryEntry( fs, entry, end_of_cluster, target_dirent );
	if ( !*nextentry )
	{
		// Something went wrong!
		// Either the Directory entry spans the bounds of a cluster,
		// or the Directory entry is invalid.
		// Load the next cluster and retry.

		// Figure out how much of the last cluster to "replay"
		UINT bytes_from_prev_chunk = end_of_cluster - entry;

		*secondcluster = FatGetNextClusterID( fs, cluster );
		if ( *secondcluster >= EOC )
		{
			// There's not another Directory cluster to load
			// and the previous entry was invalid!
			// It's possible the filesystem is corrupt or... you know...
			// my software could have bugs.
			DbgPrintFmt( "FOUND BAD DIRECTORY ENTRY!" );
		}
		// Load the cluster after the previous saved entries.
		FatGetCluster( fs, root_cluster + fs->ClusterSize, *secondcluster );
		// Set entry to its new location at the beginning of root_cluster.
		entry = root_cluster + fs->ClusterSize - bytes_from_prev_chunk;

		// Retry reading the entry.
		*nextentry = FatReadDirectoryEntry( fs, entry, end_of_cluster + fs->ClusterSize, target_dirent );
		if ( !*nextentry )
		{
			// Still can't parse the Directory entry.
			// Something is very wrong.
			DbgPrintFmt( "FAILED TO READ DIRECTORY ENTRY! THE SOFTWARE IS BUGGY!\n" );
		}
	}
}

// TODO: Refactor this. It is so similar to FatDeleteFile that it would be nice
// to combine the similar elements.
// WARN: If you fix a bug in this function, it's likely you will find the same
// bug in FatDeleteFile.
VOID
FatPopulateDir(
	FAT32 *fs,
	struct Directory *dir,
	UINT cluster
)
{
	dir->cluster = cluster;
	UINT dirs_per_cluster = fs->ClusterSize / 32;
	UINT max_dirs = 0;
	dir->entries = 0;
	UINT entry_count = 0;

	while ( 1 )
	{
		max_dirs += dirs_per_cluster;
		dir->entries = krealloc( dir->entries, max_dirs * sizeof( struct DirectoryEntry ) );
		DbgPrintFmt( "WTF1" );
		// Double the size in case we need to read a Directory entry that
		// spans the bounds of a cluster.
		UCHAR root_cluster[ fs->ClusterSize * 2 ];
		FatGetCluster( fs, root_cluster, cluster );
		DbgPrintFmt( "WTF2" );
		UCHAR *entry = root_cluster;
		while ( ( UINT )( entry - root_cluster ) < fs->ClusterSize )
		{
			UCHAR first_byte = *entry;
			if ( first_byte == 0x00 || first_byte == 0xE5 )
			{
				// This Directory entry has never been written
				// or it has been deleted.
				entry += 32;
				continue;
			}

			UINT secondcluster = 0;
			UCHAR *nextentry = NULL;
			struct DirectoryEntry *target_dirent = dir->entries + entry_count;
			DbgPrintFmt( "%lx, %lx, %lx, %lx, %lx, %d, %d", fs, root_cluster, entry, nextentry, target_dirent
			, cluster, secondcluster );
			FatNextDirectoryEntry( fs, root_cluster, entry, &nextentry, target_dirent, cluster, &secondcluster );
			DbgPrintFmt( "WTF3" );
			entry = nextentry;
			if ( secondcluster )
			{
				cluster = secondcluster;
			}

			entry_count++;
		}
		cluster = FatGetNextClusterID( fs, cluster );
		DbgPrintFmt( "WTF4" );
		if ( cluster >= EOC ) break;
	}
	dir->num_entries = entry_count;
}

STATIC VOID
FatZeroFatChain(
	FAT32 *fs,
	UINT start_cluster
)
{
	UINT cluster = start_cluster;
	while ( cluster < EOC && cluster != 0 )
	{
		UINT next_cluster = fs->FAT[ cluster ];
		fs->FAT[ cluster ] = 0;
		cluster = next_cluster;
	}
	FatFlushFAT( fs );
}

// TODO: Refactor this. It is so similar to FatPopulateDir that it would be nice
// to combine the similar elements.
// WARN: If you fix a bug in this function, it's likely you will find the same
// bug in FatPopulateDir.
VOID
FatDeleteFile(
	FAT32 *fs,
	struct Directory *dir,
	CHAR *filename
)
{ //struct DirectoryEntry *dirent) {
	UINT cluster = dir->cluster;

	// Double the size in case we need to read a Directory entry that
	// spans the bounds of a cluster.
	UCHAR root_cluster[ fs->ClusterSize * 2 ];
	struct DirectoryEntry target_dirent;

	// Try to locate and invalidate the Directory entries corresponding to the
	// filename in dirent.
	while ( 1 )
	{
		FatGetCluster( fs, root_cluster, cluster );

		UCHAR *entry = root_cluster;
		while ( ( UINT )( entry - root_cluster ) < fs->ClusterSize )
		{
			UCHAR first_byte = *entry;
			if ( first_byte == 0x00 || first_byte == 0xE5 )
			{
				// This Directory entry has never been written
				// or it has been deleted.
				entry += 32;
				continue;
			}

			UINT secondcluster = 0;
			UCHAR *nextentry = NULL;
			FatNextDirectoryEntry( fs, root_cluster, entry, &nextentry, &target_dirent, cluster, &secondcluster );

			// We have a target dirent! see if it's the one we want!
			if ( strcmp( target_dirent.name, filename ) == 0 )
			{
				// We found it! Invalidate all the entries.
				RtlSetMemory( entry, 0, nextentry - entry );
				FatPutCluster( fs, root_cluster, cluster );
				if ( secondcluster )
				{
					FatPutCluster( fs, root_cluster + fs->ClusterSize, secondcluster );
				}
				FatZeroFatChain( fs, target_dirent.first_cluster );
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
		cluster = FatGetNextClusterID( fs, cluster );
		if ( cluster >= EOC ) return;
	}
}

VOID
FatFreeDirectory(
	FAT32 *fs,
	struct Directory *dir
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
	FAT32 *fs,
	struct DirectoryEntry *dirent
)
{
	UCHAR *file = malloc( dirent->file_size );
	UCHAR *filecurrptr = file;
	UINT cluster = dirent->first_cluster;
	UINT copiedbytes = 0;
	while ( 1 )
	{
		UCHAR cbytes[ fs->ClusterSize ];
		FatGetCluster( fs, cbytes, cluster );

		UINT remaining = dirent->file_size - copiedbytes;
		UINT to_copy = remaining > fs->ClusterSize ? fs->ClusterSize : remaining;

		RtlCopyMemory( filecurrptr, cbytes, to_copy );

		filecurrptr += fs->ClusterSize;
		copiedbytes += to_copy;

		cluster = FatGetNextClusterID( fs, cluster );
		if ( cluster >= EOC ) break;
	}
	return file;
}

STATIC VOID
FatWriteFileImplementation(
	FAT32 *fs,
	struct Directory *dir,
	UCHAR *file,
	CHAR *fname,
	UINT flen,
	UCHAR attrs,
	UINT setcluster
)
{
	UINT required_clusters = flen / fs->ClusterSize;
	if ( flen % fs->ClusterSize != 0 ) required_clusters++;
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
	UCHAR root_cluster[ fs->ClusterSize ];
	UCHAR *start_entries = FatLocateEntries( fs, root_cluster, dir, required_entries_total, &cluster );
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
			UINT currcluster = FatAllocCluster( fs );
			if ( !firstcluster )
			{
				firstcluster = currcluster;
			}
			UCHAR cluster_buffer[ fs->ClusterSize ];
			RtlSetMemory( cluster_buffer, 0, fs->ClusterSize );
			UINT bytes_to_write = flen - writtenbytes;
			if ( bytes_to_write > fs->ClusterSize )
			{
				bytes_to_write = fs->ClusterSize;
			}
			RtlCopyMemory( cluster_buffer, file + writtenbytes, bytes_to_write );
			writtenbytes += bytes_to_write;
			FatPutCluster( fs, cluster_buffer, currcluster );
			if ( prevcluster )
			{
				fs->FAT[ prevcluster ] = currcluster;
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
	FatPutCluster( fs, root_cluster, cluster );
	FatFlushFAT( fs );
}

VOID
FatWriteFile(
	FAT32 *fs,
	struct Directory *dir,
	UCHAR *file,
	CHAR *fname,
	UINT flen
)
{
	FatWriteFileImplementation( fs, dir, file, fname, flen, 0, 0 );
}

VOID
FatMakeDirectory(
	FAT32 *fs,
	struct Directory *dir,
	CHAR *dirname
)
{
	FatWriteFileImplementation( fs, dir, NULL, dirname, 0, DIRECTORY, 0 );

	// We need to add the subdirectories '.' and '..'
	struct Directory subdir;
	FatPopulateDir( fs, &subdir, dir->cluster );
	UINT i;
	for ( i = 0; i < subdir.num_entries; i++ )
	{
		if ( strcmp( subdir.entries[ i ].name, dirname ) == 0 )
		{
			struct Directory newsubdir;
			FatPopulateDir( fs, &newsubdir, subdir.entries[ i ].first_cluster );

			FatFreeDirectory( fs, &newsubdir );
		}
	}
	FatFreeDirectory( fs, &subdir );
}

VOID
FatPrintDirectory(
	FAT32 *fs,
	struct Directory *dir
)
{
	UINT i;
	UINT max_name = 0;
	for ( i = 0; i < dir->num_entries; i++ )
	{
		UINT namelen = strlen( dir->entries[ i ].name );
		max_name = namelen > max_name ? namelen : max_name;
	}

	CHAR *namebuff = malloc( max_name + 1 );
	for ( i = 0; i < dir->num_entries; i++ )
	{
		//        DbgPrintFmt("[%d] %*s %c %8d bytes ",
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
			cluster = fs->FAT[ cluster ];
			if ( cluster >= EOC ) break;
			if ( cluster == 0 )
			{
				DbgPrintFmt( "BAD CLUSTER CHAIN! FS IS CORRUPT!" );
			}
			else cluster_count++;
			
		}
		DbgPrintFmt( "clusters: [%d]\n", cluster_count );
	}
	free( namebuff );
}

UINT
FatGetFreeClusters(
	FAT32 *fs
)
{
	UINT clusters_in_fat = ( fs->BiosParameterBlock.SectorsPerFat32 * 512 ) / 4;
	UINT i;
	UINT count = 0;
	for ( i = 0; i < clusters_in_fat; i++ )
	{
		if ( fs->FAT[ i ] == 0 )
		{
			count++;
		}
	}
	return count;
}