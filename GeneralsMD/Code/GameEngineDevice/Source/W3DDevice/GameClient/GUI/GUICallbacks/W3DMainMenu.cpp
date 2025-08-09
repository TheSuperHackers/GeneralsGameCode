/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DMainMenu.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Apr 2002
//
//	Filename: 	W3DMainMenu.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	The Draw Routine for the main menu
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <time.h>
//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameClient/GameWindow.h"
#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "GameClient/Display.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/Shell.h"
#include "GameClient/ShellMenuScheme.h"
#include "GameClient/Credits.h"

#include "GameClient/Gadget.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetPushButton.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DGadget.h"

#include "GameClient/GUICallbacks.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

void drawText( GameWindow *window, WinInstanceData *instData );
static Color BrownishColor = GameMakeColor(167,134,94,255);
static IRegion2D clipRegion;
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

static void advancePosition(GameWindow *window, const Image *image, UnsignedInt posX, UnsignedInt posY, UnsignedInt sizeX, UnsignedInt sizeY)
{
	if(!image)
		return;

	static Bool goingForward = TRUE;

	ICoord2D pos, size;
	if(!window)
	{
		pos.x = posX;
		pos.y = posY;
		size.x = sizeX;
		size.y = sizeY;
	}
	else
	{
		window->winGetScreenPosition(&pos.x, &pos.y);
		window->winGetSize(&size.x,&size.y);
	}
	static Int Width = size.x + image->getImageWidth();

	static Int x = -800;
	static Int y = pos.y - (image->getImageHeight()/2);

	static UnsignedInt m_startTime = timeGetTime();
	Int time = timeGetTime() - m_startTime;
	Real percentDone = INT_TO_REAL(time) / 10000;

	if(goingForward)
	{
		if(percentDone >= 1)
		{
			y = pos.y + size.y - (image->getImageHeight()/2);
			m_startTime = timeGetTime();
			goingForward = FALSE;
		}
		else
		{
			y = pos.y - (image->getImageHeight()/2);
			x = (percentDone * Width) - image->getImageWidth();
		}
	}
	else
	{
		if(percentDone >= 1)
		{
			y = pos.y - (image->getImageHeight()/2);
			m_startTime = timeGetTime();
			goingForward = TRUE;
		}
		else
		{
			y = pos.y + size.y - (image->getImageHeight()/2);
			x = size.x - (percentDone * Width);
		}
	}
	TheDisplay->drawImage(image,x, y, x + image->getImageWidth(), y + image->getImageHeight());

}

void W3DShellMenuSchemeDraw( GameWindow *window, WinInstanceData *instData )
{
	if(TheShell && TheShell->isShellActive())
		TheShell->getShellMenuSchemeManager()->draw();
}

