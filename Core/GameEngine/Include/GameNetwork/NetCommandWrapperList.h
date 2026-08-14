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

////// NetCommandWrapperList.h ////////////////////////////////
// Bryan Cleveland

#pragma once

#include "GameNetwork/NetCommandList.h"

class NetCommandWrapperListNode : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetCommandWrapperListNode, "NetCommandWrapperListNode")
public:
	NetCommandWrapperListNode(NetWrapperCommandMsg *msg);
	//virtual ~NetCommandWrapperListNode();

	Bool isComplete();
	Bool hasAllChunks() const;
	Bool isCompatible(const NetWrapperCommandMsg *msg) const;
	UnsignedShort getCommandID();
	UnsignedByte getPlayerID() const;
	UnsignedInt getRawDataLength();
	UnsignedInt getAllocationSize() const;
	Bool copyChunkData(NetWrapperCommandMsg *msg);
	Bool isExpired(UnsignedInt now) const;
	UnsignedByte * getRawData();

	Int getPercentComplete();

	NetCommandWrapperListNode *m_next;

protected:
	UnsignedShort m_commandID;
	UnsignedByte m_playerID;
	UnsignedByte *m_data;
	UnsignedInt m_totalDataLength;
	UnsignedInt m_allocationSize;
	Bool *m_chunksPresent;
	UnsignedInt *m_chunkOffsets;
	UnsignedInt *m_chunkLengths;
	UnsignedInt m_numChunks;
	UnsignedInt m_numChunksPresent;
	UnsignedInt m_lastChunkTime;

};

class NetCommandWrapperList : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetCommandWrapperList, "NetCommandWrapperList")
public:
	NetCommandWrapperList();
	//virtual ~NetCommandWrapperList();

	void init();
	void reset();

	Bool processWrapper(NetCommandRef *ref);
	NetCommandList * getReadyCommands();
	void purgeExpired(UnsignedInt now);
	void removeForPlayer(UnsignedByte playerID);
	UnsignedInt getNodeCount() const { return m_nodeCount; }
	UnsignedInt getAllocatedBytes() const { return m_allocatedBytes; }

	Int getPercentComplete(UnsignedByte playerID, UnsignedShort wrappedCommandID);

protected:
	void removeFromList(NetCommandWrapperListNode *node);

	NetCommandWrapperListNode *m_list;
	UnsignedInt m_allocatedBytes;
	UnsignedInt m_nodeCount;
	UnsignedInt m_playerNodeCounts[MAX_SLOTS];
};
