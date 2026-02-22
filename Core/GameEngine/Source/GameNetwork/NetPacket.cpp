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

////////// NetPacket.cpp ///////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameNetwork/NetPacket.h"
#include "GameNetwork/NetCommandMsg.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/networkutil.h"
#include "GameNetwork/GameMessageParser.h"
#include "GameNetwork/NetPacketStructs.h"


// This function assumes that all of the fields are either of default value or are
// present in the raw data.
NetCommandRef * NetPacket::ConstructNetCommandMsgFromRawData(UnsignedByte *data, UnsignedShort dataLength) {
	NetCommandType commandType = NETCOMMANDTYPE_GAMECOMMAND;
	UnsignedByte commandTypeByte = static_cast<UnsignedByte>(commandType);
	UnsignedShort commandID = 0;
	UnsignedInt frame = 0;
	UnsignedByte playerID = 0;
	UnsignedByte relay = 0;

	Int offset = 0;
	NetCommandRef *ref = nullptr;
	NetCommandMsg *msg = nullptr;

	while (offset < (Int)dataLength) {

		switch (data[offset]) {

		case NetPacketFieldTypes::CommandType:
			++offset;
			memcpy(&commandTypeByte, data + offset, sizeof(commandTypeByte));
			offset += sizeof(commandTypeByte);
			commandType = static_cast<NetCommandType>(commandTypeByte);
			break;

		case NetPacketFieldTypes::Relay:
			++offset;
			memcpy(&relay, data + offset, sizeof(relay));
			offset += sizeof(relay);
			break;

		case NetPacketFieldTypes::Frame:
			++offset;
			memcpy(&frame, data + offset, sizeof(frame));
			offset += sizeof(frame);
			break;

		case NetPacketFieldTypes::PlayerId:
			++offset;
			memcpy(&playerID, data + offset, sizeof(playerID));
			offset += sizeof(playerID);
			break;

		case NetPacketFieldTypes::CommandId:
			++offset;
			memcpy(&commandID, data + offset, sizeof(commandID));
			offset += sizeof(commandID);
			break;

		case NetPacketFieldTypes::Data:
			++offset;

			switch (commandType) {

			case NETCOMMANDTYPE_GAMECOMMAND:
				msg = readGameMessage(data, offset);
				break;
			case NETCOMMANDTYPE_ACKBOTH:
				msg = readAckBothMessage(data, offset);
				break;
			case NETCOMMANDTYPE_ACKSTAGE1:
				msg = readAckStage1Message(data, offset);
				break;
			case NETCOMMANDTYPE_ACKSTAGE2:
				msg = readAckStage2Message(data, offset);
				break;
			case NETCOMMANDTYPE_FRAMEINFO:
				msg = readFrameMessage(data, offset);
				break;
			case NETCOMMANDTYPE_PLAYERLEAVE:
				msg = readPlayerLeaveMessage(data, offset);
				break;
			case NETCOMMANDTYPE_RUNAHEADMETRICS:
				msg = readRunAheadMetricsMessage(data, offset);
				break;
			case NETCOMMANDTYPE_RUNAHEAD:
				msg = readRunAheadMessage(data, offset);
				break;
			case NETCOMMANDTYPE_DESTROYPLAYER:
				msg = readDestroyPlayerMessage(data, offset);
				break;
			case NETCOMMANDTYPE_KEEPALIVE:
				msg = readKeepAliveMessage(data, offset);
				break;
			case NETCOMMANDTYPE_DISCONNECTKEEPALIVE:
				msg = readDisconnectKeepAliveMessage(data, offset);
				break;
			case NETCOMMANDTYPE_DISCONNECTPLAYER:
				msg = readDisconnectPlayerMessage(data, offset);
				break;
			case NETCOMMANDTYPE_PACKETROUTERQUERY:
				msg = readPacketRouterQueryMessage(data, offset);
				break;
			case NETCOMMANDTYPE_PACKETROUTERACK:
				msg = readPacketRouterAckMessage(data, offset);
				break;
			case NETCOMMANDTYPE_DISCONNECTCHAT:
				msg = readDisconnectChatMessage(data, offset);
				break;
			case NETCOMMANDTYPE_DISCONNECTVOTE:
				msg = readDisconnectVoteMessage(data, offset);
				break;
			case NETCOMMANDTYPE_CHAT:
				msg = readChatMessage(data, offset);
				break;
			case NETCOMMANDTYPE_PROGRESS:
				msg = readProgressMessage(data, offset);
				break;
			case NETCOMMANDTYPE_LOADCOMPLETE:
				msg = readLoadCompleteMessage(data, offset);
				break;
			case NETCOMMANDTYPE_TIMEOUTSTART:
				msg = readTimeOutGameStartMessage(data, offset);
				break;
			case NETCOMMANDTYPE_WRAPPER:
				msg = readWrapperMessage(data, offset);
				break;
			case NETCOMMANDTYPE_FILE:
				msg = readFileMessage(data, offset);
				break;
			case NETCOMMANDTYPE_FILEANNOUNCE:
				msg = readFileAnnounceMessage(data, offset);
				break;
			case NETCOMMANDTYPE_FILEPROGRESS:
				msg = readFileProgressMessage(data, offset);
				break;
			case NETCOMMANDTYPE_DISCONNECTFRAME:
				msg = readDisconnectFrameMessage(data, offset);
				break;
			case NETCOMMANDTYPE_DISCONNECTSCREENOFF:
				msg = readDisconnectScreenOffMessage(data, offset);
				break;
			case NETCOMMANDTYPE_FRAMERESENDREQUEST:
				msg = readFrameResendRequestMessage(data, offset);
				break;

			}

			msg->setExecutionFrame(frame);
			msg->setID(commandID);
			msg->setPlayerID(playerID);
			msg->setNetCommandType(commandType);

			ref = NEW_NETCOMMANDREF(msg);

			ref->setRelay(relay);

			msg->detach();
			msg = nullptr;

			return ref;

		}

	}

	return ref;
}

