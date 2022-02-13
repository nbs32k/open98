#include "../ntdef.h"

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

VOID
NtGetCursorPos(
	POINT* lpPoint
);

typedef ULONG64		*HWND;

INT
NtCreateWindow(
	PCHAR	lpClassName,
	PCHAR	lpWindowName,
	ULONG32	dwStyle,
	INT		x,
	INT		y,
	INT		nWidth,
	INT		nHeight,
	HWND	hWndParent,
	ULONG	hMenu,
	ULONG	hInstance,
	PVOID	lpParam
);