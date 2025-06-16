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

// FILE: ScoreScreen.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Jun 2002
//
//	Filename: 	ScoreScreen.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	Gui callbacks for the Score Screen
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/BattleHonors.h"
#include "Common/GameEngine.h"
#include "Common/GameLOD.h"
#include "Common/GameState.h"
#include "Common/GameSpyMiscPreferences.h"
#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/RandomValue.h"
#include "Common/Recorder.h"
#include "Common/ScoreKeeper.h"
#include "Common/SkirmishBattleHonors.h"
#include "Common/ThingFactory.h"
#include "GameLogic/FPUControl.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/VictoryConditions.h"
#include "GameClient/CDCheck.h"
#include "GameClient/Display.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/GameWindowTransitions.h"
#include "GameClient/VideoPlayer.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/GameResultsThread.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/GameResultsThread.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
//Added By Saad
#include "GameClient/InGameUI.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
static NameKeyType parentID = NAMEKEY_INVALID;
static NameKeyType buttonOkID = NAMEKEY_INVALID;
///static NameKeyType buttonRehostID = NAMEKEY_INVALID;
static NameKeyType textEntryChatID = NAMEKEY_INVALID;
static NameKeyType buttonEmoteID = NAMEKEY_INVALID;
static NameKeyType chatBoxBorderID = NAMEKEY_INVALID;
static NameKeyType buttonContinueID = NAMEKEY_INVALID;
static NameKeyType buttonBuddiesID = NAMEKEY_INVALID;
static NameKeyType buttonSaveReplayID = NAMEKEY_INVALID;

static GameWindow *parent = NULL;
static GameWindow *buttonOk = NULL;
//static GameWindow *buttonRehost = NULL;
static GameWindow *buttonContinue = NULL;
static GameWindow *textEntryChat = NULL;
static GameWindow *buttonEmote = NULL;
static GameWindow *chatBoxBorder = NULL;
static GameWindow *buttonBuddies = NULL;
static GameWindow *staticTextGameSaved = NULL;

static Bool overidePlayerDisplayName = FALSE;

//Extrenal declarations
NameKeyType listboxChatWindowScoreScreenID = NAMEKEY_INVALID;
GameWindow *listboxChatWindowScoreScreen = NULL;
std::string LastReplayFileName;
static Bool canSaveReplay = FALSE;
extern void PopupReplayUpdate(WindowLayout *layout, void *userData);

void initSinglePlayer( void );
void finishSinglePlayerInit( void );
static Bool s_needToFinishSinglePlayerInit = FALSE;
static Bool buttonIsFinishCampaign = FALSE;
static WindowLayout *s_blankLayout = NULL;

void initSkirmish( void );
void initLANMultiPlayer(void);
void initInternetMultiPlayer(void);
void initReplayMultiPlayer(void);
void initReplaySinglePlayer(void);
void grabMultiPlayerInfo( void );
void grabSinglePlayerInfo( void );
void hideWindows( Int pos );
void ScoreScreenEnableControls(Bool enable);
void setObserverWindows( Player *player, Int i );
enum {
	SCORESCREEN_SINGLEPLAYER = 0,
	SCORESCREEN_SKIRMISH,
	SCORESCREEN_LAN,
	SCORESCREEN_INTERNET,
	SCORESCREEN_REPLAY
	};
static Int screenType;

struct ScoreGather
{
	Int m_totalMoneyEarned;						///< The total money that was harvested, refined, received in crates
	Int m_totalMoneySpent;						///< The total money spent on units, buildings, repairs
	Int m_totalUnitsDestroyed;				///< The total number of enimies that we've killed
	Int m_totalUnitsBuilt;						///< The total number of units we've created (created meaning that we built from a building)
	Int m_totalUnitsLost;							///< The total number of our units lost
	Int m_totalBuildingsDestroyed;		///< The total number of Buildings we've destroyed
	Int m_totalBuildingsBuilt;				///< The total number of buildings we've constructed
	Int m_totalBuildingsLost;					///< The total number of our buildings lost
	const Image *m_sideImage;
};
void populateSideInfo( UnicodeString side,ScoreGather *sg, Int pos, Color color);
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

void startNextCampaignGame(void)
{
	TheShell->popImmediate();
	TheShell->hideShell();
	TheWritableGlobalData->m_pendingFile = TheCampaignManager->getCurrentMap();
	// send a message to the logic for a new game
	GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
	msg->appendIntegerArgument(GAME_SINGLE_PLAYER);
	msg->appendIntegerArgument(TheCampaignManager->getGameDifficulty());
	msg->appendIntegerArgument(TheCampaignManager->getRankPoints());
	
	InitRandom(0);
}


void ScoreScreenEnableControls(Bool enable)
{
	// if we are using the button, do the enable thing.
	if ((buttonOk != NULL) && (buttonOk->winIsHidden() == FALSE)) {
		buttonOk->winEnable(enable);
	}

	if ((buttonContinue != NULL) && (buttonContinue->winIsHidden() == FALSE)) {
		buttonContinue->winEnable(enable);
	}

	if ((buttonBuddies != NULL) && (buttonBuddies->winIsHidden() == FALSE)) {
		buttonBuddies->winEnable(enable);
	}

	GameWindow *buttonSaveReplay = TheWindowManager->winGetWindowFromId( parent, buttonSaveReplayID );
	if ((buttonSaveReplay != NULL) && (buttonSaveReplay->winIsHidden() == FALSE)) {
		if (!canSaveReplay)
			enable = FALSE;
		buttonSaveReplay->winEnable(enable);
	}
}

extern Bool DontShowMainMenu; //KRIS
Bool ReplayWasPressed = FALSE;
/** Initialize the ScoreScreen */
//-------------------------------------------------------------------------------------------------
void ScoreScreenInit( WindowLayout *layout, void *userData )
{
	if (TheGameSpyInfo)
	{
		DEBUG_LOG(("ScoreScreenInit(): TheGameSpyInfo->stuff(%s/%s/%s)\n", TheGameSpyInfo->getLocalBaseName().str(), TheGameSpyInfo->getLocalEmail().str(), TheGameSpyInfo->getLocalPassword().str()));
	}

	DontShowMainMenu = TRUE; //KRIS
	buttonIsFinishCampaign = FALSE;

	//Store the keys so we have them later
	parentID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ParentScoreScreen" ) );
	buttonOkID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ButtonOk" ) );
	textEntryChatID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:TextEntryChat" ) );
	buttonEmoteID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ButtonEmote" ) );
	listboxChatWindowScoreScreenID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ListboxChatWindowScoreScreen" ) );
//	buttonRehostID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ButtonRehost" ) );
	chatBoxBorderID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ChatBoxBorder" ) );
	buttonBuddiesID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ButtonBuddy" ) );
	buttonContinueID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ButtonContinue" ) );
	buttonSaveReplayID = TheNameKeyGenerator->nameToKey( AsciiString( "ScoreScreen.wnd:ButtonSaveReplay" ) );

	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	buttonOk = TheWindowManager->winGetWindowFromId( parent, buttonOkID );
	textEntryChat = TheWindowManager->winGetWindowFromId( parent, textEntryChatID );
	buttonEmote = TheWindowManager->winGetWindowFromId( parent,buttonEmoteID  );
	listboxChatWindowScoreScreen = TheWindowManager->winGetWindowFromId( parent, listboxChatWindowScoreScreenID );
//	buttonRehost = TheWindowManager->winGetWindowFromId( parent, buttonRehostID );
	chatBoxBorder = TheWindowManager->winGetWindowFromId( parent, chatBoxBorderID );
	buttonContinue = TheWindowManager->winGetWindowFromId( parent, buttonContinueID );
	buttonBuddies = TheWindowManager->winGetWindowFromId( parent, buttonBuddiesID );
	staticTextGameSaved= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("ScoreScreen.wnd:StaticTextGameSaveComplete") );
	// get the replay filename for later (not full path)
	LastReplayFileName = TheRecorder->getLastReplayFileName().str();
	staticTextGameSaved->winHide(TRUE);
	overidePlayerDisplayName = FALSE;
	WindowLayout *replayLayout = TheShell->getPopupReplayLayout();
	if (replayLayout != NULL) {
		replayLayout->hide(TRUE);
	}
	canSaveReplay = FALSE;
	if(TheRecorder->getMode() == RECORDERMODETYPE_RECORD)
		canSaveReplay = TRUE;
	GameWindow *buttonSaveReplay = TheWindowManager->winGetWindowFromId( parent, buttonSaveReplayID );
	if (TheRecorder->getMode() == RECORDERMODETYPE_NONE && buttonSaveReplay)
		buttonSaveReplay->winEnable(FALSE);

	s_needToFinishSinglePlayerInit = FALSE;
	if (TheGameLogic->isInReplayGame())
	{
		if (buttonSaveReplay)
			buttonSaveReplay->winHide(TRUE);

		if (TheRecorder->isMultiplayer())
		{
			initReplayMultiPlayer();
			TheTransitionHandler->setGroup("ScoreScreenShow");
		}
		else
		{
			overidePlayerDisplayName = TRUE;
			initReplaySinglePlayer();
			TheTransitionHandler->setGroup("ScoreScreenShow");
		}
	}
	else
	{
		if(TheGameLogic->isInInternetGame())
		{
			initInternetMultiPlayer();
			TheTransitionHandler->setGroup("ScoreScreenShow");
		}
		else if( TheGameLogic->isInLanGame())
		{
			initLANMultiPlayer();
			TheTransitionHandler->setGroup("ScoreScreenShow");
		}
		else if( TheGameLogic->isInSkirmishGame())
		{
			initSkirmish();
			TheTransitionHandler->setGroup("ScoreScreenShow");
		}
		else
		{
			overidePlayerDisplayName = TRUE;
			initSinglePlayer();
		}
	}

	// Make Sure the layout is visible
	layout->hide( FALSE );

	// set keyboard focus to main parent
	TheWindowManager->winSetFocus( parent );
	ReplayWasPressed = FALSE;
	if (s_blankLayout)
	{
		s_blankLayout->hide(FALSE);
		s_blankLayout->bringForward();
	}
}

