#include "module.h"
#include "../drv/drv.h"
#include "../mm/mm.h"

#define MAX_WINDOWS 128
#define BACKGROUND_COLOR 0x00008080
#define TITLE_BAR_THICKNESS 22

typedef struct _KSYSTEM_WINDOW
{

	CHAR* Title;
	CHAR* Parent;
	UINT* Framebuffer;
	UINT  X;
	UINT  Y;
	UINT  Width;
	UINT  Height;
	UINT  WindowType;
	/*0 - Form
	  1 - Label
	  2 - Button
	  3 - Textbox
	  4 - Listview
	*/
	INT	  WindowID;


}KSYSTEM_WINDOW;

typedef struct _KELEMENT_LISTVIEW
{
	INT	  WindowID;
	CHAR* Text;
	BOOLEAN Pressed;
	INT COLOR;

}KELEMENT_LISTVIEW;

INT iLastWindow = 0;

INT iLastElementListView = 0;
KELEMENT_LISTVIEW* ElementListView;

KSYSTEM_WINDOW* Windows; // Swaggers
STATIC UINT*	DwmFramebuffer;
STATIC INT		DwmScreenWidth;
STATIC INT		DwmScreenHeight;
INT				DwmScreenPitch;

STATIC INT		DwmFramebufferSize;

UINT*			DwmAntiBuffer;

BOOLEAN			bRefreshBuffer = FALSE;

static char *dwm_strcpy( char *dest, const char *src )
{
	INT i;

	for ( i = 0; src[ i ]; i++ )
		dest[ i ] = src[ i ];
	dest[ i ] = 0;

	return dest;
}

INT
DwmGetWindowIDByParent(
	CHAR* Parent
)
{
	for ( int i = 0; i < iLastWindow; ++i )
	{

		if ( Windows[ i ].Title == Parent )
		{
			return Windows[ i ].WindowID ;
		}
	}
}

KSYSTEM_WINDOW
DwmAcquireInformation(
	INT WindowID
)
{
	for ( int i = 0; i < iLastWindow; ++i )
	{

		if ( Windows[ i ].WindowID == WindowID )
			return Windows[ i ];
	}

}
VOID
DwmPlotPixelOnWindow( 
	INT WindowID,
	INT X,
	INT Y,
	INT HEX
)
{

	
	X = X + Windows[ WindowID ].X + 3;
	Y = Y + Windows[ WindowID ].Y + TITLE_BAR_THICKNESS;

	INT newW = X + Windows[ WindowID ].Width;
	INT newH = Y + Windows[ WindowID ].Height;
	


	
	INT iOffset = X + ( DwmScreenPitch / sizeof( UINT ) ) * Y;
	DwmAntiBuffer[ iOffset ] = HEX;
}



VOID 
DwmDrawFilledWindow( 
	INT Parent,
	INT X,
	INT XEnd,
	INT Y,
	INT YEnd,
	INT COLOR
)
{
	//DbgPrintFmt( "%d, %d, %d, %d", X, Y, XEnd, YEnd );
			for ( int i = X; i < XEnd; i++ )
			{
				//DbgPrintFmt( "LOL i: %d", i );
				for ( int j = Y; j < YEnd; j++ )
				{
					//DbgPrintFmt( "LOL j: %d", j );
					DwmPlotPixelOnWindow( Parent, i, j, COLOR );
				}
			}
	
}

