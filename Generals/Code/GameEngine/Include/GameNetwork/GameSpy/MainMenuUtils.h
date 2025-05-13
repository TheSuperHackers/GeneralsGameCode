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

// FILE: MainMenuUtils.h //////////////////////////////////////////////////////
// Author: Matthew D. Campbell, Sept 2002
// Description: GameSpy version check, patch download, etc utils
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __MAINMENUUTILS_H__
#define __MAINMENUUTILS_H__

void HTTPThinkWrapper( void );
void StopAsyncDNSCheck( void );
void StartPatchCheck( void );
void CancelPatchCheckCallback( void );
void StartDownloadingPatches( void );
void HandleCanceledDownload( Bool resetDropDown = TRUE );

enum OverallStatsPeriod CPP_11(: Int)
{
	STATS_TODAY = 0,
	STATS_YESTERDAY,
	STATS_ALLTIME,
	STATS_LASTWEEK,
	STATS_MAX
};

struct OverallStats
{
	OverallStats();
	Int wins[STATS_MAX];
	Int losses[STATS_MAX];
};

void CheckOverallStats( void );
void HandleOverallStats( const OverallStats& USA, const OverallStats& China, const OverallStats& GLA );

void CheckNumPlayersOnline( void );
void HandleNumPlayersOnline( Int numPlayersOnline );

#endif // __MAINMENUUTILS_H__
