#pragma once
#include <ntdef.h>
#include <stddef.h>


BOOLEAN
DwmInitialize(

);

VOID
DwmRefreshBuffer(

);

VOID
DwmDrawFilled(
	UINT uiXStart,
	UINT uiXEnd,
	UINT uiYStart,
	UINT uiYEnd,
	UINT uiColor
);
INT				DwmScreenPitch;
UINT*			DwmAntiBuffer;

VOID
DwmSetWindowPos(
	INT WindowID,
	INT NewX,
	INT NewY
);

INT
DwmAcquireCursorWindow(

);

VOID
DwmDisplayString(
	CHAR* pcString,
	UINT  uiX,
	UINT  uiY,
	UINT  uiColor
);

BOOLEAN			bRefreshBuffer;