NetPacketList NetPacket::ConstructBigCommandPacketList(NetCommandRef *ref) {
	// if we don't have a unique command ID, then the wrapped command cannot
	// be identified.  Therefore don't allow commands without a unique ID to
	// be wrapped.
	NetCommandMsg *msg = ref->getCommand();

	if (!DoesCommandRequireACommandID(msg->getNetCommandType())) {
		DEBUG_CRASH(("Trying to wrap a command that doesn't have a unique command ID"));
		return NetPacketList();
	}

	UnsignedInt bufferSize = GetBufferSizeNeededForCommand(msg);  // need to implement.  I have a drinking problem.
	UnsignedByte *bigPacketData = nullptr;

	NetPacketList packetList;

	// create the buffer for the huge message and fill the buffer with that message.
	UnsignedInt bigPacketCurrentOffset = 0;
	bigPacketData = NEW UnsignedByte[bufferSize];
	ref->getCommand()->copyBytesForNetPacket(bigPacketData, *ref);

	// create the wrapper command message we'll be using.
	NetWrapperCommandMsg *wrapperMsg = newInstance(NetWrapperCommandMsg);
	// get the amount of space needed for the wrapper message, not including the wrapped command data.
	UnsignedInt wrapperSize = GetBufferSizeNeededForCommand(wrapperMsg);
	UnsignedInt commandSizePerPacket = MAX_PACKET_SIZE - wrapperSize;

	UnsignedInt numChunks = bufferSize / commandSizePerPacket;
	if ((bufferSize % commandSizePerPacket) > 0) {
		++numChunks;
	}
	UnsignedInt currentChunk = 0;

	// create the packets and the wrapper messages.
	while (currentChunk < numChunks) {
		NetPacket *packet = newInstance(NetPacket);

		UnsignedShort dataSizeThisPacket = commandSizePerPacket;
		if ((bufferSize - bigPacketCurrentOffset) < dataSizeThisPacket) {
			dataSizeThisPacket = bufferSize - bigPacketCurrentOffset;
		}

		if (DoesCommandRequireACommandID(wrapperMsg->getNetCommandType())) {
			wrapperMsg->setID(GenerateNextCommandID());
		}
		wrapperMsg->setPlayerID(msg->getPlayerID());
		wrapperMsg->setExecutionFrame(msg->getExecutionFrame());

		wrapperMsg->setChunkNumber(currentChunk);
		wrapperMsg->setNumChunks(numChunks);
		wrapperMsg->setDataOffset(bigPacketCurrentOffset);
		wrapperMsg->setData(bigPacketData + bigPacketCurrentOffset, dataSizeThisPacket);
		wrapperMsg->setTotalDataLength(bufferSize);
		wrapperMsg->setWrappedCommandID(msg->getID());

		bigPacketCurrentOffset += dataSizeThisPacket;

		NetCommandRef * ref = NEW_NETCOMMANDREF(wrapperMsg);
		ref->setRelay(ref->getRelay());

		if (packet->addCommand(ref) == FALSE) {
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::BeginBigCommandPacketList - failed to add a wrapper command to the packet")); // I still have a drinking problem.
		}

		packetList.push_back(packet);

		deleteInstance(ref);
		ref = nullptr;

		++currentChunk;
	}
	wrapperMsg->detach();
	wrapperMsg = nullptr;

	delete[] bigPacketData;
	bigPacketData = nullptr;

	return packetList;
}

UnsignedInt NetPacket::GetBufferSizeNeededForCommand(NetCommandMsg *msg) {
	// This is where the fun begins...

	if (msg == nullptr) {
		return 0; // There was nothing to add.
	}
	// Use the virtual function for all command message types
	return msg->getSizeForNetPacket();
}

/**
 * Constructor
 */
NetPacket::NetPacket() {
	init();
}

/**
 * Constructor given raw transport data.
 */
NetPacket::NetPacket(TransportMessage *msg) {
	init();
	m_packetLen = msg->length;
	memcpy(m_packet, msg->data, MAX_PACKET_SIZE);
	m_numCommands = -1;
	m_addr = msg->addr;
	m_port = msg->port;
}

/**
 * Destructor
 */
NetPacket::~NetPacket() {
	deleteInstance(m_lastCommand);
	m_lastCommand = nullptr;
}

/**
 * Initialize all the member variable values.
 */
void NetPacket::init() {
	m_addr = 0;
	m_port = 0;
	m_numCommands = 0;
	m_packetLen = 0;
	m_packet[0] = 0;

	m_lastPlayerID = 0;
	m_lastFrame = 0;
	m_lastCommandID = 0;
	m_lastCommandType = 0;
	m_lastRelay = 0;

	m_lastCommand = nullptr;
}

void NetPacket::reset() {
	deleteInstance(m_lastCommand);
	m_lastCommand = nullptr;

	init();
}

/**
 * Set the address to which this packet is to be sent.
 */
void NetPacket::setAddress(Int addr, Int port) {
	m_addr = addr;
	m_port = port;
}

/**
 * Adds this command to the packet.  Returns false if there wasn't enough room
 * in the packet for this message, true otherwise.
 */
Bool NetPacket::addCommand(NetCommandRef *msg) {
	// This is where the fun begins...

	if (msg == nullptr) {
		return TRUE; // There was nothing to add, so it was successful.
	}

	NetCommandMsg *cmdMsg = msg->getCommand();

	Bool ackRepeat = FALSE;
	Bool frameRepeat = FALSE;

	switch (cmdMsg->getNetCommandType())
	{
		case NETCOMMANDTYPE_ACKSTAGE1:
		case NETCOMMANDTYPE_ACKSTAGE2:
		case NETCOMMANDTYPE_ACKBOTH:
			ackRepeat = isAckRepeat(msg);
			break;
		case NETCOMMANDTYPE_FRAMEINFO:
			frameRepeat = isFrameRepeat(msg);
			break;
		default:
			break;
	}

	if (ackRepeat || frameRepeat)
	{
		// Is there enough room in the packet for this message?
		if (NetPacketRepeatCommand::getSize() > (MAX_PACKET_SIZE - m_packetLen)) {
			return FALSE;
		}

		if (frameRepeat)
		{
			m_lastCommandID = cmdMsg->getID();
			++m_lastFrame; // Need this cause we're actually advancing to the next frame by adding this command.
		}

		m_packetLen += NetPacketRepeatCommand::copyBytes(m_packet + m_packetLen);
	}
	else
	{
		SmallNetPacketCommandBaseSelect select = cmdMsg->getSmallNetPacketSelect();
		select.useCommandType &= m_lastCommandType != cmdMsg->getNetCommandType();
		select.useRelay &= m_lastRelay != msg->getRelay();
		select.useFrame &= m_lastFrame != cmdMsg->getExecutionFrame();
		select.usePlayerId &= m_lastPlayerID != cmdMsg->getPlayerID();
		select.useCommandId &= ((m_lastCommandID + 1) != cmdMsg->getID()) | select.usePlayerId;

		if (select.useCommandType)
			m_lastCommandType = cmdMsg->getNetCommandType();

		if (select.useRelay)
			m_lastRelay = msg->getRelay();

		if (select.useFrame)
			m_lastFrame = cmdMsg->getExecutionFrame();

		if (select.usePlayerId)
			m_lastPlayerID = cmdMsg->getPlayerID();

		if (select.useCommandId)
			m_lastCommandID = cmdMsg->getID();

		const size_t msglen = cmdMsg->getSizeForSmallNetPacket(&select);

		// Is there enough room in the packet for this message?
		if (msglen > (MAX_PACKET_SIZE - m_packetLen)) {
			return FALSE;
		}

		m_packetLen += cmdMsg->copyBytesForSmallNetPacket(m_packet + m_packetLen, *msg, &select);
	}

	++m_numCommands;

	deleteInstance(m_lastCommand);
	m_lastCommand = NEW_NETCOMMANDREF(msg->getCommand());
	m_lastCommand->setRelay(msg->getRelay());

	return TRUE;
}

