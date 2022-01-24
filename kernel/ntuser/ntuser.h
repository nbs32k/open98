#include "../windef.h"

typedef struct _POINT
{
	FLOAT X;
	FLOAT Y;
} POINT;

POINT KiMousePosition;
POINT pMousePositionOld;

VOID
NtSetCursorPos(
	INT X,
	INT Y
);