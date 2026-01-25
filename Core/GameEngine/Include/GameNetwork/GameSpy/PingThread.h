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

// FILE: PingThread.h //////////////////////////////////////////////////////
// Generals ping thread class interface
// Author: Matthew D. Campbell, August 2002
// Note: adapted from WOLAPI

#pragma once

// this class encapsulates a request for the thread
class PingRequest
{
public:
	std::string hostname;
	Int repetitions;
	Int timeout;
};

//-------------------------------------------------------------------------

// this class encapsulates a response from the thread
class PingResponse
{
public:
	std::string hostname;
	Int avgPing;
	Int repetitions;
};

//-------------------------------------------------------------------------

// this is the actual message queue used to pass messages between threads
class PingerInterface
{
public:
	virtual ~PingerInterface() {}
	virtual void startThreads( void ) = 0;
	virtual void endThreads( void ) = 0;
	virtual Bool areThreadsRunning( void ) = 0;

	virtual void addRequest( const PingRequest& req ) = 0;
	virtual Bool getRequest( PingRequest& resp ) = 0;

	virtual void addResponse( const PingResponse& resp ) = 0;
	virtual Bool getResponse( PingResponse& resp ) = 0;

	static PingerInterface* createNewPingerInterface( void );

	virtual Bool arePingsInProgress( void ) = 0;
	virtual Int getPing( AsciiString hostname ) = 0;
	virtual void clearPingMap( void ) = 0;
	virtual AsciiString getPingString( Int timeout ) = 0;
};

extern PingerInterface *ThePinger;
