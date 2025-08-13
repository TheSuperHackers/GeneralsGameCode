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

//------------------------------------------------------------------------------------
// The CommandMsg class is a linked-list wrapper around GameMessage objects that
// are queued up to execute at a later time

#pragma once

#ifndef __NETCOMMANDMSG_H
#define __NETCOMMANDMSG_H

#include "Lib/BaseType.h"
#include "GameNetwork/NetworkDefs.h"
#include "Common/UnicodeString.h"

//-----------------------------------------------------------------------------
class NetCommandMsg : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetCommandMsg, "NetCommandMsg")
public:
	NetCommandMsg();
	//virtual ~NetCommandMsg();
	inline UnsignedInt GetTimestamp() { return m_timestamp; }
	inline void SetTimestamp(UnsignedInt timestamp) { m_timestamp = timestamp; }
	inline void setExecutionFrame(UnsignedInt frame) { m_executionFrame = frame; }
	inline void setPlayerID(UnsignedInt playerID) { m_playerID = playerID; }
	inline void setID(UnsignedShort id) { m_id = id; }
	inline UnsignedInt getExecutionFrame() { return m_executionFrame; }
	inline UnsignedInt getPlayerID() { return m_playerID; }
	inline UnsignedShort getID() { return m_id; }
	inline void setNetCommandType(NetCommandType type) { m_commandType = type; }
	inline NetCommandType getNetCommandType() { return m_commandType; }
	virtual Int getSortNumber();
	void attach();
	void detach();

protected:
	UnsignedInt m_timestamp;
	UnsignedInt m_executionFrame;
	UnsignedInt m_playerID;
	UnsignedShort m_id;

	NetCommandType m_commandType;
	Int m_referenceCount;
};


//-----------------------------------------------------------------------------
/**
 * The NetGameCommandMsg is the NetCommandMsg representation of a GameMessage
 */
class NetGameCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetGameCommandMsg, "NetGameCommandMsg")
public:
	NetGameCommandMsg();
	NetGameCommandMsg(GameMessage *msg);
	//virtual ~NetGameCommandMsg();

	GameMessage *constructGameMessage();
	void addArgument(const GameMessageArgumentDataType type, GameMessageArgumentType arg);
	void setGameMessageType(GameMessage::Type type);

protected:
	Int m_numArgs;
	Int m_argSize;
	GameMessage::Type m_type;
	GameMessageArgument *m_argList, *m_argTail;
};

//-----------------------------------------------------------------------------
/**
 * The NetAckBothCommandMsg is the NetCommandMsg representation of the combination of a
 * stage 1 ack and a stage 2 ack.
 */
class NetAckBothCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetAckBothCommandMsg, "NetAckBothCommandMsg")
public:
	NetAckBothCommandMsg(NetCommandMsg *msg);
	NetAckBothCommandMsg();
	//virtual ~NetAckBothCommandMsg();

	UnsignedShort getCommandID();
	void setCommandID(UnsignedShort commandID);
	UnsignedByte getOriginalPlayerID();
	void setOriginalPlayerID(UnsignedByte originalPlayerID);
	virtual Int getSortNumber();

protected:
	UnsignedShort m_commandID;
	UnsignedByte m_originalPlayerID;
};

//-----------------------------------------------------------------------------
/**
 * The NetAckStage1CommandMsg is the NetCommandMsg representation of an ack message for the initial
 * recipient.
 */
class NetAckStage1CommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetAckStage1CommandMsg, "NetAckStage1CommandMsg")
public:
	NetAckStage1CommandMsg(NetCommandMsg *msg);
	NetAckStage1CommandMsg();
	//virtual ~NetAckStage1CommandMsg();

	UnsignedShort getCommandID();
	void setCommandID(UnsignedShort commandID);
	UnsignedByte getOriginalPlayerID();
	void setOriginalPlayerID(UnsignedByte originalPlayerID);
	virtual Int getSortNumber();

protected:
	UnsignedShort m_commandID;
	UnsignedByte m_originalPlayerID;
};

//-----------------------------------------------------------------------------
/**
 * The NetAckStage2CommandMsg is the NetCommandMsg representation of an ack message for all eventual
 * recipients. (when this is returned, all the players in the relay mask have received the packet)
 */
class NetAckStage2CommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetAckStage2CommandMsg, "NetAckStage2CommandMsg")
public:
	NetAckStage2CommandMsg(NetCommandMsg *msg);
	NetAckStage2CommandMsg();
	//virtual ~NetAckStage2CommandMsg();

	UnsignedShort getCommandID();
	void setCommandID(UnsignedShort commandID);
	UnsignedByte getOriginalPlayerID();
	void setOriginalPlayerID(UnsignedByte originalPlayerID);
	virtual Int getSortNumber();

protected:
	UnsignedShort m_commandID;
	UnsignedByte m_originalPlayerID;
};

