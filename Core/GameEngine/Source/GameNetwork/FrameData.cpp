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


#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameNetwork/FrameData.h"
#include "GameNetwork/networkutil.h"

/**
 * Constructor
 */
FrameData::FrameData()
{
	m_frame = 0;
	m_commandList = NULL;
	m_commandCount = 0;
	m_frameCommandCount = -1;
	//Added By Sadullah Nader
	//Initializations missing and needed
	m_lastFailedCC = 0;
	m_lastFailedFrameCC = 0;
	//
}

/**
 * Destructor
 */
FrameData::~FrameData()
{
	deleteInstance(m_commandList);
	m_commandList = NULL;
}

/**
 * Initialize this thing.
 */
void FrameData::init()
{
	m_frame = 0;
	if (m_commandList == NULL) {
		m_commandList = newInstance(NetCommandList);
		m_commandList->init();
	}
	m_commandList->reset();

	m_frameCommandCount = -1;
	//DEBUG_LOG(("FrameData::init"));
	m_commandCount = 0;
	m_lastFailedCC = -2;
	m_lastFailedFrameCC = -2;
}

/**
 * Reset this thing.
 */
void FrameData::reset() {
	init();
}

/**
 * update the thing, doesn't do anything at the moment.
 */
void FrameData::update() {
}

/**
 * return the frame number this frame data is associated with.
 */
UnsignedInt FrameData::getFrame() {
	return m_frame;
}

/**
 * Assign the frame number this frame data is associated with.
 */
void FrameData::setFrame(UnsignedInt frame) {
	m_frame = frame;
}

/**
 * Returns true if all the frame command count is equal to the number of commands that have been received.
 */
FrameDataReturnType FrameData::allCommandsReady(Bool debugSpewage) {
	if (m_frameCommandCount == m_commandCount) {
		m_lastFailedFrameCC = -2;
		m_lastFailedCC = -2;
		return FRAMEDATA_READY;
	}

	if (debugSpewage) {
		if ((m_lastFailedFrameCC != m_frameCommandCount) || (m_lastFailedCC != m_commandCount)) {
			DEBUG_LOG(("FrameData::allCommandsReady - failed, frame command count = %d, command count = %d", m_frameCommandCount, m_commandCount));
			m_lastFailedFrameCC = m_frameCommandCount;
			m_lastFailedCC = m_commandCount;
		}
	}

	if (m_commandCount > m_frameCommandCount) {
		DEBUG_LOG(("FrameData::allCommandsReady - There are more commands than there should be (%d, should be %d).  Commands in command list are...", m_commandCount, m_frameCommandCount));
		NetCommandRef *ref = m_commandList->getFirstMessage();
		while (ref != NULL) {
			DEBUG_LOG(("%s, frame = %d, id = %d", GetNetCommandTypeAsString(ref->getCommand()->getNetCommandType()), ref->getCommand()->getExecutionFrame(), ref->getCommand()->getID()));
			ref = ref->getNext();
		}
		DEBUG_LOG(("FrameData::allCommandsReady - End of command list."));
		DEBUG_LOG(("FrameData::allCommandsReady - about to clear the command list"));
		reset();
		DEBUG_LOG(("FrameData::allCommandsReady - command list cleared. command list length = %d, command count = %d, frame command count = %d", m_commandList->length(), m_commandCount, m_frameCommandCount));
		return FRAMEDATA_RESEND;
	}
	return FRAMEDATA_NOTREADY;
}

/**
 * Set the command count for this frame
 */
void FrameData::setFrameCommandCount(UnsignedInt frameCommandCount) {
	//DEBUG_LOG(("setFrameCommandCount to %d for frame %d", frameCommandCount, m_frame));
	m_frameCommandCount = frameCommandCount;
}

/**
 * Get the command count for this frame.
 */
UnsignedInt FrameData::getFrameCommandCount() {
	return m_frameCommandCount;
}

/**
 * return the number of commands received so far.
 */
UnsignedInt FrameData::getCommandCount() {
	return m_commandCount;
}

/**
 * Add a command to this frame
 */
void FrameData::addCommand(NetCommandMsg *msg) {
	// need to add the message in order of command ID
	if (m_commandList == NULL) {
		init();
	}

	// We don't need to worry about setting the relay since its not getting sent anywhere.
	if (m_commandList->findMessage(msg) != NULL) {
		// We don't want to add the same command twice.
		return;
	}
	m_commandList->addMessage(msg);

	++m_commandCount;
	//DEBUG_LOG(("added command %d, type = %d(%s), command count = %d, frame command count = %d", msg->getID(), msg->getNetCommandType(), GetNetCommandTypeAsString(msg->getNetCommandType()), m_commandCount, m_frameCommandCount));
}

/**
 * Return the list of commands for this frame
 */
NetCommandList * FrameData::getCommandList() {
	return m_commandList;
}

/**
 * Set both the command count and the frame command count to 0.
 */
void FrameData::zeroFrame() {
	m_commandCount = 0;
	m_frameCommandCount = 0;
}

/**
 * destroy all the commands in this frame.
 */
void FrameData::destroyGameMessages() {
	if (m_commandList == NULL) {
		return;
	}

	m_commandList->reset();
	m_commandCount = 0;
}