INT
DwmCreateObject(
	CHAR* Title,
	CHAR* Parent,
	UINT X,
	UINT Y,
	UINT Width,
	UINT Height,
	UINT WindowType
)
{

	if ( iLastWindow == MAX_WINDOWS ) return;

	




	Windows[ iLastWindow ].Title = Title;
	Windows[ iLastWindow ].Parent = Parent;
	Windows[ iLastWindow ].X = X;
	Windows[ iLastWindow ].Y = Y;
	Windows[ iLastWindow ].Width = Width;
	Windows[ iLastWindow ].Height = Height;
	Windows[ iLastWindow ].WindowType = WindowType;
	Windows[ iLastWindow ].WindowID = iLastWindow;

	/*if ( WindowType == 4 )
	{

		ElementListView[ iLastElementListView ].WindowID = iLastWindow;
		ElementListView[ iLastElementListView ].Pressed = FALSE;
		ElementListView[ iLastElementListView ].COLOR = 0x000000;

		iLastElementListView++;

	}*/

	DbgPrintFmt( "Created new window:\nTitle: %s\nX: %d\nY: %d\nWidth: %d\nHeight: %d\nWindowType: %d\nWindowID: %d",
				 Windows[ iLastWindow ].Title, Windows[ iLastWindow ].X, Windows[ iLastWindow ].Y, Windows[ iLastWindow ].Width, Windows[ iLastWindow ].Height, Windows[ iLastWindow ].WindowType, Windows[ iLastWindow ].WindowID );

	iLastWindow++;

	bRefreshBuffer = TRUE;
	return Windows[ iLastWindow ].WindowID;
}


VOID
DwmSetWindowPos( 
	INT WindowID,
	INT NewX,
	INT NewY
)
{


	//if ( NewX >= DwmScreenWidth || NewY >= DwmScreenHeight || NewX < 0 || NewY < 0 )
	//	return;

	if ( Windows[ WindowID ].Width == 0 )
		return;

	Windows[ WindowID ].Width = Windows[ WindowID ].Width + ( NewX - Windows[ WindowID ].X );
	Windows[ WindowID ].Height = Windows[ WindowID ].Height + ( NewY - Windows[ WindowID ].Y );
	Windows[ WindowID ].X = NewX;
	Windows[ WindowID ].Y = NewY;

	bRefreshBuffer = TRUE;
}

#include "../ntuser/ntuser.h"


INT
DwmAcquireCursorWindow(

)
{
	for ( INT i = 0; i < iLastWindow; ++i )
	{
		INT appX = Windows[ i ].X;
		INT appY = Windows[ i ].Y;
		INT appW = Windows[ i ].Width;
		INT appH = Windows[ i ].Height;
		

		if ( Windows[ i ].WindowType == 0 )//Form
		{
			if ( ( KiMousePosition.X > appX ) && ( KiMousePosition.X < appW ) )
				if ( ( KiMousePosition.Y > appY ) && ( KiMousePosition.Y < Windows[ i ].Y + TITLE_BAR_THICKNESS ) )
					return Windows[ i ].WindowID;
		}
		
	}
}


VOID
DwmInsertElementListView( 
	CHAR* Text,
	INT WindowID,
	INT COLOR,
	BOOLEAN Pressed
)
{
	
	ElementListView[ iLastElementListView ].WindowID = WindowID;
	ElementListView[ iLastElementListView ].Text = Text;
	ElementListView[ iLastElementListView ].COLOR = COLOR;
	ElementListView[ iLastElementListView ].Pressed = Pressed;

	iLastElementListView++;
}

