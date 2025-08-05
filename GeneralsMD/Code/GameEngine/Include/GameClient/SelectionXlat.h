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

// FILE: SelectionXlat.h ///////////////////////////////////////////////////////////
// Author: Steven Johnson, Dec 2001

#pragma once

#ifndef _H_SelectionXlat
#define _H_SelectionXlat

#include "GameClient/InGameUI.h"

class ThingTemplate;

typedef std::map<const ThingTemplate *, int> SelectCountMap;
typedef SelectCountMap::iterator SelectCountMapIt;

//-----------------------------------------------------------------------------
class SelectionTranslator : public GameMessageTranslator
{
	// this is an evil friend wrapper due to the current limitations of Drawable iterators.
	friend Bool selectFriendsWrapper( Drawable *draw, void *userData );
	friend Bool killThemKillThemAllWrapper( Drawable *draw, void *userData );
private:

	Bool m_leftMouseButtonIsDown;
	Bool m_dragSelecting;
	UnsignedInt m_lastGroupSelTime;
	Int m_lastGroupSelGroup;
	ICoord2D m_selectFeedbackAnchor;		// Note: Used for drawing feedback only.
	ICoord2D m_deselectFeedbackAnchor;	// Note: Used for drawing feedback only.
	Bool m_displayedMaxWarning;	// did we already display a warning about selecting too many units?
	UnsignedInt m_lastClick;    ///< timer used for checking double click for type selection

	SelectCountMap m_selectCountMap;

	Coord3D m_deselectDownCameraPosition;

	Bool selectFriends( Drawable *draw, GameMessage *createTeamMsg, Bool dragSelecting );
	Bool killThemKillThemAll( Drawable *draw, GameMessage *killThemAllMsg );



public:
	SelectionTranslator();
	~SelectionTranslator();
	virtual GameMessageDisposition translateGameMessage(const GameMessage *msg);

	//Added By Sadullah Nader
	//added for fix to the drag selection when entering control bar
	//changes the mode of drag selecting to it's opposite
	void setDragSelecting(Bool dragSelect);
	void setLeftMouseButton(Bool state);

#if defined(RTS_DEBUG) || defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)
  Bool m_HandOfGodSelectionMode;
  Bool isHandOfGodSelectionMode( void) { return m_HandOfGodSelectionMode; };
#endif

};

Bool CanSelectDrawable( const Drawable *draw, Bool dragSelecting );
extern SelectionTranslator *TheSelectionTranslator;

#endif