void W3DMainMenuDraw( GameWindow *window, WinInstanceData *instData )
{
	//W3DGameWinDefaultDraw( window, instData );

	//R:83 G:78 B:52

	static UnsignedInt color = BrownishColor;
	static UnsignedInt colorDrop = GameMakeColor(38,30,21,255);
	ICoord2D pos, size;
	Int height = TheDisplay->getHeight();

	window->winGetScreenPosition(&pos.x, &pos.y);
	window->winGetSize(&size.x,&size.y);



	IRegion2D	topHorizontal1 ={pos.x, pos.y, pos.x + size.x, pos.y	};
	IRegion2D	topHorizontal1drop ={pos.x, pos.y+1, pos.x + size.x, pos.y+1	};

	IRegion2D	topHorizontal2 ={pos.x, pos.y + Int(size.y * .1) , pos.x + size.x, pos.y + Int(size.y * .1)	};
	IRegion2D	topHorizontal2drop ={pos.x, pos.y + Int(size.y * .12) , pos.x + size.x, pos.y + Int(size.y * .12)	};

	IRegion2D	bottomHorizontal1={pos.x, pos.y + Int(size.y * .9), pos.x + size.x, pos.y + Int(size.y * .9)	};
	IRegion2D	bottomHorizontal1drop={pos.x, pos.y + Int(size.y * .92), pos.x + size.x, pos.y + Int(size.y * .92)	};

	IRegion2D	bottomHorizontal2= {pos.x, pos.y + size.y, pos.x + size.x, pos.y + size.y 	};
	IRegion2D	bottomHorizontal2drop= {pos.x, pos.y + size.y + 1, pos.x + size.x, pos.y + size.y + 1	};

	IRegion2D	verticle1 ={pos.x + Int(size.x * .225), pos.y , pos.x + Int(size.x * .225), height 	};
	IRegion2D	verticle2 ={pos.x + Int(size.x * .445), pos.y, pos.x + Int(size.x * .445), height 	};
	IRegion2D	verticle3 ={pos.x + Int(size.x * .6662), pos.y, pos.x + Int(size.x * .6662), height 	};
	IRegion2D	verticle4 ={pos.x + Int(size.x * .885), pos.y , pos.x + Int(size.x * .885), height 	};
//	static IRegion2D	verticle5 ={pos.x + (size.x * .7250), pos.y + (size.y * .12), pos.x + (size.x * .7250), pos.y + (size.y * .86) 	};
//	static IRegion2D	verticle6 ={pos.x + (size.x * .9062), pos.y + (size.y * .12), pos.x + (size.x * .9062), pos.y + (size.y * .86) 	};


	TheDisplay->drawLine(topHorizontal1.lo.x,topHorizontal1.lo.y,topHorizontal1.hi.x,topHorizontal1.hi.y,2,color);
	TheDisplay->drawLine(topHorizontal1drop.lo.x,topHorizontal1drop.lo.y,topHorizontal1drop.hi.x,topHorizontal1drop.hi.y,2,colorDrop);
	TheDisplay->drawLine(topHorizontal2.lo.x,topHorizontal2.lo.y,topHorizontal2.hi.x,topHorizontal2.hi.y,1,color);
	TheDisplay->drawLine(topHorizontal2drop.lo.x,topHorizontal2drop.lo.y,topHorizontal2drop.hi.x,topHorizontal2drop.hi.y,1,colorDrop);
	TheDisplay->drawLine(bottomHorizontal1.lo.x,bottomHorizontal1.lo.y,bottomHorizontal1.hi.x,bottomHorizontal1.hi.y,1,color);
	TheDisplay->drawLine(bottomHorizontal1drop.lo.x,bottomHorizontal1drop.lo.y,bottomHorizontal1drop.hi.x,bottomHorizontal1drop.hi.y,1,colorDrop);
	TheDisplay->drawLine(bottomHorizontal2.lo.x,bottomHorizontal2.lo.y,bottomHorizontal2.hi.x,bottomHorizontal2.hi.y,2,color);
	TheDisplay->drawLine(bottomHorizontal2drop.lo.x,bottomHorizontal2drop.lo.y,bottomHorizontal2drop.hi.x,bottomHorizontal2drop.hi.y,2,colorDrop);



	TheDisplay->drawLine(verticle1.lo.x,verticle1.lo.y,verticle1.hi.x,verticle1.hi.y,3,color);
	TheDisplay->drawLine(verticle2.lo.x,verticle2.lo.y,verticle2.hi.x,verticle2.hi.y,3,color);
	TheDisplay->drawLine(verticle3.lo.x,verticle3.lo.y,verticle3.hi.x,verticle3.hi.y,3,color);
	TheDisplay->drawLine(verticle4.lo.x,verticle4.lo.y,verticle4.hi.x,verticle4.hi.y,3,color);
//	TheDisplay->drawLine(verticle5.lo.x,verticle5.lo.y,verticle5.hi.x,verticle5.hi.y,3,color);
//	TheDisplay->drawLine(verticle6.lo.x,verticle6.lo.y,verticle6.hi.x,verticle6.hi.y,3,color);
//	TheDisplay->drawLine(m_rightLineFromButton.lo.x,m_rightLineFromButton.lo.y,m_rightLineFromButton.hi.x,m_rightLineFromButton.hi.y,3,color1,color2);




	advancePosition(NULL, TheMappedImageCollection->findImageByName("MainMenuPulse"),pos.x,pos.y,size.x, size.y);

	//TheDisplay->drawLine();

}