Bool NetPacket::isFrameRepeat(NetCommandRef *msg) {
	if (m_lastCommand == nullptr) {
		return FALSE;
	}
	if (m_lastCommand->getCommand()->getNetCommandType() != NETCOMMANDTYPE_FRAMEINFO) {
		return FALSE;
	}
	NetFrameCommandMsg *framemsg = (NetFrameCommandMsg *)(msg->getCommand());
	NetFrameCommandMsg *lastmsg = (NetFrameCommandMsg *)(m_lastCommand->getCommand());
	if (framemsg->getCommandCount() != 0) {
		return FALSE;
	}
	if (framemsg->getExecutionFrame() != (lastmsg->getExecutionFrame() + 1)) {
		return FALSE;
	}
	if (msg->getRelay() != m_lastCommand->getRelay()) {
		return FALSE;
	}
	if (framemsg->getID() != (lastmsg->getID() + 1)) {
		return FALSE;
	}
	return TRUE;
}

Bool NetPacket::isAckRepeat(NetCommandRef *msg) {
	if (m_lastCommand == nullptr) {
		return FALSE;
	}
	if (m_lastCommand->getCommand()->getNetCommandType() != msg->getCommand()->getNetCommandType()) {
		return FALSE;
	}
	if (msg->getCommand()->getNetCommandType() == NETCOMMANDTYPE_ACKBOTH) {
		return isAckBothRepeat(msg);
	}
	if (msg->getCommand()->getNetCommandType() == NETCOMMANDTYPE_ACKSTAGE1) {
		return isAckStage1Repeat(msg);
	}
	if (msg->getCommand()->getNetCommandType() == NETCOMMANDTYPE_ACKSTAGE2) {
		return isAckStage2Repeat(msg);
	}
	return FALSE;
}

Bool NetPacket::isAckBothRepeat(NetCommandRef *msg) {
	NetAckBothCommandMsg *ack = (NetAckBothCommandMsg *)(msg->getCommand());
	NetAckBothCommandMsg *lastAck = (NetAckBothCommandMsg *)(m_lastCommand->getCommand());
	if (lastAck->getCommandID() != (ack->getCommandID() - 1)) {
		return FALSE;
	}
	if (lastAck->getOriginalPlayerID() != ack->getOriginalPlayerID()) {
		return FALSE;
	}
	if (msg->getRelay() != m_lastCommand->getRelay()) {
		return FALSE;
	}
	return TRUE;
}

Bool NetPacket::isAckStage1Repeat(NetCommandRef *msg) {
	NetAckStage2CommandMsg *ack = (NetAckStage2CommandMsg *)(msg->getCommand());
	NetAckStage2CommandMsg *lastAck = (NetAckStage2CommandMsg *)(m_lastCommand->getCommand());
	if (lastAck->getCommandID() != (ack->getCommandID() - 1)) {
		return FALSE;
	}
	if (lastAck->getOriginalPlayerID() != ack->getOriginalPlayerID()) {
		return FALSE;
	}
	if (msg->getRelay() != m_lastCommand->getRelay()) {
		return FALSE;
	}
	return TRUE;
}

Bool NetPacket::isAckStage2Repeat(NetCommandRef *msg) {
	NetAckStage2CommandMsg *ack = (NetAckStage2CommandMsg *)(msg->getCommand());
	NetAckStage2CommandMsg *lastAck = (NetAckStage2CommandMsg *)(m_lastCommand->getCommand());
	if (lastAck->getCommandID() != (ack->getCommandID() - 1)) {
		return FALSE;
	}
	if (lastAck->getOriginalPlayerID() != ack->getOriginalPlayerID()) {
		return FALSE;
	}
	if (msg->getRelay() != m_lastCommand->getRelay()) {
		return FALSE;
	}
	return TRUE;
}

/**
 * Returns the list of commands that are in this packet.
 */
