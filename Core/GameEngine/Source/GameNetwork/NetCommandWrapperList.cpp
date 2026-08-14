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

////// NetCommandWrapperList.cpp ////////////////////////////////
// Bryan Cleveland

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameNetwork/NetCommandWrapperList.h"
#include "GameNetwork/NetCommandValidation.h"
#include "GameNetwork/NetPacket.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////// NetCommandWrapperListNode ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

NetCommandWrapperListNode::NetCommandWrapperListNode(NetWrapperCommandMsg *msg)
{
	m_next = nullptr;
	m_playerID = msg->getPlayerID();
	m_numChunks = msg->getNumChunks();
	m_chunksPresent = NEW Bool[m_numChunks];	// pool[]ify
	m_chunkOffsets = NEW UnsignedInt[m_numChunks];	// pool[]ify
	m_chunkLengths = NEW UnsignedInt[m_numChunks];	// pool[]ify
	m_numChunksPresent = 0;
	m_lastChunkTime = timeGetTime();

	for (UnsignedInt i = 0; i < m_numChunks; ++i) {
		m_chunksPresent[i] = FALSE;
		m_chunkOffsets[i] = 0;
		m_chunkLengths[i] = 0;
	}

	m_totalDataLength = msg->getTotalDataLength();
	m_data = NEW UnsignedByte[m_totalDataLength];	// pool[]ify
	m_allocationSize = sizeof(NetCommandWrapperListNode) +
		GetWrappedCommandAllocationSize(m_totalDataLength, m_numChunks);

	m_commandID = msg->getWrappedCommandID();
}

NetCommandWrapperListNode::~NetCommandWrapperListNode() {
	delete[] m_chunksPresent;
	m_chunksPresent = nullptr;
	delete[] m_chunkOffsets;
	m_chunkOffsets = nullptr;
	delete[] m_chunkLengths;
	m_chunkLengths = nullptr;

	delete[] m_data;
	m_data = nullptr;
}

Bool NetCommandWrapperListNode::isComplete() {
	return hasAllChunks() && IsCompleteWrappedCommandLayout(m_chunkOffsets, m_chunkLengths, m_numChunks, m_totalDataLength);
}

Bool NetCommandWrapperListNode::hasAllChunks() const {
	return m_numChunks > 0 && m_numChunksPresent == m_numChunks;
}

Bool NetCommandWrapperListNode::isCompatible(const NetWrapperCommandMsg *msg) const {
	return msg != nullptr &&
		m_playerID == msg->getPlayerID() &&
		m_commandID == msg->getWrappedCommandID() &&
		m_numChunks == msg->getNumChunks() &&
		m_totalDataLength == msg->getTotalDataLength();
}

Int NetCommandWrapperListNode::getPercentComplete() {
	if (isComplete())
		return 100;
	else
		return min(99, REAL_TO_INT( ((Real)m_numChunksPresent)/((Real)m_numChunks)*100.0f ));
}

UnsignedShort NetCommandWrapperListNode::getCommandID() {
	return m_commandID;
}

UnsignedByte NetCommandWrapperListNode::getPlayerID() const {
	return m_playerID;
}

UnsignedInt NetCommandWrapperListNode::getRawDataLength() {
	return m_totalDataLength;
}

UnsignedInt NetCommandWrapperListNode::getAllocationSize() const {
	return m_allocationSize;
}

