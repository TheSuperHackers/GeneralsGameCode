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

// W3DGadgetHorizontalSliderDraw ==============================================
/** Draw colored horizontal slider using standard graphics */
//=============================================================================
void W3DGadgetHorizontalSliderDraw(GameWindow* window, WinInstanceData* instData)
{
	Color backBorder, backColor;
	ICoord2D origin, size, start, end;

	// get screen position and size
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	// get the right colors
	if (BitIsSet(window->winGetStatus(), WIN_STATUS_ENABLED) == FALSE)
	{
		backBorder = GadgetSliderGetDisabledBorderColor(window);
		backColor = GadgetSliderGetDisabledColor(window);
	}
	else if (BitIsSet(instData->getState(), WIN_STATE_HILITED))
	{
		backBorder = GadgetSliderGetHiliteBorderColor(window);
		backColor = GadgetSliderGetHiliteColor(window);
	}
	else
	{
		backBorder = GadgetSliderGetEnabledBorderColor(window);
		backColor = GadgetSliderGetEnabledColor(window);
	}

	// draw background border and rect over whole control
	if (backBorder != WIN_COLOR_UNDEFINED)
	{
		start.x = origin.x;
		start.y = origin.y;
		end.x = start.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winOpenRect(backBorder, WIN_DRAW_LINE_WIDTH,
		                              start.x, start.y, end.x, end.y);
	}
	if (backColor != WIN_COLOR_UNDEFINED)
	{
		start.x = origin.x + 1;
		start.y = origin.y + 1;
		end.x = start.x + size.x - 2;
		end.y = start.y + size.y - 2;
		TheWindowManager->winFillRect(backColor, WIN_DRAW_LINE_WIDTH,
		                              start.x, start.y, end.x, end.y);
	}
}

// W3DGadgetHorizontalSliderImageDraw =========================================
/** Draw horizontal slider with user supplied images */
//=============================================================================
void W3DGadgetHorizontalSliderImageDraw(GameWindow* window,
                                        WinInstanceData* instData)
{
	const Image *fillSquare, *blankSquare, *highlightSquare;
	ICoord2D origin, size, start, end, highlightOffset;

	// get screen position and size
	window->winGetScreenPosition(&origin.x, &origin.y);
	window->winGetSize(&size.x, &size.y);

	highlightSquare = GadgetSliderGetHiliteImageLeft(window);
	blankSquare = GadgetSliderGetDisabledImageRight(window);
	fillSquare = GadgetSliderGetDisabledImageLeft(window);

	SliderData* s = (SliderData*)window->winGetUserData();

	Real xMulti = INT_TO_REAL(TheDisplay->getWidth()) / DEFAULT_DISPLAY_WIDTH;

	// figure out how many boxes we draw for this slider
	Int numBoxes = 0;
	Int numSelectedBoxes = 0;
	Int numHighlightBoxes = 0;
	Int boxWidth = fillSquare->getImageWidth() * xMulti;
	Int boxPadding = 2;
	start.x = origin.x;
	end.x = start.x + boxWidth;
	Real selectedPercent = (s->position - s->minVal) / INT_TO_REAL((s->maxVal - s->minVal));
	Int maxSelectedX = origin.x + REAL_TO_INT(selectedPercent * size.x);
	while (end.x < origin.x + size.x)
	{
		if (start.x <= maxSelectedX && end.x < origin.x + size.x && s->position != s->minVal)
		{
			++numSelectedBoxes;
		}
		start.x = end.x + boxPadding;
		end.x = start.x + boxWidth;
		++numBoxes;
	}
	numHighlightBoxes = numBoxes + 1;
	Int distanceCovered = end.x - origin.x - boxWidth;
	highlightOffset.x = -(boxWidth + boxPadding) / 2;
	highlightOffset.y = boxWidth / 3;
	Int blankness = size.x - distanceCovered;
	origin.x += blankness / 2;

	Int i;
	if (BitIsSet(instData->getState(), WIN_STATE_HILITED))
	{
		ICoord2D backgroundStart, backgroundEnd;
		backgroundStart.y = origin.y + highlightOffset.y;
		backgroundEnd.y = backgroundStart.y + boxWidth + boxPadding;
		for (i = 0; i < numHighlightBoxes; ++i)
		{
			backgroundStart.x = origin.x + highlightOffset.x + i * (boxWidth + boxPadding);
			backgroundEnd.x = backgroundStart.x + boxWidth + boxPadding;
			TheWindowManager->winDrawImage(highlightSquare,
			                               backgroundStart.x, backgroundStart.y,
			                               backgroundEnd.x, backgroundEnd.y);
		}
	}

	start.y = origin.y;
	end.y = start.y + boxWidth;
	for (i = 0; i < numSelectedBoxes; ++i)
	{
		start.x = origin.x + i * (boxWidth + boxPadding);
		end.x = start.x + boxWidth;
		TheWindowManager->winDrawImage(fillSquare,
		                               start.x, start.y,
		                               end.x, end.y);
	}
	for (i = numSelectedBoxes; i < numBoxes; ++i)
	{
		start.x = origin.x + i * (boxWidth + boxPadding);
		end.x = start.x + boxWidth;
		TheWindowManager->winDrawImage(blankSquare,
		                               start.x, start.y,
		                               end.x, end.y);
	}
}
