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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: ChallengeMenu.cpp
// Author: Steve Copeland, May 2003
// Description: General's Challenge Mode Menu
///////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
//#include "Common/GameLOD.h"
#include "Common/GameState.h"
#include "Common/PlayerTemplate.h"
#include "Common/RandomValue.h"
#include "Common/Recorder.h"
#include "Common/version.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/ChallengeGenerals.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GameWindowTransitions.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/MessageBox.h"
#include "GameClient/Shell.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/WindowVideoManager.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

SkirmishGameInfo *TheChallengeGameInfo = NULL;

// defines
static const Int DEFAULT_GENERAL = 0;
static const Int TELETYPE_SKIP = 2;

// window ids ------------------------------------------------------------------------------
static NameKeyType parentID = NAMEKEY_INVALID;
static NameKeyType buttonPlayID = NAMEKEY_INVALID;
static NameKeyType buttonBackID = NAMEKEY_INVALID;
static NameKeyType bioPortraitID = NAMEKEY_INVALID;
static NameKeyType bioNameEntryID = NAMEKEY_INVALID;
static NameKeyType bioDOBEntryID = NAMEKEY_INVALID;
static NameKeyType bioBirthplaceEntryID = NAMEKEY_INVALID;
static NameKeyType bioStrategyEntryID = NAMEKEY_INVALID;
static NameKeyType buttonGeneralPositionID[NUM_GENERALS] = {NAMEKEY_INVALID};
static NameKeyType backdropID = NAMEKEY_INVALID;
static NameKeyType bioParentID = NAMEKEY_INVALID;

// window pointers --------------------------------------------------------------------------------
static GameWindow *parentMenu = NULL;
static GameWindow *buttonPlay = NULL;
static GameWindow *buttonBack = NULL;
static GameWindow *bioPortrait = NULL;
static GameWindow *bioLine1Entry = NULL;
static GameWindow *bioLine2Entry = NULL;
static GameWindow *bioLine3Entry = NULL;
static GameWindow *bioLine4Entry = NULL;
static GameWindow *buttonGeneralPosition[NUM_GENERALS] = {NULL};
static GameWindow *backdrop = NULL;
static GameWindow *bioParent = NULL;

//static NameKeyType testWinID = NAMEKEY_INVALID;
//static GameWindow *testWin = NULL;
static WindowVideoManager *wndVideoManager = NULL;

//
static Int	initialGadgetDelay = 2;
static Bool justEntered = FALSE;
static Bool isShuttingDown = FALSE;

static Int lastButtonIndex = -1;
Int lastHilitedIndex = -1;
Bool isAutoSelecting = FALSE;

// for use by the teletype style bio text display
UnicodeString bioLine1;
UnicodeString bioLine2;
UnicodeString bioLine3;
UnicodeString bioLine4;
UnicodeString bioLine1Readout;
UnicodeString bioLine2Readout;
UnicodeString bioLine3Readout;
UnicodeString bioLine4Readout;
Int bioTextPosition = 0;
Int bioTotalLength = 0;

// for use by the intro animation
static Int buttonSequenceStep = 0;

// audio
AudioHandle lastSelectionSound = NULL;
AudioHandle lastPreviewSound = NULL;
static Int introAudioMagicNumber = 0;
static Bool hasPlayedIntroAudio = FALSE;


//-------------------------------------------------------------------------------------------------
// returns the index of the General Position button selected, or -1 if not found
//-------------------------------------------------------------------------------------------------
Int findPositionButton( Int controlID )
{
	for (Int i = 0; i < NUM_GENERALS; i++)
	{
		if (controlID == buttonGeneralPositionID[i]) 
			return i;
	}
	return -1;
}


