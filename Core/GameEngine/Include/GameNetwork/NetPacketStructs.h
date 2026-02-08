/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

// TheSuperHackers @refactor BobTista 07/10/2025
// Packed struct definitions for network packet serialization.

#pragma once

#include "GameNetwork/NetworkDefs.h"

// Ensure structs are packed to 1-byte alignment for network protocol compatibility
#pragma pack(push, 1)

////////////////////////////////////////////////////////////////////////////////
// Network packet field type definitions
////////////////////////////////////////////////////////////////////////////////

typedef UnsignedByte NetPacketFieldType;

namespace NetPacketFieldTypes {
	constexpr const NetPacketFieldType CommandType = 'T';
	constexpr const NetPacketFieldType Relay = 'R';
	constexpr const NetPacketFieldType PlayerId = 'P';
	constexpr const NetPacketFieldType CommandId = 'C';
	constexpr const NetPacketFieldType Frame = 'F';
	constexpr const NetPacketFieldType Data = 'D';
}

////////////////////////////////////////////////////////////////////////////////
// Common packet field structures
////////////////////////////////////////////////////////////////////////////////

struct NetPacketCommandTypeField {
	char header;
	UnsignedByte commandType;
};

struct NetPacketRelayField {
	char header;
	UnsignedByte relay;
};

struct NetPacketPlayerIdField {
	char header;
	UnsignedByte playerId;
};

struct NetPacketFrameField {
	char header;
	UnsignedInt frame;
};

struct NetPacketCommandIdField {
	char header;
	UnsignedShort commandId;
};

struct NetPacketDataFieldHeader {
	char header;
};

////////////////////////////////////////////////////////////////////////////////
// Packed Network structures
////////////////////////////////////////////////////////////////////////////////

struct NetPacketAckCommand {
	NetPacketCommandTypeField commandType;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort commandId;           // Command ID being acknowledged
	UnsignedByte originalPlayerId;     // Original player who sent the command
};

struct NetPacketFrameCommand {
	NetPacketCommandTypeField commandType;
	NetPacketFrameField frame;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort commandCount;
};

struct NetPacketPlayerLeaveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketFrameField frame;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte leavingPlayerId;
};

struct NetPacketRunAheadMetricsCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	Real averageLatency;
	UnsignedShort averageFps;
};

struct NetPacketRunAheadCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketFrameField frame;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort runAhead;
	UnsignedByte frameRate;
};

struct NetPacketDestroyPlayerCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketFrameField frame;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt playerIndex;
};

struct NetPacketKeepAliveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

struct NetPacketDisconnectKeepAliveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

struct NetPacketDisconnectPlayerCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte slot;
	UnsignedInt disconnectFrame;
};

struct NetPacketRouterQueryCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

struct NetPacketRouterAckCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

struct NetPacketDisconnectVoteCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte slot;
	UnsignedInt voteFrame;
};

struct NetPacketChatCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketFrameField frame;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte textLength;
	// Variable fields: WideChar text[textLength] + Int playerMask
};

struct NetPacketDisconnectChatCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte textLength;
	// Variable fields: WideChar text[textLength]
};

struct NetPacketGameCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketFrameField frame;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	// Variable fields: GameMessage type + argument types + argument data
};

struct NetPacketWrapperCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort wrappedCommandId;
	UnsignedInt chunkNumber;
	UnsignedInt numChunks;
	UnsignedInt totalDataLength;
	UnsignedInt dataLength;
	UnsignedInt dataOffset;
};

struct NetPacketFileCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	// Variable fields: null-terminated filename + UnsignedInt fileDataLength + file data
};

struct NetPacketFileAnnounceCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	// Variable fields: null-terminated filename + UnsignedShort fileID + UnsignedByte playerMask
};

struct NetPacketFileProgressCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort fileId;
	Int progress;
};

struct NetPacketProgressMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte percentage;
};

struct NetPacketLoadCompleteMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

struct NetPacketTimeOutGameStartMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

struct NetPacketDisconnectFrameCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt disconnectFrame;
};

struct NetPacketDisconnectScreenOffCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt newFrame;
};

struct NetPacketFrameResendRequestCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt frameToResend;
};

// Restore normal struct packing
#pragma pack(pop)
