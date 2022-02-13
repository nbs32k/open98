#pragma once

#include "../ntdef.h"
#include "gdt/gdt.h"
#include "io/io.h"
#include "idt/idt.h"

VOID
HalInitModule(

);

VOID
HalInitSSE(

);


//clock.c
VOID
KeQuerySystemTime(
	PKSYSTEM_TIME ClockTime
);

VOID KeInitializeKernelClock(

);


//pit.c
VOID
HalInitPIT(

);

VOID Sleep(
	LONG Seconds
);

//vbe.c
VOID
HalInitVBE(
	struct stivale2_struct *stivale2_struct
);