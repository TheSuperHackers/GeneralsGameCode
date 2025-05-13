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

// FILE: LoadScreen.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Mar 2002
//
//	Filename: 	LoadScreen.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	Contains each of the different derived LoadClasses for each of the
//						Different kind of games we can have.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/NameKeyGenerator.h"
#include "Common/AudioAffect.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioHandleSpecialValues.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/GameLOD.h"
#include "Common/GameState.h"
#include "Common/MultiplayerSettings.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/Display.h"
#include "GameClient/GadgetProgressBar.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GameWindowTransitions.h"
#include "GameClient/LoadScreen.h"
#include "GameClient/MapUtil.h"
#include "GameClient/Mouse.h"
#include "GameClient/Shell.h"
#include "GameClient/VideoPlayer.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/WindowVideoManager.h"
#include "GameClient/ChallengeGenerals.h"
#include "GameLogic/FPUControl.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/RankPointValue.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-----------------------------------------------------------------------------
// PRIVATE TYPES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE DATA ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC DATA ////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
void positionStartSpots( GameInfo *myGame, GameWindow *buttonMapStartPositions[], GameWindow *mapWindow);
void updateMapStartSpots( GameInfo *myGame, GameWindow *buttonMapStartPositions[], Bool onLoadScreen = FALSE );
void positionAdditionalImages( MapMetaData *mmd, GameWindow *mapWindow, Bool force);

enum{
FRAME_TITLES_START = 20,
FRAME_TELETYPE_START = 24,
FRAME_FUDGE_ADD = 30,
FRAME_PORTRAITS_START = 35,
FRAME_OUTER_CIRCLE_LINE_SHOW = 50,
FRAME_INNER_CIRCLE_LINE_SHOW = 52,
FRAME_OUTER_CIRCLE_ALPHA_SHOW = 63,
FRAME_INNER_CIRCLE_ALPHA_SHOW = 74,
FRAME_OUTER_CIRCLE_LINE_HIDE = 75,
FRAME_INNER_BACKDROP_ALPHA_SHOW = 80,
FRAME_INNER_CIRCLE_LINE_HIDE = 81,
FRAME_VS_ANIM_START = 98,
FRAME_RIGHT_VOICE = 140,
};

static const Int TELETYPE_UPDATE_FREQ = 2; // how many frames between teletype updates



//-----------------------------------------------------------------------------
// LoadScreen Class 
//-----------------------------------------------------------------------------

LoadScreen::LoadScreen( void )
{
	m_loadScreen = NULL;		
}

LoadScreen::~LoadScreen( void )
{
	//if(m_loadScreen)
	//	delete (m_loadScreen);
	if(m_loadScreen)
		TheWindowManager->winDestroy( m_loadScreen );
	m_loadScreen = NULL;
}

void LoadScreen::update( Int percent )
{
	TheGameEngine->serviceWindowsOS();
	if (TheGameEngine->getQuitting())
		return;	//don't bother with any of this if the player is exiting game.

	TheWindowManager->update();
	TheDisplay->update();
	// redraw all views, update the GUI
	TheDisplay->draw();

	setFPMode();
}


// SinglePlayerLoadScreen Class ///////////////////////////////////////////////
//-----------------------------------------------------------------------------
SinglePlayerLoadScreen::SinglePlayerLoadScreen( void )
{
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_currentObjectiveLine = 0;
	m_currentObjectiveLineCharacter = 0;
	m_finishedObjectiveText = NULL;
	m_currentObjectiveWidthOffset = 0;
	//
	m_progressBar = NULL;
	m_percent = NULL;
	m_videoStream = NULL;
	m_videoBuffer = NULL;
	m_objectiveWin = NULL;
	for(Int i = 0; i < MAX_OBJECTIVE_LINES; ++i)
		m_objectiveLines[i] = NULL;

}
	
SinglePlayerLoadScreen::~SinglePlayerLoadScreen( void )
{
	m_progressBar = NULL;
	m_percent = NULL;
	m_objectiveWin = NULL;
	for(Int i = 0; i < MAX_OBJECTIVE_LINES; ++i)
		m_objectiveLines[i] = NULL;
	if(m_videoBuffer)
		delete m_videoBuffer;
	m_videoBuffer = NULL;

	if ( m_videoStream )
		m_videoStream->close();
	m_videoStream = NULL;
	
	TheAudio->removeAudioEvent( m_ambientLoopHandle );
	m_ambientLoopHandle = NULL;
	
}

void SinglePlayerLoadScreen::moveWindows( Int frame )
{
	enum{
		STATE_BEGIN = 250,
		STATE_SHOW_LOCATION = 251,
		STATE_BEGIN_BREIFING = 255,
//		STATE_BEGIN_ANIMATING_TEXT = 250,
		STATE_SHOW_CAMEO_1 = 434,
		STATE_BEGIN_ANIMATING_TEXT = 356,
		STATE_HIDE_CAMEO_1 = 459,
		STATE_SHOW_CAMEO_2 = 464,
		STATE_HIDE_CAMEO_2 = 492,
		STATE_SHOW_CAMEO_3 = 497,
		STATE_HIDE_CAMEO_3 = 524,
//		STATE_END_ANIM_HEAD = 450,
		STATE_END_ANIMATING_TEXT = 730,
		STATE_END = 730
	};
	if(frame < STATE_BEGIN || frame > STATE_END)
		return;
	
	if( frame == STATE_BEGIN_BREIFING)
	{
		// add sound support here
		TheAudio->friend_forcePlayAudioEventRTS(&TheCampaignManager->getCurrentMission()->m_briefingVoice);
	}

	if( frame == STATE_BEGIN_ANIMATING_TEXT)
	{
		m_objectiveWin->winHide(FALSE);
		// animate the text and stuff
	}
	
	if( frame > STATE_BEGIN_ANIMATING_TEXT && frame <= STATE_END_ANIMATING_TEXT && !m_finishedObjectiveText)
	{
		if(m_currentObjectiveLineCharacter >= m_unicodeObjectiveLines[m_currentObjectiveLine].getLength() )
		{
			m_currentObjectiveLine++;
			m_currentObjectiveLineCharacter =0;
		}
		if(m_currentObjectiveLine >= MAX_OBJECTIVE_LINES || m_unicodeObjectiveLines[m_currentObjectiveLine].isEmpty())
		{
			m_finishedObjectiveText = TRUE;
		}
		else
		{
			WideChar wChar = m_unicodeObjectiveLines[m_currentObjectiveLine].getCharAt(m_currentObjectiveLineCharacter);
			UnicodeString text = GadgetStaticTextGetText(m_objectiveLines[m_currentObjectiveLine]);
			text.concat(wChar);
			GadgetStaticTextSetText(m_objectiveLines[m_currentObjectiveLine], text);

		}
		m_currentObjectiveLineCharacter++;
	}
	switch (frame) {
	
	case STATE_SHOW_LOCATION:
		m_location->winHide(FALSE);
		break;
	case STATE_SHOW_CAMEO_1:
		m_unitDesc[0]->winHide(FALSE);
		break;
	case STATE_HIDE_CAMEO_1:
		m_unitDesc[0]->winHide(TRUE);
		break;
	case STATE_SHOW_CAMEO_2:
		m_unitDesc[1]->winHide(FALSE);
		break;
	case STATE_HIDE_CAMEO_2:
		m_unitDesc[1]->winHide(TRUE);
		break;
	case STATE_SHOW_CAMEO_3:
		m_unitDesc[2]->winHide(FALSE);
		break;
	case STATE_HIDE_CAMEO_3:
		m_unitDesc[2]->winHide(TRUE);
		break;
	}

}
/*
	static Bool on = FALSE;
	static ICoord2D startPos, endPos;
	enum{
		STATE_BEGIN = 275,
		STATE_BEGIN_ANIM = 290,
		STATE_ANIM_CAMEO1 = 300,
		STATE_ANIM_CAMEO1_TRASITION_CAMEO2 = 350,
		STATE_ANIM_CAMEO2 = 400,
		STATE_ANIM_CAMEO2_TRASITION_CAMEO3 = 450,
		STATE_ANIM_CAMEO3 = 500,
		STATED_END_ANIM = 550,
		STATE_END = 800
	};
	if(frame < STATE_BEGIN)
		return;
	else if(frame == STATE_BEGIN )
	{
		m_cameoWindow1->winHide(FALSE);
		m_cameoWindow2->winHide(FALSE);
		m_cameoWindow3->winHide(FALSE);
		m_cameoFrame->winHide(FALSE);
	}
	else if( frame == STATE_ANIM_CAMEO1)
	{
		m_cameoWindow1->winEnable(TRUE);
		GadgetStaticTextSetText(m_cameoText, TheGameText->fetch(TheCampaignManager->getCurrentMission()->m_cameoImageName[0]));
		//save of positions
	}
	else if( frame == STATE_ANIM_CAMEO1_TRASITION_CAMEO2)
	{
		m_cameoWindow1->winEnable(FALSE);
		GadgetStaticTextSetText(m_cameoText, UnicodeString::TheEmptyString);
		ICoord2D tempPos;
		Int xOffset;
		m_cameoFrame->winGetPosition(&startPos.x, &startPos.y);
		m_cameoWindow1->winGetPosition(&tempPos.x, &tempPos.y);
		xOffset = tempPos.x - startPos.x;
		m_cameoWindow2->winGetPosition(&endPos.x, &endPos.y);
		endPos.x = endPos.x - xOffset;
		endPos.y = startPos.y;
	
	}
	else if( frame > STATE_ANIM_CAMEO1_TRASITION_CAMEO2 && frame < STATE_ANIM_CAMEO2)
	{
		
		//extrapolate between start and end pos
		Real percent = INT_TO_REAL((frame - STATE_ANIM_CAMEO1_TRASITION_CAMEO2)) / (STATE_ANIM_CAMEO2 - STATE_ANIM_CAMEO1_TRASITION_CAMEO2);
		m_cameoFrame->winSetPosition(startPos.x + (endPos.x - startPos.x) * percent, endPos.y);
	}
	else if( frame == STATE_ANIM_CAMEO2 )
	{
		m_cameoWindow2->winEnable(TRUE);
		m_cameoFrame->winSetPosition(endPos.x, endPos.y);
		GadgetStaticTextSetText(m_cameoText, TheGameText->fetch(TheCampaignManager->getCurrentMission()->m_cameoImageName[1]));
	}
	else if( frame == STATE_ANIM_CAMEO2_TRASITION_CAMEO3)
	{
		m_cameoWindow2->winEnable(FALSE);
		GadgetStaticTextSetText(m_cameoText, UnicodeString::TheEmptyString);
		ICoord2D tempPos;
		Int xOffset;
		m_cameoFrame->winGetPosition(&startPos.x, &startPos.y);
		m_cameoWindow2->winGetPosition(&tempPos.x, &tempPos.y);
		xOffset = tempPos.x - startPos.x;
		m_cameoWindow3->winGetPosition(&endPos.x, &endPos.y);
		endPos.x = endPos.x - xOffset;
		endPos.y = startPos.y;
	
	}
	else if( frame > STATE_ANIM_CAMEO2_TRASITION_CAMEO3 && frame < STATE_ANIM_CAMEO3)
	{
		
		//extrapolate between start and end pos
		Real percent = INT_TO_REAL((frame - STATE_ANIM_CAMEO2_TRASITION_CAMEO3)) / (STATE_ANIM_CAMEO3 - STATE_ANIM_CAMEO2_TRASITION_CAMEO3);
		m_cameoFrame->winSetPosition(startPos.x + (endPos.x - startPos.x) * percent, endPos.y);
	}
	else if( frame == STATE_ANIM_CAMEO3 )
	{
		m_cameoFrame->winSetPosition(endPos.x, endPos.y);
		m_cameoWindow3->winEnable(TRUE);
		GadgetStaticTextSetText(m_cameoText, TheGameText->fetch(TheCampaignManager->getCurrentMission()->m_cameoImageName[2]));
	}
	else if( frame ==STATED_END_ANIM)
	{
		m_cameoWindow3->winEnable(FALSE);
		GadgetStaticTextSetText(m_cameoText, UnicodeString::TheEmptyString);
		m_cameoFrame->winHide(TRUE);
		
	}
}*/
 
