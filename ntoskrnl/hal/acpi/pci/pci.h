#pragma once

#include "../../../drv/drv.h"
#include "../../../windef.h"
#include "../../../ke/ki.h"
#include "../acpi.h"


#define CONFIG_ADDRESS 0xCF8 /* these are I/O locations */
#define CONFIG_DATA 0xCFC

/* CONFIG_ADDRESS is mapped to a 32-bit register */
/* with the following structure */

/* Bit 31      Bits 30-24  Bits 23-16   Bits 15-11     Bits 10-8       Bits 7-0 */
/* Enable Bit  Reserved    Bus Number   Device Number  Function Number Register Offset */

/* brief explanation
 * 31. configuration flag
 * 23-16. choose an specific bus address
 * 15-11. select a device attached to the bus
 * 10-8. choose an specific function of the device
 */

typedef struct dev
{
	USHORT vendor;
	USHORT dev;
	USHORT class;
	USHORT subclass;
	UINT ioaddr;
	UINT abar;
} dev_t;

typedef struct class
{
	const CHAR *class_name;
	const CHAR *subclass_name;
} class_t;

VOID
HalPCIInit(

);