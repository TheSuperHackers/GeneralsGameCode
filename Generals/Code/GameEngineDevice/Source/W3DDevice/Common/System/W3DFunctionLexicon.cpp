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

	{ NAMEKEY_INVALID, "GameWinDefaultDraw",						GameWinDefaultDraw },
	{ NAMEKEY_INVALID, "W3DGameWinDefaultDraw",					W3DGameWinDefaultDraw },

	{ NAMEKEY_INVALID, "W3DGadgetPushButtonDraw",				W3DGadgetPushButtonDraw },
	{ NAMEKEY_INVALID, "W3DGadgetPushButtonImageDraw",	W3DGadgetPushButtonImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxDraw",					W3DGadgetCheckBoxDraw },
	{ NAMEKEY_INVALID, "W3DGadgetCheckBoxImageDraw",		W3DGadgetCheckBoxImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonDraw",			W3DGadgetRadioButtonDraw },
	{ NAMEKEY_INVALID, "W3DGadgetRadioButtonImageDraw",	W3DGadgetRadioButtonImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlDraw",				W3DGadgetTabControlDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTabControlImageDraw",	W3DGadgetTabControlImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxDraw",					W3DGadgetListBoxDraw },
	{ NAMEKEY_INVALID, "W3DGadgetListBoxImageDraw",			W3DGadgetListBoxImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxDraw",					W3DGadgetComboBoxDraw },
	{ NAMEKEY_INVALID, "W3DGadgetComboBoxImageDraw",			W3DGadgetComboBoxImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderDraw",				W3DGadgetHorizontalSliderDraw },
	{ NAMEKEY_INVALID, "W3DGadgetHorizontalSliderImageDraw",	W3DGadgetHorizontalSliderImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderDraw",					W3DGadgetVerticalSliderDraw },
	{ NAMEKEY_INVALID, "W3DGadgetVerticalSliderImageDraw",		W3DGadgetVerticalSliderImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarDraw",			W3DGadgetProgressBarDraw },
	{ NAMEKEY_INVALID, "W3DGadgetProgressBarImageDraw",	W3DGadgetProgressBarImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextDraw",				W3DGadgetStaticTextDraw },
	{ NAMEKEY_INVALID, "W3DGadgetStaticTextImageDraw",	W3DGadgetStaticTextImageDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryDraw",				W3DGadgetTextEntryDraw },
	{ NAMEKEY_INVALID, "W3DGadgetTextEntryImageDraw",		W3DGadgetTextEntryImageDraw },

	{ NAMEKEY_INVALID, "W3DLeftHUDDraw",								W3DLeftHUDDraw },
	{ NAMEKEY_INVALID, "W3DCameoMovieDraw",							W3DCameoMovieDraw },
	{ NAMEKEY_INVALID, "W3DRightHUDDraw",								W3DRightHUDDraw },
	{ NAMEKEY_INVALID, "W3DPowerDraw",									W3DPowerDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuDraw",								W3DMainMenuDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuFourDraw",						W3DMainMenuFourDraw },
	{ NAMEKEY_INVALID, "W3DMetalBarMenuDraw",						W3DMetalBarMenuDraw },
	{ NAMEKEY_INVALID, "W3DCreditsMenuDraw",						W3DCreditsMenuDraw },
	{ NAMEKEY_INVALID, "W3DClockDraw",									W3DClockDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuMapBorder",					W3DMainMenuMapBorder },
	{ NAMEKEY_INVALID, "W3DMainMenuButtonDropShadowDraw",	W3DMainMenuButtonDropShadowDraw },
	{ NAMEKEY_INVALID, "W3DMainMenuRandomTextDraw",			W3DMainMenuRandomTextDraw },
	{ NAMEKEY_INVALID, "W3DThinBorderDraw",							W3DThinBorderDraw },
	{ NAMEKEY_INVALID, "W3DShellMenuSchemeDraw",				W3DShellMenuSchemeDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarBackgroundDraw",		W3DCommandBarBackgroundDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarTopDraw",		W3DCommandBarTopDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarGenExpDraw",		W3DCommandBarGenExpDraw },
	{ NAMEKEY_INVALID, "W3DCommandBarHelpPopupDraw",		W3DCommandBarHelpPopupDraw },

	{ NAMEKEY_INVALID, "W3DCommandBarGridDraw",		W3DCommandBarGridDraw },


	{ NAMEKEY_INVALID, "W3DCommandBarForegroundDraw",		W3DCommandBarForegroundDraw },
	{ NAMEKEY_INVALID, "W3DNoDraw",											W3DNoDraw },
	{ NAMEKEY_INVALID, "W3DDrawMapPreview",							W3DDrawMapPreview },

	{ NAMEKEY_INVALID, NULL,														NULL },

};

// Game Window init methods -----------------------------------------------------------------------
static FunctionLexicon::TableEntry layoutInitTable [] =
{

	{ NAMEKEY_INVALID, "W3DMainMenuInit",								W3DMainMenuInit },

	{ NAMEKEY_INVALID, NULL,														NULL },

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DFunctionLexicon::W3DFunctionLexicon( void )
{

}  // end W3DFunctionLexicon

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DFunctionLexicon::~W3DFunctionLexicon( void )
{

}  // end ~W3DFunctionLexicon

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

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::reset( void )
{

	// Pay attention to the order of what happens in the base class as you reset

	// extend
	FunctionLexicon::reset();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void W3DFunctionLexicon::update( void )
{

	// extend?
	FunctionLexicon::update();

}  // end update