void W3DMainMenuFourDraw( GameWindow *window, WinInstanceData *instData )
{
	//W3DGameWinDefaultDraw( window, instData );

	//R:83 G:78 B:52

	static UnsignedInt color = BrownishColor;
	static UnsignedInt colorDrop = GameMakeColor(38,30,21,255);
	ICoord2D pos, size;
	Int height = TheDisplay->getHeight();

	window->winGetScreenPosition(&pos.x, &pos.y);
	window->winGetSize(&size.x,&size.y);



	IRegion2D	topHorizontal1 ={pos.x, pos.y, pos.x + size.x, pos.y	};
	IRegion2D	topHorizontal1drop ={pos.x, pos.y+1, pos.x + size.x, pos.y+1	};

	IRegion2D	topHorizontal2 ={pos.x, pos.y + Int(size.y * .1) , pos.x + size.x, pos.y + Int(size.y * .1)	};
	IRegion2D	topHorizontal2drop ={pos.x, pos.y + Int(size.y * .12) , pos.x + size.x, pos.y + Int(size.y * .12)	};

	IRegion2D	bottomHorizontal1={pos.x, pos.y + Int(size.y * .9), pos.x + size.x, pos.y + Int(size.y * .9)	};
	IRegion2D	bottomHorizontal1drop={pos.x, pos.y + Int(size.y * .92), pos.x + size.x, pos.y + Int(size.y * .92)	};

	IRegion2D	bottomHorizontal2= {pos.x, pos.y + size.y, pos.x + size.x, pos.y + size.y 	};
	IRegion2D	bottomHorizontal2drop= {pos.x, pos.y + size.y + 1, pos.x + size.x, pos.y + size.y + 1	};

	IRegion2D	verticle1 ={pos.x + Int(size.x * .295), pos.y , pos.x + Int(size.x * .295), height 	};
	IRegion2D	verticle2 ={pos.x + Int(size.x * .59), pos.y, pos.x + Int(size.x * .59), height 	};
	//IRegion2D	verticle3 ={pos.x + (size.x * .6662), pos.y, pos.x + (size.x * .6662), height 	};
	IRegion2D	verticle4 ={pos.x + Int(size.x * .885), pos.y , pos.x + Int(size.x * .885), height 	};
//	static IRegion2D	verticle5 ={pos.x + (size.x * .7250), pos.y + (size.y * .12), pos.x + (size.x * .7250), pos.y + (size.y * .86) 	};
//	static IRegion2D	verticle6 ={pos.x + (size.x * .9062), pos.y + (size.y * .12), pos.x + (size.x * .9062), pos.y + (size.y * .86) 	};


	TheDisplay->drawLine(topHorizontal1.lo.x,topHorizontal1.lo.y,topHorizontal1.hi.x,topHorizontal1.hi.y,2,color);
	TheDisplay->drawLine(topHorizontal1drop.lo.x,topHorizontal1drop.lo.y,topHorizontal1drop.hi.x,topHorizontal1drop.hi.y,2,colorDrop);
	TheDisplay->drawLine(topHorizontal2.lo.x,topHorizontal2.lo.y,topHorizontal2.hi.x,topHorizontal2.hi.y,1,color);
	TheDisplay->drawLine(topHorizontal2drop.lo.x,topHorizontal2drop.lo.y,topHorizontal2drop.hi.x,topHorizontal2drop.hi.y,1,colorDrop);
	TheDisplay->drawLine(bottomHorizontal1.lo.x,bottomHorizontal1.lo.y,bottomHorizontal1.hi.x,bottomHorizontal1.hi.y,1,color);
	TheDisplay->drawLine(bottomHorizontal1drop.lo.x,bottomHorizontal1drop.lo.y,bottomHorizontal1drop.hi.x,bottomHorizontal1drop.hi.y,1,colorDrop);
	TheDisplay->drawLine(bottomHorizontal2.lo.x,bottomHorizontal2.lo.y,bottomHorizontal2.hi.x,bottomHorizontal2.hi.y,2,color);
	TheDisplay->drawLine(bottomHorizontal2drop.lo.x,bottomHorizontal2drop.lo.y,bottomHorizontal2drop.hi.x,bottomHorizontal2drop.hi.y,2,colorDrop);



	TheDisplay->drawLine(verticle1.lo.x,verticle1.lo.y,verticle1.hi.x,verticle1.hi.y,3,color);
	TheDisplay->drawLine(verticle2.lo.x,verticle2.lo.y,verticle2.hi.x,verticle2.hi.y,3,color);
	//TheDisplay->drawLine(verticle3.lo.x,verticle3.lo.y,verticle3.hi.x,verticle3.hi.y,3,color);
	TheDisplay->drawLine(verticle4.lo.x,verticle4.lo.y,verticle4.hi.x,verticle4.hi.y,3,color);
//	TheDisplay->drawLine(verticle5.lo.x,verticle5.lo.y,verticle5.hi.x,verticle5.hi.y,3,color);
//	TheDisplay->drawLine(verticle6.lo.x,verticle6.lo.y,verticle6.hi.x,verticle6.hi.y,3,color);
//	TheDisplay->drawLine(m_rightLineFromButton.lo.x,m_rightLineFromButton.lo.y,m_rightLineFromButton.hi.x,m_rightLineFromButton.hi.y,3,color1,color2);




	advancePosition(NULL, TheMappedImageCollection->findImageByName("MainMenuPulse"),pos.x,pos.y,size.x, size.y);

	//TheDisplay->drawLine();

}


void W3DMetalBarMenuDraw( GameWindow *window, WinInstanceData *instData )
{
	//ICoord2D original, size;
//	TheDisplay->setClipRegion(&clipRegion);
	window->winDrawBorder();
	//window->winGetScreenPosition(&original.x, &original.y);
	//window->winGetSize(&size.x, &size.y);
	//blitBorderRect( original.x, original.y, size.x, size.y );
	//W3DGameWinDefaultDraw( window, instData );
//	TheDisplay->enableClipping(FALSE);
}


	//W3DGameWinDefaultDraw( window, instData );
