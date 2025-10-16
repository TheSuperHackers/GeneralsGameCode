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

/*
Ok, how this should have been done is to make each of the command types
have a bitmask telling which command message header information
each command type required.  That would make finding out the size of
a particular command easier to find, without so much repetitious code.
We would still need to have a separate function for each command type
for the data, but at least that wouldn't be repeating code, that would
be specialized code.
*/



#pragma once

#ifndef __NETPACKET_H
#define __NETPACKET_H

#include "NetworkDefs.h"
#include "GameNetwork/NetCommandList.h"
#include "Common/MessageStream.h"
#include "Common/GameMemory.h"

class NetPacket;

typedef std::list<NetPacket *> NetPacketList;
typedef std::list<NetPacket *>::iterator NetPacketListIter;

class NetPacket : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetPacket, "NetPacket")
public:
	NetPacket();
	NetPacket(TransportMessage *msg);
	//virtual ~NetPacket();

	void init();
	void reset();
	void setAddress(Int addr, Int port);
	Bool addCommand(NetCommandRef *msg);
	Int getNumCommands();

	NetCommandList *getCommandList();

	static NetCommandRef * ConstructNetCommandMsgFromRawData(UnsignedByte *data, UnsignedShort dataLength);
	static NetPacketList ConstructBigCommandPacketList(NetCommandRef *ref);

	UnsignedByte *getData();
	Int getLength();
	UnsignedInt getAddr();
	UnsignedShort getPort();

