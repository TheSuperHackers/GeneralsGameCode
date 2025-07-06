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
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetSlider.h"
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
#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// W3DGadgetHorizontalSliderDraw ==============================================
/** Draw colored horizontal slider using standard graphics */
//=============================================================================
void W3DGadgetHorizontalSliderDraw( GameWindow *window, WinInstanceData *instData )
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

	}  // end if, disabled
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		backBorder		= GadgetSliderGetHiliteBorderColor( window );
		backColor			= GadgetSliderGetHiliteColor( window );

	}  // end else if, hilited
	else
	{

		backBorder		= GadgetSliderGetEnabledBorderColor( window );
		backColor			= GadgetSliderGetEnabledColor( window );

	}  // end else, enabled

	// draw background border and rect over whole control
	if( backBorder != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x;
		start.y = origin.y;
		end.x = start.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winOpenRect( backBorder, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}  // end if
	if( backColor != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x + 1;
		start.y = origin.y + 1;
		end.x = start.x + size.x - 2;
		end.y = start.y + size.y - 2;
		TheWindowManager->winFillRect( backColor, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}  // end if


}  // end W3DGadgetHorizontalSliderDraw

// W3DGadgetHorizontalSliderImageDraw =========================================
/** Draw horizontal slider with user supplied images */
//=============================================================================
void W3DGadgetHorizontalSliderImageDraw( GameWindow *window, 
																				 WinInstanceData *instData )
{
	const Image *fillSquare, *blankSquare, *highlightSquare;
	ICoord2D origin, size, start, end, highlightOffset;

	// get screen position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	highlightSquare = GadgetSliderGetHiliteImageLeft( window );
	blankSquare	= GadgetSliderGetDisabledImageRight( window );
	fillSquare = GadgetSliderGetDisabledImageLeft( window );

	SliderData *s = (SliderData *)window->winGetUserData();
	
	Real xMulti = INT_TO_REAL(TheDisplay->getWidth()) / 800;

	// figure out how many boxes we draw for this slider
	Int numBoxes = 0;
	Int numSelectedBoxes = 0;
	Int numHighlightBoxes = 0;
	Int boxWidth = fillSquare->getImageWidth()* xMulti;
	Int boxPadding = 2;
	start.x = origin.x;
	end.x	= start.x + boxWidth;
	Real selectedPercent = (s->position - s->minVal)/INT_TO_REAL((s->maxVal - s->minVal));
	Int maxSelectedX = origin.x + REAL_TO_INT(selectedPercent * size.x);
	while(end.x < origin.x + size.x )
	{
		if (start.x <= maxSelectedX && end.x < origin.x + size.x && s->position != s->minVal)
			++numSelectedBoxes;
		start.x = end.x + boxPadding;
		end.x	= start.x + boxWidth;
		++numBoxes;
	}
	numHighlightBoxes = numBoxes + 1;
	Int distanceCovered = end.x - origin.x - boxWidth;
	highlightOffset.x = -(boxWidth + boxPadding)/2;
	highlightOffset.y = boxWidth/3;
	Int blankness = size.x - distanceCovered;
	origin.x += blankness/2;

	Int i;
	if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		ICoord2D backgroundStart, backgroundEnd;
		backgroundStart.y = origin.y + highlightOffset.y;
		backgroundEnd.y = backgroundStart.y + boxWidth + boxPadding;
		for (i=0; i<numHighlightBoxes; ++i)
		{
			backgroundStart.x = origin.x + highlightOffset.x + i*(boxWidth+boxPadding);
			backgroundEnd.x = backgroundStart.x + boxWidth + boxPadding;
			TheWindowManager->winDrawImage( highlightSquare, 
																		backgroundStart.x, backgroundStart.y,
																		backgroundEnd.x, backgroundEnd.y );
		}
	}

	start.y = origin.y;
	end.y = start.y + boxWidth;
	for (i=0; i<numSelectedBoxes; ++i)
	{
		start.x = origin.x + i*(boxWidth+boxPadding);
		end.x = start.x + boxWidth;
		TheWindowManager->winDrawImage( fillSquare, 
																	start.x, start.y,
																	end.x, end.y );
	}
	for (i=numSelectedBoxes; i<numBoxes; ++i)
	{
		start.x = origin.x + i*(boxWidth+boxPadding);
		end.x = start.x + boxWidth;
		TheWindowManager->winDrawImage( blankSquare, 
																	start.x, start.y,
																	end.x, end.y );
	}
}

