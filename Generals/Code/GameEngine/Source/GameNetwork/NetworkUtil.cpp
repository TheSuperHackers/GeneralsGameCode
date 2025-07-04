/*
**	Command & Conquer Generals(tm)
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


#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/networkutil.h"

Int MAX_FRAMES_AHEAD = 128;
Int MIN_RUNAHEAD = 10;
Int FRAME_DATA_LENGTH = (MAX_FRAMES_AHEAD+1)*2;
Int FRAMES_TO_KEEP = (MAX_FRAMES_AHEAD/2) + 1;

#ifdef DEBUG_LOGGING

void dumpBufferToLog(const void *vBuf, Int len, const char *fname, Int line)
{
	DEBUG_LOG(("======= dumpBufferToLog() %d bytes =======", len));
	DEBUG_LOG(("Source: %s:%d", fname, line));
	const char *buf = (const char *)vBuf;
	Int numLines = len / 8;
	if ((len % 8) != 0)
	{
		++numLines;
	}
	for (Int dumpindex = 0; dumpindex < numLines; ++dumpindex)
	{
		Int offset = dumpindex*8;
		DEBUG_LOG(("\t%5.5d\t", offset));
		Int dumpindex2;
		Int numBytesThisLine = min(8, len - offset);
		for (dumpindex2 = 0; dumpindex2 < numBytesThisLine; ++dumpindex2)
		{
			Int c = (buf[offset + dumpindex2] & 0xff);
			DEBUG_LOG(("%02X ", c));
		}
		for (; dumpindex2 < 8; ++dumpindex2)
		{
			DEBUG_LOG(("   "));
		}
		DEBUG_LOG((" | "));
		for (dumpindex2 = 0; dumpindex2 < numBytesThisLine; ++dumpindex2)
		{
			char c = buf[offset + dumpindex2];
			DEBUG_LOG(("%c", (isprint(c)?c:'.')));
		}
		DEBUG_LOG((""));
	}
	DEBUG_LOG(("End of packet dump"));
}

#endif // DEBUG_LOGGING

/**
 * ResolveIP turns a string ("games2.westwood.com", or "192.168.0.1") into
 * a 32-bit unsigned integer.
 */
UnsignedInt ResolveIP(AsciiString host)
{
  struct hostent *hostStruct;
  struct in_addr *hostNode;

  if (host.getLength() == 0)
  {
	  DEBUG_LOG(("ResolveIP(): Can't resolve NULL"));
	  return 0;
  }

  // String such as "127.0.0.1"
  if (isdigit(host.getCharAt(0)))
  {
    return ( ntohl(inet_addr(host.str())) );
  }

  // String such as "localhost"
  hostStruct = gethostbyname(host.str());
  if (hostStruct == NULL)
  {
	  DEBUG_LOG(("ResolveIP(): Can't resolve %s", host.str()));
	  return 0;
  }
  hostNode = (struct in_addr *) hostStruct->h_addr;
  return ( ntohl(hostNode->s_addr) );
}

/**
 * Returns the next network command ID.
 */
UnsignedShort GenerateNextCommandID() {
	static UnsignedShort commandID = 64000;
	++commandID;
	return commandID;
}

/**
 * Returns true if this type of command requires a unique command ID.
 */
Bool DoesCommandRequireACommandID(NetCommandType type) {
	if ((type == NETCOMMANDTYPE_GAMECOMMAND) ||
			(type == NETCOMMANDTYPE_FRAMEINFO) ||
			(type == NETCOMMANDTYPE_PLAYERLEAVE) ||
			(type == NETCOMMANDTYPE_DESTROYPLAYER) ||
			(type == NETCOMMANDTYPE_RUNAHEADMETRICS) ||
			(type == NETCOMMANDTYPE_RUNAHEAD) ||
			(type == NETCOMMANDTYPE_CHAT) ||
			(type == NETCOMMANDTYPE_DISCONNECTVOTE) ||
			(type == NETCOMMANDTYPE_LOADCOMPLETE) ||
			(type == NETCOMMANDTYPE_TIMEOUTSTART) ||
			(type == NETCOMMANDTYPE_WRAPPER) ||
			(type == NETCOMMANDTYPE_FILE) ||
			(type == NETCOMMANDTYPE_FILEANNOUNCE) ||
			(type == NETCOMMANDTYPE_FILEPROGRESS) ||
			(type == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(type == NETCOMMANDTYPE_DISCONNECTFRAME) ||
			(type == NETCOMMANDTYPE_DISCONNECTSCREENOFF) ||
			(type == NETCOMMANDTYPE_FRAMERESENDREQUEST))
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * Returns true if this type of network command requires an ack.
 */
Bool CommandRequiresAck(NetCommandMsg *msg) {
	if ((msg->getNetCommandType() == NETCOMMANDTYPE_GAMECOMMAND) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FRAMEINFO) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_PLAYERLEAVE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DESTROYPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_RUNAHEADMETRICS) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_RUNAHEAD) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_CHAT) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTVOTE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_LOADCOMPLETE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_TIMEOUTSTART) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_WRAPPER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEANNOUNCE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEPROGRESS) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTSCREENOFF) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FRAMERESENDREQUEST))
	{
		return TRUE;
	}
	return FALSE;
}

