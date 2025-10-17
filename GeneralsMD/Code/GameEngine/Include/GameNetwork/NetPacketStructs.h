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

struct NetPacketCommandTypeField {
	const NetPacketFieldType type;
	UnsignedByte commandType;

	NetPacketCommandTypeField() : type(NetPacketFieldTypes::CommandType) {}
};

struct NetPacketRelayField {
	const NetPacketFieldType type;
	UnsignedByte relay;

	NetPacketRelayField() : type(NetPacketFieldTypes::Relay) {}
};

struct NetPacketPlayerIdField {
	const NetPacketFieldType type;
	UnsignedByte playerId;

	NetPacketPlayerIdField() : type(NetPacketFieldTypes::PlayerId) {}
};

struct NetPacketFrameField {
	const NetPacketFieldType type;
	UnsignedInt frame;

	NetPacketFrameField() : type(NetPacketFieldTypes::Frame) {}
};

struct NetPacketCommandIdField {
	const NetPacketFieldType type;
	UnsignedShort commandId;

	NetPacketCommandIdField() : type(NetPacketFieldTypes::CommandId) {}
};

struct NetPacketDataFieldHeader {
	const NetPacketFieldType type;

	NetPacketDataFieldHeader() : type(NetPacketFieldTypes::Data) {}
};

////////////////////////////////////////////////////////////////////////////////
// Acknowledgment Command Packets
////////////////////////////////////////////////////////////////////////////////

struct NetPacketAckCommand {
	NetPacketCommandTypeField commandType;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedShort commandId;
	UnsignedByte originalPlayerId;
};

////////////////////////////////////////////////////////////////////////////////
// Frame Info Command Packet
////////////////////////////////////////////////////////////////////////////////

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

struct NetPacketKeepAliveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

////////////////////////////////////////////////////////////////////////////////
// Disconnect Keep Alive Command Packet
////////////////////////////////////////////////////////////////////////////////

struct NetPacketDisconnectKeepAliveCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

////////////////////////////////////////////////////////////////////////////////
// Disconnect Player Command Packet
////////////////////////////////////////////////////////////////////////////////

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
struct NetPacketRouterQueryCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

// Packet router ack command packet
struct NetPacketRouterAckCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

////////////////////////////////////////////////////////////////////////////////
// Base Packed Struct for All Command Messages
// Contains the minimum common fields that all command messages share
////////////////////////////////////////////////////////////////////////////////

struct PackedNetCommandMsg {
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
struct NetPacketChatCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketFrameField frame;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

struct NetPacketDisconnectChatCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
};

////////////////////////////////////////////////////////////////////////////////
// Derived Packed Structs for getByteCount() calculations
// These structs inherit from PackedNetCommandMsg and add command-specific fields
////////////////////////////////////////////////////////////////////////////////

struct PackedNetChatCommandMsg : public PackedNetCommandMsg {
	NetPacketFrameField frame;
	NetPacketCommandIdField commandId;
};

struct PackedNetDisconnectChatCommandMsg : public PackedNetCommandMsg {
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
// Variable: null-terminated filename + UnsignedInt fileDataLength + file data
struct NetPacketFileCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// File announce command header (variable: filename and metadata follow)
// Variable: null-terminated filename + UnsignedShort fileID + UnsignedByte playerMask
struct NetPacketFileAnnounceCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// File progress command packet
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
struct NetPacketProgressMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte percentage;
};

// Load complete message packet
struct NetPacketLoadCompleteMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// Timeout game start message packet
struct NetPacketTimeOutGameStartMessage {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
};

// Disconnect frame command packet
struct NetPacketDisconnectFrameCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt disconnectFrame;
};

// Disconnect screen off command packet
struct NetPacketDisconnectScreenOffCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedInt newFrame;
};

// Frame resend request command packet
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