VOID
DwmRefreshBuffer(

)
{
	if ( !bRefreshBuffer ) return;

	bRefreshBuffer = FALSE;

	//Set background
	for ( INT i = 0; i < DwmFramebufferSize; ++i )
		DwmAntiBuffer[ i ] = BACKGROUND_COLOR;


	//Draw Forms
	for ( INT tempI = 0; tempI < iLastWindow; ++tempI )
	{
		//DbgPrintFmt( "lol" );

		//If NULL, don't render!
			//DbgPrintFmt( "sex" );
			/*switch ( Window.WindowType )
			{
				case 0:
					break;
				case 1:
					break;
				case 2:
					break;
			}*/

			//render test
			/*DbgPrintFmt( "%d", Windows[ tempI ].X );
			for ( INT i = Windows[ tempI ].X; i < Windows[ tempI ].Width; ++i )
			{
				for ( INT j = Windows[ tempI ].Y; j < Windows[ tempI ].Height; ++j )
				{
					INT iOffset = i + ( DwmScreenPitch / sizeof( UINT ) ) * j;
					DwmAntiBuffer[ iOffset ] = 0xffffff;
				}
			}*/
		INT iStartX = Windows[ tempI ].X;
		INT iEndX = Windows[ tempI ].Width;
		INT iStartY = Windows[ tempI ].Y;
		INT iEndY = Windows[ tempI ].Height;
		switch ( Windows[ tempI ].WindowType )
		{
			case 0: //FORM

				DwmDrawFilled( iStartX, iEndX, iStartY, iEndY, 0x000000 );
				DwmDrawFilled( iStartX, iEndX - 1, iStartY, iEndY - 1, 0xDFDFDF );
				DwmDrawFilled( iStartX + 1, iEndX - 1, iStartY + 1, iEndY - 1, 0x808080 );
				DwmDrawFilled( iStartX + 1, iEndX - 2, iStartY + 1, iEndY - 2, 0xFFFFFF );
				DwmDrawFilled( iStartX + 2, iEndX - 2, iStartY + 2, iEndY - 2, 0xC0C0C0 );
				//TitlebAr
				DwmDrawFilled( iStartX + 3, iEndX - 3, iStartY + 3, iStartY + TITLE_BAR_THICKNESS - 1, 0x000080 );
				DwmDisplayString( Windows[ tempI ].Title, iStartX + 6, iStartY + 3, 0xFFFFFF );

				break;
			case 2:
				
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX, iEndX, iStartY, iEndY, 0x00000000 );
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 1, iEndX - 2, iStartY + 1, iEndY - 2, 0xFFFFFFFF );


				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 2, iEndX - 2, iStartY + 2, iEndY - 2, 0x808080 );
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 2, iEndX - 3, iStartY + 2, iEndY - 3, 0xC0C0C0 );

				iStartX = iStartX + Windows[ tempI ].X + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].X;
				iStartY = iStartY + Windows[ tempI ].Y + TITLE_BAR_THICKNESS + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].Y;

				DwmDisplayString( "OK", iStartX + 3, iStartY + 3, 0x00000000 );
				break;
			case 1:

				iStartX = iStartX + Windows[ tempI ].X + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].X;
				iStartY = iStartY + Windows[ tempI ].Y + TITLE_BAR_THICKNESS + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].Y;
				DwmDisplayString( Windows[ tempI ].Title, iStartX, iStartY, 0x00000000 );

				break;
			case 3:

				iEndY = iStartY + 20;
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX, iStartX + 1, iStartY,  iEndY, 0x808080 ); //0x808080
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 1, iStartX + 2, iStartY + 1,  iEndY, 0x000000 );
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX, iEndX - 1, iStartY,  iStartY + 1, 0x808080 ); //0x808080
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 1, iEndX - 1, iStartY + 1,  iStartY + 2, 0x000000 );
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iEndX - 1, iEndX, iStartY,  iEndY, 0xFFFFFF );
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iEndX - 2, iEndX - 1, iStartY + 1,  iEndY - 1, 0xC3C3C3 ); //0xC3C3C3
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX, iEndX, iEndY,  iEndY + 1, 0xFFFFFF );
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 2, iEndX - 1, iEndY - 1,  iEndY, 0xC3C3C3 );
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 2, iEndX - 2, iStartY + 2, iEndY - 1, 0xFFFFFF ); //0x808080
				

				iStartX = iStartX + Windows[ tempI ].X + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].X;
				iStartY = iStartY + Windows[ tempI ].Y + TITLE_BAR_THICKNESS + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].Y;
				DwmDisplayString( Windows[ tempI ].Title, iStartX + 4, iStartY + 3, 0x00000000 );
				break;

			case 4:

				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX, iStartX + 1, iStartY,  iEndY, 0x808080 ); //0x808080
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 1, iStartX + 2, iStartY + 1,  iEndY, 0x000000 );
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX, iEndX - 1, iStartY,  iStartY + 1, 0x808080 ); //0x808080
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 1, iEndX - 1, iStartY + 1,  iStartY + 2, 0x000000 );
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iEndX - 1, iEndX, iStartY, iEndY, 0xFFFFFF );
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iEndX - 2, iEndX - 1, iStartY + 1,  iEndY - 1, 0xC3C3C3 ); //0xC3C3C3
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX, iEndX, iEndY,  iEndY + 1, 0xFFFFFF );
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 2, iEndX - 1, iEndY - 1,  iEndY, 0xC3C3C3 );
				//
				DwmDrawFilledWindow( DwmGetWindowIDByParent( Windows[ tempI ].Parent ), iStartX + 2, iEndX - 2, iStartY + 2,  iEndY - 1, 0xFFFFFF ); //0x808080


				iStartX = iStartX + Windows[ tempI ].X + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].X;
				iStartY = iStartY + Windows[ tempI ].Y + TITLE_BAR_THICKNESS + Windows[ DwmGetWindowIDByParent( Windows[ tempI ].Parent ) ].Y;
				

				//TODO: fix this shit
				/*INT lastY = 16;
				for ( int i = 0; i < iLastElementListView; ++i )
				{
					DbgPrintFmt( "%d", ElementListView[ i ].WindowID );
					if ( ElementListView[ i ].WindowID == Windows[ tempI ].WindowID )
					{
						DbgPrintFmt( "ELEMENT!!: %s", ElementListView[ i ].Text );
						DwmDisplayString( ElementListView[ i ].Text, iStartX + 3, iStartY + lastY, 0x00000000 );
						lastY *= 16;
					}
					
					
				}*/
				break;
		}
			

			
			/*INT in_x = 1;
			INT in_y = TITLE_BAR_THICKNESS;
			for ( INT i = 0; i < ( Window.Width - Window.X ) * ( Window.Height - Window.Y ) * sizeof( UINT ); i++ )
			{
				in_x++;
				plot_px( in_x + Window.X, in_y + Window.Y, Window.Framebuffer[ i ] );
				if ( in_x - 1 == Window.Width - Window.X - 4 )
				{
					in_y++;
					in_x = 1;
				}

				if ( in_y == Window.Height - Window.Y - 3 )
					break;
			}*/




		

	}


	//Apply changes
	for ( INT i = 0; i < DwmFramebufferSize / sizeof( UINT ); ++i )
		DwmFramebuffer[ i ] = DwmAntiBuffer[ i ];

	POINT CursorPos;
	NtGetCursorPos( &CursorPos );
	NtSetCursorPos( CursorPos.X, CursorPos.Y );
}

