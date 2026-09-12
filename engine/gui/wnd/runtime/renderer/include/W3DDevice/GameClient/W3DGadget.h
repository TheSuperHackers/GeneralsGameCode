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

// FILE: W3DGadget.h //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  W3DGadget.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Implementation details for various gadgets as they pertain to
//						 W3D will go here
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Gadget.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////
/// when drawing line art for gadgets, the borders are this size
#define WIN_DRAW_LINE_WIDTH (1.0f)

// INLINING ///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// EXTERNALS //////////////////////////////////////////////////////////////////
#if !defined(ENGINE_UI_WND_RUNTIME_MODULE)
///////////////////////////////////////////////////////////////////////////////

extern Bool W3DGadgetPushButtonDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetPushButtonImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetRadioButtonDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetRadioButtonImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetTabControlDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetTabControlImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetListBoxDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetListBoxImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetComboBoxDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetComboBoxImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetHorizontalSliderDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetHorizontalSliderImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetVerticalSliderDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetVerticalSliderImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetProgressBarDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetProgressBarImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetStaticTextDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetStaticTextImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetCheckBoxDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetCheckBoxImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetTextEntryDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
extern Bool W3DGadgetTextEntryImageDrawData( GameWindow *window, WinInstanceData *instData, void *drawList );
#endif
