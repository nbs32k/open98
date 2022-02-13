#pragma once
#include <ntdef.h>


BOOLEAN
DwmInitialize(

);

VOID
DwmRefreshBuffer(

);

UINT*			DwmAntiBuffer;
INT				DwmScreenPitch;

VOID
DwmDrawFilled(
	UINT uiXStart,
	UINT uiXEnd,
	UINT uiYStart,
	UINT uiYEnd,
	UINT uiColor
);