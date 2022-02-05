#pragma once

#include <stdint.h>

void init_ahci( uint64_t hba_addr );

/* SATA uses FIS (Frame Information Structure), FISs are packets
 * containing a header that identifies itself and the payload data
 * (the data that is not metadata)
 *
 * there are different types of FIS, so we are going to define them
 * along with their identification code */

#define FIS_TYPE_REG_H2D 0x27   /* Register FIS; host to dev        */
#define FIS_TYPE_REG_D2H 0x34   /* Register FIX; dev to host        */
#define FIS_TYPE_DMA_ACT 0x39   /* DMA activate FIS; dev to host    */
#define FIS_TYPE_DMA_SETUP 0x41 /* DMA setup FIS; bidirectional     */
#define FIS_TYPE_DATA 0x46      /* DATA FIS; bidirectional          */
#define FIS_TYPE_BIST 0x58      /* BIST activate FIS; bidirectional */
#define FIS_TYPE_PIO_SETUP 0x5F /* PIO setup FIS; dev to host       */
#define FIS_TYPE_DEV_BITS 0xA1  /* Set dev bits FIS; dev to host    */

 /* a host to dev register FIS is used by the host to send a command or control
  * to a device, the kernel will send this */
typedef struct fis_reg_h2d
{
	uint8_t fis_type; /* FIS_TYPE_REG_H2D */

	uint8_t pm_port : 4; /* port multiplier */
	uint8_t reserved_1 : 3;
	uint8_t c : 1; /* 1: command, 0: control */

	uint8_t command;     /* command register */
	uint8_t feature_low; /* feature register */

	uint8_t lba_low;  /* LBA low register  */
	uint8_t lba_mid;  /* LBA mid register  */
	uint8_t lba_high; /* LBA high register */
	uint8_t dev;      /* dev register      */

	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t feature_high; /* feature register */

	uint8_t count_low;  /* count register                 */
	uint8_t count_high; /* count register                 */
	uint8_t icc;        /* isochronous command completion */
	uint8_t control;    /* control register               */

	uint8_t reserved_2[ 4 ];
} fis_reg_h2d_t;

/* a dev to host register FIS is used by the device to notify the host
 * that some ATA register has changed, it contains data about things
 * that have been updated (status, error and other registers), the kernel
 * will receive this */
typedef struct fis_reg_d2h
{
	uint8_t fis_type; /* FIS_TYPE_REG_D2H */

	uint8_t pm_port : 4; /* port multiplier */
	uint8_t reserved_1 : 2;
	uint8_t i : 1; /* interrupt bit */
	uint8_t reserved_2 : 1;

	uint8_t status; /* status register */
	uint8_t error;  /* error register  */

	uint8_t lba_low;  /* LBA low register  */
	uint8_t lba_mid;  /* LBA mid register  */
	uint8_t lba_high; /* LBA high register */
	uint8_t dev;      /* dev register      */

	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t reserved_3;

	uint8_t count_low;  /* count register */
	uint8_t count_high; /* count register */
	uint8_t reserved_4[ 2 ];

	uint8_t reserved_5[ 4 ];
} fis_reg_d2h_t;

/* used by the host or the device itself to send data payload */
typedef struct fis_data
{
	uint8_t fis_type; /* FIS_TYPE_DATA */

	uint8_t pm_port : 4; /* port multiplier */
	uint8_t reserved_1 : 4;

	uint8_t reserved_2[ 2 ];

	uint32_t data[ 1 ]; /* payload */
} fis_data_t;

/* used by the device to tell the host that it's about to
 * send or ready to receive a PIO data payload */
