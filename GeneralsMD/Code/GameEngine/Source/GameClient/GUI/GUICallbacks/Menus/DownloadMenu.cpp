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

// FILE: DownloadMenu.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: DownloadMenu.cpp
//
// Created:   Matthew D. Campbell, July 2002
//
// Desc:      the Patch Download window control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetProgressBar.h"
#include "GameClient/GameText.h"
#include "GameClient/MessageBox.h"

#include "GameLogic/GameLogic.h"

#include "GameNetwork/DownloadManager.h"
#include "GameNetwork/GameSpy/MainMenuUtils.h"

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static NameKeyType buttonCancelID = NAMEKEY_INVALID;
static NameKeyType staticTextSizeID = NAMEKEY_INVALID;
static NameKeyType staticTextTimeID = NAMEKEY_INVALID;
static NameKeyType staticTextFileID = NAMEKEY_INVALID;
static NameKeyType staticTextStatusID = NAMEKEY_INVALID;
static NameKeyType progressBarMunkeeID = NAMEKEY_INVALID;

static GameWindow * staticTextSize = NULL;
static GameWindow * staticTextTime = NULL;
static GameWindow * staticTextFile = NULL;
static GameWindow * staticTextStatus = NULL;
static GameWindow * progressBarMunkee = NULL;

static GameWindow *parent = NULL;

static void closeDownloadWindow( void )
{
	DEBUG_ASSERTCRASH(parent, ("No Parent"));
	if (!parent)
		return;

	WindowLayout *menuLayout = parent->winGetLayout();
	if (menuLayout)
	{
		menuLayout->runShutdown();
		menuLayout->destroyWindows();
		deleteInstance(menuLayout);
		menuLayout = NULL;
	}

	GameWindow *mainWin = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("MainMenu.wnd:MainMenuParent") );
	if (mainWin)
		TheWindowManager->winSetFocus( mainWin );
}

static void errorCallback( void )
{
	HandleCanceledDownload();
	closeDownloadWindow();
}

static void successQuitCallback( void )
{
	TheGameEngine->setQuitting( TRUE );
	closeDownloadWindow();

	// Clean up game data.  No crashy-crash for you!
	if (TheGameLogic->isInGame())
		TheGameLogic->exitGame();
}

static void successNoQuitCallback( void )
{
	HandleCanceledDownload();
	closeDownloadWindow();
}

class DownloadManagerMunkee : public DownloadManager
{
public:
	DownloadManagerMunkee() {m_shouldQuitOnSuccess = true; m_shouldQuitOnSuccess = false;}
	virtual HRESULT OnError( Int error );
	virtual HRESULT OnEnd();
	virtual HRESULT OnProgressUpdate( Int bytesread, Int totalsize, Int timetaken, Int timeleft );
	virtual HRESULT OnStatusUpdate( Int status );
	virtual HRESULT downloadFile( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume );

private:
	Bool m_shouldQuitOnSuccess;
};

HRESULT DownloadManagerMunkee::downloadFile( AsciiString server, AsciiString username, AsciiString password, AsciiString file, AsciiString localfile, AsciiString regkey, Bool tryResume )
{
	// see if we'll need to restart
	if (strstr(localfile.str(), "patches\\") != NULL)
	{
		m_shouldQuitOnSuccess = true;
	}

	if (staticTextFile)
	{
		AsciiString bob = file;

		// just get the filename, not the pathname
		const char *tmp = bob.reverseFind('/');
		if (tmp)
			bob = tmp+1;
		tmp = bob.reverseFind('\\');
		if (tmp)
			bob = tmp+1;

		UnicodeString fileString;
		fileString.translate(bob);
		GadgetStaticTextSetText(staticTextFile, fileString);
	}

	password.format("-%s", password.str());
	return DownloadManager::downloadFile( server, username, password, file, localfile, regkey, tryResume );
}
HRESULT DownloadManagerMunkee::OnError( Int error )
{
	HRESULT ret = DownloadManager::OnError( error );

	MessageBoxOk(TheGameText->fetch("GUI:DownloadErrorTitle"), getErrorString(), errorCallback);
	return ret;
}
HRESULT DownloadManagerMunkee::OnEnd()
{
	HRESULT ret = DownloadManager::OnEnd();

	if (isFileQueuedForDownload())
	{
		return downloadNextQueuedFile();
	}
	if (m_shouldQuitOnSuccess)
		MessageBoxOk(TheGameText->fetch("GUI:DownloadSuccessTitle"), TheGameText->fetch("GUI:DownloadSuccessMustQuit"), successQuitCallback);
	else
		MessageBoxOk(TheGameText->fetch("GUI:DownloadSuccessTitle"), TheGameText->fetch("GUI:DownloadSuccess"), successNoQuitCallback);
	return ret;
}