void SinglePlayerLoadScreen::init( GameInfo *game )
{
	//No music in SinglePlayerLoadScreen

	// create the layout of the load screen
	m_loadScreen = TheWindowManager->winCreateFromScript( AsciiString( "Menus/SinglePlayerLoadScreen.wnd" ) );
	DEBUG_ASSERTCRASH(m_loadScreen, ("Can't initialize the single player loadscreen"));
	m_loadScreen->winHide(FALSE);
	m_loadScreen->winBringToTop();
//	Mission *mission = TheCampaignManager->getCurrentMission();
	// Store the pointer to the progress bar on the loadscreen
	m_progressBar = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:ProgressLoad" ) ));
	DEBUG_ASSERTCRASH(m_progressBar, ("Can't initialize the progressbar for the single player loadscreen"));
	GadgetProgressBarSetProgress(m_progressBar, 0 );	

	m_percent = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:Percent" ) ));
	DEBUG_ASSERTCRASH(m_percent, ("Can't initialize the m_percent for the single player loadscreen"));
	GadgetStaticTextSetText(m_percent,UnicodeString(L"0%"));
	m_percent->winHide(TRUE);

	m_objectiveWin = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:ObjectivesWin" ) ));
	DEBUG_ASSERTCRASH(m_objectiveWin, ("Can't initialize the m_objectiveWin for the single player loadscreen"));
	m_objectiveWin->winHide(TRUE);

	
	Mission *mission = TheCampaignManager->getCurrentMission();
	AsciiString lineName;
	Int i = 0;
	for(; i < MAX_OBJECTIVE_LINES; ++i)
	{
		lineName.format("SinglePlayerLoadScreen.wnd:StaticTextLine%d",i);
		m_objectiveLines[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( lineName ));
		DEBUG_ASSERTCRASH(m_objectiveLines[i], ("Can't initialize the m_objectiveLines[%d] for the single player loadscreen", i));
		GadgetStaticTextSetText(m_objectiveLines[i],UnicodeString::TheEmptyString);

		// translate the objective lines
		if(mission->m_missionObjectivesLabel[i].isNotEmpty())
			m_unicodeObjectiveLines[i] = TheGameText->fetch(mission->m_missionObjectivesLabel[i]);
	}

	for(i = 0; i < MAX_DISPLAYED_UNITS; ++i)
	{
		lineName.format("SinglePlayerLoadScreen.wnd:StaticTextCameoText%d",i);
		m_unitDesc[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( lineName ));
		DEBUG_ASSERTCRASH(m_unitDesc[i], ("Can't initialize the m_objectiveLines[%d] for the single player loadscreen", i));
		GadgetStaticTextSetText(m_unitDesc[i],TheGameText->fetch(mission->m_unitNames[i]));
		m_unitDesc[i]->winHide(TRUE);
	}
	m_location = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:StaticTextCameoText3" ) ));
	DEBUG_ASSERTCRASH(m_location, ("Can't initialize the m_objectiveWin for the single player loadscreen"));
	m_location->winHide(TRUE);
	GadgetStaticTextSetText(m_location, TheGameText->fetch(mission->m_locationNameLabel));
	

	
	m_currentObjectiveLine = 0;
	m_currentObjectiveWidthOffset = 0;
	m_currentObjectiveLineCharacter = 0;
	m_finishedObjectiveText = FALSE;
/*
	m_cameoWindow1 = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:WindowCameo1" ) ));
	DEBUG_ASSERTCRASH(m_cameoWindow1, ("Can't initialize the m_cameoWindow1 for the single player loadscreen"));
	m_cameoWindow1->winHide(TRUE);
	m_cameoWindow1->winEnable(FALSE);
	m_cameoWindow1->winSetEnabledImage(0, mission->m_cameoImage[0]);
	m_cameoWindow1->winSetDisabledImage(0, mission->m_cameoDisabledImage[0]);

	m_cameoWindow2 = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:WindowCameo2" ) ));
	DEBUG_ASSERTCRASH(m_cameoWindow2, ("Can't initialize the m_cameoWindow2 for the single player loadscreen"));
	m_cameoWindow2->winHide(TRUE);
	m_cameoWindow2->winEnable(FALSE);
	m_cameoWindow2->winSetEnabledImage(0, mission->m_cameoImage[1]);
	m_cameoWindow2->winSetDisabledImage(0, mission->m_cameoDisabledImage[1]);

	m_cameoWindow3 = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:WindowCameo3" ) ));
	DEBUG_ASSERTCRASH(m_cameoWindow3, ("Can't initialize the m_cameoWindow3 for the single player loadscreen"));
	m_cameoWindow3->winHide(TRUE);
	m_cameoWindow3->winEnable(FALSE);
	m_cameoWindow3->winSetEnabledImage(0, mission->m_cameoImage[2]);
	m_cameoWindow3->winSetDisabledImage(0, mission->m_cameoDisabledImage[2]);

	m_headMovie = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:WindowHead" ) ));
	DEBUG_ASSERTCRASH(m_headMovie, ("Can't initialize the m_headMovie for the single player loadscreen"));
	m_headMovie->winHide(TRUE);
	m_cameoFrame = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:WindowHiliteCameo" ) ));
	DEBUG_ASSERTCRASH(m_cameoFrame, ("Can't initialize the m_cameoFrame for the single player loadscreen"));
	m_cameoFrame->winHide(TRUE);
	m_cameoText = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:StaticTextCameoText" ) ));
	DEBUG_ASSERTCRASH(m_cameoText, ("Can't initialize the m_cameoText for the single player loadscreen"));

*/
	m_ambientLoop.setEventName("LoadScreenAmbient");
	// create the new stream
	m_videoStream = TheVideoPlayer->open( TheCampaignManager->getCurrentMission()->m_movieLabel );
	if ( m_videoStream == NULL )
	{
		m_percent->winHide(TRUE);
		return;
	}

	// Create the new buffer
	m_videoBuffer = TheDisplay->createVideoBuffer();
	if (	m_videoBuffer == NULL || 
				!m_videoBuffer->allocate(	m_videoStream->width(), 
													m_videoStream->height())
		)
	{
		delete m_videoBuffer;
		m_videoBuffer = NULL;

		if ( m_videoStream )
			m_videoStream->close();
		m_videoStream = NULL;

		return;
	}

	// format the progress bar: USA to blue, GLA to green, China to red
	// and set the background image
	AsciiString campaignName = TheCampaignManager->getCurrentCampaign()->m_name;
	GameWindow *backgroundWin = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "SinglePlayerLoadScreen.wnd:ParentSinglePlayerLoadScreen" ) ));
	if (campaignName.compareNoCase("USA") == 0)
	{
		backgroundWin->winSetEnabledImage( 0, TheMappedImageCollection->findImageByName("MissionLoad_USA") );
		m_progressBar->winSetEnabledImage( 6, TheMappedImageCollection->findImageByName("LoadingBar_ProgressCenter2") );
	}
	else if (campaignName.compareNoCase("GLA") == 0)
	{
		backgroundWin->winSetEnabledImage( 0, TheMappedImageCollection->findImageByName("MissionLoad_GLA") );
		m_progressBar->winSetEnabledImage( 6, TheMappedImageCollection->findImageByName("LoadingBar_ProgressCenter3") );
	}
	else if (campaignName.compareNoCase("China") == 0)
	{
		backgroundWin->winSetEnabledImage( 0, TheMappedImageCollection->findImageByName("MissionLoad_China") );
		m_progressBar->winSetEnabledImage( 6, TheMappedImageCollection->findImageByName("LoadingBar_ProgressCenter1") );
	} 
	// else leave the default background screen


	if(TheGameLODManager && TheGameLODManager->didMemPass())
	{
		Int progressUpdateCount = m_videoStream->frameCount() / FRAME_FUDGE_ADD;
		Int shiftedPercent = -FRAME_FUDGE_ADD + 1;
		while (m_videoStream->frameIndex() < m_videoStream->frameCount() - 1 )
		{
			TheGameEngine->serviceWindowsOS();

			if(!m_videoStream->isFrameReady())
			{
				Sleep(1);	
				continue;
			}

			if (!TheGameEngine->isActive())
			{/*	//we are alt-tabbed out, so just increment the frame
				m_videoStream->frameNext();
				m_videoStream->frameDecompress();*/

				//Changing for MissionDisk, just skip to end.
				break;
			}
			
			m_videoStream->frameDecompress();
			m_videoStream->frameRender(m_videoBuffer);

// PULLED FROM THE MISSION DISK
//			moveWindows( m_videoStream->frameIndex());

			m_videoStream->frameNext();

			if(m_videoBuffer)
				m_loadScreen->winGetInstanceData()->setVideoBuffer(m_videoBuffer);
			if(m_videoStream->frameIndex() % progressUpdateCount == 0)
			{
				shiftedPercent++;
				if(shiftedPercent >0)
					shiftedPercent = 0;
				Int percent = (shiftedPercent + FRAME_FUDGE_ADD)/1.3;
				UnicodeString per;
				per.format(L"%d%%",percent);
				TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);
				GadgetProgressBarSetProgress(m_progressBar, percent);
				GadgetStaticTextSetText(m_percent, per);
			
			}
			TheWindowManager->update();

			// redraw all views, update the GUI
			TheDisplay->draw();
		}

		// let the background image show through
		m_videoStream->close();
		m_videoStream = NULL;
		m_loadScreen->winGetInstanceData()->setVideoBuffer( NULL );
		TheDisplay->draw();
	}
	else
	{
		// if we're min spec'ed don't play a movie
		
		Int delay = mission->m_voiceLength * 1000;
		Int begin = timeGetTime();
		Int currTime = begin;
		Int fudgeFactor = 0;
		while(begin + delay > currTime )
		{		
			fudgeFactor = 30 * ((currTime - begin)/ INT_TO_REAL(delay ));
			GadgetProgressBarSetProgress(m_progressBar, fudgeFactor);

			TheWindowManager->update();
			TheDisplay->draw();
			Sleep(100);
			currTime = timeGetTime();
		}
		

		TheWindowManager->update();
		TheDisplay->draw();

	}
	setFPMode();
	m_percent->winHide(TRUE);
	m_ambientLoopHandle = TheAudio->addAudioEvent(&m_ambientLoop);
	
}