//-------------------------------------------------------------------------------------------------
// enable the appropriate buttons, make sure they aren't hidden, and set the correct images
//-------------------------------------------------------------------------------------------------
void setEnabledButtons()
{
	for (Int i = 0; i < NUM_GENERALS; i++)
	{
		const GeneralPersona* generals = TheChallengeGenerals->getChallengeGenerals();
		buttonGeneralPosition[i]->winEnable(generals[i].isStartingEnabled());
		buttonGeneralPosition[i]->winHide(! generals[i].isStartingEnabled());

		Int templateNum = ThePlayerTemplateStore->getTemplateNumByName(generals[i].getPlayerTemplateName());
		const PlayerTemplate *playerTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(templateNum);
		if (playerTemplate)
		{
			const Image *enabledImage = TheMappedImageCollection->findImageByName( playerTemplate->getMedallionNormal() );
			GadgetCheckBoxSetEnabledImage( buttonGeneralPosition[i], enabledImage );
			if (enabledImage)
				// image size keeps changing, so it'll drive the window size directly
				buttonGeneralPosition[i]->winSetSize( enabledImage->getImageWidth(), enabledImage->getImageWidth() );

			const Image *selectedImage = TheMappedImageCollection->findImageByName( playerTemplate->getMedallionSelected() );
			GadgetCheckBoxSetHiliteUncheckedBoxImage( buttonGeneralPosition[i], selectedImage);
			GadgetCheckBoxSetDisabledUncheckedBoxImage( buttonGeneralPosition[i], selectedImage);

			const Image *hilitedImage = TheMappedImageCollection->findImageByName( playerTemplate->getMedallionHilite() );
			GadgetCheckBoxSetHiliteImage( buttonGeneralPosition[i], hilitedImage);
		}
	}
}


//-------------------------------------------------------------------------------------------------
// sets the appropriate campaign for the chosen general
//-------------------------------------------------------------------------------------------------
void setGeneralCampaign( Int buttonIndex )
{
	if (buttonIndex < 0 || buttonIndex >= NUM_GENERALS)
		return;

	// determine which general and player template is selected and store it
	const GeneralPersona* generals = TheChallengeGenerals->getChallengeGenerals();
	TheCampaignManager->setCampaign(generals[buttonIndex].getCampaign());
	Int templateNum = ThePlayerTemplateStore->getTemplateNumByName(generals[buttonIndex].getPlayerTemplateName());
	TheChallengeGenerals->setCurrentPlayerTemplateNum(templateNum);

	// set up the skirmish games single player slot
	GameSlot slot;
	const PlayerTemplate *playerTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(templateNum);
	slot.setState(SLOT_PLAYER, playerTemplate->getDisplayName());
	slot.setPlayerTemplate(templateNum);
	TheChallengeGameInfo->setSlot(0, slot);
}


//-------------------------------------------------------------------------------------------------
// set the appropriate bio for the given general and initialize the bio windows
//-------------------------------------------------------------------------------------------------
void setGeneralBio( Int buttonIndex )
{
	if (buttonIndex < 0 || buttonIndex >= NUM_GENERALS)
		return;

	// this is hidden until the a bio is set
	// @todo: use a fancy transition
	bioParent->winHide(FALSE);

	const GeneralPersona* generals = TheChallengeGenerals->getChallengeGenerals();
	const Image *image = generals[buttonIndex].getBioPortraitSmall();
	bioPortrait->winSetEnabledImage( 0, image );
	bioPortrait->winSetStatus( WIN_STATUS_IMAGE );

	bioTextPosition = 0;
	bioLine1 = TheGameText->fetch(generals[buttonIndex].getBioName());
	bioLine2 = TheGameText->fetch(generals[buttonIndex].getBioRank());
	bioLine3 = TheGameText->fetch(generals[buttonIndex].getBioBranch());
	bioLine4 = TheGameText->fetch(generals[buttonIndex].getBioStrategy());
	bioTotalLength = bioLine1.getLength() + bioLine2.getLength() + bioLine3.getLength() + bioLine4.getLength();

	// clear the bio readout text, because updateBio likes it that way
	GadgetStaticTextSetText(bioLine1Entry, UnicodeString::TheEmptyString);
	GadgetStaticTextSetText(bioLine2Entry, UnicodeString::TheEmptyString);
	GadgetStaticTextSetText(bioLine3Entry, UnicodeString::TheEmptyString);
	GadgetStaticTextSetText(bioLine4Entry, UnicodeString::TheEmptyString);
}

