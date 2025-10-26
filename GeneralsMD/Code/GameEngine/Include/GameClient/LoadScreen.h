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

// FILE: LoadScreen.h /////////////////////////////////////////////////////////////////////////////////
// Author: Chris Huybregts, March 2002
// Desc:   The file will hold the LoadScreen Base class and it's derived classes
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Common/SubsystemInterface.h"
#include "GameClient/GameWindow.h"
#include "GameNetwork/GameInfo.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/ChallengeGenerals.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////
class VideoBuffer;
class VideoStreamInterface;
class WindowVideoManager;



///////////////////////////////////////////////////////////////////////////////////////////////////
// Class LoadScreen is the parent class for each other kind of load screen
///////////////////////////////////////////////////////////////////////////////////////////////////
class LoadScreen
{
public:
	LoadScreen( void );
	virtual ~LoadScreen( void );

	virtual void init( GameInfo *game ) = 0;		///< Init the loadscreen
	virtual void reset( void ) = 0;		///< Reset the system
	virtual void update( void ) = 0;  ///< Update the state of the slider bars
	virtual void update( Int percent ); ///< Update the state of the slider bars
	virtual void processProgress(Int playerId, Int percentage) = 0;
	virtual void setProgressRange( Int min, Int max ) = 0;
protected:
	void setLoadScreen( GameWindow *g ) { m_loadScreen = g; }
	GameWindow *m_loadScreen;		///< The GameWindow that is our loadscreen

private:

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// class SinglePlayerLoadScreen is to be used only when we're loading a single player mission
///////////////////////////////////////////////////////////////////////////////////////////////////
class SinglePlayerLoadScreen : public LoadScreen
{
public:
	SinglePlayerLoadScreen( void );
	virtual ~SinglePlayerLoadScreen( void );

	virtual void init( GameInfo *game );		///< Init the loadscreen
	virtual void reset( void );		///< Reset the system
	virtual void update( void )
	{
		DEBUG_CRASH(("Call update(Int) instead.  This update isn't supported"));
	};
	virtual void update(Int percent);		 ///< Update the state of the progress bar
	virtual void processProgress(Int playerId, Int percentage)
	{
		DEBUG_CRASH(("We Got to a single player load screen throw the Network..."));
	}

	virtual void setProgressRange( Int min, Int max );

private:
	GameWindow *m_progressBar;				///< Pointer to the Progress Bar on the window
	GameWindow *m_percent;

	GameWindow *m_objectiveWin;
	GameWindow *m_objectiveLines[MAX_OBJECTIVE_LINES];
	GameWindow *m_unitDesc[MAX_DISPLAYED_UNITS];
	GameWindow *m_location;

	Int m_currentObjectiveLine;
	Int m_currentObjectiveLineCharacter;
	Int m_currentObjectiveWidthOffset;
	Bool m_finishedObjectiveText;

	UnicodeString m_unicodeObjectiveLines[MAX_OBJECTIVE_LINES];

	VideoBuffer *m_videoBuffer;
	VideoStreamInterface *m_videoStream;

	void moveWindows( Int frame );

	AudioEventRTS m_ambientLoop;
	AudioHandle m_ambientLoopHandle;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// class ChallengeLoadScreen is to be used only when we're loading a Generals' Challenge mission
///////////////////////////////////////////////////////////////////////////////////////////////////
class ChallengeLoadScreen : public LoadScreen
{
public:
	ChallengeLoadScreen( void );
	virtual ~ChallengeLoadScreen( void );

	virtual void init( GameInfo *game );		///< Init the loadscreen
	virtual void reset( void );		///< Reset the system
	virtual void update( void )
	{
		DEBUG_CRASH(("Call update(Int) instead.  This update isn't supported"));
	};
	virtual void update(Int percent);		 ///< Update the state of the progress bar
	virtual void processProgress(Int playerId, Int percentage)
	{
		DEBUG_CRASH(("We Got to a single player load screen throw the Network..."));
	}

	virtual void setProgressRange( Int min, Int max );

private:
	GameWindow *m_progressBar;				///< Pointer to the Progress Bar on the window