Bool IsCommandSynchronized(NetCommandType type) {
	if ((type == NETCOMMANDTYPE_GAMECOMMAND) ||
			(type == NETCOMMANDTYPE_FRAMEINFO) ||
			(type == NETCOMMANDTYPE_PLAYERLEAVE) ||
			(type == NETCOMMANDTYPE_DESTROYPLAYER) ||
			(type == NETCOMMANDTYPE_RUNAHEAD))
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * Returns true if this type of network command requires the ack to be sent directly to the player
 * rather than going through the packet router.  This should really only be used by commands
 * used on the disconnect screen.
 */
Bool CommandRequiresDirectSend(NetCommandMsg *msg) {
	if ((msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTVOTE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTPLAYER) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_LOADCOMPLETE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_TIMEOUTSTART) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEANNOUNCE) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FILEPROGRESS) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTSCREENOFF) ||
			(msg->getNetCommandType() == NETCOMMANDTYPE_FRAMERESENDREQUEST)) {
		return TRUE;
	}
	return FALSE;
}

AsciiString GetAsciiNetCommandType(NetCommandType type) {
	AsciiString s;
	if (type == NETCOMMANDTYPE_FRAMEINFO) {
		s.set("NETCOMMANDTYPE_FRAMEINFO");
	} else if (type == NETCOMMANDTYPE_GAMECOMMAND) {
		s.set("NETCOMMANDTYPE_GAMECOMMAND");
	} else if (type == NETCOMMANDTYPE_PLAYERLEAVE) {
		s.set("NETCOMMANDTYPE_PLAYERLEAVE");
	} else if (type == NETCOMMANDTYPE_RUNAHEADMETRICS) {
		s.set("NETCOMMANDTYPE_RUNAHEADMETRICS");
	} else if (type == NETCOMMANDTYPE_RUNAHEAD) {
		s.set("NETCOMMANDTYPE_RUNAHEAD");
	} else if (type == NETCOMMANDTYPE_DESTROYPLAYER) {
		s.set("NETCOMMANDTYPE_DESTROYPLAYER");
	} else if (type == NETCOMMANDTYPE_ACKBOTH) {
		s.set("NETCOMMANDTYPE_ACKBOTH");
	} else if (type == NETCOMMANDTYPE_ACKSTAGE1) {
		s.set("NETCOMMANDTYPE_ACKSTAGE1");
	} else if (type == NETCOMMANDTYPE_ACKSTAGE2) {
		s.set("NETCOMMANDTYPE_ACKSTAGE2");
	} else if (type == NETCOMMANDTYPE_FRAMEINFO) {
		s.set("NETCOMMANDTYPE_FRAMEINFO");
	} else if (type == NETCOMMANDTYPE_KEEPALIVE) {
		s.set("NETCOMMANDTYPE_KEEPALIVE");
	} else if (type == NETCOMMANDTYPE_DISCONNECTCHAT) {
		s.set("NETCOMMANDTYPE_DISCONNECTCHAT");
	} else if (type == NETCOMMANDTYPE_CHAT) {
		s.set("NETCOMMANDTYPE_CHAT");
	} else if (type == NETCOMMANDTYPE_MANGLERQUERY) {
		s.set("NETCOMMANDTYPE_MANGLERQUERY");
	} else if (type == NETCOMMANDTYPE_MANGLERRESPONSE) {
		s.set("NETCOMMANDTYPE_MANGLERRESPONSE");
	} else if (type == NETCOMMANDTYPE_DISCONNECTKEEPALIVE) {
		s.set("NETCOMMANDTYPE_DISCONNECTKEEPALIVE");
	} else if (type == NETCOMMANDTYPE_DISCONNECTPLAYER) {
		s.set("NETCOMMANDTYPE_DISCONNECTPLAYER");
	} else if (type == NETCOMMANDTYPE_PACKETROUTERQUERY) {
		s.set("NETCOMMANDTYPE_PACKETROUTERQUERY");
	} else if (type == NETCOMMANDTYPE_PACKETROUTERACK) {
		s.set("NETCOMMANDTYPE_PACKETROUTERACK");
	} else if (type == NETCOMMANDTYPE_DISCONNECTVOTE) {
		s.set("NETCOMMANDTYPE_DISCONNECTVOTE");
	} else if (type == NETCOMMANDTYPE_PROGRESS) {
		s.set("NETCOMMANDTYPE_PROGRESS");
	} else if (type == NETCOMMANDTYPE_LOADCOMPLETE) {
		s.set("NETCOMMANDTYPE_LOADCOMPLETE");
	} else if (type == NETCOMMANDTYPE_TIMEOUTSTART) {
		s.set("NETCOMMANDTYPE_TIMEOUTSTART");
	} else if (type == NETCOMMANDTYPE_WRAPPER) {
		s.set("NETCOMMANDTYPE_WRAPPER");
	} else if (type == NETCOMMANDTYPE_FILE) {
		s.set("NETCOMMANDTYPE_FILE");
	} else if (type == NETCOMMANDTYPE_FILEANNOUNCE) {
		s.set("NETCOMMANDTYPE_FILEANNOUNCE");
	} else if (type == NETCOMMANDTYPE_FILEPROGRESS) {
		s.set("NETCOMMANDTYPE_FILEPROGRESS");
	} else if (type == NETCOMMANDTYPE_DISCONNECTFRAME) {
		s.set("NETCOMMANDTYPE_DISCONNECTFRAME");
	} else if (type == NETCOMMANDTYPE_DISCONNECTSCREENOFF) {
		s.set("NETCOMMANDTYPE_DISCONNECTSCREENOFF");
	} else if (type == NETCOMMANDTYPE_FRAMERESENDREQUEST) {
		s.set("NETCOMMANDTYPE_FRAMERESENDREQUEST");
	} else {
		s.set("UNKNOWN");
	}
	return s;
}
