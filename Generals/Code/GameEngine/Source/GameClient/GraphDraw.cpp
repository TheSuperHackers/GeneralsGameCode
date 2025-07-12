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

// FILE: GraphDraw.cpp ////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Aug 2002
//
//	Filename: 	GraphDraw.cpp
//
//	author:		John McDonald
//	
//	purpose:	Contains the functions to queue up and display a single graph for 
//						each frame
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "GameClient/GraphDraw.h"

#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"

#ifdef PERF_TIMERS

//-------------------------------------------------------------------------------------------------
GraphDraw::GraphDraw()
{
	for (Int i = 0; i < MAX_GRAPH_VALUES; ++i) {
		m_displayStrings[i] = TheDisplayStringManager->newDisplayString();
		m_displayStrings[i]->setFont(TheFontLibrary->getFont("Courier", 10, false));
	}
}

//-------------------------------------------------------------------------------------------------
GraphDraw::~GraphDraw()
{
	for (Int i = 0; i < MAX_GRAPH_VALUES; ++i) {
		TheDisplayStringManager->freeDisplayString(m_displayStrings[i]);
	}
}

//-------------------------------------------------------------------------------------------------
void GraphDraw::addEntry(AsciiString str, Real val)
{
	m_graphEntries.push_back(std::make_pair(str, val));
}

//-------------------------------------------------------------------------------------------------
void GraphDraw::render()
{
	Int width = TheDisplay->getWidth();

	// divide the width by two because we're going to use the left half of the screen for labels.
	//width /= 2;

	// give more to bars than labels. (srj)
	Int start = width * 0.33f;
	width -= start;

	Int height = TheDisplay->getHeight();

	Int totalCount = m_graphEntries.size();
	DEBUG_ASSERTCRASH(totalCount < MAX_GRAPH_VALUES, ("MAX_GRAPH_VALUES must be increased, not all labels will appear (max %d, cur %d).",MAX_GRAPH_VALUES,totalCount));
	DEBUG_ASSERTCRASH(BAR_HEIGHT * totalCount < height, ("BAR_HEIGHT must be reduced, as bars are being drawn off-screen."));
	VecGraphEntriesIt it;

	Int count = 0;
	for (it = m_graphEntries.begin(); it != m_graphEntries.end(); ++it) {
		
		if (count < MAX_GRAPH_VALUES) {
			// draw the label.
			UnicodeString uniStr;
			uniStr.translate(it->first);
		
			m_displayStrings[count]->setText(uniStr);
			m_displayStrings[count]->draw(5, count * BAR_HEIGHT, 0xFFFFFFFF, 0x00000000, 1, 1);
		}

		TheDisplay->drawFillRect(start, count * BAR_HEIGHT - (BAR_SPACE / 2), it->second / 100000.0f * width, BAR_HEIGHT - BAR_SPACE, 0x7FFFFFFF);

		++count;
	}
}

//-------------------------------------------------------------------------------------------------
void GraphDraw::clear()
{
	m_graphEntries.clear();
}

GraphDraw *TheGraphDraw = NULL;

#endif /* PERF_TIMERS */