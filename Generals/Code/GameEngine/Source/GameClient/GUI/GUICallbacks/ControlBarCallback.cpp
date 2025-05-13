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

// FILE: ControlBarCallback.cpp ///////////////////////////////////////////////////////////////////
// Author: Colin Day - October 2001
// Desc: GUI Control bar at the bottom of the screen that houses the
//			 the build buttons, radar etc.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/NameKeyGenerator.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/MessageStream.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Radar.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GameClient.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ControlBar.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/LanguageFilter.h"
#include "GameClient/CommandXlat.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"

//external declarations of the Gadgets the callbacks can use
WindowLayout *popupCommunicatorLayout = NULL;


//-------------------------------------------------------------------------------------------------
/** Input procedure for the left HUD */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType LeftHUDInput( GameWindow *window, UnsignedInt msg,
																	 WindowMsgData mData1, WindowMsgData mData2 )
{
	
	// get player
	Player *player = ThePlayerList->getLocalPlayer();

	//
	// if the player doesn't have a radar, or the radar is hidden, and the radar is not being
	// forced to on, we just eat input over the radar window
	//
	if( !TheRadar->isRadarForced() && (TheRadar->isRadarHidden() || !player->hasRadar()) )
		return MSG_HANDLED;
	
	// If the middle mouse button is depressed, then just let the message fall all the 
	// way back to the usual middle mouse button processing.
	// jkmcd
	if( TheMouse->getMouseStatus()->middleState == MBS_Down )
		return MSG_IGNORED;

	switch( msg ) 
	{

		/** @todo
			This is wrong.  The radar should be in the message stream, and eat all messages and propagate them
			as a new message with the coords converted to world coords and the message flagged as being from 
			the radar.  This would let all of the normal processing occur, and allow individual commands to easily 
			reject being used on the radar.
		*/

		// ------------------------------------------------------------------------
		case GWM_NONE:
		case GWM_MOUSE_ENTERING:
		case GWM_MOUSE_LEAVING:
		{

			//
			// consider changing the mouse cursor if we are not in the process of firing
			// targeted "superweapons" which we can use the radar itself to fire
			//
			Bool targeting = FALSE;
			const CommandButton *command = TheInGameUI->getGUICommand();
			if( command 
					&& (command->getCommandType() == GUI_COMMAND_SPECIAL_POWER || command->getCommandType() == GUI_COMMAND_SPECIAL_POWER_FROM_COMMAND_CENTER)
					&& BitIsSet( command->getOptions(), NEED_TARGET_POS ) )
				targeting = TRUE;

			if( targeting == FALSE )
			{
				const DrawableList *drawableList = TheInGameUI->getAllSelectedLocalDrawables();
				Mouse::MouseCursor cur = Mouse::ARROW;

				if (!(drawableList->empty() || msg == GWM_MOUSE_LEAVING)) 
				{
					if (command && command->getCommandType() == GUI_COMMAND_ATTACK_MOVE)
					{
						cur = Mouse::ATTACKMOVETO;
					}
					else
					{
						cur = Mouse::MOVETO;
					}
				}

				// Groovy
				TheMouse->setCursor(cur);

			}  // end if

			return MSG_HANDLED;
		}

		// --------------------------------------------------------------------------------------------
		case GWM_MOUSE_POS:
		{

			// get mouse position
			ICoord2D mouse;
			mouse.x = mData1 & 0xFFFF;
			mouse.y = mData1 >> 16;

			// get window screen position
			ICoord2D screenPos;
			window->winGetScreenPosition( &screenPos.x, &screenPos.y );

			// set mouse position to be relative to this window
			mouse.x -= screenPos.x;
			mouse.y -= screenPos.y;

			// is the mouse in the radar window
			ICoord2D radar;
			if( (TheRadar->isRadarHidden() == FALSE || TheRadar->isRadarForced()) &&
					TheRadar->localPixelToRadar( &mouse, &radar ) )
			{

/*
//
// this is an example piece of code to find the object under the pixel position 
// of the radar ... should we in the future wish to allow commands to be executed
// on objects throught he radar.  note tho that this is extremely hard to do because
// the pixels on the radar are very small and it's hard to do accurate targeting
//

				Object *obj = TheRadar->objectUnderRadarPixel( &mouse );
				UnicodeString msg;
				if( obj )
					msg.format( L"Object under mouse on radar '%S'(%d)", 
											obj->getTemplate()->getName().str(), obj->getID() );
				else
					msg.format( L"Mouse (%d,%d) in Radar window L(%d,%d)", mouse.x, mouse.y, radar.x, radar.y );
				TheInGameUI->message( msg );
*/

				// keep the cursor for any context commands
				const CommandButton *command = TheInGameUI->getGUICommand();
				if( command 
						&& (command->getCommandType() == GUI_COMMAND_SPECIAL_POWER || command->getCommandType() == GUI_COMMAND_SPECIAL_POWER_FROM_COMMAND_CENTER)
						&& BitIsSet( command->getOptions(), NEED_TARGET_POS ) )
				{
					Int index = TheMouse->getCursorIndex( command->getCursorName() );

					if( index != Mouse::INVALID_MOUSE_CURSOR )
						TheMouse->setCursor( (Mouse::MouseCursor)index );
					else
						TheMouse->setCursor( Mouse::CROSS );

				}  // end if
				else
				{
					// Else we are not super targeting, so we have to try to refresh the move cursor.
					// We can't just do this on Enter and Exit, because hotkeys allow state to change
					// while we are in the radar.
					const DrawableList *drawableList = TheInGameUI->getAllSelectedLocalDrawables();
					Mouse::MouseCursor cur = Mouse::ARROW;

					if (!(drawableList->empty() || msg == GWM_MOUSE_LEAVING)) 
					{
						if (command && command->getCommandType() == GUI_COMMAND_ATTACK_MOVE)
						{
							cur = Mouse::ATTACKMOVETO;
						}
						else
						{
							cur = Mouse::MOVETO;
						}
					}

					// Groovy
					TheMouse->setCursor(cur);
				}

			}  // end if

			break;

		}  // end case mouse position

		// ------------------------------------------------------------------------
		case GWM_RIGHT_UP:// Here to eat
		case GWM_LEFT_UP:// Here to eat
			break;

		case GWM_RIGHT_DOWN:
		case GWM_LEFT_DOWN:
		{
			ICoord2D mouse;
			ICoord2D radar;
			ICoord2D size;
			ICoord2D screenPos;
			Coord3D world;

			// get window size
			window->winGetSize( &size.x, &size.y );

			// get mouse position
			mouse.x = mData1 & 0xFFFF;
			mouse.y = mData1 >> 16;
			
			// get window screen position
			window->winGetScreenPosition( &screenPos.x, &screenPos.y );

			// set mouse position to be relative to this window
			mouse.x -= screenPos.x;
			mouse.y -= screenPos.y;

			//
			// translate mouse position to radar position ... we know that the mouse
			// location given to us here is relative to the HUD window, which is
			// completely drawn with the radar ... so it's just a translation from
			// our window size we're drawing into to the radar cell size
			//
			if( (TheRadar->isRadarHidden() == FALSE || TheRadar->isRadarForced()) &&
					TheRadar->localPixelToRadar( &mouse, &radar ) &&
					TheRadar->radarToWorld( &radar, &world ) )
			{

				// No drawables, or a right click automatically means its a look at.
				// Having drawables and being in attack move mode means that we should attack move.
				// Having drawables and not being in attack move mode means that we should move.

				const DrawableList *drawableList = TheInGameUI->getAllSelectedLocalDrawables(); // locally-owned only
				

 				// see if the user wants to move the tactical view
 				if (	drawableList->empty() 
 					||	(! TheGlobalData->m_useAlternateMouse && msg == GWM_RIGHT_DOWN)
 					||	(TheGlobalData->m_useAlternateMouse && msg == GWM_LEFT_DOWN)	)
				{
					TheTacticalView->lookAt( &world );
					break;
				}

				// evaluate any special powers that can be executed from the radar
				const CommandButton *command = TheInGameUI->getGUICommand();
				if( command 
					&& (command->getCommandType() == GUI_COMMAND_SPECIAL_POWER || command->getCommandType() == GUI_COMMAND_SPECIAL_POWER_FROM_COMMAND_CENTER)
					&& BitIsSet( command->getOptions(), NEED_TARGET_POS ) 
					)
				{

					// do the command
					TheGameClient->evaluateContextCommand( NULL, &world, CommandTranslator::DO_COMMAND );

				}  // end if
				else if( command && command->getCommandType() == GUI_COMMAND_ATTACK_MOVE)
				{
					// Attack move has changed from a modifier to a command, so it moves up here.

					GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_DO_ATTACKMOVETO );
					msg->appendLocationArgument( world );

					// Play the unit voice response
					pickAndPlayUnitVoiceResponse(TheInGameUI->getAllSelectedDrawables(), GameMessage::MSG_DO_ATTACKMOVETO);
				}
				else
				{
					GameMessage *newMsg = NULL;

					// Do the superweapon stuff here, before issuing these other messages

					// GS Leaving commented out to show that isInAttackMoveToMode is NEVER SET.  It's a command now, not a modifier.
//					if (TheInGameUI->isInAttackMoveToMode()) 
//					{
//						newMsg = TheMessageStream->appendMessage(GameMessage::MSG_DO_ATTACKMOVETO);
//						newMsg->appendLocationArgument(world);
//						// Play the unit voice response
//						pickAndPlayUnitVoiceResponse(drawableList, GameMessage::MSG_DO_ATTACKMOVETO);
//						break;
//					}

					newMsg = TheMessageStream->appendMessage(GameMessage::MSG_DO_MOVETO);
					newMsg->appendLocationArgument(world);
					// Play the unit voice response
					pickAndPlayUnitVoiceResponse(drawableList, GameMessage::MSG_DO_MOVETO);
				
				}  // end else

			}
			

	break;

		}  // end left down

		// ------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	TheInGameUI->clearAttackMoveToMode();
	return MSG_HANDLED;

}  // end LeftHUDInput

