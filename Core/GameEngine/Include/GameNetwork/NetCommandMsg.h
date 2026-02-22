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

#include "Lib/BaseType.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/NetPacketStructs.h"
#include "Common/UnicodeString.h"

class NetCommandRef;

//-----------------------------------------------------------------------------
class NetCommandMsg : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetCommandMsg, "NetCommandMsg")
public:
	typedef SmallNetPacketCommandBaseSelect Select;

	NetCommandMsg();
	//virtual ~NetCommandMsg();
	UnsignedInt GetTimestamp() { return m_timestamp; }
	void SetTimestamp(UnsignedInt timestamp) { m_timestamp = timestamp; }
	void setExecutionFrame(UnsignedInt frame) { m_executionFrame = frame; }
	void setPlayerID(UnsignedInt playerID) { m_playerID = playerID; }
	void setID(UnsignedShort id) { m_id = id; }
	UnsignedInt getExecutionFrame() const { return m_executionFrame; }
	UnsignedInt getPlayerID() const { return m_playerID; }
	UnsignedShort getID() const { return m_id; }
	void setNetCommandType(NetCommandType type) { m_commandType = type; }
	NetCommandType getNetCommandType() const { return m_commandType; }
	virtual Int getSortNumber();
	virtual size_t getSizeForNetPacket() const = 0;
	virtual size_t copyBytesForNetPacket(UnsignedByte* buffer, const NetCommandRef& ref) const = 0;
	virtual size_t getSizeForSmallNetPacket(const Select* select = nullptr) const = 0;
	virtual size_t copyBytesForSmallNetPacket(UnsignedByte* buffer, const NetCommandRef& ref, const Select* select = nullptr) const = 0;
	virtual Select getSmallNetPacketSelect() const = 0;
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
template<typename NetPacketType, typename SmallNetPacketType>
class NetCommandMsgT : public NetCommandMsg
{
	virtual size_t getSizeForNetPacket() const
	{
		return NetPacketType::getSize(*this);
	}

	virtual size_t copyBytesForNetPacket(UnsignedByte* buffer, const NetCommandRef& ref) const
	{
		return NetPacketType::copyBytes(buffer, ref);
	}

	virtual size_t getSizeForSmallNetPacket(const Select* select = nullptr) const
	{
		return SmallNetPacketType::getSize(*this, select);
	}

	virtual size_t copyBytesForSmallNetPacket(UnsignedByte* buffer, const NetCommandRef& ref, const Select* select = nullptr) const
	{
		return SmallNetPacketType::copyBytes(buffer, ref, select);
	}
};

//-----------------------------------------------------------------------------
/**
 * The NetGameCommandMsg is the NetCommandMsg representation of a GameMessage
 */
class NetGameCommandMsg : public NetCommandMsgT<NetPacketGameCommand, SmallNetPacketGameCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetGameCommandMsg, "NetGameCommandMsg")
public:
	NetGameCommandMsg();
	NetGameCommandMsg(GameMessage *msg);
	//virtual ~NetGameCommandMsg();

	GameMessage *constructGameMessage() const;
	void addArgument(const GameMessageArgumentDataType type, GameMessageArgumentType arg);
	void setGameMessageType(GameMessage::Type type);

	virtual Select getSmallNetPacketSelect() const;

protected:
	Int m_numArgs;
	Int m_argSize;
	GameMessage::Type m_type;
	GameMessageArgument *m_argList, *m_argTail;
};

//-----------------------------------------------------------------------------
/**
 * The NetAckCommandMsg is the base class for other ack command messages.
 */
