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

// FILE: W3DFunctionLexicon.cpp ///////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Function lexicon for w3d specific function pointers
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "GameClient/GameWindow.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/GameClient/W3DGUICallbacks.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "W3DDevice/GameClient/W3DGadget.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA
///////////////////////////////////////////////////////////////////////////////////////////////////

// Game Window draw methods -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinDrawTable [] =
{

	{ NAMEKEY_INVALID, "GameWinDefaultDraw",                  (void*)GameWinDefaultDraw },
	{ NAMEKEY_INVALID, "W3DGameWinDefaultDraw",               (void*)static_cast<GameWinDrawFunc>(&W3DGameWinDefaultDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetPushButtonDraw",             (void*)static_cast<GameWinDrawFunc>(&W3DGadgetPushButtonDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetPushButtonImageDraw",        (void*)static_cast<GameWinDrawFunc>(&W3DGadgetPushButtonImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxDraw",               (void*)static_cast<GameWinDrawFunc>(&W3DGadgetCheckBoxDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxImageDraw",          (void*)static_cast<GameWinDrawFunc>(&W3DGadgetCheckBoxImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonDraw",            (void*)static_cast<GameWinDrawFunc>(&W3DGadgetRadioButtonDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonImageDraw",       (void*)static_cast<GameWinDrawFunc>(&W3DGadgetRadioButtonImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlDraw",             (void*)static_cast<GameWinDrawFunc>(&W3DGadgetTabControlDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlImageDraw",        (void*)static_cast<GameWinDrawFunc>(&W3DGadgetTabControlImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxDraw",                (void*)static_cast<GameWinDrawFunc>(&W3DGadgetListBoxDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxImageDraw",           (void*)static_cast<GameWinDrawFunc>(&W3DGadgetListBoxImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxDraw",               (void*)static_cast<GameWinDrawFunc>(&W3DGadgetComboBoxDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxImageDraw",          (void*)static_cast<GameWinDrawFunc>(&W3DGadgetComboBoxImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderDraw",       (void*)static_cast<GameWinDrawFunc>(&W3DGadgetHorizontalSliderDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderImageDraw",  (void*)static_cast<GameWinDrawFunc>(&W3DGadgetHorizontalSliderImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderDraw",         (void*)static_cast<GameWinDrawFunc>(&W3DGadgetVerticalSliderDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderImageDraw",    (void*)static_cast<GameWinDrawFunc>(&W3DGadgetVerticalSliderImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarDraw",            (void*)static_cast<GameWinDrawFunc>(&W3DGadgetProgressBarDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarImageDraw",       (void*)static_cast<GameWinDrawFunc>(&W3DGadgetProgressBarImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextDraw",             (void*)static_cast<GameWinDrawFunc>(&W3DGadgetStaticTextDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextImageDraw",        (void*)static_cast<GameWinDrawFunc>(&W3DGadgetStaticTextImageDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryDraw",              (void*)static_cast<GameWinDrawFunc>(&W3DGadgetTextEntryDrawData) },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryImageDraw",         (void*)static_cast<GameWinDrawFunc>(&W3DGadgetTextEntryImageDrawData) },
	{ NAMEKEY_INVALID, "W3DLeftHUDDraw",                      (void*)static_cast<GameWinDrawFunc>(&W3DLeftHUDDrawData) },
	{ NAMEKEY_INVALID, "W3DCameoMovieDraw",                   (void*)static_cast<GameWinDrawFunc>(&W3DCameoMovieDrawData) },
	{ NAMEKEY_INVALID, "W3DRightHUDDraw",                     (void*)static_cast<GameWinDrawFunc>(&W3DRightHUDDrawData) },
	{ NAMEKEY_INVALID, "W3DPowerDraw",                        (void*)static_cast<GameWinDrawFunc>(&W3DPowerDrawData) },
	{ NAMEKEY_INVALID, "W3DMainMenuDraw",                     (void*)static_cast<GameWinDrawFunc>(&W3DMainMenuDrawData) },
	{ NAMEKEY_INVALID, "W3DMainMenuFourDraw",                 (void*)static_cast<GameWinDrawFunc>(&W3DMainMenuFourDrawData) },
	{ NAMEKEY_INVALID, "W3DMetalBarMenuDraw",                 (void*)static_cast<GameWinDrawFunc>(&W3DMetalBarMenuDrawData) },
	{ NAMEKEY_INVALID, "W3DCreditsMenuDraw",                  (void*)static_cast<GameWinDrawFunc>(&W3DCreditsMenuDrawData) },
	{ NAMEKEY_INVALID, "W3DClockDraw",                        (void*)static_cast<GameWinDrawFunc>(&W3DClockDrawData) },
	{ NAMEKEY_INVALID, "W3DMainMenuMapBorder",                (void*)static_cast<GameWinDrawFunc>(&W3DMainMenuMapBorderDrawData) },
	{ NAMEKEY_INVALID, "W3DMainMenuButtonDropShadowDraw",     (void*)static_cast<GameWinDrawFunc>(&W3DMainMenuButtonDropShadowDrawData) },
	{ NAMEKEY_INVALID, "W3DMainMenuRandomTextDraw",           (void*)static_cast<GameWinDrawFunc>(&W3DMainMenuRandomTextDrawData) },
	{ NAMEKEY_INVALID, "W3DThinBorderDraw",                   (void*)static_cast<GameWinDrawFunc>(&W3DThinBorderDrawData) },
	{ NAMEKEY_INVALID, "W3DShellMenuSchemeDraw",              (void*)static_cast<GameWinDrawFunc>(&W3DShellMenuSchemeDrawData) },
	{ NAMEKEY_INVALID, "W3DCommandBarBackgroundDraw",         (void*)static_cast<GameWinDrawFunc>(&W3DCommandBarBackgroundDrawData) },
	{ NAMEKEY_INVALID, "W3DCommandBarTopDraw",                (void*)static_cast<GameWinDrawFunc>(&W3DCommandBarTopDrawData) },
	{ NAMEKEY_INVALID, "W3DCommandBarGenExpDraw",             (void*)static_cast<GameWinDrawFunc>(&W3DCommandBarGenExpDrawData) },
	{ NAMEKEY_INVALID, "W3DCommandBarHelpPopupDraw",          (void*)static_cast<GameWinDrawFunc>(&W3DCommandBarHelpPopupDrawData) },
	{ NAMEKEY_INVALID, "W3DCommandBarGridDraw",               (void*)static_cast<GameWinDrawFunc>(&W3DCommandBarGridDrawData) },
	{ NAMEKEY_INVALID, "W3DCommandBarForegroundDraw",         (void*)static_cast<GameWinDrawFunc>(&W3DCommandBarForegroundDrawData) },
	{ NAMEKEY_INVALID, "W3DNoDraw",                           (void*)static_cast<GameWinDrawFunc>(&W3DNoDrawData) },
	{ NAMEKEY_INVALID, "W3DDrawMapPreview",                   (void*)W3DDrawMapPreviewData },

	{ NAMEKEY_INVALID, nullptr,                               nullptr }

};

// Game Window init methods -----------------------------------------------------------------------
static FunctionLexicon::TableEntry layoutInitTable [] =
{

	{ NAMEKEY_INVALID, "W3DMainMenuInit",  (void*)W3DMainMenuInit },

	{ NAMEKEY_INVALID, nullptr,            nullptr }

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DFunctionLexicon::W3DFunctionLexicon()
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DFunctionLexicon::~W3DFunctionLexicon()
{

}

//-------------------------------------------------------------------------------------------------
/** Initialize the function table specific for our implementations of
	* the w3d device */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::init()
{

	// extend functionality
	FunctionLexicon::init();

	// load our own tables
	loadTable( gameWinDrawTable, TABLE_GAME_WIN_DEVICEDRAW );
	loadTable( layoutInitTable, TABLE_WIN_LAYOUT_DEVICEINIT );

}

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::reset()
{

	// Pay attention to the order of what happens in the base class as you reset

	// extend
	FunctionLexicon::reset();

}

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::update()
{

	// extend?
	FunctionLexicon::update();

}


