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

// FILE: GameSpyPersistentStorage.h //////////////////////////////////////////////////////
// Generals GameSpy Persistent Storage
// Author: Matthew D. Campbell, March 2002

#pragma once

class GameSpyPlayerInfoInterface : public SubsystemInterface
{
public:
	virtual ~GameSpyPlayerInfoInterface() { };

	virtual void init( void ) = 0;
	virtual void reset( void ) = 0;
	virtual void update( void ) = 0;

	virtual void setLocale( AsciiString locale, Bool setOnServer = true ) = 0;
	virtual AsciiString getLocale( void ) = 0;
	virtual void setWins( Int wins, Bool setOnServer = true ) = 0;
	virtual Int getWins( void ) = 0;
	virtual void setLosses( Int losses, Bool setOnServer = true ) = 0;
	virtual Int getLosses( void ) = 0;

	virtual void readFromServer( void ) = 0;

	virtual void threadReadFromServer( void ) = 0;
	virtual void threadSetLocale( AsciiString val ) = 0;
	virtual void threadSetWins  ( AsciiString val ) = 0;
	virtual void threadSetLosses( AsciiString val ) = 0;
};

GameSpyPlayerInfoInterface *createGameSpyPlayerInfo( void );

extern GameSpyPlayerInfoInterface *TheGameSpyPlayerInfo;