void SinglePlayerLoadScreen::reset( void )
{
 setLoadScreen(NULL);
 m_progressBar = NULL;
}

void SinglePlayerLoadScreen::update( Int percent )
{
	percent = (percent + FRAME_FUDGE_ADD)/1.3;
	UnicodeString per;
	per.format(L"%d%%",percent);
	TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);
	GadgetProgressBarSetProgress(m_progressBar, percent);
	GadgetStaticTextSetText(m_percent, per);
	
	// Do this last!
	LoadScreen::update( percent );
}

void SinglePlayerLoadScreen::setProgressRange( Int min, Int max )
{

}

// ChallengeLoadScreen Class ///////////////////////////////////////////////
//-----------------------------------------------------------------------------
ChallengeLoadScreen::ChallengeLoadScreen( void )
{
	m_progressBar = NULL;
	m_videoStream = NULL;
	m_videoBuffer = NULL;

	m_bioNameLeft = NULL;
	m_bioAgeLeft = NULL;
	m_bioBirthplaceLeft = NULL;
	m_bioStrategyLeft = NULL;
	m_bioBigNameEntryLeft = NULL;
	m_bioNameEntryLeft = NULL;
	m_bioAgeEntryLeft = NULL;
	m_bioBirthplaceEntryLeft = NULL;
	m_bioStrategyEntryLeft = NULL;
	m_bioBigNameEntryRight = NULL;
	m_bioNameRight = NULL;
	m_bioAgeRight = NULL;
	m_bioBirthplaceRight = NULL;
	m_bioStrategyRight = NULL;
	m_bioNameEntryRight = NULL;
	m_bioAgeEntryRight = NULL;
	m_bioBirthplaceEntryRight = NULL;
	m_bioStrategyEntryRight = NULL;

	m_portraitLeft = NULL;
	m_portraitRight = NULL;
	m_portraitMovieLeft = NULL;
	m_portraitMovieRight = NULL;

//	m_overlayReticleCrosshairs = NULL;
//	m_overlayReticleCircleLineOuter = NULL;
//	m_overlayReticleCircleLineInner = NULL;
	m_overlayReticleCircleAlphaOuter = NULL;
	m_overlayReticleCircleAlphaInner = NULL;
	m_overlayVsBackdrop = NULL;
	m_overlayVs = NULL;
	m_wndVideoManager = NULL;
}
	
ChallengeLoadScreen::~ChallengeLoadScreen( void )
{
	m_progressBar = NULL;
	if(m_videoBuffer)
		delete m_videoBuffer;
	m_videoBuffer = NULL;
	if ( m_videoStream )
		m_videoStream->close();
	m_videoStream = NULL;
	
	m_bioNameLeft = NULL;
	m_bioAgeLeft = NULL;
	m_bioBirthplaceLeft = NULL;
	m_bioStrategyLeft = NULL;
	m_bioBigNameEntryLeft = NULL;
	m_bioNameEntryLeft = NULL;
	m_bioAgeEntryLeft = NULL;
	m_bioBirthplaceEntryLeft = NULL;
	m_bioStrategyEntryLeft = NULL;
	m_bioBigNameEntryRight = NULL;
	m_bioNameRight = NULL;
	m_bioAgeRight = NULL;
	m_bioBirthplaceRight = NULL;
	m_bioStrategyRight = NULL;
	m_bioNameEntryRight = NULL;
	m_bioAgeEntryRight = NULL;
	m_bioBirthplaceEntryRight = NULL;
	m_bioStrategyEntryRight = NULL;

	m_portraitLeft = NULL;
	m_portraitRight = NULL;
	m_portraitMovieLeft = NULL;
	m_portraitMovieRight = NULL;

//	m_overlayReticleCrosshairs = NULL;
//	m_overlayReticleCircleLineOuter = NULL;
//	m_overlayReticleCircleLineInner = NULL;
	m_overlayReticleCircleAlphaOuter = NULL;
	m_overlayReticleCircleAlphaInner = NULL;
	m_overlayVsBackdrop = NULL;
	m_overlayVs = NULL;

	if(m_wndVideoManager)
		delete m_wndVideoManager;
	m_wndVideoManager = NULL;

	TheAudio->removeAudioEvent( m_ambientLoopHandle );
	m_ambientLoopHandle = NULL;
}

// accepts the number of chars to advance, the window we're concerned with, the total text for final display, and the current position of the readout
// returns the updated position of the readout
Int updateTeletypeText( Int num_chars, GameWindow* window, UnicodeString full_text, Int current_text_pos )
{
	DEBUG_ASSERTCRASH(window, ("No window for teletype text update"));
	UnicodeString currentText = GadgetStaticTextGetText(window);
	WideChar wChar;
	for (Int i = 0; i < num_chars; i++)
	{
		if (current_text_pos < full_text.getLength())
		{
			wChar = full_text.getCharAt(current_text_pos);
			currentText.concat(wChar);
			current_text_pos++;
		}
	}	
	GadgetStaticTextSetText(window, currentText);
	return current_text_pos;
}

void ChallengeLoadScreen::activatePieces( Int frame, const GeneralPersona *generalPlayer, const GeneralPersona *generalOpponent )
{
	static Int textPosBigNameRight = 0;
	static Int textPosNameRight = 0;
	static Int textPosAgeRight = 0;
	static Int textPosBirthplaceRight = 0;
	static Int textPosStrategyRight = 0;
	static Int textPosBigNameLeft = 0;
	static Int textPosNameLeft = 0;
	static Int textPosAgeLeft = 0;
	static Int textPosBirthplaceLeft = 0;
	static Int textPosStrategyLeft = 0;

	AudioEventRTS eventLeftGeneral( generalPlayer->getNameSound() );
	AudioEventRTS eventVS("Taunts_GCAnnouncer12");
	AudioEventRTS eventRightGeneral( generalOpponent->getNameSound() );

	switch (frame)
	{
		case FRAME_TITLES_START:
			m_bioNameLeft->winHide(FALSE);
//			m_bioAgeLeft->winHide(FALSE);
			m_bioBirthplaceLeft->winHide(FALSE);
			m_bioStrategyLeft->winHide(FALSE);
			m_bioNameRight->winHide(FALSE);
//			m_bioAgeRight->winHide(FALSE);
			m_bioBirthplaceRight->winHide(FALSE);
			m_bioStrategyRight->winHide(FALSE);

			break;
		case FRAME_TELETYPE_START:
			// reinit the statics for each new load screen
			textPosBigNameRight = 0;
			textPosNameRight = 0;
			textPosAgeRight = 0;
			textPosBirthplaceRight = 0;
			textPosStrategyRight = 0;
			textPosBigNameLeft = 0;
			textPosNameLeft = 0;
			textPosAgeLeft = 0;
			textPosBirthplaceLeft = 0;
			textPosStrategyLeft = 0;

			m_bioBigNameEntryLeft->winHide(FALSE);
			m_bioNameEntryLeft->winHide(FALSE);
//			m_bioAgeEntryLeft->winHide(FALSE);
			m_bioBirthplaceEntryLeft->winHide(FALSE);
			m_bioStrategyEntryLeft->winHide(FALSE);
			GadgetStaticTextSetText( m_bioBigNameEntryLeft, UnicodeString::TheEmptyString );
			GadgetStaticTextSetText( m_bioNameEntryLeft, UnicodeString::TheEmptyString );
//			GadgetStaticTextSetText( m_bioAgeEntryLeft, UnicodeString::TheEmptyString );
			GadgetStaticTextSetText( m_bioBirthplaceEntryLeft, UnicodeString::TheEmptyString );
			GadgetStaticTextSetText( m_bioStrategyEntryLeft, UnicodeString::TheEmptyString );

			m_bioBigNameEntryRight->winHide(FALSE);
			m_bioNameEntryRight->winHide(FALSE);
//			m_bioAgeEntryRight->winHide(FALSE);
			m_bioBirthplaceEntryRight->winHide(FALSE);
			m_bioStrategyEntryRight->winHide(FALSE);
			GadgetStaticTextSetText( m_bioBigNameEntryRight, UnicodeString::TheEmptyString );
			GadgetStaticTextSetText( m_bioNameEntryRight, UnicodeString::TheEmptyString );
//			GadgetStaticTextSetText( m_bioAgeEntryRight, UnicodeString::TheEmptyString );
			GadgetStaticTextSetText( m_bioBirthplaceEntryRight, UnicodeString::TheEmptyString );
			GadgetStaticTextSetText( m_bioStrategyEntryRight, UnicodeString::TheEmptyString );
			break;
		case FRAME_PORTRAITS_START:
			
			m_wndVideoManager->playMovie( m_portraitMovieLeft, generalPlayer->getPortraitMovieLeftName(), WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME);
			m_wndVideoManager->playMovie( m_portraitMovieRight, generalOpponent->getPortraitMovieRightName(), WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME);
			m_portraitMovieLeft->winHide(FALSE);
			m_portraitMovieRight->winHide(FALSE);

			TheAudio->addAudioEvent( &eventLeftGeneral );
			
			break;
		case FRAME_OUTER_CIRCLE_LINE_SHOW:
//			m_overlayReticleCircleLineOuter->winHide(FALSE);
			break;
		case FRAME_INNER_CIRCLE_LINE_SHOW:
//			m_overlayReticleCircleLineInner->winHide(FALSE);
			break;
		case FRAME_OUTER_CIRCLE_ALPHA_SHOW:
			m_overlayReticleCircleAlphaOuter->winHide(FALSE);
			break;
		case FRAME_INNER_CIRCLE_ALPHA_SHOW:
			m_overlayReticleCircleAlphaInner->winHide(FALSE);
			break;
		case FRAME_OUTER_CIRCLE_LINE_HIDE:
//			m_overlayReticleCircleLineOuter->winHide(TRUE);
			break;
		case FRAME_INNER_BACKDROP_ALPHA_SHOW:
			m_overlayVsBackdrop->winHide(FALSE);
			break;
		case FRAME_INNER_CIRCLE_LINE_HIDE:
//			m_overlayReticleCircleLineInner->winHide(TRUE);
			break;
		case FRAME_VS_ANIM_START:
			// it's time to start the overlay movie
//					m_overlayVsBackdrop->winSetEnabledImage( 0, TheMappedImageCollection->findImageByFilename("))
			m_overlayVsBackdrop->winHide(FALSE);
			m_overlayVs->winHide(FALSE);
			m_wndVideoManager->playMovie( m_overlayVs, AsciiString("VSSmall"), WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME);

			// "Verses"
			TheAudio->addAudioEvent( &eventVS );

			break;
		case FRAME_RIGHT_VOICE:
			TheAudio->addAudioEvent( &eventRightGeneral );

			break;
	}

	// update the teletype readout
	if (frame > FRAME_TELETYPE_START && (frame % TELETYPE_UPDATE_FREQ) == 0)
	{
		textPosNameLeft = updateTeletypeText( 1, m_bioNameEntryLeft, TheGameText->fetch(generalPlayer->getBioName()), textPosNameLeft);
		textPosBigNameLeft = updateTeletypeText( 1, m_bioBigNameEntryLeft, TheGameText->fetch(generalPlayer->getBioName()), textPosBigNameLeft);
//		textPosAgeLeft = updateTeletypeText( 1, m_bioAgeEntryLeft, TheGameText->fetch(generalPlayer->getBioDOB()), textPosAgeLeft);
		textPosBirthplaceLeft = updateTeletypeText( 1, m_bioBirthplaceEntryLeft, TheGameText->fetch(generalPlayer->getBioRank()), textPosBirthplaceLeft);
		textPosStrategyLeft = updateTeletypeText( 1, m_bioStrategyEntryLeft, TheGameText->fetch(generalPlayer->getBioStrategy()), textPosStrategyLeft);
		
		textPosNameRight = updateTeletypeText( 1, m_bioNameEntryRight, TheGameText->fetch(generalOpponent->getBioName()), textPosNameRight);
		textPosBigNameRight = updateTeletypeText( 1, m_bioBigNameEntryRight, TheGameText->fetch(generalOpponent->getBioName()), textPosBigNameRight);
//		textPosAgeRight = updateTeletypeText( 1, m_bioAgeEntryRight, TheGameText->fetch(generalOpponent->getBioDOB()), textPosAgeRight);
		textPosBirthplaceRight = updateTeletypeText( 1, m_bioBirthplaceEntryRight, TheGameText->fetch(generalOpponent->getBioRank()), textPosBirthplaceRight);
		textPosStrategyRight = updateTeletypeText( 1, m_bioStrategyEntryRight, TheGameText->fetch(generalOpponent->getBioStrategy()), textPosStrategyRight);
	}
}
 