static time_t lastUpdate = 0;
static Int timeLeft = 0;
HRESULT DownloadManagerMunkee::OnProgressUpdate( Int bytesread, Int totalsize, Int timetaken, Int timeleft )
{
	HRESULT ret = DownloadManager::OnProgressUpdate( bytesread, totalsize, timetaken, timeleft );

	if (progressBarMunkee)
	{
		Int percent = bytesread * 100 / totalsize;
		GadgetProgressBarSetProgress( progressBarMunkee, percent );
	}

	if (staticTextSize)
	{
		UnicodeString sizeString;
		sizeString.format(TheGameText->fetch("GUI:DownloadBytesRatio"), bytesread, totalsize);
		GadgetStaticTextSetText(staticTextSize, sizeString);
	}
	timeLeft = timeleft;
	if (staticTextTime && GadgetStaticTextGetText(staticTextTime).isEmpty()) // only update immediately the first time
	{
		lastUpdate = time(NULL);
		UnicodeString timeString;
		if (timeleft)
		{
			DEBUG_ASSERTCRASH(timeleft > 0, ("Time left is negative!"));
			timeleft = max(1, timeleft);
			Int takenHour, takenMin, takenSec;
			takenHour = timeleft / 60 / 60;
			takenMin = timeleft / 60;
			takenSec = timeleft % 60;
			timeString.format(TheGameText->fetch("GUI:DownloadTimeLeft"), takenHour, takenMin, takenSec);
		}
		else
		{
			timeString = TheGameText->fetch("GUI:DownloadUnknownTime");
		}
		GadgetStaticTextSetText(staticTextTime, timeString);
	}
	return ret;
}

HRESULT DownloadManagerMunkee::OnStatusUpdate( Int status )
{
	HRESULT ret = DownloadManager::OnStatusUpdate( status );

	if (staticTextStatus)
	{
		GadgetStaticTextSetText(staticTextStatus, getStatusString());
	}
	return ret;
}

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Initialize the menu */
//-------------------------------------------------------------------------------------------------
void DownloadMenuInit( WindowLayout *layout, void *userData )
{

	//set keyboard focus to main parent and set modal
	NameKeyType parentID = TheNameKeyGenerator->nameToKey("DownloadMenu.wnd:ParentDownload");
	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );

  // get ids for our children controls
	buttonCancelID = TheNameKeyGenerator->nameToKey( "DownloadMenu.wnd:ButtonCancel" );
	staticTextSizeID = TheNameKeyGenerator->nameToKey( "DownloadMenu.wnd:StaticTextSize" );
	staticTextTimeID = TheNameKeyGenerator->nameToKey( "DownloadMenu.wnd:StaticTextTime" );
	staticTextFileID = TheNameKeyGenerator->nameToKey( "DownloadMenu.wnd:StaticTextFile" );
	staticTextStatusID = TheNameKeyGenerator->nameToKey( "DownloadMenu.wnd:StaticTextStatus" );
	progressBarMunkeeID = TheNameKeyGenerator->nameToKey( "DownloadMenu.wnd:ProgressBarMunkee" );

	staticTextSize = TheWindowManager->winGetWindowFromId( parent, staticTextSizeID );
	staticTextTime = TheWindowManager->winGetWindowFromId( parent, staticTextTimeID );
	staticTextFile = TheWindowManager->winGetWindowFromId( parent, staticTextFileID );
	staticTextStatus = TheWindowManager->winGetWindowFromId( parent, staticTextStatusID );
	progressBarMunkee = TheWindowManager->winGetWindowFromId( parent, progressBarMunkeeID );

	DEBUG_ASSERTCRASH(!TheDownloadManager, ("Download manager already exists"));
	if (TheDownloadManager)
	{
		delete TheDownloadManager;
	}
	TheDownloadManager = NEW DownloadManagerMunkee;

}

//-------------------------------------------------------------------------------------------------
/** menu shutdown method */
//-------------------------------------------------------------------------------------------------
void DownloadMenuShutdown( WindowLayout *layout, void *userData )
{
	DEBUG_ASSERTCRASH(TheDownloadManager, ("No download manager"));
	if (TheDownloadManager)
	{
		delete TheDownloadManager;
		TheDownloadManager = NULL;
	}

	staticTextSize = NULL;
	staticTextTime = NULL;
	staticTextFile = NULL;
	staticTextStatus = NULL;
	progressBarMunkee = NULL;
	parent = NULL;

}

//-------------------------------------------------------------------------------------------------
/** menu update method */
//-------------------------------------------------------------------------------------------------
void DownloadMenuUpdate( WindowLayout *layout, void *userData )
{
	if (staticTextTime && !GadgetStaticTextGetText(staticTextTime).isEmpty())
	{
		time_t now = time(NULL);
		if (now <= lastUpdate)
			return;

		lastUpdate = now;

		UnicodeString timeString;
		if (timeLeft)
		{
			DEBUG_ASSERTCRASH(timeLeft > 0, ("Time left is negative!"));
			timeLeft = max(1, timeLeft);
			Int takenHour, takenMin, takenSec;
			takenHour = timeLeft / 60 / 60;
			takenMin = timeLeft / 60;
			takenSec = timeLeft % 60;
			timeString.format(TheGameText->fetch("GUI:DownloadTimeLeft"), takenHour, takenMin, takenSec);
		}
		else
		{
			timeString = TheGameText->fetch("GUI:DownloadUnknownTime");
		}
		GadgetStaticTextSetText(staticTextTime, timeString);
	}

}

//-------------------------------------------------------------------------------------------------
/** menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DownloadMenuInput( GameWindow *window, UnsignedInt msg,
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
						AsciiString buttonName( "DownloadMenu.wnd:ButtonCancel" );
						NameKeyType buttonID = TheNameKeyGenerator->nameToKey( buttonName );
						GameWindow *button = TheWindowManager->winGetWindowFromId( window, buttonID );

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																								(WindowMsgData)button, buttonID );

					}

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}

			}

		}

	}

	return MSG_IGNORED;

}

//-------------------------------------------------------------------------------------------------
/** menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DownloadMenuSystem( GameWindow *window, UnsignedInt msg,
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

    //----------------------------------------------------------------------------------------------
    case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			break;

		}
    //---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			if( controlID == buttonCancelID )
			{
				HandleCanceledDownload();
				closeDownloadWindow();
			}

			break;

		}

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}