//-------------------------------------------------------------------------------------------------
/** Input procedure for the control bar */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ControlBarInput( GameWindow *window, UnsignedInt msg,
																			WindowMsgData mData1, WindowMsgData mData2 )
{

	return MSG_IGNORED;

}  // end ControlBarInput
void ToggleQuitMenu(void);
//-------------------------------------------------------------------------------------------------
/** System callback for the control bar parent */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ControlBarSystem( GameWindow *window, UnsignedInt msg, 
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	static NameKeyType buttonCommunicator = NAMEKEY_INVALID;
	if(TheScriptEngine && TheScriptEngine->isGameEnding())
		return MSG_IGNORED;
	switch( msg ) 
	{
		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			// get ids for our children controls
			buttonCommunicator = TheNameKeyGenerator->nameToKey( AsciiString("ControlBar.wnd:PopupCommunicator") );

			break;

		}  // end create

		//---------------------------------------------------------------------------------------------
		case GBM_MOUSE_ENTERING:
		case GBM_MOUSE_LEAVING:
		{
			GameWindow *control = (GameWindow *)mData1;

			TheControlBar->processContextSensitiveButtonTransition( control, (GadgetGameMessage)msg);
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		case GBM_SELECTED_RIGHT:
		{
			GameWindow *control = (GameWindow *)mData1;
			static NameKeyType beaconPlacementButtonID = NAMEKEY("ControlBar.wnd:ButtonPlaceBeacon");
			static NameKeyType beaconDeleteButtonID = NAMEKEY("ControlBar.wnd:ButtonDeleteBeacon");
			static NameKeyType beaconClearTextButtonID = NAMEKEY("ControlBar.wnd:ButtonClearBeaconText");
			static NameKeyType beaconGeneralButtonID = NAMEKEY("ControlBar.wnd:ButtonGeneral");
//			static NameKeyType buttonSmallID = NAMEKEY("ControlBar.wnd:ButtonSmall");
//			static NameKeyType buttonMediumID = NAMEKEY("ControlBar.wnd:ButtonMedium");
			static NameKeyType buttonLargeID = NAMEKEY("ControlBar.wnd:ButtonLarge");
			static NameKeyType buttonOptions = NAMEKEY("ControlBar.wnd:ButtonOptions");
			static NameKeyType buttonIdleWorker = NAMEKEY("ControlBar.wnd:ButtonIdleWorker");

			Int controlID = control->winGetWindowId();
			if( controlID == buttonCommunicator )
			{
				ToggleDiplomacy(FALSE);
			}
			else if( controlID == beaconPlacementButtonID && TheGameLogic->isInMultiplayerGame() &&
				ThePlayerList->getLocalPlayer()->isPlayerActive())
			{
				const CommandButton *commandButton = TheControlBar->findCommandButton( "Command_PlaceBeacon" );
				TheInGameUI->setGUICommand( commandButton );
			}
			else if( controlID == beaconDeleteButtonID && TheGameLogic->isInMultiplayerGame() )
			{
				TheMessageStream->appendMessage( GameMessage::MSG_REMOVE_BEACON );
			}
			else if( controlID == beaconClearTextButtonID && TheGameLogic->isInMultiplayerGame() )
			{
				static NameKeyType textID = NAMEKEY("ControlBar.wnd:EditBeaconText");
				GameWindow *win = TheWindowManager->winGetWindowFromId(NULL, textID);
				if (win)
				{
					GadgetTextEntrySetText( win, UnicodeString::TheEmptyString );
				}
			}
			else if( controlID == beaconGeneralButtonID)
			{
				HideQuitMenu( );
				TheControlBar->togglePurchaseScience();
			}
			//else if( controlID == buttonSmallID)
			//			{
			//				TheControlBar->switchControlBarStage( CONTROL_BAR_STAGE_LOW	);
			//			}
			//			else if( controlID == buttonMediumID)
			//			{
			//				TheControlBar->switchControlBarStage( CONTROL_BAR_STAGE_SQUISHED	);
			//			}
			
			else if( controlID == buttonLargeID)
			{
				TheControlBar->toggleControlBarStage();
			}
			else if( controlID == buttonOptions)
			{
				ToggleQuitMenu();
			}
			else if( controlID == buttonIdleWorker)
			{
				HideQuitMenu( );
				TheInGameUI->selectNextIdleWorker();
			}
			else
			{
				//
				// all buttons from all the context sensitive user interface windows are part of the
				// control bar, send the button processing that way
				//
				TheControlBar->processContextSensitiveButtonClick( control, (GadgetGameMessage)msg );
			}
			break;

		}  // end button selected

		//---------------------------------------------------------------------------------------------
		case GEM_EDIT_DONE:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			static NameKeyType textID = NAMEKEY("ControlBar.wnd:EditBeaconText");
			if (controlID == textID)
			{
				// set beacon text
				if (TheInGameUI->getSelectCount() == 1)
				{
					GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_SET_BEACON_TEXT );
					UnicodeString newText = GadgetTextEntryGetText( control );
					TheLanguageFilter->filterLine(newText);
					const WideChar * c = newText.str();
					while ( c && *c )
					{
						msg->appendWideCharArgument( *c++ );
					}
					msg->appendWideCharArgument( L'\0' ); // trailing NULL
				}
			}
			break;
		} // end edit done

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end ControlBarSystem