//-----------------------------------------------------------------------------
class NetFrameCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFrameCommandMsg, "NetFrameCommandMsg")
public:
	NetFrameCommandMsg();
	//virtual ~NetFrameCommandMsg();

	void setCommandCount(UnsignedShort commandCount);
	UnsignedShort getCommandCount();

protected:
	UnsignedShort m_commandCount;
};

//-----------------------------------------------------------------------------
class NetPlayerLeaveCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetPlayerLeaveCommandMsg, "NetPlayerLeaveCommandMsg")
public:
	NetPlayerLeaveCommandMsg();
	//virtual ~NetPlayerLeaveCommandMsg();

	UnsignedByte getLeavingPlayerID();
	void setLeavingPlayerID(UnsignedByte id);

protected:
	UnsignedByte m_leavingPlayerID;
};

//-----------------------------------------------------------------------------
class NetRunAheadMetricsCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetRunAheadMetricsCommandMsg, "NetRunAheadMetricsCommandMsg")
public:
	NetRunAheadMetricsCommandMsg();
	//virtual ~NetRunAheadMetricsCommandMsg();

	Real getAverageLatency();
	void setAverageLatency(Real avgLat);
	Int  getAverageFps();
	void setAverageFps(Int fps);

protected:
	Real m_averageLatency;
	Int  m_averageFps;
};

//-----------------------------------------------------------------------------
class NetRunAheadCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetRunAheadCommandMsg, "NetRunAheadCommandMsg")
public:
	NetRunAheadCommandMsg();
	//virtual ~NetRunAheadCommandMsg();

	UnsignedShort getRunAhead();
	void setRunAhead(UnsignedShort runAhead);

	UnsignedByte getFrameRate();
	void setFrameRate(UnsignedByte frameRate);

protected:
	UnsignedShort m_runAhead;
	UnsignedByte m_frameRate;
};

//-----------------------------------------------------------------------------
class NetDestroyPlayerCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDestroyPlayerCommandMsg, "NetDestroyPlayerCommandMsg")
public:
	NetDestroyPlayerCommandMsg();
	//virtual ~NetDestroyPlayerCommandMsg();

	UnsignedInt getPlayerIndex();
	void setPlayerIndex(UnsignedInt playerIndex);

protected:
	UnsignedInt m_playerIndex;
};

//-----------------------------------------------------------------------------
class NetKeepAliveCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetKeepAliveCommandMsg, "NetKeepAliveCommandMsg")
public:
	NetKeepAliveCommandMsg();
	//virtual ~NetKeepAliveCommandMsg();
};

//-----------------------------------------------------------------------------
class NetDisconnectKeepAliveCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectKeepAliveCommandMsg, "NetDisconnectKeepAliveCommandMsg")
public:
	NetDisconnectKeepAliveCommandMsg();
	//virtual ~NetDisconnectKeepAliveCommandMsg();
};

//-----------------------------------------------------------------------------
class NetDisconnectPlayerCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectPlayerCommandMsg, "NetDisconnectPlayerCommandMsg")
public:
	NetDisconnectPlayerCommandMsg();
	//virtual ~NetDisconnectPlayerCommandMsg();

	UnsignedByte getDisconnectSlot();
	void setDisconnectSlot(UnsignedByte slot);

	UnsignedInt getDisconnectFrame();
	void setDisconnectFrame(UnsignedInt frame);

protected:
	UnsignedByte m_disconnectSlot;
	UnsignedInt m_disconnectFrame;
};

//-----------------------------------------------------------------------------
class NetPacketRouterQueryCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetPacketRouterQueryCommandMsg, "NetPacketRouterQueryCommandMsg")
public:
	NetPacketRouterQueryCommandMsg();
	//virtual ~NetPacketRouterQueryCommandMsg();
};

//-----------------------------------------------------------------------------
class NetPacketRouterAckCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetPacketRouterAckCommandMsg, "NetPacketRouterAckCommandMsg")
public:
	NetPacketRouterAckCommandMsg();
	//virtual ~NetPacketRouterAckCommandMsg();
};

//-----------------------------------------------------------------------------
class NetDisconnectChatCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectChatCommandMsg, "NetDisconnectChatCommandMsg")
public:
	NetDisconnectChatCommandMsg();
	//virtual ~NetDisconnectChatCommandMsg();

	UnicodeString getText();
	void setText(UnicodeString text);

protected:
	UnicodeString m_text;
};

//-----------------------------------------------------------------------------
class NetChatCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetChatCommandMsg, "NetChatCommandMsg")
public:
	NetChatCommandMsg();
	//virtual ~NetChatCommandMsg();

	UnicodeString getText();
	void setText(UnicodeString text);

	Int getPlayerMask( void );
	void setPlayerMask( Int playerMask );

protected:
	UnicodeString m_text;
	Int m_playerMask;
};

//-----------------------------------------------------------------------------
class NetDisconnectVoteCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectVoteCommandMsg, "NetDisconnectVoteCommandMsg")
public:
	NetDisconnectVoteCommandMsg();
	//virtual ~NetDisconnectVoteCommandMsg();

	UnsignedByte getSlot();
	void setSlot(UnsignedByte slot);

	UnsignedInt getVoteFrame();
	void setVoteFrame(UnsignedInt voteFrame);