protected:
	static UnsignedInt GetBufferSizeNeededForCommand(NetCommandMsg *msg);
	static void FillBufferWithCommand(UnsignedByte *buffer, NetCommandRef *msg);

	// These functions return the size of the command without any compression, repetition, etc.
	// i.e. All of the required fields are taken into account when returning the size.
	static UnsignedInt GetGameCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetAckCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetFrameCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetPlayerLeaveCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetRunAheadMetricsCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetRunAheadCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetDestroyPlayerCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetKeepAliveCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetDisconnectKeepAliveCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetDisconnectPlayerCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetPacketRouterQueryCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetPacketRouterAckCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetDisconnectChatCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetDisconnectVoteCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetChatCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetProgressMessageSize(NetCommandMsg *msg);
	static UnsignedInt GetLoadCompleteMessageSize(NetCommandMsg *msg);
	static UnsignedInt GetTimeOutGameStartMessageSize(NetCommandMsg *msg);
	static UnsignedInt GetWrapperCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetFileCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetFileAnnounceCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetFileProgressCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetDisconnectFrameCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetDisconnectScreenOffCommandSize(NetCommandMsg *msg);
	static UnsignedInt GetFrameResendRequestCommandSize(NetCommandMsg *msg);

	static void FillBufferWithGameCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithAckCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithFrameCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithPlayerLeaveCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithRunAheadMetricsCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithRunAheadCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithDestroyPlayerCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithKeepAliveCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithDisconnectKeepAliveCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithDisconnectPlayerCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithPacketRouterQueryCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithPacketRouterAckCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithDisconnectChatCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithDisconnectVoteCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithChatCommand(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithProgressMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithLoadCompleteMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithTimeOutGameStartMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithFileMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithFileProgressMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithFileAnnounceMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithDisconnectFrameMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithDisconnectScreenOffMessage(UnsignedByte *buffer, NetCommandRef *msg);
	static void FillBufferWithFrameResendRequestMessage(UnsignedByte *buffer, NetCommandRef *msg);

	Bool addFrameCommand(NetCommandRef *msg);
	Bool isRoomForFrameMessage(NetCommandRef *msg);
	Bool addAckCommand(NetCommandRef *msg, UnsignedShort commandID, UnsignedByte originalPlayerID);
	Bool addAckStage1Command(NetCommandRef *msg);
	Bool addAckStage2Command(NetCommandRef *msg);
	Bool addAckBothCommand(NetCommandRef *msg);
	Bool isRoomForAckMessage(NetCommandRef *msg);
	Bool addGameCommand(NetCommandRef *msg);
	Bool isRoomForGameMessage(NetCommandRef *msg, GameMessage *gmsg);
	Bool addPlayerLeaveCommand(NetCommandRef *msg);
	Bool isRoomForPlayerLeaveMessage(NetCommandRef *msg);
	Bool addRunAheadMetricsCommand(NetCommandRef *msg);
	Bool isRoomForRunAheadMetricsMessage(NetCommandRef *msg);
	Bool addRunAheadCommand(NetCommandRef *msg);
	Bool isRoomForRunAheadMessage(NetCommandRef *msg);
	Bool addDestroyPlayerCommand(NetCommandRef *msg);
	Bool isRoomForDestroyPlayerMessage(NetCommandRef *msg);
	Bool addKeepAliveCommand(NetCommandRef *msg);
	Bool isRoomForKeepAliveMessage(NetCommandRef *msg);
	Bool addDisconnectKeepAliveCommand(NetCommandRef *msg);
	Bool isRoomForDisconnectKeepAliveMessage(NetCommandRef *msg);
	Bool addDisconnectPlayerCommand(NetCommandRef *msg);
	Bool isRoomForDisconnectPlayerMessage(NetCommandRef *msg);
	Bool addPacketRouterQueryCommand(NetCommandRef *msg);
	Bool isRoomForPacketRouterQueryMessage(NetCommandRef *msg);
	Bool addPacketRouterAckCommand(NetCommandRef *msg);
	Bool isRoomForPacketRouterAckMessage(NetCommandRef *msg);
	Bool addDisconnectChatCommand(NetCommandRef *msg);
	Bool isRoomForDisconnectChatMessage(NetCommandRef *msg);
	Bool addChatCommand(NetCommandRef *msg);
	Bool isRoomForChatMessage(NetCommandRef *msg);
	Bool addDisconnectVoteCommand(NetCommandRef *msg);
	Bool isRoomForDisconnectVoteMessage(NetCommandRef *msg);
	Bool addProgressMessage( NetCommandRef *msg );
	Bool isRoomForProgressMessage( NetCommandRef *msg );
	Bool addLoadCompleteMessage( NetCommandRef *msg );
	Bool isRoomForLoadCompleteMessage( NetCommandRef *msg );
	Bool addTimeOutGameStartMessage( NetCommandRef *msg );
	Bool isRoomForTimeOutGameStartMessage( NetCommandRef *msg );
	Bool addWrapperCommand(NetCommandRef *msg);
	Bool isRoomForWrapperMessage(NetCommandRef *msg);
	Bool addFileCommand(NetCommandRef *msg);
	Bool isRoomForFileMessage(NetCommandRef *msg);
	Bool addFileAnnounceCommand(NetCommandRef *msg);
	Bool isRoomForFileAnnounceMessage(NetCommandRef *msg);
	Bool addFileProgressCommand(NetCommandRef *msg);
	Bool isRoomForFileProgressMessage(NetCommandRef *msg);
	Bool addDisconnectFrameCommand(NetCommandRef *msg);
	Bool isRoomForDisconnectFrameMessage(NetCommandRef *msg);
	Bool addDisconnectScreenOffCommand(NetCommandRef *msg);
	Bool isRoomForDisconnectScreenOffMessage(NetCommandRef *msg);
	Bool addFrameResendRequestCommand(NetCommandRef *msg);
	Bool isRoomForFrameResendRequestMessage(NetCommandRef *msg);

	Bool isAckRepeat(NetCommandRef *msg);
	Bool isAckBothRepeat(NetCommandRef *msg);
	Bool isAckStage1Repeat(NetCommandRef *msg);
	Bool isAckStage2Repeat(NetCommandRef *msg);
	Bool isFrameRepeat(NetCommandRef *msg);

	static NetCommandMsg * readGameMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readAckBothMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readAckStage1Message(UnsignedByte *data, Int &i);
	static NetCommandMsg * readAckStage2Message(UnsignedByte *data, Int &i);
	static NetCommandMsg * readFrameMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readPlayerLeaveMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readRunAheadMetricsMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readRunAheadMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readDestroyPlayerMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readKeepAliveMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readDisconnectKeepAliveMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readDisconnectPlayerMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readPacketRouterQueryMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readPacketRouterAckMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readDisconnectChatMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readDisconnectVoteMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readChatMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readProgressMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readLoadCompleteMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readTimeOutGameStartMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readWrapperMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readFileMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readFileAnnounceMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readFileProgressMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readDisconnectFrameMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readDisconnectScreenOffMessage(UnsignedByte *data, Int &i);
	static NetCommandMsg * readFrameResendRequestMessage(UnsignedByte *data, Int &i);

	void writeGameMessageArgumentToPacket(GameMessageArgumentDataType type, GameMessageArgumentType arg);
	static void readGameMessageArgumentFromPacket(GameMessageArgumentDataType type, NetGameCommandMsg *msg, UnsignedByte *data, Int &i);

	void dumpPacketToLog();

protected:
	UnsignedByte		m_packet[MAX_PACKET_SIZE];
	Int							m_packetLen;
	UnsignedInt			m_addr;
	Int							m_numCommands;
	NetCommandRef*	m_lastCommand;
	UnsignedInt			m_lastFrame;
	UnsignedShort		m_port;
	UnsignedShort		m_lastCommandID;
	UnsignedByte		m_lastPlayerID;
	UnsignedByte		m_lastCommandType;
	UnsignedByte		m_lastRelay;
};

#endif // __NETPACKET_H
