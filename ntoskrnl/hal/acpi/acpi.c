#include "../../windef.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "acpi.h"


struct rsdp
{
	char signature[ 8 ];
	UCHAR checksum;
	char oem_id[ 6 ];
	UCHAR rev;
	UINT rsdt_addr;
	// ver 2.0 only
	UINT length;
	uint64_t xsdt_addr;
	UCHAR ext_checksum;
	UCHAR reserved[ 3 ];
} __attribute__( ( packed ) );

struct rsdt
{
	struct sdt sdt;
	symbol ptrs_start;
} __attribute__( ( packed ) );

static bool use_xsdt;
static struct rsdt *rsdt;

/* This function should look for all the ACPI tables and index them for
   later use */
VOID
HalInitACPI(
	struct rsdp *rsdp
)
{
	if ( rsdp->rev >= 2 && rsdp->xsdt_addr )
	{
		use_xsdt = true;
		rsdt = ( struct rsdt * )( ( uintptr_t )rsdp->xsdt_addr );
	}
	else
	{
		use_xsdt = false;
		rsdt = ( struct rsdt * )( ( uintptr_t )rsdp->rsdt_addr );
	}
}

/* Find SDT by signature */
void *acpi_find_sdt( const char *signature, size_t index )
{
	size_t cnt = 0;

	for ( size_t i = 0; i < rsdt->sdt.length - sizeof( struct sdt ); i++ )
	{
		struct sdt *ptr;
		if ( use_xsdt )
			ptr = ( struct sdt * )( uintptr_t )( ( uint64_t * )rsdt->ptrs_start )[ i ];
		else
			ptr = ( struct sdt * )( uintptr_t )( ( UINT * )rsdt->ptrs_start )[ i ];

		if ( !strncmp( ptr->signature, signature, 4 ) && cnt++ == index )
		{
			return ( void * )ptr;
		}
	}

	return NULL;
}