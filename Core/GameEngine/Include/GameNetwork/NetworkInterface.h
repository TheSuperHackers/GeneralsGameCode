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

// NetworkInterface.h ///////////////////////////////////////////////////////////////
// Network singleton class - defines interface to Network methods
// Author: Matthew D. Campbell, July 2001

#pragma once

#include "Common/MessageStream.h"
#include "GameNetwork/ConnectionManager.h"
#include "GameNetwork/User.h"
#include "GameNetwork/NetworkDefs.h"

// forward declarations

struct CommandPacket;
class GameMessage;
class GameInfo;

void ClearCommandPacket(UnsignedInt frame);										///< ClearCommandPacket clears the command packet at the start of the frame.
CommandPacket *GetCommandPacket(void);											///< TheNetwork calls GetCommandPacket to get commands to send.



/**
 * Interface definition for the Network.
 */
class NetworkInterface : public SubsystemInterface
{
protected:

public:
	virtual ~NetworkInterface() { };

	static NetworkInterface * createNetwork( void );

	//---------------------------------------------------------------------------------------
	// SubsystemInterface functions
	virtual void init( void ) = 0;																		///< Initialize the network
	virtual void reset( void ) = 0;																		///< Re-initialize the network
	virtual void update( void ) = 0;																	///< Updates the network
	virtual void liteupdate( void ) = 0;															///< does a lightweight update for passing messages around.

	virtual void setLocalAddress(UnsignedInt ip, UnsignedInt port) = 0;	///< Tell the network what local ip and port to bind to.
	virtual Bool isFrameDataReady( void ) = 0;												///< Are the commands for the next frame available?
	virtual Bool isStalling() = 0;
	virtual void parseUserList( const GameInfo *game ) = 0;						///< Parse a userlist, creating connections
	virtual void startGame(void) = 0;																	///< Sets the network game frame counter to -1
	virtual UnsignedInt getRunAhead(void) = 0;												///< Get the current RunAhead value
	virtual UnsignedInt getFrameRate(void) = 0;												///< Get the current allowed frame rate.
	virtual UnsignedInt getPacketArrivalCushion(void) = 0;						///< Get the smallest packet arrival cushion since this was last called.

	// Chat functions
	virtual void sendChat(UnicodeString text, Int playerMask) = 0;		///< Send a chat line using the normal system.
	virtual void sendDisconnectChat(UnicodeString text) = 0;					///< Send a chat line using the disconnect manager.

	virtual void sendFile(AsciiString path, UnsignedByte playerMask, UnsignedShort commandID) = 0;
	virtual UnsignedShort sendFileAnnounce(AsciiString path, UnsignedByte playerMask) = 0;
	virtual Int getFileTransferProgress(Int playerID, AsciiString path) = 0;
	virtual Bool areAllQueuesEmpty(void) = 0;

	virtual void quitGame() = 0;																			///< Quit the game right now.

	virtual void selfDestructPlayer(Int index) = 0;

	virtual void voteForPlayerDisconnect(Int slot) = 0;								///< register a vote towards this player's disconnect.
	virtual Bool isPacketRouter( void ) = 0;

	// Bandwidth metrics
	virtual Real getIncomingBytesPerSecond( void ) = 0;
	virtual Real getIncomingPacketsPerSecond( void ) = 0;
	virtual Real getOutgoingBytesPerSecond( void ) = 0;
	virtual Real getOutgoingPacketsPerSecond( void ) = 0;
	virtual Real getUnknownBytesPerSecond( void ) = 0;
	virtual Real getUnknownPacketsPerSecond( void ) = 0;

	virtual void updateLoadProgress( Int percent ) = 0;
	virtual void loadProgressComplete( void ) = 0;
	virtual void sendTimeOutGameStart( void ) = 0;
	virtual UnsignedInt getLocalPlayerID()= 0;
	virtual UnicodeString getPlayerName(Int playerNum)= 0;
	virtual Int getNumPlayers() = 0;

	virtual Int getAverageFPS() = 0;
	virtual Int getSlotAverageFPS(Int slot) = 0;

	virtual void attachTransport(Transport *transport) = 0;
	virtual void initTransport() = 0;
	virtual Bool sawCRCMismatch() = 0;
	virtual void setSawCRCMismatch() = 0;

	virtual Bool isPlayerConnected(Int playerID) = 0;

	virtual void notifyOthersOfCurrentFrame() = 0;					///< Tells all the other players what frame we are on.
	virtual void notifyOthersOfNewFrame(UnsignedInt frame) = 0;							///< Tells all the other players that we are on a new frame.

	virtual Int  getExecutionFrame() = 0;																			///< Returns the next valid frame for simultaneous command execution.

#if defined(RTS_DEBUG)
	virtual void toggleNetworkOn( void ) = 0;													///< toggle whether or not to send network traffic.
#endif

	// For disconnect blame assignment
	virtual UnsignedInt getPingFrame() = 0;
	virtual Int getPingsSent() = 0;
	virtual Int getPingsRecieved() = 0;
};


/**
 * ResolveIP turns a string ("games2.westwood.com", or "192.168.0.1") into
 * a 32-bit unsigned integer.
 */
UnsignedInt ResolveIP(AsciiString host);