class NetAckCommandMsg : public NetCommandMsgT<NetPacketAckCommand, SmallNetPacketAckCommand>
{
protected:
	NetAckCommandMsg(NetCommandMsg *msg)
	{
		m_commandID = msg->getID();
		m_originalPlayerID = msg->getPlayerID();
	}
	NetAckCommandMsg()
	{
		m_commandID = 0;
		m_originalPlayerID = 0;
	}

public:
	UnsignedShort getCommandID() const;
	void setCommandID(UnsignedShort commandID);
	UnsignedByte getOriginalPlayerID() const;
	void setOriginalPlayerID(UnsignedByte originalPlayerID);
	virtual Int getSortNumber() const;

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedShort m_commandID;
	UnsignedByte m_originalPlayerID;
};

//-----------------------------------------------------------------------------
/**
 * The NetAckBothCommandMsg is the NetCommandMsg representation of the combination of a
 * stage 1 ack and a stage 2 ack.
 */
class NetAckBothCommandMsg : public NetAckCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetAckBothCommandMsg, "NetAckBothCommandMsg")
public:
	NetAckBothCommandMsg(NetCommandMsg *msg);
	NetAckBothCommandMsg();
	//virtual ~NetAckBothCommandMsg();
};

//-----------------------------------------------------------------------------
/**
 * The NetAckStage1CommandMsg is the NetCommandMsg representation of an ack message for the initial
 * recipient.
 */
class NetAckStage1CommandMsg : public NetAckCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetAckStage1CommandMsg, "NetAckStage1CommandMsg")
public:
	NetAckStage1CommandMsg(NetCommandMsg *msg);
	NetAckStage1CommandMsg();
	//virtual ~NetAckStage1CommandMsg();
};

//-----------------------------------------------------------------------------
/**
 * The NetAckStage2CommandMsg is the NetCommandMsg representation of an ack message for all eventual
 * recipients. (when this is returned, all the players in the relay mask have received the packet)
 */
class NetAckStage2CommandMsg : public NetAckCommandMsg
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetAckStage2CommandMsg, "NetAckStage2CommandMsg")
public:
	NetAckStage2CommandMsg(NetCommandMsg *msg);
	NetAckStage2CommandMsg();
	//virtual ~NetAckStage2CommandMsg();
};

//-----------------------------------------------------------------------------
class NetFrameCommandMsg : public NetCommandMsgT<NetPacketFrameCommand, SmallNetPacketFrameCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFrameCommandMsg, "NetFrameCommandMsg")
public:
	NetFrameCommandMsg();
	//virtual ~NetFrameCommandMsg();

	void setCommandCount(UnsignedShort commandCount);
	UnsignedShort getCommandCount() const;

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedShort m_commandCount;
};

//-----------------------------------------------------------------------------
class NetPlayerLeaveCommandMsg : public NetCommandMsgT<NetPacketPlayerLeaveCommand, SmallNetPacketPlayerLeaveCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetPlayerLeaveCommandMsg, "NetPlayerLeaveCommandMsg")
public:
	NetPlayerLeaveCommandMsg();
	//virtual ~NetPlayerLeaveCommandMsg();

	UnsignedByte getLeavingPlayerID() const;
	void setLeavingPlayerID(UnsignedByte id);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedByte m_leavingPlayerID;
};

//-----------------------------------------------------------------------------
class NetRunAheadMetricsCommandMsg : public NetCommandMsgT<NetPacketRunAheadMetricsCommand, SmallNetPacketRunAheadMetricsCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetRunAheadMetricsCommandMsg, "NetRunAheadMetricsCommandMsg")
public:
	NetRunAheadMetricsCommandMsg();
	//virtual ~NetRunAheadMetricsCommandMsg();

	Real getAverageLatency() const;
	void setAverageLatency(Real avgLat);
	Int  getAverageFps() const;
	void setAverageFps(Int fps);

	virtual Select getSmallNetPacketSelect() const;

protected:
	Real m_averageLatency;
	Int  m_averageFps;
};