NetCommandList * NetPacket::getCommandList() {
	NetCommandList *retval = newInstance(NetCommandList);
//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::getCommandList, packet length = %d", m_packetLen));
	retval->init();

	// These need to be the same as the default values for m_lastPlayerID, m_lastFrame, etc.
	UnsignedByte playerID = 0;
	UnsignedInt frame = 0;
	UnsignedShort commandID = 1; // The first command is going to be
	UnsignedByte commandType = 0;
	UnsignedByte relay = 0;
	NetCommandRef *lastCommand = nullptr;

	Int i = 0;
	while (i < m_packetLen) {

		switch(m_packet[i]) {

		case NetPacketFieldTypes::CommandType:
			++i;
			memcpy(&commandType, m_packet + i, sizeof(UnsignedByte));
			i += sizeof(UnsignedByte);
			break;
		case NetPacketFieldTypes::Frame:
			++i;
			memcpy(&frame, m_packet + i, sizeof(UnsignedInt));
			i += sizeof(UnsignedInt);
			break;
		case NetPacketFieldTypes::PlayerId:
			++i;
			memcpy(&playerID, m_packet + i, sizeof(UnsignedByte));
			i += sizeof(UnsignedByte);
			break;
		case NetPacketFieldTypes::Relay:
			++i;
			memcpy(&relay, m_packet + i, sizeof(UnsignedByte));
			i += sizeof(UnsignedByte);
			break;
		case NetPacketFieldTypes::CommandId:
			++i;
			memcpy(&commandID, m_packet + i, sizeof(UnsignedShort));
			i += sizeof(UnsignedShort);
			break;
		case NetPacketFieldTypes::Data: {
			++i;

			NetCommandMsg *msg = nullptr;

			//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::getCommandList() - command of type %d(%s)", commandType, GetNetCommandTypeAsString((NetCommandType)commandType)));

			switch((NetCommandType)commandType)
			{
			case NETCOMMANDTYPE_GAMECOMMAND:
				msg = readGameMessage(m_packet, i);
				//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read game command from player %d for frame %d", playerID, frame));
				break;
			case NETCOMMANDTYPE_ACKBOTH:
				msg = readAckBothMessage(m_packet, i);
				break;
			case NETCOMMANDTYPE_ACKSTAGE1:
				msg = readAckStage1Message(m_packet, i);
				break;
			case NETCOMMANDTYPE_ACKSTAGE2:
				msg = readAckStage2Message(m_packet, i);
				break;
			case NETCOMMANDTYPE_FRAMEINFO:
				msg = readFrameMessage(m_packet, i);
				// frameinfodebug
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read frame %d from player %d, command count = %d, relay = 0x%X", frame, playerID, ((NetFrameCommandMsg *)msg)->getCommandCount(), relay));
				break;
			case NETCOMMANDTYPE_PLAYERLEAVE:
				msg = readPlayerLeaveMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read player leave message from player %d for execution on frame %d", playerID, frame));
				break;
			case NETCOMMANDTYPE_RUNAHEADMETRICS:
				msg = readRunAheadMetricsMessage(m_packet, i);
				break;
			case NETCOMMANDTYPE_RUNAHEAD:
				msg = readRunAheadMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read run ahead message from player %d for execution on frame %d", playerID, frame));
				break;
			case NETCOMMANDTYPE_DESTROYPLAYER:
				msg = readDestroyPlayerMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read CRC info message from player %d for execution on frame %d", playerID, frame));
				break;
			case NETCOMMANDTYPE_KEEPALIVE:
				msg = readKeepAliveMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read keep alive message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_DISCONNECTKEEPALIVE:
				msg = readDisconnectKeepAliveMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read keep alive message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_DISCONNECTPLAYER:
				msg = readDisconnectPlayerMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read disconnect player message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_PACKETROUTERQUERY:
				msg = readPacketRouterQueryMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read packet router query message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_PACKETROUTERACK:
				msg = readPacketRouterAckMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read packet router ack message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_DISCONNECTCHAT:
				msg = readDisconnectChatMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read disconnect chat message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_DISCONNECTVOTE:
				msg = readDisconnectVoteMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read disconnect vote message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_CHAT:
				msg = readChatMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read chat message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_PROGRESS:
				msg = readProgressMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read Progress message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_LOADCOMPLETE:
				msg = readLoadCompleteMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read LoadComplete message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_TIMEOUTSTART:
				msg = readTimeOutGameStartMessage(m_packet, i);
//				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read TimeOutGameStart message from player %d", playerID));
				break;
			case NETCOMMANDTYPE_WRAPPER:
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read Wrapper message from player %d", playerID));
				msg = readWrapperMessage(m_packet, i);
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Done reading Wrapper message from player %d - wrapped command was %d", playerID,
					((NetWrapperCommandMsg *)msg)->getWrappedCommandID()));
				break;
			case NETCOMMANDTYPE_FILE:
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read file message from player %d", playerID));
				msg = readFileMessage(m_packet, i);
				break;
			case NETCOMMANDTYPE_FILEANNOUNCE:
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read file announce message from player %d", playerID));
				msg = readFileAnnounceMessage(m_packet, i);
				break;
			case NETCOMMANDTYPE_FILEPROGRESS:
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read file progress message from player %d", playerID));
				msg = readFileProgressMessage(m_packet, i);
				break;
			case NETCOMMANDTYPE_DISCONNECTFRAME:
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read disconnect frame message from player %d", playerID));
				msg = readDisconnectFrameMessage(m_packet, i);
				break;
			case NETCOMMANDTYPE_DISCONNECTSCREENOFF:
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read disconnect screen off message from player %d", playerID));
				msg = readDisconnectScreenOffMessage(m_packet, i);
				break;
			case NETCOMMANDTYPE_FRAMERESENDREQUEST:
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("read frame resend request message from player %d", playerID));
				msg = readFrameResendRequestMessage(m_packet, i);
				break;
			}

			if (msg == nullptr) {
				DEBUG_CRASH(("Didn't read a message from the packet. Things are about to go wrong."));
				continue;
			}

			// set the info
			msg->setExecutionFrame(frame);
			msg->setPlayerID(playerID);
			msg->setNetCommandType((NetCommandType)commandType);
			msg->setID(commandID);

//			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("frame = %d, player = %d, command type = %d, id = %d", frame, playerID, commandType, commandID));

			// increment to the next command ID.
			if (DoesCommandRequireACommandID((NetCommandType)commandType)) {
				++commandID;
			}

			// add the message to the list.
			NetCommandRef *ref = retval->addMessage(msg);
			if (ref != nullptr) {
				ref->setRelay(relay);
			} else {
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::getCommandList - failed to set relay for message %d", msg->getID()));
			}

			deleteInstance(lastCommand);
			lastCommand = newInstance(NetCommandRef)(msg);

			msg->detach();  // Need to detach from new NetCommandMsg created by the "readXMessage" above.

			// since the message is part of the list now, we don't have to keep track of it.  So we'll just set it to null.
			msg = nullptr;
			break;
		}

		case 'Z': {

			++i;
			// Repeat the last command, doing some funky cool byte-saving stuff
			if (lastCommand == nullptr) {
				DEBUG_CRASH(("Got a repeat command with no command to repeat."));
			}

			NetCommandMsg *msg = nullptr;

			switch(commandType) {

			case NETCOMMANDTYPE_ACKSTAGE1: {
				msg = newInstance(NetAckStage1CommandMsg)();
				NetAckStage1CommandMsg* laststageone = (NetAckStage1CommandMsg*)(lastCommand->getCommand());
				((NetAckStage1CommandMsg*)msg)->setCommandID(laststageone->getCommandID() + 1);
				((NetAckStage1CommandMsg*)msg)->setOriginalPlayerID(laststageone->getOriginalPlayerID());
				break;
			}
			case NETCOMMANDTYPE_ACKSTAGE2: {
				msg = newInstance(NetAckStage2CommandMsg)();
				NetAckStage2CommandMsg* laststagetwo = (NetAckStage2CommandMsg*)(lastCommand->getCommand());
				((NetAckStage2CommandMsg*)msg)->setCommandID(laststagetwo->getCommandID() + 1);
				((NetAckStage2CommandMsg*)msg)->setOriginalPlayerID(laststagetwo->getOriginalPlayerID());
				break;
			}
			case NETCOMMANDTYPE_ACKBOTH: {
				msg = newInstance(NetAckBothCommandMsg)();
				NetAckBothCommandMsg* lastboth = (NetAckBothCommandMsg*)(lastCommand->getCommand());
				((NetAckBothCommandMsg*)msg)->setCommandID(lastboth->getCommandID() + 1);
				((NetAckBothCommandMsg*)msg)->setOriginalPlayerID(lastboth->getOriginalPlayerID());
				break;
			}
			case NETCOMMANDTYPE_FRAMEINFO: {
				msg = newInstance(NetFrameCommandMsg)();
				++frame; // this is set below.
				((NetFrameCommandMsg*)msg)->setCommandCount(0);
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Read a repeated frame command, frame = %d, player = %d, commandID = %d", frame, playerID, commandID));
				break;
			}
			default:
				DEBUG_CRASH(("Trying to repeat a command that shouldn't be repeated."));
				continue;

			}

			msg->setExecutionFrame(frame);
			msg->setPlayerID(playerID);
			msg->setNetCommandType((NetCommandType)commandType);
			msg->setID(commandID);

//			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("frame = %d, player = %d, command type = %d, id = %d", frame, playerID, commandType, commandID));

			// increment to the next command ID.
			if (DoesCommandRequireACommandID((NetCommandType)commandType)) {
				++commandID;
			}

			// add the message to the list.
			NetCommandRef *ref = retval->addMessage(msg);
			if (ref != nullptr) {
				ref->setRelay(relay);
			}

			deleteInstance(lastCommand);
//			lastCommand = newInstance(NetCommandRef)(msg);
			lastCommand = NEW_NETCOMMANDREF(msg);

			msg->detach();  // Need to detach from new NetCommandMsg created by the "readXMessage" above.

			// since the message is part of the list now, we don't have to keep track of it.  So we'll just set it to null.
			msg = nullptr;
			break;
		}

		default:
			// we don't recognize this command, but we have to increment i so we don't fall into an infinite loop.
			DEBUG_CRASH(("Unrecognized packet entry, ignoring."));
			DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::getCommandList - Unrecognized packet entry at index %d", i));
			dumpPacketToLog();
			++i;
			break;

		}

	}

	deleteInstance(lastCommand);
	lastCommand = nullptr;

	return retval;
}