protected:
	UnsignedByte m_slot;
	UnsignedInt m_voteFrame;
};

//-----------------------------------------------------------------------------
class NetProgressCommandMsg: public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetProgressCommandMsg, "NetProgressCommandMsg")
public:
	NetProgressCommandMsg( void );
	//virtual ~NetProgressCommandMsg( void );

	UnsignedByte getPercentage();
	void setPercentage( UnsignedByte percent );
protected:
	UnsignedByte m_percent;
};

//-----------------------------------------------------------------------------
class NetWrapperCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetWrapperCommandMsg, "NetWrapperCommandMsg")
public:
	NetWrapperCommandMsg( void );
	//virtual ~NetWrapperCommandMsg();

	UnsignedByte * getData();
	void setData(UnsignedByte *data, UnsignedInt dataLength);

	UnsignedInt getChunkNumber();
	void setChunkNumber(UnsignedInt chunkNumber);

	UnsignedInt getNumChunks();
	void setNumChunks(UnsignedInt numChunks);

	UnsignedInt getDataLength();

	UnsignedInt getTotalDataLength();
	void setTotalDataLength(UnsignedInt totalDataLength);

	UnsignedInt getDataOffset();
	void setDataOffset(UnsignedInt offset);

	UnsignedShort getWrappedCommandID();
	void setWrappedCommandID(UnsignedShort wrappedCommandID);

private:
	UnsignedByte *m_data;
	// using UnsignedInt's so we can send around files of effectively unlimited size easily
	UnsignedInt m_dataLength;
	UnsignedInt m_dataOffset;
	UnsignedInt m_totalDataLength;
	UnsignedInt m_chunkNumber;
	UnsignedInt m_numChunks;
	UnsignedShort m_wrappedCommandID;
};

//-----------------------------------------------------------------------------
class NetFileCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFileCommandMsg, "NetFileCommandMsg")
public:
	NetFileCommandMsg();
	//virtual ~NetFileCommandMsg();

	AsciiString getRealFilename();
	void setRealFilename(AsciiString filename);

	AsciiString getPortableFilename() { return m_portableFilename; }
	void setPortableFilename(AsciiString filename) { m_portableFilename = filename; }

	UnsignedInt getFileLength();

	UnsignedByte * getFileData();
	void setFileData(UnsignedByte *data, UnsignedInt dataLength);

protected:
	AsciiString m_portableFilename;

	UnsignedByte *m_data;
	UnsignedInt m_dataLength;
};

//-----------------------------------------------------------------------------
class NetFileAnnounceCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFileAnnounceCommandMsg, "NetFileAnnounceCommandMsg")
public:
	NetFileAnnounceCommandMsg();
	//virtual ~NetFileAnnounceCommandMsg();

	AsciiString getRealFilename();
	void setRealFilename(AsciiString filename);

	AsciiString getPortableFilename() { return m_portableFilename; }
	void setPortableFilename(AsciiString filename) { m_portableFilename = filename; }

	UnsignedShort getFileID();
	void setFileID(UnsignedShort fileID);

	UnsignedByte getPlayerMask(void);
	void setPlayerMask(UnsignedByte playerMask);

protected:
	AsciiString m_portableFilename;
	UnsignedShort m_fileID;
	UnsignedByte m_playerMask;
};

//-----------------------------------------------------------------------------
class NetFileProgressCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFileProgressCommandMsg, "NetFileProgressCommandMsg")
public:
	NetFileProgressCommandMsg();
	//virtual ~NetFileProgressCommandMsg();

	UnsignedShort getFileID();
	void setFileID(UnsignedShort val);

	Int getProgress();
	void setProgress(Int val);

protected:
	UnsignedShort m_fileID;
	Int m_progress;
};

//-----------------------------------------------------------------------------
class NetDisconnectFrameCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectFrameCommandMsg, "NetDisconnectFrameCommandMsg")
public:
	NetDisconnectFrameCommandMsg();

	UnsignedInt getDisconnectFrame();
	void setDisconnectFrame(UnsignedInt disconnectFrame);

protected:
	UnsignedInt m_disconnectFrame;
};

//-----------------------------------------------------------------------------
class NetDisconnectScreenOffCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectScreenOffCommandMsg, "NetDisconnectScreenOffCommandMsg")
public:
	NetDisconnectScreenOffCommandMsg();

	UnsignedInt getNewFrame();
	void setNewFrame(UnsignedInt newFrame);

protected:
	UnsignedInt m_newFrame;
};

//-----------------------------------------------------------------------------
class NetFrameResendRequestCommandMsg : public NetCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFrameResendRequestCommandMsg, "NetFrameResendRequestCommandMsg")
public:
	NetFrameResendRequestCommandMsg();

	UnsignedInt getFrameToResend();
	void setFrameToResend(UnsignedInt frame);

protected:
	UnsignedInt m_frameToResend;
};
#endif
