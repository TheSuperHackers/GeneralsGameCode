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

////////////////////////////////////////////////////////////////////////////////
// Common packet field structures
////////////////////////////////////////////////////////////////////////////////

// Command Type field: 'T' + UnsignedByte
struct NetPacketCommandTypeField {
	const NetPacketFieldType type;    // 'T'
	UnsignedByte commandType;

	NetPacketCommandTypeField() : type(NetPacketFieldTypes::CommandType) {}
};

// Relay field: 'R' + UnsignedByte
struct NetPacketRelayField {
	const NetPacketFieldType type;    // 'R'
	UnsignedByte relay;

	NetPacketRelayField() : type(NetPacketFieldTypes::Relay) {}
};

// Player ID field: 'P' + UnsignedByte
struct NetPacketPlayerIdField {
	const NetPacketFieldType type;    // 'P'
	UnsignedByte playerId;

	NetPacketPlayerIdField() : type(NetPacketFieldTypes::PlayerId) {}
};

// Frame field: 'F' + UnsignedInt
struct NetPacketFrameField {
	const NetPacketFieldType type;    // 'F'
	UnsignedInt frame;

	NetPacketFrameField() : type(NetPacketFieldTypes::Frame) {}
};

// Command ID field: 'C' + UnsignedShort
struct NetPacketCommandIdField {
	const NetPacketFieldType type;    // 'C'
	UnsignedShort commandId;

	NetPacketCommandIdField() : type(NetPacketFieldTypes::CommandId) {}
};

// Data field header: 'D' (followed by variable-length data)
struct NetPacketDataFieldHeader {
	const NetPacketFieldType type;    // 'D'

	NetPacketDataFieldHeader() : type(NetPacketFieldTypes::Data) {}
};

////////////////////////////////////////////////////////////////////////////////
// Acknowledgment Command Packets
////////////////////////////////////////////////////////////////////////////////

// ACK command packet structure
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
// Variable-Length Packet Headers
// These structs represent the fixed portion of packets with variable data
////////////////////////////////////////////////////////////////////////////////

// Chat command header (variable: text follows)
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
// Variable: WideChar text[textLength]
struct NetPacketDisconnectChatCommandHeader {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte textLength;
};

// Disconnect vote command header (variable: none after fixed portion)
struct NetPacketDisconnectVoteCommand {
	NetPacketCommandTypeField commandType;
	NetPacketRelayField relay;
	NetPacketPlayerIdField playerId;
	NetPacketCommandIdField commandId;
	NetPacketDataFieldHeader dataHeader;
	UnsignedByte slot;
	UnsignedInt voteFrame;
};

// Wrapper command packet (fixed size - contains metadata about wrapped command)
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

////////////////////////////////////////////////////////////////////////////////
// Static Assert Tests - Verify struct sizes match original manual calculations
// These tests ensure the refactor maintains exact compatibility
////////////////////////////////////////////////////////////////////////////////

// Test function for Frame Resend Request Command
constexpr UnsignedInt GetFrameResendRequestCommandSize()
{
	UnsignedInt msglen = 0;
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::CommandType and command type
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::PlayerId and player ID
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedShort); // NetPacketFieldTypes::CommandId and command ID
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::Relay and relay

	++msglen; // NetPacketFieldTypes::Data
	msglen += sizeof(UnsignedInt); // frame to resend

	return msglen;
}

// Test function for ACK Command
constexpr UnsignedInt GetAckCommandSize()
{
	UnsignedInt msglen = 0;
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::CommandType and command type
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::PlayerId and player ID
	++msglen; // NetPacketFieldTypes::Data
	msglen += sizeof(UnsignedShort); // command ID being acknowledged
	msglen += sizeof(UnsignedByte);  // original player ID

	return msglen;
}

// Test function for Frame Command
constexpr UnsignedInt GetFrameCommandSize()
{
	UnsignedInt msglen = 0;
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::CommandType and command type
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::Relay and relay
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedInt);	// NetPacketFieldTypes::Frame and frame
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::PlayerId and player ID
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedShort); // NetPacketFieldTypes::CommandId and command ID
	++msglen; // NetPacketFieldTypes::Data
	msglen += sizeof(UnsignedShort); // command count

	return msglen;
}

// Test function for Player Leave Command
constexpr UnsignedInt GetPlayerLeaveCommandSize()
{
	UnsignedInt msglen = 0;
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::CommandType and command type
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::Relay and relay
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedInt);	// NetPacketFieldTypes::Frame and frame
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::PlayerId and player ID
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedShort); // NetPacketFieldTypes::CommandId and command ID
	++msglen; // NetPacketFieldTypes::Data
	msglen += sizeof(UnsignedByte); // leaving player ID

	return msglen;
}

// Test function for Keep Alive Command
constexpr UnsignedInt GetKeepAliveCommandSize()
{
	UnsignedInt msglen = 0;
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::CommandType and command type
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::Relay and relay
	msglen += sizeof(UnsignedByte) + sizeof(UnsignedByte);	// NetPacketFieldTypes::PlayerId and player ID
	++msglen; // NetPacketFieldTypes::Data

	return msglen;
}

// Static assertions to verify sizes match
static_assert(GetFrameResendRequestCommandSize() == sizeof(NetPacketFrameResendRequestCommand), "FrameResendRequestCommand size mismatch");
static_assert(GetAckCommandSize() == sizeof(NetPacketAckCommand), "AckCommand size mismatch");
static_assert(GetFrameCommandSize() == sizeof(NetPacketFrameCommand), "FrameCommand size mismatch");
static_assert(GetPlayerLeaveCommandSize() == sizeof(NetPacketPlayerLeaveCommand), "PlayerLeaveCommand size mismatch");
static_assert(GetKeepAliveCommandSize() == sizeof(NetPacketKeepAliveCommand), "KeepAliveCommand size mismatch");

