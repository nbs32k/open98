#include "ahci.h"
#include <lib/utill.h>
#include "../drv/drv.h"
#include "../ke/ki.h"

#include <stdbool.h>


/* the whole AHCI explanation can be found in the ahci.h header file,
 * the documentation for specific functions can be found here */

#define SATA_SIG_ATA 0x00000101   /* SATA drive                  */
#define SATA_SIG_ATAPI 0xEB140101 /* SATAPI drive                */
#define SATA_SIG_SEMB 0xC33C0101  /* enclosure management bridge */
#define SATA_SIG_PM 0x96690101    /* port multiplier             */

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define AHCI_BASE 0x400000 /* 4096 bytes */



#define HBA_PxCMD_CR 0x8000
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FR 0x4000

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define ATA_CMD_READ_DMA_EX 0x25

#define HBA_PxIS_TFES (1 << 30)


int check_type( hba_port_t *port )
{
	uint32_t ssts = port->ssts; /* SATA status */

	uint8_t ipm = ( ssts >> 8 ) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if ( det != HBA_PORT_DET_PRESENT )
		return AHCI_DEV_NULL;
	if ( ipm != HBA_PORT_IPM_ACTIVE )
		return AHCI_DEV_NULL;

	switch ( port->sig ) /* signature */
	{
		case SATA_SIG_ATAPI:
			return AHCI_DEV_SATAPI;
		case SATA_SIG_SEMB:
			return AHCI_DEV_SEMB;
		case SATA_SIG_PM:
			return AHCI_DEV_PM;
		default:
			return AHCI_DEV_SATA;
	}
}
bool ReadAhci( hba_port_t* hbaPort, uint64_t sector, uint32_t sectorCount, void* buffer )
{
	uint32_t sectorL = ( uint32_t )sector;
	uint32_t sectorH = ( uint32_t )( sector >> 32 );

	hbaPort->is = ( uint32_t )-1; // Clear pending interrupt bits

	hba_cmd_header_t* cmdHeader = ( hba_cmd_header_t* )hbaPort->clb;
	cmdHeader->cfl = sizeof( fis_reg_h2d_t ) / sizeof( uint32_t ); //command FIS size;
	cmdHeader->w = 0; //this is a read
	cmdHeader->prdtl = 1;

	hba_cmd_tbl_t* commandTable = ( hba_cmd_tbl_t* )( cmdHeader->ctba );
	memset( commandTable, 0, sizeof( hba_cmd_tbl_t ) + ( cmdHeader->prdtl - 1 ) * sizeof( hba_prdt_entry_t ) );

	commandTable->prdt_entry[ 0 ].dba = ( uint32_t )( uint64_t )buffer;
	commandTable->prdt_entry[ 0 ].dbau = ( uint32_t )( ( uint64_t )buffer >> 32 );
	commandTable->prdt_entry[ 0 ].dbc = ( sectorCount << 9 ) - 1; // 512 bytes per sector
	commandTable->prdt_entry[ 0 ].i = 1;

	fis_reg_h2d_t* cmdFIS = ( fis_reg_h2d_t* )( &commandTable->cfis );

	cmdFIS->fis_type = FIS_TYPE_REG_H2D;
	cmdFIS->control = 1; // command
	cmdFIS->command = ATA_CMD_READ_DMA_EX;

	cmdFIS->lba_low = ( uint8_t )sectorL;
	cmdFIS->lba_mid = ( uint8_t )( sectorL >> 8 );
	cmdFIS->lba_high = ( uint8_t )( sectorL >> 16 );
	cmdFIS->lba3 = ( uint8_t )sectorH;
	cmdFIS->lba4 = ( uint8_t )( sectorH >> 8 );
	cmdFIS->lba4 = ( uint8_t )( sectorH >> 16 );

	cmdFIS->dev = 1 << 6; //LBA mode

	cmdFIS->count_low = sectorCount & 0xFF;
	cmdFIS->count_high = ( sectorCount >> 8 ) & 0xFF;

	uint64_t spin = 0;

	while ( ( hbaPort->tfd & ( ATA_DEV_BUSY | ATA_DEV_DRQ ) ) && spin < 1000000 )
	{
		spin++;
	}
	if ( spin == 1000000 )
	{
		return false;
	}

	hbaPort->ci = 1;

	while ( true )
	{

		if ( ( hbaPort->ci == 0 ) ) break;
		if ( hbaPort->is & HBA_PxIS_TFES )
		{
			return false;
		}
	}

	return true;
}

