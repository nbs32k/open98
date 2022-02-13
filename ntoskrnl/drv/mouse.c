#include "drv.h"
#include "../ntuser/ntuser.h"
#include "../hal/hal.h"

#include "../mod/module.h"

INT KiMouseState = 0;

UCHAR prev_button_state[ 3 ];
UCHAR curr_button_state[ 3 ];

int left_button_down( )
{
	return !prev_button_state[ 0 ] && curr_button_state[ 0 ];
}

int right_button_down( )
{
	return !prev_button_state[ 2 ] && curr_button_state[ 2 ];
}

int left_button_up( )
{
	return prev_button_state[ 0 ] && !curr_button_state[ 0 ];
}

int right_button_up( )
{
	return prev_button_state[ 2 ] && !curr_button_state[ 2 ];
}



VOID
KiWaitMouse(

)
{
	ULONG64 ulTimeout = 100000;
	while ( ulTimeout-- )
	{
		if ( ( inb( 0x64 ) & 0b10 ) == 0 )
		{
			return;
		}
	}
}

VOID
KiWaitMouseInput(

)
{
	ULONG64 ulTimeout = 100000;
	while ( ulTimeout-- )
	{
		if ( inb( 0x64 ) & 0b1 )
		{
			return;
		}
	}
}

VOID
KiWriteMouse(
	UCHAR ucvalue
)
{
	KiWaitMouse( );
	outb( 0x64, 0xD4 );
	KiWaitMouse( );
	outb( 0x60, ucvalue );
}

UCHAR
KiReadMouse(

)
{
	KiWaitMouseInput( );
	return inb( 0x60 );
}

UCHAR KiMouseCycle = 0;

BOOLEAN bMousePacketReady = FALSE;

VOID
KiHandlePS2Mouse(
	UCHAR ucData
)
{
	KiProcessMouse( );
	static BOOLEAN bSkip = TRUE;

	if ( bSkip )
	{
		bSkip = FALSE;
		return;
	}

	switch ( KiMouseCycle )
	{
		case 0:
			if ( bMousePacketReady )
				break;
			if ( ( ucData & 0b00001000 ) == 0 )
				break;
			KiMousePacket[ 0 ] = ucData;
			KiMouseCycle++;
			break;
		case 1:
			if ( bMousePacketReady )
				break;
			KiMousePacket[ 1 ] = ucData;
			KiMouseCycle++;
			break;
		case 2:
			if ( bMousePacketReady )
				break;
			KiMousePacket[ 2 ] = ucData;
			bMousePacketReady = TRUE;
			KiMouseCycle = 0;
			break;
	}
}



VOID
KiProcessMouse(

)
{
	if ( !bMousePacketReady )
		return;
	//if (isKeyboardUsed) return;


	BOOLEAN xNegative, yNegative, xOverflow, yOverflow;

	if ( KiMousePacket[ 0 ] & PS2XSign )
	{
		xNegative = TRUE;
	}
	else
		xNegative = FALSE;

	if ( KiMousePacket[ 0 ] & PS2YSign )
	{
		yNegative = TRUE;
	}
	else
		yNegative = FALSE;

	if ( KiMousePacket[ 0 ] & PS2XOverflow )
	{
		xOverflow = TRUE;
	}
	else
		xOverflow = FALSE;

	if ( KiMousePacket[ 0 ] & PS2YOverflow )
	{
		yOverflow = TRUE;
	}
	else
		yOverflow = FALSE;

	if ( !xNegative )
	{
		KiMousePosition.X += KiMousePacket[ 1 ];
		if ( xOverflow )
		{
			KiMousePosition.X += 255;
		}
	}
	else
	{
		KiMousePacket[ 1 ] = 256 - KiMousePacket[ 1 ];
		KiMousePosition.X -= KiMousePacket[ 1 ];
		if ( xOverflow )
		{
			KiMousePosition.X -= 255;
		}
	}

	if ( !yNegative )
	{
		KiMousePosition.Y -= KiMousePacket[ 2 ];
		if ( yOverflow )
		{
			KiMousePosition.Y -= 255;
		}
	}
	else
	{
		KiMousePacket[ 2 ] = 256 - KiMousePacket[ 2 ];
		KiMousePosition.Y += KiMousePacket[ 2 ];
		if ( yOverflow )
		{
			KiMousePosition.Y += 255;
		}
	}

	if ( KiMousePosition.X < 0 )
		KiMousePosition.X = 0;
	if ( KiMousePosition.X > KiVBEData.Width - 1 )
		KiMousePosition.X = KiVBEData.Width - 1;

	if ( KiMousePosition.Y < 0 )
		KiMousePosition.Y = 0;
	if ( KiMousePosition.Y > KiVBEData.Height - 1 )
		KiMousePosition.Y = KiVBEData.Height - 1;




	//ClearMouseCursor(MousePointer, pMousePositionOld);

	//DrawOverlayMouseCursor(MousePointer, KiMousePosition, 0xffffff);

	if ( KiMousePacket[ 0 ] & PS2Leftbutton )
	{
		//PutPix(KiMousePosition.X, KiMousePosition.Y, 0x0000ff00);
		//GlobalRenderer->PutChar('a', KiMousePosition.X, KiMousePosition.Y);
		KiMouseState = MOUSE_LEFT_CLICK;

		//curr_button_state[ 0 ] = 1;
		
		//dragging
		//DbgPrintFmt("%d", DwmGetWindowUnderCursor( ) );

		//DraggedWindowID = DwmGetWindowUnderCursor( );
			//DwmSetObjectPosition( WinID, lolX, lolY );
		
	}
	else
	{
		
		//curr_button_state[ 0 ] = 0;
	}
	if ( KiMousePacket[ 0 ] & PS2Middlebutton )
	{
		KiMouseState = MOUSE_MIDDLE_CLICK;
	}
	if ( KiMousePacket[ 0 ] & PS2Rightbutton )
	{
		KiMouseState = MOUSE_RIGHT_CLICK;

		curr_button_state[ 2 ] = 1;
	}
	else
	{
		curr_button_state[ 2 ] = 0;
	}


	////////////////////////////
	///////////////////////////
	/////////////////////////

	
	NtSetCursorPos( KiMousePosition.X, KiMousePosition.Y );

	KiMouseState = 0; // reset mouse state
	

	bMousePacketReady = FALSE;
	pMousePositionOld = KiMousePosition;

}


VOID
KiUpdateMouse(
	struct InterruptRegisters *reg
)
{

	UCHAR ucData = inb( 0x60 );
	
	KiHandlePS2Mouse( ucData );
}


VOID
KiInitMouse(

)
{
	DbgPrintFmt( "dawg hard" );

	outb( 0x64, 0xA8 ); //enabling the auxiliary device - mouse

	KiWaitMouse( );
	outb( 0x64, 0x20 ); //tells the keyboard controller that we want to send a command to the mouse
	KiWaitMouseInput( );
	UCHAR ucStatus = inb( 0x60 );
	ucStatus |= 0b10;
	KiWaitMouse( );
	outb( 0x64, 0x60 );
	KiWaitMouse( );
	outb( 0x60, ucStatus ); // setting the correct bit is the "compaq" status byte

	KiWriteMouse( 0xF6 );
	KiReadMouse( );

	KiWriteMouse( 0xF4 );
	KiReadMouse( );


	HalRegisterInterrupt(
		IRQ12,
		KiUpdateMouse
	); //init
}