// W3DGadgetHorizontalSliderImageDraw =========================================
/** Draw horizontal slider with user supplied images */
//=============================================================================
void W3DGadgetHorizontalSliderImageDrawB( GameWindow *window, 
																				 WinInstanceData *instData )
{
	const Image *fillSquare, *blankSquare, *highlightSquare;//, *progressArrow;
	ICoord2D origin, size, start, end;
	Int xOffset, yOffset;

	// get screen position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	SliderData *s = (SliderData *)window->winGetUserData();
	
	Real xMulti = INT_TO_REAL(TheDisplay->getWidth()) / 800;
	Real yMulti = INT_TO_REAL(TheDisplay->getHeight())/ 600;
	// get image offset
	xOffset = instData->m_imageOffset.x;
	yOffset = instData->m_imageOffset.y;

	UnicodeString tooltip, tmp;
	tooltip.format(L"mult:%g/%g, img offset:%d,%d", xMulti, yMulti, xOffset, yOffset);

	tmp.format(L"\norigin: %d,%d size:%d,%d", origin.x, origin.y, size.x, size.y);
	tooltip.concat(tmp);

	tmp.format(L"\ns= %d <--> %d, numTicks=%g, pos = %d", s->minVal, s->maxVal, s->numTicks, s->position);
	tooltip.concat(tmp);

	if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		highlightSquare					= GadgetSliderGetHiliteImageLeft( window );
		ICoord2D backgroundStart, backgroundEnd;
		backgroundStart.x = origin.x - (highlightSquare->getImageWidth() * xMulti)/2;
		backgroundStart.y = origin.y + (highlightSquare->getImageHeight() *yMulti)/3;
		backgroundEnd.y = backgroundStart.y + highlightSquare->getImageHeight()* yMulti;
		backgroundEnd.x = backgroundStart.x + highlightSquare->getImageWidth() * xMulti;

		tmp.format(L"\nHighlighted: (%d,%d) -> (%d,%d), step %d/%g, full %d/%d", backgroundStart.x, backgroundStart.y,
			backgroundEnd.x, backgroundEnd.y, highlightSquare->getImageWidth(), highlightSquare->getImageWidth() * xMulti,
			origin.x, size.x);
		tooltip.concat(tmp);

		while(backgroundStart.x < origin.x + size.x)
		{
			TheWindowManager->winDrawImage( highlightSquare, 
																		backgroundStart.x, backgroundStart.y,
																		backgroundEnd.x, backgroundEnd.y );
			backgroundStart.x = backgroundEnd.x;
			backgroundEnd.x = backgroundStart.x + highlightSquare->getImageWidth() * xMulti;
		}		
		tmp.format(L"\n  bsX = %d, beX = %d (%d < %d+%d or %d?)", backgroundStart.x, backgroundEnd.x,
			backgroundStart.x, origin.x, size.x, origin.x + size.x);
		tooltip.concat(tmp);
	}

	fillSquare = GadgetSliderGetDisabledImageLeft( window );
	start.x = origin.x;
	start.y = origin.y;
	end.y = start.y + fillSquare->getImageHeight() * yMulti;
	end.x	= start.x + fillSquare->getImageWidth()* xMulti;

	tmp.format(L"\ntop: start=%d,%d, end=%d,%d", start.x, start.y, end.x, end.y);
	tooltip.concat(tmp);

	while(start.x <= origin.x + (s->numTicks * (s->position - s->minVal)) && end.x < origin.x + size.x && s->position != s->minVal)
	{
		TheWindowManager->winDrawImage( fillSquare, 
																		start.x, start.y,
																		end.x, end.y );
		start.x = end.x + 2;
		end.x	= start.x + fillSquare->getImageWidth()* xMulti;

	}

	blankSquare	= GadgetSliderGetDisabledImageRight( window );
	end.x	= start.x + blankSquare->getImageWidth()* xMulti;

	while(end.x < origin.x + size.x )
	{
		TheWindowManager->winDrawImage( blankSquare, 
																		start.x, start.y,
																		end.x, end.y );
		start.x = end.x + 2;
		end.x	= start.x + blankSquare->getImageWidth()* xMulti;
	}

	instData->setTooltipText(tooltip);