//
//	//R:83 G:78 B:52
//
//
////	UnsignedInt color = GameMakeColor(113,108,82,212);
//	ICoord2D pos, size;
//
//	window->winGetScreenPosition(&pos.x, &pos.y);
//	window->winGetSize(&size.x,&size.y);
//
//	const Image *image = TheMappedImageCollection->findImageByName("LogoGlow");
//
//	if(!image)
//		return;
//
//	Int Width = size.x + image->getImageWidth();
//
//	static Int x = pos.x - image->getImageWidth();
//	static Int y = pos.y - (image->getImageHeight()/2);
//
//	static UnsignedInt m_startTime = timeGetTime();
//	Int time = timeGetTime() - m_startTime;
//	Real percentDone = INT_TO_REAL(time) / 15624;
//
//	if(percentDone >= 1)
//	{
////			y = pos.y + size.y - (image->getImageHeight()/2) - 2;
//			m_startTime = timeGetTime();
//	}
//		else
//		{
//			x = (percentDone * Width) - image->getImageWidth();
//		}
//
//	IRegion2D clip;
//	clip.lo.x = pos.x;
//	clip.lo.y = pos.y - image->getImageHeight();
//	clip.hi.x = pos.x + size.x;
//	clip.hi.y = pos.y + size.y;
//	TheDisplay->setClipRegion(&clip);
//	Int alpha;
//	if(percentDone > .5)
//		alpha = ((1-percentDone) * 2) * 255;
//	else
//		alpha = ( percentDone * 2) * 255;
//
//	TheDisplay->drawImage(image,x, y , x + image->getImageWidth(), y + image->getImageHeight(), GameMakeColor(255,255,255,alpha));
//	//TheDisplay->drawImage(image,x, y , x + image->getImageWidth(), y + image->getImageHeight(), GameMakeColor(255,200,250,alpha));
//	TheDisplay->enableClipping(FALSE );
//	//TheDisplay->drawLine();
//


void W3DClockDraw( GameWindow *window, WinInstanceData *instData )
{
	W3DGameWinDefaultDraw( window, instData );
	ICoord2D pos, size;

	window->winGetScreenPosition(&pos.x, &pos.y);
	window->winGetSize(&size.x,&size.y);

	char datestr[256] = "";
	time_t longTime;
	struct tm *curtime;
	time(&longTime);
	curtime = localtime(&longTime);
	strftime(datestr, 256, "%H:%M:%S", curtime);
	UnicodeString temp;
	temp.translate(datestr);
	instData->setText(temp);

	DisplayString *dString;
	dString = instData->getTextDisplayString();


	dString->setFont(TheFontLibrary->getFont("Arial",16,0));

	Int textWidth, textHeight;
	ICoord2D  textPos;
	IRegion2D clockClipRegion;
	// sanity

	// how much space will this text take up
	dString->getSize( &textWidth, &textHeight );

	//Init the clip region
	clockClipRegion.lo.x = pos.x + 1;
	clockClipRegion.lo.y = pos.y + 1;
	clockClipRegion.hi.x = pos.x + size.x - 1;
	clockClipRegion.hi.y = pos.y + size.y - 1;

	textPos.x = pos.x + (size.x / 2) - (textWidth / 2);
	textPos.y = pos.y + (size.y / 2) - (textHeight / 2);
	dString->setClipRegion(&clockClipRegion);
	dString->draw( textPos.x, textPos.y, GameMakeColor(255,255,255,255), GameMakeColor(0,0,0,255) );



}

