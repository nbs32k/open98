
#pragma once

#include <stdint.h>
#include <stddef.h>

struct rsdp;

struct sdt
{
	char signature[ 4 ];
	UINT length;
	UCHAR rev;
	UCHAR checksum;
	char oem_id[ 6 ];
	char oem_table_id[ 8 ];
	UINT oem_rev;
	UINT creator_id;
	UINT creator_rev;
} __attribute__( ( packed ) );

VOID
HalInitACPI(
	struct rsdp *rsdp
);
void *acpi_find_sdt( const char *signature, size_t index );


typedef char symbol[ ];