/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers.
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
//
// TheSuperHackers @refactor BobTista 10/07/2025
// Packed struct definitions for network packet serialization/deserialization.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameNetwork/NetworkDefs.h"

// Ensure structs are packed to 1-byte alignment for network protocol compatibility
#pragma pack(push, 1)

////////////////////////////////////////////////////////////////////////////////
// Network packet field type definitions
////////////////////////////////////////////////////////////////////////////////

// Network packet field type definitions
typedef UnsignedByte NetPacketFieldType;

namespace NetPacketFieldTypes {
	constexpr const NetPacketFieldType CommandType = 'T';		// NetCommandType field
	constexpr const NetPacketFieldType Relay = 'R';				// Relay field
	constexpr const NetPacketFieldType PlayerId = 'P';			// Player ID field
	constexpr const NetPacketFieldType CommandId = 'C';			// Command ID field
	constexpr const NetPacketFieldType Frame = 'F';				// Frame field
	constexpr const NetPacketFieldType Data = 'D';				// Data payload field
}

////////////////////////////////////////////////////////////////////////////////
// Common packet field structures
////////////////////////////////////////////////////////////////////////////////

// Command Type field: 'T' + UnsignedByte
struct NetPacketCommandTypeField {
	char header;              // 'T'
	UnsignedByte commandType;
};

// Relay field: 'R' + UnsignedByte
struct NetPacketRelayField {
	char header;              // 'R'
	UnsignedByte relay;
};

// Player ID field: 'P' + UnsignedByte
struct NetPacketPlayerIdField {
	char header;              // 'P'
	UnsignedByte playerId;
};

// Frame field: 'F' + UnsignedInt
struct NetPacketFrameField {
	char header;              // 'F'
	UnsignedInt frame;
};

// Command ID field: 'C' + UnsignedShort
struct NetPacketCommandIdField {
	char header;              // 'C'
	UnsignedShort commandId;
};

// Data field header: 'D' (followed by variable-length data)
struct NetPacketDataFieldHeader {
	char header;              // 'D'
};

////////////////////////////////////////////////////////////////////////////////
// Acknowledgment Command Packets
////////////////////////////////////////////////////////////////////////////////

// ACK command packet structure
// Fields: T + type, P + playerID, D + commandID + originalPlayerID
struct NetPacketAckCommand {
	NetPacketCommandTypeField commandType;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort commandId;           // Command ID being acknowledged
	UnsignedByte originalPlayerId;     // Original player who sent the command
};

////////////////////////////////////////////////////////////////////////////////
// Frame Info Command Packet
////////////////////////////////////////////////////////////////////////////////

// Frame info command packet structure
// Fields: T + type, R + relay, F + frame, P + playerID, C + commandID, D + commandCount
struct NetPacketFrameCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketFrameField frame;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort commandCount;
};

////////////////////////////////////////////////////////////////////////////////
// Player Leave Command Packet
////////////////////////////////////////////////////////////////////////////////

// Player leave command packet structure
// Fields: T + type, R + relay, F + frame, P + playerID, C + commandID, D + leavingPlayerID
struct NetPacketPlayerLeaveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketFrameField frame;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte leavingPlayerId;
};

////////////////////////////////////////////////////////////////////////////////
// Run Ahead Metrics Command Packet
////////////////////////////////////////////////////////////////////////////////

// Run ahead metrics command packet structure  
// Fields: T + type, R + relay, P + playerID, C + commandID, D + averageLatency + averageFps
struct NetPacketRunAheadMetricsCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	Real averageLatency;
	UnsignedShort averageFps;
};

////////////////////////////////////////////////////////////////////////////////
// Run Ahead Command Packet
////////////////////////////////////////////////////////////////////////////////

// Run ahead command packet structure
// Fields: T + type, R + relay, F + frame, P + playerID, C + commandID, D + runAhead + frameRate
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

////////////////////////////////////////////////////////////////////////////////
// Destroy Player Command Packet
////////////////////////////////////////////////////////////////////////////////

// Destroy player command packet structure
// Fields: T + type, R + relay, F + frame, P + playerID, C + commandID, D + playerIndex
struct NetPacketDestroyPlayerCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketFrameField frame;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt playerIndex;
};

////////////////////////////////////////////////////////////////////////////////
// Keep Alive Command Packet
////////////////////////////////////////////////////////////////////////////////

// Keep alive command packet structure
// Fields: T + type, R + relay, P + playerID, D
struct NetPacketKeepAliveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

////////////////////////////////////////////////////////////////////////////////
// Disconnect Keep Alive Command Packet
////////////////////////////////////////////////////////////////////////////////

// Disconnect keep alive command packet structure
// Fields: T + type, R + relay, P + playerID, D
struct NetPacketDisconnectKeepAliveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