void FixupScoreScreenMovieWindow( void )
{
	if (s_blankLayout)
	{
		s_blankLayout->hide(FALSE);
		s_blankLayout->bringForward();
	}
}

/** Shutdown the ScoreScreen */
//-------------------------------------------------------------------------------------------------
void ScoreScreenShutdown( WindowLayout *layout, void *userData )
{
	DontShowMainMenu = FALSE; //KRIS

	// hide the layout
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );
}

/** Update the ScoreScreen */
//-------------------------------------------------------------------------------------------------
void ScoreScreenUpdate( WindowLayout * layout, void *userData)
{
	WindowLayout *popupReplayLayout = TheShell->getPopupReplayLayout();
	if (popupReplayLayout != NULL) {
		if (popupReplayLayout->isHidden() == FALSE) {
			PopupReplayUpdate(popupReplayLayout, NULL);
		}
	}

	if (s_needToFinishSinglePlayerInit)
	{
		finishSinglePlayerInit();
		s_needToFinishSinglePlayerInit = FALSE;
	}
}

/** Input function for the ScoreScreen */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ScoreScreenInput( GameWindow *window, UnsignedInt msg,
																		WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg ) 
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;

			switch( key )
			{

				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{
					
					//
					// send a simulated selected event to the parent window of the
					// back/exit button
					//
					if( BitIsSet( state, KEY_STATE_UP ) )
					{

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED, 
																								(WindowMsgData)buttonOk, buttonOkID );

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;

}  // end MainMenuInput

/** System Function for the ScoreScreen */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ScoreScreenSystem( GameWindow *window, UnsignedInt msg, 
																				  WindowMsgData mData1, WindowMsgData mData2 )
{
	UnicodeString txtInput;

	switch( msg ) 
	{
		// --------------------------------------------------------------------------------------------
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

			break;
		}  // end input

		// --------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			TheTransitionHandler->remove("ScoreScreenShow", TRUE);
			ReplayWasPressed = FALSE;

			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			if( controlID == buttonOkID )
			{
				TheShell->pop();
				TheCampaignManager->setCampaign(AsciiString::TheEmptyString);
			}
			else if ( controlID == buttonContinueID )	
			{
				if(!buttonIsFinishCampaign)
					ReplayWasPressed = TRUE;
				if( screenType == SCORESCREEN_SINGLEPLAYER)	
				{
					AsciiString mapName = TheCampaignManager->getCurrentMap();

					if( mapName.isEmpty() )
					{
						ReplayWasPressed = FALSE;
						TheShell->pop();
					}
					else
					{
						CheckForCDAtGameStart( startNextCampaignGame );
					}
				}
			}
			else if ( controlID == buttonBuddiesID )	
			{
				GameSpyToggleOverlay( GSOVERLAY_BUDDY );
			}
			else if ( controlID == buttonSaveReplayID )
			{
				ScoreScreenEnableControls(FALSE);
        WindowLayout *saveReplayLayout = TheShell->getPopupReplayLayout();
				DEBUG_ASSERTCRASH( saveReplayLayout, ("Unable to get save replay menu layout.\n") );
				saveReplayLayout->runInit();
				saveReplayLayout->hide( FALSE );
				saveReplayLayout->bringForward();
			}

			else if ( controlID == buttonEmoteID )
			{
				// read the user's input
				txtInput.set(GadgetTextEntryGetText( textEntryChat ));
				// Clear the text entry line
				GadgetTextEntrySetText(textEntryChat, UnicodeString::TheEmptyString);
				// Clean up the text (remove leading/trailing chars, etc)
				txtInput.trim();
				// Echo the user's input to the chat window
				if (!txtInput.isEmpty())
					if(TheLAN)
						TheLAN->RequestChat(txtInput, LANAPIInterface::LANCHAT_EMOTE);
					//add the gamespy chat request here
			} //if ( controlID == buttonEmote )
			for(Int i = 0; i < MAX_SLOTS; ++i)
			{
				AsciiString name;
				name.format("ScoreScreen.wnd:ButtonAdd%d", i);
				if( controlID == TheNameKeyGenerator->nameToKey(name))
				{
					Bool notBuddy = TRUE;
					Int playerID = (Int)GadgetButtonGetData(TheWindowManager->winGetWindowFromId(NULL,controlID));
											// request to add a buddy
					BuddyInfoMap *buddies = TheGameSpyInfo->getBuddyMap();
					BuddyInfoMap::iterator bIt;
					if( playerID > 0)
					{
						bIt = buddies->find(playerID);
						if (bIt != buddies->end())
						{
							notBuddy = FALSE;
						}
					}
					if(notBuddy)
					{
						BuddyRequest req;
						req.buddyRequestType = BuddyRequest::BUDDYREQUEST_ADDBUDDY;
						req.arg.addbuddy.id = playerID;
						UnicodeString buddyAddstr;
						buddyAddstr = TheGameText->fetch("GUI:BuddyAddReq");
						wcsncpy(req.arg.addbuddy.text, buddyAddstr.str(), MAX_BUDDY_CHAT_LEN);
						req.arg.addbuddy.text[MAX_BUDDY_CHAT_LEN-1] = 0;
						TheGameSpyBuddyMessageQueue->addRequest(req);
					}
					break;
				}
			}
		}

		case GEM_EDIT_DONE:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			// Take the user's input and echo it into the chat window as well as
			// send it to the other clients on the lan
			if ( controlID == textEntryChatID )
			{
				
				// read the user's input
				txtInput.set(GadgetTextEntryGetText( textEntryChat ));
				// Clear the text entry line
				GadgetTextEntrySetText(textEntryChat, UnicodeString::TheEmptyString);
				// Clean up the text (remove leading/trailing chars, etc)
				txtInput.trim();
				// Echo the user's input to the chat window
				if (!txtInput.isEmpty())
					if(TheLAN)
						TheLAN->RequestChat(txtInput, LANAPIInterface::LANCHAT_NORMAL);
					//add the gamespy chat request here

			}// if ( controlID == textEntryChatID )
		}
	}		
	return MSG_HANDLED;
}


//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

/** Special Init path for making this a single player Score Screen */
//-------------------------------------------------------------------------------------------------
void initSkirmish( void )
{
	screenType = SCORESCREEN_SKIRMISH;
	grabMultiPlayerInfo();
	if (textEntryChat)
		textEntryChat->winHide(TRUE);
	if (buttonEmote)
		buttonEmote->winHide(TRUE);
	if (chatBoxBorder)
		chatBoxBorder->winHide(TRUE);
	if (buttonBuddies)
		buttonBuddies->winHide(TRUE);
	if (buttonContinue)
		buttonContinue->winHide(TRUE);
	if (listboxChatWindowScoreScreen)
		listboxChatWindowScoreScreen->winHide(TRUE);
	if(staticTextGameSaved)
		staticTextGameSaved->winHide(TRUE);
//	if (buttonRehost)
//		buttonRehost->winHide(TRUE);
}

void PlayMovieAndBlock(AsciiString movieTitle)
{
	VideoStreamInterface *videoStream = TheVideoPlayer->open( movieTitle );
	if ( videoStream == NULL )
	{
		return;
	}

	// Create the new buffer
	VideoBuffer *videoBuffer = TheDisplay->createVideoBuffer();
	if (	videoBuffer == NULL || 
				!videoBuffer->allocate(	videoStream->width(), 
													videoStream->height())
		)
	{
		delete videoBuffer;
		videoBuffer = NULL;

		if ( videoStream )
			videoStream->close();
		videoStream = NULL;

		return;
	}

	GameWindow *movieWindow = s_blankLayout->getFirstWindow();
	TheWritableGlobalData->m_loadScreenRender = TRUE;
	while (videoStream->frameIndex() < videoStream->frameCount() - 1)
	{
		TheGameEngine->serviceWindowsOS();

		if(!videoStream->isFrameReady())
		{
			Sleep(1);
			continue;
		}

		if (!TheGameEngine->isActive())
		{	//we are alt-tabbed out, so just increment the frame
			videoStream->frameNext();
			videoStream->frameDecompress();
			continue;
		}
		
		videoStream->frameDecompress();
		videoStream->frameRender(videoBuffer);
		videoStream->frameNext();

		if(videoBuffer)
			movieWindow->winGetInstanceData()->setVideoBuffer(videoBuffer);

		//TheWindowManager->update();

		TheDisplay->draw();
	}
	TheWritableGlobalData->m_loadScreenRender = FALSE;
	movieWindow->winGetInstanceData()->setVideoBuffer(NULL);
	if (videoBuffer)
	{
		delete videoBuffer;
		videoBuffer = NULL;
	}
	if (videoStream)
	{
		videoStream->close();
		videoStream = NULL;
	}

	setFPMode();
}

