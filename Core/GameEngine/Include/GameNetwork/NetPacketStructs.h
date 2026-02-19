/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#pragma once

#include "GameNetwork/NetworkDefs.h"

class AsciiString;
class UnicodeString;
class GameMessage;

class NetCommandRef;
class NetCommandMsg;
class NetGameCommandMsg;
class NetAckCommandMsg;
class NetAckBothCommandMsg;
class NetAckStage1CommandMsg;
class NetAckStage2CommandMsg;
class NetFrameCommandMsg;
class NetPlayerLeaveCommandMsg;
class NetRunAheadMetricsCommandMsg;
class NetRunAheadCommandMsg;
class NetDestroyPlayerCommandMsg;
class NetKeepAliveCommandMsg;
class NetDisconnectKeepAliveCommandMsg;
class NetDisconnectPlayerCommandMsg;
class NetPacketRouterQueryCommandMsg;
class NetPacketRouterAckCommandMsg;
class NetDisconnectChatCommandMsg;
class NetChatCommandMsg;
class NetDisconnectVoteCommandMsg;
class NetProgressCommandMsg;
class NetWrapperCommandMsg;
class NetFileCommandMsg;
class NetFileAnnounceCommandMsg;
class NetFileProgressCommandMsg;
class NetDisconnectFrameCommandMsg;
class NetDisconnectScreenOffCommandMsg;
class NetFrameResendRequestCommandMsg;
class NetLoadCompleteCommandMsg;
class NetTimeOutGameStartCommandMsg;

////////////////////////////////////////////////////////////////////////////////
// Helper functions for raw byte data handling
////////////////////////////////////////////////////////////////////////////////

namespace network
{

template<typename T>
size_t writePrimitive(UnsignedByte *dest, T value)
{
	memcpy(dest, &value, sizeof(value));
	return sizeof(value);
}

template<typename T>
size_t writeObject(UnsignedByte *dest, const T& value)
{
	memcpy(dest, &value, sizeof(value));
	return sizeof(value);
}

inline size_t writeBytes(UnsignedByte *dest, const UnsignedByte* src, size_t len)
{
	memcpy(dest, src, len);
	return len;
}

inline size_t writeStringWithoutNull(UnsignedByte *dest, const UnicodeString& value)
{
	memcpy(dest, value.str(), value.getByteCount());
	return static_cast<size_t>(value.getByteCount());
}

inline size_t writeStringWithNull(UnsignedByte *dest, const AsciiString& value)
{
	memcpy(dest, value.str(), value.getByteCount() + 1);
	return static_cast<size_t>(value.getByteCount() + 1);
}

} // namespace network

// Ensure structs are packed to 1-byte alignment for network protocol compatibility
#pragma pack(push, 1)

////////////////////////////////////////////////////////////////////////////////
// Network packet field type definitions
////////////////////////////////////////////////////////////////////////////////

typedef UnsignedByte NetPacketFieldType;

namespace NetPacketFieldTypes
{
	constexpr const NetPacketFieldType CommandType = 'T';
	constexpr const NetPacketFieldType Relay = 'R';
	constexpr const NetPacketFieldType Frame = 'F';
	constexpr const NetPacketFieldType PlayerId = 'P';
	constexpr const NetPacketFieldType CommandId = 'C';
	constexpr const NetPacketFieldType Data = 'D';
	constexpr const NetPacketFieldType Repeat = 'Z';
}

////////////////////////////////////////////////////////////////////////////////
// Common packet field structures
////////////////////////////////////////////////////////////////////////////////

struct NetPacketCommandTypeField
{
	NetPacketCommandTypeField() : fieldType(NetPacketFieldTypes::CommandType) {}
	char fieldType;
	UnsignedByte commandType;
};

struct NetPacketRelayField
{
	NetPacketRelayField() : fieldType(NetPacketFieldTypes::Relay) {}
	char fieldType;
	UnsignedByte relay;
};

struct NetPacketFrameField
{
	NetPacketFrameField() : fieldType(NetPacketFieldTypes::Frame) {}
	char fieldType;
	UnsignedInt frame;
};

