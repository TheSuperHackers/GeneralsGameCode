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

/**
 * The connection manager maintains a collection of Connections to each player.  Messages are queued up
 * from here, as well as removed through here.
 */

#pragma once

#ifndef __CONNECTIONMANAGER_H
#define __CONNECTIONMANAGER_H

#include "GameNetwork/Connection.h"
#include "GameNetwork/NetCommandList.h"
#include "GameNetwork/Transport.h"
#include "GameNetwork/FrameDataManager.h"
#include "GameNetwork/FrameMetrics.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/DisconnectManager.h"

class GameInfo;
class NetCommandWrapperList;

typedef std::map<UnsignedShort, AsciiString> FileCommandMap;
typedef std::map<UnsignedShort, UnsignedByte> FileMaskMap;
typedef std::map<UnsignedShort, Int> FileProgressMap;

class ConnectionManager
{
public:
	ConnectionManager();
	~ConnectionManager();

	virtual void init();				///< Initialize this instance.
	virtual void reset();				///< Take this instance back to the initial state.
	virtual void update(Bool isInGame);			///< Service the Connections being managed by this instance.

	// End SubsystemInterface functions

	void updateRunAhead(Int oldRunAhead, Int frameRate, Bool didSelfSlug, Int nextExecutionFrame);	///< Update the run ahead value.  If we are the current packet router, issue the command.

	void attachTransport(Transport *transport);

	void parseUserList(const GameInfo *game);
	void sendChat(UnicodeString text, Int playerMask, UnsignedInt executionFrame);
	void sendDisconnectChat(UnicodeString text);
	void sendLocalCommand(NetCommandMsg *msg, UnsignedByte relay = 0xff);		///< Send command to the players specified in the relay, goes through packet router.
	void sendLocalCommandDirect(NetCommandMsg *msg, UnsignedByte relay);		///< Send command directly to the players specified, doesn't go through packet router.
	void sendLocalGameMessage(GameMessage *msg, UnsignedInt frame);
	void sendCommand(NetCommandMsg *msg);
	Bool allCommandsReady(UnsignedInt frame, Bool justTesting = FALSE);
	void handleAllCommandsReady(void);
	NetCommandList *getFrameCommandList(UnsignedInt frame);
//	void AddConnection(User *user, UnsignedInt slot);
	void determineRouterFallbackPlan();
	void zeroFrames(UnsignedInt startingFrame, UnsignedInt numFrames);
	void destroyGameMessages();
//	void createConnections(UnsignedInt numberOfPlayers, UnsignedInt localSlot);
	void setLocalAddress(UnsignedInt ip, UnsignedInt port);
	void initTransport();
	void processFrameTick(UnsignedInt frame);
	void handleLocalPlayerLeaving(UnsignedInt frame);

	void sendFile(AsciiString path, UnsignedByte playerMask, UnsignedShort commandID);
	UnsignedShort sendFileAnnounce(AsciiString path, UnsignedByte playerMask);
	Int getFileTransferProgress(Int playerID, AsciiString path);
	Bool areAllQueuesEmpty(void);

	UnsignedInt getLocalPlayerID();
	UnicodeString getPlayerName(Int playerNum);
	Int getNumPlayers();

	UnsignedInt getPacketRouterFallbackSlot(Int packetRouterNumber);	///< Returns the slot of the given packet router number in the fallback plan.
	UnsignedInt getPacketRouterSlot();																///< Returns the current packet router's slot.
	PlayerLeaveCode disconnectPlayer(Int slot);												///< Disconnect this player immediately.  This should only be called by the disconnect manager.
	void disconnectLocalPlayer();																			///< Does whatever is necessary to get TheNetwork to realize that it should be leaving the game now.
	void quitGame();																									///< Disconnect from the game RIGHT NOW!!  Tell everyone else we are disconnecting.

	void voteForPlayerDisconnect(Int slot);														///< Register a vote for a player to be disconnected.

	void resendPendingCommands();																			///< Resend the pending commands to the packet router.

	void setFrameGrouping(time_t frameGrouping);				///< Set the number of frames that are grouped together into packets.

	PlayerLeaveCode processPlayerLeave(NetPlayerLeaveCommandMsg *msg);
	Bool canILeave();																///< Returns true if the local player is allowed to leave.