////////////////////////////////////////////////////////////////////////////////
// Disconnect Player Command Packet
////////////////////////////////////////////////////////////////////////////////

// Disconnect player command packet structure
// Fields: T + type, R + relay, P + playerID, C + commandID, D + slot + disconnectFrame
struct NetPacketDisconnectPlayerCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte slot;
	UnsignedInt disconnectFrame;
};

////////////////////////////////////////////////////////////////////////////////
// Packet Router Command Packets
////////////////////////////////////////////////////////////////////////////////

// Packet router query command packet
// Fields: T + type, R + relay, P + playerID, D
struct NetPacketRouterQueryCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

// Packet router ack command packet
// Fields: T + type, R + relay, P + playerID, D
struct NetPacketRouterAckCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

////////////////////////////////////////////////////////////////////////////////
// Variable-Length Packet Headers
// These structs represent the fixed portion of packets with variable data
////////////////////////////////////////////////////////////////////////////////

// Chat command header (variable: text follows)
// Fixed fields: T + type, F + frame, R + relay, P + playerID, C + commandID, D + textLength
// Variable: WideChar text[textLength] + Int playerMask
struct NetPacketChatCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketFrameField frame;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte textLength;
};

// Disconnect chat command header (variable: text follows)
// Fixed fields: T + type, R + relay, P + playerID, D + textLength
// Variable: WideChar text[textLength]
struct NetPacketDisconnectChatCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte textLength;
};

// Disconnect vote command header (variable: none after fixed portion)
// Fields: T + type, R + relay, P + playerID, C + commandID, D + slot + voteFrame
struct NetPacketDisconnectVoteCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte slot;
	UnsignedInt voteFrame;
};

////////////////////////////////////////////////////////////////////////////////
// Packed Structs for getPackedByteCount() calculations
// These structs represent the fixed portion of variable-length command messages
////////////////////////////////////////////////////////////////////////////////

struct NetPacketChatCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	NetPacketFrameField frame;
	NetPacketCommandIdField commandId;
};

struct NetPacketDisconnectChatCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

// Game command packed struct (variable: game message data follows)
struct NetPacketGameCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	NetPacketFrameField frame;
	NetPacketCommandIdField commandId;
};

// Wrapper command packet (fixed size - contains metadata about wrapped command)
// Fields: T + type, P + playerID, C + commandID, R + relay, D + metadata
struct NetPacketWrapperCommand {
	NetPacketCommandTypeField commandType;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketRelayField relay;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort wrappedCommandId;
	UnsignedInt chunkNumber;
	UnsignedInt numChunks;
	UnsignedInt totalDataLength;
	UnsignedInt dataLength;
	UnsignedInt dataOffset;
};

// File command header (variable: filename and file data follow)
// Fixed fields: T + type, R + relay, P + playerID, C + commandID, D
// Variable: null-terminated filename + UnsignedInt fileDataLength + file data
struct NetPacketFileCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// File command packed struct (variable: filename and file data follow)
struct NetPacketFileCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	NetPacketCommandIdField commandId;
};

// File announce command header (variable: filename and metadata follow)
// Fixed fields: T + type, R + relay, P + playerID, C + commandID, D
// Variable: null-terminated filename + UnsignedShort fileID + UnsignedByte playerMask
struct NetPacketFileAnnounceCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// File announce command packed struct (variable: filename and metadata follow)
struct NetPacketFileAnnounceCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	NetPacketCommandIdField commandId;
};

// File progress command packet
// Fields: T + type, R + relay, P + playerID, C + commandID, D + fileID + progress
struct NetPacketFileProgressCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort fileId;
	Int progress;
};

// Game command header (variable: game message data follows)
// Fixed fields: T + type, F + frame, R + relay, P + playerID, C + commandID, D
// Variable: GameMessage type + argument types + argument data
struct NetPacketGameCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketFrameField frame;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// Progress message packet
// Fields: T + type, R + relay, P + playerID, C + commandID, D + percentage
struct NetPacketProgressMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte percentage;
};

// Load complete message packet
// Fields: T + type, R + relay, P + playerID, C + commandID, D
struct NetPacketLoadCompleteMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// Timeout game start message packet
// Fields: T + type, R + relay, P + playerID, C + commandID, D
struct NetPacketTimeOutGameStartMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// Disconnect frame command packet
// Fields: T + type, R + relay, P + playerID, C + commandID, D + disconnectFrame
struct NetPacketDisconnectFrameCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt disconnectFrame;
};

// Disconnect screen off command packet
// Fields: T + type, R + relay, P + playerID, C + commandID, D + newFrame
struct NetPacketDisconnectScreenOffCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt newFrame;
};

// Frame resend request command packet
// Fields: T + type, R + relay, P + playerID, C + commandID, D + frameToResend
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