void ChallengeLoadScreen::activatePiecesMinSpec(const GeneralPersona *generalPlayer, const GeneralPersona *generalOpponent)
{
	m_bioNameLeft->winHide(FALSE);
//	m_bioAgeLeft->winHide(FALSE);
	m_bioBirthplaceLeft->winHide(FALSE);
	m_bioStrategyLeft->winHide(FALSE);
	m_bioNameRight->winHide(FALSE);
//	m_bioAgeRight->winHide(FALSE);
	m_bioBirthplaceRight->winHide(FALSE);
	m_bioStrategyRight->winHide(FALSE);
	m_bioBigNameEntryLeft->winHide(FALSE);
	m_bioNameEntryLeft->winHide(FALSE);
//	m_bioAgeEntryLeft->winHide(FALSE);
	m_bioBirthplaceEntryLeft->winHide(FALSE);
	m_bioStrategyEntryLeft->winHide(FALSE);
	GadgetStaticTextSetText( m_bioBigNameEntryLeft, TheGameText->fetch(generalPlayer->getBioName()) );
	GadgetStaticTextSetText( m_bioNameEntryLeft, TheGameText->fetch(generalPlayer->getBioName()) );
//	GadgetStaticTextSetText( m_bioAgeEntryLeft, TheGameText->fetch(generalPlayer->getBioDOB()) );
	GadgetStaticTextSetText( m_bioBirthplaceEntryLeft, TheGameText->fetch(generalPlayer->getBioRank()) );
	GadgetStaticTextSetText( m_bioStrategyEntryLeft, TheGameText->fetch(generalPlayer->getBioStrategy()) );
	m_bioBigNameEntryRight->winHide(FALSE);
	m_bioNameEntryRight->winHide(FALSE);
//	m_bioAgeEntryRight->winHide(FALSE);
	m_bioBirthplaceEntryRight->winHide(FALSE);
	m_bioStrategyEntryRight->winHide(FALSE);
	GadgetStaticTextSetText( m_bioBigNameEntryRight, TheGameText->fetch(generalOpponent->getBioName()) );
	GadgetStaticTextSetText( m_bioNameEntryRight, TheGameText->fetch(generalOpponent->getBioName()) );
//	GadgetStaticTextSetText( m_bioAgeEntryRight, TheGameText->fetch(generalOpponent->getBioDOB()) );
	GadgetStaticTextSetText( m_bioBirthplaceEntryRight, TheGameText->fetch(generalOpponent->getBioRank()) );
	GadgetStaticTextSetText( m_bioStrategyEntryRight, TheGameText->fetch(generalOpponent->getBioStrategy()) );
	m_portraitLeft->winSetEnabledImage(0, generalPlayer->getBioPortraitLarge() );
	m_portraitRight->winSetEnabledImage(0, generalOpponent->getBioPortraitLarge() );
	m_portraitLeft->winHide(FALSE);
	m_portraitRight->winHide(FALSE);
	m_overlayReticleCircleAlphaOuter->winHide(FALSE);
	m_overlayReticleCircleAlphaInner->winHide(FALSE);
	m_overlayVsBackdrop->winHide(FALSE);
	m_overlayVsBackdrop->winHide(FALSE);
	m_overlayVs->winHide(FALSE);
	m_wndVideoManager->playMovie( m_overlayVs, AsciiString("VSSmall"), WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME);
}


void ChallengeLoadScreen::init( GameInfo *game )
{
	const Campaign *campaign = TheCampaignManager->getCurrentCampaign();
	const Mission *mission = TheCampaignManager->getCurrentMission();

	// the player general is tied to the campaign
	const GeneralPersona* generalPlayer = TheChallengeGenerals->getPlayerGeneralByCampaignName( campaign->m_name );

	// the opponent general is tied to the mission
	DEBUG_ASSERTCRASH(mission->m_generalName.isNotEmpty(), ("No GeneralName associated with this mission, check Campaign.ini"));
	const GeneralPersona* generalOpponent = TheChallengeGenerals->getGeneralByGeneralName( mission->m_generalName );

	// create the layout of the load screen
	m_loadScreen = TheWindowManager->winCreateFromScript( AsciiString( "Menus/ChallengeLoadScreen.wnd" ) );
	DEBUG_ASSERTCRASH(m_loadScreen, ("Can't initialize the single player loadscreen"));
	m_loadScreen->winHide(FALSE);
	m_loadScreen->winBringToTop();

	// Store the pointer to the progress bar on the loadscreen
	m_progressBar = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "ChallengeLoadScreen.wnd:ProgressLoad" ) ));
	DEBUG_ASSERTCRASH(m_progressBar, ("Can't initialize the progressbar for the single player loadscreen"));
	GadgetProgressBarSetProgress(m_progressBar, 0 );	

	m_ambientLoop.setEventName("LoadScreenAmbient");

	// create the new background video stream
	m_videoStream = TheVideoPlayer->open( TheCampaignManager->getCurrentMission()->m_movieLabel );

	// Create the new buffer
	m_videoBuffer = TheDisplay->createVideoBuffer();
	if (m_videoBuffer == NULL || !m_videoBuffer->allocate(	m_videoStream->width(), m_videoStream->height() ))
	{
		delete m_videoBuffer;
		m_videoBuffer = NULL;

		if ( m_videoStream )
			m_videoStream->close();
		m_videoStream = NULL;

		return;
	}

	// init overlays
	NameKeyType namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:PortraitLeft"));
	m_portraitLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:PortraitRight"));
	m_portraitRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );

	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:PortraitMovieLeft"));
	m_portraitMovieLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:PortraitMovieRight"));
	m_portraitMovieRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );

//	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:ReticleCrosshairs"));
//	m_overlayReticleCrosshairs = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
/*
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:OuterCircleLine"));
	m_overlayReticleCircleLineOuter = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:InnerCircleLine"));
	m_overlayReticleCircleLineInner = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
*/
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:CircleAlphaOuter"));
	m_overlayReticleCircleAlphaOuter = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:CircleAlphaInner"));
	m_overlayReticleCircleAlphaInner = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:VersusBackdrop"));
	m_overlayVsBackdrop = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:OverlayVs"));
	m_overlayVs = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );

	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioNameLeft"));
	m_bioNameLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
//	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioDOBLeft"));
//	m_bioAgeLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioBirthplaceLeft"));
	m_bioBirthplaceLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioStrategyLeft"));
	m_bioStrategyLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BigNameEntryLeft"));
	m_bioBigNameEntryLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioNameEntryLeft"));
	m_bioNameEntryLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
