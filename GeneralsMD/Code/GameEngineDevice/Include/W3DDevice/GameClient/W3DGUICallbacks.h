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
extern void W3DLeftHUDDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DCameoMovieDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DRightHUDDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DPowerDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DMainMenuDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DMainMenuFourDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DMetalBarMenuDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DCreditsMenuDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DClockDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DMainMenuMapBorder( GameWindow *window, WinInstanceData *instData );
extern void W3DMainMenuButtonDropShadowDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DMainMenuRandomTextDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DThinBorderDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DShellMenuSchemeDraw( GameWindow *window, WinInstanceData *instData );

extern void W3DCommandBarGridDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DCommandBarGenExpDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DCommandBarHelpPopupDraw( GameWindow *window, WinInstanceData *instData );

extern void W3DCommandBarBackgroundDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DCommandBarForegroundDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DCommandBarTopDraw( GameWindow *window, WinInstanceData *instData );

extern void W3DNoDraw( GameWindow *window, WinInstanceData *instData );
extern void W3DDrawMapPreview( GameWindow *window, WinInstanceData *instData );

void W3DMainMenuInit( WindowLayout *layout, void *userData );
