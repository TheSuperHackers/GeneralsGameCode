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


#pragma once

#ifndef __NETWORKDEFS_H
#define __NETWORKDEFS_H

#include "Lib/BaseType.h"
#include "Common/MessageStream.h"

static const Int WOL_NAME_LEN = 64;

/// Max number of commands per frame
static const Int MAX_COMMANDS = 256;

extern Int MIN_LOGIC_FRAMES;
extern Int MAX_FRAMES_AHEAD;
extern Int MIN_RUNAHEAD;

// FRAME_DATA_LENGTH needs to be MAX_FRAMES_AHEAD+1 because a player on a different
// computer can send commands for a frame that is one beyond twice the max runahead.
extern Int FRAME_DATA_LENGTH;
extern Int FRAMES_TO_KEEP;

// This is the connection numbering: 1-8 are for players, 9 is a broadcast con.
enum ConnectionNumbers CPP_11(: Int)
{
	MAX_PLAYER = 7,			// The index of the highest possible player number.  This is 0 based, so the most players allowed in a game is MAX_PLAYER+1.
	NUM_CONNECTIONS
};

static const Int MAX_SLOTS = MAX_PLAYER+1;

// UDP (8 bytes) + IP header (28 bytes) = 36 bytes total.  We want a total packet size of 512, so 512 - 36 = 476
static const Int MAX_PACKET_SIZE = 476;

/**
 * Command packet - contains frame #, total # of commands, and each command.  This is what gets sent
 * to each player every frame
 */
#define MAX_MESSAGE_LEN 1024
#define MAX_MESSAGES 128
static const Int numCommandsPerCommandPacket = (MAX_MESSAGE_LEN - sizeof(UnsignedInt) - sizeof(UnsignedShort))/sizeof(GameMessage);
#pragma pack(push, 1)
struct CommandPacket
{
	UnsignedInt m_frame;
	UnsignedShort m_numCommands;
	unsigned char m_commands[numCommandsPerCommandPacket * sizeof(GameMessage)];
};
#pragma pack(pop)

#define MAX_TRANSPORT_STATISTICS_SECONDS 30

#pragma pack(push, 1)
struct TransportMessageHeader
{
	UnsignedInt crc;											///< packet-level CRC (must be first in packet)
	UnsignedShort magic;									///< Magic number identifying Generals packets
//	Int id;
//	NetMessageFlags flags;
};
#pragma pack(pop)

/**
 * Transport message - encapsulating info kept by the transport layer about each
 * packet.  These structs make up the in/out buffers at the transport layer.
 */
#pragma pack(push, 1)
struct TransportMessage
{
	TransportMessageHeader header;
	UnsignedByte data[MAX_MESSAGE_LEN];
	Int length;
	UnsignedInt addr;
	UnsignedShort port;
};
#pragma pack(pop)

#if defined(RTS_DEBUG)
#pragma pack(push, 1)
struct DelayedTransportMessage
{
	UnsignedInt deliveryTime;
	TransportMessage message;
};
#pragma pack(pop)
#endif

/**
 * Message types
 */
enum NetMessageFlag CPP_11(: Int) {
	MSG_ACK = 1,
	MSG_NEEDACK = 2,
	MSG_SEQUENCED = 4,
	MSG_SUPERCEDING = 8
};
typedef UnsignedByte NetMessageFlags;