void initSinglePlayer( void )
{
	screenType = SCORESCREEN_SINGLEPLAYER;
	TheCampaignManager->setRankPoints(ThePlayerList->getLocalPlayer()->getSkillPoints());
	TheCampaignManager->setGameDifficulty(TheScriptEngine->getGlobalDifficulty());
	grabSinglePlayerInfo();
	s_needToFinishSinglePlayerInit = TRUE;
	s_blankLayout = TheWindowManager->winCreateLayout("Menus/BlankWindow.wnd");
	DEBUG_ASSERTCRASH(s_blankLayout,("We Couldn't Load Menus/BlankWindow.wnd"));
	s_blankLayout->hide(FALSE);
	s_blankLayout->bringForward();
	s_blankLayout->getFirstWindow()->winClearStatus(WIN_STATUS_IMAGE);
}

void finishSinglePlayerInit( void )
{
	if(TheCampaignManager->isVictorious())
	{
		TheCampaignManager->gotoNextMission();

		if(TheCampaignManager->getCurrentMap().isEmpty())
		{
			GadgetButtonSetText(buttonContinue, TheGameText->fetch("GUI:EndCampaign"));
			buttonIsFinishCampaign = TRUE;
			// mark us as having completed the campaign
			Campaign* campaign = TheCampaignManager->getCurrentCampaign();
			if (campaign)
			{
				SkirmishBattleHonors stats;
				if (campaign->m_name.compareNoCase("USA") == 0)
					stats.setHonors(BATTLE_HONOR_CAMPAIGN_USA);
				if (campaign->m_name.compareNoCase("China") == 0)
					stats.setHonors(BATTLE_HONOR_CAMPAIGN_CHINA);
				if (campaign->m_name.compareNoCase("GLA") == 0)
					stats.setHonors(BATTLE_HONOR_CAMPAIGN_GLA);
				stats.write();

				if (buttonOk)
					buttonOk->winHide(TRUE);
				if (buttonContinue)
					buttonContinue->winHide(TRUE);
				if (textEntryChat)
					textEntryChat->winHide(TRUE);
				if (buttonEmote)
					buttonEmote->winHide(TRUE);
				if (listboxChatWindowScoreScreen)
					listboxChatWindowScoreScreen->winHide(TRUE);
				if (chatBoxBorder)
					chatBoxBorder->winHide(TRUE);
				if (buttonBuddies)
					buttonBuddies->winHide(TRUE);
				if (campaign->getFinalVictoryMovie().isNotEmpty())
				{
					AsciiString vidName;
					vidName = campaign->getFinalVictoryMovie();
					Bool useLowRes = FALSE;
					if (TheGameLODManager) {
						if (!TheGameLODManager->didMemPass()) {
							useLowRes = TRUE;
						}
						if (TheGameLODManager->findStaticLODLevel()==STATIC_GAME_LOD_LOW) {
							useLowRes = TRUE;
						}
						if (TheGameLODManager->getStaticLODLevel()==STATIC_GAME_LOD_LOW) {
							useLowRes = TRUE;
						}
					}
					if(!useLowRes)
						PlayMovieAndBlock(vidName);
				}
			}
		}
		else 
		{
			GadgetButtonSetText(buttonContinue, TheGameText->fetch("GUI:SaveAndContinue"));
			
			// auto save game
			TheGameState->missionSave();
			if(staticTextGameSaved)
				staticTextGameSaved->winHide(FALSE);
		}
	}
	else
	{
		GadgetButtonSetText(buttonContinue, TheGameText->fetch("GUI:Retry"));

	}

	//Added By Sadullah Nader
	//Fix for the black screen text that appears after loading sequence

	TheInGameUI->freeMessageResources();

	//
	if (s_blankLayout)
	{
		s_blankLayout->destroyWindows();
		deleteInstance(s_blankLayout);
		s_blankLayout = NULL;
	}

	// set keyboard focus to main parent
	TheWindowManager->winSetFocus( parent );

	if (buttonOk)
		buttonOk->winHide(FALSE);
	if (buttonContinue)
		buttonContinue->winHide(FALSE);
	if (textEntryChat)
		textEntryChat->winHide(TRUE);
	if (buttonEmote)
		buttonEmote->winHide(TRUE);
	if (listboxChatWindowScoreScreen)
		listboxChatWindowScoreScreen->winHide(TRUE);
	if (chatBoxBorder)
		chatBoxBorder->winHide(TRUE);
	if (buttonBuddies)
		buttonBuddies->winHide(TRUE);
//	if (buttonRehost)
//		buttonRehost->winHide(TRUE);

	// need to do this here
	TheTransitionHandler->setGroup("ScoreScreenShow");
}

/** Special Init path for making this a single player replay Score Screen */
//-------------------------------------------------------------------------------------------------
void initReplaySinglePlayer( void )
{
	screenType = SCORESCREEN_REPLAY;
	grabSinglePlayerInfo();
	if(staticTextGameSaved)
		staticTextGameSaved->winHide(TRUE);
	if (textEntryChat)
		textEntryChat->winHide(TRUE);
	if (buttonEmote)
		buttonEmote->winHide(TRUE);
	if (chatBoxBorder)
		chatBoxBorder->winHide(TRUE);
	if (buttonContinue)
		buttonContinue->winHide(TRUE);
	if (buttonBuddies)
		buttonBuddies->winHide(TRUE);
	if (listboxChatWindowScoreScreen)
		listboxChatWindowScoreScreen->winHide(TRUE);
//	if (buttonRehost)
//		buttonRehost->winHide(TRUE);
}

/** Special Init path for making this a Multiplayer Score Screen(LAN) */
//-------------------------------------------------------------------------------------------------
void initLANMultiPlayer(void)
{
	screenType = SCORESCREEN_LAN;
	grabMultiPlayerInfo();
	GadgetTextEntrySetText(textEntryChat, UnicodeString::TheEmptyString);
	TheWindowManager->winSetFocus( textEntryChat );
	if(staticTextGameSaved)
		staticTextGameSaved->winHide(TRUE);
	if (textEntryChat)
		textEntryChat->winHide(FALSE);
	if (buttonEmote)
		buttonEmote->winHide(FALSE);
	if (buttonContinue)
		buttonContinue->winHide(TRUE);
	if (listboxChatWindowScoreScreen)
		listboxChatWindowScoreScreen->winHide(FALSE);
	if (chatBoxBorder)
		chatBoxBorder->winHide(FALSE);
	if (buttonBuddies)
		buttonBuddies->winHide(TRUE);
}

/** Special Init path for making this a Multiplayer Score Screen(Internet) */
//-------------------------------------------------------------------------------------------------
void initInternetMultiPlayer(void)
{
	screenType = SCORESCREEN_INTERNET;
	grabMultiPlayerInfo();
	GadgetTextEntrySetText(textEntryChat, UnicodeString::TheEmptyString);
	TheWindowManager->winSetFocus( textEntryChat );
	if(staticTextGameSaved)
		staticTextGameSaved->winHide(TRUE);
	if (buttonContinue)
		buttonContinue->winHide(TRUE);
	if (textEntryChat)
		textEntryChat->winHide(TRUE);
	if (buttonEmote)
		buttonEmote->winHide(TRUE);
	if (listboxChatWindowScoreScreen)
		listboxChatWindowScoreScreen->winHide(FALSE);
	if (chatBoxBorder)
		chatBoxBorder->winHide(FALSE);

	if(TheGameSpyInfo && TheGameSpyInfo->getLocalProfileID() ==0)
		buttonBuddies->winHide(TRUE);
	else
		buttonBuddies->winHide(FALSE);

	if (!TheGameSpyBuddyMessageQueue)
		return;
	BuddyRequest req;
	req.buddyRequestType = BuddyRequest::BUDDYREQUEST_SETSTATUS;
	req.arg.status.status = GP_ONLINE;
	strcpy(req.arg.status.statusString, "Online");
	strcpy(req.arg.status.locationString, "");
	TheGameSpyBuddyMessageQueue->addRequest(req);
}

/** Special Init path for making this a Multiplayer Score Screen(Replay) */
//-------------------------------------------------------------------------------------------------
void initReplayMultiPlayer(void)
{
	screenType = SCORESCREEN_REPLAY;
	grabMultiPlayerInfo();
	if(staticTextGameSaved)
		staticTextGameSaved->winHide(TRUE);
	if (textEntryChat)
		textEntryChat->winHide(TRUE);
	if (buttonEmote)
		buttonEmote->winHide(TRUE);
	if (listboxChatWindowScoreScreen)
		listboxChatWindowScoreScreen->winHide(TRUE);
	if (chatBoxBorder)
		chatBoxBorder->winHide(TRUE);
	if (buttonContinue)
		buttonContinue->winHide(TRUE);
	if (buttonBuddies)
		buttonBuddies->winHide(TRUE);
//	if (buttonRehost)
//		buttonRehost->winHide(TRUE);
}