/**
 * Reads the data portion of a game message from the given position in the packet.
 */
NetCommandMsg * NetPacket::readGameMessage(UnsignedByte *data, Int &i)
{
	NetGameCommandMsg *msg = newInstance(NetGameCommandMsg);

//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readGameMessage"));

	// Get the GameMessage command type.
	GameMessage::Type newType;
	memcpy(&newType, data + i, sizeof(GameMessage::Type));
	i += sizeof(GameMessage::Type);
	msg->setGameMessageType(newType);

	// Get the number of argument types
	UnsignedByte numArgTypes = 0;
	memcpy(&numArgTypes, data + i, sizeof(numArgTypes));
	i += sizeof(numArgTypes);

	// Get the types and the number of arguments of those types.
	Int totalArgCount = 0;
	GameMessageParser *parser = newInstance(GameMessageParser)();
	Int j = 0;
	for (; j < numArgTypes; ++j) {
		UnsignedByte type = (UnsignedByte)ARGUMENTDATATYPE_UNKNOWN;
		memcpy(&type, data + i, sizeof(type));
		i += sizeof(type);

		UnsignedByte argCount = 0;
		memcpy(&argCount, data + i, sizeof(argCount));
		i += sizeof(argCount);

		parser->addArgType((GameMessageArgumentDataType)type, argCount);
		totalArgCount += argCount;
	}

	GameMessageParserArgumentType *parserArgType = parser->getFirstArgumentType();
	GameMessageArgumentDataType lasttype = ARGUMENTDATATYPE_UNKNOWN;
	Int argsLeftForType = 0;
	if (parserArgType != nullptr) {
		lasttype = parserArgType->getType();
		argsLeftForType = parserArgType->getArgCount();
	}
	for (j = 0; j < totalArgCount; ++j) {
		readGameMessageArgumentFromPacket(lasttype, msg, data, i);

		--argsLeftForType;
		if (argsLeftForType == 0) {
			DEBUG_ASSERTCRASH(parserArgType != nullptr, ("parserArgType was null when it shouldn't have been."));
			if (parserArgType == nullptr) {
				return nullptr;
			}

			parserArgType = parserArgType->getNext();
			// parserArgType is allowed to be null here
			if (parserArgType != nullptr) {
				argsLeftForType = parserArgType->getArgCount();
				lasttype = parserArgType->getType();
			}
		}
	}

	deleteInstance(parser);
	parser = nullptr;

	return (NetCommandMsg *)msg;
}

void NetPacket::readGameMessageArgumentFromPacket(GameMessageArgumentDataType type, NetGameCommandMsg *msg, UnsignedByte *data, Int &i) {

	GameMessageArgumentType arg;

	switch (type) {

	case ARGUMENTDATATYPE_INTEGER:
		Int theint;
		memcpy(&theint, data + i, sizeof(theint));
		i += sizeof(theint);
		arg.integer = theint;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_REAL:
		Real thereal;
		memcpy(&thereal, data + i, sizeof(thereal));
		i += sizeof(thereal);
		arg.real = thereal;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_BOOLEAN:
		Bool thebool;
		memcpy(&thebool, data + i, sizeof(thebool));
		i += sizeof(thebool);
		arg.boolean = thebool;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_OBJECTID:
		ObjectID theobjectid;
		memcpy(&theobjectid, data + i, sizeof(theobjectid));
		i += sizeof(theobjectid);
		arg.objectID = theobjectid;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_DRAWABLEID:
		DrawableID thedrawableid;
		memcpy(&thedrawableid, data + i, sizeof(thedrawableid));
		i += sizeof(thedrawableid);
		arg.drawableID = thedrawableid;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_TEAMID:
		UnsignedInt theunsignedint;
		memcpy(&theunsignedint, data + i, sizeof(theunsignedint));
		i += sizeof(theunsignedint);
		arg.teamID = theunsignedint;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_LOCATION:
		Coord3D coord;
		memcpy(&coord, data + i, sizeof(coord));
		i += sizeof(coord);
		arg.location = coord;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_PIXEL:
		ICoord2D pixel;
		memcpy(&pixel, data + i, sizeof(pixel));
		i += sizeof(pixel);
		arg.pixel = pixel;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_PIXELREGION:
		IRegion2D reg;
		memcpy(&reg, data + i, sizeof(reg));
		i += sizeof(reg);
		arg.pixelRegion = reg;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_TIMESTAMP:
		UnsignedInt stamp;
		memcpy(&stamp, data + i, sizeof(stamp));
		i += sizeof(stamp);
		arg.timestamp = stamp;
		msg->addArgument(type, arg);
		break;

	case ARGUMENTDATATYPE_WIDECHAR:
		WideChar c;
		memcpy(&c, data + i, sizeof(c));
		i += sizeof(c);
		arg.wChar = c;
		msg->addArgument(type, arg);
		break;

	}

}