struct NetPacketPlayerIdField
{
	NetPacketPlayerIdField() : fieldType(NetPacketFieldTypes::PlayerId) {}
	char fieldType;
	UnsignedByte playerId;
};

struct NetPacketCommandIdField
{
	NetPacketCommandIdField() : fieldType(NetPacketFieldTypes::CommandId) {}
	char fieldType;
	UnsignedShort commandId;
};

struct NetPacketDataField
{
	NetPacketDataField() : fieldType(NetPacketFieldTypes::Data) {}
	char fieldType;
};

struct NetPacketRepeatField
{
	NetPacketRepeatField() : fieldType(NetPacketFieldTypes::Repeat) {}
	char fieldType;
};

////////////////////////////////////////////////////////////////////////////////
// Packed Network structures
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// NetPacketRepeatCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketRepeatCommand
{
	struct CommandBase
	{
		NetPacketRepeatField repeat;
	};

	static size_t getSize()
	{
		return sizeof(CommandBase);
	}
	static size_t copyBytes(UnsignedByte *buffer)
	{
		CommandBase base;
		return network::writeObject(buffer, base);
	}
};

////////////////////////////////////////////////////////////////////////////////
// SmallNetPacketCommandBase
////////////////////////////////////////////////////////////////////////////////

struct SmallNetPacketCommandBaseSelect
{
	UnsignedByte useCommandType : 1;
	UnsignedByte useRelay : 1;
	UnsignedByte useFrame : 1;
	UnsignedByte usePlayerId : 1;
	UnsignedByte useCommandId : 1;
};

struct SmallNetPacketCommandBase
{
	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize(const SmallNetPacketCommandBaseSelect *select = nullptr);
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref, const SmallNetPacketCommandBaseSelect *select = nullptr);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketCommandTemplate
////////////////////////////////////////////////////////////////////////////////

template<typename Base, typename Data>
struct NetPacketCommandTemplate
{
	typedef Base CommandBase;
	typedef Data CommandData;

	static size_t getSize(const NetCommandMsg &msg)
	{
		size_t size = 0;
		size += CommandBase::getSize();
		size += CommandData::getSize(msg);
		return size;
	}
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref)
	{
		size_t size = 0;
		size += CommandBase::copyBytes(buffer + size, ref);
		size += CommandData::copyBytes(buffer + size, ref);
		return size;
	}
};

////////////////////////////////////////////////////////////////////////////////
// SmallNetPacketCommandTemplate
////////////////////////////////////////////////////////////////////////////////

template<typename Base, typename Data>
struct SmallNetPacketCommandTemplate
{
	typedef Base CommandBase;
	typedef Data CommandData;

	static size_t getSize(const NetCommandMsg &msg, const SmallNetPacketCommandBaseSelect *select = nullptr)
	{
		size_t size = 0;
		size += CommandBase::getSize(select);
		size += CommandData::getSize(msg);
		return size;
	}
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref, const SmallNetPacketCommandBaseSelect *select = nullptr)
	{
		size_t size = 0;
		size += CommandBase::copyBytes(buffer + size, ref, select);
		size += CommandData::copyBytes(buffer + size, ref);
		return size;
	}
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketNoData
////////////////////////////////////////////////////////////////////////////////

struct NetPacketNoData
{
	typedef void CommandMsg;

