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

// FILE: GraphDraw.h //////////////////////////////////////////////////////////
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
//	Filename: 	GraphDraw.h
//
//	author:		John McDonald
//
//	purpose:	Contains the functions to queue up and display a single graph for
//						each frame. Note: This class is presently only intended for use by
//						the Performance timers, all though it could be easily adapted for
//						other purposes.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/PerfTimer.h"
#include "Common/STLTypedefs.h"

#ifdef PERF_TIMERS

typedef std::pair<AsciiString, Real> PairAsciiStringReal;
typedef std::vector<PairAsciiStringReal> VecGraphEntries;
typedef VecGraphEntries::iterator VecGraphEntriesIt;

enum { MAX_GRAPH_VALUES = 36 };
enum { BAR_HEIGHT = 14 };
enum { BAR_SPACE = 2 };

class DisplayString;

class GraphDraw
{
	public:
		GraphDraw();
		virtual ~GraphDraw();

		void addEntry(AsciiString str, Real val);
		// Called during begin/end
		void render();
		void clear();

	protected:
		VecGraphEntries m_graphEntries;
		DisplayString *m_displayStrings[MAX_GRAPH_VALUES];
};

extern GraphDraw *TheGraphDraw;


#endif /* PERF_TIMERS */
