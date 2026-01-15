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

// FILE: DifficultySelect.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Nov 2002
//
//	Filename: 	DifficultySelect.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	The popup campaign difficulty select
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/UserPreferences.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/CampaignManager.h"

#include "GameLogic/ScriptEngine.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
static GameDifficulty s_AIDiff = DIFFICULTY_NORMAL;
static NameKeyType    buttonOkID      = NAMEKEY_INVALID;
static GameWindow *   buttonOk       = nullptr;
static NameKeyType    buttonCancelID      = NAMEKEY_INVALID;
static GameWindow *   buttonCancel       = nullptr;
static NameKeyType    radioButtonEasyAIID      = NAMEKEY_INVALID;
static NameKeyType    radioButtonMediumAIID      = NAMEKEY_INVALID;
static NameKeyType    radioButtonHardAIID      = NAMEKEY_INVALID;
static GameWindow *   radioButtonEasyAI       = nullptr;
static GameWindow *   radioButtonMediumAI       = nullptr;
static GameWindow *   radioButtonHardAI       = nullptr;

void setupGameStart(AsciiString mapName, GameDifficulty diff);
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
static void SetDifficultyRadioButton( void )
{
	OptionPreferences pref;
	if (!TheScriptEngine)
	{
		s_AIDiff = DIFFICULTY_NORMAL;
	}
	else
	{
		switch (pref.getCampaignDifficulty())
		{
			case DIFFICULTY_EASY:
			{
				GadgetRadioSetSelection(radioButtonEasyAI, FALSE);
				s_AIDiff = DIFFICULTY_EASY;
				break;
			}
			case DIFFICULTY_NORMAL:
			{
				GadgetRadioSetSelection(radioButtonMediumAI, FALSE);
				s_AIDiff = DIFFICULTY_NORMAL;
				break;
			}
			case DIFFICULTY_HARD:
			{
				GadgetRadioSetSelection(radioButtonHardAI, FALSE);
				s_AIDiff = DIFFICULTY_HARD;
				break;
			}

		default:
			{
				DEBUG_CRASH(("unrecognized difficulty level in the script engine"));
			}

		}
	}

}


void DifficultySelectInit( WindowLayout *layout, void *userData )
{
	NameKeyType parentID = TheNameKeyGenerator->nameToKey( "DifficultySelect.wnd:DifficultySelectParent" );
	GameWindow *parent = TheWindowManager->winGetWindowFromId( nullptr, parentID );

	buttonOkID = TheNameKeyGenerator->nameToKey( "DifficultySelect.wnd:ButtonOk" );
	buttonOk = TheWindowManager->winGetWindowFromId( parent, buttonOkID );
	buttonCancelID = TheNameKeyGenerator->nameToKey( "DifficultySelect.wnd:ButtonCancel" );
	buttonCancel = TheWindowManager->winGetWindowFromId( parent, buttonCancelID );
	radioButtonEasyAIID = TheNameKeyGenerator->nameToKey( "DifficultySelect.wnd:RadioButtonEasy" );
	radioButtonEasyAI = TheWindowManager->winGetWindowFromId( parent, radioButtonEasyAIID );
	radioButtonMediumAIID = TheNameKeyGenerator->nameToKey( "DifficultySelect.wnd:RadioButtonMedium" );
	radioButtonMediumAI = TheWindowManager->winGetWindowFromId( parent, radioButtonMediumAIID );
	radioButtonHardAIID = TheNameKeyGenerator->nameToKey( "DifficultySelect.wnd:RadioButtonHard" );
	radioButtonHardAI = TheWindowManager->winGetWindowFromId( parent, radioButtonHardAIID );

	s_AIDiff = DIFFICULTY_NORMAL;
	SetDifficultyRadioButton();
	// set keyboard focus to main parent
//	AsciiString parentName( "SkirmishMapSelectMenu.wnd:SkrimishMapSelectMenuParent" );
//	NameKeyType parentID = TheNameKeyGenerator->nameToKey( parentName );
//	parent = TheWindowManager->winGetWindowFromId( nullptr, parentID );
//
//	TheWindowManager->winSetFocus( parent );
//
	parent->winBringToTop();
	TheWindowManager->winSetModal(parent);

}


//-------------------------------------------------------------------------------------------------
/** Map select menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DifficultySelectInput( GameWindow *window, UnsignedInt msg,
																				 WindowMsgData mData1, WindowMsgData mData2 )
{

//	switch( msg )
//	{
//
//		// --------------------------------------------------------------------------------------------
//		case GWM_CHAR:
//		{
//			UnsignedByte key = mData1;
//			UnsignedByte state = mData2;
//
//			switch( key )
//			{
//
//				// ----------------------------------------------------------------------------------------
//				case KEY_ESC:
//				{
//
//					//
//					// send a simulated selected event to the parent window of the
//					// back/exit button
//					//
//					if( BitIsSet( state, KEY_STATE_UP ) )
//					{
//						AsciiString buttonName( "SkirmishMapSelectMenu.wnd:ButtonBack" );
//						NameKeyType buttonID = TheNameKeyGenerator->nameToKey( buttonName );
//						GameWindow *button = TheWindowManager->winGetWindowFromId( window, buttonID );
//
//						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
//																								(WindowMsgData)button, buttonID );
//
//					}  // end if
//
//					// don't let key fall through anywhere else
//					return MSG_HANDLED;
//
//				}  // end escape
//
//			}  // end switch( key )
//
//		}  // end char
//
//	}  // end switch( msg )

	return MSG_IGNORED;

}

//-------------------------------------------------------------------------------------------------
/** MapSelect menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DifficultySelectSystem( GameWindow *window, UnsignedInt msg,
																				  WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			break;

		}

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			// this isn't fixed yet
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			if ( controlID == buttonOkID )
			{
				OptionPreferences pref;
				pref.setCampaignDifficulty(s_AIDiff);
				pref.write();
				//TheScriptEngine->setGlobalDifficulty(s_AIDiff); // CANNOT DO THIS! REPLAYS WILL BREAK!
				WindowLayout *layout = window->winGetLayout();
				if (layout)
				{
					layout->destroyWindows();
					deleteInstance(layout);
				}

				setupGameStart(TheCampaignManager->getCurrentMap(), s_AIDiff);
				// start the game
			}
			else if ( controlID == buttonCancelID )
			{
				TheCampaignManager->setCampaign( AsciiString::TheEmptyString );
				TheWindowManager->winUnsetModal(window);
				WindowLayout *layout = window->winGetLayout();
				if (layout)
				{
					layout->destroyWindows();
					deleteInstance(layout);
				}

			}
			else if ( controlID == radioButtonEasyAIID )
			{
				s_AIDiff = DIFFICULTY_EASY;
			}
			else if ( controlID == radioButtonMediumAIID )
			{
				s_AIDiff = DIFFICULTY_NORMAL;
			}
			else if ( controlID == radioButtonHardAIID )
			{
				s_AIDiff = DIFFICULTY_HARD;
			}

			break;

		}

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}
//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

