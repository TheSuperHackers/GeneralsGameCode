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

#pragma once

#ifdef _WIN32
#include <process.h>
#endif
#include <configfile.h>
#include <critsec.h>
#include <threadfac.h>
#include <tcp.h>
#include <cstdarg>
#include <sem4.h>

#include <string>

#include <peer/peer.h>

class MatcherClass
{
public:
	MatcherClass();
	virtual ~MatcherClass()
	{}

	virtual void init(void)
	{}

	virtual void checkMatches(void)
	{}

	virtual void handleDisconnect( const char *reason )
	{}
	virtual void handleRoomMessage( const char *nick, const char *message, MessageType messageType )
	{}
	virtual void handlePlayerMessage( const char *nick, const char *message, MessageType messageType )
	{}
	virtual void handlePlayerJoined( const char *nick )
	{}
	virtual void handlePlayerLeft( const char *nick )
	{}
	virtual void handlePlayerChangedNick( const char *oldNick, const char *newNick )
	{}
	virtual void handlePlayerEnum( bool success, int gameSpyIndex, const char *nick, int flags )
	{}

	void handleConnect( bool success );
	void handleGroupRoomList( bool success, int groupID, const char *name );
	void handleJoin( bool success );
	void handleNickError( const char *badNick );

	void connectAndLoop( void );

protected:

	Wstring getString(const Wstring& key);

	Wstring m_baseNick;
	std::string m_nick;
	int m_profileID;
	PEER m_peer;
	bool m_connectSuccess;
	bool m_joinSuccess;
	void readLoop( void );

	int done;  // 0=no, neg=quit;error, pos=quit;success
	bool quiet;
	int m_groupID;

	bool m_rotateLogs; // check for log rotation in this thread?
	time_t m_lastRotation;

};
