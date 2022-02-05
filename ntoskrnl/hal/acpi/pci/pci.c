#include "pci.h"
#include "../../../mm/mm.h"
#include "../../../drv/ahci.h"
USHORT pci_conf_read_word( UCHAR bus, UCHAR dev, UCHAR func, UCHAR offset )
{ /* refer to the pci.h figure for more information */
	UINT addr;
	UINT lbus = ( UINT )bus;
	UINT ldev = ( UINT )dev;
	UINT lfunc = ( UINT )func;
	USHORT tmp = 0;

	/* remember that the OR operator flips bits without affecting the other ones */
	/* that is, 0101 OR 1010 = 1111                                              */
	addr = ( UINT )( ( lbus << 16 ) | ( ldev << 11 ) | ( lfunc << 8 ) | ( offset & 0xFC ) | ( 1 << 31 ) );

	outl( CONFIG_ADDRESS, addr ); /* write our new config to the config register, as seen in pci.h */
	tmp = ( USHORT )( ( inl( CONFIG_DATA ) >> ( offset & 2 ) * 8 ) & 0xFFFF );
	return tmp;
}

UINT pci_conf_read_dword( UCHAR bus, UCHAR dev, UCHAR func, UCHAR offset )
{ /* refer to the pci.h figure for more information */
	UINT addr;
	UINT lbus = ( UINT )bus;
	UINT ldev = ( UINT )dev;
	UINT lfunc = ( UINT )func;
	UINT tmp = 0;

	addr = ( UINT )( ( lbus << 16 ) | ( ldev << 11 ) | ( lfunc << 8 ) | ( offset & 0xFC ) | ( 1 << 31 ) );

	outl( CONFIG_ADDRESS, addr ); /* write our new config to the config register, as seen in pci.h */
	tmp = ( UINT )( inl( CONFIG_DATA ) );
	return tmp;
}

/* Register Offset | Bits 31-24 | Bits 23-16 | Bits 15-8 | Bits 7-0
 * 0x0      0x0    | Device ID               | Vendor ID
 * 0x2      0x8    | Class code | Subclass   | Prog IF   | Revision ID
 */

USHORT get_vendor_id( UCHAR bus, UCHAR dev, UCHAR func )
{
	return pci_conf_read_word( bus, dev, func, 0 );
}

USHORT get_dev_id( UCHAR bus, UCHAR dev, UCHAR func )
{
	return pci_conf_read_word( bus, dev, func, 0x0 + 2 );
}

USHORT get_class( UCHAR bus, UCHAR dev, UCHAR func )
{
	/* this will read bits 31-16, but device class is located
	 * in bits 31-24, so we will shift everything 8 bits, discarding
	 * the first byte and getting the second one */
	return pci_conf_read_word( bus, dev, func, 0x8 + 2 ) >> 8;
}

USHORT get_subclass( UCHAR bus, UCHAR dev, UCHAR func )
{
	/* this will read bits 31-16, but device subclass is located
	 * in bits 23-16, so we will AND 0xFF (equivalent to an entire byte)
	 * to just get bits 23-16 and discard bits 31-24 */
	return pci_conf_read_word( bus, dev, func, 0x8 + 2 ) & 0xFF;
}

UINT get_io_address( UCHAR bus, UCHAR dev, UCHAR func )
{
	/* Base Address Registers (or BARs) are used to hold memory
	 * addresses used by the device
	 *
	 * NOTE keep in mind that this implementation is simplified
	 * so it just works for some specific devices
	 */
	ULONG64 addr = 0x0;
	UINT bar0 = pci_conf_read_dword( bus, dev, func, 0x10 );
	if ( bar0 & 1 )                /* I/O space BAR */
		addr = ( bar0 >> 2 ) << 2; /* all the bits except the first two */
	return addr;
}

UINT get_abar( UCHAR bus, UCHAR dev, UCHAR func )
{
	ULONG64 addr = 0x0;
	UINT bar5 = pci_conf_read_dword( bus, dev, func, 0x24 );
	if ( !( bar5 & 1 ) )             /* Memory Space BAR */
		addr = ( bar5 >> 4 ) << 4; /* all bits except the first four */
	return addr;
}

