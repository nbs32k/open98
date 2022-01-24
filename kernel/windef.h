#pragma once




void *term_write_ptr;


#ifndef NULL
#define NULL 0
#endif

#define CONST                   const
#define CONSTANT                const

#define EXTERN                  extern
#define STATIC                  static
#define VOLATILE                volatile

typedef unsigned long long      NTSTATUS, *PNTSTATUS;

typedef signed char             CHAR, *PCHAR, *PSTR;
typedef signed short            SHORT, *PSHORT;
typedef signed int              INT, *PINT;
typedef signed long             LONG, *PLONG;
typedef signed long             LONG32, *PLONG32;
typedef signed long long        LONG64, *PLONG64;

typedef unsigned char           UCHAR, *PUCHAR;
typedef unsigned short          USHORT, *PUSHORT;
typedef unsigned int            UINT, *PUINT;
typedef unsigned long           ULONG, *PULONG;
typedef unsigned long           ULONG32, *PULONG32;
typedef unsigned long long      ULONG64, *PULONG64;

typedef unsigned char           BOOLEAN, *PBOOLEAN;
typedef long long               HANDLE, *PHANDLE;
typedef unsigned long long      SIZE_T, *PSIZE_T;
typedef unsigned long long      UCHAR_PTR, USHORT_PTR, ULONG_PTR, ULONG32_PTR, ULONG64_PTR;

typedef void                    *PVOID;
typedef const void              *PCVOID;

typedef unsigned char           BYTE, *PBYTE;
typedef unsigned short          WCHAR, *PWCHAR, *PWSTR;
typedef const char              *PCCHAR, *PCSTR;

typedef float                   FLOAT;
typedef double                  DOUBLE;
typedef char*                   VA_LIST;



#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

#define VOID                    void
#define TRUE                    1
#define FALSE                   0


#define FLAT_PTR(PTR) (*((PUINT(*)[])(PTR)))

#define BYTE_PTR(PTR) (*((UCHAR *)(PTR)))
#define WORD_PTR(PTR) (*((USHORT *)(PTR)))
#define DWORD_PTR(PTR) (*((UINT *)(PTR)))
#define QWORD_PTR(PTR) (*((ULONG64 *)(PTR)))

typedef struct _KSYSTEM_TIME
{
	INT Seconds;
	INT Minutes;
	INT Hours;
	INT Days;
	INT Months;
	INT Years;
} KSYSTEM_TIME, *PKSYSTEM_TIME;