/**
 * Reads the data portion of the ack message at this position in the packet.
 */
NetCommandMsg * NetPacket::readAckBothMessage(UnsignedByte *data, Int &i) {
	NetAckBothCommandMsg *msg = newInstance(NetAckBothCommandMsg);

	//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readAckMessage, "));
	UnsignedShort cmdID = 0;

	memcpy(&cmdID, data + i, sizeof(UnsignedShort));
	i += sizeof(UnsignedShort);
	msg->setCommandID(cmdID);
	//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("commandID = %d, ", cmdID));

	UnsignedByte origPlayerID = 0;
	memcpy(&origPlayerID, data + i, sizeof(UnsignedByte));
	i += sizeof(UnsignedByte);
	msg->setOriginalPlayerID(origPlayerID);
	//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("original player id = %d", origPlayerID));

	return msg;
}

/**
 * Reads the data portion of the ack message at this position in the packet.
 */
NetCommandMsg * NetPacket::readAckStage1Message(UnsignedByte *data, Int &i) {
	NetAckStage1CommandMsg *msg = newInstance(NetAckStage1CommandMsg);

//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readAckMessage, "));
	UnsignedShort cmdID = 0;

	memcpy(&cmdID, data + i, sizeof(UnsignedShort));
	i += sizeof(UnsignedShort);
	msg->setCommandID(cmdID);
//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("commandID = %d, ", cmdID));

	UnsignedByte origPlayerID = 0;
	memcpy(&origPlayerID, data + i, sizeof(UnsignedByte));
	i += sizeof(UnsignedByte);
	msg->setOriginalPlayerID(origPlayerID);
//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("original player id = %d", origPlayerID));

	return msg;
}

/**
 * Reads the data portion of the ack message at this position in the packet.
 */
NetCommandMsg * NetPacket::readAckStage2Message(UnsignedByte *data, Int &i) {
	NetAckStage2CommandMsg *msg = newInstance(NetAckStage2CommandMsg);

//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readAckMessage, "));
	UnsignedShort cmdID = 0;

	memcpy(&cmdID, data + i, sizeof(UnsignedShort));
	i += sizeof(UnsignedShort);
	msg->setCommandID(cmdID);
//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("commandID = %d, ", cmdID));

	UnsignedByte origPlayerID = 0;
	memcpy(&origPlayerID, data + i, sizeof(UnsignedByte));
	i += sizeof(UnsignedByte);
	msg->setOriginalPlayerID(origPlayerID);
//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("original player id = %d", origPlayerID));

	return msg;
}

/**
 * Reads the data portion of the frame message at this position in the packet.
 */
NetCommandMsg * NetPacket::readFrameMessage(UnsignedByte *data, Int &i) {
	NetFrameCommandMsg *msg = newInstance(NetFrameCommandMsg);

//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readFrameMessage, "));
	UnsignedShort cmdCount = 0;

	memcpy(&cmdCount, data + i, sizeof(UnsignedShort));
	i += sizeof(UnsignedShort);
	msg->setCommandCount(cmdCount);
//	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("command count = %d, ", cmdCount));

	return msg;
}

/**
 * Reads the player leave message at this position in the packet.
 */
NetCommandMsg * NetPacket::readPlayerLeaveMessage(UnsignedByte *data, Int &i) {
	NetPlayerLeaveCommandMsg *msg = newInstance(NetPlayerLeaveCommandMsg);

	UnsignedByte leavingPlayerID = 0;

	memcpy(&leavingPlayerID, data + i, sizeof(UnsignedByte));
	i += sizeof(UnsignedByte);
	msg->setLeavingPlayerID(leavingPlayerID);

	return msg;
}

/**
 * Reads the run ahead metrics message at this position in the packet.
 */
NetCommandMsg * NetPacket::readRunAheadMetricsMessage(UnsignedByte *data, Int &i) {
	NetRunAheadMetricsCommandMsg *msg = newInstance(NetRunAheadMetricsCommandMsg);

	Real averageLatency = (Real)0.2;
	UnsignedShort averageFps = 30;

	memcpy(&averageLatency, data + i, sizeof(Real));
	i += sizeof(Real);
	msg->setAverageLatency(averageLatency);

	memcpy(&averageFps, data + i, sizeof(UnsignedShort));
	i += sizeof(UnsignedShort);
	msg->setAverageFps((Int)averageFps);
	return msg;
}

/**
 * Reads the run ahead message at this position in the packet.
 */
NetCommandMsg * NetPacket::readRunAheadMessage(UnsignedByte *data, Int &i) {
	NetRunAheadCommandMsg *msg = newInstance(NetRunAheadCommandMsg);

	UnsignedShort newRunAhead = 20;
	memcpy(&newRunAhead, data + i, sizeof(UnsignedShort));
	i += sizeof(UnsignedShort);
	msg->setRunAhead(newRunAhead);

	UnsignedByte newFrameRate = 30;
	memcpy(&newFrameRate, data + i, sizeof(UnsignedByte));
	i += sizeof(UnsignedByte);
	msg->setFrameRate(newFrameRate);

	return msg;
}

/**
 * Reads the CRC info message at this position in the packet.
 */
NetCommandMsg * NetPacket::readDestroyPlayerMessage(UnsignedByte *data, Int &i) {
	NetDestroyPlayerCommandMsg *msg = newInstance(NetDestroyPlayerCommandMsg);

	UnsignedInt newVal = 0;
	memcpy(&newVal, data + i, sizeof(UnsignedInt));
	i += sizeof(UnsignedInt);
	msg->setPlayerIndex(newVal);
	//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Saw CRC of 0x%8.8X", newCRC));

	return msg;
}

