#include "mm.h"
#include "../ke/ki.h"
DEFINE_LOCK( liballoc_slock );

#define ROUND_UP(A, B)                                                         \
  ({                                                                           \
    __typeof__(A) _a_ = A;                                                     \
    __typeof__(B) _b_ = B;                                                     \
    (_a_ + (_b_ - 1)) / _b_;                                                   \
  })



void *liballoc_alloc( int size )
{
	return ( void * )( ( uintptr_t )pcalloc( ROUND_UP( size, PAGE_SIZE ) ) +
					PHYS_MEM_OFFSET );
}

int liballoc_free_( void *ptr, int pages )
{
	pmm_free_pages( ptr - PHYS_MEM_OFFSET, pages + 1 );
	return 0;
}

int liballoc_lock( )
{
	 KeTryAcquireMutex( liballoc_slock );
	return 0;
}

int liballoc_unlock( )
{
	KeReleaseMutex( liballoc_slock );
	return 0;
}

void *malloc( size_t size )
{
	return liballoc_malloc( size );
}

void free( void *ptr )
{
	liballoc_free( ptr );
}

void *krealloc( void *ptr, size_t size )
{
	return liballoc_realloc( ptr, size );
}

void *kcalloc( size_t size )
{
	void *ptr = liballoc_malloc( size );
	memset( ptr, 0, size );
	return ptr;
}