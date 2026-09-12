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

// FILE: W3DGUICallbacks.h ////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, August 2001
// Desc:       Callbacks for GUI elements that are specifically tied to
//						 a W3D implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

class GameWindow;
class WindowLayout;
class WinInstanceData;

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////

// Message of the day message window --------------------------------------------------------------
extern Bool W3DMainMenuDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DMainMenuFourDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DMainMenuMapBorderDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DMainMenuButtonDropShadowDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DMainMenuRandomTextDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DThinBorderDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DLeftHUDDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DRightHUDDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DPowerDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DMetalBarMenuDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DCreditsMenuDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DClockDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DShellMenuSchemeDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );

extern Bool W3DCameoMovieDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DCommandBarGridDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DCommandBarTopDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DCommandBarBackgroundDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DCommandBarForegroundDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DCommandBarGenExpDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DCommandBarHelpPopupDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DDrawMapPreviewData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DNoDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );

void W3DMainMenuInit( WindowLayout *layout, void *userData );