char *get_vendor_name( USHORT vendor )
{
	switch ( vendor )
	{
		case 0x8086:
			return "Intel Corporation";
		case 0x10DE:
			return "NVIDIA";
		case 0x1002:
			return "Advanced Micro Devices Inc.";
		case 0x10EC:
			return "Realtek Semiconductor Corp.";
		case 0x1234:
			return "Qemu";
		case 0x80EE:
			return "Oracle Corporation";
		default:
			return "Unknown";
	}
}

/* for a more extensive list, refer to
 * https://wiki.osdev.org/PCI
 */

const char *class_codes[ ] = {
	"Unclassified",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge",
	"Simple Communication Controller",
	"Base System Peripheral",
	"Input Device Controller",
	"Docking Station",
	"Processor",
	"Serial Bus Controller",
	"Wireless Controller" };

static struct
{
	USHORT class, subclass;
	const char *name;
} subclass_codes[ ] = {
	{0x00, 0x00, "Non-VGA-Compatible Unclassified Device"},
	{0x00, 0x01, "VGA-Compatible Unclassified Device"},

	{0x01, 0x00, "SCSI Bus Controller"},
	{0x01, 0x01, "IDE Controller"},
	{0x01, 0x02, "Floppy Disk Controller"},
	{0x01, 0x03, "IPI Bus Controller"},
	{0x01, 0x04, "RAID Controller"},
	{0x01, 0x05, "ATA Controller"},
	{0x01, 0x06, "SATA Controller"},
	{0x01, 0x07, "Serial Attached SCSI Controller"},
	{0x01, 0x08, "Non-Volatile Memory Controller"},

	{0x02, 0x00, "Ethernet Controller"},
	{0x02, 0x01, "Token Ring Controller"},
	{0x02, 0x02, "FDDI Controller"},
	{0x02, 0x03, "ATM Controller"},
	{0x02, 0x04, "ISDN Controller"},
	{0x02, 0x05, "WorldFip Controller"},
	{0x02, 0x06, "PICMG 2.14 Multi Computing"},
	{0x02, 0x07, "Infiniband Controller"},

	{0x03, 0x00, "VGA Compatible Controller"},
	{0x03, 0x01, "XGA Controller"},
	{0x03, 0x02, "3D Controller"},

	{0x04, 0x00, "Multimedia Video Controller"},
	{0x04, 0x01, "Multimedia Audio Controller"},
	{0x04, 0x02, "Computer Telephony Device"},
	{0x04, 0x03, "Audio Device"},

	{0x05, 0x00, "RAM Controller"},
	{0x05, 0x01, "Flash Controller"},

	{0x06, 0x00, "Host Bridge"},
	{0x06, 0x01, "ISA Bridge"},
	{0x06, 0x02, "EISA Bridge"},
	{0x06, 0x03, "MCA Bridge"},
	{0x06, 0x04, "PCI-to-PCI Bridge"},
	{0x06, 0x05, "PCMCIA Bridge"},
	{0x06, 0x06, "NuBus Bridge"},
	{0x06, 0x07, "CardBus Bridge"},
	{0x06, 0x08, "RACEWay Bridge"},
	{0x06, 0x09, "PCI-to-PCI Bridge"},
	{0x06, 0x0A, "InfiniBand-to-PCI Host Bridge"},

	{0x07, 0x00, "Serial Controller"},
	{0x07, 0x01, "Parallel Controller"},
	{0x07, 0x02, "Multiport Serial Controller"},
	{0x07, 0x03, "Modem"},
	{0x07, 0x04, "IEEE 488.1/2 (GPIB) Controller"},
	{0x07, 0x05, "Smart Card Controller"},

	{0x08, 0x00, "PIC"},
	{0x08, 0x01, "DMA Controller"},
	{0x08, 0x02, "Timer"},
	{0x08, 0x03, "RTC Controller"},
	{0x08, 0x04, "PCI Hot-Plug Controller"},
	{0x08, 0x05, "SD Host controller"},
	{0x08, 0x06, "IOMMU"},

	{0x09, 0x00, "Keyboard Controller"},
	{0x09, 0x01, "Digitizer Pen"},
	{0x09, 0x02, "Mouse Controller"},
	{0x09, 0x03, "Scanner Controller"},
	{0x09, 0x04, "Gameport Controller"},

	{0x0B, 0x00, "386"},
	{0x0B, 0x01, "486"},
	{0x0B, 0x02, "Pentium"},
	{0x0B, 0x03, "Pentium Pro"},
	{0x0B, 0x10, "Alpha"},
	{0x0B, 0x20, "PowerPC"},
	{0x0B, 0x30, "MIPS"},
	{0x0B, 0x40, "Co-Processor"},

	{0x0C, 0x00, "FireWire (IEEE 1394) Controller"},
	{0x0C, 0x01, "ACCESS Bus Controller"},
	{0x0C, 0x01, "ACCESS Bus Controller"},
	{0x0C, 0x02, "SSA"},
	{0x0C, 0x03, "USB Controller"},
	{0x0C, 0x04, "Fibre Channel"},
	{0x0C, 0x05, "SMBus Controller"},
	{0x0C, 0x06, "InfiniBand Controller"},
	{0x0C, 0x07, "IPMI Interface"},
	{0x0C, 0x08, "SERCOS Interface (IEC 61491)"},
	{0x0C, 0x09, "CANbus Controller"},

	{0x0D, 0x00, "iRDA Compatible Controller"},
	{0x0D, 0x01, "Consumer IR Controller"},
	{0x0D, 0x10, "RF Controller"},
	{0x0D, 0x11, "Bluetooth Controller"},
	{0x0D, 0x12, "Broadband Controller"},
	{0x0D, 0x20, "Ethernet Controller (802.1a)"},
	{0x0D, 0x21, "Ethernet Controller (802.1b)"},
};