//-----------------------------------------------------------------------------
class NetRunAheadCommandMsg : public NetCommandMsgT<NetPacketRunAheadCommand, SmallNetPacketRunAheadCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetRunAheadCommandMsg, "NetRunAheadCommandMsg")
public:
	NetRunAheadCommandMsg();
	//virtual ~NetRunAheadCommandMsg();

	UnsignedShort getRunAhead() const;
	void setRunAhead(UnsignedShort runAhead);

	UnsignedByte getFrameRate() const;
	void setFrameRate(UnsignedByte frameRate);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedShort m_runAhead;
	UnsignedByte m_frameRate;
};

//-----------------------------------------------------------------------------
class NetDestroyPlayerCommandMsg : public NetCommandMsgT<NetPacketDestroyPlayerCommand, SmallNetPacketDestroyPlayerCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDestroyPlayerCommandMsg, "NetDestroyPlayerCommandMsg")
public:
	NetDestroyPlayerCommandMsg();
	//virtual ~NetDestroyPlayerCommandMsg();

	UnsignedInt getPlayerIndex() const;
	void setPlayerIndex(UnsignedInt playerIndex);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedInt m_playerIndex;
};

//-----------------------------------------------------------------------------
class NetKeepAliveCommandMsg : public NetCommandMsgT<NetPacketKeepAliveCommand, SmallNetPacketKeepAliveCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetKeepAliveCommandMsg, "NetKeepAliveCommandMsg")
public:
	NetKeepAliveCommandMsg();
	//virtual ~NetKeepAliveCommandMsg();

	virtual Select getSmallNetPacketSelect() const;
};

//-----------------------------------------------------------------------------
class NetDisconnectKeepAliveCommandMsg : public NetCommandMsgT<NetPacketDisconnectKeepAliveCommand, SmallNetPacketDisconnectKeepAliveCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectKeepAliveCommandMsg, "NetDisconnectKeepAliveCommandMsg")
public:
	NetDisconnectKeepAliveCommandMsg();
	//virtual ~NetDisconnectKeepAliveCommandMsg();

	virtual Select getSmallNetPacketSelect() const;
};

//-----------------------------------------------------------------------------
class NetDisconnectPlayerCommandMsg : public NetCommandMsgT<NetPacketDisconnectPlayerCommand, SmallNetPacketDisconnectPlayerCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectPlayerCommandMsg, "NetDisconnectPlayerCommandMsg")
public:
	NetDisconnectPlayerCommandMsg();
	//virtual ~NetDisconnectPlayerCommandMsg();

	UnsignedByte getDisconnectSlot() const;
	void setDisconnectSlot(UnsignedByte slot);

	UnsignedInt getDisconnectFrame() const;
	void setDisconnectFrame(UnsignedInt frame);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedByte m_disconnectSlot;
	UnsignedInt m_disconnectFrame;
};

//-----------------------------------------------------------------------------
class NetPacketRouterQueryCommandMsg : public NetCommandMsgT<NetPacketRouterQueryCommand, SmallNetPacketRouterQueryCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetPacketRouterQueryCommandMsg, "NetPacketRouterQueryCommandMsg")
public:
	NetPacketRouterQueryCommandMsg();
	//virtual ~NetPacketRouterQueryCommandMsg();

	virtual Select getSmallNetPacketSelect() const;
};

//-----------------------------------------------------------------------------
class NetPacketRouterAckCommandMsg : public NetCommandMsgT<NetPacketRouterAckCommand, SmallNetPacketRouterAckCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetPacketRouterAckCommandMsg, "NetPacketRouterAckCommandMsg")
public:
	NetPacketRouterAckCommandMsg();
	//virtual ~NetPacketRouterAckCommandMsg();

	virtual Select getSmallNetPacketSelect() const;
};

//-----------------------------------------------------------------------------
class NetDisconnectChatCommandMsg : public NetCommandMsgT<NetPacketDisconnectChatCommand, SmallNetPacketDisconnectChatCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectChatCommandMsg, "NetDisconnectChatCommandMsg")
public:
	NetDisconnectChatCommandMsg();
	//virtual ~NetDisconnectChatCommandMsg();

	UnicodeString getText() const;
	void setText(UnicodeString text);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnicodeString m_text;
};

