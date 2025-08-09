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

/**
 * The Connection class handles queues for individual players, one connection per player.
 * Connections are identified by their names (m_name).  This should accomodate changing IPs
 * in the face of modem disconnects, NAT irregularities, etc.
 * Messages can be guaranteed or non-guaranteed, sequenced or not.
 *
 *
 * So how this works is this:
 * The connection contains the incoming and outgoing commands to and from the player this
 * connection object represents.  The incoming packets are separated into the different frames
 * of execution.  This is done for efficiency reasons since we will be keeping track of commands
 * that are to be executed on many different frames, and they will need to be retrieved by the
 * connection manager on a per-frame basis.  We don't really care what frame the outgoing
 * commands are executed on as they all need to be sent out right away to ensure that the commands
 * get to their destination in time.
 */

#pragma once

#ifndef __CONNECTION_H
#define __CONNECTION_H

#include "GameNetwork/NetCommandList.h"
#include "GameNetwork/User.h"
#include "GameNetwork/Transport.h"
#include "GameNetwork/NetPacket.h"

#define CONNECTION_LATENCY_HISTORY_LENGTH 200

class Connection : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Connection, "Connection")
public:
	Connection();
	//~Connection();

	void update();
	void init();
	void reset();

	UnsignedInt doSend();
	void doRecv();

	Bool allCommandsReady(UnsignedInt frame);
	Bool isQueueEmpty();
	void attachTransport(Transport *transport);
	void setUser(User *user);
	User *getUser();
	void setFrameGrouping(time_t frameGrouping);

	void sendNetCommandMsg(NetCommandMsg *msg, UnsignedByte relay);

	// These two processAck calls do the same thing, just take different types of ACK commands.
	NetCommandRef * processAck(NetAckBothCommandMsg *msg);
	NetCommandRef * processAck(NetAckStage1CommandMsg *msg);
	NetCommandRef * processAck(NetCommandMsg *msg);
	NetCommandRef * processAck(UnsignedShort commandID, UnsignedByte originalPlayerID);

	void clearCommandsExceptFrom( Int playerIndex );

	void setQuitting( void );
	Bool isQuitting( void ) { return m_isQuitting; }

#if defined(RTS_DEBUG)
	void debugPrintCommands();
#endif

protected:
	void doRetryMetrics();

	Bool m_isQuitting;
	UnsignedInt m_quitTime;

	Transport *m_transport;
	User *m_user;

	NetCommandList *m_netCommandList;
	time_t m_retryTime;						///< The time between sending retry packets for this connection.  Time is in milliseconds.
	Real m_averageLatency;			///< The average time between sending a command and receiving an ACK.
	Real m_latencies[CONNECTION_LATENCY_HISTORY_LENGTH];	///< List of the last 100 latencies.

	time_t m_frameGrouping;				///< The minimum time between packet sends.
	time_t m_lastTimeSent;				///< The time of the last packet send.
	Int m_numRetries;							///< The number of retries for the last second.
	time_t m_retryMetricsTime;		///< The start time of the current retry metrics thing.
};

#endif
