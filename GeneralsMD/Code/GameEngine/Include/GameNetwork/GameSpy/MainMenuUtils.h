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

// FILE: MainMenuUtils.h //////////////////////////////////////////////////////
// Author: Matthew D. Campbell, Sept 2002
// Description: GameSpy version check, patch download, etc utils
///////////////////////////////////////////////////////////////////////////////

#pragma once

void HTTPThinkWrapper( void );
void StopAsyncDNSCheck( void );
void StartPatchCheck( void );
void CancelPatchCheckCallback( void );
void StartDownloadingPatches( void );
void HandleCanceledDownload( Bool resetDropDown = TRUE );

void CheckOverallStats( void );
void HandleOverallStats( const char* szHTTPStats, unsigned len );

void CheckNumPlayersOnline( void );
void HandleNumPlayersOnline( Int numPlayersOnline );