//	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioDOBEntryLeft"));
//	m_bioAgeEntryLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioBirthplaceEntryLeft"));
	m_bioBirthplaceEntryLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioStrategyEntryLeft"));
	m_bioStrategyEntryLeft = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioNameRight"));
	m_bioNameRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
//	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioDOBRight"));
//	m_bioAgeRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioBirthplaceRight"));
	m_bioBirthplaceRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioStrategyRight"));
	m_bioStrategyRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BigNameEntryRight"));
	m_bioBigNameEntryRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioNameEntryRight"));
	m_bioNameEntryRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
//	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioDOBEntryRight"));
//	m_bioAgeEntryRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioBirthplaceEntryRight"));
	m_bioBirthplaceEntryRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );
	namekey = TheNameKeyGenerator->nameToKey( AsciiString("ChallengeLoadScreen.wnd:BioStrategyEntryRight"));
	m_bioStrategyEntryRight = TheWindowManager->winGetWindowFromId( m_loadScreen, namekey );


	// make sure reticle stuff starts out hidden
//	m_overlayReticleCircleLineOuter->winHide(TRUE);
//	m_overlayReticleCircleLineInner->winHide(TRUE);
	m_overlayReticleCircleAlphaOuter->winHide(TRUE);
	m_overlayReticleCircleAlphaInner->winHide(TRUE);
	m_overlayVsBackdrop->winHide(TRUE);
	m_overlayVs->winHide(TRUE);

	m_wndVideoManager = NEW WindowVideoManager;
	m_wndVideoManager->init();

	if(TheGameLODManager && TheGameLODManager->didMemPass())
	{
		Int progressUpdateCount = m_videoStream->frameCount() / FRAME_FUDGE_ADD;
		Int shiftedPercent = -FRAME_FUDGE_ADD + 1;
		while (m_videoStream->frameIndex() < m_videoStream->frameCount() - 1 )
		{
			TheGameEngine->serviceWindowsOS();

			if(!m_videoStream->isFrameReady())
			{
				Sleep(1);	
				continue;
			}

			if (!TheGameEngine->isActive())
			{	//we are alt-tabbed out, so just increment the frame
				m_videoStream->frameNext();
				m_videoStream->frameDecompress();
				continue;
			}
			
			m_videoStream->frameDecompress();
			m_videoStream->frameRender(m_videoBuffer);
			m_videoStream->frameNext();

			if(m_videoBuffer)
				m_loadScreen->winGetInstanceData()->setVideoBuffer(m_videoBuffer);

			Int frame = m_videoStream->frameIndex();
			if(frame % progressUpdateCount == 0)
			{
				shiftedPercent++;
				if(shiftedPercent >0)
					shiftedPercent = 0;
				Int percent = (shiftedPercent + FRAME_FUDGE_ADD)/1.3;
				UnicodeString per;
				per.format(L"%d%%",percent);
				TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);
				GadgetProgressBarSetProgress(m_progressBar, percent);
			}
			TheWindowManager->update();

			activatePieces(frame, generalPlayer, generalOpponent);
			m_wndVideoManager->update();

			// redraw all views, update the GUI
			TheDisplay->draw();

			TheAudio->update();
		}
	}
	else
	{
		// if we're min speced
		m_videoStream->frameGoto(m_videoStream->frameCount()); // zero based
		while(!m_videoStream->isFrameReady())
			Sleep(1);
		m_videoStream->frameDecompress();
		m_videoStream->frameRender(m_videoBuffer);
		if(m_videoBuffer)
			m_loadScreen->winGetInstanceData()->setVideoBuffer(m_videoBuffer);

		activatePiecesMinSpec(generalPlayer, generalOpponent);

		Int delay = mission->m_voiceLength * 1000;
		Int begin = timeGetTime();
		Int currTime = begin;
		Int fudgeFactor = 0;
		while(begin + delay > currTime )
		{		
			fudgeFactor = 30 * ((currTime - begin)/ INT_TO_REAL(delay ));
			GadgetProgressBarSetProgress(m_progressBar, fudgeFactor);

			TheWindowManager->update();
			TheDisplay->draw();
			Sleep(100);
			currTime = timeGetTime();
		}
		
		m_wndVideoManager->update();
		TheWindowManager->update();
		TheDisplay->draw();
	}
	setFPMode();

	
	AudioEventRTS event( generalOpponent->getRandomTauntSound() );
	TheAudio->addAudioEvent( &event );

	m_ambientLoopHandle = TheAudio->addAudioEvent(&m_ambientLoop);
	TheAudio->update();
}

void ChallengeLoadScreen::reset( void )
{
 setLoadScreen(NULL);
 m_progressBar = NULL;
}

void ChallengeLoadScreen::update( Int percent )
{
	percent = (percent + FRAME_FUDGE_ADD)/1.3;
	UnicodeString per;
	per.format(L"%d%%",percent);
	TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);
	GadgetProgressBarSetProgress(m_progressBar, percent);

	// Do this last!
	LoadScreen::update( percent );
}

void ChallengeLoadScreen::setProgressRange( Int min, Int max )
{

}

// ShellGameLoadScreen Class //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
ShellGameLoadScreen::ShellGameLoadScreen( void )
{
	m_progressBar = NULL;
}
	
ShellGameLoadScreen::~ShellGameLoadScreen( void )
{
	
	m_progressBar = NULL;
}

void ShellGameLoadScreen::init( GameInfo *game )
{
	static BOOL firstLoad = TRUE;

	
	// create the layout of the load screen
	m_loadScreen = TheWindowManager->winCreateFromScript( AsciiString( "Menus/ShellGameLoadScreen.wnd" ) );
	DEBUG_ASSERTCRASH(m_loadScreen, ("Can't initialize the ShellGame loadscreen"));
	m_loadScreen->winHide(FALSE);
	m_loadScreen->winBringToTop();

	// Store the pointer to the progress bar on the loadscreen
	m_progressBar = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "ShellGameLoadScreen.wnd:ProgressLoad" ) ));
	DEBUG_ASSERTCRASH(m_progressBar, ("Can't initialize the progressbar for the single player loadscreen"));
	GadgetProgressBarSetProgress(m_progressBar, 0 );	
	m_progressBar->winHide(TRUE);
	
	if(m_loadScreen && firstLoad && TheGameLODManager && TheGameLODManager->didMemPass())
	{
		m_loadScreen->winSetEnabledImage(0, TheMappedImageCollection->findImageByName("TitleScreen"));
		TheWritableGlobalData->m_breakTheMovie = FALSE;

//		m_videoStream = TheVideoPlayer->open( "Sizzle" );
//		if ( m_videoStream == NULL )
//		{
//			m_progressBar->winHide(FALSE);
//			return;
//		}
//
//		// Create the new buffer
//		m_videoBuffer = TheDisplay->createVideoBuffer();
//		if (	m_videoBuffer == NULL || 
//					!m_videoBuffer->allocate(	m_videoStream->width(), 
//														m_videoStream->height())
//			)
//		{
//			delete m_videoBuffer;
//			m_videoBuffer = NULL;
//
//			if ( m_videoStream )
//				m_videoStream->close();
//			m_videoStream = NULL;
//
//			return;
//		}
//		TheGlobalData->m_isBreakableMovie = TRUE;
//		TheGlobalData->m_breakTheMovie = FALSE;
//		while (m_videoStream->frameIndex() < m_videoStream->frameCount() - 1 )
//		{
//			if(TheGlobalData->m_breakTheMovie)
//			{
//				TheGlobalData->m_breakTheMovie = FALSE;
//				m_videoStream->frameGoto(m_videoStream->frameCount() - 1);
//			}
//			if(m_videoStream->frameIndex() < m_videoStream->frameCount() - 1)
//			{
//				if(!m_videoStream->isFrameReady())
//					continue;
//
//				m_videoStream->frameDecompress();
//				m_videoStream->frameRender(m_videoBuffer);
//				m_videoStream->frameNext();
//				if(m_videoBuffer)
//					m_loadScreen->winGetInstanceData()->setVideoBuffer(m_videoBuffer);
//			}
//
//			TheWindowManager->update();
//		//	TheShell->update();
//			//TheDisplay->update();
//			// redraw all views, update the GUI
//			TheDisplay->draw();
//		}
//		TheGlobalData->m_isBreakableMovie = FALSE;
//		TheGlobalData->m_breakTheMovie = FALSE;
//		GameWindow *win = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "ShellGameLoadScreen.wnd:EAGamesLogo" ) ));
//		if(win)
//			win->winHide(FALSE);

		GameWindow *win = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( AsciiString( "ShellGameLoadScreen.wnd:StaticTextLegal" ) ));
		if(win)
			win->winHide(FALSE);
		firstLoad = FALSE;

		UnsignedInt showTime = timeGetTime();
		while(showTime + 3000 > timeGetTime())
		{	
			LoadScreen::update(0);
			Sleep(100);
		}

	}
	m_progressBar->winHide(FALSE);	
}

void ShellGameLoadScreen::reset( void )
{
 setLoadScreen(NULL);
 m_progressBar = NULL;
}

void ShellGameLoadScreen::update( Int percent )
{
	TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);
	GadgetProgressBarSetProgress(m_progressBar, percent);

	// Do this last!
	LoadScreen::update( percent );
}

// MultiPlayerLoadScreen Class //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
MultiPlayerLoadScreen::MultiPlayerLoadScreen( void )
{
	//Added By Sadullah Nader
	//Initializations missing and needed
	m_mapPreview = NULL;

	//
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		//Added By Sadullah Nader
		//Initializations missing and needed
		m_buttonMapStartPosition[i] = NULL;
		//
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_playerSide[i]= NULL;
		m_playerLookup[i] = -1;
	}
}
	
MultiPlayerLoadScreen::~MultiPlayerLoadScreen( void )
{
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_playerSide[i]= NULL;
		m_playerLookup[i] = -1;
	}

	m_portraitLocalGeneral = NULL;
	m_featuresLocalGeneral = NULL;
	m_nameLocalGeneral = NULL;

	TheAudio->removeAudioEvent( AHSV_StopTheMusicFade );
//	TheAudio->stopAudio( AudioAffect_Music );
}

