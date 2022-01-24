#pragma once

#include <stddef.h>
#include <stdint.h>
typedef enum _SHUTDOWN_ACTION
{
	ShutdownNoReboot,
	ShutdownReboot,
	ShutdownPowerOff
} SHUTDOWN_ACTION, *PSHUTDOWN_ACTION;

INT
NtShutdownSystem(
	SHUTDOWN_ACTION Action
);