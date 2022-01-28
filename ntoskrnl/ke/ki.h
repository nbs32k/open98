#include "../windef.h"
#include "../hal/hal.h"


typedef volatile BOOLEAN KMUTEX_OBJECT;
#define DEFINE_LOCK(name) static KMUTEX_OBJECT name;

VOID
KeTryAcquireMutex(
	KMUTEX_OBJECT Mutex
);

VOID
KeReleaseMutex(
	KMUTEX_OBJECT Mutex
);


VOID
DbgPrintFmt(
	CONST CHAR *fmt, ...
);

VOID
KeBugCheck(
	struct InterruptRegisters* Reg
);