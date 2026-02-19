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

	// This function returns the size of the command without any compression, repetition, etc.
	// i.e. All of the required fields are taken into account when returning the size.
	static UnsignedInt GetBufferSizeNeededForCommand(NetCommandMsg *msg);

protected:
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