void MultiPlayerLoadScreen::init( GameInfo *game )
{
	// create the layout of the load screen
	m_loadScreen = TheWindowManager->winCreateFromScript( AsciiString( "Menus/MultiplayerLoadScreen.wnd" ) );
	DEBUG_ASSERTCRASH(m_loadScreen, ("Can't initialize the Multiplayer loadscreen"));
	m_loadScreen->winHide(FALSE);
	m_loadScreen->winBringToTop();
	m_mapPreview = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "MultiplayerLoadScreen.wnd:WinMapPreview"));
	GameSlot *lSlot = game->getSlot(game->getLocalSlotNum());
	const PlayerTemplate* pt;
	if (lSlot->getPlayerTemplate() >= 0)
		pt = ThePlayerTemplateStore->getNthPlayerTemplate(lSlot->getPlayerTemplate());
	else
		pt = ThePlayerTemplateStore->findPlayerTemplate( TheNameKeyGenerator->nameToKey("FactionObserver") );
//	const Image *loadScreenImage = TheMappedImageCollection->findImageByName(pt->getLoadScreen());

	// add portrait, features, and name for the local player's general
	const GeneralPersona *localGeneral = TheChallengeGenerals->getGeneralByTemplateName( pt->getName() );
	const Image *portrait = NULL;
	UnicodeString localName;
	if (localGeneral)
	{
		portrait = localGeneral->getBioPortraitLarge();
		localName = TheGameText->fetch( localGeneral->getBioName() );
	}
	else 
	{
		// the main original factions don't have associated generals
		AsciiString imageName;
		if (pt->getName() == "FactionAmerica")
			portrait = TheMappedImageCollection->findImageByName("SAFactionLogoLg_US");
		else if (pt->getName() == "FactionGLA")
			portrait = TheMappedImageCollection->findImageByName("SUFactionLogoLg_GLA");
		else if (pt->getName() == "FactionChina")
			portrait = TheMappedImageCollection->findImageByName("SNFactionLogoLg_China");
		else
			DEBUG_ASSERTCRASH(NULL, ("Unexpected player template"));

		localName = pt->getDisplayName();
	}
	m_portraitLocalGeneral = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "MultiplayerLoadScreen.wnd:LocalGeneralPortrait"));
	m_portraitLocalGeneral->winSetEnabledImage( 0, portrait);
	m_featuresLocalGeneral = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "MultiplayerLoadScreen.wnd:LocalGeneralFeatures"));
	AsciiString features = pt->getGeneralFeatures();
	GadgetStaticTextSetText( m_featuresLocalGeneral, TheGameText->fetch( features.isEmpty() ? AsciiString( "GUI:PlayerObserver" ) : pt->getGeneralFeatures() ) );
	m_nameLocalGeneral = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "MultiplayerLoadScreen.wnd:LocalGeneralName"));
	GadgetStaticTextSetText( m_nameLocalGeneral, localName );

	AsciiString musicName = pt->getLoadScreenMusic();
	if ( ! musicName.isEmpty() )
	{
		TheAudio->removeAudioEvent( AHSV_StopTheMusicFade );
		AudioEventRTS event( musicName );
		event.setShouldFade( TRUE );

		TheAudio->addAudioEvent( &event );
		TheAudio->update();//Since GameEngine::update() is suspended until after I am gone... 

	}


//	if(loadScreenImage)
//		m_loadScreen->winSetEnabledImage(0, loadScreenImage);
	//DEBUG_ASSERTCRASH(TheNetwork, ("Where the Heck is the Network!!!!"));
	//DEBUG_LOG(("NumPlayers %d\n", TheNetwork->getNumPlayers()));

	GameWindow *teamWin[MAX_SLOTS];
	Int i = 0;
	for (; i < MAX_SLOTS; ++i)
	{
		teamWin[i] = NULL;
	}

	Int netSlot = 0;
	// Loop through and make the loadscreen look all good.
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		// Load the Progress Bar
		AsciiString winName;
		winName.format( "MultiplayerLoadScreen.wnd:ProgressLoad%d",i);
		m_progressBars[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_progressBars[i], ("Can't initialize the progressbars for the Multiplayer loadscreen"));
		// set the progressbar to zero
		GadgetProgressBarSetProgress(m_progressBars[i], 0 );

		// Load MapStart Positions
		winName.format( "MultiplayerLoadScreen.wnd:ButtonMapStartPosition%d",i);
		m_buttonMapStartPosition[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_buttonMapStartPosition[i], ("Can't initialize the MapStart Positions for the MultiplayerLoadScreen loadscreen"));


		// Load the Player's name
		winName.format( "MultiplayerLoadScreen.wnd:StaticTextPlayer%d",i);
		m_playerNames[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerNames[i], ("Can't initialize the Names for the Multiplayer loadscreen"));
		
		// Load the Player's Side
		winName.format( "MultiplayerLoadScreen.wnd:StaticTextSide%d",i);
		m_playerSide[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerSide[i], ("Can't initialize the Sides for the Multiplayer loadscreen"));
		
		winName.format( "MultiplayerLoadScreen.wnd:StaticTextTeam%d",i);
		teamWin[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));

		// get the slot man!
		GameSlot *slot = game->getSlot(i);
		if (!slot || !slot->isOccupied())
			continue;		

		Color houseColor = TheMultiplayerSettings->getColor(slot->getApparentColor())->getColor();

		// format the progress bar to house colors
		AsciiString imageName;
		imageName.format("LoadingBar_ProgressCenter%d", slot->getApparentColor());
		const Image *houseImage = TheMappedImageCollection->findImageByName(imageName);
		if (! houseImage)
			houseImage = TheMappedImageCollection->findImageByName("LoadingBar_Progress");
		m_progressBars[netSlot]->winSetEnabledImage( 6, houseImage );

		UnicodeString name = slot->getName();
		GadgetStaticTextSetText(m_playerNames[netSlot], name );
		m_playerNames[netSlot]->winSetEnabledTextColors(houseColor, m_playerNames[netSlot]->winGetEnabledTextBorderColor());

		GadgetStaticTextSetText(m_playerSide[netSlot], slot->getApparentPlayerTemplateDisplayName() );
		m_playerSide[netSlot]->winSetEnabledTextColors(houseColor, m_playerSide[netSlot]->winGetEnabledTextBorderColor());

		if (slot->isAI() && m_progressBars[netSlot])
			m_progressBars[netSlot]->winHide(TRUE);

		if (teamWin[netSlot])
		{
			AsciiString teamStr;
			teamStr.format("Team:%d", slot->getTeamNumber() + 1);
			GadgetStaticTextSetText(teamWin[netSlot], TheGameText->fetch(teamStr));
			teamWin[netSlot]->winSetEnabledTextColors(houseColor, m_playerNames[netSlot]->winGetEnabledTextBorderColor());
		}

		m_playerLookup[i] = netSlot; // save our mapping so we can update progress correctly

		netSlot++;
	}
	
	for(i = netSlot; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i]->winHide(TRUE);
		m_playerNames[i]->winHide(TRUE);
		m_playerSide[i]->winHide(TRUE);
		teamWin[i]->winHide(TRUE);
	}

	if(m_mapPreview)
	{
		const MapMetaData *mmd = TheMapCache->findMap(game->getMap());
		Image *image = getMapPreviewImage(game->getMap());
		m_mapPreview->winSetUserData((void *)mmd);
		
		positionStartSpots( game, m_buttonMapStartPosition, m_mapPreview);
		updateMapStartSpots( game, m_buttonMapStartPosition, TRUE );
		//positionAdditionalImages((MapMetaData *)mmd, m_mapPreview, TRUE);
		if(image)
		{
			m_mapPreview->winSetStatus(WIN_STATUS_IMAGE);
			m_mapPreview->winSetEnabledImage(0, image);
		}
		else
		{
			m_mapPreview->winClearStatus(WIN_STATUS_IMAGE);
		}
	}

	
	TheGameLogic->initTimeOutValues();
}

void MultiPlayerLoadScreen::reset( void )
{
	setLoadScreen(NULL);
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_playerSide[i]= NULL;
	}
}

void MultiPlayerLoadScreen::update( Int percent )
{
	if (TheNetwork)
	{
		if(percent <= 100)
			TheNetwork->updateLoadProgress( percent );
		TheNetwork->liteupdate();
	}
	else
	{
		if (percent <= 100)
			TheGameLogic->processProgress( TheGameInfo->getLocalSlotNum(), percent );
	}

	//GadgetProgressBarSetProgress(m_progressBars[TheNetwork->getLocalPlayerID()], percent );	

	TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);

	// Do this last!
	LoadScreen::update( percent );
}

void MultiPlayerLoadScreen::processProgress(Int playerId, Int percentage)
{
	
	if( percentage < 0 || percentage > 100 || playerId >= MAX_SLOTS || playerId < 0 || m_playerLookup[playerId] == -1)
	{
		DEBUG_ASSERTCRASH(FALSE, ("Percentage %d was passed in for Player %d\n", percentage, playerId));
	}
	//DEBUG_LOG(("Percentage %d was passed in for Player %d (in loadscreen position %d)\n", percentage, playerId, m_playerLookup[playerId]));
	if(m_progressBars[m_playerLookup[playerId]])
		GadgetProgressBarSetProgress(m_progressBars[m_playerLookup[playerId]], percentage );	
}

// GameSpyLoadScreen Class //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
GameSpyLoadScreen::GameSpyLoadScreen( void )
{

	// Added By Sadullah Nader
	// Initializations missing and needed
	m_mapPreview = NULL;
	//

	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		
		// Added By Sadullah Nader
		// Initializations missing and needed
		m_buttonMapStartPosition[i] = NULL;
		m_playerRank[i] = NULL;
		//

		m_playerOfficerMedal[i] = NULL;
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_playerSide[i]= NULL;
		m_playerLookup[i] = -1;
		m_playerFavoriteFactions[i]= NULL;
		m_playerTotalDisconnects[i]= NULL;
		m_playerWin[i]= NULL;
		m_playerWinLosses[i]= NULL;		
	}
}
	
GameSpyLoadScreen::~GameSpyLoadScreen( void )
{
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_playerSide[i]= NULL;
		m_playerLookup[i] = -1;
		m_playerFavoriteFactions[i]= NULL;
		m_playerTotalDisconnects[i]= NULL;
		m_playerWin[i]= NULL;
		m_playerWinLosses[i]= NULL;		
	}
}

extern Int GetAdditionalDisconnectsFromUserFile(Int playerID);

void GameSpyLoadScreen::init( GameInfo *game )
{
	// create the layout of the load screen
	m_loadScreen = TheWindowManager->winCreateFromScript( AsciiString( "Menus/GameSpyLoadScreen.wnd" ) );
	DEBUG_ASSERTCRASH(m_loadScreen, ("Can't initialize the Multiplayer loadscreen"));
	m_loadScreen->winHide(FALSE);
	m_loadScreen->winBringToTop();
	m_mapPreview = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "GameSpyLoadScreen.wnd:WinMapPreview"));
	DEBUG_ASSERTCRASH(TheNetwork, ("Where the Heck is the Network!!!!"));
	DEBUG_LOG(("NumPlayers %d\n", TheNetwork->getNumPlayers()));