//-----------------------------------------------------------------------------
class NetChatCommandMsg : public NetCommandMsgT<NetPacketChatCommand, SmallNetPacketChatCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetChatCommandMsg, "NetChatCommandMsg")
public:
	NetChatCommandMsg();
	//virtual ~NetChatCommandMsg();

	UnicodeString getText() const;
	void setText(UnicodeString text);

	Int getPlayerMask() const;
	void setPlayerMask( Int playerMask );

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnicodeString m_text;
	Int m_playerMask;
};

//-----------------------------------------------------------------------------
class NetDisconnectVoteCommandMsg : public NetCommandMsgT<NetPacketDisconnectVoteCommand, SmallNetPacketDisconnectVoteCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectVoteCommandMsg, "NetDisconnectVoteCommandMsg")
public:
	NetDisconnectVoteCommandMsg();
	//virtual ~NetDisconnectVoteCommandMsg();

	UnsignedByte getSlot() const;
	void setSlot(UnsignedByte slot);

	UnsignedInt getVoteFrame() const;
	void setVoteFrame(UnsignedInt voteFrame);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedByte m_slot;
	UnsignedInt m_voteFrame;
};

//-----------------------------------------------------------------------------
class NetProgressCommandMsg: public NetCommandMsgT<NetPacketProgressCommand, SmallNetPacketProgressCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetProgressCommandMsg, "NetProgressCommandMsg")
public:
	NetProgressCommandMsg();
	//virtual ~NetProgressCommandMsg();

	UnsignedByte getPercentage() const;
	void setPercentage( UnsignedByte percent );

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedByte m_percent;
};

//-----------------------------------------------------------------------------
class NetWrapperCommandMsg : public NetCommandMsgT<NetPacketWrapperCommand, SmallNetPacketWrapperCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetWrapperCommandMsg, "NetWrapperCommandMsg")
public:
	NetWrapperCommandMsg();
	//virtual ~NetWrapperCommandMsg();

	const UnsignedByte * getData() const;
	UnsignedByte * getData();
	void setData(UnsignedByte *data, UnsignedInt dataLength);

	UnsignedInt getChunkNumber() const;
	void setChunkNumber(UnsignedInt chunkNumber);

	UnsignedInt getNumChunks() const;
	void setNumChunks(UnsignedInt numChunks);

	UnsignedInt getDataLength() const;

	UnsignedInt getTotalDataLength() const;
	void setTotalDataLength(UnsignedInt totalDataLength);

	UnsignedInt getDataOffset() const;
	void setDataOffset(UnsignedInt offset);

	UnsignedShort getWrappedCommandID() const;
	void setWrappedCommandID(UnsignedShort wrappedCommandID);

	virtual Select getSmallNetPacketSelect() const;

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
class NetFileCommandMsg : public NetCommandMsgT<NetPacketFileCommand, SmallNetPacketFileCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFileCommandMsg, "NetFileCommandMsg")
public:
	NetFileCommandMsg();
	//virtual ~NetFileCommandMsg();

	AsciiString getRealFilename() const;
	void setRealFilename(AsciiString filename);

	AsciiString getPortableFilename() const { return m_portableFilename; }
	void setPortableFilename(AsciiString filename) { m_portableFilename = filename; }

	UnsignedInt getFileLength() const;

	const UnsignedByte * getFileData() const;
	UnsignedByte * getFileData();
	void setFileData(UnsignedByte *data, UnsignedInt dataLength);

	virtual Select getSmallNetPacketSelect() const;

protected:
	AsciiString m_portableFilename;

	UnsignedByte *m_data;
	UnsignedInt m_dataLength;
};

