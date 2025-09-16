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

// FILE: LobbyUtils.h //////////////////////////////////////////////////////
// Generals lobby utils
// Author: Matthew D. Campbell, Sept 2002

#pragma once

#ifndef __LOBBYUTILS_H__
#define __LOBBYUTILS_H__

class GameWindow;

GameWindow *GetGameListBox( void );
GameWindow *GetGameInfoListBox( void );
NameKeyType GetGameListBoxID( void );
NameKeyType GetGameInfoListBoxID( void );
void GrabWindowInfo( void );
void ReleaseWindowInfo( void );
void RefreshGameInfoListBox( GameWindow *mainWin, GameWindow *win );
void RefreshGameListBoxes( void );
void ToggleGameListType( void );

void playerTemplateComboBoxTooltip(GameWindow *wndComboBox, WinInstanceData *instData, UnsignedInt mouse);
void playerTemplateListBoxTooltip(GameWindow *wndListBox, WinInstanceData *instData, UnsignedInt mouse);

enum GameSortType CPP_11(: Int)
{
	GAMESORT_ALPHA_ASCENDING = 0,
	GAMESORT_ALPHA_DESCENDING,
	GAMESORT_PING_ASCENDING,
	GAMESORT_PING_DESCENDING,
};

Bool HandleSortButton( NameKeyType sortButton );
void PopulateLobbyPlayerListbox(void);

#endif // __LOBBYUTILS_H__