GameSlot *lSlot = game->getSlot(game->getLocalSlotNum());
	const PlayerTemplate* pt;
	if (lSlot->getPlayerTemplate() >= 0)
		pt = ThePlayerTemplateStore->getNthPlayerTemplate(lSlot->getPlayerTemplate());
	else
		pt = ThePlayerTemplateStore->findPlayerTemplate( TheNameKeyGenerator->nameToKey("FactionObserver") );
//	const Image *loadScreenImage = TheMappedImageCollection->findImageByName(pt->getLoadScreen());
//	if(loadScreenImage)
//		m_loadScreen->winSetEnabledImage(0, loadScreenImage);

	// add portrait, features, and name for the local player's general
	const GeneralPersona *localGeneral = TheChallengeGenerals->getGeneralByTemplateName( pt->getName() );
	const Image *portrait = NULL;
	UnicodeString localName;
	if (localGeneral)
	{
		portrait = localGeneral->getBioPortraitLarge();
		localName = TheGameText->fetch( localGeneral->getBioName() );
	}
	else 
	{
		// the main original factions don't have associated generals
		AsciiString imageName;
		if (pt->getName() == "FactionAmerica")
			portrait = TheMappedImageCollection->findImageByName("SAFactionLogo144_US");
		else if (pt->getName() == "FactionGLA")
			portrait = TheMappedImageCollection->findImageByName("SUFactionLogo144_GLA");
		else if (pt->getName() == "FactionChina")
			portrait = TheMappedImageCollection->findImageByName("SNFactionLogo144_China");
		else
			DEBUG_ASSERTCRASH(NULL, ("Unexpected player template"));

		localName = pt->getDisplayName();
	}
	m_portraitLocalGeneral = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "GameSpyLoadScreen.wnd:LocalGeneralPortrait"));
	m_portraitLocalGeneral->winSetEnabledImage( 0, portrait);
	m_featuresLocalGeneral = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "GameSpyLoadScreen.wnd:LocalGeneralFeatures"));
	AsciiString features = pt->getGeneralFeatures();
	GadgetStaticTextSetText( m_featuresLocalGeneral, TheGameText->fetch( features.isEmpty() ? AsciiString( "GUI:PlayerObserver" ) : pt->getGeneralFeatures() ) );
	m_nameLocalGeneral = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( "GameSpyLoadScreen.wnd:LocalGeneralName"));
	GadgetStaticTextSetText( m_nameLocalGeneral, localName );

	GameWindow *teamWin[MAX_SLOTS];
	Int i = 0;
	for (; i < MAX_SLOTS; ++i)
	{
		teamWin[i] = NULL;
	}

	Int netSlot = 0;
	// Loop through and make the loadscreen look all good.
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		// Load the Progress Bar
		AsciiString winName;
		winName.format( "GameSpyLoadScreen.wnd:ProgressLoad%d",i);
		m_progressBars[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_progressBars[i], ("Can't initialize the progressbars for the GameSpyLoadScreen loadscreen"));
		// set the progressbar to zero
		GadgetProgressBarSetProgress(m_progressBars[i], 0 );	

		// Load the Player's name
		winName.format( "GameSpyLoadScreen.wnd:StaticTextPlayer%d",i);
		m_playerNames[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerNames[i], ("Can't initialize the Names for the GameSpyLoadScreen loadscreen"));
		
		// Load MapStart Positions
		winName.format( "GameSpyLoadScreen.wnd:ButtonMapStartPosition%d",i);
		m_buttonMapStartPosition[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_buttonMapStartPosition[i], ("Can't initialize the MapStart Positions for the GameSpyLoadScreen loadscreen"));


		// Load the Player's Side
		winName.format( "GameSpyLoadScreen.wnd:StaticTextSide%d",i);
		m_playerSide[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerSide[i], ("Can't initialize the Sides for the GameSpyLoadScreen loadscreen"));
		
		// Load the Player's window
		winName.format( "GameSpyLoadScreen.wnd:WinPlayer%d",i);
		m_playerWin[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerWin[i], ("Can't initialize the WinPlayer for the GameSpyLoadScreen loadscreen"));
		
		// Load the Player's m_playerTotalDisconnects
		winName.format( "GameSpyLoadScreen.wnd:StaticTextTotalDisconnects%d",i);
		m_playerTotalDisconnects[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerTotalDisconnects[i], ("Can't initialize the m_playerTotalDisconnects for the GameSpyLoadScreen loadscreen"));

//		// Load the Player's m_playerFavoriteFactions
//		winName.format( "GameSpyLoadScreen.wnd:StaticTextFavoriteFaction%d",i);
//		m_playerFavoriteFactions[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
//		DEBUG_ASSERTCRASH(m_playerFavoriteFactions[i], ("Can't initialize the StaticTextFavoriteFaction for the GameSpyLoadScreen loadscreen"));

		// Load the Player's m_playerWinLosses
		winName.format( "GameSpyLoadScreen.wnd:StaticTextWinLoss%d",i);
		m_playerWinLosses[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerWinLosses[i], ("Can't initialize the m_playerWinLosses for the GameSpyLoadScreen loadscreen"));

		// Load the Player's m_playerWinLosses
		winName.format( "GameSpyLoadScreen.wnd:WinRank%d",i);
		m_playerRank[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerRank[i], ("Can't initialize the m_playerRank for the GameSpyLoadScreen loadscreen"));

		// Load the Player's m_playerOfficerMedal
		winName.format( "GameSpyLoadScreen.wnd:WinOfficer%d",i);
		m_playerOfficerMedal[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerOfficerMedal[i], ("Can't initialize the m_playerOfficerMedal for the GameSpyLoadScreen loadscreen"));

		winName.format( "MultiplayerLoadScreen.wnd:StaticTextTeam%d",i);
		teamWin[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));

		// get the slot man!
		GameSpyGameSlot *slot = (GameSpyGameSlot *)game->getSlot(i);
		if (!slot || !slot->isOccupied())
			continue;

		Color houseColor = TheMultiplayerSettings->getColor(slot->getApparentColor())->getColor();

		// format the progress bar to house colors
		AsciiString imageName;
		imageName.format("LoadingBar_ProgressCenter%d", slot->getApparentColor());
		const Image *houseImage = TheMappedImageCollection->findImageByName(imageName);
		if (! houseImage)
			houseImage = TheMappedImageCollection->findImageByName("LoadingBar_Progress");
		m_progressBars[netSlot]->winSetEnabledImage( 6, houseImage );

		UnicodeString name = slot->getName();
		GadgetStaticTextSetText(m_playerNames[netSlot], name );
		m_playerNames[netSlot]->winSetEnabledTextColors(houseColor, m_playerNames[netSlot]->winGetEnabledTextBorderColor());

		// Get the stats for the player
		PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(slot->getProfileID());
		DEBUG_LOG(("LoadScreen - populating info for %ls(%d) - stats returned id %d\n",
			slot->getName().str(), slot->getProfileID(), stats.id));

		Bool isPreorder = TheGameSpyInfo->didPlayerPreorder(stats.id);
		Int rankPoints = CalculateRank(stats);
		Int favSide = GetFavoriteSide(stats);
		const Image *preorderImg = TheMappedImageCollection->findImageByName("OfficersClubsmall");
		if (!isPreorder)
			preorderImg = NULL;
		const Image *rankImg = LookupSmallRankImage(favSide, rankPoints);
		m_playerOfficerMedal[i]->winSetEnabledImage(0, preorderImg);
		m_playerRank[i]->winSetEnabledImage(0, rankImg);

		UnicodeString formatString;
	
		// pop wins and losses
		Int numLosses = 0;
		PerGeneralMap::iterator it;
		for(it = stats.losses.begin(); it != stats.losses.end(); ++it)
		{
			numLosses += it->second;
		}
		Int numWins = 0;
		for(it =stats.wins.begin(); it != stats.wins.end(); ++it)
		{
			numWins += it->second;
		}
		formatString.format(L"%d/%d", numWins, numLosses);
		GadgetStaticTextSetText(m_playerWinLosses[netSlot], formatString);
		m_playerWinLosses[netSlot]->winSetEnabledTextColors(houseColor, m_playerWinLosses[netSlot]->winGetEnabledTextBorderColor());
		// favoriteFaction
			Int numGames = 0;
		Int favorite = 0;
		for(it =stats.games.begin(); it != stats.games.end(); ++it)
		{
			if(it->second >= numGames)
			{
				numGames = it->second;
				favorite = it->first;
			}
		}
