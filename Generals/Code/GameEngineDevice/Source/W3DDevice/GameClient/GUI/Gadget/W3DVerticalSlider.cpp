/*
**	Command & Conquer Generals(tm)
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

// FILE: .cpp /////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:
//
// File name: .cpp
//
// Created:
//
// Desc:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GadgetSlider.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplay.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGadgetVerticalSliderDraw ================================================
/** Draw colored vertical slider using standard graphics */
//=============================================================================
void W3DGadgetVerticalSliderDraw( GameWindow *window,
																	WinInstanceData *instData )
{
	Color backBorder, backColor;
	ICoord2D origin, size, start, end;

	// get screen position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get the right colors
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		backBorder		= GadgetSliderGetDisabledBorderColor( window );
		backColor			= GadgetSliderGetDisabledColor( window );

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		backBorder		= GadgetSliderGetHiliteBorderColor( window );
		backColor			= GadgetSliderGetHiliteColor( window );

	}
	else
	{

		backBorder		= GadgetSliderGetEnabledBorderColor( window );
		backColor			= GadgetSliderGetEnabledColor( window );

	}

	// draw background border and rect over whole control
	if( backBorder != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x;
		start.y = origin.y;
		end.x = start.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winOpenRect( backBorder, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}
	if( backColor != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x + 1;
		start.y = origin.y + 1;
		end.x = start.x + size.x - 2;
		end.y = start.y + size.y - 2;
		TheWindowManager->winFillRect( backColor, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}



}

// W3DGadgetVerticalSliderImageDraw ===========================================
/** Draw vertical slider with user supplied images */
//=============================================================================
void W3DGadgetVerticalSliderImageDraw( GameWindow *window,
																			 WinInstanceData *instData )
{
	const Image *topImage, *bottomImage, *centerImage, *smallCenterImage;
	ICoord2D origin, size, start, end;
	Int xOffset, yOffset;
	Int i;

	// get screen position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get image offset
	xOffset = instData->m_imageOffset.x;
	yOffset = instData->m_imageOffset.y;

	// get the right images
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		topImage					= GadgetSliderGetDisabledImageTop( window );
		bottomImage				= GadgetSliderGetDisabledImageBottom( window );
		centerImage				= GadgetSliderGetDisabledImageCenter( window );
		smallCenterImage	= GadgetSliderGetDisabledImageSmallCenter( window );

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		topImage					= GadgetSliderGetHiliteImageTop( window );
		bottomImage				= GadgetSliderGetHiliteImageBottom( window );
		centerImage				= GadgetSliderGetHiliteImageCenter( window );
		smallCenterImage	= GadgetSliderGetHiliteImageSmallCenter( window );

	}
	else
	{

		topImage					= GadgetSliderGetEnabledImageTop( window );
		bottomImage				= GadgetSliderGetEnabledImageBottom( window );
		centerImage				= GadgetSliderGetEnabledImageCenter( window );
		smallCenterImage	= GadgetSliderGetEnabledImageSmallCenter( window );

	}

	// sanity, we need to have these images to make it look right
	if( topImage == nullptr || bottomImage == nullptr ||
			centerImage == nullptr || smallCenterImage == nullptr )
		return;

	// get image sizes for the ends
	ICoord2D topSize, bottomSize;
	topSize.x = topImage->getImageWidth();
	topSize.y = topImage->getImageHeight();
	bottomSize.x = bottomImage->getImageWidth();
	bottomSize.y = bottomImage->getImageHeight();

	if(topSize.y + bottomSize.y >= size.y)
	{
		// draw top end
		start.x = origin.x + xOffset;
		start.y = origin.y + yOffset;
		end.x = origin.x + xOffset + topSize.x;
		end.y = origin.y + size.y /2;
		TheWindowManager->winDrawImage(topImage, start.x, start.y, end.x, end.y);

		// draw bottom end
		start.y = origin.y + size.y /2;
		end.x = origin.x + xOffset + bottomSize.x;
		end.y = origin.y + yOffset + size.y;
		TheWindowManager->winDrawImage(bottomImage, start.x, start.y, end.x, end.y);
	}
	else
	{

		// get two key points used in the end drawing
		ICoord2D topEnd, bottomStart;
		topEnd.x = origin.x + topSize.x + xOffset;
		topEnd.y = origin.y + topSize.y + yOffset;
		bottomStart.x = origin.x + xOffset;
		bottomStart.y = origin.y + size.y - bottomSize.y + yOffset;

		// draw the center repeating bar
		Int centerHeight, pieces;

		// get size we have to draw our repeating center in
		centerHeight = bottomStart.y - topEnd.y;

		// how many whole repeating pieces will fit in that size
		pieces = centerHeight / centerImage->getImageHeight();

		// draw the pieces
		start.x = origin.x + xOffset;
		start.y = topEnd.y;
		end.x = start.x + centerImage->getImageWidth();
		end.y = start.y + centerImage->getImageHeight();
		for( i = 0; i < pieces; i++ )
		{

			TheWindowManager->winDrawImage( centerImage,
																			start.x, start.y,
																			end.x, end.y );
			start.y += centerImage->getImageHeight();
			end.y += centerImage->getImageHeight();

		}

		//
		// how many small repeating pieces will fit in the gap from where the
		// center repeating bar stopped and the bottom image, draw them
		// and overlapping underneath where the bottom end will go
		//
		centerHeight = bottomStart.y - start.y;
		pieces = centerHeight / smallCenterImage->getImageHeight() + 1;
		end.y = start.y + smallCenterImage->getImageHeight();
		for( i = 0; i < pieces; i++ )
		{

			TheWindowManager->winDrawImage( smallCenterImage,
																			start.x, start.y,
																			end.x, end.y );
			start.y += smallCenterImage->getImageHeight();
			end.y += smallCenterImage->getImageHeight();

		}

		// draw top end
		start.x = origin.x + xOffset;
		start.y = origin.y + yOffset;
		end = topEnd;
		TheWindowManager->winDrawImage(topImage, start.x, start.y, end.x, end.y);

		// draw bottom end
		start = bottomStart;
		end.x = start.x + bottomSize.x;
		end.y = start.y + bottomSize.y;
		TheWindowManager->winDrawImage(bottomImage, start.x, start.y, end.x, end.y);
	}


}