extern void showReplayControls( void );
extern void hideReplayControls( void );
extern void toggleReplayControls( void );

//-------------------------------------------------------------------------------------------------
/** Force the control bar to be shown */
//-------------------------------------------------------------------------------------------------
void ShowControlBar( Bool immediate )
{
	showReplayControls();
	if(TheControlBar)
		TheControlBar->showSpecialPowerShortcut();
	if (TheWindowManager)
	{
		Int id = (Int)TheNameKeyGenerator->nameToKey(AsciiString("ControlBar.wnd:ControlBarParent"));
		GameWindow *window = TheWindowManager->winGetWindowFromId(NULL, id);

		if (window)
		{	
			TheControlBar->switchControlBarStage(CONTROL_BAR_STAGE_DEFAULT);
			TheTacticalView->setHeight((Int)(TheDisplay->getHeight() * 0.80f));
			if (TheControlBar->m_animateWindowManager && !immediate)
			{
				TheControlBar->m_animateWindowManager->reset();
				//TheControlBar->m_animateWindowManager->registerGameWindow(window, WIN_ANIMATION_SLIDE_BOTTOM_TIMED, TRUE, 1000, 0);
				TheControlBar->m_animateWindowManager->registerGameWindow(window, WIN_ANIMATION_SLIDE_BOTTOM, TRUE, 500, 0);
				TheControlBar->animateSpecialPowerShortcut(TRUE);
			}
			window->winHide(FALSE);
		}

	}  

	// We want to get everything recalced since this is a major state change.
	if(TheControlBar)
		TheControlBar->markUIDirty();

}// void ShowControlBar(void)

