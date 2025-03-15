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


// FILE: W3DPushButton.cpp ////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: W3DPushButton.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:			W3D implementation for the push button control element
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/Display.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DGadget.h"


#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off) 
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif


// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

void W3DGadgetPushButtonImageDrawThree(GameWindow *window, WinInstanceData *instData );
void W3DGadgetPushButtonImageDrawOne(GameWindow *window, WinInstanceData *instData );

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

// drawButtonText =============================================================
/** Draw button text to the screen */
//=============================================================================
static void drawButtonText( GameWindow *window, WinInstanceData *instData )
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
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_SHORTCUT_BUTTON ) )
	{
		// Oh god... this is a total hack for shortcut buttons to handle rendering text top left corner...
		textPos.x = origin.x + 2;
		textPos.y = origin.y + 0;
	}
	else
	{
		textPos.x = origin.x + (size.x / 2) - (width / 2);
		textPos.y = origin.y + (size.y / 2) - (height / 2);
	}

	// draw it
	text->draw( textPos.x, textPos.y, textColor, dropColor );

}  // end drawButtonText

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGadgetPushButtonDraw ====================================================
/** Draw colored pushbutton using standard graphics */
//=============================================================================
void W3DGadgetPushButtonDraw( GameWindow *window, WinInstanceData *instData )
{
	Color color, border;
	ICoord2D origin, size, start, end;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	//
	// get pointer to image we want to draw depending on our state,
	// see GadgetPushButton.h for info
	//
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			color			= GadgetButtonGetDisabledSelectedColor( window );
			border		= GadgetButtonGetDisabledSelectedBorderColor( window );
		}
		else
		{
			color			= GadgetButtonGetDisabledColor( window );
			border		= GadgetButtonGetDisabledBorderColor( window );
		}

	}  // end if, disabled
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			color			= GadgetButtonGetHiliteSelectedColor( window );
			border		= GadgetButtonGetHiliteSelectedBorderColor( window );
		}
		else
		{
			color			= GadgetButtonGetHiliteColor( window );
			border		= GadgetButtonGetHiliteBorderColor( window );
		}

	}  // end else if, hilited and enabled
	else
	{

		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			color			= GadgetButtonGetEnabledSelectedColor( window );
			border		= GadgetButtonGetEnabledSelectedBorderColor( window );
		}
		else
		{
			color			= GadgetButtonGetEnabledColor( window );
			border		= GadgetButtonGetEnabledBorderColor( window );
		}

	}  // end else, enabled only

	// compute draw position
	start.x = origin.x;
	start.y = origin.y;
	end.x = start.x + size.x;
	end.y = start.y + size.y;

	// box and border
	if( border != WIN_COLOR_UNDEFINED )
	{

		TheWindowManager->winOpenRect( border, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}  // end if

	if( color != WIN_COLOR_UNDEFINED )
	{

		// draw inside border
		start.x++;
		start.y++;
		end.x--;
		end.y--;
		TheWindowManager->winFillRect( color, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}  // end if

	// draw the button text
	if( instData->getTextLength() )
		drawButtonText( window, instData );

	// if we have a video buffer, draw the video buffer
	if ( instData->m_videoBuffer )
	{
		TheDisplay->drawVideoBuffer( instData->m_videoBuffer, origin.x, origin.y, origin.x + size.x, origin.y + size.y );
	}
	
	PushButtonData *pData = (PushButtonData *)window->winGetUserData();
	if( pData )
	{
		if( pData->overlayImage )
		{
			//Render the overlay image now.
			TheDisplay->drawImage( pData->overlayImage, origin.x, origin.y, origin.x + size.x, origin.y + size.y );
		}

		if( pData->drawClock )
		{
			if( pData->drawClock == NORMAL_CLOCK )
			{
				TheDisplay->drawRectClock(origin.x, origin.y, size.x, size.y, pData->percentClock,pData->colorClock);
			}
			else if( pData->drawClock == INVERSE_CLOCK )
			{
				TheDisplay->drawRemainingRectClock( origin.x, origin.y, size.x, size.y, pData->percentClock,pData->colorClock );
			}
			pData->drawClock = NO_CLOCK;
			window->winSetUserData(pData);
		}
		
		if( pData->drawBorder && pData->colorBorder != GAME_COLOR_UNDEFINED )
		{
			TheDisplay->drawOpenRect(origin.x -1, origin.y - 1, size.x + 2, size.y + 2,1 , pData->colorBorder);
		}
	}

}  // end W3DGadgetPushButtonDraw




// W3DGadgetPushButtonImageDraw ===============================================
/** Draw pushbutton with user supplied images */
//=============================================================================
void W3DGadgetPushButtonImageDraw( GameWindow *window, 
																	 WinInstanceData *instData )
{
	// if we return NULL then we'll call the one picture drawing code, if we return a value
	// then we'll call the 3 picture drawing code
	if( GadgetButtonGetMiddleEnabledImage( window ) ) 
	{
		if( BitIsSet( instData->getState(), WIN_STATUS_USE_OVERLAY_STATES ) )
		{
			ICoord2D size, start;
			// get window position
			window->winGetScreenPosition( &start.x, &start.y );
			window->winGetSize( &size.x, &size.y );
			// offset position by image offset
			start.x += instData->m_imageOffset.x;
			start.y += instData->m_imageOffset.y;

			DEBUG_CRASH( ("Button at %d,%d is attempting to render with W3DGadgetPushButtonImageDrawThree(), but is using overlay states! Forcing the code to use W3DGadgetPushButtonImageDrawOne() instead.", start.x, start.y ) );
			W3DGadgetPushButtonImageDrawOne( window, instData );
		}
		else
		{
			W3DGadgetPushButtonImageDrawThree( window, instData );
		}
	}
	else
	{
		W3DGadgetPushButtonImageDrawOne( window, instData );
	}
}

void W3DGadgetPushButtonImageDrawOne( GameWindow *window, 
																	 WinInstanceData *instData )
{
	const Image *image = NULL;
	ICoord2D size, start, end;

	//
	// get pointer to image we want to draw depending on our state,
	// see GadgetPushButton.h for info
	//
	image = GadgetButtonGetEnabledImage( window );

	if( !BitIsSet( window->winGetStatus(), WIN_STATUS_USE_OVERLAY_STATES ) )
	{
		//Certain buttons have the option to specify specific images for
		//altered states. If they do, then we won't render the auto-overlay versions.
		if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
		{

			if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
				image			= GadgetButtonGetDisabledSelectedImage( window );
			else
				image			= GadgetButtonGetDisabledImage( window );

		}  // end if, disabled
		else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
		{

			if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
				image			= GadgetButtonGetHiliteSelectedImage( window );
			else
				image			= GadgetButtonGetHiliteImage( window );

		}  // end else if, hilited and enabled
		else
		{

			if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
				image			= GadgetButtonGetHiliteSelectedImage( window );
		}  // end else, enabled only
	}


	// draw the image
	if( image )
	{

		// get window position
		window->winGetScreenPosition( &start.x, &start.y );
		window->winGetSize( &size.x, &size.y );


		// offset position by image offset
		start.x += instData->m_imageOffset.x;
		start.y += instData->m_imageOffset.y;

		// find end point
		end.x = start.x + size.x;
		end.y = start.y + size.y;

		Display::DrawImageMode	drawMode=Display::DRAW_IMAGE_ALPHA;
		Int colorMultiplier = 0xffffffff;

		if(BitIsSet( window->winGetStatus(), WIN_STATUS_USE_OVERLAY_STATES ) )
		{	
			//we're using a new drawing system which does "grayscale" disabled buttons using original color artwork.
			if( !BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) )
			{
				if( !BitIsSet( window->winGetStatus(), WIN_STATUS_NOT_READY ) )
				{
					//The button is disabled -- but if the button isn't "ready", we don't want to do this because
					//we want to show the button in color with just the clock overlay.
					if( !BitIsSet( window->winGetStatus(), WIN_STATUS_ALWAYS_COLOR ) )
					{
						drawMode=Display::DRAW_IMAGE_GRAYSCALE;
					}
					else
					{
						colorMultiplier = 0xff909090; //RGB values are 144/255 (90) -- Alpha is opaque (ff) --> ff909090;
					}
				}
			}
		}
		TheDisplay->drawImage( image, start.x, start.y, end.x, end.y, colorMultiplier, drawMode );
	}  // end if

	// draw the button text
	if( instData->getTextLength() )
		drawButtonText( window, instData );

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
			//Render the overlay image now.
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

	//Now render overlays that pertain to the correct state.

	if( BitIsSet( window->winGetStatus(), WIN_STATUS_FLASHING ) )
	{
		//Handle cameo flashing (let the flashing stack with overlay states)
		static const Image *hilitedOverlayIcon = TheMappedImageCollection->findImageByName( "Cameo_push" );
		TheDisplay->drawImage( hilitedOverlayIcon, start.x, start.y, start.x + size.x, start.y + size.y );
	}
	
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_USE_OVERLAY_STATES ) )
	{
		image = NULL;
		static const Image *pushedOverlayIcon	= TheMappedImageCollection->findImageByName( "Cameo_push" );
		static const Image *hilitedOverlayIcon = TheMappedImageCollection->findImageByName( "Cameo_hilited" );
		if( pushedOverlayIcon && hilitedOverlayIcon )
		{
			if(BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED))
			{
				if (BitIsSet( instData->getState(), WIN_STATE_HILITED ))
				{
					if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
					{
						//The button is hilited and pushed
						TheDisplay->drawImage( pushedOverlayIcon, start.x, start.y, start.x + size.x, start.y + size.y );
					}
					else
					{
						//The button is hilited
						TheDisplay->drawImage( hilitedOverlayIcon, start.x, start.y, start.x + size.x, start.y + size.y );
					}
				}
  			else if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
 				{
 					//The button appears to be pushed -- CHECK_LIKE buttons that are on.
 					TheDisplay->drawImage( pushedOverlayIcon, start.x, start.y, start.x + size.x, start.y + size.y );
  			}
			}
		}
	}
}  // end W3DGadgetPushButtonImageDraw


void W3DGadgetPushButtonImageDrawThree(GameWindow *window, WinInstanceData *instData )
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
		reg.lo.y = start.y;
		reg.hi.x = rightStart.x;
		reg.hi.y = end.y;
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
		drawButtonText( window, instData );

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
			//Render the overlay image now.
			TheDisplay->drawImage( pData->overlayImage, origin.x, origin.y, origin.x + size.x, origin.y + size.y );
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
}