typedef struct fis_pio_setup
{
	uint8_t fis_type; /* FIS_TYPE_PIO_SETUP */

	uint8_t pm_port : 4; /* port multiplier */
	uint8_t reserved_1 : 1;
	uint8_t d : 1; /* data transfer direction, 1: dev to host */
	uint8_t i : 1; /* interrupt bit                           */
	uint8_t reserved_2 : 1;

	uint8_t status; /* status register */
	uint8_t error;  /* error register  */

	uint8_t lba_low;  /* LBA low register  */
	uint8_t lba_mid;  /* LBA mid register  */
	uint8_t lba_high; /* LBA high register */
	uint8_t dev;      /* dev register      */

	uint8_t lba3;
	uint8_t lba4;
	uint8_t lba5;
	uint8_t reserved_3;

	uint8_t count_low;  /* count register */
	uint8_t count_high; /* count register */
	uint8_t reserved_4;
	uint8_t e_status; /* new value of status register */

	uint16_t tc; /* transfer count */
	uint8_t reserved_5[ 2 ];
} fis_pio_setup_t;

/* Direct memory access (DMA), way faster than PIO */
typedef struct fis_dma_setup
{
	uint8_t fis_type; /* FIS_TYPE_DMA_SETUP */

	uint8_t pm_port : 4; /* port multiplier */
	uint8_t reserved_1 : 1;
	uint8_t d : 1; /* data transfer direction, 1: dev to host */
	uint8_t i : 1; /* interrupt bit                           */
	uint8_t a : 1; /* specifies if DMA Activate FIS is needed */

	uint8_t reserved_2[ 2 ];

	uint64_t DMA_buffer_id; /* DMA buffer identifier */

	uint32_t reserved_3;

	uint32_t DMA_buffer_offset;

	uint32_t transfer_count; /* number of bytes to transfer */

	uint32_t reserved_4;
} fis_dma_setup_t;

/* define the port structure for later use in hba_mem */
typedef volatile struct hba_port
{
	uint32_t clb;  /* command list base address                 */
	uint32_t clbu; /* command list base address (upper 32 bits) */
	uint32_t fb;   /* FIS base address                          */
	uint32_t fbu;  /* FIS base address (upper 32 bits)           */
	uint32_t is;   /* interrupt status                          */
	uint32_t ie;   /* interrupt enable                          */
	uint32_t cmd;  /* command and status                        */
	uint32_t reserved_1;
	uint32_t tfd;  /* task file data                         */
	uint32_t sig;  /* signature                              */
	uint32_t ssts; /* SATA status (SCR0:SStatus)             */
	uint32_t sctl; /* SATA control (SCR2:SControl)           */
	uint32_t serr; /* SATA error (SCR1:SError)               */
	uint32_t sact; /* SATA active (SCR3:SActive)             */
	uint32_t ci;   /* command issue                          */
	uint32_t sntf; /* SATA notification (SCR4:SNotification) */
	uint32_t fbs;  /* FIS-based switch control               */
	uint32_t reserved_2[ 11 ];
	uint32_t vendor[ 4 ]; /* vendor specific */
} hba_port_t;

/* https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/serial-ata-ahci-spec-rev1-3-1.pdf
 * page 15 (onwards) has some useful information for the following structures */

 /* (https://wiki.osdev.org/AHCI#AHCI_Registers_and_Memory_Structures)
  *
  * the last BAR (BAR5 or ABAR) of the AHCI controller PCI device, points to
  * the AHCI base address
  *
  * the HBA (host bus adapter) structure can be found at that base address
  * and it contains several memory registers
  *
  * generic host control registers control the behavior of the whole controller,
  * while each port owns its own set of port control registers (that is, hba_port)
  */