//-----------------------------------------------------------------------------
class NetFileAnnounceCommandMsg : public NetCommandMsgT<NetPacketFileAnnounceCommand, SmallNetPacketFileAnnounceCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFileAnnounceCommandMsg, "NetFileAnnounceCommandMsg")
public:
	NetFileAnnounceCommandMsg();
	//virtual ~NetFileAnnounceCommandMsg();

	AsciiString getRealFilename() const;
	void setRealFilename(AsciiString filename);

	AsciiString getPortableFilename() const { return m_portableFilename; }
	void setPortableFilename(AsciiString filename) { m_portableFilename = filename; }

	UnsignedShort getFileID() const;
	void setFileID(UnsignedShort fileID);

	UnsignedByte getPlayerMask() const;
	void setPlayerMask(UnsignedByte playerMask);

	virtual Select getSmallNetPacketSelect() const;

protected:
	AsciiString m_portableFilename;
	UnsignedShort m_fileID;
	UnsignedByte m_playerMask;
};

//-----------------------------------------------------------------------------
class NetFileProgressCommandMsg : public NetCommandMsgT<NetPacketFileProgressCommand, SmallNetPacketFileProgressCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFileProgressCommandMsg, "NetFileProgressCommandMsg")
public:
	NetFileProgressCommandMsg();
	//virtual ~NetFileProgressCommandMsg();

	UnsignedShort getFileID() const;
	void setFileID(UnsignedShort val);

	Int getProgress() const;
	void setProgress(Int val);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedShort m_fileID;
	Int m_progress;
};

//-----------------------------------------------------------------------------
class NetDisconnectFrameCommandMsg : public NetCommandMsgT<NetPacketDisconnectFrameCommand, SmallNetPacketDisconnectFrameCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectFrameCommandMsg, "NetDisconnectFrameCommandMsg")
public:
	NetDisconnectFrameCommandMsg();

	UnsignedInt getDisconnectFrame() const;
	void setDisconnectFrame(UnsignedInt disconnectFrame);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedInt m_disconnectFrame;
};

//-----------------------------------------------------------------------------
class NetDisconnectScreenOffCommandMsg : public NetCommandMsgT<NetPacketDisconnectScreenOffCommand, SmallNetPacketDisconnectScreenOffCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetDisconnectScreenOffCommandMsg, "NetDisconnectScreenOffCommandMsg")
public:
	NetDisconnectScreenOffCommandMsg();

	UnsignedInt getNewFrame() const;
	void setNewFrame(UnsignedInt newFrame);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedInt m_newFrame;
};

//-----------------------------------------------------------------------------
class NetFrameResendRequestCommandMsg : public NetCommandMsgT<NetPacketFrameResendRequestCommand, SmallNetPacketFrameResendRequestCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetFrameResendRequestCommandMsg, "NetFrameResendRequestCommandMsg")
public:
	NetFrameResendRequestCommandMsg();

	UnsignedInt getFrameToResend() const;
	void setFrameToResend(UnsignedInt frame);

	virtual Select getSmallNetPacketSelect() const;

protected:
	UnsignedInt m_frameToResend;
};

class NetLoadCompleteCommandMsg : public NetCommandMsgT<NetPacketLoadCompleteCommand, SmallNetPacketLoadCompleteCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetLoadCompleteCommandMsg, "NetLoadCompleteCommandMsg")
public:
	NetLoadCompleteCommandMsg();
	//virtual ~NetLoadCompleteCommandMsg();

	virtual Select getSmallNetPacketSelect() const;
};

class NetTimeOutGameStartCommandMsg : public NetCommandMsgT<NetPacketTimeOutGameStartCommand, SmallNetPacketTimeOutGameStartCommand>
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(NetTimeOutGameStartCommandMsg, "NetTimeOutGameStartCommandMsg")
public:
	NetTimeOutGameStartCommandMsg();
	//virtual ~NetTimeOutGameStartCommandMsg();

	virtual Select getSmallNetPacketSelect() const;
};