void W3DMainMenuMapBorder( GameWindow *window, WinInstanceData *instData )
{
	enum
	{
		BORDER_CORNER_SIZE	= 10,
		BORDER_LINE_SIZE		= 20,
	};
	//( Int x, Int y, Int width, Int height )

// save original x, y
	Int x, y, width, height;
	window->winGetScreenPosition(&x, &y);
	window->winGetSize(&width,&height);

//	TheDisplay->setClipRegion(&clipRegion);
	Int originalX = x;
	Int originalY = y;
	Int maxX = x + width;
	Int maxY = y + height;
	Int x2, y2;			// used for simultaneous drawing of line pairs
	Int size = 20;
	Int halfSize = size / 2;

	const Image *image = NULL;

	// Draw Horizontal Lines
	// All border pieces are based on a 10 pixel offset from the centerline
	y = originalY - 10;
	y2 = maxY - 10;
	x2 = maxX - (10 + BORDER_LINE_SIZE);

	image = TheMappedImageCollection->findImageByName("FrameCornerHorizontal");
	if(image)
	{

		for( x=(originalX + 10); x <= x2; x += BORDER_LINE_SIZE )
		{

			TheDisplay->drawImage( image,
														 x, y, x + size, y + size );
			TheDisplay->drawImage( image,
														 x, y2, x + size, y2 + size );

		}


		x2 = maxX - BORDER_CORNER_SIZE;

		// x == place to draw remainder if any
		if( (x2 - x) >= (BORDER_LINE_SIZE / 2) )
		{

			//Blit Half piece
			TheDisplay->drawImage( image,
														 x, y, x + halfSize, y + size );
			TheDisplay->drawImage( image,
														 x, y2, x + halfSize, y2 + size );

			x += (BORDER_LINE_SIZE / 2);

		}


		// x2 - x ... must now be less than a half piece
		// check for equals and if not blit an adjusted half piece border pieces have
		// a two pixel repeat so we will blit one pixel over if necessary to line up
		// the art, but we'll cover-up the overlap with the corners
		if( x < x2 )
		{
			x -= ((BORDER_LINE_SIZE / 2) - (((x2 - x) + 1) & ~1));

			//Blit Half piece
			TheDisplay->drawImage( image,
														 x, y, x + halfSize, y + size );
			TheDisplay->drawImage( image,
														 x, y2, x + halfSize, y2 + size );

		}
	}
	image = TheMappedImageCollection->findImageByName("FrameCornerVertical");

	if( image )
	{

		// Draw Vertical Lines
		// All border pieces are based on a 10 pixel offset from the centerline
		x = originalX - 10;
		x2 = maxX - 10;
		y2 = maxY - (10 + BORDER_LINE_SIZE);

		for( y=(originalY + 10); y <= y2; y += BORDER_LINE_SIZE )
		{

			TheDisplay->drawImage( image,
														 x, y, x + size, y + size );
			TheDisplay->drawImage( image,
														 x2, y, x2 + size, y + size );

		}

		y2 = maxY - BORDER_CORNER_SIZE;

		// y == place to draw remainder if any
		if( (y2 - y) >= (BORDER_LINE_SIZE / 2) )
		{

			//Blit Half piece
			TheDisplay->drawImage( image,
														 x, y, x + size, y + halfSize );
			TheDisplay->drawImage( image,
														 x2, y, x2 + size, y + halfSize );

			y += (BORDER_LINE_SIZE / 2);
		}

		// y2 - y ... must now be less than a half piece
		// check for equals and if not blit an adjusted half piece border pieces have
		// a two pixel repeat so we will blit one pixel over if necessary to line up
		// the art, but we'll cover-up the overlap with the corners
		if( y < y2 )
		{
			y -= ((BORDER_LINE_SIZE / 2) - (((y2 - y) + 1) & ~1));

			//Blit Half piece
			TheDisplay->drawImage( image,
														 x, y, x + size, y + halfSize );
			TheDisplay->drawImage( image,
														 x2, y, x2 + size, y + halfSize );

		}
	}

	// Draw Corners
	x = originalX - BORDER_CORNER_SIZE;
	y = originalY - BORDER_CORNER_SIZE;
	TheDisplay->drawImage( TheMappedImageCollection->findImageByName("FrameCornerUL"),
												 x, y, x + size, y + size );
	x = maxX - BORDER_CORNER_SIZE;
	y = originalY - BORDER_CORNER_SIZE;
	TheDisplay->drawImage( TheMappedImageCollection->findImageByName("FrameCornerUR"),
												 x, y, x + size, y + size );
	x = originalX - BORDER_CORNER_SIZE;
	y = maxY - BORDER_CORNER_SIZE;
	TheDisplay->drawImage( TheMappedImageCollection->findImageByName("FrameCornerLL"),
												 x, y, x + size, y + size );
	x = maxX - BORDER_CORNER_SIZE;
	y = maxY - BORDER_CORNER_SIZE;
	TheDisplay->drawImage(TheMappedImageCollection->findImageByName("FrameCornerLR"),
												 x, y, x + size, y + size );

	TheDisplay->enableClipping(FALSE);

}


