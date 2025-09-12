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

// FILE: W3DGameWindow.cpp ////////////////////////////////////////////////////
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
// File name: W3DGameWindow.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      W3D implementation of a game window
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowGlobal.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "W3DDevice/GameClient/W3DGameWindowManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"

// DEFINES ////////////////////////////////////////////////////////////////////
enum
{
	BORDER_CORNER_SIZE	= 15,
	BORDER_LINE_SIZE		= 20,
};

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static Bool bordersInit = FALSE;
static const Image *borderPieces[NUM_BORDER_PIECES] = { 0 };

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// initBorders ================================================================
//=============================================================================
static void initBorders( void )
{

	borderPieces[ BORDER_CORNER_UL ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderCornerUL" ) );

	borderPieces[ BORDER_CORNER_UR ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderCornerUR" ) );

	borderPieces[ BORDER_CORNER_LL ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderCornerLL" ) );

	borderPieces[ BORDER_CORNER_LR ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderCornerLR" ) );

	borderPieces[ BORDER_VERTICAL_LEFT ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderLeft" ) );

	borderPieces[ BORDER_VERTICAL_LEFT_SHORT ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderLeftShort" ) );

	borderPieces[ BORDER_HORIZONTAL_TOP ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderTop" ) );

	borderPieces[ BORDER_HORIZONTAL_TOP_SHORT ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderTopShort" ) );

	borderPieces[ BORDER_VERTICAL_RIGHT ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderRight" ) );

	borderPieces[ BORDER_VERTICAL_RIGHT_SHORT ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderRightShort" ) );

	borderPieces[ BORDER_HORIZONTAL_BOTTOM ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderBottom" ) );

	borderPieces[ BORDER_HORIZONTAL_BOTTOM_SHORT ] =
						TheMappedImageCollection->findImageByName( AsciiString( "BorderBottomShort" ) );

	bordersInit = TRUE;

}

// W3DGameWindow::blitBorderRect ==============================================
//=============================================================================
void W3DGameWindow::blitBorderRect( Int x, Int y, Int width, Int height )
{
	Int Offset = 15;
	Int OffsetLower = 5;
	// init image loc if needed
	if( bordersInit == FALSE )
		initBorders();

	// save original x, y
	Int originalX = x;
	Int originalY = y;
	Int maxX = x + width;
	Int maxY = y + height;
	Int x2, y2;			// used for simultaneous drawing of line pairs
	Int size = 20;
	Int halfSize = size / 2;

	// Draw Horizontal Lines
	// All border pieces are based on a 10 pixel offset from the centerline
	y = originalY - Offset;
	y2 = maxY - OffsetLower;
	x2 = maxX - (OffsetLower + BORDER_LINE_SIZE);

	for( x=(originalX + OffsetLower); x <= x2; x += BORDER_LINE_SIZE )
	{

		TheDisplay->drawImage( borderPieces[ BORDER_HORIZONTAL_TOP ],
													 x, y, x + size, y + size );
		TheDisplay->drawImage( borderPieces[ BORDER_HORIZONTAL_BOTTOM ],
													 x, y2, x + size, y2 + size );

	}

	x2 = maxX - 5;//BORDER_CORNER_SIZE;

	// x == place to draw remainder if any
	if( (x2 - x) >= (BORDER_LINE_SIZE / 2) )
	{

		//Blit Half piece
		TheDisplay->drawImage( borderPieces[ BORDER_HORIZONTAL_TOP_SHORT ],
													 x, y, x + halfSize, y + size );
		TheDisplay->drawImage( borderPieces[ BORDER_HORIZONTAL_BOTTOM_SHORT ],
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
		TheDisplay->drawImage( borderPieces[ BORDER_HORIZONTAL_TOP_SHORT ],
													 x, y, x + halfSize, y + size );
		TheDisplay->drawImage( borderPieces[ BORDER_HORIZONTAL_BOTTOM_SHORT ],
													 x, y2, x + halfSize, y2 + size );

	}

	// Draw Vertical Lines
	// All border pieces are based on a 10 pixel offset from the centerline
	x = originalX - Offset;
	x2 = maxX - OffsetLower;
	y2 = maxY - (OffsetLower + BORDER_LINE_SIZE);

	for( y=(originalY + OffsetLower); y <= y2; y += BORDER_LINE_SIZE )
	{

		TheDisplay->drawImage( borderPieces[ BORDER_VERTICAL_LEFT ],
													 x, y, x + size, y + size );
		TheDisplay->drawImage( borderPieces[ BORDER_VERTICAL_RIGHT ],
													 x2, y, x2 + size, y + size );

	}

	y2 = maxY - OffsetLower;//BORDER_CORNER_SIZE;

	// y == place to draw remainder if any
	if( (y2 - y) >= (BORDER_LINE_SIZE / 2) )
	{

		//Blit Half piece
		TheDisplay->drawImage( borderPieces[ BORDER_VERTICAL_LEFT_SHORT ],
													 x, y, x + size, y + halfSize );
		TheDisplay->drawImage( borderPieces[ BORDER_VERTICAL_RIGHT_SHORT ],
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
		TheDisplay->drawImage( borderPieces[ BORDER_VERTICAL_LEFT_SHORT ],
													 x, y, x + size, y + halfSize );
		TheDisplay->drawImage( borderPieces[ BORDER_VERTICAL_RIGHT_SHORT ],
													 x2, y, x2 + size, y + halfSize );

	}

	// Draw Corners
	x = originalX - BORDER_CORNER_SIZE ;
	y = originalY - BORDER_CORNER_SIZE;
	TheDisplay->drawImage( borderPieces[ BORDER_CORNER_UL ],
												 x, y, x + size, y + size );
	x = maxX - 5;//BORDER_CORNER_SIZE;
	y = originalY - BORDER_CORNER_SIZE;
	TheDisplay->drawImage( borderPieces[ BORDER_CORNER_UR ],
												 x, y, x + size, y + size );
	x = originalX - BORDER_CORNER_SIZE;
	y = maxY - 5;//BORDER_CORNER_SIZE;
	TheDisplay->drawImage( borderPieces[ BORDER_CORNER_LL ],
												 x, y, x + size, y + size );
	x = maxX - 5;//BORDER_CORNER_SIZE;
	y = maxY - 5;//BORDER_CORNER_SIZE;
	TheDisplay->drawImage( borderPieces[ BORDER_CORNER_LR ],
												 x, y, x + size, y + size );

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGameWindow::W3DGameWindow ===============================================
//=============================================================================
W3DGameWindow::W3DGameWindow( void )
{

	// override the default draw with our own default draw function for W3D
	winSetDrawFunc( TheWindowManager->getDefaultDraw() );

	m_textPos.x = m_textPos.y = 0;
	m_currTextColor = WIN_COLOR_UNDEFINED;
	m_needPolyDraw = FALSE;
	m_newTextPos = FALSE;

}

// W3DGameWindow::~W3DGameWindow ==============================================
//=============================================================================
W3DGameWindow::~W3DGameWindow( void )
{

}

// W3DGameWinDefaultDraw ======================================================
/** The default redraw callback.  Draws the background using either
	* the drawData or the background color unless the background color
	* is set to -1 indicating that default drawing is turned off. */
//=============================================================================
void W3DGameWinDefaultDraw( GameWindow *window, WinInstanceData *instData )
{
	Real borderWidth = 1.0f;
	ICoord2D origin;
	ICoord2D size;
	W3DGameWindow *w3dWindow = (W3DGameWindow *)window;

	/** @todo NOTE that we're making a W3DGameWindow cast here, it seems
	logical because we are in a W3D draw function so it's reasonable to assume
	that we have a W3DGameWindow.  However, we may want to revisit this
	type of casting in the future, we have the same problems with the
	ObjectModules where we cast object modules for their individual methods.
	Also note that the other W3D implementations of GUI controls are making
	a similar cast for their device implementation functions */

	// get the window position in the screen coordinates
	w3dWindow->winGetScreenPosition( &origin.x, &origin.y );

	// get size of window
	w3dWindow->winGetSize( &size.x, &size.y );

	// image drawing vs color drawing
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_IMAGE ) )
	{
		const Image *image;

		// get image
		if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
			image = window->winGetDisabledImage( 0 );
		else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
			image = window->winGetHiliteImage( 0 );
		else
			image = window->winGetEnabledImage( 0 );

		if( image )
		{
			ICoord2D start, end;

			start.x = origin.x + instData->m_imageOffset.x;
			start.y = origin.y + instData->m_imageOffset.y;
			end.x = start.x + size.x;
			end.y = start.y + size.y;
			TheWindowManager->winDrawImage( image, start.x, start.y, end.x, end.y );

		}

	}
	else
	{
		Color color, borderColor;

		// get colors
		if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
		{

			color				= window->winGetDisabledColor( 0 );
			borderColor = window->winGetDisabledBorderColor( 0 );

		}
		else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
		{

			color				= window->winGetHiliteColor( 0 );
			borderColor = window->winGetHiliteBorderColor( 0 );

		}
		else
		{

			color				= window->winGetEnabledColor( 0 );
			borderColor = window->winGetEnabledBorderColor( 0 );

		}

		//
		// draw the border at the edges
		//
		if( borderColor != WIN_COLOR_UNDEFINED )
			TheWindowManager->winOpenRect( borderColor, borderWidth,
																		 origin.x, origin.y,
																		 origin.x + size.x, origin.y + size.y );

		// draw filled background
		if( color != WIN_COLOR_UNDEFINED )
			TheWindowManager->winFillRect( color, borderWidth,
																		 origin.x + borderWidth,
																		 origin.y + borderWidth,
																		 origin.x + size.x - borderWidth,
																		 origin.y + size.y - borderWidth );

	}

	// if we have a video buffer, draw the video buffer
	if ( instData->m_videoBuffer )
	{
		ICoord2D pos, size;
		window->winGetScreenPosition( &pos.x, &pos.y );
		window->winGetSize( &size.x, &size.y );

		TheDisplay->drawVideoBuffer( instData->m_videoBuffer, pos.x, pos.y, pos.x + size.x, pos.y + size.y );
	}

}

// W3DGameWindow::winDrawBorder ===============================================
//=============================================================================
void W3DGameWindow::winDrawBorder( void )
{
	Bool found = FALSE;
	Int originalX, originalY;
	Int x, y;
	Int width;
	Int i, bits;

/** @todo this WinDrawBorder is the old Nox function for drawing the borders
	* on various windows and controls.  We should derive classes of game
	* windows for the different GUI controls and move the specific pieces of
	* code that apply for those gadgets to those classes */

	// based on window class pass different regions to real draw

	winGetScreenPosition( &originalX, &originalY );

	for( i = 0; (i < (sizeof(UnsignedInt) * 8)) && (found == FALSE); i++ )
	{

		bits = (1 << i);

		if( m_instData.getStyle() & bits )
		{

			switch( m_instData.getStyle() & bits )
			{
				case GWS_CHECK_BOX:
					found = TRUE;
					break;

				case GWS_ENTRY_FIELD:
				{
//					EntryData *e = (EntryData *)m_userData;
					width = m_size.x;

					x = originalX;
					y = originalY;

					// Calculate space for Label
					if( m_instData.getTextLength() )
					{
						Int textWidth = 0;

						TheWindowManager->winGetTextSize( m_instData.getFont(),
																							m_instData.getText(),
																							&textWidth, NULL, 0 );
						width -= textWidth + 6;
						x += textWidth + 6;

					}

/*
// Colin: The very notion of entry width makes no sense to me since
// we already have a gadget width, and the max characters for
// an entry box so I am removing this.
					// Adjust entry box if an entryWidth is provided
					if( (e->entryWidth > 0) && (width > e->entryWidth) )
					{
						width = e->entryWidth;
						x = originalX + (m_size.x - e->entryWidth);
					}
*/

					blitBorderRect( x, y, width, m_size.y );

					found = TRUE;
					break;
				}

				// Sliders may need a bar for the button
				// it is assumed that the smaller size component (width, height)
				// is the axis of the slider
				case GWS_VERT_SLIDER:
				case GWS_HORZ_SLIDER:
//					blitBorderLine( originalX, originalY, window->size.x, window->size.y );

					found = TRUE;
					break;

				case GWS_SCROLL_LISTBOX:
				{
					ListboxData *list = (ListboxData *)m_userData;
					Int sliderAdjustment = 0;
					Int labelAdjustment = 0;

					if( list->scrollBar )
					{
						GameWindow *child = list->slider->winGetChild();
						ICoord2D size;

						child->winGetSize( &size.x, &size.y );
						sliderAdjustment = size.y;

					}

					if( m_instData.getTextLength() )
						labelAdjustment = 4;

					blitBorderRect( (originalX - 3),
													(originalY - (3 + labelAdjustment)),
													(m_size.x + 3 - sliderAdjustment),
													(m_size.y + 6) );

					found = TRUE;
					break;
				}

				case GWS_RADIO_BUTTON:
				case GWS_STATIC_TEXT:
				case GWS_PROGRESS_BAR:
				case GWS_PUSH_BUTTON:
				case GWS_USER_WINDOW:
				case GWS_TAB_CONTROL:
					blitBorderRect( originalX, originalY, m_size.x, m_size.y );

					found = TRUE;
					break;

			}

		}

	}

}

// W3DGameWindow::winSetFont ==================================================
/** Set the font for a widow */
//=============================================================================
void W3DGameWindow::winSetFont( GameFont *font )
{

	// extending functionality
	GameWindow::winSetFont( font );

	// assign font to text renderer
	m_textRenderer.Set_Font( static_cast<FontCharsClass *>(font->fontData) );

	// this is a visual change
	m_needPolyDraw = TRUE;

}

// W3DGameWindow::winSetText ==================================================
/** Set the text for window */
//=============================================================================
Int W3DGameWindow::winSetText( UnicodeString newText )
{

	// extending functionality
	GameWindow::winSetText( newText );

	// rebuild the sentence in our text renderer
	m_textRenderer.Build_Sentence( m_instData.getText().str(),NULL, NULL );

	// this is a visual change
	m_needPolyDraw = TRUE;

	return WIN_ERR_OK;

}

// W3DGameWindow::winSetPosition ==============================================
/** Set window position */
//=============================================================================
Int W3DGameWindow::winSetPosition( Int x, Int y )
{
	ICoord2D prevPos;

	// get previous position
	prevPos.x = m_region.lo.x;
	prevPos.y = m_region.lo.y;

	// extending functionality
	GameWindow::winSetPosition( x, y );

	// update any text position change
	m_textPos.x += m_region.lo.x - prevPos.x;
	m_textPos.y += m_region.lo.y - prevPos.y;
	m_newTextPos = TRUE;

	return WIN_ERR_OK;

}

// W3DGameWindow::getTextSize =================================================
/** Get the size of the text in our inst data */
//=============================================================================
void W3DGameWindow::getTextSize( Int *width, Int *height )
{
	Vector2 extents = m_textRenderer.Get_Text_Extents( m_instData.getText().str() );

	if( width )
		*width = extents.X;
	if( height )
		*height = extents.Y;

}

// W3DGameWindow::getTextLoc ==================================================
// Set our text rendering location */
//=============================================================================
void W3DGameWindow::setTextLoc( Int x, Int y )
{

	if( m_textPos.x != x )
	{

		m_textPos.x = x;
		m_newTextPos = TRUE;

	}
	if( m_textPos.y != y )
	{

		m_textPos.y = y;
		m_newTextPos = TRUE;

	}

}

// W3DGameWindow::drawText ====================================================
/** Draw the text in our 2d sentence renderer */
//=============================================================================
void W3DGameWindow::drawText( Color color )
{
	Bool needDraw = FALSE;

	// if new text pos we need to redraw
	if( m_newTextPos )
	{

		m_newTextPos = FALSE;
		needDraw = TRUE;

	}

	// if color switch, set new color
	if( m_currTextColor != color )
	{

		m_currTextColor = color;
		needDraw = TRUE;

	}

	// draw the quads if needed
	if( needDraw || m_needPolyDraw )
	{
		UnsignedInt outline = TheWindowManager->winMakeColor( 0, 0, 0, 255 );

		m_textRenderer.Reset_Polys();
		m_textRenderer.Set_Location( Vector2( m_textPos.x + 1, m_textPos.y + 1 ) );
		m_textRenderer.Draw_Sentence( outline );

		m_textRenderer.Set_Location( Vector2( m_textPos.x, m_textPos.y ) );
		m_textRenderer.Draw_Sentence( m_currTextColor );

		m_needPolyDraw = FALSE;

	}

	// do the render
	m_textRenderer.Render();

}

