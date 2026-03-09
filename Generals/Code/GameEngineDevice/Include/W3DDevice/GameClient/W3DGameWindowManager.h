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

// FILE: W3DGameWindowManager.h ///////////////////////////////////////////////////////////////////
// Created:    Colin Day, June 2001
// Desc:			 W3D implementation specific parts for the game window manager,
//						 which controls all access to the game windows for the in and
//						 of game GUI controls and windows
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameClient/GameWindowManager.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "W3DDevice/GameClient/W3DGadget.h"

//-------------------------------------------------------------------------------------------------
/** W3D implementation of the game window manager which controls all windows
	* and user interface controls */
//-------------------------------------------------------------------------------------------------
class W3DGameWindowManager : public GameWindowManager
{

public:

	W3DGameWindowManager();
	virtual ~W3DGameWindowManager() override;

	virtual void init() override;  ///< initialize the singlegon

	virtual GameWindow *allocateNewWindow() override;  ///< allocate a new game window
	virtual GameWinDrawFunc getDefaultDraw() override;  ///< return default draw func to use

	virtual GameWinDrawFunc getPushButtonImageDrawFunc() override;
	virtual GameWinDrawFunc getPushButtonDrawFunc() override;
	virtual GameWinDrawFunc getCheckBoxImageDrawFunc() override;
	virtual GameWinDrawFunc getCheckBoxDrawFunc() override;
	virtual GameWinDrawFunc getRadioButtonImageDrawFunc() override;
	virtual GameWinDrawFunc getRadioButtonDrawFunc() override;
	virtual GameWinDrawFunc getTabControlImageDrawFunc() override;
	virtual GameWinDrawFunc getTabControlDrawFunc() override;
	virtual GameWinDrawFunc getListBoxImageDrawFunc() override;
	virtual GameWinDrawFunc getListBoxDrawFunc() override;
	virtual GameWinDrawFunc getComboBoxImageDrawFunc() override;
	virtual GameWinDrawFunc getComboBoxDrawFunc() override;
	virtual GameWinDrawFunc getHorizontalSliderImageDrawFunc() override;
	virtual GameWinDrawFunc getHorizontalSliderDrawFunc() override;
	virtual GameWinDrawFunc getVerticalSliderImageDrawFunc() override;
	virtual GameWinDrawFunc getVerticalSliderDrawFunc() override;
	virtual GameWinDrawFunc getProgressBarImageDrawFunc() override;
	virtual GameWinDrawFunc getProgressBarDrawFunc() override;
	virtual GameWinDrawFunc getStaticTextImageDrawFunc() override;
	virtual GameWinDrawFunc getStaticTextDrawFunc() override;
	virtual GameWinDrawFunc getTextEntryImageDrawFunc() override;
	virtual GameWinDrawFunc getTextEntryDrawFunc() override;

protected:

};

// INLINE //////////////////////////////////////////////////////////////////////////////////////////
inline GameWindow *W3DGameWindowManager::allocateNewWindow() { return newInstance(W3DGameWindow); }
inline GameWinDrawFunc W3DGameWindowManager::getDefaultDraw() { return W3DGameWinDefaultDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getPushButtonImageDrawFunc() { return W3DGadgetPushButtonImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getPushButtonDrawFunc() { return W3DGadgetPushButtonDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getCheckBoxImageDrawFunc() { return W3DGadgetCheckBoxImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getCheckBoxDrawFunc() { return W3DGadgetCheckBoxDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getRadioButtonImageDrawFunc() { return W3DGadgetRadioButtonImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getRadioButtonDrawFunc() { return W3DGadgetRadioButtonDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getTabControlImageDrawFunc() { return W3DGadgetTabControlImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getTabControlDrawFunc() { return W3DGadgetTabControlDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getListBoxImageDrawFunc() { return W3DGadgetListBoxImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getListBoxDrawFunc() { return W3DGadgetListBoxDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getComboBoxImageDrawFunc() { return W3DGadgetComboBoxImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getComboBoxDrawFunc() { return W3DGadgetComboBoxDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getHorizontalSliderImageDrawFunc() { return W3DGadgetHorizontalSliderImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getHorizontalSliderDrawFunc() { return W3DGadgetHorizontalSliderDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getVerticalSliderImageDrawFunc() { return W3DGadgetVerticalSliderImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getVerticalSliderDrawFunc() { return W3DGadgetVerticalSliderDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getProgressBarImageDrawFunc() { return W3DGadgetProgressBarImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getProgressBarDrawFunc() { return W3DGadgetProgressBarDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getStaticTextImageDrawFunc() { return W3DGadgetStaticTextImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getStaticTextDrawFunc() { return W3DGadgetStaticTextDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getTextEntryImageDrawFunc() { return W3DGadgetTextEntryImageDraw; }
inline GameWinDrawFunc W3DGameWindowManager::getTextEntryDrawFunc() { return W3DGadgetTextEntryDraw; }