//Specialized drawing function for the buttons with the weird drop shadow
// W3DMainMenuButtonDropShadowDraw ===============================================
/** Draw pushbutton with user supplied images */
//=============================================================================
void W3DMainMenuButtonDropShadowDraw( GameWindow *window,
																	 WinInstanceData *instData )
{
	const Image *leftImage, *rightImage, *centerImage;
	ICoord2D origin, size, start, end;
	Int xOffset, yOffset;
	Int i;

	// get screen position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get image offset
	xOffset = instData->m_imageOffset.x;
	yOffset = instData->m_imageOffset.y;


	//
	// get pointer to image we want to draw depending on our state,
	// see GadgetPushButton.h for info
	//
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			leftImage					= GadgetButtonGetLeftDisabledSelectedImage( window );
			rightImage				= GadgetButtonGetRightDisabledSelectedImage( window );
			centerImage				= GadgetButtonGetMiddleDisabledSelectedImage( window );
		}
		else
		{

			leftImage					= GadgetButtonGetLeftDisabledImage( window );
			rightImage				= GadgetButtonGetRightDisabledImage( window );
			centerImage				= GadgetButtonGetMiddleDisabledImage( window );

		}

	}  // end if, disabled
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			leftImage					= GadgetButtonGetLeftHiliteSelectedImage( window );
			rightImage				= GadgetButtonGetRightHiliteSelectedImage( window );
			centerImage				= GadgetButtonGetMiddleHiliteSelectedImage( window );
		}
		else
		{

			leftImage					= GadgetButtonGetLeftHiliteImage( window );
			rightImage				= GadgetButtonGetRightHiliteImage( window );
			centerImage				= GadgetButtonGetMiddleHiliteImage( window );

		}

	}  // end else if, hilited and enabled
	else
	{

		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			leftImage					= GadgetButtonGetLeftEnabledSelectedImage( window );
			rightImage				= GadgetButtonGetRightEnabledSelectedImage( window );
			centerImage				= GadgetButtonGetMiddleEnabledSelectedImage( window );
		}
		else
		{

			leftImage					= GadgetButtonGetLeftEnabledImage( window );
			rightImage				= GadgetButtonGetRightEnabledImage( window );
			centerImage				= GadgetButtonGetMiddleEnabledImage( window );

		}

	}  // end else, enabled only

	// sanity, we need to have these images to make it look right
	if( leftImage == NULL || rightImage == NULL ||
			centerImage == NULL )
		return;

	// get image sizes for the ends
	ICoord2D leftSize, rightSize;
	leftSize.x = leftImage->getImageWidth();
	leftSize.y = leftImage->getImageHeight();
	rightSize.x = rightImage->getImageWidth();
	rightSize.y = rightImage->getImageHeight();

	// get two key points used in the end drawing
	ICoord2D leftEnd, rightStart;
	leftEnd.x = origin.x + leftSize.x + xOffset;
	leftEnd.y = origin.y + size.y + yOffset;
	rightStart.x = origin.x + size.x - rightSize.x + xOffset;
	rightStart.y = origin.y + yOffset;

	// draw the center repeating bar
	Int centerWidth, pieces;

	// get width we have to draw our repeating center in
	centerWidth = rightStart.x - leftEnd.x;

	if( centerWidth <= 0)
	{
//		TheDisplay->setClipRegion(&clipRegion);
		// draw left end
		start.x = origin.x + xOffset;
		start.y = origin.y + yOffset;
		end.y = leftEnd.y;
		end.x = origin.x + xOffset + size.x/2;
		TheWindowManager->winDrawImage(leftImage, start.x, start.y, end.x, end.y);

		// draw right end
		start.y = rightStart.y;
		start.x = end.x;
		end.x = origin.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winDrawImage(rightImage, start.x, start.y, end.x, end.y);
	}
	else
	{

		// how many whole repeating pieces will fit in that width
		pieces = centerWidth / centerImage->getImageWidth();

		// draw the pieces
		start.x = leftEnd.x;
		start.y = origin.y + yOffset;
		end.y = start.y + size.y + yOffset; //centerImage->getImageHeight() + yOffset;
//		TheDisplay->setClipRegion(&clipRegion);
		for( i = 0; i < pieces; i++ )
		{

			end.x = start.x + centerImage->getImageWidth();
			TheWindowManager->winDrawImage( centerImage,
																			start.x, start.y,
																			end.x, end.y );
			start.x += centerImage->getImageWidth();

		}  // end for i

		// we will draw the image but clip the parts we don't want to show
		IRegion2D reg;
		reg.lo.x = start.x;
		reg.lo.y = clipRegion.lo.y;
		reg.hi.x = rightStart.x;
		reg.hi.y = clipRegion.hi.y;
		centerWidth = rightStart.x - start.x;
		if( centerWidth > 0)
		{
			TheDisplay->setClipRegion(&reg);
			end.x = start.x + centerImage->getImageWidth();
			TheWindowManager->winDrawImage( centerImage,
																			start.x, start.y,
																			end.x, end.y );
			TheDisplay->enableClipping(FALSE);
		}

//		TheDisplay->setClipRegion(&clipRegion);
		// draw left end
		start.x = origin.x + xOffset;
		start.y = origin.y + yOffset;
		end = leftEnd;
		TheWindowManager->winDrawImage(leftImage, start.x, start.y, end.x, end.y);

		// draw right end
		start = rightStart;
		end.x = start.x + rightSize.x;
		end.y = start.y + size.y;
		TheWindowManager->winDrawImage(rightImage, start.x, start.y, end.x, end.y);
	}

	// draw the button text
	if( instData->getTextLength() )
		drawText( window, instData );

	// get window position
	window->winGetScreenPosition( &start.x, &start.y );
	window->winGetSize( &size.x, &size.y );


	// if we have a video buffer, draw the video buffer
	if ( instData->m_videoBuffer )
	{
		TheDisplay->drawVideoBuffer( instData->m_videoBuffer, start.x, start.y, start.x + size.x, start.y + size.y );
	}
	PushButtonData *pData = (PushButtonData *)window->winGetUserData();
	if( pData )
	{
		if( pData->overlayImage )
		{
			TheDisplay->drawImage( pData->overlayImage, start.x, start.y, start.x + size.x, start.y + size.y );
		}

		if( pData->drawClock )
		{
			if( pData->drawClock == NORMAL_CLOCK )
			{
				TheDisplay->drawRectClock(start.x, start.y, size.x, size.y, pData->percentClock,pData->colorClock);
			}
			else if( pData->drawClock == INVERSE_CLOCK )
			{
				TheDisplay->drawRemainingRectClock( start.x, start.y, size.x, size.y, pData->percentClock,pData->colorClock );
			}
			pData->drawClock = NO_CLOCK;
			window->winSetUserData(pData);
		}

		if( pData->drawBorder && pData->colorBorder != GAME_COLOR_UNDEFINED )
		{
			TheDisplay->drawOpenRect(start.x - 1, start.y - 1, size.x + 2, size.y + 2, 1, pData->colorBorder);
		}
	}