	VideoBuffer *m_videoBuffer;
	VideoStreamInterface *m_videoStream;

	WindowVideoManager *m_wndVideoManager;

	AudioEventRTS m_ambientLoop;
	AudioHandle m_ambientLoopHandle;

	GameWindow *m_bioNameLeft;
	GameWindow *m_bioAgeLeft;
	GameWindow *m_bioBirthplaceLeft;
	GameWindow *m_bioStrategyLeft;
	GameWindow *m_bioBigNameEntryLeft;
	GameWindow *m_bioNameEntryLeft;
	GameWindow *m_bioAgeEntryLeft;
	GameWindow *m_bioBirthplaceEntryLeft;
	GameWindow *m_bioStrategyEntryLeft;
	GameWindow *m_bioNameRight;
	GameWindow *m_bioAgeRight;
	GameWindow *m_bioBirthplaceRight;
	GameWindow *m_bioStrategyRight;
	GameWindow *m_bioBigNameEntryRight;
	GameWindow *m_bioNameEntryRight;
	GameWindow *m_bioAgeEntryRight;
	GameWindow *m_bioBirthplaceEntryRight;
	GameWindow *m_bioStrategyEntryRight;

	GameWindow *m_portraitLeft;
	GameWindow *m_portraitRight;
	GameWindow *m_portraitMovieLeft;
	GameWindow *m_portraitMovieRight;

//	GameWindow *m_overlayReticleCrosshairs;
	GameWindow *m_overlayReticleCircleLineOuter;
	GameWindow *m_overlayReticleCircleLineInner;
	GameWindow *m_overlayReticleCircleAlphaOuter;
	GameWindow *m_overlayReticleCircleAlphaInner;
	GameWindow *m_overlayVsBackdrop;
	GameWindow *m_overlayVs;

	void activatePieces( Int frame, const GeneralPersona *generalPlayer, const GeneralPersona *generalOpponent );
	void activatePiecesMinSpec(const GeneralPersona *generalPlayer, const GeneralPersona *generalOpponent);
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// class ShellGameLoadScreen is to be used for the Shell Game loadscreen
////	///////////////////////////////////////////////////////////////////////////////////////////////
class ShellGameLoadScreen : public LoadScreen
{
public:
	ShellGameLoadScreen( void );
	virtual ~ShellGameLoadScreen( void );

	virtual void init( GameInfo *game );		///< Init the loadscreen
	virtual void reset( void );		///< Reset the system
	virtual void update( void )
	{
		DEBUG_CRASH(("Call update(Int) instead.  This update isn't supported"));
	};
	virtual void update(Int percent);		 ///< Update the state of the progress bar
	virtual void processProgress(Int playerId, Int percentage)
	{
		DEBUG_CRASH(("We Got to a single player load screen throw the Network..."));
	}
	virtual void setProgressRange( Int min, Int max ) { }

private:
	GameWindow *m_progressBar	;				///< Pointer to the Progress Bar on the window

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// class MultiPlayerLoadScreen is to be used for multiplayer communication on the loadscreens
////	///////////////////////////////////////////////////////////////////////////////////////////////
class MultiPlayerLoadScreen : public LoadScreen
{
public:
	MultiPlayerLoadScreen( void );
	virtual ~MultiPlayerLoadScreen( void );

	virtual void init( GameInfo *game );		///< Init the loadscreen
	virtual void reset( void );		///< Reset the system
	virtual void update( void )
	{
		DEBUG_CRASH(("Call update(Int) instead.  This update isn't supported"));
	};
	virtual void update(Int percent);		 ///< Update the state of the progress bar
	void processProgress(Int playerId, Int percentage);
	virtual void setProgressRange( Int min, Int max ) { }
private:
	GameWindow *m_progressBars[MAX_SLOTS];	///< pointer array to all the progress bars on the window
	GameWindow *m_playerNames[MAX_SLOTS];		///< pointer array to all the static text player names on the window
	GameWindow *m_playerSide[MAX_SLOTS];		///< pointer array to all the static text player sides
	Int m_playerLookup[MAX_SLOTS];					///< lookup table to translate network slot info screen slot (to account for holes in the slot list)
	GameWindow *m_mapPreview;
	GameWindow *m_buttonMapStartPosition[MAX_SLOTS];
	GameWindow *m_portraitLocalGeneral;
	GameWindow *m_featuresLocalGeneral;
	GameWindow *m_nameLocalGeneral;

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// class MultiPlayerLoadScreen is to be used for multiplayer communication on the loadscreens
////	///////////////////////////////////////////////////////////////////////////////////////////////
class GameSpyLoadScreen : public LoadScreen
{
public:
	GameSpyLoadScreen( void );
	virtual ~GameSpyLoadScreen( void );