class_t get_class_name( USHORT class, USHORT subclass )
{
	class_t temp;
	temp.class_name = ( class_codes[ class ] ) ? class_codes[ class ] : "Other";
	temp.subclass_name = NULL;
	for ( UCHAR i = 0; subclass_codes[ i ].name; i++ )
	{
		if ( subclass_codes[ i ].class == class && subclass_codes[ i ].subclass == subclass )
			temp.subclass_name = subclass_codes[ i ].name;
	}
	if ( temp.subclass_name == NULL ) /* (subclass_code = 0x80) */
		temp.subclass_name = "Other";
	return temp;
}

VOID
HalPCIInit(

)
{
	PCHAR PCI = malloc( 512 );
	for ( UINT bus = 0; bus < 256; bus++ )
	{
		for ( UINT dev = 0; dev < 32; dev++ )
		{
			for ( UINT func = 0; func < 8; func++ )
			{
				dev_t temp;
				if ( ( temp.vendor = get_vendor_id( bus, dev, func ) ) == 0xFFFF ) /* not a valid vendor */
					continue;
				temp.dev = get_dev_id( bus, dev, func );
				temp.class = get_class( bus, dev, func );
				temp.subclass = get_subclass( bus, dev, func );

				/* if it is a configurable device, configure it,
				 * otherwise, just print the pertinent information */
				if ( temp.dev == 0x8029 ) /* NE2000 network card */
				{
					temp.ioaddr = get_io_address( bus, dev, func );
					if ( temp.ioaddr );
						//init_ne2k( temp.ioaddr );
				}
				else if ( temp.dev == 0x8139 ) /* RTL8139 network card */
				{
					temp.ioaddr = get_io_address( bus, dev, func );
					if ( temp.ioaddr );
						//init_rtl8139( temp.ioaddr );
				}
				else if ( temp.class == 0x1 && temp.subclass == 0x6 ) /* AHCI controller */
				{
					temp.abar = get_abar( bus, dev, func );
					if ( temp.abar );
						//init_ahci( temp.abar + PHYS_MEM_OFFSET ); /* not virtually mapped */
				}
				else
				{
					class_t name_temp = get_class_name( temp.class, temp.subclass );

					
					sprintf( PCI, "[ INFO ] PCI: (0x%x) vendor = %s, class = %s, subclass = %s", temp.dev, get_vendor_name( temp.vendor ),
							name_temp.class_name,
							name_temp.subclass_name );
					KiPrintToScreen( PCI );
					
				}
			}
		}
	}

	free( PCI );
}