//-------------------------------------------------------------------------------------------------
// update the intro button sequence UNFINISHED
//-------------------------------------------------------------------------------------------------
void updateButtonSequence(Int stepsPerUpdate)
{
	const static Int cleanupStates = 2;
	if (buttonSequenceStep > NUM_GENERALS + cleanupStates)
		return;

	const GeneralPersona* generals = TheChallengeGenerals->getChallengeGenerals();

	for (Int i = 0; i < stepsPerUpdate; i++)
	{
		// selected look
		Int pos = buttonSequenceStep;
		if (pos < NUM_GENERALS && !buttonGeneralPosition[pos]->winIsHidden())
		{
			Int templateNum = ThePlayerTemplateStore->getTemplateNumByName(generals[pos].getPlayerTemplateName());
			const PlayerTemplate *playerTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(templateNum);
			if (playerTemplate)
				GadgetCheckBoxSetEnabledImage( buttonGeneralPosition[pos], TheMappedImageCollection->findImageByName( playerTemplate->getMedallionSelected() ) );
		}

		// mouseover look
		if (--pos > 0 && pos < NUM_GENERALS && !buttonGeneralPosition[pos]->winIsHidden())
		{
			Int templateNum = ThePlayerTemplateStore->getTemplateNumByName(generals[pos].getPlayerTemplateName());
			const PlayerTemplate *playerTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(templateNum);
			if (playerTemplate)
				GadgetCheckBoxSetEnabledImage( buttonGeneralPosition[pos], TheMappedImageCollection->findImageByName( playerTemplate->getMedallionHilite() ) );
		}
		
		// regular look
		if (--pos > 0 && pos < NUM_GENERALS && !buttonGeneralPosition[pos]->winIsHidden())
		{
			Int templateNum = ThePlayerTemplateStore->getTemplateNumByName(generals[pos].getPlayerTemplateName());
			const PlayerTemplate *playerTemplate = ThePlayerTemplateStore->getNthPlayerTemplate(templateNum);
			if (playerTemplate)
				GadgetCheckBoxSetEnabledImage( buttonGeneralPosition[pos], TheMappedImageCollection->findImageByName( playerTemplate->getMedallionNormal() ) );
		}

		buttonSequenceStep++;
	}
}

//-------------------------------------------------------------------------------------------------
// update the general's bio, teletype style, call this as often as you want frames advanced
// NOTE: static text windows must be initialized to empty before calling
// accepts the number of frames to advance
// returns TRUE if an update occurred (the update was necessary)
//-------------------------------------------------------------------------------------------------
Bool updateBio(Int frames)
{
	Bool ret = FALSE;

	for (Int i = 0; i < frames; i++)
	{
		if (bioTextPosition < bioTotalLength)
		{
			UnicodeString text;
			WideChar wChar;
			GameWindow *window;
			if (bioTextPosition < bioLine1.getLength())
			{
				text = GadgetStaticTextGetText(bioLine1Entry);
				wChar = bioLine1.getCharAt(bioTextPosition);
				window = bioLine1Entry;
			}
			else if (bioTextPosition < bioLine1.getLength() + bioLine2.getLength())
			{
				text = GadgetStaticTextGetText(bioLine2Entry);
				wChar = bioLine2.getCharAt(bioTextPosition - bioLine1.getLength());
				window = bioLine2Entry;
			}
			else if (bioTextPosition < bioLine1.getLength() + bioLine2.getLength() + bioLine3.getLength())
			{
				text = GadgetStaticTextGetText(bioLine3Entry);
				wChar = bioLine3.getCharAt(bioTextPosition - bioLine1.getLength() - bioLine2.getLength());
				window = bioLine3Entry;
			}
			else
			{
				text = GadgetStaticTextGetText(bioLine4Entry);
				wChar = bioLine4.getCharAt(bioTextPosition - bioLine1.getLength() - bioLine2.getLength() - bioLine3.getLength());
				window = bioLine4Entry;
			}

			text.concat(wChar);
			GadgetStaticTextSetText(window, text);
			bioTextPosition++;
			ret = TRUE;
		}
	}

	return ret;
}


