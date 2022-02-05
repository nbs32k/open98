#pragma once


#include "../../windef.h"

#include <stivale2.h>

struct rsdp_ptr_1
{
	CHAR signature[ 8 ];
	UCHAR checksum;
	CHAR oemid[ 6 ];
	UCHAR revision;
	UINT rsdt_addr;
} __attribute__( ( packed ) );

/* all the System Description Tables have the
 * following header */
struct sdt_header
{
	CHAR signature[ 4 ];
	UINT len;
	UCHAR revision;
	UCHAR checksum;
	CHAR oem_id[ 6 ];
	CHAR oem_table_id[ 8 ];
	UINT oem_revision;
	UINT creator_id;
	UINT creator_revision;
} __attribute__( ( packed ) );

struct rsdt_ptr
{
	struct sdt_header sdt_head;
	UINT other_sdt_ptr[ ];
} __attribute__( ( packed ) );

/* ACPI describe registers as following */
struct gen_addr_struct
{
	UCHAR addr_space;
	UCHAR bit_width;
	UCHAR bit_offset;
	UCHAR access_size;
	ULONG64 addr;
};

/* simplified from https://wiki.osdev.org/FADT */
struct fadt_ptr
{
	struct sdt_header sdt_head;
	UINT firmware_ctrl;
	UINT dsdt_addr;

	UCHAR reserved_1;

	UCHAR power_man_profile;
	USHORT sci_interrupt;
	UINT smi_command_port;
	UCHAR acpi_enable;
	UCHAR acpi_disable;
	UCHAR S4BIOS_REQ;
	UCHAR PSTATE_control;
	UINT PM1a_event_block;
	UINT PM1b_event_block;
	UINT PM1a_control_block;
	UINT PM1b_control_block;
	UINT PM2_control_block;
	UINT PM_timer_block;
	UINT GPE0_block;
	UINT GPE1_block;
	UCHAR PM1_event_len;
	UCHAR PM1_control_len;
	UCHAR PM2_control_len;
	UCHAR PM_timer_len;
	UCHAR GPE0_len;
	UCHAR GPE1_len;
	UCHAR GPE1_base;
	UCHAR C_state_control;
	USHORT worst_C2_latency;
	USHORT worst_C3_latency;
	USHORT flush_size;
	USHORT flush_stride;
	UCHAR duty_offset;
	UCHAR duty_width;

	UCHAR day_alarm;
	UCHAR month_alarm;
	UCHAR century;

	USHORT reserved_boot_arch_flags; /* reserved in ACPI 1.0 */

	UCHAR reserved_2;
	UINT flags;

	struct gen_addr_struct reset_reg;

	UCHAR reset_value;
	UCHAR reserved_3[ 3 ];
} __attribute__( ( packed ) );

VOID
HalInitACPI(
	struct stivale2_struct_tag_rsdp* rsdp_tag
);

static struct fadt_ptr *fadt;
ULONG64 get_SLP_TYPa( );

VOID
HalACPIShutdown( );