	// Bandwidth metrics
	Real getIncomingBytesPerSecond( void );
	Real getIncomingPacketsPerSecond( void );
	Real getOutgoingBytesPerSecond( void );
	Real getOutgoingPacketsPerSecond( void );
	Real getUnknownBytesPerSecond( void );
	Real getUnknownPacketsPerSecond( void );
	UnsignedInt getPacketArrivalCushion( void );

	UnsignedInt getMinimumCushion();

	void flushConnections();

	void processChat(NetChatCommandMsg *msg); // this actually needs to be public because it is frame-synchronized

	void updateLoadProgress( Int progress );
	void loadProgressComplete( void );
	void sendTimeOutGameStart( void );

	Bool isPacketRouter( void );

	Bool isPlayerConnected( Int playerID );

	void notifyOthersOfCurrentFrame(Int frame);
	void sendFrameDataToPlayer(UnsignedInt playerID, UnsignedInt startingFrame);
	void sendSingleFrameToPlayer(UnsignedInt playerID, UnsignedInt frame);
	void notifyOthersOfNewFrame(UnsignedInt frame);

	UnsignedInt getNextPacketRouterSlot(UnsignedInt playerID); ///< returns the packet router player that comes after the given player.

	Int getAverageFPS( void );
	Int getSlotAverageFPS(Int slot);

#if defined(RTS_DEBUG)
	void debugPrintConnectionCommands();
#endif

	// For disconnect blame assignment
	UnsignedInt getPingFrame();
	Int getPingsSent();
	Int getPingsRecieved();

private:
	void doRelay();
	void doKeepAlive();
	void sendRemoteCommand(NetCommandRef *msg);
	void ackCommand(NetCommandRef *ref, UnsignedInt localSlot);

	Bool processNetCommand(NetCommandRef *ref);
	void processAckStage1(NetCommandMsg *msg);
	void processAckStage2(NetCommandMsg *msg);
	void processAck(NetCommandMsg *msg);
	void processFrameInfo(NetFrameCommandMsg *msg);
	void processRunAheadMetrics(NetRunAheadMetricsCommandMsg *msg);
	void processDisconnectChat(NetDisconnectChatCommandMsg *msg);
	void processProgress( NetProgressCommandMsg *msg );
	void processLoadComplete( NetCommandMsg *msg );
	void processTimeOutGameStart( NetCommandMsg *msg );
	void processWrapper(NetCommandRef *ref);
	void processFrameResendRequest(NetFrameResendRequestCommandMsg *msg);

	void processFile(NetFileCommandMsg *ref);
	void processFileAnnounce(NetFileAnnounceCommandMsg *ref);
	void processFileProgress(NetFileProgressCommandMsg *ref);

	//	void doPerFrameMetrics(UnsignedInt frame);
	void getMinimumFps(Int &minFps, Int &minFpsPlayer);			///< Returns the smallest FPS in the m_fpsAverages list.
	Real getMaximumLatency(); ///< Returns the highest average latency between players.

	void requestFrameDataResend(Int playerID, UnsignedInt frame); ///< request of this player that he send the specified frame's data.

	// The connections are set up like the slot list. The connection corresponding to the local
	// player's position in the slot list will be NULL.  Connections corresponding to slots that
	// do not have a player will also be NULL.
	Connection *m_connections[MAX_SLOTS];

	Transport *m_transport;
	UnsignedInt m_localSlot;
	UnsignedInt m_packetRouterSlot;
	UnsignedInt m_packetRouterFallback[MAX_SLOTS];
	UnsignedInt m_localAddr;
	UnsignedInt m_localPort;
	User* m_localUser;

	DisconnectManager *m_disconnectManager;						///< Controls the disconnect dialog.

	FrameDataManager *m_frameData[MAX_SLOTS];

	NetCommandList *m_pendingCommands;
	NetCommandList *m_relayedCommands;

	FrameMetrics m_frameMetrics;

	NetCommandWrapperList *m_netCommandWrapperList;

	// These variables are used to keep track of the other players' average fps and latency.
	// yup.
	Real m_latencyAverages[MAX_SLOTS];
	Int  m_fpsAverages[MAX_SLOTS];
	Int  m_minFpsPlayer;
	Int  m_minFps;
	UnsignedInt m_smallestPacketArrivalCushion;
	Bool m_didSelfSlug;

	// -----------------------------------------------------------------------------
	FileCommandMap s_fileCommandMap;
	FileMaskMap s_fileRecipientMaskMap;
	FileProgressMap s_fileProgressMap[MAX_SLOTS];
	// -----------------------------------------------------------------------------
};

#endif