//-------------------------------------------------------------------------------------------------
/** Force the control bar to be hidden */
//-------------------------------------------------------------------------------------------------
void HideControlBar( Bool immediate )
{
	hideReplayControls();
	if(TheControlBar)
		TheControlBar->hideSpecialPowerShortcut();
	if (TheWindowManager)
	{
		Int id = (Int)TheNameKeyGenerator->nameToKey(AsciiString("ControlBar.wnd:ControlBarParent"));
		GameWindow *window = TheWindowManager->winGetWindowFromId(NULL, id);

		if (window)
		{
#ifdef SLIDE_LETTERBOX
				TheTacticalView->setHeight((Int)(TheDisplay->getHeight() * 0.80f)); 
#else
				TheTacticalView->setHeight(TheDisplay->getHeight());
#endif
		}
		if (immediate)
		{
			window->winHide(TRUE);
			if(TheControlBar)
				TheControlBar->hideSpecialPowerShortcut();
	
		}
		else
		{
			TheControlBar->m_animateWindowManager->reverseAnimateWindow();
			TheControlBar->animateSpecialPowerShortcut(FALSE);
		}

		//Always get rid of the purchase science screen!
		if( TheControlBar )
		{
			TheControlBar->hidePurchaseScience();
		}
	}  
}//void HideControlBar( void )

