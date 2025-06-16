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


// FILE: W3DListBox.cpp ///////////////////////////////////////////////////////
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
// File name: W3DListBox.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      W3D implementation for the list box control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplay.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// drawHiliteBar ==============================================================
/** Draw image for the hilite bar */
//=============================================================================
static void drawHiliteBar( const Image *left, const Image *right, 
													 const Image *center, const Image *smallCenter, 
													 Int startX, Int startY,
													 Int endX, Int endY )
{
	ICoord2D barWindowSize;  // end point of bar from window origin
	Int xOffset = 0, yOffset = 0;  // incase we want this functionality later
	ICoord2D start, end;
	Int i;
	IRegion2D clipRegion;



	barWindowSize.x = endX - startX;
	barWindowSize.y = endY - startY;

	//
	// the bar window size will always be at least big enough to accomodate
	// the left and right ends
	//
	if( barWindowSize.x < left->getImageWidth() + right->getImageWidth() )
		barWindowSize.x = left->getImageWidth() + right->getImageWidth();

	// get image sizes for the ends
	ICoord2D leftSize, rightSize;
	leftSize.x = left->getImageWidth();
	leftSize.y = left->getImageHeight();
	rightSize.x = right->getImageWidth();
	rightSize.y = right->getImageHeight();

	// get two key points used in the end drawing
	ICoord2D leftEnd, rightStart;
	leftEnd.x = startX + leftSize.x + xOffset;
	leftEnd.y = startY + barWindowSize.y + yOffset;
	rightStart.x = startX + barWindowSize.x - rightSize.x + xOffset;
	rightStart.y = startY + yOffset;

	// draw the center repeating bar
	Int centerWidth, pieces;

	// get width we have to draw our repeating center in
	centerWidth = rightStart.x - leftEnd.x;

	// how many whole repeating pieces will fit in that width
	pieces = centerWidth / center->getImageWidth();



	// draw the pieces
	start.x = leftEnd.x;
	start.y = startY + yOffset;
	end.y = start.y + barWindowSize.y;
	for( i = 0; i < pieces; i++ )
	{

		end.x = start.x + center->getImageWidth();
		TheWindowManager->winDrawImage( center, 
																		start.x, start.y,
																		end.x, end.y );
		start.x += center->getImageWidth();

	}  // end for i

	//
	// how many small repeating pieces will fit in the gap from where the
	// center repeating bar stopped and the right image, draw them
	// and overlapping underneath where the right end will go
	//
		// set the text clip region to the outline of the listbox
	clipRegion.lo.x = leftEnd.x;
	clipRegion.lo.y = startY + yOffset;
	clipRegion.hi.x = leftEnd.x + centerWidth;
	clipRegion.hi.y = start.y + barWindowSize.y;
	TheDisplay->setClipRegion(&clipRegion);
	centerWidth = rightStart.x - start.x;
	if( centerWidth )
	{

		pieces = centerWidth / smallCenter->getImageWidth() + 1;
		end.y = start.y + barWindowSize.y;
		for( i = 0; i < pieces; i++ )
		{

			end.x = start.x + smallCenter->getImageWidth();
			TheWindowManager->winDrawImage( smallCenter,
																			start.x, start.y,
																			end.x, end.y );
			start.x += smallCenter->getImageWidth();

		}  // end for i

	}  // end if
	TheDisplay->enableClipping(FALSE);
	// draw left end
	start.x = startX + xOffset;
	start.y = startY + yOffset;
	end = leftEnd;
	TheWindowManager->winDrawImage(left, start.x, start.y, end.x, end.y);

	// draw right end
	start = rightStart;
	end.x = start.x + rightSize.x;
	end.y = start.y + barWindowSize.y;
	TheWindowManager->winDrawImage(right, start.x, start.y, end.x, end.y);

}  // end drawHiliteBar