//-------------------------------------------------------------------------------------------------
/** init the challenge mode menu */
//-------------------------------------------------------------------------------------------------
void ChallengeMenuInit( WindowLayout *layout, void *userData )
{
	if( !TheChallengeGameInfo )
		TheChallengeGameInfo = NEW SkirmishGameInfo;

	TheChallengeGameInfo->init();  
	TheChallengeGameInfo->clearSlotList();
	TheChallengeGameInfo->reset();
	TheChallengeGameInfo->enterGame();


	TheShell->showShellMap(TRUE);

	// init window ids and pointers
	parentID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:ParentChallengeMenu") );
	parentMenu = TheWindowManager->winGetWindowFromId( NULL, parentID );
	buttonPlayID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:ButtonPlay") );
	buttonPlay = TheWindowManager->winGetWindowFromId( parentMenu, buttonPlayID );
	buttonBackID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:ButtonBack") );
	buttonBack = TheWindowManager->winGetWindowFromId( parentMenu, buttonBackID );
	bioPortraitID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:BioPortrait") );
	bioPortrait = TheWindowManager->winGetWindowFromId( parentMenu, bioPortraitID );
	bioNameEntryID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:BioNameEntry") );
	bioLine1Entry = TheWindowManager->winGetWindowFromId( parentMenu, bioNameEntryID ); // this window has been repurposed
	bioDOBEntryID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:BioDOBEntry") );
	bioLine2Entry = TheWindowManager->winGetWindowFromId( parentMenu, bioDOBEntryID ); // this window has been repurposed
	bioBirthplaceEntryID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:BioBirthplaceEntry") );
	bioLine3Entry = TheWindowManager->winGetWindowFromId( parentMenu, bioBirthplaceEntryID ); // this window has been repurposed
	bioStrategyEntryID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:BioStrategyEntry") );
	bioLine4Entry = TheWindowManager->winGetWindowFromId( parentMenu, bioStrategyEntryID ); // this window has been repurposed
	backdropID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:MainBackdrop") );
	backdrop = TheWindowManager->winGetWindowFromId( parentMenu, backdropID);
	bioParentID = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeMenu.wnd:GeneralsBioParent") );
	bioParent = TheWindowManager->winGetWindowFromId( parentMenu, bioParentID);

	AsciiString strButtonName;
	for (Int i = 0; i < NUM_GENERALS; i++)
	{
		strButtonName.format("ChallengeMenu.wnd:GeneralPosition%d", i);
		buttonGeneralPositionID[i] = TheNameKeyGenerator->nameToKey( strButtonName );
		buttonGeneralPosition[i] = TheWindowManager->winGetWindowFromId( parentMenu, buttonGeneralPositionID[i] );
		DEBUG_ASSERTCRASH(buttonGeneralPosition[i], ("Could not find the ButtonGeneralPosition[%d]",i ));

		// start all buttons hidden, then expose them later if there is a general for this spot
		buttonGeneralPosition[i]->winHide( TRUE );
	}

	// set defaults
	bioParent->winHide(TRUE);
	buttonPlay->winHide(TRUE);
	isAutoSelecting = FALSE;
	buttonSequenceStep = 0;
	setEnabledButtons();

	// show menu
	layout->hide( FALSE );

	// set keyboard focus to main parent
	TheWindowManager->winSetFocus( parentMenu );
	justEntered = TRUE;
	initialGadgetDelay = 2;
	GameWindow *winGadgetParent = TheWindowManager->winGetWindowFromId(NULL, TheNameKeyGenerator->nameToKey("ChallengeMenu.wnd:GadgetParent"));
	if(winGadgetParent)
		winGadgetParent->winHide(TRUE);
	isShuttingDown = FALSE;

	if(!wndVideoManager)
		wndVideoManager = NEW WindowVideoManager;
	wndVideoManager->init();

	lastSelectionSound = NULL;
	lastPreviewSound = NULL;
	hasPlayedIntroAudio = FALSE;

}


