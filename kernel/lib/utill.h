#pragma once
#include "../windef.h"
#include "printf.h"

PCHAR strcpy( PCHAR destination, const char* source );
PCHAR strcat( PCHAR destination, const char* source );
int strcmp( const char* a, const char* b );
void memcpy( void* dest, void* src, INT n );
void memset( void* str, char ch, INT n );
void memmove( void* dest, void* src, INT n );
int strncmp( const char *s1, const char *s2, INT n );
int memcmp( const void *s1, const void *s2, INT n );

ULONG64
strlen(
	PCSTR String
);

#define RtlCopyMemory memcpy
#define RtlSetMemory memset
#define RtlZeroMemory(A, B) RtlSetMemory(A, 0, B)
#define RtlCompareMemory memcmp
#define RtlStringLength strlen




//math.c - thanks to sleep
FLOAT sqrt( FLOAT x );
FLOAT fast_isqrt( FLOAT x );
FLOAT hypot( FLOAT x, FLOAT y );
FLOAT fabs( FLOAT x );

VOID
KiInitSinTable(

);

FLOAT sin( FLOAT x );
FLOAT cos( FLOAT x );
FLOAT tan( FLOAT x );