//		if(numGames == 0)
//			GadgetStaticTextSetText(m_playerFavoriteFactions[netSlot], TheGameText->fetch("GUI:None"));	
//		else if( stats.gamesAsRandom > numGames )
//			GadgetStaticTextSetText(m_playerFavoriteFactions[netSlot], TheGameText->fetch("GUI:Random"));	
//		else
//		{		
//			const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(favorite);
//			if (fac)
//			{
//				AsciiString side;
//				side.format("SIDE:%s", fac->getSide().str());
//				
//				GadgetStaticTextSetText(m_playerFavoriteFactions[netSlot], TheGameText->fetch(side));
//			}
//		}
//		m_playerFavoriteFactions[netSlot]->winSetEnabledTextColors(houseColor, m_playerFavoriteFactions[netSlot]->winGetEnabledTextBorderColor());
		// disconnects
		numGames = 0;
		for(it =stats.discons.begin(); it != stats.discons.end(); ++it)
		{
			numGames += it->second;
		}
		for(it =stats.desyncs.begin(); it != stats.desyncs.end(); ++it)
		{
			numGames += it->second;
		}
		numGames += GetAdditionalDisconnectsFromUserFile(stats.id);

		formatString.format(L"%d", numGames);
		GadgetStaticTextSetText(m_playerTotalDisconnects[netSlot], formatString);
		m_playerTotalDisconnects[netSlot]->winSetEnabledTextColors(houseColor, m_playerTotalDisconnects[netSlot]->winGetEnabledTextBorderColor());
		GadgetStaticTextSetText(m_playerSide[netSlot], slot->getApparentPlayerTemplateDisplayName() );
		m_playerSide[netSlot]->winSetEnabledTextColors(houseColor, m_playerSide[netSlot]->winGetEnabledTextBorderColor());

		if (slot->isAI())
		{
			if (m_progressBars[netSlot])
				m_progressBars[netSlot]->winHide(TRUE);
			if (m_playerTotalDisconnects[netSlot])
				m_playerTotalDisconnects[netSlot]->winHide(TRUE);
//			if (m_playerFavoriteFactions[netSlot])
//				m_playerFavoriteFactions[netSlot]->winHide(TRUE);
			if (m_playerWinLosses[netSlot])
				m_playerWinLosses[netSlot]->winHide(TRUE);
			if (m_playerRank[netSlot])
				m_playerRank[netSlot]->winHide(TRUE);
			if (m_playerOfficerMedal[netSlot])
				m_playerOfficerMedal[netSlot]->winHide(TRUE);
		}

		if (teamWin[netSlot])
		{
			AsciiString teamStr;
			teamStr.format("Team:%d", slot->getTeamNumber() + 1);
			if (slot->isAI() && slot->getTeamNumber() == -1)
				teamStr = "Team:AI";
			GadgetStaticTextSetText(teamWin[netSlot], TheGameText->fetch(teamStr));
			teamWin[netSlot]->winSetEnabledTextColors(houseColor, m_playerNames[netSlot]->winGetEnabledTextBorderColor());
		}

		m_playerLookup[i] = netSlot; // save our mapping so we can update progress correctly

		netSlot++;
	}
	
	for(i = netSlot; i < MAX_SLOTS; ++i)
	{
		m_playerWin[i]->winHide(TRUE);
		//m_playerNames[i]->winHide(TRUE);
		//m_playerSide[i]->winHide(TRUE);
	}

	if(m_mapPreview)
	{
		const MapMetaData *mmd = TheMapCache->findMap(game->getMap());
		Image *image = getMapPreviewImage(game->getMap());
		m_mapPreview->winSetUserData((void *)mmd);
		
		positionStartSpots( game, m_buttonMapStartPosition, m_mapPreview);
		updateMapStartSpots( game, m_buttonMapStartPosition, TRUE );
		//positionAdditionalImages((MapMetaData *)mmd, m_mapPreview, TRUE);
		if(image)
		{
			m_mapPreview->winSetStatus(WIN_STATUS_IMAGE);
			m_mapPreview->winSetEnabledImage(0, image);
		}
		else
		{
			m_mapPreview->winClearStatus(WIN_STATUS_IMAGE);
		}
	}

	TheGameLogic->initTimeOutValues();
}

void GameSpyLoadScreen::reset( void )
{
	setLoadScreen(NULL);
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_playerSide[i]= NULL;
	}
}

void GameSpyLoadScreen::update( Int percent )
{
	if(percent <= 100)
		TheNetwork->updateLoadProgress( percent );
	TheNetwork->liteupdate();

	//GadgetProgressBarSetProgress(m_progressBars[TheNetwork->getLocalPlayerID()], percent );	

	TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);

	// Do this last!
	LoadScreen::update( percent );
}

void GameSpyLoadScreen::processProgress(Int playerId, Int percentage)
{
	
	if( percentage < 0 || percentage > 100 || playerId >= MAX_SLOTS || playerId < 0 || m_playerLookup[playerId] == -1)
	{
		DEBUG_ASSERTCRASH(FALSE, ("Percentage %d was passed in for Player %d\n", percentage, playerId));
	}
	//DEBUG_LOG(("Percentage %d was passed in for Player %d (in loadscreen position %d)\n", percentage, playerId, m_playerLookup[playerId]));
	if(m_progressBars[m_playerLookup[playerId]])
		GadgetProgressBarSetProgress(m_progressBars[m_playerLookup[playerId]], percentage );	
}

// MapTransferLoadScreen Class //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
MapTransferLoadScreen::MapTransferLoadScreen( void )
{
	//Added By Sadullah Nader
	//Initializations missing and needed
	m_oldTimeout = 0;
	//
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_progressText[i]= NULL;
		m_playerLookup[i] = -1;
		m_oldProgress[i] = -1;
	}
	m_fileNameText = NULL;
	m_timeoutText = NULL;
}
	
MapTransferLoadScreen::~MapTransferLoadScreen( void )
{
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_progressText[i]= NULL;
		m_playerLookup[i] = -1;
		m_oldProgress[i] = -1;
	}
	m_fileNameText = NULL;
	m_timeoutText = NULL;
}

void MapTransferLoadScreen::init( GameInfo *game )
{
	// create the layout of the load screen
	m_loadScreen = TheWindowManager->winCreateFromScript( AsciiString( "Menus/MapTransferScreen.wnd" ) );
	DEBUG_ASSERTCRASH(m_loadScreen, ("Can't initialize the map transfer loadscreen"));
	if (!m_loadScreen)
		return;

	m_loadScreen->winHide(FALSE);
	m_loadScreen->winBringToTop();

	DEBUG_ASSERTCRASH(TheNetwork, ("Where the Heck is the Network?!!!!"));
	DEBUG_LOG(("NumPlayers %d\n", TheNetwork->getNumPlayers()));

	AsciiString winName;
	Int i;

	// Load the Filename Text
	winName.format( "MapTransferScreen.wnd:StaticTextCurrentFile");
	m_fileNameText = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
	DEBUG_ASSERTCRASH(m_fileNameText, ("Can't initialize the filename for the map transfer loadscreen"));

	// Load the Timeout Text
	winName.format( "MapTransferScreen.wnd:StaticTextTimeout");
	m_timeoutText = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
	DEBUG_ASSERTCRASH(m_timeoutText, ("Can't initialize the timeout for the map transfer loadscreen"));

	Int netSlot = 0;
	// Loop through and make the loadscreen look all good.
	for (i = 0; i < MAX_SLOTS; ++i)
	{
		// Load the Progress Bar
		winName.format( "MapTransferScreen.wnd:ProgressLoad%d",i);
		m_progressBars[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_progressBars[i], ("Can't initialize the progressbars for the map transfer loadscreen"));
		// set the progressbar to zero
		GadgetProgressBarSetProgress(m_progressBars[i], 0 );

		// Load the Player's name
		winName.format( "MapTransferScreen.wnd:StaticTextPlayer%d",i);
		m_playerNames[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_playerNames[i], ("Can't initialize the Names for the map transfer loadscreen"));

		// Load the Progress Text
		winName.format( "MapTransferScreen.wnd:StaticTextProgress%d",i);
		m_progressText[i] = TheWindowManager->winGetWindowFromId( m_loadScreen,TheNameKeyGenerator->nameToKey( winName ));
		DEBUG_ASSERTCRASH(m_progressText[i], ("Can't initialize the progress text for the map transfer loadscreen"));

		// get the slot man!
		GameSlot *slot = game->getSlot(i);
		if (!slot || !slot->isHuman())
			continue;
		Color houseColor = TheMultiplayerSettings->getColor(slot->getApparentColor())->getColor();
		GadgetProgressBarSetEnabledBarColor(m_progressBars[netSlot], houseColor );

		UnicodeString name = slot->getName();
		GadgetStaticTextSetText(m_playerNames[netSlot], name );
		m_playerNames[netSlot]->winSetEnabledTextColors(houseColor, m_playerNames[netSlot]->winGetEnabledTextBorderColor());

		GadgetStaticTextSetText(m_progressText[netSlot], UnicodeString::TheEmptyString );
		m_progressText[netSlot]->winSetEnabledTextColors(houseColor, m_progressText[netSlot]->winGetEnabledTextBorderColor());

		if ((i == 0 || (TheGameInfo->getConstSlot(i)->isHuman() && TheGameInfo->getConstSlot(i)->hasMap())) && m_progressBars[netSlot])
			m_progressBars[netSlot]->winHide(TRUE);

		m_playerLookup[i] = netSlot; // save our mapping so we can update progress correctly

		netSlot++;
	}
	
	for(i = netSlot; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i]->winHide(TRUE);
		m_playerNames[i]->winHide(TRUE);
		m_progressText[i]->winHide(TRUE);
	}
}

void MapTransferLoadScreen::reset( void )
{
	setLoadScreen(NULL);
	for(Int i = 0; i < MAX_SLOTS; ++i)
	{
		m_progressBars[i] = NULL;
		m_playerNames[i] = NULL;
		m_progressText[i]= NULL;
		m_playerLookup[i] = -1;
		m_oldProgress[i] = -1;
	}
	m_fileNameText = NULL;
	m_timeoutText = NULL;
}

void MapTransferLoadScreen::update( Int percent )
{
	if (TheNetwork)
	{
		TheNetwork->liteupdate();
	}

	TheMouse->setCursorTooltip(UnicodeString::TheEmptyString);

	// Do this last!
	LoadScreen::update( percent );
}

void MapTransferLoadScreen::processProgress(Int playerId, Int percentage, AsciiString stateStr)
{
	
	if( percentage < 0 || percentage > 100 || playerId >= MAX_SLOTS || playerId < 0 || m_playerLookup[playerId] == -1)
	{
		DEBUG_ASSERTCRASH(FALSE, ("Percentage %d was passed in for Player %d\n", percentage, playerId));
	}

	if (m_oldProgress[playerId] == percentage)
		return;
	m_oldProgress[playerId] = percentage;

	Int translatedSlot = m_playerLookup[playerId];
	if(m_progressBars[translatedSlot])
		GadgetProgressBarSetProgress(m_progressBars[translatedSlot], percentage );	
	if (m_progressText[translatedSlot])
		GadgetStaticTextSetText(m_progressText[translatedSlot], TheGameText->fetch(stateStr));
}

void MapTransferLoadScreen::processTimeout(Int secondsLeft)
{
	if (m_oldTimeout == secondsLeft)
		return;
	m_oldTimeout = secondsLeft;

	if (m_timeoutText)
	{
		UnicodeString txt;
		txt.format(TheGameText->fetch("MapTransfer:Timeout"), (secondsLeft/60), (secondsLeft%60));
		GadgetStaticTextSetText(m_timeoutText, txt);
	}
}

void MapTransferLoadScreen::setCurrentFilename(AsciiString filename)
{
	if (m_fileNameText)
	{
		UnicodeString txt;
		txt.translate(TheGameState->getMapLeafName(filename));
		txt.format(TheGameText->fetch("MapTransfer:CurrentFile"), txt.str());
		GadgetStaticTextSetText(m_fileNameText, txt);
	}
}