/**
 * Reads the keep alive data, of which there is none.
 */
NetCommandMsg * NetPacket::readKeepAliveMessage(UnsignedByte *data, Int &i) {
	NetKeepAliveCommandMsg *msg = newInstance(NetKeepAliveCommandMsg);

	return msg;
}

/**
 * Reads the disconnect keep alive data, of which there is none.
 */
NetCommandMsg * NetPacket::readDisconnectKeepAliveMessage(UnsignedByte *data, Int &i) {
	NetDisconnectKeepAliveCommandMsg *msg = newInstance(NetDisconnectKeepAliveCommandMsg);

	return msg;
}

/**
 * Reads the disconnect player data.  Which is the slot number of the player being disconnected.
 */
NetCommandMsg * NetPacket::readDisconnectPlayerMessage(UnsignedByte *data, Int &i) {
	NetDisconnectPlayerCommandMsg *msg = newInstance(NetDisconnectPlayerCommandMsg);

	UnsignedByte slot = 0;
	memcpy(&slot, data + i, sizeof(slot));
	i += sizeof(slot);
	msg->setDisconnectSlot(slot);

	UnsignedInt disconnectFrame = 0;
	memcpy(&disconnectFrame, data + i, sizeof(disconnectFrame));
	i += sizeof(disconnectFrame);
	msg->setDisconnectFrame(disconnectFrame);

	return msg;
}

/**
 * Reads the packet router query data, of which there is none.
 */
NetCommandMsg * NetPacket::readPacketRouterQueryMessage(UnsignedByte *data, Int &i) {
	NetPacketRouterQueryCommandMsg *msg = newInstance(NetPacketRouterQueryCommandMsg);

	return msg;
}

/**
 * Reads the packet router ack data, of which there is none.
 */
NetCommandMsg * NetPacket::readPacketRouterAckMessage(UnsignedByte *data, Int &i) {
	NetPacketRouterAckCommandMsg *msg = newInstance(NetPacketRouterAckCommandMsg);

	return msg;
}

/**
 * Reads the disconnect chat data, which is just the string.
 */
NetCommandMsg * NetPacket::readDisconnectChatMessage(UnsignedByte *data, Int &i) {
	NetDisconnectChatCommandMsg *msg = newInstance(NetDisconnectChatCommandMsg);

	WideChar text[256];
	UnsignedByte length;
	memcpy(&length, data + i, sizeof(UnsignedByte));
	++i;
	memcpy(text, data + i, length * sizeof(WideChar));
	i += length * sizeof(WideChar);
	text[length] = 0;

	UnicodeString unitext;
	unitext.set(text);

	//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readDisconnectChatMessage - read message, message is %ls", unitext.str()));

	msg->setText(unitext);
	return msg;
}

/**
 * Reads the chat data, which is just the string.
 */
NetCommandMsg * NetPacket::readChatMessage(UnsignedByte *data, Int &i) {
	NetChatCommandMsg *msg = newInstance(NetChatCommandMsg);

	WideChar text[256];
	UnsignedByte length;
	Int playerMask;
	memcpy(&length, data + i, sizeof(UnsignedByte));
	++i;
	memcpy(text, data + i, length * sizeof(WideChar));
	i += length * sizeof(WideChar);
	text[length] = 0;
	memcpy(&playerMask, data + i, sizeof(Int));
	i += sizeof(Int);


	UnicodeString unitext;
	unitext.set(text);

	//DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readChatMessage - read message, message is %ls", unitext.str()));

	msg->setText(unitext);
	msg->setPlayerMask(playerMask);
	return msg;
}

/**
 * Reads the disconnect vote data.  Which is the slot number of the player being disconnected.
 */
NetCommandMsg * NetPacket::readDisconnectVoteMessage(UnsignedByte *data, Int &i) {
	NetDisconnectVoteCommandMsg *msg = newInstance(NetDisconnectVoteCommandMsg);

	UnsignedByte slot = 0;
	memcpy(&slot, data + i, sizeof(slot));
	i += sizeof(slot);
	msg->setSlot(slot);

	UnsignedInt voteFrame = 0;
	memcpy(&voteFrame, data + i, sizeof(voteFrame));
	i += sizeof(voteFrame);
	msg->setVoteFrame(voteFrame);

	return msg;
}

/**
 * Reads the Progress data.  Which is the slot number of the player being disconnected.
 */
NetCommandMsg * NetPacket::readProgressMessage(UnsignedByte *data, Int &i) {
	NetProgressCommandMsg *msg = newInstance(NetProgressCommandMsg);

	UnsignedByte percentage = 0;
	memcpy(&percentage, data + i, sizeof(UnsignedByte));
	i += sizeof(UnsignedByte);
	msg->setPercentage(percentage);

	return msg;
}

NetCommandMsg * NetPacket::readLoadCompleteMessage(UnsignedByte *data, Int &i) {
	NetLoadCompleteCommandMsg *msg = newInstance(NetLoadCompleteCommandMsg);
	return msg;
}

NetCommandMsg * NetPacket::readTimeOutGameStartMessage(UnsignedByte *data, Int &i) {
	NetTimeOutGameStartCommandMsg *msg = newInstance(NetTimeOutGameStartCommandMsg);
	return msg;
}

NetCommandMsg * NetPacket::readWrapperMessage(UnsignedByte *data, Int &i) {
	NetWrapperCommandMsg *msg = newInstance(NetWrapperCommandMsg);

	// get the wrapped command ID
	UnsignedShort wrappedCommandID = 0;
	memcpy(&wrappedCommandID, data + i, sizeof(wrappedCommandID));
	msg->setWrappedCommandID(wrappedCommandID);
	i += sizeof(wrappedCommandID);
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readWrapperMessage - wrapped command ID == %d", wrappedCommandID));

	// get the chunk number.
	UnsignedInt chunkNumber = 0;
	memcpy(&chunkNumber, data + i, sizeof(chunkNumber));
	msg->setChunkNumber(chunkNumber);
	i += sizeof(chunkNumber);
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readWrapperMessage - chunk number = %d", chunkNumber));

	// get the number of chunks
	UnsignedInt numChunks = 0;
	memcpy(&numChunks, data + i, sizeof(numChunks));
	msg->setNumChunks(numChunks);
	i += sizeof(numChunks);
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readWrapperMessage - number of chunks = %d", numChunks));

	// get the total data length
	UnsignedInt totalDataLength = 0;
	memcpy(&totalDataLength, data + i, sizeof(totalDataLength));
	msg->setTotalDataLength(totalDataLength);
	i += sizeof(totalDataLength);
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readWrapperMessage - total data length = %d", totalDataLength));

	// get the data length for this chunk
	UnsignedInt dataLength = 0;
	memcpy(&dataLength, data + i, sizeof(dataLength));
	i += sizeof(dataLength);
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readWrapperMessage - data length = %d", dataLength));

	UnsignedInt dataOffset = 0;
	memcpy(&dataOffset, data + i, sizeof(dataOffset));
	msg->setDataOffset(dataOffset);
	i += sizeof(dataOffset);
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readWrapperMessage - data offset = %d", dataOffset));

	msg->setData(data + i, dataLength);
	i += dataLength;

	return msg;
}

