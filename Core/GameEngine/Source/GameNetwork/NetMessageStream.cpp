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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine


// NetMessageStream.cpp
// Holds misc functions to encapsulate GameMessages into Command Packets to send
// over the network.
// Author: Matthew D. Campbell, July 2001
/*
#include "stdlib.h"				// VC++ wants this here, or gives compile error...

#include "Common/GameType.h"
#include "Common/MessageStream.h"
#include "Common/GameEngine.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/NetworkDefs.h"



// The per-player pointers for the list of commands
static CommandMsg *CommandHead[MAX_SLOTS] = {  /// @todo: remove static initialization
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};
static CommandMsg *CommandTail[MAX_SLOTS] = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

/**
 * AddToNetCommandList adds a CommandMsg to a list of commands.
 *
static Bool AddToNetCommandList(GameMessage *msg, UnsignedInt timestamp, CommandMsg *& CommandHead, CommandMsg *& CommandTail)
{
	CommandMsg *cmdMsg = NEW CommandMsg(timestamp, msg);
	if (!cmdMsg)
	{
		DEBUG_LOG(("Alloc failed!"));
		return false;
	}

	if (CommandTail == nullptr)
	{
		CommandHead = cmdMsg;
		CommandTail = cmdMsg;
	}
	else
	{
		cmdMsg->SetPrevCommandMsg(CommandTail);
		CommandTail->SetNextCommandMsg(cmdMsg);
		CommandTail = cmdMsg;
	}

	return true;
}

/**
 * AddToRemoteNetCommandList is used by TheNetwork to queue up commands recieved from other players.
 *
Bool AddToNetCommandList(Int playerNum, GameMessage *msg, UnsignedInt timestamp)
{
	if (playerNum < 0 || playerNum >= MAX_SLOTS)
		return false;

	DEBUG_LOG(("Adding msg to NetCommandList %d", playerNum));
	return AddToNetCommandList(msg, timestamp, CommandHead[playerNum], CommandTail[playerNum]);
}

/**
 * GetCommandMsg returns a GameMessage (deleting its CommandMsg wrapper) that is valid
 * for the current frame, or nullptr.
 *
static GameMessage * GetCommandMsg(UnsignedInt timestamp, CommandMsg *& CommandHead, CommandMsg *& CommandTail)
{
	if (!CommandHead)
		return nullptr;

	if (CommandHead->GetTimestamp() < timestamp)
	{
		DEBUG_LOG(("Time is %d, yet message timestamp is %d!", timestamp, CommandHead->GetTimestamp()));
		return nullptr;
	}

	if (CommandHead->GetTimestamp() != timestamp)
		return nullptr;

	CommandMsg *theMsg = CommandHead;

	if (CommandHead->GetNextCommandMsg())
	{
		CommandHead->GetNextCommandMsg()->SetPrevCommandMsg(nullptr);
		CommandHead = CommandHead->GetNextCommandMsg();
	}
	else
	{
		CommandHead = CommandTail = nullptr;
	}

	GameMessage *msg = theMsg->GetGameMessage();
	delete theMsg;
	return msg;
}

/**
 * GetCommandMsg returns a message from the command list.
 *
GameMessage * GetCommandMsg(UnsignedInt timestamp, Int playerNum)
{
	if (playerNum < 0 || playerNum >= MAX_SLOTS)
		return nullptr;

	//DEBUG_LOG(("Adding msg to NetCommandList %d", playerNum));
	return GetCommandMsg(timestamp, CommandHead[playerNum], CommandTail[playerNum]);
}




//====================================================================================

// The commandBuf & commandPacket hold the commands we're building up for the frame.
static unsigned char commandBuf[sizeof(CommandPacket)+1];
static CommandPacket *commandPacket = (CommandPacket *)(commandBuf+1);

/**
 * ClearCommandPacket clears the command packet at the start of the frame.
 *
void ClearCommandPacket(UnsignedInt frame)
{
	commandPacket->m_frame = frame;
	commandPacket->m_numCommands = 0;
}

/**
 * AddCommandToPacket creates a packet containing all move/attack/etc commands
 * for the current frame.
 *
Bool AddCommandToPacket(const GameMessage *msg)
{
	int messageSize = sizeofMessageHeader + sizeofMessageArg * msg->getArgumentCount();

	// If we have too much, send what we have
	if (bytesUsed && (bytesUsed + sizeof(CommandPacketHeader) + messageSize >= MAX_MESSAGE_LEN))
	{
		commandBuf[0] = MSGTYPE_PARTIALCOMMAND;
		if (!TheNetwork->queueSend(BROADCAST_CON, commandBuf, bytesUsed + sizeof(CommandPacketHeader) + 1, MSG_NEEDACK | MSG_SEQUENCED))
		{
			//DEBUG_ASSERTCRASH(false, ("Too many commands in one frame!  Some will be dropped."));
			DEBUG_LOG(("Too many commands in one frame!  Some will be dropped."));
			return false;
		}
		commandBuf[0] = MSGTYPE_COMMANDCOUNT;
		commandPacket->header.m_numCommands = 0;
		bytesUsed = 0;
	}

	if (bytesUsed + sizeof(CommandPacketHeader) + messageSize >= MAX_MESSAGE_LEN)
	{
		//DEBUG_ASSERTCRASH(false, ("Too many commands in one frame!  Some will be dropped."));
		DEBUG_LOG(("Too many commands in one frame!  Some will be dropped."));
		return false;
	}

	// We have room, so add the message
	commandPacket->header.m_numCommands++;
	commandPacket->m_commands[bytesUsed++] = (unsigned char)msg->getType();
	commandPacket->m_commands[bytesUsed++] = msg->getArgumentCount();

	for (int i=0; i<msg->getArgumentCount(); ++i)
	{
		memcpy((unsigned char *)(commandPacket->m_commands + bytesUsed), (unsigned char *)msg->getArgument(i), sizeofMessageArg);
		bytesUsed += sizeofMessageArg;
	}

	//DEBUG_ASSERTCRASH(bytesUsed + sizeof(CommandPacketHeader) < MAX_MESSAGE_LEN, ("Memory overwrite constructing command packet!"));
	//DEBUG_LOG(("Memory overwrite constructing command packet!"));
	return true;
}

/**
 * TheNetwork calls GetCommandPacket to get commands to send.
 *
CommandPacket *GetCommandPacket(void)
{
	commandBuf[0] = MSGTYPE_COMMANDCOUNT;
	return commandPacket;
}

//====================================================================================

*/