// drawListBoxText ============================================================
/** Draw the text for a listbox */
//=============================================================================
static void drawListBoxText( GameWindow *window, WinInstanceData *instData,
														 Int x, Int y, Int width, Int height,
														 Bool useImages )
{
	Int drawY;
	ListboxData *list = (ListboxData *)window->winGetUserData();
	Int i;
	Bool selected;
	Int listLineHeight;
	Color textColor;
//	W3DGameWindow *w3dWindow = static_cast<W3DGameWindow *>(window);
	IRegion2D clipRegion;
	ICoord2D start, end;

	//
	// save the clipping information region cause we're going to use it here
	// in drawing the text
	//
//	TheWindowManager->winGetClipRegion( &clipRegion );

	// set clip region to inside the outline box.
//	TheWindowManager->winSetClipRegion( x, y, width, height );

	// set the text clip region to the outline of the listbox
	clipRegion.lo.x = x + 1;
	clipRegion.lo.y = y -3;
	clipRegion.hi.x = x + width - 1;
	clipRegion.hi.y = y + height - 1;

	drawY = y - list->displayPos;

	for( i = 0; ; i++ )
	{

		if( i > 0 )
			if( list->listData[(i - 1)].listHeight > 
					(list->displayPos + list->displayHeight) )
				break;

		if( i == list->endPos )
			break;

		if( list->listData[i].listHeight < list->displayPos )
		{
			drawY += (list->listData[i].height + 1);
			continue;
		}

		listLineHeight = list->listData[i].height + 1;
		//textColor =  list->listData[i].textColor;
		selected = FALSE;

		if( list->multiSelect )
		{
			Int j = 0;

			while( list->selections[j] >= 0 )
			{
				if( i == list->selections[j] )
				{
					selected = TRUE;
					break;
				}

				j++;
			}
		}
		else
		{
			if( i == list->selectPos )
				selected = TRUE;
		}

		// this item is selected, draw the selection color or image
		if( selected )
		{
			
			if( useImages )
			{
				const Image *left, *right, *center, *smallCenter;

				if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
				{

					left				= GadgetListBoxGetDisabledSelectedItemImageLeft( window );
					right				= GadgetListBoxGetDisabledSelectedItemImageRight( window );
					center			= GadgetListBoxGetDisabledSelectedItemImageCenter( window );
					smallCenter = GadgetListBoxGetDisabledSelectedItemImageSmallCenter( window );

				}  // end if
				else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
				{

					left				= GadgetListBoxGetHiliteSelectedItemImageLeft( window );
					right				= GadgetListBoxGetHiliteSelectedItemImageRight( window );
					center			= GadgetListBoxGetHiliteSelectedItemImageCenter( window );
					smallCenter = GadgetListBoxGetHiliteSelectedItemImageSmallCenter( window );
				
				}  // end else if
				else
				{

					left				= GadgetListBoxGetEnabledSelectedItemImageLeft( window );
					right				= GadgetListBoxGetEnabledSelectedItemImageRight( window );
					center			= GadgetListBoxGetEnabledSelectedItemImageCenter( window );
					smallCenter = GadgetListBoxGetEnabledSelectedItemImageSmallCenter( window );

				}  // end else

				// draw select image across area

				//
				// where are we going to draw ... taking into account the clipping
				// region of the edge of the listbox
				//
				start.x = x;
				start.y = drawY;
				end.x = start.x + width;
				end.y = start.y + listLineHeight;

				if( end.y > clipRegion.hi.y )
					end.y = clipRegion.hi.y;
				if( start.y < clipRegion.lo.y )
					start.y = clipRegion.lo.y;

				if( left && right && center && smallCenter )
					drawHiliteBar( left, right, center, smallCenter, start.x + 1, start.y, end.x , end.y );

			}  // end if, use images
			else
			{
				Color selectColor = WIN_COLOR_UNDEFINED,
							selectBorder = WIN_COLOR_UNDEFINED;

				if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
				{
					selectColor  = GadgetListBoxGetDisabledSelectedItemColor( window );
					selectBorder = GadgetListBoxGetDisabledSelectedItemBorderColor( window );
				}  // end if, disabled
				else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
				{
					selectColor  = GadgetListBoxGetHiliteSelectedItemColor( window );
					selectBorder = GadgetListBoxGetHiliteSelectedItemBorderColor( window );
				}  // end else if, hilited
				else
				{
					selectColor  = GadgetListBoxGetEnabledSelectedItemColor( window );
					selectBorder = GadgetListBoxGetEnabledSelectedItemBorderColor( window );
				}  // end else, enabled

				// draw border

				//
				// where are we going to draw ... taking into account the clipping
				// region of the edge of the listbox
				//
				start.x = x;
				start.y = drawY;
				end.x = start.x + width;
				end.y = start.y + listLineHeight;

				if( end.y > clipRegion.hi.y )
					end.y = clipRegion.hi.y;
				if( start.y < clipRegion.lo.y )
					start.y = clipRegion.lo.y;

				if( selectBorder != WIN_COLOR_UNDEFINED )
					TheWindowManager->winOpenRect( selectBorder,	
																				 WIN_DRAW_LINE_WIDTH,
																				 start.x, start.y,
																				 end.x, end.y );

				// draw filled inner rect

				//
				// where are we going to draw ... taking into account the clipping
				// region of the edge of the listbox
				//
				start.x = x + 1;
				start.y = drawY + 1;
				end.x = start.x + width - 2;
				end.y = start.y + listLineHeight - 2;

				if( end.y > clipRegion.hi.y )
					end.y = clipRegion.hi.y;
				if( start.y < clipRegion.lo.y )
					start.y = clipRegion.lo.y;

				if( selectColor != WIN_COLOR_UNDEFINED )
					TheWindowManager->winFillRect( selectColor,
																				 WIN_DRAW_LINE_WIDTH,
																				 start.x, start.y,
																				 end.x, end.y );

			}  // end else, draw selection with colors

		}  // end if

		

		
		Color dropColor = TheWindowManager->winMakeColor( 0, 0, 0, 255 );
		DisplayString *string;

		ListEntryCell *cells = list->listData[i].cell;
		Int columnX = x;
		IRegion2D columnRegion;
		if( cells )
		{
			// loop through all the cells
			for( Int j = 0; j < list->columns; j++ )
			{
				// setup the Clip Region size

				columnRegion.lo.x = columnX;
				columnRegion.lo.y = drawY;
				if(list->columns == 1 && list->slider && list->slider->winIsHidden())
					columnRegion.hi.x = columnX + width-3;
				else
					columnRegion.hi.x = columnX + list->columnWidth[j];
				columnRegion.hi.y = drawY + list->listData[i].height;
				if(columnRegion.lo.y < clipRegion.lo.y )
					columnRegion.lo.y = clipRegion.lo.y;
				if( columnRegion.hi.y > clipRegion.hi.y )
					columnRegion.hi.y = clipRegion.hi.y;
				
				// Display the Text Case;
				if(cells[j].cellType == LISTBOX_TEXT)
				{
					textColor = cells[j].color;
					string = (DisplayString *)cells[j].data;
					if( BitIsSet( window->winGetStatus(), WIN_STATUS_ONE_LINE ) == TRUE )
					{
						string->setWordWrap(0);
						// make sure the font of the text is the same as the windows
						if( string->getFont() != window->winGetFont() )
							string->setFont( window->winGetFont() );

						// draw this text after setting the clip region for it
						string->setClipRegion( &columnRegion );
						string->draw( columnX + TEXT_X_OFFSET,
													drawY,
													textColor,
													dropColor );
									
					}
					else
					{
					
						// make sure the font of the text is the same as the windows
						if( string->getFont() != window->winGetFont() )
							string->setFont( window->winGetFont() );

						// set clip region and draw
						string->setClipRegion( &columnRegion );
						string->draw( columnX + TEXT_X_OFFSET,
													drawY,
													textColor,
													dropColor );
					}//else
				}// if
				else if(cells[j].cellType == LISTBOX_IMAGE && cells[j].data)
				{
					Int width, height;
					if (cells[j].width > 0)
						width = cells[j].width;
					else
						width = list->columnWidth[j];
					if(cells[j].height > 0)
						height = cells[j].height;
					else
						height = list->listData[i].height;
					if(j == 0)
						width--;
					Int offsetX,offsetY;
					if(width < list->columnWidth[j])
						offsetX = columnX + ((list->columnWidth[j] - width) / 2);
					else
						offsetX = columnX;
					if(height < list->listData[i].height)
						offsetY = drawY + ((list->listData[i].height - height) / 2);
					else
						offsetY = drawY;

					offsetY++;
					if(offsetX <x+1)
						offsetX = x+1;
					TheDisplay->setClipRegion( &columnRegion );					
					TheWindowManager->winDrawImage( (const Image *)cells[j].data, 
																offsetX, offsetY,
																offsetX + width, offsetY + height,cells[j].color );

				}//else
				columnX = columnX + list->columnWidth[j];
			}// for
		}//if
		

		drawY += listLineHeight;
		TheDisplay->enableClipping(FALSE);
	}

//	TheWindowManager->winSetClipRegion( clipRegion.lo.x, clipRegion.lo.y,
//																			clipRegion.hi.x, clipRegion.hi.y );

}  // end drawListBoxText

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGadgetListBoxDraw =======================================================
/** Draw colored list box using standard graphics */
//=============================================================================
void W3DGadgetListBoxDraw( GameWindow *window, WinInstanceData *instData )
{
	Int width, height, fontHeight, x, y;
	Color background, border, titleColor, titleBorder;
	ListboxData *list = (ListboxData *)window->winGetUserData();
	ICoord2D size;
	DisplayString *title = instData->getTextDisplayString();

	// get window position and size
	window->winGetScreenPosition( &x, &y );
	window->winGetSize( &size.x, &size.y );

	// get font height
	fontHeight = TheWindowManager->winFontHeight( instData->getFont() );

	// alias width and height from size
	width = size.x;
	height = size.y;

	// get the right colors
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{
		background		= GadgetListBoxGetDisabledColor( window );
		border				= GadgetListBoxGetDisabledBorderColor( window );
		titleColor		= window->winGetDisabledTextColor();
		titleBorder		= window->winGetDisabledTextBorderColor();
	}  // end if, disabled
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		background		= GadgetListBoxGetHiliteColor( window );
		border				= GadgetListBoxGetHiliteBorderColor( window );
		titleColor		= window->winGetHiliteTextColor();
		titleBorder		= window->winGetHiliteTextBorderColor();
	}  // end else if, hilited
	else
	{
		background		= GadgetListBoxGetEnabledColor( window );
		border				= GadgetListBoxGetEnabledBorderColor( window );
		titleColor		= window->winGetEnabledTextColor();
		titleBorder		= window->winGetEnabledTextBorderColor();
	}  // end else, enabled

	// Draw the title
	if( title && title->getTextLength() )
	{
	
		// set the font of this text to that of the window if not already
		if( title->getFont() != window->winGetFont() )
			title->setFont( window->winGetFont() );
			
		// draw the text
		title->draw( x + 1, y, titleColor, titleBorder );		

		y += fontHeight + 1;
		height -= fontHeight + 1;

	}  // end if

	// draw the back border
	if( border != WIN_COLOR_UNDEFINED )
		TheWindowManager->winOpenRect( border, WIN_DRAW_LINE_WIDTH,
																	 x, y, x + width, y + height );

	// draw background
	if( background != WIN_COLOR_UNDEFINED )
		TheWindowManager->winFillRect( background, WIN_DRAW_LINE_WIDTH,
																	 x + 1, y + 1, 
																	 x + width - 1, y + height - 1 );

	// If ScrollBar was requested ... adjust width.
	if( list->slider  && !list->slider->winIsHidden())
	{
		ICoord2D sliderSize;

		list->slider->winGetSize( &sliderSize.x, &sliderSize.y );
		width -= (sliderSize.x +3);

	}  // end if

	// draw the text
	drawListBoxText( window, instData, x, y + 4 , width, height-4, TRUE );

	

}  // end W3DGadgetListBoxDraw