//-------------------------------------------------------------------------------------------------
/** Toggle the control bar on or off */
//-------------------------------------------------------------------------------------------------
void ToggleControlBar( Bool immediate )
{
	toggleReplayControls();

	if (TheWindowManager)
	{
		Int id = (Int)TheNameKeyGenerator->nameToKey(AsciiString("ControlBar.wnd:ControlBarParent"));
		GameWindow *window = TheWindowManager->winGetWindowFromId(NULL, id);

		if (window)
		{
			if (window->winIsHidden())
			{
				if(TheControlBar)	
					TheControlBar->showSpecialPowerShortcut();

				//now hidden, we're making it visible again so shrink viewport under the window
				TheTacticalView->setHeight((Int)(TheDisplay->getHeight() * 0.80f)); 
				window->winHide(!window->winIsHidden());
				TheControlBar->switchControlBarStage(CONTROL_BAR_STAGE_DEFAULT);
				if (TheControlBar->m_animateWindowManager && !immediate)
				{
					TheControlBar->m_animateWindowManager->reset();
					//TheControlBar->m_animateWindowManager->registerGameWindow(window, WIN_ANIMATION_SLIDE_BOTTOM_TIMED, FALSE, 500, 0);
					TheControlBar->m_animateWindowManager->registerGameWindow(window, WIN_ANIMATION_SLIDE_BOTTOM, TRUE, 500, 0);
					TheControlBar->animateSpecialPowerShortcut(TRUE);
				}
			}
			else
			{
				if(TheControlBar)
					TheControlBar->hideSpecialPowerShortcut();
				TheTacticalView->setHeight(TheDisplay->getHeight());
				window->winHide(!window->winIsHidden());
			}
			
		}

	}
}// end void ToggleControlBar( void )

//-------------------------------------------------------------------------------------------------
/** Resize the control bar */
//-------------------------------------------------------------------------------------------------