static Bool isSlotLocalAlly(GameInfo *game, const GameSlot *slot)
{
	const GameSlot *localSlot = game->getConstSlot(game->getLocalSlotNum());
	if (!localSlot)
		return TRUE;

	if (slot == localSlot)
		return TRUE;

	if (slot->getTeamNumber() < 0)
		return FALSE;

	return slot->getTeamNumber() == localSlot->getTeamNumber();
}

static void updateSkirmishBattleHonors(SkirmishBattleHonors& stats)
{
	DEBUG_LOG(("Updating Skirmish battle honors\n"));
	Player *localPlayer = ThePlayerList->getLocalPlayer();
	ScoreKeeper *s = localPlayer->getScoreKeeper();

	if (stats.getWinStreak() >= 3)
		stats.setHonors(BATTLE_HONOR_STREAK_3);
	if (stats.getWinStreak() >= 10)
		stats.setHonors(BATTLE_HONOR_STREAK_10);
	if (stats.getWinStreak() >= 25)
		stats.setHonors(BATTLE_HONOR_STREAK_25);

	const ThingTemplate *pTemplate = TheThingFactory->findTemplate("GLAScudStorm");
	if (s->getTotalObjectsBuilt(pTemplate) > 0)
		stats.setBuiltSCUD();

	pTemplate = TheThingFactory->findTemplate("AmericaParticleCannonUplink");
	if (s->getTotalObjectsBuilt(pTemplate) > 0)
		stats.setBuiltParticleCannon();

	pTemplate = TheThingFactory->findTemplate("ChinaNuclearMissileLauncher");
	if (s->getTotalObjectsBuilt(pTemplate) > 0)
		stats.setBuiltNuke();

	if (stats.builtNuke() && stats.builtParticleCannon() && stats.builtSCUD())
		stats.setHonors(BATTLE_HONOR_APOCALYPSE);

	KindOfMaskType validMask, invalidMask;
	validMask.set(KINDOF_VEHICLE);
	invalidMask.set(KINDOF_AIRCRAFT);
	if (/*TheGameInfo->isSkirmish() &&*/ s->getTotalUnitsBuilt(validMask, invalidMask) >= 50)
		stats.setHonors(BATTLE_HONOR_BATTLE_TANK);

	validMask.clear();
	validMask.set(KINDOF_AIRCRAFT);
	invalidMask.clear();
	if (/*TheGameInfo->isSkirmish() &&*/ s->getTotalUnitsBuilt(validMask, invalidMask) >= 20)
		stats.setHonors(BATTLE_HONOR_AIR_WING);

	if (/*TheGameInfo->isSkirmish() &&*/ (TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND / 60) < 5)
	{
		stats.setHonors(BATTLE_HONOR_BLITZ5);
	}

	if (/*TheGameInfo->isSkirmish() &&*/ (TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND / 60) < 10)
	{
		stats.setHonors(BATTLE_HONOR_BLITZ10);
	}

	if (stats.getNumGamesLoyal() >= 20 && localPlayer->getSide() == "America")
		stats.setHonors(BATTLE_HONOR_LOYALTY_USA);

	if (stats.getNumGamesLoyal() >= 20 && 	localPlayer->getSide() == "China")
		stats.setHonors(BATTLE_HONOR_LOYALTY_CHINA);

	if (stats.getNumGamesLoyal() >= 20 && 	localPlayer->getSide() == "GLA")
		stats.setHonors(BATTLE_HONOR_LOYALTY_GLA);

	// endurance medal(s)
	Int numEasy = 0;
	Int numMedium = 0;
	Int numBrutal = 0;
	Bool anyAlliedAI = FALSE;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = TheGameInfo->getConstSlot(i);
		if (slot->isAI() && !isSlotLocalAlly(TheGameInfo, slot))
		{
			if (TheGameInfo->getConstSlot(i)->getState() == SLOT_EASY_AI)
				++numEasy;
			if (TheGameInfo->getConstSlot(i)->getState() == SLOT_MED_AI)
				++numMedium;
			if (TheGameInfo->getConstSlot(i)->getState() == SLOT_BRUTAL_AI)
				++numBrutal;
		}
		else if (slot->isAI())
		{
			// can't get challenge medals with AI helpers
			anyAlliedAI = TRUE;
		}
	}
	if (/*!anyAlliedAI &&*/ (numEasy || numMedium || numBrutal))
	{
		Int oldEasy = stats.getEnduranceMedal(TheGameInfo->getMap(), SLOT_EASY_AI);
		Int oldMedium = stats.getEnduranceMedal(TheGameInfo->getMap(), SLOT_MED_AI);
		Int oldBrutal = stats.getEnduranceMedal(TheGameInfo->getMap(), SLOT_BRUTAL_AI);
		if (numEasy)
		{
			stats.setEnduranceMedal(TheGameInfo->getMap(), SLOT_EASY_AI, max(oldEasy, numEasy + numMedium + numBrutal));
		}
		if (numMedium)
		{
			stats.setEnduranceMedal(TheGameInfo->getMap(), SLOT_MED_AI, max(oldMedium, numMedium + numBrutal));
		}
		if (numBrutal)
		{
			stats.setEnduranceMedal(TheGameInfo->getMap(), SLOT_BRUTAL_AI, max(oldBrutal, numBrutal));
		}
	}
}

static void updateMPBattleHonors(Int& honors, PSPlayerStats& stats)
{
	DEBUG_LOG(("Updating MP battle honors\n"));
	Player *localPlayer = ThePlayerList->getLocalPlayer();
	ScoreKeeper *s = localPlayer->getScoreKeeper();

	//	BATTLE_HONOR_LADDER_CHAMP		= 0x0000001, (IGNORED HERE)

	//	BATTLE_HONOR_STREAK_3				= 0x0000002,
	if (stats.winsInARow >= 3)
		honors |= BATTLE_HONOR_STREAK_3;

	//	BATTLE_HONOR_STREAK_10			= 0x0000008,
	if (stats.winsInARow >= 10)
		honors |= BATTLE_HONOR_STREAK_10;

	//	BATTLE_HONOR_STREAK_25			= 0x0000010,
	if (stats.winsInARow >= 25)
		honors |= BATTLE_HONOR_STREAK_25;

	//	BATTLE_HONOR_LOYALTY_USA		= 0x0000020,
	if (stats.gamesInRowWithLastGeneral >= 20 && 	localPlayer->getSide() == "America")
		honors |= BATTLE_HONOR_LOYALTY_USA;

	//	BATTLE_HONOR_LOYALTY_CHINA	= 0x0000040,
	if (stats.gamesInRowWithLastGeneral >= 20 && 	localPlayer->getSide() == "China")
		honors |= BATTLE_HONOR_LOYALTY_CHINA;

	//	BATTLE_HONOR_LOYALTY_GLA		= 0x0000060,
	if (stats.gamesInRowWithLastGeneral >= 20 && 	localPlayer->getSide() == "GLA")
		honors |= BATTLE_HONOR_LOYALTY_GLA;

	//	BATTLE_HONOR_BATTLE_TANK		= 0x0000080,
	KindOfMaskType validMask, invalidMask;
	validMask.set(KINDOF_VEHICLE);
	invalidMask.set(KINDOF_AIRCRAFT);
	if (/*TheGameInfo->isSkirmish() &&*/ s->getTotalUnitsBuilt(validMask, invalidMask) >= 50)
		honors |= BATTLE_HONOR_BATTLE_TANK;

	//	BATTLE_HONOR_AIR_WING				= 0x0000100,
	validMask.clear();
	validMask.set(KINDOF_AIRCRAFT);
	invalidMask.clear();
	if (/*TheGameInfo->isSkirmish() &&*/ s->getTotalUnitsBuilt(validMask, invalidMask) >= 20)
		honors |= BATTLE_HONOR_AIR_WING;

	if (stats.builtNuke && stats.builtParticleCannon && stats.builtSCUD)
		honors |= BATTLE_HONOR_APOCALYPSE;

	//	BATTLE_HONOR_SPECIAL_FORCES	= 0x0000200, (Not Valid)

	//	BATTLE_HONOR_ENDURANCE			= 0x0000400,

	//	BATTLE_HONOR_CAMPAIGN_USA		= 0x0000800,

	//	BATTLE_HONOR_CAMPAIGN_CHINA	= 0x0001000,

	//	BATTLE_HONOR_CAMPAIGN_GLA  	= 0x0002000,

	//	BATTLE_HONOR_BLITZ5					= 0x0008000,
	if (/*TheGameInfo->isSkirmish() &&*/ (TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND / 60) < 5)
	{
		honors |= BATTLE_HONOR_BLITZ5;
	}

	//	BATTLE_HONOR_BLITZ10				= 0x0008000,
	if (/*TheGameInfo->isSkirmish() &&*/ (TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND / 60) < 10)
	{
		honors |= BATTLE_HONOR_BLITZ10;
	}

	//	BATTLE_HONOR_SOLO_USA_B			= 0x0010000,

	//	BATTLE_HONOR_SOLO_USA_S			= 0x0020000,

	//	BATTLE_HONOR_SOLO_USA_G			= 0x0040000,

	//	BATTLE_HONOR_SOLO_CHINA_B		= 0x0080000,

	//	BATTLE_HONOR_SOLO_CHINA_S		= 0x0100000,

	//	BATTLE_HONOR_SOLO_CHINA_G		= 0x0200000,

	//	BATTLE_HONOR_SOLO_GLA_B			= 0x0400000,

	//	BATTLE_HONOR_SOLO_GLA_S			= 0x0800000,

	//	BATTLE_HONOR_SOLO_GLA_G			= 0x1000000,

}

