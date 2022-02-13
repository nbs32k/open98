#include "module.h" 

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