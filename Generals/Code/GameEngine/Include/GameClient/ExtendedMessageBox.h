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

// FILE: ExtendedMessageBox.h /////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, January 2003
// Description: We go quiet in 1 day, gold in 15.  Poor time to rewrite message boxes, so
//              we get this file instead.  Phooey.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __EXTENDEDMESSAGEBOX_H_
#define __EXTENDEDMESSAGEBOX_H_

#include "GameClient/GameWindowManager.h"

// return codes for message box callbacks
enum MessageBoxReturnType CPP_11(: Int) {
	MB_RETURN_CLOSE,
	MB_RETURN_KEEPOPEN
};

typedef MessageBoxReturnType (* MessageBoxFunc)( void *userData );

// WindowExMessageBoxData ---------------------------------------------------------
/** Data attached to each extended Message box window */
//-----------------------------------------------------------------------------
struct WindowExMessageBoxData
{
	MessageBoxFunc yesCallback;		///<Function pointer to the Yes Button Callback
  MessageBoxFunc noCallback;			///<Function pointer to the No Button Callback
  MessageBoxFunc okCallback;			///<Function pointer to the Ok Button Callback
  MessageBoxFunc cancelCallback;	///<Function pointer to the Cancel Button Callback
	void *userData;
};


GameWindow *ExMessageBoxYesNo				(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc yesCallback, MessageBoxFunc noCallback);

GameWindow *ExMessageBoxYesNoCancel	(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc yesCallback, MessageBoxFunc noCallback, MessageBoxFunc cancelCallback);

GameWindow *ExMessageBoxOkCancel		(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc okCallback, MessageBoxFunc cancelCallback);

GameWindow *ExMessageBoxOk					(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc okCallback);

GameWindow *ExMessageBoxCancel			(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc cancelCallback);

#endif //__EXTENDEDMESSAGEBOX_H_