//	TheDisplay->enableClipping(FALSE);
}  // end W3DGadgetPushButtonImageDraw


// drawButtonText =============================================================
/** Draw button text to the screen */
//=============================================================================
static void drawText( GameWindow *window, WinInstanceData *instData )
{
	ICoord2D origin, size, textPos;
	Int width, height;
	Color textColor, dropColor;
	DisplayString *text = instData->getTextDisplayString();

	// sanity
	if( text == NULL || text->getTextLength() == 0 )
		return;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// set whether or not we center the wrapped text
	text->setWordWrapCentered( BitIsSet( instData->getStatus(), WIN_STATUS_WRAP_CENTERED ));
	text->setWordWrap(size.x);
	// get the right text color
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{
		textColor = window->winGetDisabledTextColor();
		dropColor = window->winGetDisabledTextBorderColor();
	}  // end if, disabled
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		textColor = window->winGetHiliteTextColor();
		dropColor = window->winGetHiliteTextBorderColor();
	}  // end else if, hilited
	else
	{
		textColor = window->winGetEnabledTextColor();
		dropColor = window->winGetEnabledTextBorderColor();
	}  // end enabled only

	// set our font to that of our parent if not the same
	if( text->getFont() != window->winGetFont() )
		text->setFont( window->winGetFont() );

	// get text size
	text->getSize( &width, &height );

	// where to draw
	textPos.x = origin.x + (size.x / 2) - (width / 2);
	textPos.y = origin.y + (size.y / 2) - (height / 2);
//	text->setClipRegion(&clipRegion);
	// draw it
	text->draw( textPos.x, textPos.y, textColor, dropColor );

}  // end drawButtonText

// W3DMainMenuRandomTextDraw ==================================================
/** Specialized drawing function for the random text */
//=============================================================================
void W3DMainMenuRandomTextDraw( GameWindow *window, WinInstanceData *instData )
{
	TextData *tData = (TextData *)window->winGetUserData();
	Color textColor, textOutlineColor;
	ICoord2D size, origin, textPos;
	Int textWidth, textHeight;
	IRegion2D textclipRegion;
	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );
	textColor					= window->winGetDisabledTextColor();
	textOutlineColor	= window->winGetDisabledTextBorderColor();
	// draw the text

  if( !(tData->text && (textColor != WIN_COLOR_UNDEFINED)) )
		return;

	DisplayString *text = tData->text;
	//Init the clip region
	textclipRegion.lo.x = origin.x + 1;
	textclipRegion.lo.y = origin.y + 1;
	textclipRegion.hi.x = origin.x + size.x - 1;
	textclipRegion.hi.y = origin.y + size.y - 1;

	// how much space will this text take up
	text->getSize( &textWidth, &textHeight );

	// draw the text
	textPos.x = origin.x;
	textPos.y = origin.y + (size.y / 2) - (textHeight / 2);
	text->setClipRegion(&textclipRegion);
	text->draw( textPos.x, textPos.y, textColor, textOutlineColor );

}
void W3DThinBorderDraw( GameWindow *window, WinInstanceData *instData )
{
	ICoord2D start, size;
	const Image *image;

	//W3DGameWinDefaultDraw( window, instData );
//	TheDisplay->setClipRegion(&clipRegion);
	window->winGetScreenPosition( &start.x, &start.y );
	window->winGetSize( &size.x, &size.y );
	image = window->winGetEnabledImage( 0 );

	if( image )
	{
		ICoord2D begin, end;

		begin.x = start.x + instData->m_imageOffset.x;
		begin.y = start.y + instData->m_imageOffset.y;
		end.x = begin.x + size.x;
		end.y = begin.y + size.y;
		TheWindowManager->winDrawImage( image, begin.x, begin.y, end.x, end.y );

	}  // end if
	// get window position

//	TheDisplay->drawOpenRect(start.x - 1, start.y - 1, size.x + 2, size.y + 2, 1, BrownishColor);
	TheDisplay->enableClipping(FALSE);
}