NetCommandMsg * NetPacket::readFileMessage(UnsignedByte *data, Int &i) {
	NetFileCommandMsg *msg = newInstance(NetFileCommandMsg);
	char filename[_MAX_PATH];

	// TheSuperHackers @security Mauller/Jbremer/SkyAero 11/12/2025 Prevent buffer overflow when copying filepath string
	i += strlcpy(filename, reinterpret_cast<const char*>(data + i), ARRAY_SIZE(filename));
	++i; //Increment for null terminator
	msg->setPortableFilename(AsciiString(filename));	// it's transferred as a portable filename

	UnsignedInt dataLength = 0;
	memcpy(&dataLength, data + i, sizeof(dataLength));
	i += sizeof(dataLength);

	UnsignedByte *buf = NEW UnsignedByte[dataLength];
	memcpy(buf, data + i, dataLength);
	i += dataLength;

	msg->setFileData(buf, dataLength);

	return msg;
}

NetCommandMsg * NetPacket::readFileAnnounceMessage(UnsignedByte *data, Int &i) {
	NetFileAnnounceCommandMsg *msg = newInstance(NetFileAnnounceCommandMsg);
	char filename[_MAX_PATH];

	// TheSuperHackers @security Mauller/Jbremer/SkyAero 11/12/2025 Prevent buffer overflow when copying filepath string
	i += strlcpy(filename, reinterpret_cast<const char*>(data + i), ARRAY_SIZE(filename));
	++i; //Increment for null terminator
	msg->setPortableFilename(AsciiString(filename));	// it's transferred as a portable filename

	UnsignedShort fileID = 0;
	memcpy(&fileID, data + i, sizeof(fileID));
	i += sizeof(fileID);
	msg->setFileID(fileID);

	UnsignedByte playerMask = 0;
	memcpy(&playerMask, data + i, sizeof(playerMask));
	i += sizeof(playerMask);
	msg->setPlayerMask(playerMask);

	return msg;
}

NetCommandMsg * NetPacket::readFileProgressMessage(UnsignedByte *data, Int &i) {
	NetFileProgressCommandMsg *msg = newInstance(NetFileProgressCommandMsg);

	UnsignedShort fileID = 0;
	memcpy(&fileID, data + i, sizeof(fileID));
	i += sizeof(fileID);
	msg->setFileID(fileID);

	Int progress = 0;
	memcpy(&progress, data + i, sizeof(progress));
	i += sizeof(progress);
	msg->setProgress(progress);

	return msg;
}

NetCommandMsg * NetPacket::readDisconnectFrameMessage(UnsignedByte *data, Int &i) {
	NetDisconnectFrameCommandMsg *msg = newInstance(NetDisconnectFrameCommandMsg);

	UnsignedInt disconnectFrame = 0;
	memcpy(&disconnectFrame, data + i, sizeof(disconnectFrame));
	i += sizeof(disconnectFrame);
	msg->setDisconnectFrame(disconnectFrame);

	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::readDisconnectFrameMessage - read disconnect frame for frame %d", disconnectFrame));

	return msg;
}

NetCommandMsg * NetPacket::readDisconnectScreenOffMessage(UnsignedByte *data, Int &i) {
	NetDisconnectScreenOffCommandMsg *msg = newInstance(NetDisconnectScreenOffCommandMsg);

	UnsignedInt newFrame = 0;
	memcpy(&newFrame, data + i, sizeof(newFrame));
	i += sizeof(newFrame);
	msg->setNewFrame(newFrame);

	return msg;
}

NetCommandMsg * NetPacket::readFrameResendRequestMessage(UnsignedByte *data, Int &i) {
	NetFrameResendRequestCommandMsg *msg = newInstance(NetFrameResendRequestCommandMsg);

	UnsignedInt frameToResend = 0;
	memcpy(&frameToResend, data + i, sizeof(frameToResend));
	i += sizeof(frameToResend);
	msg->setFrameToResend(frameToResend);

	return msg;
}

/**
 * Returns the number of commands in this packet.  Only valid if the packet is locally constructed.
 */
Int NetPacket::getNumCommands() {
	return m_numCommands;
}

/**
 * Returns the address that this packet is to be sent to.  Only valid if the packet is locally constructed.
 */
UnsignedInt NetPacket::getAddr() {
	return m_addr;
}

/**
 * Returns the port that this packet is to be sent to.  Only valid if the packet is locally constructed.
 */
UnsignedShort NetPacket::getPort() {
	return m_port;
}

/**
 * Returns the data of this packet.
 */
UnsignedByte * NetPacket::getData() {
	return m_packet;
}

/**
 * Returns the length of the packet.
 */
Int NetPacket::getLength() {
	return m_packetLen;
}

/**
 * Dumps the packet to the debug log file
 */
void NetPacket::dumpPacketToLog() {
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::dumpPacketToLog() - packet is %d bytes", m_packetLen));
	Int numLines = m_packetLen / 8;
	if ((m_packetLen % 8) != 0) {
		++numLines;
	}
	for (Int dumpindex = 0; dumpindex < numLines; ++dumpindex) {
		DEBUG_LOG_LEVEL_RAW(DEBUG_LEVEL_NET, ("\t%d\t", dumpindex*8));
		for (Int dumpindex2 = 0; (dumpindex2 < 8) && ((dumpindex*8 + dumpindex2) < m_packetLen); ++dumpindex2) {
			DEBUG_LOG_LEVEL_RAW(DEBUG_LEVEL_NET, ("%02x '%c' ", m_packet[dumpindex*8 + dumpindex2], m_packet[dumpindex*8 + dumpindex2]));
		}
		DEBUG_LOG_LEVEL_RAW(DEBUG_LEVEL_NET, ("\n"));
	}
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("End of packet dump"));
}