	static size_t getSize(const NetCommandMsg &) { return 0; }
	static size_t copyBytes(UnsignedByte *, const NetCommandRef &) { return 0; }
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketAckCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketAckCommandData
{
	typedef NetAckCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedShort commandId; // Command ID being acknowledged
		UnsignedByte originalPlayerId; // Original player who sent the command
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketAckCommandBase
{
	typedef NetAckCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		//NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		//NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketFrameCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketFrameCommandData
{
	typedef NetFrameCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedShort commandCount;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketFrameCommandBase
{
	typedef NetFrameCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketFrameField frame;
		NetPacketRelayField relay;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketPlayerLeaveCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketPlayerLeaveCommandData
{
	typedef NetPlayerLeaveCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedByte leavingPlayerId;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketPlayerLeaveCommandBase
{
	typedef NetPlayerLeaveCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketRunAheadMetricsCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketRunAheadMetricsCommandData
{
	typedef NetRunAheadMetricsCommandMsg CommandMsg;

	struct FixedData
	{
		Real averageLatency;
		UnsignedShort averageFps;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketRunAheadMetricsCommandBase
{
	typedef NetRunAheadMetricsCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketRunAheadCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketRunAheadCommandData
{
	typedef NetRunAheadCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedShort runAhead;
		UnsignedByte frameRate;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketRunAheadCommandBase
{
	typedef NetRunAheadCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketDestroyPlayerCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDestroyPlayerCommandData
{
	typedef NetDestroyPlayerCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedInt playerIndex;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketDestroyPlayerCommandBase
{
	typedef NetDestroyPlayerCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketKeepAliveCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketKeepAliveCommandBase
{
	typedef NetKeepAliveCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		//NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketDisconnectKeepAliveCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDisconnectKeepAliveCommandBase
{
	typedef NetDisconnectKeepAliveCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketDisconnectPlayerCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDisconnectPlayerCommandData
{
	typedef NetDisconnectPlayerCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedByte disconnectSlot;
		UnsignedInt disconnectFrame;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketDisconnectPlayerCommandBase
{
	typedef NetDisconnectPlayerCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketRouterQueryCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketRouterQueryCommandBase
{
	typedef NetPacketRouterQueryCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		//NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketRouterAckCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketRouterAckCommandBase
{
	typedef NetPacketRouterAckCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketDisconnectVoteCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDisconnectVoteCommandData
{
	typedef NetDisconnectVoteCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedByte slot;
		UnsignedInt voteFrame;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketDisconnectVoteCommandBase
{
	typedef NetDisconnectVoteCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketChatCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketChatCommandData
{
	typedef NetChatCommandMsg CommandMsg;

	static size_t getSize(const NetCommandMsg &msg);
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketChatCommandBase
{
	typedef NetChatCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketFrameField frame;
		NetPacketRelayField relay;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketDisconnectChatCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDisconnectChatCommandData
{
	typedef NetDisconnectChatCommandMsg CommandMsg;

	static size_t getSize(const NetCommandMsg &msg);
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketDisconnectChatCommandBase
{
	typedef NetDisconnectChatCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		//NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketGameCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketGameCommandData
{
	typedef NetGameCommandMsg CommandMsg;

	static size_t getSize(const NetCommandMsg &msg);
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketGameCommandBase
{
	typedef NetGameCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketFrameField frame;
		NetPacketRelayField relay;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketWrapperCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketWrapperCommandData
{
	typedef NetWrapperCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedShort wrappedCommandId;
		UnsignedInt chunkNumber;
		UnsignedInt numChunks;
		UnsignedInt totalDataLength;
		UnsignedInt dataLength;
		UnsignedInt dataOffset;
	};

	static size_t getSize(const NetCommandMsg &msg);
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketWrapperCommandBase
{
	typedef NetWrapperCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketFileCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketFileCommandData
{
	typedef NetFileCommandMsg CommandMsg;

	static size_t getSize(const NetCommandMsg &msg);
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketFileCommandBase
{
	typedef NetFileCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketFileAnnounceCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketFileAnnounceCommandData
{
	typedef NetFileAnnounceCommandMsg CommandMsg;

	static size_t getSize(const NetCommandMsg &msg);
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketFileAnnounceCommandBase
{
	typedef NetFileAnnounceCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketFileProgressCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketFileProgressCommandData
{
	typedef NetFileProgressCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedShort fileId;
		Int progress;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketFileProgressCommandBase
{
	typedef NetFileProgressCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketProgressCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketProgressCommandData
{
	typedef NetProgressCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedByte percentage;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketProgressCommandBase
{
	typedef NetProgressCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		//NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketLoadCompleteCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketLoadCompleteCommandBase
{
	typedef NetLoadCompleteCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketTimeOutGameStartCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketTimeOutGameStartCommandBase
{
	typedef NetTimeOutGameStartCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketDisconnectFrameCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDisconnectFrameCommandData
{
	typedef NetDisconnectFrameCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedInt disconnectFrame;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketDisconnectFrameCommandBase
{
	typedef NetDisconnectFrameCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketDisconnectScreenOffCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDisconnectScreenOffCommandData
{
	typedef NetDisconnectScreenOffCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedInt newFrame;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketDisconnectScreenOffCommandBase
{
	typedef NetDisconnectScreenOffCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		//NetPacketFrameField frame;
		NetPacketPlayerIdField playerId;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////
// NetPacketFrameResendRequestCommand
////////////////////////////////////////////////////////////////////////////////

struct NetPacketFrameResendRequestCommandData
{
	typedef NetFrameResendRequestCommandMsg CommandMsg;

	struct FixedData
	{
		UnsignedInt frameToResend;
	};

	static size_t getSize(const NetCommandMsg &msg) { return sizeof(FixedData); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

struct NetPacketFrameResendRequestCommandBase
{
	typedef NetFrameResendRequestCommandMsg CommandMsg;

	struct CommandBase
	{
		NetPacketCommandTypeField commandType;
		NetPacketRelayField relay;
		NetPacketPlayerIdField playerId;
		//NetPacketFrameField frame;
		NetPacketCommandIdField commandId;
		NetPacketDataField dataHeader;
	};

	static size_t getSize() { return sizeof(CommandBase); }
	static size_t copyBytes(UnsignedByte *buffer, const NetCommandRef &ref);
};

////////////////////////////////////////////////////////////////////////////////

struct NetPacketAckCommand                      : public NetPacketCommandTemplate<NetPacketAckCommandBase, NetPacketAckCommandData> {};
struct NetPacketFrameCommand                    : public NetPacketCommandTemplate<NetPacketFrameCommandBase, NetPacketFrameCommandData> {};
struct NetPacketPlayerLeaveCommand              : public NetPacketCommandTemplate<NetPacketPlayerLeaveCommandBase, NetPacketPlayerLeaveCommandData> {};
struct NetPacketRunAheadMetricsCommand          : public NetPacketCommandTemplate<NetPacketRunAheadMetricsCommandBase, NetPacketRunAheadMetricsCommandData> {};
struct NetPacketRunAheadCommand                 : public NetPacketCommandTemplate<NetPacketRunAheadCommandBase, NetPacketRunAheadCommandData> {};
struct NetPacketDestroyPlayerCommand            : public NetPacketCommandTemplate<NetPacketDestroyPlayerCommandBase, NetPacketDestroyPlayerCommandData> {};
struct NetPacketKeepAliveCommand                : public NetPacketCommandTemplate<NetPacketKeepAliveCommandBase, NetPacketNoData> {};
struct NetPacketDisconnectKeepAliveCommand      : public NetPacketCommandTemplate<NetPacketDisconnectKeepAliveCommandBase, NetPacketNoData> {};
struct NetPacketDisconnectPlayerCommand         : public NetPacketCommandTemplate<NetPacketDisconnectPlayerCommandBase, NetPacketDisconnectPlayerCommandData> {};
struct NetPacketRouterQueryCommand              : public NetPacketCommandTemplate<NetPacketRouterQueryCommandBase, NetPacketNoData> {};
struct NetPacketRouterAckCommand                : public NetPacketCommandTemplate<NetPacketRouterAckCommandBase, NetPacketNoData> {};
struct NetPacketDisconnectVoteCommand           : public NetPacketCommandTemplate<NetPacketDisconnectVoteCommandBase, NetPacketDisconnectVoteCommandData> {};
struct NetPacketChatCommand                     : public NetPacketCommandTemplate<NetPacketChatCommandBase, NetPacketChatCommandData> {};
struct NetPacketDisconnectChatCommand           : public NetPacketCommandTemplate<NetPacketDisconnectChatCommandBase, NetPacketDisconnectChatCommandData> {};
struct NetPacketGameCommand                     : public NetPacketCommandTemplate<NetPacketGameCommandBase, NetPacketGameCommandData> {};
struct NetPacketWrapperCommand                  : public NetPacketCommandTemplate<NetPacketWrapperCommandBase, NetPacketWrapperCommandData> {};
struct NetPacketFileCommand                     : public NetPacketCommandTemplate<NetPacketFileCommandBase, NetPacketFileCommandData> {};
struct NetPacketFileAnnounceCommand             : public NetPacketCommandTemplate<NetPacketFileAnnounceCommandBase, NetPacketFileAnnounceCommandData> {};
struct NetPacketFileProgressCommand             : public NetPacketCommandTemplate<NetPacketFileProgressCommandBase, NetPacketFileProgressCommandData> {};
struct NetPacketProgressCommand                 : public NetPacketCommandTemplate<NetPacketProgressCommandBase, NetPacketProgressCommandData> {};
struct NetPacketLoadCompleteCommand             : public NetPacketCommandTemplate<NetPacketLoadCompleteCommandBase, NetPacketNoData> {};
struct NetPacketTimeOutGameStartCommand         : public NetPacketCommandTemplate<NetPacketTimeOutGameStartCommandBase, NetPacketNoData> {};
struct NetPacketDisconnectFrameCommand          : public NetPacketCommandTemplate<NetPacketDisconnectFrameCommandBase, NetPacketDisconnectFrameCommandData> {};
struct NetPacketDisconnectScreenOffCommand      : public NetPacketCommandTemplate<NetPacketDisconnectScreenOffCommandBase, NetPacketDisconnectScreenOffCommandData> {};
struct NetPacketFrameResendRequestCommand       : public NetPacketCommandTemplate<NetPacketFrameResendRequestCommandBase, NetPacketFrameResendRequestCommandData> {};

struct SmallNetPacketAckCommand                 : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketAckCommandData> {};
struct SmallNetPacketFrameCommand               : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketFrameCommandData> {};
struct SmallNetPacketPlayerLeaveCommand         : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketPlayerLeaveCommandData> {};
struct SmallNetPacketRunAheadMetricsCommand     : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketRunAheadMetricsCommandData> {};
struct SmallNetPacketRunAheadCommand            : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketRunAheadCommandData> {};
struct SmallNetPacketDestroyPlayerCommand       : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketDestroyPlayerCommandData> {};
struct SmallNetPacketKeepAliveCommand           : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketNoData> {};
struct SmallNetPacketDisconnectKeepAliveCommand : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketNoData> {};
struct SmallNetPacketDisconnectPlayerCommand    : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketDisconnectPlayerCommandData> {};
struct SmallNetPacketRouterQueryCommand         : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketNoData> {};
struct SmallNetPacketRouterAckCommand           : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketNoData> {};
struct SmallNetPacketDisconnectVoteCommand      : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketDisconnectVoteCommandData> {};
struct SmallNetPacketChatCommand                : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketChatCommandData> {};
struct SmallNetPacketDisconnectChatCommand      : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketDisconnectChatCommandData> {};
struct SmallNetPacketGameCommand                : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketGameCommandData> {};
struct SmallNetPacketWrapperCommand             : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketWrapperCommandData> {};
struct SmallNetPacketFileCommand                : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketFileCommandData> {};
struct SmallNetPacketFileAnnounceCommand        : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketFileAnnounceCommandData> {};
struct SmallNetPacketFileProgressCommand        : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketFileProgressCommandData> {};
struct SmallNetPacketProgressCommand            : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketProgressCommandData> {};
struct SmallNetPacketLoadCompleteCommand        : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketNoData> {};
struct SmallNetPacketTimeOutGameStartCommand    : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketNoData> {};
struct SmallNetPacketDisconnectFrameCommand     : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketDisconnectFrameCommandData> {};
struct SmallNetPacketDisconnectScreenOffCommand : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketDisconnectScreenOffCommandData> {};
struct SmallNetPacketFrameResendRequestCommand  : public SmallNetPacketCommandTemplate<SmallNetPacketCommandBase, NetPacketFrameResendRequestCommandData> {};

// Restore normal struct packing
#pragma pack(pop)
