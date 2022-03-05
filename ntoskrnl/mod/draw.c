#include "module.h"
#include <lib/utill.h>
#include "../drv/drv.h"

VOID
DwmDrawFilled(
	UINT uiXStart,
	UINT uiXEnd,
	UINT uiYStart,
	UINT uiYEnd,
	UINT uiColor
)
{
	for ( INT iX = uiXStart; iX < uiXEnd; ++iX )
	{
		for ( INT iY = uiYStart; iY < uiYEnd; ++iY )
		{
			INT iOffset = iX + ( DwmScreenPitch / sizeof( UINT ) ) * iY;
			DwmAntiBuffer[ iOffset ] = uiColor;
		}
	}
}


VOID
DwmDisplayChar(
	CHAR  cChararcter,
	UINT  uiX,
	UINT  uiY,
	UINT  uiColor
)
{
	UCHAR ucDisplay;


	for ( INT iy = uiY; uiY + 16 > iy; iy++ )
	{
		for ( INT ix = uiX; uiX + 8 > ix; ix++ )
		{
			ucDisplay = KiDisplayFont[ ( INT )cChararcter * 16 + iy - uiY ];

			if ( ucDisplay & ( 1 << ( 7 - ( ix - uiX ) ) ) &&
				   ix >= 0 && iy >= 0 && ix < KiVBEData.Width && iy < KiVBEData.Height )
			{

				INT iOffset = ix + ( DwmScreenPitch / sizeof( UINT ) ) * iy;
				DwmAntiBuffer[ iOffset ] = uiColor;

			}

		}


	}



}

VOID
DwmDisplayString(
	CHAR* pcString,
	UINT  uiX,
	UINT  uiY,
	UINT  uiColor
)
{
	INT iX = uiX;
	for ( INT i = 0; RtlStringLength( pcString ) > i; i++ )
	{
		if ( pcString[ i ] == '\n' )
			uiY += 16;
		else if ( pcString[ i ] == '\r' )
			iX = uiX;
		else if ( iX < KiVBEData.Width )
		{
			DwmDisplayChar(
				pcString[ i ],
				iX,
				uiY,
				uiColor
			);

			iX += 8;
		}
	}
}