// W3DGadgetListBoxImageDraw ==================================================
/** Draw list box with user supplied images */
//=============================================================================
void W3DGadgetListBoxImageDraw( GameWindow *window, WinInstanceData *instData )
{
	Int width, height, x, y;
	const Image *image;
	ListboxData *list = (ListboxData *)window->winGetUserData();
	ICoord2D size;
	Color titleColor, titleBorder;
	DisplayString *title = instData->getTextDisplayString();

	// get window position and size
	window->winGetScreenPosition( &x, &y );
	window->winGetSize( &size.x, &size.y );

	// save off width and height so we can change them
	width = size.x;
	height = size.y;

	// If ScrollBar was requested ... adjust width.
	if( list->slider )
	{
		ICoord2D sliderSize;

		list->slider->winGetSize( &sliderSize.x, &sliderSize.y );
		width -= sliderSize.x;

	}  // end if

	// get the image
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{
		image				= GadgetListBoxGetDisabledImage( window );
		titleColor	= window->winGetDisabledTextColor();
		titleBorder = window->winGetDisabledTextBorderColor();
	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		image				= GadgetListBoxGetHiliteImage( window );
		titleColor	= window->winGetHiliteTextColor();
		titleBorder = window->winGetHiliteTextBorderColor();
	}
	else
	{
		image				= GadgetListBoxGetEnabledImage( window );
		titleColor	= window->winGetEnabledTextColor();
		titleBorder = window->winGetEnabledTextBorderColor();
	}

	// draw the back image
	if( image )
	{
		ICoord2D start, end;

		start.x = x + instData->m_imageOffset.x;
		start.y = y + instData->m_imageOffset.y;
		end.x = start.x + width;
		end.y = start.y + height;
		TheWindowManager->winDrawImage( image, 
																		start.x, start.y, 
																		end.x, end.y );

	}  // end if

	// Draw the title
	if( title && title->getTextLength() )
	{

		// set font to font of the window if not already
		if( title->getFont() != window->winGetFont() )
			title->setFont( window->winGetFont() );

		// draw the text
		title->draw( x + 1, y, titleColor, titleBorder );

		y += TheWindowManager->winFontHeight( instData->getFont() );
		height -= TheWindowManager->winFontHeight( instData->getFont() ) + 1;

	}  // end if

	// draw the listbox text
	drawListBoxText( window, instData, x, y+4, width, height-4, TRUE );

	

}  // end W3DGadgetListBoxImageDraw

