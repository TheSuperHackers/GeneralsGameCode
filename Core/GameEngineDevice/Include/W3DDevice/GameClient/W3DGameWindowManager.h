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
#include "W3DDevice/GameClient/W3DGUICallbacks.h"

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
	virtual GameWinDrawDataFunc getBorderDrawDataFunc() override;

protected:

};

// INLINE //////////////////////////////////////////////////////////////////////////////////////////
inline GameWindow *W3DGameWindowManager::allocateNewWindow() { return newInstance(W3DGameWindow); }
inline GameWinDrawFunc W3DGameWindowManager::getDefaultDraw() { return W3DGameWinDefaultDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getPushButtonImageDrawFunc() { return W3DGadgetPushButtonImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getPushButtonDrawFunc() { return W3DGadgetPushButtonDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getCheckBoxImageDrawFunc() { return W3DGadgetCheckBoxImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getCheckBoxDrawFunc() { return W3DGadgetCheckBoxDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getRadioButtonImageDrawFunc() { return W3DGadgetRadioButtonImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getRadioButtonDrawFunc() { return W3DGadgetRadioButtonDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getTabControlImageDrawFunc() { return W3DGadgetTabControlImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getTabControlDrawFunc() { return W3DGadgetTabControlDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getListBoxImageDrawFunc() { return W3DGadgetListBoxImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getListBoxDrawFunc() { return W3DGadgetListBoxDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getComboBoxImageDrawFunc() { return W3DGadgetComboBoxImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getComboBoxDrawFunc() { return W3DGadgetComboBoxDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getHorizontalSliderImageDrawFunc() { return W3DGadgetHorizontalSliderImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getHorizontalSliderDrawFunc() { return W3DGadgetHorizontalSliderDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getVerticalSliderImageDrawFunc() { return W3DGadgetVerticalSliderImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getVerticalSliderDrawFunc() { return W3DGadgetVerticalSliderDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getProgressBarImageDrawFunc() { return W3DGadgetProgressBarImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getProgressBarDrawFunc() { return W3DGadgetProgressBarDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getStaticTextImageDrawFunc() { return W3DGadgetStaticTextImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getStaticTextDrawFunc() { return W3DGadgetStaticTextDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getTextEntryImageDrawFunc() { return W3DGadgetTextEntryImageDrawData; }
inline GameWinDrawFunc W3DGameWindowManager::getTextEntryDrawFunc() { return W3DGadgetTextEntryDrawData; }
inline GameWinDrawDataFunc W3DGameWindowManager::getBorderDrawDataFunc() { return W3DGameWinBorderDrawData; }