typedef volatile struct hba_mem
{
	/* 0x00 to 0x2C, generic host control memory registers */
	uint32_t cap;     /* host capability                       */
	uint32_t ghc;     /* global host control                   */
	uint32_t is;      /* interrupt status                      */
	uint32_t pi;      /* port implemented                      */
	uint32_t vs;      /* version                               */
	uint32_t ccc_ctl; /* command completion coalescing control */
	uint32_t ccc_pts; /* command completion coalescing ports   */
	uint32_t em_loc;  /* enclosure management location         */
	uint32_t em_ctl;  /* enclosure management control          */
	uint32_t cap2;    /* host capabilities extended            */
	uint32_t bohc;    /* BIOS/OS handoff control and status    */

	/* 0x2C to 0xA0, reserved */
	uint8_t reserved[ 0xA0 - 0x2C ];

	/* 0xA0 to 0x100, vendor specific memory registers */
	uint8_t vendor[ 0x100 - 0xA0 ];

	/* 0x100 to 0x1100, port control registers  */
	hba_port_t ports[ 1 ]; /* [port 0] to port 31 */
} hba_mem_t;

/* each port will correspond to one, and only one, SATA device,
 * that is, each port can attach one SATA device
 *
 * the host (the kernel) will send commands to the device using the command
 * list structure and the device will deliver data to the kernel using the
 * received FIS structure
 *
 * they are located at hba_port.clb/clbu and hba_port.fb/fbu */

typedef struct hba_cmd_header
{
	/* to send a command, the host will fill a header like this
	 * one, and will set the corresponding bit in the hba_port.ci (command issue)
	 * register
	 *
	 * each port has 32 headers like this one (the command list is compound
	 * of 32 command headers)
	 *
	 * each command header describes an ATA or ATAPI command; hba_port.ci
	 * is a 32-bit register, so there is one bit for each command
	 *
	 * the AHCI controller will send the command to the device attached
	 * to that specific port
	 *
	 * if everything worked, the bit in the command issue register will be cleared */

	uint8_t cfl : 5; /* command FIS len in DWORDS */
	uint8_t a : 1;   /* ATAPI                     */
	uint8_t w : 1;   /* write, 1: H2D, 0: D2H     */
	uint8_t p : 1;   /* prefetchable              */

	uint8_t r : 1; /* reset */
	uint8_t b : 1; /* BIST  */
	uint8_t c : 1; /* clear busy upon R_OK */
	uint8_t reserved_1 : 1;
	uint8_t pmp : 4; /* port multiplier port */

	uint16_t prdtl; /* physical region descriptor table length in entries */

	volatile uint32_t prdbc; /* physical region descriptor byte count transferred */

	uint32_t ctba;  /* command table descriptor base address                 */
	uint32_t ctbau; /* command table descriptor base address (upper 32 bits) */

	uint32_t reserved_2[ 4 ];
} hba_cmd_header_t;

typedef volatile struct hba_fis
{
	/* there are 4 types of FIS that may be sent by the device
	 * to the host (the kernel)
	 *
	 * the FIS will be copied into the host memory and a bit
	 * will be set in hba_port.is */
	fis_dma_setup_t dsfis; /* DMA setup FIS */
	uint8_t pad1[ 4 ];

	fis_pio_setup_t psfis; /* PIO setup FIS */
	uint8_t pad2[ 12 ];

	fis_reg_d2h_t rfis; /* register - dev to host FIS */
	uint8_t pad3[ 4 ];

	uint16_t sdbfis; /* set device bit FIS */

	uint8_t ufis[ 64 ];

	uint8_t rsv[ 0x100 - 0xA0 ];
} hba_fis_t;

/* define the PRDT entry structure for later use in hba_cmd_tbl */
typedef struct hba_prdt_entry
{
	uint32_t dba;  /* data base address                 */
	uint32_t dbau; /* data base address (upper 32 bits) */
	uint32_t reserved_1;

	uint32_t dbc : 22; /* byte count */
	uint32_t reserved_2 : 9;
	uint32_t i : 1; /* interrupt completion */
} hba_prdt_entry_t;

typedef struct hba_cmd_tbl
{
	uint8_t cfis[ 64 ]; /* command FIS */

	uint8_t acmd[ 16 ]; /* ATAPI command */

	uint8_t reserved[ 48 ];

	/* physical region descriptor table entries, 0 ~ 65535 */
	hba_prdt_entry_t prdt_entry[ 1 ];
} hba_cmd_tbl_t;