	virtual void init( GameInfo *game );		///< Init the loadscreen
	virtual void reset( void );		///< Reset the system
	virtual void update( void )
	{
		DEBUG_CRASH(("Call update(Int) instead.  This update isn't supported"));
	};
	virtual void update(Int percent);		 ///< Update the state of the progress bar
	void processProgress(Int playerId, Int percentage);
	virtual void setProgressRange( Int min, Int max ) { }
private:
	GameWindow *m_progressBars[MAX_SLOTS];	///< pointer array to all the progress bars on the window
	GameWindow *m_playerNames[MAX_SLOTS];		///< pointer array to all the static text player names on the window
	GameWindow *m_playerSide[MAX_SLOTS];		///< pointer array to all the static text player sides
	GameWindow *m_playerFavoriteFactions[MAX_SLOTS];		///< pointer array to all the static text player sides
	GameWindow *m_playerTotalDisconnects[MAX_SLOTS];		///< pointer array to all the static text player sides
	GameWindow *m_playerWin[MAX_SLOTS];		///< pointer array to all the static text player sides
	GameWindow *m_playerWinLosses[MAX_SLOTS];		///< pointer array to all the static text player sides
	GameWindow *m_playerRank[MAX_SLOTS];		///< pointer array to all the static text player sides
	GameWindow *m_playerOfficerMedal[MAX_SLOTS];		///< pointer array to all the static text player munkees
	GameWindow *m_mapPreview;
	GameWindow *m_buttonMapStartPosition[MAX_SLOTS];

	Int m_playerLookup[MAX_SLOTS];					///< lookup table to translate network slot info screen slot (to account for holes in the slot list)

	GameWindow *m_portraitLocalGeneral;
	GameWindow *m_featuresLocalGeneral;
	GameWindow *m_nameLocalGeneral;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// class MapTransferLoadScreen is to be used for map transfers before multiplayer game load screens
////	///////////////////////////////////////////////////////////////////////////////////////////////
class MapTransferLoadScreen : public LoadScreen
{
public:
	MapTransferLoadScreen( void );
	virtual ~MapTransferLoadScreen( void );

	virtual void init( GameInfo *game );		///< Init the loadscreen
	virtual void reset( void );							///< Reset the system
	virtual void update( void )
	{
		DEBUG_CRASH(("Call update(Int) instead.  This update isn't supported"));
	};
	virtual void update(Int percent);				///< Update the state of the progress bar
	virtual void processProgress(Int playerId, Int percentage)
	{
		DEBUG_CRASH(("Call processProgress(Int, Int, AsciiString) instead."));
	}
	void processProgress(Int playerId, Int percentage, AsciiString stateStr);
	virtual void setProgressRange( Int min, Int max ) { }
	void processTimeout(Int secondsLeft);
	void setCurrentFilename(AsciiString filename);
private:
	GameWindow *m_progressBars[MAX_SLOTS];	///< pointer array to all the progress bars on the window
	GameWindow *m_playerNames[MAX_SLOTS];		///< pointer array to all the static text player names on the window
	GameWindow *m_progressText[MAX_SLOTS];	///< pointer array to all the static text player sides
	Int m_playerLookup[MAX_SLOTS];					///< lookup table to translate network slot info screen slot (to account for holes in the slot list)
	Int m_oldProgress[MAX_SLOTS];						///< old vals, so we can call processProgress() every frame and not touch the GUI
	GameWindow *m_fileNameText;
	GameWindow *m_timeoutText;
	Int m_oldTimeout;												///< old val, so we can call processTimeout() every frame and not touch the GUI
};