//-------------------------------------------------------------------------------------------------
/** update the challenge mode menu */
//-------------------------------------------------------------------------------------------------
void ChallengeMenuUpdate( WindowLayout *layout, void *userData )
{
	if(justEntered)
	{
		if(initialGadgetDelay == 1)
		{
			TheTransitionHandler->setGroup("ChallengeMenuFade");
//			TheTransitionHandler->setGroup("ChallengeButtonsIntro");


			initialGadgetDelay = 2;
			justEntered = FALSE;
		}
		else
			initialGadgetDelay--;
	}

//	if (TheTransitionHandler->isFinished())
//		updateButtonSequence( 1 );

	// delay the voice for N updates after the transition is done
	if (!hasPlayedIntroAudio && TheTransitionHandler->isFinished())
	{
		introAudioMagicNumber++;
		if (introAudioMagicNumber == 10)
		{
			// "Choose your general."
			AudioEventRTS event( "Taunts_GCAnnouncer01" );
			TheAudio->addAudioEvent( &event );
			hasPlayedIntroAudio = TRUE;
		}
	}

	updateBio( TELETYPE_SKIP );

	if(isShuttingDown && TheShell->isAnimFinished() && TheTransitionHandler->isFinished())
	{
		TheShell->shutdownComplete( layout );
	}

	if(wndVideoManager)
		wndVideoManager->update();
}


//-------------------------------------------------------------------------------------------------
/** shutdown the challenge mode menu */
//-------------------------------------------------------------------------------------------------
void ChallengeMenuShutdown( WindowLayout *layout, void *userData )
{
	if(wndVideoManager)
		delete wndVideoManager;
	wndVideoManager = NULL;

	lastButtonIndex = -1;

	buttonSequenceStep = 0;
	
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{
		layout->hide( TRUE );
		TheShell->shutdownComplete( layout );
		return;
	}

	TheTransitionHandler->reverse("ChallengeMenuFade");
	isShuttingDown = TRUE;

	if(TheChallengeGameInfo)
		delete TheChallengeGameInfo;
	TheChallengeGameInfo = NULL;

	TheAudio->removeAudioEvent( lastSelectionSound );
	TheAudio->removeAudioEvent( lastPreviewSound );
	lastSelectionSound = NULL;
	lastPreviewSound = NULL;
	introAudioMagicNumber = 0;
}