enum NetCommandType CPP_11(: Int) {
	NETCOMMANDTYPE_UNKNOWN = -1,
	NETCOMMANDTYPE_ACKBOTH = 0,
	NETCOMMANDTYPE_ACKSTAGE1,
	NETCOMMANDTYPE_ACKSTAGE2,
	NETCOMMANDTYPE_FRAMEINFO,
	NETCOMMANDTYPE_GAMECOMMAND,
	NETCOMMANDTYPE_PLAYERLEAVE,
	NETCOMMANDTYPE_RUNAHEADMETRICS,
	NETCOMMANDTYPE_RUNAHEAD,
	NETCOMMANDTYPE_DESTROYPLAYER,
	NETCOMMANDTYPE_KEEPALIVE,
	NETCOMMANDTYPE_DISCONNECTCHAT,
	NETCOMMANDTYPE_CHAT,
	NETCOMMANDTYPE_MANGLERQUERY,
	NETCOMMANDTYPE_MANGLERRESPONSE,
	NETCOMMANDTYPE_PROGRESS,
	NETCOMMANDTYPE_LOADCOMPLETE,
	NETCOMMANDTYPE_TIMEOUTSTART,
	NETCOMMANDTYPE_WRAPPER,							// A wrapper command that holds a command thats too big to fit in a single packet.
	NETCOMMANDTYPE_FILE,
	NETCOMMANDTYPE_FILEANNOUNCE,
	NETCOMMANDTYPE_FILEPROGRESS,
	NETCOMMANDTYPE_FRAMERESENDREQUEST,

	// Disconnect menu command section.
	NETCOMMANDTYPE_DISCONNECTSTART,
	NETCOMMANDTYPE_DISCONNECTKEEPALIVE,
	NETCOMMANDTYPE_DISCONNECTPLAYER,
	NETCOMMANDTYPE_PACKETROUTERQUERY,
	NETCOMMANDTYPE_PACKETROUTERACK,
	NETCOMMANDTYPE_DISCONNECTVOTE,
	NETCOMMANDTYPE_DISCONNECTFRAME,
	NETCOMMANDTYPE_DISCONNECTSCREENOFF,
	NETCOMMANDTYPE_DISCONNECTEND,

	NETCOMMANDTYPE_MAX
};

enum NetLocalStatus CPP_11(: Int) {
	NETLOCALSTATUS_PREGAME = 0,
	NETLOCALSTATUS_INGAME,
	NETLOCALSTATUS_LEAVING,
	NETLOCALSTATUS_LEFT,
	NETLOCALSTATUS_POSTGAME
};

enum PlayerLeaveCode CPP_11(: Int) {
	PLAYERLEAVECODE_CLIENT = 0,
	PLAYERLEAVECODE_LOCAL,
	PLAYERLEAVECODE_PACKETROUTER,
	PLAYERLEAVECODE_UNKNOWN
};

// Magic number for identifying a Generals packet.
static const UnsignedShort GENERALS_MAGIC_NUMBER = 0xF00D;

// The number of fps history entries.
//static const Int NETWORK_FPS_HISTORY_LENGTH = 30;

// The number of ping history entries.
//static const Int NETWORK_LATENCY_HISTORY_LENGTH = 200;

// The number of miliseconds between run ahead metrics things
//static const Int NETWORK_RUN_AHEAD_METRICS_TIME = 5000;

// The number of cushion values to keep.
//static const Int NETWORK_CUSHION_HISTORY_LENGTH = 10;

// The amount of slack in the run ahead value.  This is the percentage of the calculated run ahead that is added.
//static const Int NETWORK_RUN_AHEAD_SLACK = 20;

// The number of seconds between when the connections to each player send a keep-alive packet.
// This should be less than 30 just to keep firewall ports open.
//static const Int NETWORK_KEEPALIVE_DELAY = 20;

// The number of milliseconds between when the game gets stuck on a frame for a network stall and
// and when the disconnect dialog comes up.
//static const Int NETWORK_DISCONNECT_TIME = 5000;

// The number of miliseconds between when a player's last disconnect keep alive command
// was recieved and when they are considered disconnected from the game.
//static const Int NETWORK_PLAYER_TIMEOUT_TIME = 60000;

// The base port number used for the transport socket.  A players slot number is added to this
// value to get their actual port number.
static const Int NETWORK_BASE_PORT_NUMBER = 8088;

// the singleton
class NetworkInterface;
extern NetworkInterface *TheNetwork;

#define PRINTF_IP_AS_4_INTS(ip) ((ip) >> 24) & 0xff, ((ip) >> 16) & 0xff, ((ip) >> 8 ) & 0xff, (ip) & 0xff

#endif