// challenge medals are beating 1-7 Brutal AIs
static void updateChallengeMedals(Int& medals)
{
	if (!TheGameInfo->isSkirmish())
		return;

	Int numAIs = 0;
	Int numBrutals = 0;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = TheGameInfo->getConstSlot(i);
		if (slot->isAI() && !isSlotLocalAlly(TheGameInfo, slot))
		{
			++numAIs;
			if (TheGameInfo->getConstSlot(i)->getState() == SLOT_BRUTAL_AI)
				++numBrutals;
		}
		else if (slot->isAI())
		{
			// can't get challenge medals with AI helpers
			return;
		}
	}

	if (numAIs)
	{
		switch(numBrutals)
		{
		case 1:
			medals |= BH_CHALLENGE_MASK_1;
			break;
		case 2:
			medals |= BH_CHALLENGE_MASK_2;
			break;
		case 3:
			medals |= BH_CHALLENGE_MASK_3;
			break;
		case 4:
			medals |= BH_CHALLENGE_MASK_4;
			break;
		case 5:
			medals |= BH_CHALLENGE_MASK_5;
			break;
		case 6:
			medals |= BH_CHALLENGE_MASK_6;
			break;
		case 7:
			medals |= BH_CHALLENGE_MASK_7;
			break;
		}
	}
}

/** Populate the various windows with the information about the game based on each player's score
		keeper. */
