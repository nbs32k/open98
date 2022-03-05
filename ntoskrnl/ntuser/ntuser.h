#include "../ntdef.h"

typedef struct _POINT
{
	INT X;
	INT Y;
} POINT;

POINT KiMousePosition;
POINT pMousePositionOld;

VOID
NtSetCursorPos(
	INT X,
	INT Y
);

VOID
NtGetCursorPos(
	POINT* lpPoint
);

typedef ULONG64		*HWND;

INT
NtCreateWindow(
	PCHAR	lpClassName,
	PCHAR	lpWindowName,
	ULONG32	dwStyle,
	INT		x,
	INT		y,
	INT		nWidth,
	INT		nHeight,
	HWND	hWndParent,
	ULONG	hMenu,
	ULONG	hInstance,
	PVOID	lpParam
);



//elf.c

#include <stdint.h>

typedef void( *void_func_t )( void );

#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define ELF_DATA_LITTLE 1
#define ELF_DATA_BIG 2

#define ELF_FILE_NONE 0
#define ELF_FILE_REL 1
#define ELF_FILE_EXEC 2
#define ELF_FILE_DYN 3
#define ELF_FILE_CORE 4

struct elf64_header
{
	UINT magic;
	UCHAR file_class;
	UCHAR encoding;
	UCHAR file_version;
	UCHAR os_abi;
	UCHAR abi_version;
	UCHAR padding[ 6 ];
	UCHAR ident_size;
	USHORT file_type;
	USHORT machine_type;
	UINT version;
	ULONG64 entry_point;
	ULONG64 program_header_offset;
	ULONG64 section_header_offset;
	UINT flags;
	USHORT header_size;
	USHORT program_header_size;
	USHORT program_header_count;
	USHORT section_header_size;
	USHORT section_header_count;
	USHORT string_table_index;
};

#define ELF_SECTION_NULL 0
#define ELF_SECTION_PROGBITS 1
#define ELF_SECTION_SYMTAB 2
#define ELF_SECTION_STRTAB 3
#define ELF_SECTION_RELA 4
#define ELF_SECTION_HASH 5
#define ELF_SECTION_DYNAMIC 6
#define ELF_SECTION_NOTE 7
#define ELF_SECTION_NOBITS 8
#define ELF_SECTION_REL 9
#define ELF_SECTION_DYNSYM 11

#define ELF_FLAG_WRITE 1
#define ELF_FLAG_ALLOC 2
#define ELF_FLAG_EXEC 4

struct elf64_section_header
{
	UINT name_index;
	UINT type;
	ULONG64 flags;
	ULONG64 address;
	ULONG64 offset;
	ULONG64 size;
	UINT link;
	UINT info;
	ULONG64 alignment;
	ULONG64 entry_size;
};

#define ELF_REL_TYPE_64 1

struct elf64_rela_entry
{
	ULONG64 offset;
	UINT type;
	UINT symbol;
	LONG64 addend;
};

struct elf64_symbol
{
	UINT name_index;
	UCHAR info;
	UCHAR other;
	USHORT section;
	ULONG64 value;
	ULONG64 size;
};

void
getelf( CHAR* path );