BOOLEAN
DwmInitialize(

)
{
	DwmFramebuffer = ( UINT* )KiVBEData.Address; //cast
	DwmScreenWidth = KiVBEData.Width;
	DwmScreenHeight = KiVBEData.Height;
	DwmScreenPitch = KiVBEData.Pitch;

	Windows = ( KSYSTEM_WINDOW* )malloc( sizeof( KSYSTEM_WINDOW ) * MAX_WINDOWS );
	ElementListView = ( KELEMENT_LISTVIEW* )malloc( sizeof( KELEMENT_LISTVIEW ) * MAX_WINDOWS );
	for ( INT i = 0; i < iLastElementListView; ++i )
	{
		ElementListView[ i ].Text = malloc( 20 );//20 lenght
	}
	DwmFramebufferSize = ( DwmScreenPitch / sizeof( UINT ) ) * DwmScreenHeight * sizeof( UINT );

	DwmAntiBuffer = kcalloc( DwmFramebufferSize );

	if ( !DwmAntiBuffer ) return FALSE;


	bRefreshBuffer = TRUE;



	//INT WinID = DwmCreateObject( "test", "Disk Management", 2, 70, 250, 250, 4 );
	//DwmInsertElementListView( "noobes", WinID, 0x000000, 0  );
	//DwmPlotPixelOnWindowFramebuffer( 1, 10, 10, 0x000000);
	//DwmCreateObject( "tst", 280, 10, 300, 200, 0 );
}