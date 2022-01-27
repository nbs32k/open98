#include "ntuser.h"
#include "../drv/drv.h"

POINT KiMousePosition = { 0 };
POINT pMousePositionOld = { 0 };


UCHAR
ucMousePointer[ 16 * 24 ];

UCHAR
ucMousePointerInside[ 16 * 24 ];

BOOLEAN bMouseDown;


UINT MouseCursorBuffer[ 16 * 24 ];
UINT MouseCursorBufferAfter[ 16 * 24 ];


VOID KiDrawCursor(
	UCHAR ucCursor[ ],
	POINT pPosition,
	BOOLEAN bBack,
	INT iColor
)
{

	INT xMax = 16;
	INT yMax = 24;
	INT differenceX = KiVBEData.Width - pPosition.X;
	INT differenceY = KiVBEData.Height - pPosition.Y;

	if ( differenceX < 16 ) xMax = differenceX;
	if ( differenceY < 24 ) yMax = differenceY;

	for ( INT y = 0; y < yMax; y++ )
	{
		for ( INT x = 0; x < xMax; x++ )
		{
			INT bit = y * 16 + x;
			INT byte = bit / 8;
			if ( ( ucCursor[ byte ] & ( 0b10000000 >> ( x % 8 ) ) ) )
			{
				if ( bBack ) MouseCursorBuffer[ x + y * 16 ] = KiReadPixel( pPosition.X + x, pPosition.Y + y );
				KiDrawPixel( pPosition.X + x, pPosition.Y + y, iColor );

				if ( bBack )
					MouseCursorBufferAfter[ x + y * 16 ] = iColor;

			}

		}
	}
	bMouseDown = TRUE;
}

VOID 
KiClearCursor(
	POINT position
)
{
	if ( !bMouseDown ) return;

	INT xMax = 16;
	INT yMax = 24;
	INT differenceX = KiVBEData.Width - position.X;
	INT differenceY = KiVBEData.Height - position.Y;

	if ( differenceX < 16 ) xMax = differenceX;
	if ( differenceY < 24 ) yMax = differenceY;

	for ( INT y = 0; y < yMax; y++ )
	{
		for ( INT x = 0; x < xMax; x++ )
		{
			INT bit = y * 16 + x;
			INT byte = bit / 8;
			if ( ( ucMousePointerInside[ byte ] & ( 0b10000000 >> ( x % 8 ) ) ) )
			{

				KiDrawPixel( position.X + x, position.Y + y, MouseCursorBuffer[ x + y * 16 ] );

			}
		}
	}

}

VOID
NtSetCursorPos(
	INT X,
	INT Y
)
{
	POINT position;
	position.X = ( LONG )X;
	position.Y = ( LONG )Y;

	KiMousePosition = position;

	KiClearCursor( pMousePositionOld );

	KiDrawCursor( ucMousePointerInside, position, TRUE, 0xffffff );
	KiDrawCursor( ucMousePointer, position, FALSE, 0x000000 );


	pMousePositionOld = KiMousePosition;
}

VOID
NtGetCursorPos(
	POINT* lpPoint
)
{
	*lpPoint = KiMousePosition;
}


UCHAR
ucMousePointer[ 16 * 24 ] =
{
	0b10000000, 0b00000000,
	0b11000000, 0b00000000,
	0b10100000, 0b00000000,
	0b10010000, 0b00000000,
	0b10001000, 0b00000000,
	0b10000100, 0b00000000,
	0b10000010, 0b00000000,
	0b10000001, 0b00000000,
	0b10000000, 0b10000000,
	0b10000000, 0b01000000,
	0b10000000, 0b00100000,
	0b10000001, 0b11110000,
	0b10001001, 0b00000000,
	0b10011001, 0b00000000,
	0b10100100, 0b10000000,
	0b11000100, 0b10000000,
	0b10000010, 0b01000000,
	0b00000010, 0b01000000,
	0b00000001, 0b00100000,
	0b00000001, 0b00100000,
	0b00000000, 0b11000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
};

UCHAR
ucMousePointerInside[ 16 * 24 ] =
{
	0b10000000, 0b00000000,
	0b11000000, 0b00000000,
	0b11100000, 0b00000000,
	0b11110000, 0b00000000,
	0b11111000, 0b00000000,
	0b11111100, 0b00000000,
	0b11111110, 0b00000000,
	0b11111111, 0b00000000,
	0b11111111, 0b10000000,
	0b11111111, 0b11000000,
	0b11111111, 0b11100000,
	0b11111111, 0b11110000,
	0b11111111, 0b00000000,
	0b11111111, 0b00000000,
	0b11100111, 0b10000000,
	0b11000111, 0b10000000,
	0b10000011, 0b11000000,
	0b00000011, 0b11000000,
	0b00000001, 0b11100000,
	0b00000001, 0b11100000,
	0b00000000, 0b11000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
};