Bool NetCommandWrapperListNode::copyChunkData(NetWrapperCommandMsg *msg) {
	if (msg == nullptr) {
		DEBUG_CRASH(("Trying to copy data from a non-existent wrapper command message"));
		return false;
	}

	if (!isCompatible(msg)) {
		DEBUG_CRASH(("Wrapper command metadata changed during transfer"));
		return false;
	}

	UnsignedInt chunkNumber = msg->getChunkNumber();

	if (chunkNumber >= m_numChunks) {
		DEBUG_CRASH(("Data chunk %u exceeds the expected maximum of %u chunks", chunkNumber, m_numChunks));
		return false;
	}

	UnsignedInt chunkDataOffset = msg->getDataOffset();
	UnsignedInt chunkDataLength = msg->getDataLength();

	// Exact retransmissions are harmless, but accepting conflicting data for a
	// completed slot would allow one transfer to be assembled from two layouts.
	if (m_chunksPresent[chunkNumber] == TRUE) {
		if (m_chunkOffsets[chunkNumber] != chunkDataOffset ||
			m_chunkLengths[chunkNumber] != chunkDataLength ||
			memcmp(m_data + m_chunkOffsets[chunkNumber], msg->getData(), chunkDataLength) != 0)
		{
			DEBUG_LOG(("NetCommandWrapperListNode::copyChunkData - rejected conflicting duplicate chunk"));
			return false;
		}
		return true;
	}

	// TheSuperHackers @security Mauller 04/12/2025 Prevent out of bounds memory access
	if (chunkDataOffset >= m_totalDataLength) {
		DEBUG_CRASH(("Data chunk offset %u exceeds the total data length %u", chunkDataOffset, m_totalDataLength));
		return false;
	}

	if (chunkDataLength > MAX_PACKET_SIZE ) {
		DEBUG_CRASH(("Data Chunk size %u greater than max packet size %u", chunkDataLength, MAX_PACKET_SIZE));
		return false;
	}

	if (chunkDataLength > m_totalDataLength - chunkDataOffset) {
		DEBUG_CRASH(("Data chunk exceeds data array size"));
		return false;
	}

	DEBUG_LOG(("NetCommandWrapperListNode::copyChunkData() - copying chunk %u", chunkNumber));

	memcpy(m_data + chunkDataOffset, msg->getData(), chunkDataLength);

	m_chunkOffsets[chunkNumber] = chunkDataOffset;
	m_chunkLengths[chunkNumber] = chunkDataLength;
	m_chunksPresent[chunkNumber] = TRUE;
	++m_numChunksPresent;
	m_lastChunkTime = timeGetTime();
	return true;
}

Bool NetCommandWrapperListNode::isExpired(UnsignedInt now) const {
	return IsWrappedCommandExpired(now, m_lastChunkTime);
}

