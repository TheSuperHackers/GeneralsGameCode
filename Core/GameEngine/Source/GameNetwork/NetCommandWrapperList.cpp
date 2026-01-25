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
#include "GameNetwork/NetPacket.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
////// NetCommandWrapperListNode ///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

NetCommandWrapperListNode::NetCommandWrapperListNode(NetWrapperCommandMsg *msg)
{
	m_next = nullptr;
	m_numChunks = msg->getNumChunks();
	m_chunksPresent = NEW Bool[m_numChunks];	// pool[]ify
	m_numChunksPresent = 0;

	for (UnsignedInt i = 0; i < m_numChunks; ++i) {
		m_chunksPresent[i] = FALSE;
	}

	m_totalDataLength = msg->getTotalDataLength();
	m_data = NEW UnsignedByte[m_totalDataLength];	// pool[]ify

	m_commandID = msg->getWrappedCommandID();
}

NetCommandWrapperListNode::~NetCommandWrapperListNode() {
	delete[] m_chunksPresent;
	m_chunksPresent = nullptr;

	delete[] m_data;
	m_data = nullptr;
}

Bool NetCommandWrapperListNode::isComplete() {
	return m_numChunksPresent == m_numChunks;
}

Int NetCommandWrapperListNode::getPercentComplete(void) {
	if (isComplete())
		return 100;
	else
		return min(99, REAL_TO_INT( ((Real)m_numChunksPresent)/((Real)m_numChunks)*100.0f ));
}

UnsignedShort NetCommandWrapperListNode::getCommandID() {
	return m_commandID;
}

UnsignedInt NetCommandWrapperListNode::getRawDataLength() {
	return m_totalDataLength;
}

void NetCommandWrapperListNode::copyChunkData(NetWrapperCommandMsg *msg) {
	if (msg == nullptr) {
		DEBUG_CRASH(("Trying to copy data from a non-existent wrapper command message"));
		return;
	}

	UnsignedInt chunkNumber = msg->getChunkNumber();

	if (chunkNumber >= m_numChunks) {
		DEBUG_CRASH(("Data chunk %u exceeds the expected maximum of %u chunks", chunkNumber, m_numChunks));
		return;
	}

	// we already received this chunk, no need to recopy it.
	if (m_chunksPresent[chunkNumber] == TRUE) {
		return;
	}

	UnsignedInt chunkDataOffset = msg->getDataOffset();
	UnsignedInt chunkDataLength = msg->getDataLength();

	// TheSuperHackers @security Mauller 04/12/2025 Prevent out of bounds memory access
	if (chunkDataOffset >= m_totalDataLength) {
		DEBUG_CRASH(("Data chunk offset %u exceeds the total data length %u", chunkDataOffset, m_totalDataLength));
		return;
	}

	if (chunkDataLength > MAX_PACKET_SIZE ) {
		DEBUG_CRASH(("Data Chunk size %u greater than max packet size %u", chunkDataLength, MAX_PACKET_SIZE));
		return;
	}

	if (chunkDataOffset + chunkDataLength > m_totalDataLength) {
		DEBUG_CRASH(("Data chunk exceeds data array size"));
		return;
	}

	DEBUG_LOG(("NetCommandWrapperListNode::copyChunkData() - copying chunk %u", chunkNumber));

	memcpy(m_data + chunkDataOffset, msg->getData(), chunkDataLength);

	m_chunksPresent[chunkNumber] = TRUE;
	++m_numChunksPresent;
}

UnsignedByte * NetCommandWrapperListNode::getRawData() {
	return m_data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////// NetCommandWrapperList ///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

NetCommandWrapperList::NetCommandWrapperList() {
	m_list = nullptr;
}

NetCommandWrapperList::~NetCommandWrapperList() {
	NetCommandWrapperListNode *temp;
	while (m_list != nullptr) {
		temp = m_list->m_next;
		deleteInstance(m_list);
		m_list = temp;
	}
}

void NetCommandWrapperList::init() {
	m_list = nullptr;
}

void NetCommandWrapperList::reset() {
	NetCommandWrapperListNode *temp;
	while (m_list != nullptr) {
		temp = m_list->m_next;
		deleteInstance(m_list);
		m_list = temp;
	}
}

Int NetCommandWrapperList::getPercentComplete(UnsignedShort wrappedCommandID)
{
	NetCommandWrapperListNode *temp = m_list;

	while ((temp != nullptr) && (temp->getCommandID() != wrappedCommandID)) {
		temp = temp->m_next;
	}

	if (!temp)
		return 0;

	return temp->getPercentComplete();
}

void NetCommandWrapperList::processWrapper(NetCommandRef *ref) {
	NetCommandWrapperListNode *temp = m_list;
	NetWrapperCommandMsg *msg = (NetWrapperCommandMsg *)(ref->getCommand());

	while ((temp != nullptr) && (temp->getCommandID() != msg->getWrappedCommandID())) {
		temp = temp->m_next;
	}

	if (temp == nullptr) {
		temp = newInstance(NetCommandWrapperListNode)(msg);
		temp->m_next = m_list;
		m_list = temp;
	}

	temp->copyChunkData(msg);
}

NetCommandList * NetCommandWrapperList::getReadyCommands()
{
	NetCommandList *retlist = newInstance(NetCommandList);
	retlist->init();

	NetCommandWrapperListNode *temp = m_list;
	NetCommandWrapperListNode *next = nullptr;

	while (temp != nullptr) {
		next = temp->m_next;
		if (temp->isComplete()) {
			NetCommandRef *msg = NetPacket::ConstructNetCommandMsgFromRawData(temp->getRawData(), temp->getRawDataLength());
			NetCommandRef *ret = retlist->addMessage(msg->getCommand());
			ret->setRelay(msg->getRelay());

			deleteInstance(msg);
			msg = nullptr;

			removeFromList(temp);
			temp = nullptr;
		}
		temp = next;
	}

	return retlist;
}

void NetCommandWrapperList::removeFromList(NetCommandWrapperListNode *node) {
	if (node == nullptr) {
		return;
	}

	NetCommandWrapperListNode *temp = m_list;
	NetCommandWrapperListNode *prev = nullptr;

	while ((temp != nullptr) && (temp->getCommandID() != node->getCommandID())) {
		prev = temp;
		temp = temp->m_next;
	}

	if (temp == nullptr) {
		return;
	}

	if (prev == nullptr) {
		m_list = temp->m_next;
		deleteInstance(temp);
		temp = nullptr;
	} else {
		prev->m_next = temp->m_next;
		deleteInstance(temp);
		temp = nullptr;
	}
}
