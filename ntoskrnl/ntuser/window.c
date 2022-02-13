#include "ntuser.h"
#include "../mod/module.h"

//INT
//NtFindWindow(
//	PCHAR	lpClassName,
//	PCHAR	lpWindowName
//)
//{
//	for ( int i = 0; i < MAX_WINDOWS; ++i )
//	{
//		if ( WindowStructures[ i ].ElementType == lpClassName &&
//			 WindowStructures[ i ].Title == lpWindowName )
//			return WindowStructures[ i ].WindowID;
//	}
//}
//
//BOOLEAN
//NtDestroyWindow(
//  INT WindowID
//)
//{
//	DwmDestroyObject( WindowID );
//}
//
//
//INT
//NtCreateWindow(
//	PCHAR	lpClassName,
//	PCHAR	lpWindowName,
//	ULONG32	dwStyle,
//	INT		x,
//	INT		y,
//	INT		nWidth,
//	INT		nHeight,
//	HWND	hWndParent,
//	ULONG	hMenu,
//	ULONG	hInstance,
//	PVOID	lpParam
//)
//{
//
//	return DwmCreateObject(
//		lpClassName,
//		lpWindowName,
//		x,
//		nWidth,
//		y,
//		nHeight
//	);
//
//}