//-------------------------------------------------------------------------------------------------
void populatePlayerInfo( Player *player, Int pos)
{
	if(!player || pos > MAX_SLOTS)
		return;
	Color color = player->getPlayerColor();
	ScoreKeeper *scoreKpr = player->getScoreKeeper();
	if(!scoreKpr)
	{
		DEBUG_ASSERTCRASH(FALSE,("Player %s does not have a scoreKeeper", player->getPlayerDisplayName().str()));
		return;
	}
	AsciiString winName;
	UnicodeString winValue;
	GameWindow *win;
	// set the player name
	winName.format("ScoreScreen.wnd:StaticTextPlayer%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	if(overidePlayerDisplayName)
	{
		GadgetStaticTextSetText(win, TheGameText->fetch("GUI:Player"));
	}
	else
		GadgetStaticTextSetText(win, player->getPlayerDisplayName());
	win->winHide(FALSE);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());

	// set the player name
	winName.format("ScoreScreen.wnd:StaticTextObserver%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total units built
	winName.format("ScoreScreen.wnd:StaticTextUnitsBuilt%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalUnitsBuilt());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total units Lost
	winName.format("ScoreScreen.wnd:StaticTextUnitsLost%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalUnitsLost());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total units Destroyed
	winName.format("ScoreScreen.wnd:StaticTextUnitsDestroyed%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalUnitsDestroyed());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total BuildingsBuilt
	winName.format("ScoreScreen.wnd:StaticTextBuildingsBuilt%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalBuildingsBuilt());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total BuildingsLost
	winName.format("ScoreScreen.wnd:StaticTextBuildingsLost%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalBuildingsLost());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total BuildingsDestroyed
	winName.format("ScoreScreen.wnd:StaticTextBuildingsDestroyed%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalBuildingsDestroyed());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total BuildingsDestroyed
	winName.format("ScoreScreen.wnd:StaticTextBuildingsDestroyed%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalBuildingsDestroyed());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total Resources
	winName.format("ScoreScreen.wnd:StaticTextResources%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->getTotalMoneyEarned());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the Score
	/*
winName.format("ScoreScreen.wnd:StaticTextScore%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->calculateScore());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);
*/
	// set the Buttons
//	winName.format("ScoreScreen.wnd:ButtonAdd%d", pos);
	//	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	//	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	//	if(screenType ==	SCORESCREEN_INTERNET && TheGameSpyInfo && TheGameSpyInfo->getLocalProfileID() != 0)
	//	{
	//		// Get the stats for the player
	//		
	//		AsciiString aName;
	//		aName.translate(player->getPlayerDisplayName());
	//
	//		PlayerInfoMap::iterator blah = TheGameSpyInfo->getPlayerInfoMap()->find(aName);
	//		if(blah->second.m_profileID != 0)
	//		{
	//			
	//			BuddyInfoMap::iterator bIt = TheGameSpyInfo->getBuddyMap()->find(blah->second.m_profileID);
	//			if(bIt !=  TheGameSpyInfo->getBuddyMap()->end()  || (TheGameSpyInfo->getLocalProfileID() == blah->second.m_profileID))
	//			{
	//				// already our buddy
	//				win->winHide(TRUE);
	//			}
	//			else
	//			{
	//		
	//				GadgetButtonSetData(win, (void *) blah->second.m_profileID);
	//				win->winEnable(TRUE);
	//				win->winHide(FALSE);
	//			}
	//		}
	//		else
	//			win->winHide(TRUE);
	//	}
	//	else
	//	{
	//		win->winHide(TRUE);
	//	}
	

	// set a marker for who won and lost
	winName.format("ScoreScreen.wnd:GameWindowWinner%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(FALSE);
//	if(TheVictoryConditions->hasAchievedVictory(player))
//		win->winEnable(TRUE);
//	else if(TheVictoryConditions->hasBeenDefeated(player))
//		win->winEnable(FALSE);
//	else
//		win->winHide(TRUE);
//
	const PlayerTemplate *fact = player->getPlayerTemplate();
	if(fact != NULL)
	{
		win->winSetEnabledImage(0, fact->getSideIconImage());
	}
	

	if ( screenType == SCORESCREEN_SKIRMISH && player->isLocalPlayer() )
	{
		if (TheGameInfo->isSandbox() || !(TheVictoryConditions->isLocalAlliedDefeat() || TheVictoryConditions->isLocalAlliedVictory()))
		{
			// If we died, and are watching sparring AIs, we still get the loss.
			if (player->isPlayerActive())
			{
				DEBUG_LOG(("Skipping skirmish stats update: sandbox:%d defeat:%d victory:%d\n",
					TheGameInfo->isSandbox(), TheVictoryConditions->isLocalAlliedDefeat(), TheVictoryConditions->isLocalAlliedVictory()));
				return;
			}
		}

		SkirmishBattleHonors stats;

		if (TheVictoryConditions->isLocalAlliedVictory())
		{
			stats.setWins(stats.getWins()+1);
			stats.setWinStreak(stats.getWinStreak()+1);
			stats.setBestWinStreak(max(stats.getBestWinStreak(), stats.getWinStreak()));
			updateSkirmishBattleHonors(stats);
			Int challengeMedals = stats.getChallengeMedals();
			updateChallengeMedals(challengeMedals);
			stats.setChallengeMedals(challengeMedals);
		}
		else
		{
			stats.setLosses(stats.getLosses()+1);
			stats.setWinStreak(0);
		}

		AsciiString lastGeneral = stats.getLastGeneral();
		stats.setLastGeneral(player->getPlayerTemplate()->getSide());
		if (lastGeneral != stats.getLastGeneral())
		{
			stats.setNumGamesLoyal(0);
		}
		else
		{
			stats.setNumGamesLoyal(stats.getNumGamesLoyal()+1);
		}

		stats.write();
	}

	if ( screenType == SCORESCREEN_INTERNET )
	{
		Int localID = TheGameSpyInfo->getLocalProfileID();
		if (localID)
		{
			Int localSlotNum = TheGameSpyGame->getLocalSlotNum();
			if (player->isLocalPlayer())
			{
				GameSpyGameSlot *localSlot = TheGameSpyGame->getGameSpySlot(localSlotNum);
				if (localSlot)
				{
					if (TheVictoryConditions->amIObserver())
					{
						// nothing to track
						DEBUG_LOG(("populatePlayerInfo() - not tracking stats for observer\n"));
						return;
					}
					PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(localID);

					UnsignedInt latestHumanInGame = 0;
					UnsignedInt lastFrameOfGame = 0;
					Bool gameEndedInDisconnect = TRUE;
					Bool sawAnyDisconnects = FALSE;
					Bool anyNonAI = FALSE;
					Bool anyAI = FALSE;
					Int i=0;
					for (; i<MAX_SLOTS; ++i)
					{
						const GameSlot *slot = TheGameInfo->getConstSlot(i);
						if (slot->isOccupied() && i != localSlotNum && !slot->isAI())
						{
							anyNonAI = TRUE;
						}
						if (slot->isOccupied())
						{
							lastFrameOfGame = max(lastFrameOfGame, slot->lastFrameInGame());
						}
						if (slot->isHuman())
						{
							if (i != localSlotNum)
							{
								latestHumanInGame = max(latestHumanInGame, slot->lastFrameInGame());
							}
						}
						if (slot->isAI())
						{
							anyAI = TRUE;
						}
					}
					DEBUG_LOG(("Game ended on frame %d - TheGameLogic->getFrame()=%d\n", lastFrameOfGame-1, TheGameLogic->getFrame()-1));
					for (i=0; i<MAX_SLOTS; ++i)
					{
						const GameSlot *slot = TheGameInfo->getConstSlot(i);
						DEBUG_LOG(("latestHumanInGame=%d, slot->isOccupied()=%d, slot->disconnected()=%d, slot->isAI()=%d, slot->lastFrameInGame()=%d\n",
							latestHumanInGame, slot->isOccupied(), slot->disconnected(), slot->isAI(), slot->lastFrameInGame()));
						if (slot->isOccupied() && slot->disconnected())
						{
							DEBUG_LOG(("Marking game as a possible disconnect game\n"));
							sawAnyDisconnects = TRUE;
						}
						if (slot->isOccupied() && !slot->disconnected() && i != localSlotNum &&
							(slot->isAI() || (slot->lastFrameInGame() >= lastFrameOfGame/*TheGameLogic->getFrame()*/-1)))
						{
							DEBUG_LOG(("Marking game as not ending in disconnect\n"));
							gameEndedInDisconnect = FALSE;
						}
					}

					if (!sawAnyDisconnects)
					{
						DEBUG_LOG(("Didn't see any disconnects - making gameEndedInDisconnect == FALSE\n"));
						gameEndedInDisconnect = FALSE;
					}

					if (gameEndedInDisconnect)
					{
						if (latestHumanInGame == TheNetwork->getPingFrame())
						{
							// we pinged on the last frame someone was there - i.e. game ended in a disconnect.
							// check if we were to blame.
							if (TheNetwork->getPingsRecieved() < max(1, TheNetwork->getPingsSent()/2)) /// @todo: what's a good percent of pings to have gotten?
							{
								DEBUG_LOG(("We were to blame.  Leaving gameEndedInDisconnect = true\n"));
							}
							else
							{
								DEBUG_LOG(("We were not to blame.  Changing gameEndedInDisconnect = false\n"));
								gameEndedInDisconnect = FALSE;
							}
						}
						else
						{
							DEBUG_LOG(("gameEndedInDisconnect, and we didn't ping on last frame.  What's up with that?\n"));
						}
					}
					
					if (anyAI)
					{
						// no stats if any computer controlled forces were in play.
						return;
					}

 					//Remove the extra disconnection we add to all games when they start.
					DEBUG_LOG(("populatePlayerInfo() - removing extra disconnect\n"));
 					if (TheGameSpyInfo)
						TheGameSpyInfo->updateAdditionalGameSpyDisconnections(-1);

					Bool sawEndOfGame = FALSE;
					if (TheVictoryConditions->isLocalAlliedDefeat() || TheVictoryConditions->isLocalAlliedVictory())
					{
						sawEndOfGame = TRUE;
					}
					if (TheVictoryConditions->isLocalDefeat())
					{
						sawEndOfGame = TRUE;
					}
					if (TheNetwork->sawCRCMismatch() || gameEndedInDisconnect)
					{
						sawEndOfGame = TRUE;
					}
					if (!sawEndOfGame)
					{
						DEBUG_LOG(("Not sending results - we didn't finish a game\n"));
						return;
					}

					// send ladder results (even if we end the game in the first N seconds)
					if (TheGameSpyGame->getLadderPort() && TheGameSpyGame->getLadderIP().isNotEmpty())
					{
						GameResultsRequest gameResReq;
						gameResReq.hostname = TheGameSpyGame->getLadderIP().str();
						gameResReq.port = TheGameSpyGame->getLadderPort();
						gameResReq.results = TheGameSpyGame->generateLadderGameResultsPacket().str();
						DEBUG_ASSERTCRASH(TheGameResultsQueue, ("No Game Results queue!\n"));
						if (TheGameResultsQueue)
						{
							TheGameResultsQueue->addRequest(gameResReq);
						}
					}

					if (TheVictoryConditions->getEndFrame() < LOGICFRAMES_PER_SECOND * 25 && TheVictoryConditions->getEndFrame())
					{
						DEBUG_LOG(("populatePlayerInfo() - not tracking stats for short game\n"));
						return;
					}

					// generate and send a gameres packet
					AsciiString resultsPacket = TheGameSpyGame->generateGameSpyGameResultsPacket();
					DEBUG_LOG(("About to send results packet: %s\n", resultsPacket.str()));
					PSRequest grReq;
					grReq.requestType = PSRequest::PSREQUEST_SENDGAMERESTOGAMESPY;
					grReq.results = resultsPacket.str();
					TheGameSpyPSMessageQueue->addRequest(grReq);

					Int ptIdx;
					const PlayerTemplate *myTemplate = player->getPlayerTemplate();
					DEBUG_LOG(("myTemplate = %X(%s)\n", myTemplate, myTemplate->getName().str()));
					for (ptIdx = 0; ptIdx < ThePlayerTemplateStore->getPlayerTemplateCount(); ++ptIdx)
					{
						const PlayerTemplate *nthTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(ptIdx);
						DEBUG_LOG(("nthTemplate = %X(%s)\n", nthTemplate, nthTemplate->getName().str()));
						if (nthTemplate == myTemplate)
						{
							break;
						}
					}

					if (stats.id == 0)
					{
						// we haven't gotten stats for ourselves yet.  Bummer.
						// what we'll do is just update our disconnects in the registry if we disconnected.
						// other than that, there's not much we can do.  :P
						if (gameEndedInDisconnect || TheNetwork->sawCRCMismatch())
						{
							/* @todo: this the right way
							UnsignedInt discons = 0;
							UnsignedInt syncs = 0;
							GetUnsignedIntFromRegistry("", "dc", discons);
							GetUnsignedIntFromRegistry("", "se", sync);
							++discons;
							++syncs;
							SetUnsignedIntInRegistry("", "dc", discons);
							SetUnsignedIntInRegistry("", "se", syncs);
							*/
							DEBUG_LOG(("populatePlayerInfo() - need to save off info for disconnect games!\n"));

							PSRequest req;
							req.requestType = PSRequest::PSREQUEST_UPDATEPLAYERSTATS;
							req.email = TheGameSpyInfo->getLocalEmail().str();
							req.nick = TheGameSpyInfo->getLocalBaseName().str();
							req.password = "";
							req.player = stats;
							req.addDesync = TheNetwork->sawCRCMismatch();
							req.addDiscon = gameEndedInDisconnect;
							req.lastHouse = ptIdx;
							TheGameSpyPSMessageQueue->addRequest(req);
						}
						DEBUG_CRASH(("populatePlayerInfo() - not tracking stats - we haven't gotten the original stuff yet\n"));
						return;
					}

					if (TheNetwork->sawCRCMismatch())
					{
						++stats.desyncs[ptIdx];
					}
					else if (gameEndedInDisconnect)
					{
						++stats.discons[ptIdx];
					}
					else if (TheVictoryConditions->isLocalAlliedDefeat() || !TheVictoryConditions->getEndFrame())
					{
						++stats.losses[ptIdx];
					}
					else
					{
						++stats.wins[ptIdx];
					}

					ScoreKeeper *s = player->getScoreKeeper();
					stats.buildingsBuilt[ptIdx] += s->getTotalBuildingsBuilt();
					stats.buildingsKilled[ptIdx] += s->getTotalBuildingsDestroyed();
					stats.buildingsLost[ptIdx] += s->getTotalBuildingsLost();

					if (TheGameSpyGame->isQMGame())
					{
						stats.QMGames[ptIdx]++;
					}
					else
					{
						stats.customGames[ptIdx]++;
					}

					if (TheNetwork->sawCRCMismatch())
					{
						stats.lossesInARow = 0;
						stats.desyncsInARow++;
						stats.disconsInARow = 0;
						stats.winsInARow = 0;
						stats.maxDesyncsInARow = max(stats.desyncsInARow, stats.maxDesyncsInARow);
					}
					else if (gameEndedInDisconnect)
					{
						stats.lossesInARow = 0;
						stats.desyncsInARow = 0;
						stats.disconsInARow++;
						stats.winsInARow = 0;
						stats.maxDisconsInARow = max(stats.disconsInARow, stats.maxDisconsInARow);
					}
					else if (TheVictoryConditions->isLocalAlliedVictory())
					{
						stats.lossesInARow = 0;
						stats.desyncsInARow = 0;
						stats.disconsInARow = 0;
						stats.winsInARow++;
						stats.maxWinsInARow = max(stats.winsInARow, stats.maxWinsInARow);
					}
					else
					{
						stats.lossesInARow++;
						stats.desyncsInARow = 0;
						stats.disconsInARow = 0;
						stats.winsInARow = 0;
						stats.maxLossesInARow = max(stats.lossesInARow, stats.maxLossesInARow);
					}

					stats.earnings[ptIdx] += s->getTotalMoneyEarned();
					stats.duration[ptIdx] += TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND / 60; // in minutes
					stats.games[ptIdx]++;
 					//since we raise this number when game starts, we need to lower it back on completion
 					Int disCons;
 					disCons = stats.discons[ptIdx];
 					disCons -= 1;
 					if (disCons >= 0)
 						stats.discons[ptIdx] = disCons;

					stats.gamesAsRandom += (localSlot->getOriginalPlayerTemplate() == PLAYERTEMPLATE_RANDOM);

					if (stats.lastGeneral != ptIdx)
						stats.gamesInRowWithLastGeneral = 0;
					stats.gamesInRowWithLastGeneral++;
					stats.lastGeneral = ptIdx;

					Int gameSize = 0;
					for (i=0; i<MAX_SLOTS; ++i)
					{
						if (TheGameSpyGame->getConstSlot(i)->isOccupied() && TheGameSpyGame->getConstSlot(i)->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
							++gameSize;
					}
					switch (gameSize)
					{
					case 2:
						stats.gamesOf2p[ptIdx]++;
						break;
					case 3:
						stats.gamesOf3p[ptIdx]++;
						break;
					case 4:
						stats.gamesOf4p[ptIdx]++;
						break;
					case 5:
						stats.gamesOf5p[ptIdx]++;
						break;
					case 6:
						stats.gamesOf6p[ptIdx]++;
						break;
					case 7:
						stats.gamesOf7p[ptIdx]++;
						break;
					case 8:
						stats.gamesOf8p[ptIdx]++;
						break;
					default:
						return; // nothing to track.
					}

					stats.lastFPS = TheDisplay->getAverageFPS(); ///@todo: need something more than this, really. :(

					stats.surrenders[ptIdx] += TheGameInfo->haveWeSurrendered()  || !TheVictoryConditions->getEndFrame();

					AsciiString systemSpec;
					systemSpec.format("LOD%d", TheGameLODManager->findStaticLODLevel());
					stats.systemSpec = systemSpec.str();

					stats.techCaptured[ptIdx] += s->getTotalTechBuildingsCaptured();

					stats.unitsBuilt[ptIdx] += s->getTotalUnitsBuilt();
					stats.unitsKilled[ptIdx] += s->getTotalUnitsDestroyed();
					stats.unitsLost[ptIdx] += s->getTotalUnitsLost();

					const ThingTemplate *pTemplate = TheThingFactory->findTemplate("GLAScudStorm");
					if (s->getTotalObjectsBuilt(pTemplate) > 0)
						stats.builtSCUD = 1;

					pTemplate = TheThingFactory->findTemplate("AmericaParticleCannonUplink");
					if (s->getTotalObjectsBuilt(pTemplate) > 0)
						stats.builtParticleCannon = 1;

					pTemplate = TheThingFactory->findTemplate("ChinaNuclearMissileLauncher");
					if (s->getTotalObjectsBuilt(pTemplate) > 0)
						stats.builtNuke = 1;

					if (TheGameSpyGame->getLadderPort() && TheGameSpyGame->getLadderIP().isNotEmpty())
					{
						stats.lastLadderPort = TheGameSpyGame->getLadderPort();
						stats.lastLadderHost = TheGameSpyGame->getLadderIP().str();
					}

					if (!TheNetwork->sawCRCMismatch() && !gameEndedInDisconnect && !TheVictoryConditions->isLocalAlliedDefeat() && TheVictoryConditions->getEndFrame())
					{
						updateMPBattleHonors(stats.battleHonors, stats);
						updateChallengeMedals(stats.challengeMedals);
					}

					DEBUG_LOG(("populatePlayerInfo() - tracking stats for %s/%s/%s\n", TheGameSpyInfo->getLocalBaseName().str(), TheGameSpyInfo->getLocalEmail().str(), TheGameSpyInfo->getLocalPassword().str()));

					PSRequest req;
					req.requestType = PSRequest::PSREQUEST_UPDATEPLAYERSTATS;
					req.email = TheGameSpyInfo->getLocalEmail().str();
					req.nick = TheGameSpyInfo->getLocalBaseName().str();
					req.password = TheGameSpyInfo->getLocalPassword().str();
					req.player = stats;
					req.addDesync = TheNetwork->sawCRCMismatch();
					req.addDiscon = gameEndedInDisconnect;
					req.lastHouse = ptIdx;
					TheGameSpyPSMessageQueue->addRequest(req);
					TheGameSpyPSMessageQueue->trackPlayerStats(stats);
					
					// force an update of our shtuff
					PSResponse newResp;
					newResp.responseType = PSResponse::PSRESPONSE_PLAYERSTATS;
					newResp.player = stats;
					TheGameSpyPSMessageQueue->addResponse(newResp);

					// cache our stuff for easy reading next time
					GameSpyMiscPreferences mPref;
					mPref.setCachedStats(GameSpyPSMessageQueueInterface::formatPlayerKVPairs(stats).c_str());
					mPref.write();
				}
			}
		}
	}
}

/** We Grab information about the players differently in Multiplayer.  We only want the players
		listed in the slots */
//-------------------------------------------------------------------------------------------------
void grabMultiPlayerInfo( void )
{
	typedef std::map<Int, Player *> ScoreMap;
	typedef ScoreMap::iterator ScoreMapIt;
	typedef ScoreMap::reverse_iterator RevScoreMapIt;

	Int playerCount = 0;
	AsciiString playerName;
	Player *player;
	ScoreMap scores;
	ScoreMapIt it;
	scores.clear();
	Int adder = 1; // Varible used to add on an offset to the score to make sure we don't add people to the same map

	player = ThePlayerList->getLocalPlayer();
	if (player)
	{
		const Image *image = TheMappedImageCollection->findImageByName("MutiPlayer_ScoreScreen");
		if(image)
		{
			parent->winSetEnabledImage(0, image);
			parent->winSetStatus(parent->winGetStatus() | WIN_STATUS_IMAGE );
		}
	}

	// Add each player and score to the map. THis allows us to sort the players based on score.
	for( Int i = 0; i < MAX_SLOTS; ++i)
	{
		playerName.format("player%d",i);
		player = ThePlayerList->findPlayerWithNameKey(TheNameKeyGenerator->nameToKey(playerName));
		if(player)
		{
			Int score = player->getScoreKeeper()->calculateScore();
			it = scores.find( score );
			if (it != scores.end())
			score += adder++;			
			scores[score] = player;
			++playerCount;
		}
	}
	hideWindows(playerCount);
	Int count =0;
	RevScoreMapIt revIt;
	// display the players based on Score
	for ( revIt = scores.rbegin(); revIt != scores.rend(); ++revIt) 
	{
		Player *p = revIt->second;
		if(p->isPlayerObserver())
			setObserverWindows( p, count );
		else
			populatePlayerInfo( p, count);	
		count ++;
	}
	
	DEBUG_ASSERTCRASH(count == playerCount, ("For some reason we added %d players to the scores map, but only read %d", playerCount, count));
	
}

enum
{
	USA_FRIEND = 0,
	CHINA_FRIEND,
	GLA_FRIEND,
	USA_ENEMY,		// Keep friends with friends, enemys with enemys
	CHINA_ENEMY,
	GLA_ENEMY,
	MAX_RELATIONS // keep me last
};
/**	Grab the single player info */
//-------------------------------------------------------------------------------------------------
void grabSinglePlayerInfo( void )
{
	Int playerCount = 0;
	Player *player, *localPlayer;
	localPlayer = ThePlayerList->getLocalPlayer();
	
	if(localPlayer)
	{
		if(!localPlayer->isPlayerObserver())
		{
			populatePlayerInfo(localPlayer, playerCount);
			++playerCount;
		}
		else
		{
			for(Int k = 0; k < MAX_PLAYER_COUNT; ++k)
			{
				localPlayer = ThePlayerList->getNthPlayer(k);
				if(localPlayer->getPlayerType() == PLAYER_HUMAN)
				{
					populatePlayerInfo(localPlayer, playerCount);
					++playerCount;
					break;
				}
				localPlayer = NULL;
			}
		}
		PlayerTemplate const *fact = ThePlayerList->getLocalPlayer()->getPlayerTemplate();
		if(fact != NULL)
		{
			const Image *image = TheMappedImageCollection->findImageByName(ThePlayerList->getLocalPlayer()->getPlayerTemplate()->getScoreScreen());
			if(image)
			{
				parent->winSetEnabledImage(0, image);
				parent->winSetStatus(parent->winGetStatus() | WIN_STATUS_IMAGE );
			}
		}
	}
	if(!localPlayer)
		return;
	AsciiString side;
	// okay, there's all kinds of hard coding going on here.  THe reason why! well,
	// We have no way of telling what sides we have in the game.  Hence, the hardcoding.
	for(Int j = 0; j < MAX_RELATIONS; ++j )
	{
		Bool isFriend = TRUE;
		
		// set the string we'll be compairing to
		switch (j) {
		case USA_ENEMY:
			isFriend = FALSE;
		case USA_FRIEND:
			side.set("America");
			break;
		case CHINA_ENEMY:
			isFriend = FALSE;
		case CHINA_FRIEND:
			side.set("China");
			break;
		case GLA_ENEMY:
			isFriend = FALSE;	
		case GLA_FRIEND:
			side.set("GLA");
			break;
		}
		ScoreGather sg;
		sg.m_totalBuildingsBuilt = 0;
		sg.m_totalBuildingsDestroyed = 0;
		sg.m_totalBuildingsLost = 0;
		sg.m_totalMoneyEarned = 0;
		sg.m_totalMoneySpent = 0;
		sg.m_totalUnitsBuilt = 0;
		sg.m_totalUnitsDestroyed = 0;
		sg.m_totalUnitsLost = 0;
		sg.m_sideImage = NULL;
		Bool populate = FALSE;
		Color color;
		for(Int i = 0; i < MAX_PLAYER_COUNT; ++i)
		{
			player = ThePlayerList->getNthPlayer(i);
			if(player && player != localPlayer &&	
				 player->getSide().compare(side) == 0)
			{
				if ((TheGameLogic->isInSinglePlayerGame() == FALSE) || (player->getListInScoreScreen() == TRUE))
				{
					if((isFriend == TRUE && localPlayer->getRelationship(player->getDefaultTeam()) == ALLIES) ||
							(isFriend == FALSE && localPlayer->getRelationship(player->getDefaultTeam()) == ENEMIES))
					{
						ScoreKeeper *sk = player->getScoreKeeper();
						sg.m_totalBuildingsBuilt += sk->getTotalBuildingsBuilt();
						sg.m_totalBuildingsDestroyed += sk->getTotalBuildingsDestroyed();
						sg.m_totalBuildingsLost += sk->getTotalBuildingsLost();
						sg.m_totalMoneyEarned += sk->getTotalMoneyEarned();
						sg.m_totalMoneySpent += sk->getTotalMoneySpent();
						sg.m_totalUnitsBuilt += sk->getTotalUnitsBuilt();
						sg.m_totalUnitsDestroyed += sk->getTotalUnitsDestroyed();
						sg.m_totalUnitsLost += sk->getTotalUnitsLost();
						sg.m_sideImage = player->getPlayerTemplate()->getSideIconImage();
						color = player->getPlayerColor();
						populate = TRUE;	
					}
				}
			}
		}
		if(populate)
		{
			AsciiString label;
			label.set("GUI:");
			label.concat(side);
			if(isFriend)
				label.concat("Allies");
			else
				label.concat("Enemies");
			populateSideInfo(TheGameText->fetch(label), &sg, playerCount, color);
			++playerCount;
		}
	}
	hideWindows(playerCount);
	
}

/** Hide the windows we're not using */
//-------------------------------------------------------------------------------------------------
void hideWindows( Int pos )
{
	if(pos < 0 || pos >= MAX_SLOTS)
		return;
	AsciiString winName;
	GameWindow *win;
	for( Int i = pos; i < MAX_SLOTS; ++i)
	{

		// set the player name
		winName.format("ScoreScreen.wnd:StaticTextPlayer%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the player name
		winName.format("ScoreScreen.wnd:StaticTextObserver%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total units built
		winName.format("ScoreScreen.wnd:StaticTextUnitsBuilt%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total units Lost
		winName.format("ScoreScreen.wnd:StaticTextUnitsLost%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total units Destroyed
		winName.format("ScoreScreen.wnd:StaticTextUnitsDestroyed%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total BuildingsBuilt
		winName.format("ScoreScreen.wnd:StaticTextBuildingsBuilt%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total BuildingsLost
		winName.format("ScoreScreen.wnd:StaticTextBuildingsLost%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total BuildingsDestroyed
		winName.format("ScoreScreen.wnd:StaticTextBuildingsDestroyed%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total BuildingsDestroyed
		winName.format("ScoreScreen.wnd:StaticTextBuildingsDestroyed%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);

		// set the total Resources
		winName.format("ScoreScreen.wnd:StaticTextResources%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);
		
		// set the total score
		/*
winName.format("ScoreScreen.wnd:StaticTextScore%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);
*/

		// Set the Game Winner marker
		winName.format("ScoreScreen.wnd:GameWindowWinner%d", i);
		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
		win->winHide(TRUE);
		

//		// Set the Game Add Buttons
//		winName.format("ScoreScreen.wnd:ButtonAdd%d", i);
//		win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
//		DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
//		win->winHide(TRUE);
	}
}

/** Hide the windows we're not using */
//-------------------------------------------------------------------------------------------------
void setObserverWindows( Player *player, Int i )
{
	if(i < 0 || i >= MAX_SLOTS)
		return;
	AsciiString winName;
	GameWindow *win;
	
	Color color = 0xffffffff;
	
	// set the player name
	winName.format("ScoreScreen.wnd:StaticTextPlayer%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	if (player)
	{
		GadgetStaticTextSetText(win, player->getPlayerDisplayName());
		win->winHide(FALSE);
		win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	}
	else
	{
		win->winHide(TRUE);
	}

	// set the player name
	winName.format("ScoreScreen.wnd:StaticTextObserver%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(FALSE);


	// set the total units built
	winName.format("ScoreScreen.wnd:StaticTextUnitsBuilt%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total units Lost
	winName.format("ScoreScreen.wnd:StaticTextUnitsLost%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total units Destroyed
	winName.format("ScoreScreen.wnd:StaticTextUnitsDestroyed%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total BuildingsBuilt
	winName.format("ScoreScreen.wnd:StaticTextBuildingsBuilt%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total BuildingsLost
	winName.format("ScoreScreen.wnd:StaticTextBuildingsLost%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total BuildingsDestroyed
	winName.format("ScoreScreen.wnd:StaticTextBuildingsDestroyed%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total BuildingsDestroyed
	winName.format("ScoreScreen.wnd:StaticTextBuildingsDestroyed%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total Resources
	winName.format("ScoreScreen.wnd:StaticTextResources%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);
	
	// set the total score
	/*
winName.format("ScoreScreen.wnd:StaticTextScore%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);
*/

	// Set the Game Winner marker
	winName.format("ScoreScreen.wnd:GameWindowWinner%d", i);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(FALSE);
	const PlayerTemplate *fact = player->getPlayerTemplate();
	if(fact != NULL)
	{
		win->winSetEnabledImage(0, fact->getSideIconImage());
	}
	
//	// set the Buttons
//	winName.format("ScoreScreen.wnd:ButtonAdd%d", i);
//	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
//	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
////	if(screenType ==	SCORESCREEN_INTERNET)
////		win->winHide(FALSE);
////	else
////		win->winHide(TRUE);
//	if(screenType ==	SCORESCREEN_INTERNET && TheGameSpyInfo && TheGameSpyInfo->getLocalProfileID() != 0)
//	{
//		// Get the stats for the player
//		
//		AsciiString aName;
//		aName.translate(player->getPlayerDisplayName());
//
//		PlayerInfoMap::iterator blah = TheGameSpyInfo->getPlayerInfoMap()->find(aName);
//		if(blah->second.m_profileID != 0)
//		{
//			
//			BuddyInfoMap::iterator bIt = TheGameSpyInfo->getBuddyMap()->find(blah->second.m_profileID);
//			if(bIt !=  TheGameSpyInfo->getBuddyMap()->end()  || (TheGameSpyInfo->getLocalProfileID() == blah->second.m_profileID))
//			{
//				// already our buddy
//				win->winHide(TRUE);
//			}
//			else
//			{
//		
//				GadgetButtonSetData(win, (void *) blah->second.m_profileID);
//				win->winEnable(TRUE);
//				win->winHide(FALSE);
//			}
//		}
//		else
//			win->winHide(TRUE);
//	}
//	else
//	{
//		win->winHide(TRUE);
//	}


}

/** Populate the various windows with the information about the game based on each player's score
		keeper. */
//-------------------------------------------------------------------------------------------------
void populateSideInfo( UnicodeString side,ScoreGather *sg, Int pos, Color color)
{
	if(pos < 0 || pos > MAX_SLOTS)
		return;
	
	AsciiString winName;
	UnicodeString winValue;
	GameWindow *win;
	// set the player name
	winName.format("ScoreScreen.wnd:StaticTextPlayer%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	GadgetStaticTextSetText(win, side);
	win->winHide(FALSE);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());

	// set the player name
	winName.format("ScoreScreen.wnd:StaticTextObserver%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	win->winHide(TRUE);

	// set the total units built
	winName.format("ScoreScreen.wnd:StaticTextUnitsBuilt%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", sg->m_totalUnitsBuilt);
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total units Lost
	winName.format("ScoreScreen.wnd:StaticTextUnitsLost%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", sg->m_totalUnitsLost);
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total units Destroyed
	winName.format("ScoreScreen.wnd:StaticTextUnitsDestroyed%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", sg->m_totalUnitsDestroyed);
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total BuildingsBuilt
	winName.format("ScoreScreen.wnd:StaticTextBuildingsBuilt%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", sg->m_totalBuildingsBuilt);
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total BuildingsLost
	winName.format("ScoreScreen.wnd:StaticTextBuildingsLost%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", sg->m_totalBuildingsLost);
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total BuildingsDestroyed
	winName.format("ScoreScreen.wnd:StaticTextBuildingsDestroyed%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", sg->m_totalBuildingsDestroyed);
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the total Resources
	winName.format("ScoreScreen.wnd:StaticTextResources%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", sg->m_totalMoneyEarned);
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);

	// set the Score
	/*
winName.format("ScoreScreen.wnd:StaticTextScore%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	winValue.format(L"%d", scoreKpr->calculateScore());
	GadgetStaticTextSetText(win, winValue);
	win->winSetEnabledTextColors(color, win->winGetEnabledTextBorderColor());
	win->winHide(FALSE);
*/

	// set a marker for who won and lost
	winName.format("ScoreScreen.wnd:GameWindowWinner%d", pos);
	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
	if(sg->m_sideImage)
	{
		win->winHide(FALSE);	
		win->winSetEnabledImage(0, sg->m_sideImage);
	}

//	// set the Buttons
//	winName.format("ScoreScreen.wnd:ButtonAdd%d", pos);
//	win =  TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey( winName ) );
//	DEBUG_ASSERTCRASH(win,("Could not find window %s on the score screen", winName.str()));
//	if(screenType ==	SCORESCREEN_INTERNET)
//		win->winHide(FALSE);
//	else
//		win->winHide(TRUE);
}