UnsignedByte * NetCommandWrapperListNode::getRawData() {
	return m_data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////// NetCommandWrapperList ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

NetCommandWrapperList::NetCommandWrapperList() {
	init();
}

NetCommandWrapperList::~NetCommandWrapperList() {
	NetCommandWrapperListNode *temp;
	while (m_list != nullptr) {
		temp = m_list->m_next;
		deleteInstance(m_list);
		m_list = temp;
	}
	m_allocatedBytes = 0;
	m_nodeCount = 0;
	memset(m_playerNodeCounts, 0, sizeof(m_playerNodeCounts));
}

void NetCommandWrapperList::init() {
	m_list = nullptr;
	m_allocatedBytes = 0;
	m_nodeCount = 0;
	memset(m_playerNodeCounts, 0, sizeof(m_playerNodeCounts));
}

void NetCommandWrapperList::reset() {
	NetCommandWrapperListNode *temp;
	while (m_list != nullptr) {
		temp = m_list->m_next;
		deleteInstance(m_list);
		m_list = temp;
	}
	m_allocatedBytes = 0;
	m_nodeCount = 0;
	memset(m_playerNodeCounts, 0, sizeof(m_playerNodeCounts));
}

Int NetCommandWrapperList::getPercentComplete(UnsignedByte playerID, UnsignedShort wrappedCommandID)
{
	NetCommandWrapperListNode *temp = m_list;

	while (temp != nullptr && (temp->getPlayerID() != playerID || temp->getCommandID() != wrappedCommandID)) {
		temp = temp->m_next;
	}

	if (!temp)
		return 0;

	return temp->getPercentComplete();
}

Bool NetCommandWrapperList::processWrapper(NetCommandRef *ref) {
	if (ref == nullptr || ref->getCommand() == nullptr)
		return false;

	NetWrapperCommandMsg *msg = (NetWrapperCommandMsg *)(ref->getCommand());
	WrappedCommandMetadata metadata;
	metadata.playerID = msg->getPlayerID();
	metadata.chunkNumber = msg->getChunkNumber();
	metadata.numChunks = msg->getNumChunks();
	metadata.totalDataLength = msg->getTotalDataLength();
	metadata.dataLength = msg->getDataLength();
	metadata.dataOffset = msg->getDataOffset();

	if (!IsValidWrappedCommandMetadata(metadata)) {
		DEBUG_LOG(("NetCommandWrapperList::processWrapper - rejected invalid wrapper metadata"));
		return false;
	}

	purgeExpired(timeGetTime());
	NetCommandWrapperListNode *temp = m_list;

	while (temp != nullptr && (temp->getPlayerID() != msg->getPlayerID() || temp->getCommandID() != msg->getWrappedCommandID())) {
		temp = temp->m_next;
	}

	// Metadata is immutable for the lifetime of an active transfer. Replacing a live
	// node here would let a peer force repeated large allocations. Expiry handles
	// legitimate command-ID reuse after an abandoned transfer.
	if (temp != nullptr && !temp->isCompatible(msg)) {
		DEBUG_LOG(("NetCommandWrapperList::processWrapper - rejected incompatible wrapper metadata"));
		return false;
	}

	if (temp == nullptr) {
		const UnsignedInt allocationSize = sizeof(NetCommandWrapperListNode) +
			GetWrappedCommandAllocationSize(metadata.totalDataLength, metadata.numChunks);
		if (!CanTrackWrappedCommand(m_nodeCount, m_playerNodeCounts[metadata.playerID],
			m_allocatedBytes, allocationSize)) {
			DEBUG_LOG(("NetCommandWrapperList::processWrapper - wrapper memory budget exhausted"));
			return false;
		}

		temp = newInstance(NetCommandWrapperListNode)(msg);
		temp->m_next = m_list;
		m_list = temp;
		m_allocatedBytes += temp->getAllocationSize();
		++m_nodeCount;
		++m_playerNodeCounts[metadata.playerID];
	}

	return temp->copyChunkData(msg);
}

NetCommandList * NetCommandWrapperList::getReadyCommands()
{
	purgeExpired(timeGetTime());
	NetCommandList *retlist = newInstance(NetCommandList);
	retlist->init();

	NetCommandWrapperListNode *temp = m_list;
	NetCommandWrapperListNode *next = nullptr;

	while (temp != nullptr) {
		next = temp->m_next;
		if (temp->hasAllChunks()) {
			NetCommandRef *msg = nullptr;
			if (temp->isComplete()) {
				msg = NetPacket::ConstructNetCommandMsgFromRawData(temp->getRawData(), temp->getRawDataLength());
			}
			else {
				DEBUG_LOG(("NetCommandWrapperList::getReadyCommands - discarded non-contiguous wrapped command"));
			}
			if (msg != nullptr && msg->getCommand() != nullptr) {
				NetCommandRef *ret = retlist->addMessage(msg->getCommand());
				ret->setRelay(msg->getRelay());
			}
			else {
				DEBUG_LOG(("NetCommandWrapperList::getReadyCommands - discarded malformed wrapped command"));
			}

			if (msg != nullptr) {
				deleteInstance(msg);
				msg = nullptr;
			}

			removeFromList(temp);
			temp = nullptr;
		}
		temp = next;
	}

	return retlist;
}

void NetCommandWrapperList::purgeExpired(UnsignedInt now) {
	NetCommandWrapperListNode *node = m_list;
	while (node != nullptr) {
		NetCommandWrapperListNode *next = node->m_next;
		if (node->isExpired(now))
			removeFromList(node);
		node = next;
	}
}

void NetCommandWrapperList::removeForPlayer(UnsignedByte playerID) {
	if (playerID >= MAX_SLOTS)
		return;

	NetCommandWrapperListNode *node = m_list;
	while (node != nullptr) {
		NetCommandWrapperListNode *next = node->m_next;
		if (node->getPlayerID() == playerID)
			removeFromList(node);
		node = next;
	}
}

void NetCommandWrapperList::removeFromList(NetCommandWrapperListNode *node) {
	if (node == nullptr) {
		return;
	}

	NetCommandWrapperListNode *temp = m_list;
	NetCommandWrapperListNode *prev = nullptr;

	while (temp != nullptr && temp != node) {
		prev = temp;
		temp = temp->m_next;
	}

	if (temp == nullptr) {
		return;
	}

	if (prev == nullptr) {
		m_list = temp->m_next;
	} else {
		prev->m_next = temp->m_next;
	}

	const UnsignedByte playerID = temp->getPlayerID();
	const UnsignedInt allocationSize = temp->getAllocationSize();
	DEBUG_ASSERTCRASH(m_allocatedBytes >= allocationSize, ("Invalid wrapper allocation accounting"));
	DEBUG_ASSERTCRASH(m_nodeCount > 0, ("Invalid wrapper node accounting"));
	DEBUG_ASSERTCRASH(playerID < MAX_SLOTS && m_playerNodeCounts[playerID] > 0,
		("Invalid per-player wrapper node accounting"));
	m_allocatedBytes -= min(m_allocatedBytes, allocationSize);
	if (m_nodeCount > 0)
		--m_nodeCount;
	if (playerID < MAX_SLOTS && m_playerNodeCounts[playerID] > 0)
		--m_playerNodeCounts[playerID];
	deleteInstance(temp);
}