//-------------------------------------------------------------------------------------------------
/** challenge mode menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ChallengeMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
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

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED, (WindowMsgData)buttonBack, buttonBackID );

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;

}


//-------------------------------------------------------------------------------------------------
/** challenge mode menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ChallengeMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg ) 
	{
		case GWM_CREATE: break;

		case GWM_DESTROY: break;

		case GWM_INPUT_FOCUS:
		{
			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;
		}

		case GBM_MOUSE_ENTERING:
		{
			// preview the bio for this position
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			Int buttonIndex = findPositionButton(controlID);
			if( buttonIndex != -1 && buttonIndex != lastButtonIndex )
			{
				setGeneralBio(buttonIndex);

				// special sound for Harvard
				AudioEventRTS event( "GUILogoMouseOver" );
				TheAudio->addAudioEvent( &event );

				lastHilitedIndex = buttonIndex;
			}

			break;
		}

		case GBM_MOUSE_LEAVING:
		{
			// set the bio back to the selected one
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			Int buttonIndex = findPositionButton(controlID);
			if( buttonIndex != -1 && buttonIndex != lastButtonIndex )
			{
				if (lastButtonIndex == -1)
				{
					// they're just browsing and haven't made a selection yet
					// @todo: make this a nice transition
//					bioParent->winHide(TRUE);
//					buttonPlay->winHide(TRUE);
				}

				setGeneralBio(lastButtonIndex);
			}
			break;
		}

		case GBM_SELECTED:
		{
			UnicodeString filename;
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

 			// we don't need to handle these
 			if ( isAutoSelecting )
 			{
 				isAutoSelecting = FALSE;
 				break;
 			}

			Int buttonIndex = findPositionButton(controlID);
			if( buttonIndex != -1)
			{
				// partial radio button behavior (I'm not using actual radio buttons
				// because it limits options on how the interface can work, which is a
				// bad thing when the design is constantly in flux.)
				// ...basically this just makes you have exactly one button selected
				// once the first choice has been made.
				if (lastButtonIndex != -1)
				{
					isAutoSelecting = TRUE;
					GameWindow *lastControl = TheWindowManager->winGetWindowFromId( NULL, buttonGeneralPositionID[lastButtonIndex]);
					GadgetCheckBoxToggle(lastControl);
				}

				// play audio to indicate selection
				TheAudio->removeAudioEvent(lastSelectionSound);
				TheAudio->removeAudioEvent(lastPreviewSound);
				const GeneralPersona* generals = TheChallengeGenerals->getChallengeGenerals();
				AudioEventRTS event( generals[buttonIndex].getPreviewSound() );
				lastPreviewSound = TheAudio->addAudioEvent( &event );

/*
				// play audio to indicate selection
				TheAudio->removeAudioEvent(lastSelectionSound);
				TheAudio->removeAudioEvent(lastPreviewSound);
				const GeneralPersona* generals = TheChallengeGenerals->getChallengeGenerals();
				AudioEventRTS event( generals[buttonIndex].getSelectionSound() );
				lastSelectionSound = TheAudio->addAudioEvent( &event );
*/

				lastButtonIndex = buttonIndex;

				buttonPlay->winHide(FALSE);

				//@todo: outro transitions
			}
			else if( controlID == buttonPlayID )
 			{
				if( TheChallengeGameInfo == NULL )
				{
					// If this is NULL, then we must be on the way back out of this menu.  
					// Don't crash, just eat the button click message.
					return MSG_HANDLED;
				}

				setGeneralCampaign(lastButtonIndex);
				TheWritableGlobalData->m_pendingFile = TheCampaignManager->getCurrentMap();
				TheChallengeGameInfo->setMap(TheCampaignManager->getCurrentMap());

				// turn off the last button so the screen will be pristine when the user returns
				isAutoSelecting = TRUE;
				GameWindow *lastControl = TheWindowManager->winGetWindowFromId( NULL, buttonGeneralPositionID[lastButtonIndex]);
				GadgetCheckBoxSetChecked(lastControl, FALSE);
				lastButtonIndex = -1;
//				introAudioHasPlayed = FALSE;

				buttonSequenceStep = 0;

				if (TheGameLogic->isInGame())
					TheGameLogic->clearGameData();

				// If the campaign has been reset, so has the campaign difficulty.  Restore it, just in case.
				DEBUG_ASSERTCRASH(TheChallengeGenerals, ("TheChallengeGenerals are not initialized."));
				if (TheChallengeGenerals) 
				{
	        TheCampaignManager->setGameDifficulty(TheChallengeGenerals->getCurrentDifficulty());
					TheScriptEngine->setGlobalDifficulty(TheChallengeGenerals->getCurrentDifficulty());
				}

				// put a request in the message stream for a new game
				GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
				msg->appendIntegerArgument(GAME_SINGLE_PLAYER);
				msg->appendIntegerArgument(TheCampaignManager->getGameDifficulty());
				msg->appendIntegerArgument(TheCampaignManager->getRankPoints());
	
        
        // Added so that, even though a ChallengeGame is really a SkirmishGame in SinglePlayerGame's clothing,
        // GameEngine will still apply the default "FRAME CAP" as it does during "Solo Missions."
        msg->appendIntegerArgument(LOGICFRAMES_PER_SECOND);	// FPS limit
				
				InitRandom(0);
			}
			else if( controlID == buttonBackID )
			{
				TheShell->pop();
			}
			break;
		}

		default: return MSG_IGNORED;
	}

	return MSG_HANDLED;
}