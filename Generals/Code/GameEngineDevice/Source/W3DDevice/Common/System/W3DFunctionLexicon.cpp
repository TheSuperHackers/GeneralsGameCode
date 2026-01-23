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
	{ NAMEKEY_INVALID, "W3DGameWinDefaultDraw",               (void*)W3DGameWinDefaultDraw },

	{ NAMEKEY_INVALID, "W3DGadgetPushButtonDraw",             (void*)W3DGadgetPushButtonDraw },
	{ NAMEKEY_INVALID, "W3DGadgetPushButtonImageDraw",        (void*)W3DGadgetPushButtonImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxDraw",               (void*)W3DGadgetCheckBoxDraw },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxImageDraw",          (void*)W3DGadgetCheckBoxImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonDraw",            (void*)W3DGadgetRadioButtonDraw },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonImageDraw",       (void*)W3DGadgetRadioButtonImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlDraw",             (void*)W3DGadgetTabControlDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlImageDraw",        (void*)W3DGadgetTabControlImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxDraw",                (void*)W3DGadgetListBoxDraw },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxImageDraw",           (void*)W3DGadgetListBoxImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxDraw",               (void*)W3DGadgetComboBoxDraw },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxImageDraw",          (void*)W3DGadgetComboBoxImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderDraw",       (void*)W3DGadgetHorizontalSliderDraw },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderImageDraw",  (void*)W3DGadgetHorizontalSliderImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderDraw",         (void*)W3DGadgetVerticalSliderDraw },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderImageDraw",    (void*)W3DGadgetVerticalSliderImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarDraw",            (void*)W3DGadgetProgressBarDraw },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarImageDraw",       (void*)W3DGadgetProgressBarImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextDraw",             (void*)W3DGadgetStaticTextDraw },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextImageDraw",        (void*)W3DGadgetStaticTextImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryDraw",              (void*)W3DGadgetTextEntryDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryImageDraw",         (void*)W3DGadgetTextEntryImageDraw },

	{ NAMEKEY_INVALID, "W3DLeftHUDDraw",                      (void*)W3DLeftHUDDraw },
	{ NAMEKEY_INVALID, "W3DCameoMovieDraw",                   (void*)W3DCameoMovieDraw },
	{ NAMEKEY_INVALID, "W3DRightHUDDraw",                     (void*)W3DRightHUDDraw },
	{ NAMEKEY_INVALID, "W3DPowerDraw",                        (void*)W3DPowerDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuDraw",                     (void*)W3DMainMenuDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuFourDraw",                 (void*)W3DMainMenuFourDraw },
	{ NAMEKEY_INVALID, "W3DMetalBarMenuDraw",                 (void*)W3DMetalBarMenuDraw },
	{ NAMEKEY_INVALID, "W3DCreditsMenuDraw",                  (void*)W3DCreditsMenuDraw },
	{ NAMEKEY_INVALID, "W3DClockDraw",                        (void*)W3DClockDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuMapBorder",                (void*)W3DMainMenuMapBorder },
	{ NAMEKEY_INVALID, "W3DMainMenuButtonDropShadowDraw",     (void*)W3DMainMenuButtonDropShadowDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuRandomTextDraw",           (void*)W3DMainMenuRandomTextDraw },
	{ NAMEKEY_INVALID, "W3DThinBorderDraw",                   (void*)W3DThinBorderDraw },
	{ NAMEKEY_INVALID, "W3DShellMenuSchemeDraw",              (void*)W3DShellMenuSchemeDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarBackgroundDraw",         (void*)W3DCommandBarBackgroundDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarTopDraw",                (void*)W3DCommandBarTopDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarGenExpDraw",             (void*)W3DCommandBarGenExpDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarHelpPopupDraw",          (void*)W3DCommandBarHelpPopupDraw },

	{ NAMEKEY_INVALID, "W3DCommandBarGridDraw",               (void*)W3DCommandBarGridDraw },


	{ NAMEKEY_INVALID, "W3DCommandBarForegroundDraw",         (void*)W3DCommandBarForegroundDraw },
	{ NAMEKEY_INVALID, "W3DNoDraw",                           (void*)W3DNoDraw },
	{ NAMEKEY_INVALID, "W3DDrawMapPreview",                   (void*)W3DDrawMapPreview },

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
W3DFunctionLexicon::W3DFunctionLexicon( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DFunctionLexicon::~W3DFunctionLexicon( void )
{

}

//-------------------------------------------------------------------------------------------------
/** Initialize the function table specific for our implementations of
	* the w3d device */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::init( void )
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
void W3DFunctionLexicon::reset( void )
{

	// Pay attention to the order of what happens in the base class as you reset

	// extend
	FunctionLexicon::reset();

}

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::update( void )
{

	// extend?
	FunctionLexicon::update();

}


