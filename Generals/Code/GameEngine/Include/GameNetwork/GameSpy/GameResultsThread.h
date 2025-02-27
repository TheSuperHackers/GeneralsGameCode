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

// FILE: GameResultsThread.h //////////////////////////////////////////////////////
// Generals game results thread class interface
// Author: Matthew D. Campbell, August 2002

#pragma once

#include "Common/SubsystemInterface.h"

// this class encapsulates a request for the thread
class GameResultsRequest
{
public:
	std::string hostname;
	UnsignedShort port;
	std::string results;
};

//-------------------------------------------------------------------------

// this class encapsulates a response from the thread
class GameResultsResponse
{
public:
	std::string hostname;
	UnsignedShort port;
	Bool sentOk;
};

//-------------------------------------------------------------------------

// this is the actual message queue used to pass messages between threads
class GameResultsInterface : public SubsystemInterface
{
public:
	virtual ~GameResultsInterface() {}
	virtual void startThreads( void ) = 0;
	virtual void endThreads( void ) = 0;
	virtual Bool areThreadsRunning( void ) = 0;

	virtual void addRequest( const GameResultsRequest& req ) = 0;
	virtual Bool getRequest( GameResultsRequest& resp ) = 0;

	virtual void addResponse( const GameResultsResponse& resp ) = 0;
	virtual Bool getResponse( GameResultsResponse& resp ) = 0;

	static GameResultsInterface* createNewGameResultsInterface( void );

	virtual Bool areGameResultsBeingSent( void ) = 0;
};

extern GameResultsInterface *TheGameResultsQueue;
