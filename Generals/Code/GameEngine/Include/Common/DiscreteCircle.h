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

// DiscreteCircle.h ///////////////////////////////////////////////////////////////////////////////
// John K McDonald, Jr.
// September 2002
// DO NOT DISTRIBUTE

#pragma once

#ifndef __DISCRETECIRCLE_H__
#define __DISCRETECIRCLE_H__

//-------------------------------------------------------------------------------------------------
/**
	One horizontal line of the circle we are going to generate, the points drawn should be from
	(xStart, yPos)-(xEnd, yPos), inclusive.
*/
struct HorzLine
{
	Int yPos;
	Int xStart;
	Int xEnd;
};

// Vector and Iterators for the HorzLine struct.
typedef std::vector<HorzLine> VecHorzLine;
typedef VecHorzLine::iterator VecHorzLineIt;

//-------------------------------------------------------------------------------------------------

// Useful if you'd like to not have to deal with the logic of drawing the circle.
typedef void (*ScanlineDrawFunc)(Int xStart, Int xEnd, Int yPos, void *otherParms);

/**
	DiscreteCircle generates a circle centered at xCenter, yCenter, including radius. It generates
	horizontal segments for the top half of the circle only, so they need to be duplicated for the
	bottom half.
*/
class DiscreteCircle
{
	VecHorzLine m_edges;	// Should be HorzLines
	Int m_yPos;						// Used to know when to draw the bottom scanline
	Int m_yPosDoubled;		// Used to draw the bottom half of the circle.

	public:
		DiscreteCircle(Int xCenter, Int yCenter, Int radius);
		__inline const VecHorzLine &getEdges(void) const { return m_edges; }
		__inline Int getEdgeCount(void) const { return m_edges.size(); }
		void drawCircle(ScanlineDrawFunc functionToDrawWith, void *parmToPass);

	protected:
		void generateEdgePairs(Int xCenter, Int yCenter, Int radius);
		void removeDuplicates();
};

#endif /* __DISCRETECIRCLE_H__ */