//	if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
//	{
//		progressArrow				= GadgetSliderGetHiliteImageRight( window );
//		if(!progressArrow)
//			return;
//		Int transPos = (s->numTicks * (s->position - s->minVal)) - progressArrow->getImageWidth() /2;
//		start.x = origin.x + transPos;
//		start.y = origin.y + fillSquare->getImageHeight()/3*2;
//		end.y = start.y + progressArrow->getImageHeight();
//		end.x	= start.x + progressArrow->getImageWidth();
//		TheWindowManager->winDrawImage( progressArrow, 
//																			start.x, start.y,
//																			end.x, end.y );
//	}
}


// W3DGadgetHorizontalSliderImageDraw =========================================
/** Draw horizontal slider with user supplied images */
//=============================================================================
void W3DGadgetHorizontalSliderImageDrawA( GameWindow *window, 
																				 WinInstanceData *instData )
{
	const Image *leftImageLeft, *rightImageLeft, *centerImageLeft, *smallCenterImageLeft;
	const Image *leftImageRight, *rightImageRight, *centerImageRight, *smallCenterImageRight;
	ICoord2D origin, size, start, end;
	Int xOffset, yOffset;
	Int i;

	// get screen position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	SliderData *s = (SliderData *)window->winGetUserData();
	Int transPos = (s->numTicks * (s->position - s->minVal)) + HORIZONTAL_SLIDER_THUMB_WIDTH/2;
	IRegion2D clipLeft, clipRight;

	// get image offset
	xOffset = instData->m_imageOffset.x;
	yOffset = instData->m_imageOffset.y;

	// get the right images
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		leftImageRight = leftImageLeft					= GadgetSliderGetDisabledImageLeft( window );
		rightImageRight = rightImageLeft				= GadgetSliderGetDisabledImageRight( window );
//		centerImageRight = centerImageLeft				= GadgetSliderGetDisabledImageCenter( window );
//		smallCenterImageRight = smallCenterImageLeft	= GadgetSliderGetDisabledImageSmallCenter( window );

	}  // end if, disabled
	else //if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		leftImageLeft					= GadgetSliderGetHiliteImageLeft( window );
		rightImageLeft				= GadgetSliderGetHiliteImageRight( window );
		centerImageLeft				= GadgetSliderGetHiliteImageCenter( window );
		smallCenterImageLeft	= GadgetSliderGetHiliteImageSmallCenter( window );

		leftImageRight					= GadgetSliderGetEnabledImageLeft( window );
		rightImageRight				= GadgetSliderGetEnabledImageRight( window );
		centerImageRight				= GadgetSliderGetEnabledImageCenter( window );
		smallCenterImageRight	= GadgetSliderGetEnabledImageSmallCenter( window );

	}  // end else, enabled

	// sanity, we need to have these images to make it look right
	if( leftImageLeft == NULL || rightImageLeft == NULL || 
			centerImageLeft == NULL || smallCenterImageLeft == NULL ||
			leftImageRight == NULL || rightImageRight == NULL || 
			centerImageRight == NULL || smallCenterImageRight == NULL )
		return;

	// get image sizes for the ends
	ICoord2D leftSize, rightSize;
	leftSize.x = leftImageLeft->getImageWidth();
	leftSize.y = leftImageLeft->getImageHeight();
	rightSize.x = rightImageLeft->getImageWidth();
	rightSize.y = rightImageLeft->getImageHeight();

	// get two key points used in the end drawing
	ICoord2D leftEnd, rightStart;
	leftEnd.x = origin.x + leftSize.x + xOffset;
	leftEnd.y = origin.y + size.y + yOffset;
	rightStart.x = origin.x + size.x - rightSize.x + xOffset;
	rightStart.y = origin.y  + size.y - leftSize.y + yOffset;



	// draw the center repeating bar
	Int centerWidth, pieces;

	// get width we have to draw our repeating center in
	centerWidth = rightStart.x - leftEnd.x;

	// how many whole repeating pieces will fit in that width
	pieces = centerWidth / centerImageLeft->getImageWidth();

	// draw the pieces
	start.x = leftEnd.x;
	start.y = origin.y + size.y - leftSize.y + yOffset;
	end.y =origin.y + size.y + yOffset;
	
	clipLeft.lo.x = origin.x;
	clipLeft.lo.y = rightStart.y;
	clipLeft.hi.y = leftEnd.y;
	clipLeft.hi.x = origin.x + transPos;
	clipRight.lo.x = origin.x + transPos;
	clipRight.lo.y = rightStart.y;
	clipRight.hi.y = leftEnd.y;
	clipRight.hi.x = origin.x + size.x;


	for( i = 0; i < pieces; i++ )
	{

		end.x = start.x + centerImageLeft->getImageWidth();
		TheDisplay->setClipRegion(&clipLeft);
		TheWindowManager->winDrawImage( centerImageLeft, 
																		start.x, start.y,
																		end.x, end.y );
		TheDisplay->setClipRegion(&clipRight);
		TheWindowManager->winDrawImage( centerImageRight, 
																		start.x, start.y,
																		end.x, end.y );
		start.x += centerImageLeft->getImageWidth();

	}  // end for i
	
	//
	// how many small repeating pieces will fit in the gap from where the
	// center repeating bar stopped and the right image, draw them
	// and overlapping underneath where the right end will go
	//
	centerWidth = rightStart.x - start.x;
	pieces = centerWidth / smallCenterImageLeft->getImageWidth() + 1;
	end.y = leftEnd.y;//start.y + smallCenterImageLeft->getImageHeight();
	for( i = 0; i < pieces; i++ )
	{

		end.x = start.x + smallCenterImageLeft->getImageWidth();
		TheDisplay->setClipRegion(&clipLeft);
		TheWindowManager->winDrawImage( smallCenterImageLeft,
																		start.x, start.y,
																		end.x, end.y );
		TheDisplay->setClipRegion(&clipRight);
		TheWindowManager->winDrawImage( smallCenterImageRight,
																		start.x, start.y,
																		end.x, end.y );
		start.x += smallCenterImageLeft->getImageWidth();

	}  // end for i
	
	// draw left end
	start.x = origin.x + xOffset;
	start.y = rightStart.y;
	end = leftEnd;
	TheDisplay->setClipRegion(&clipLeft);
	TheWindowManager->winDrawImage(leftImageLeft, start.x, start.y, end.x, end.y);
	TheDisplay->setClipRegion(&clipRight);
	TheWindowManager->winDrawImage(leftImageRight, start.x, start.y, end.x, end.y);
	// draw right end
	start = rightStart;
	end.x = start.x + rightSize.x;
	end.y = leftEnd.y;
	TheDisplay->setClipRegion(&clipLeft);
	TheWindowManager->winDrawImage(rightImageLeft, start.x, start.y, end.x, end.y);
	TheDisplay->setClipRegion(&clipRight);
	TheWindowManager->winDrawImage(rightImageRight, start.x, start.y, end.x, end.y);

	TheDisplay->enableClipping(FALSE);
}  // end W3DGadgetHorizontalSliderImageDraw

