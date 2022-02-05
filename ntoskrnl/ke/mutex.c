#include "ki.h"

VOID
KeTryAcquireMutex(
	KMUTEX_OBJECT Mutex
)
{
	while ( !__sync_bool_compare_and_swap( &Mutex, 0, 1 ) )
		 while ( Mutex )
			 asm volatile ("pause");
}

VOID
KeReleaseMutex(
	KMUTEX_OBJECT Mutex
)
{
	__sync_lock_release( &Mutex );
}
