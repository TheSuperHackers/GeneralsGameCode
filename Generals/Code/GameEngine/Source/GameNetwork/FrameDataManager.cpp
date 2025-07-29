/*
**	Command & Conquer Generals(tm)
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


#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/FrameDataManager.h"
#include "GameNetwork/networkutil.h"

/**
 * Constructor.  isLocal tells it whether its the frame data manager for the local player or not.
 */
FrameDataManager::FrameDataManager(Bool isLocal) {
	m_isLocal = isLocal;
	
	m_frameData = NEW FrameData[FRAME_DATA_LENGTH];

	m_isQuitting = FALSE;
	m_quitFrame = 0;
}

/**
 * destructor.
 */
FrameDataManager::~FrameDataManager() {
	for (Int i = 0; i < FRAME_DATA_LENGTH; ++i) {
		m_frameData[i].reset();
	}

	if (m_frameData)
	{
		delete[] m_frameData;
		m_frameData = NULL;
	}
}

/**
 * Initialize all of the frame datas associated with this manager.
 */
void FrameDataManager::init() {
	for (Int i = 0; i < FRAME_DATA_LENGTH; ++i) {
		m_frameData[i].init();
		if (m_isLocal) {
			// If this is the local connection, adjust the frame command count.
			m_frameData[i].setFrameCommandCount(m_frameData[i].getCommandCount());
		}
	}

	m_isQuitting = FALSE;
	m_quitFrame = 0;
}

/**
 * Reset the state of all the frames.
 */
void FrameDataManager::reset() {
	init();
}

/**
 * update function. Does nothing at this time.
 */
void FrameDataManager::update() {
}

/**
 * Add a network command to the appropriate frame.
 */
void FrameDataManager::addNetCommandMsg(NetCommandMsg *msg) {
	UnsignedInt frame = msg->getExecutionFrame();
	UnsignedInt frameindex = frame % FRAME_DATA_LENGTH;
	DEBUG_LOG_LEVEL(DEBUG_LEVEL_NET, ("FrameDataManager::addNetCommandMsg - about to add a command of type %s for frame %d, frame index %d", GetAsciiNetCommandType(msg->getNetCommandType()).str(), frame, frameindex));
	m_frameData[frameindex].addCommand(msg);

	if (m_isLocal) {
		// If this is the local connection, adjust the frame command count.
		m_frameData[frameindex].setFrameCommandCount(m_frameData[frameindex].getCommandCount());
	}
}

/**
 * Returns true if all the commands for the given frame are ready.
 */
FrameDataReturnType FrameDataManager::allCommandsReady(UnsignedInt frame, Bool debugSpewage) {
	UnsignedInt frameindex = frame % FRAME_DATA_LENGTH;
	//DEBUG_ASSERTCRASH(m_frameData[frameindex].getFrame() == frame || frame == 256, ("Looking at old commands!"));
	return m_frameData[frameindex].allCommandsReady(debugSpewage);
}

/**
 * Returns the command list for the given frame.
 */
NetCommandList * FrameDataManager::getFrameCommandList(UnsignedInt frame) {
	UnsignedInt frameindex = frame % FRAME_DATA_LENGTH;
	return m_frameData[frameindex].getCommandList();
}

/**
 * Reset the contents of the given frame.
 */
void FrameDataManager::resetFrame(UnsignedInt frame, Bool isAdvancing) {
	UnsignedInt frameindex = frame % FRAME_DATA_LENGTH;

	m_frameData[frameindex].reset();

	if (isAdvancing) {
		m_frameData[frameindex].setFrame(frame + MAX_FRAMES_AHEAD);
	}

	if (m_isLocal) {
		m_frameData[frameindex].setFrameCommandCount(m_frameData[frameindex].getCommandCount());
	}

	DEBUG_ASSERTCRASH(m_frameData[frameindex].getCommandCount() == 0, ("we just reset the frame data and the command count is not zero, huh?"));
}

/**
 * Returns the command count for the given frame.
 */
UnsignedInt FrameDataManager::getCommandCount(UnsignedInt frame) {
	UnsignedInt frameindex = frame % FRAME_DATA_LENGTH;

	return m_frameData[frameindex].getCommandCount();
}

/**
 * Set the frame command count for the given frame.
 */
void FrameDataManager::setFrameCommandCount(UnsignedInt frame, UnsignedInt commandCount) {
	UnsignedInt frameindex = frame % FRAME_DATA_LENGTH;

	m_frameData[frameindex].setFrameCommandCount(commandCount);
}

/**
 *
 */
UnsignedInt FrameDataManager::getFrameCommandCount(UnsignedInt frame) {
	UnsignedInt frameindex = frame % FRAME_DATA_LENGTH;

	return m_frameData[frameindex].getFrameCommandCount();
}

/**
 * Set both the command count and the frame command count to 0 for the given frames.
 */
void FrameDataManager::zeroFrames(UnsignedInt startingFrame, UnsignedInt numFrames) {
	UnsignedInt frameIndex = startingFrame % FRAME_DATA_LENGTH;
	for (UnsignedInt i = 0; i < numFrames; ++i) {
		//DEBUG_LOG(("Calling zeroFrame for frame index %d", frameIndex));
		m_frameData[frameIndex].zeroFrame();
		++frameIndex;
		frameIndex = frameIndex % FRAME_DATA_LENGTH;
	}
}

/**
 * Destroy all the commands held by this object.
 */
void FrameDataManager::destroyGameMessages() {
	for (Int i = 0; i < FRAME_DATA_LENGTH; ++i) {
		m_frameData[i].destroyGameMessages();
	}
}

/**
 * Sets the quit frame, also sets the isQuitting flag.
 */
void FrameDataManager::setQuitFrame(UnsignedInt frame) {
	m_isQuitting = TRUE;
	m_quitFrame = frame;
}

/**
 * returns the quit frame.
 */
UnsignedInt FrameDataManager::getQuitFrame() {
	return m_quitFrame;
}

/**
 * returns true if this frame data manager is quitting.
 */
Bool FrameDataManager::getIsQuitting() {
	return m_isQuitting;
}
