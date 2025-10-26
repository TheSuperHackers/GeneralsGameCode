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
//#include <wstring.h>
//#include <dictionary.h>
//#include <arraylist.h>
#include "matcher.h"
#include "global.h"

#include <string>
#include <bitset>
#include <vector>
#include <map>
#include <Utility/hash_map_adapter.h>

typedef std::vector<bool> MapBitSet;

// =====================================================================
// Users
// =====================================================================

// Here are the states a matcher can be in:
typedef enum
{
    STATUS_INVAL = 0,
    STATUS_INCHANNEL,  // Just entered the channel
    STATUS_WORKING,    // Sent info, needs to be matched
    STATUS_MATCHED,    // Been matched, but is still in the channel
} UserStatus;

class GeneralsUser
{
public:
	GeneralsUser(void);
	UserStatus status;

	int points, minPoints, maxPoints;

	int discons, maxDiscons;

	int country;
	int color;

	bool widened;
	time_t timeToWiden;
	time_t matchStart; // when did we request a match?

	// This is a ping to a designated 3rd-party server who just
	// responds to pings.  The idea is that if the game server is
	// behind a firewall, the client will have a 1000ms ping to it,
	// even though he might be very close.  To combat this, we have
	// clients & servers ping some 3rd-party servers & calculate a
	// pesudo-ping based on the sum of pings server-->3rd-->client.
	std::vector<int> pseudoPing;
	int maxPing;

	unsigned int IP;
	int NAT;

	MapBitSet maps;

	int numPlayers;
};

// =====================================================================
// Matcher class
// =====================================================================

typedef std::map<std::string, GeneralsUser*> UserMap;
typedef std::map<int, UserMap> LadderMap;

class GeneralsMatcher : public MatcherClass
{
public:
	GeneralsMatcher();
	virtual ~GeneralsMatcher()
	{}

	virtual void init(void);
	virtual void checkMatches(void);

	virtual void handleDisconnect( const char *reason );
	virtual void handleRoomMessage( const char *nick, const char *message, MessageType messageType );
	virtual void handlePlayerMessage( const char *nick, const char *message, MessageType messageType );
	virtual void handlePlayerJoined( const char *nick );
	virtual void handlePlayerLeft( const char *nick );
	virtual void handlePlayerChangedNick( const char *oldNick, const char *newNick );
	virtual void handlePlayerEnum( bool success, int gameSpyIndex, const char *nick, int flags );

private:
	LadderMap m_ladders;
	UserMap m_nonLadderUsers1v1;
	UserMap m_nonLadderUsers2v2;
	UserMap m_nonLadderUsers3v3;
	UserMap m_nonLadderUsers4v4;
	UserMap m_nonMatchingUsers;

	double computeMatchFitness(const std::string& i1, const GeneralsUser *u1, const std::string& i2, const GeneralsUser *u2);

	GeneralsUser* findUser(const std::string& who);
	GeneralsUser* findUserInLadder(const std::string& who, int ladderID);
	GeneralsUser* findUserInAnyLadder(const std::string& who);
	GeneralsUser* findNonLadderUser(const std::string& who);
	GeneralsUser* findNonMatchingUser(const std::string& who);

	void addUser(const std::string& who);
	void addUserInLadder(const std::string& who, int ladderID, GeneralsUser *user);
	void addUserInAnyLadder(const std::string& who, GeneralsUser *user);
	void addNonLadderUser(const std::string& who, GeneralsUser *user);
	void addNonMatchingUser(const std::string& who, GeneralsUser *user);

	bool removeUser(const std::string& who);
	GeneralsUser* removeUserInLadder(const std::string& who, int ladderID);
	GeneralsUser* removeUserInAnyLadder(const std::string& who);
	GeneralsUser* removeNonLadderUser(const std::string& who);
	GeneralsUser* removeNonMatchingUser(const std::string& who);

	void checkMatchesInUserMap(UserMap& userMap, int ladderID, int numPlayers, bool showPoolSize);

	void dumpUsers(void);

	void sendMatchInfo(std::string name1, std::string name2, std::string name3, std::string name4,
	                   std::string name5, std::string name6, std::string name7, std::string name8,
	                   GeneralsUser *user1, GeneralsUser *user2, GeneralsUser *user3, GeneralsUser *user4,
	                   GeneralsUser *user5, GeneralsUser *user6, GeneralsUser *user7, GeneralsUser *user8,
	                   int numPlayers, int ladderID);

	// Command handlers for above privmsg commands (offset is the
	// offset for the getToken() past the command token)
	bool handleUserInfo(const char *nick, const std::string& msg);
	bool handleUserWiden(const char *nick);

	// Weights for various matching parameters
	int weightLowPing;
	int weightAvgPoints;
	int totalWeight;

	time_t m_nextPoolSizeAnnouncement;
	int m_secondsBetweenPoolSizeAnnouncements;

	//typedef std::vector<std::string> StringVec;
	//StringVec mapFileList;
}
;

// =====================================================================
// TEST Client Matcher class
// =====================================================================

class GeneralsClientMatcher : public MatcherClass
{
public:
	GeneralsClientMatcher();
	virtual ~GeneralsClientMatcher()
	{}

	virtual void init(void);
	virtual void checkMatches(void);

	virtual void handleDisconnect( const char *reason );
	virtual void handleRoomMessage( const char *nick, const char *message, MessageType messageType );
	virtual void handlePlayerMessage( const char *nick, const char *message, MessageType messageType );
	virtual void handlePlayerJoined( const char *nick );
	virtual void handlePlayerLeft( const char *nick );
	virtual void handlePlayerChangedNick( const char *oldNick, const char *newNick );
	virtual void handlePlayerEnum( bool success, int gameSpyIndex, const char *nick, int flags );

private:
}
;