#include "../mm/mm.h"
void probe_port( hba_mem_t *hba_addr )
{
	//UCHAR* bufferSect[ 512 ];
	uint32_t pi = hba_addr->pi; /* copy the port implemented register */
	for ( uint8_t i = 0; i < 32; i++ )
	{
		if ( pi & 1 ) /* test the first bit */
		{
			int dt = check_type( &hba_addr->ports[ i ] );
			
			if ( dt == AHCI_DEV_SATA )
			{
				CHAR* x = ( CHAR* )malloc( 4096 );
				DbgPrintFmt( "[AHCI] SATA drive found, port = %u\n", i );
				if ( ReadAhci( &hba_addr->ports[ i ], 1, 4, x ) )
				{
					static int sex = 8;
					static int sex2 = 400;
					for ( int t = 0; t < 1024; ++t )
					{
						if ( sex < 700 )sex += 8;
						else
						{
							sex = 8;
							sex2 += 16;
						}
						KiDisplayChar( x[ t ], sex, sex2, 0xFFFFFFFF );
						//DbgPrintFmt( "%c", x[ t ] );

					}
					DbgPrintFmt( "[AHCI] SUCCESS READ!" );
					
				}
				free( x );
			}
				
			else if ( dt == AHCI_DEV_SATAPI )
			{
				DbgPrintFmt( "[AHCI] SATAPI drive found, port = %u\n", i );

			}
				
			else if ( dt == AHCI_DEV_SEMB )
				DbgPrintFmt( "[AHCI] SEMB drive found, port = %u\n", i );
			else if ( dt == AHCI_DEV_PM )
				DbgPrintFmt( "[AHCI] PM drive found, port = %u\n", i );
		}
		pi >>= 1; /* next port to the first bit */
	}
}

void start_cmd( hba_port_t *port )
{
	/* wait until CR (bit 15) is cleared */
	while ( port->cmd & ( 1 << 15 ) )
		;

	/* cmd is the command and status register */
	port->cmd |= 1 << 4; /* set FRE */
	port->cmd |= 1;      /* set ST  */
}

void stop_cmd( hba_port_t *port )
{
	/* remember that ~ is NOT and & is AND
	 *  0b101 &= ~0b001
	 *  0b101 &= 0b110
	 *  being the result 0b100 */

	 /* cmd is the command and status register */
	port->cmd &= ~1;        /* clear ST  */
	port->cmd &= ~( 1 << 4 ); /* clear FRE */

	/* wait until FR and CR are cleared */
	for ( ;;)
	{
		if ( port->cmd & ( 1 << 14 ) )
			continue;
		if ( port->cmd & ( 1 << 15 ) )
			continue;
		break;
	}
}

void port_rebase( hba_mem_t *hba_addr )
{
	/* (https://wiki.osdev.org/AHCI#AHCI_port_memory_space_initialization)
	 *
	 * before rebasing the port memory space, our kernel needs to wait for
	 * pending commands */

	uint32_t pi = hba_addr->pi; /* copy the port implemented register */
	for ( uint8_t i = 0; i < 32; i++ )
	{
		if ( pi & 1 ) /* test the first bit */
		{
			hba_port_t *port = &hba_addr->ports[ i ];
			stop_cmd( port ); /* stop the command engine */

			/* clb is command list base address
			 * command list base address: 1K*port */
			port->clb = AHCI_BASE + ( i << 10 );
			port->clbu = 0;
			memset( ( void * )&port->clb, 0, 1024 );

			/* fb is FIS base address
			 * FIS base address: 32K + 256*port */
			port->fb = AHCI_BASE + ( 32 << 10 ) + ( i << 8 );
			port->fbu = 0;
			memset( ( void * )&port->fb, 0, 256 );

			hba_cmd_header_t *cmd_header = ( hba_cmd_header_t * )( uint64_t )port->clb;
			for ( uint8_t i = 0; i < 32; i++ )
			{
				cmd_header[ i ].prdtl = 8;

				cmd_header[ i ].ctba = AHCI_BASE + ( 40 << 10 ) + ( i << 13 ) + ( i << 8 );
				cmd_header[ i ].ctbau = 0;
				memset( ( void * )&cmd_header[ i ].ctba, 0, 256 );
			}

			start_cmd( port ); /* restart the command engine */
		}
		pi >>= 1; /* next port to the first bit */
	}
}


void init_ahci( uint64_t hba_addr )
{
	DbgPrintFmt( "[AHCI] ABAR = 0x%lx, reconfiguring AHCI memory\n", hba_addr );
	port_rebase( ( hba_mem_t * )hba_addr );
	DbgPrintFmt( "[AHCI] AHCI memory spaces configured, enumerating devices\n" );
	probe_port( ( hba_mem_t * )hba_addr );
	DbgPrintFmt( "[AHCI] drive enumeration completed\n" );
}