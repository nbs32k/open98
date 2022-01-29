#include "../windef.h"

typedef struct _POINT
{
	INT X;
	INT Y;
} POINT;

POINT KiMousePosition;
POINT pMousePositionOld;

VOID
NtSetCursorPos(
	INT X,
	INT Y
);