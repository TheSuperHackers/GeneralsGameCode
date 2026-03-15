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


static size_t constructNetCommandRef(NetCommandRef *&ref, SmallNetPacketCommandBase::CommandBase &base, NetPacketBuf buf)
{
	size_t size = SmallNetPacketCommandBase::readMessage(ref, base, buf);

	if (ref != nullptr)
	{
		DEBUG_ASSERTCRASH(ref->getCommand() != nullptr, ("constructNetCommandRef: ref->getCommand() is null"));
		size += ref->getCommand()->readMessageData(*ref, buf.offset(size));
	}

	return size;
}

// This function assumes that all of the fields are either of default value or are
// present in the raw data.
NetCommandRef *NetPacket::ConstructNetCommandMsgFromRawData(const UnsignedByte *data, UnsignedInt dataLength) {
	SmallNetPacketCommandBase::CommandBase commandBase;
	commandBase.commandType.commandType = static_cast<UnsignedByte>(NETCOMMANDTYPE_GAMECOMMAND);
	commandBase.relay.relay = 0;
	commandBase.frame.frame = 0;
	commandBase.playerId.playerId = 0;
	commandBase.commandId.commandId = 0;

	NetPacketBuf buf(data, dataLength);
	NetCommandRef *ref = nullptr;
	constructNetCommandRef(ref, commandBase, buf);

	if (ref == nullptr)
	{
		DEBUG_CRASH(("Unrecognized packet entry, ignoring."));
		DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::ConstructNetCommandMsgFromRawData - Unrecognized packet"));
		dumpPacketToLog(data, dataLength);
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

		UnsignedInt dataSizeThisPacket = commandSizePerPacket;
		if ((bufferSize - bigPacketCurrentOffset) < dataSizeThisPacket) {
			dataSizeThisPacket = bufferSize - bigPacketCurrentOffset;
		}
		NetCommandDataChunk bigPacket(dataSizeThisPacket);
		memcpy(bigPacket.data(), bigPacketData + bigPacketCurrentOffset, bigPacket.size());

		if (DoesCommandRequireACommandID(wrapperMsg->getNetCommandType())) {
			wrapperMsg->setID(GenerateNextCommandID());
		}
		wrapperMsg->setPlayerID(msg->getPlayerID());
		wrapperMsg->setExecutionFrame(msg->getExecutionFrame());

		wrapperMsg->setChunkNumber(currentChunk);
		wrapperMsg->setNumChunks(numChunks);
		wrapperMsg->setDataOffset(bigPacketCurrentOffset);
		wrapperMsg->setData(bigPacket);
		wrapperMsg->setTotalDataLength(bufferSize);
		wrapperMsg->setWrappedCommandID(msg->getID());

		bigPacketCurrentOffset += dataSizeThisPacket;

		NetCommandRef *ref = NEW_NETCOMMANDREF(wrapperMsg);
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
		const UnsignedByte updateLastCommandId = select.useCommandId;

		select.useCommandType &= m_lastCommandType != cmdMsg->getNetCommandType();
		select.useRelay &= m_lastRelay != msg->getRelay();
		select.useFrame &= m_lastFrame != cmdMsg->getExecutionFrame();
		select.usePlayerId &= m_lastPlayerID != cmdMsg->getPlayerID();
		select.useCommandId &= ((m_lastCommandID + 1) != cmdMsg->getID()) | select.usePlayerId;

		const size_t msglen = cmdMsg->getSizeForSmallNetPacket(&select);

		// Is there enough room in the packet for this message?
		if (msglen > (MAX_PACKET_SIZE - m_packetLen)) {
			return FALSE;
		}

		if (select.useCommandType)
			m_lastCommandType = cmdMsg->getNetCommandType();

		if (select.useRelay)
			m_lastRelay = msg->getRelay();

		if (select.useFrame)
			m_lastFrame = cmdMsg->getExecutionFrame();

		if (select.usePlayerId)
			m_lastPlayerID = cmdMsg->getPlayerID();

		if (updateLastCommandId)
			m_lastCommandID = cmdMsg->getID();

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
	retval->init();

	// These need to be the same as the default values for m_lastPlayerID, m_lastFrame, etc.
	SmallNetPacketCommandBase::CommandBase commandBase;
	commandBase.commandType.commandType = 0;
	commandBase.relay.relay = 0;
	commandBase.frame.frame = 0;
	commandBase.playerId.playerId = 0;
	commandBase.commandId.commandId = 1; // The first command is going to be

	NetCommandRef *lastCommand = nullptr;

	Int i = 0;
	NetPacketBuf buf(m_packet, m_packetLen);

	while (i < buf.size())
	{
		const Bool isRepeat = m_packet[i] == NetPacketFieldTypes::Repeat;

		if (!isRepeat)
		{
			NetCommandRef *ref = nullptr;
			i += constructNetCommandRef(ref, commandBase, buf.offset(i));

			if (ref == nullptr)
			{
				DEBUG_CRASH(("Unrecognized packet entry, ignoring."));
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::getCommandList - Unrecognized packet entry at index %d", i));
				dumpPacketToLog(m_packet, m_packetLen);
				break;
			}

			// increment to the next command ID.
			if (DoesCommandRequireACommandID((NetCommandType)commandBase.commandType.commandType)) {
				++commandBase.commandId.commandId;
			}

			NetCommandMsg *msg = ref->getCommand();
			msg->attach();

			// add the message to the list.
			retval->addMessage(ref);

			deleteInstance(lastCommand);
			lastCommand = NEW_NETCOMMANDREF(msg);
			msg->detach();
		}
		else
		{
			i += NetPacketRepeatCommand::getSize();

			// Repeat the last command, doing some funky cool byte-saving stuff
			if (lastCommand == nullptr) {
				DEBUG_CRASH(("Got a repeat command with no command to repeat."));
			}

			NetCommandMsg *msg = nullptr;

			switch (commandBase.commandType.commandType)
			{
			case NETCOMMANDTYPE_ACKSTAGE1: {
				msg = newInstance(NetAckStage1CommandMsg)();
				NetAckStage1CommandMsg* laststageone = static_cast<NetAckStage1CommandMsg*>(lastCommand->getCommand());
				((NetAckStage1CommandMsg*)msg)->setCommandID(laststageone->getCommandID() + 1);
				((NetAckStage1CommandMsg*)msg)->setOriginalPlayerID(laststageone->getOriginalPlayerID());
				break;
			}
			case NETCOMMANDTYPE_ACKSTAGE2: {
				msg = newInstance(NetAckStage2CommandMsg)();
				NetAckStage2CommandMsg* laststagetwo = static_cast<NetAckStage2CommandMsg*>(lastCommand->getCommand());
				((NetAckStage2CommandMsg*)msg)->setCommandID(laststagetwo->getCommandID() + 1);
				((NetAckStage2CommandMsg*)msg)->setOriginalPlayerID(laststagetwo->getOriginalPlayerID());
				break;
			}
			case NETCOMMANDTYPE_ACKBOTH: {
				msg = newInstance(NetAckBothCommandMsg)();
				NetAckBothCommandMsg* lastboth = static_cast<NetAckBothCommandMsg*>(lastCommand->getCommand());
				((NetAckBothCommandMsg*)msg)->setCommandID(lastboth->getCommandID() + 1);
				((NetAckBothCommandMsg*)msg)->setOriginalPlayerID(lastboth->getOriginalPlayerID());
				break;
			}
			case NETCOMMANDTYPE_FRAMEINFO: {
				msg = newInstance(NetFrameCommandMsg)();
				++commandBase.frame.frame; // this is set below.
				((NetFrameCommandMsg*)msg)->setCommandCount(0);
				DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("Read a repeated frame command, frame = %d, playerId = %d, commandId = %d",
					commandBase.frame.frame, commandBase.playerId.playerId, commandBase.commandId.commandId));
				break;
			}
			default:
				DEBUG_CRASH(("Trying to repeat a command that shouldn't be repeated."));
				continue;
			}

			msg->setExecutionFrame(commandBase.frame.frame);
			msg->setPlayerID(commandBase.playerId.playerId);
			msg->setNetCommandType((NetCommandType)commandBase.commandType.commandType);
			msg->setID(commandBase.commandId.commandId);

			// increment to the next command ID.
			if (DoesCommandRequireACommandID((NetCommandType)commandBase.commandType.commandType)) {
				++commandBase.commandId.commandId;
			}

			// add the message to the list.
			NetCommandRef *ref = retval->addMessage(msg);
			if (ref != nullptr) {
				ref->setRelay(commandBase.relay.relay);
			}

			deleteInstance(lastCommand);
			lastCommand = NEW_NETCOMMANDREF(msg);
			msg->detach();
		}
	}

	deleteInstance(lastCommand);

	return retval;
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
void NetPacket::dumpPacketToLog(const UnsignedByte *packet, Int packetLen) {
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("NetPacket::dumpPacketToLog() - packet is %d bytes", packetLen));
	Int numLines = packetLen / 8;
	if ((packetLen % 8) != 0) {
		++numLines;
	}
	for (Int dumpindex = 0; dumpindex < numLines; ++dumpindex) {
		DEBUG_LOG_LEVEL_RAW(DEBUG_LEVEL_NET, ("\t%d\t", dumpindex*8));
		for (Int dumpindex2 = 0; (dumpindex2 < 8) && ((dumpindex*8 + dumpindex2) < packetLen); ++dumpindex2) {
			DEBUG_LOG_LEVEL_RAW(DEBUG_LEVEL_NET, ("%02x '%c' ", packet[dumpindex*8 + dumpindex2], packet[dumpindex*8 + dumpindex2]));
		}
		DEBUG_LOG_LEVEL_RAW(DEBUG_LEVEL_NET, ("\n"));
	}
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("End of packet dump"));
}