void W3DMainMenuInit( WindowLayout *layout, void *userData )
{
/*
	GameWindow *parent = layout->getFirstWindow();
//	NameKeyType buttonWorldBuilderID = TheNameKeyGenerator->nameToKey( "MainMenu.wnd:ButtonWorldBuilder" );
//	GameWindow *buttonWorldBuilder = TheWindowManager->winGetWindowFromId( parent, buttonWorldBuilderID );
//	if (buttonWorldBuilder)
//		buttonWorldBuilder->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);

	NameKeyType staticTextRandom1ID = TheNameKeyGenerator->nameToKey( "MainMenu.wnd:StaticTextRandom1" );
	NameKeyType staticTextRandom2ID = TheNameKeyGenerator->nameToKey( "MainMenu.wnd:StaticTextRandom2" );
	GameWindow *staticTextRandom1 = TheWindowManager->winGetWindowFromId( parent, staticTextRandom1ID);
	GameWindow *staticTextRandom2 = TheWindowManager->winGetWindowFromId( parent, staticTextRandom2ID);
	if (staticTextRandom1)
		staticTextRandom1->winSetDrawFunc(W3DMainMenuRandomTextDraw);
	if (staticTextRandom2)
		staticTextRandom2->winSetDrawFunc(W3DMainMenuRandomTextDraw);

//	//NameKeyType getUpdateID = TheNameKeyGenerator->nameToKey( "MainMenu.wnd:ButtonGetUpdate" );
	NameKeyType buttonUSAID = TheNameKeyGenerator->nameToKey( "MainMenu.wnd:ButtonUSA" );
	NameKeyType buttonGLAID = TheNameKeyGenerator->nameToKey( "MainMenu.wnd:ButtonGLA" );
	NameKeyType buttonChinaID = TheNameKeyGenerator->nameToKey( "MainMenu.wnd:ButtonChina" );
	NameKeyType skirmishID = TheNameKeyGenerator->nameToKey( AsciiString("MainMenu.wnd:ButtonSkirmish") );
	NameKeyType onlineID = TheNameKeyGenerator->nameToKey( AsciiString("MainMenu.wnd:ButtonOnline") );
	NameKeyType networkID = TheNameKeyGenerator->nameToKey( AsciiString("MainMenu.wnd:ButtonNetwork") );

	GameWindow *button = TheWindowManager->winGetWindowFromId( parent, skirmishID );
	if (button)
		button->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	button = TheWindowManager->winGetWindowFromId( parent, onlineID );
	if (button)
		button->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	button = TheWindowManager->winGetWindowFromId( parent, networkID );
	if (button)
		button->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);

//GameWindow *getUpdate = TheWindowManager->winGetWindowFromId( parent, getUpdateID );
		//	if (getUpdate)
		//		getUpdate->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);

	GameWindow *buttonUSA = TheWindowManager->winGetWindowFromId( parent, buttonUSAID );
	if (buttonUSA)
		buttonUSA->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	GameWindow *buttonGLA = TheWindowManager->winGetWindowFromId( parent, buttonGLAID );
	if (buttonGLA)
		buttonGLA->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	GameWindow *buttonChina = TheWindowManager->winGetWindowFromId( parent, buttonChinaID );
	if (buttonChina)
		buttonChina->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);

	GameWindow *win = NULL;
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonMultiBack"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonSingleBack"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonExit"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonOptions"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonMultiplayer"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonSinglePlayer"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonReplay"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonLoadGame"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonLoadReplay"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);
	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonLoadReplayBack"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);

	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonTRAINING"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);

	win = TheWindowManager->winGetWindowFromId(parent, TheNameKeyGenerator->nameToKey("MainMenu.wnd:ButtonCredits"));
	if(win)
		win->winSetDrawFunc(W3DMainMenuButtonDropShadowDraw);

	GameWindow *clipRegionWin = TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( AsciiString("MainMenu.wnd:MapBorder") ));
	Int x,y,width,height;
	clipRegionWin->winGetScreenPosition(&x, &y);
	clipRegionWin->winGetSize(&width, &height);
	clipRegion.lo.x = x - 10;
	clipRegion.lo.y = y ;
	clipRegion.hi.x = x + width + 10;
	clipRegion.hi.y = y + height + 10;
*/
	MainMenuInit( layout, userData );
}

void W3DCreditsMenuDraw( GameWindow *window, WinInstanceData *instData )
{
	TheCredits->draw();
}
