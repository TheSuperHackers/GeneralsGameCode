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


#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/Connection.h"
#include "GameNetwork/networkutil.h"
#include "GameLogic/GameLogic.h"

enum { MaxQuitFlushTime = 30000 }; // wait this many milliseconds at most to retry things before quitting

/**
 * The constructor.
 */
Connection::Connection() {
	m_transport = NULL;
	m_user = NULL;
	m_netCommandList = NULL;
	m_retryTime = 2000; // set retry time to 2 seconds.
	m_lastTimeSent = 0;
	m_frameGrouping = 1;
	m_isQuitting = false;
	m_quitTime = 0;
	// Added By Sadullah Nader
	// clearing out the latency tracker
	m_averageLatency = 0.0f;
	Int i;
	for(i = 0; i < CONNECTION_LATENCY_HISTORY_LENGTH; i++)
	{
		m_latencies[i] = 0.0f;
	}
	// End Add
}

/**
 * The destructor.
 */
Connection::~Connection() {
	if (m_user != NULL) {
		m_user->deleteInstance();
		m_user = NULL;
	}

	if (m_netCommandList != NULL) {
		m_netCommandList->deleteInstance();
		m_netCommandList = NULL;
	}
}

/**
 * Initialize the connection and any subsystems.
 */
void Connection::init() {
	m_transport = NULL;

	if (m_user != NULL) {
		m_user->deleteInstance();
		m_user = NULL;
	}

	if (m_netCommandList == NULL) {
		m_netCommandList = newInstance(NetCommandList);
		m_netCommandList->init();
	}
	m_netCommandList->reset();

	m_lastTimeSent = 0;
	m_frameGrouping = 1;
	m_numRetries = 0;
	m_retryMetricsTime = 0;

	for (Int i = 0; i < CONNECTION_LATENCY_HISTORY_LENGTH; ++i) {
		m_latencies[i] = 0;
	}
	m_averageLatency = 0;
	m_isQuitting = FALSE;
	m_quitTime = 0;
}

/**
 * Take the connection back to the initial state.
 */
void Connection::reset() {
	init();
}

/**
 * Doesn't really do anything.
 */
void Connection::update() {
}

/**
 * Attach the transport object that this connection should use.
 */
void Connection::attachTransport(Transport *transport) {
	m_transport = transport;
}

/**
 * Assign this connection a user.  This is the user to whome we send all our packetized goodies.
 */
void Connection::setUser(User *user) {
	if (m_user != NULL) {
		m_user->deleteInstance();
	}

	m_user = user;
}

/**
 * Return the user object.
 */
User * Connection::getUser() {
	return m_user;
}

/**
 * Add this network command to the send queue for this connection.
 * The relay is the mask specifying the people the person we are sending to should send to.
 * The relay mostly has to do with the packet router.
 */
void Connection::sendNetCommandMsg(NetCommandMsg *msg, UnsignedByte relay) {
	static NetPacket *packet = NULL;

	// this is done so we don't have to allocate and delete a packet every time we send a message.
	if (packet == NULL) {
		packet = newInstance(NetPacket);
	}


	if (m_isQuitting)
		return;

	if (m_netCommandList != NULL) {
		// check to see if this command will fit in a packet.  If not, we need to split it up.
		// we are splitting up the command here so that the retry logic will not try to 
		// resend the ENTIRE command (i.e. multiple packets work of data) and only do the retry
		// one wrapper command at a time.
		packet->reset();

		NetCommandRef *tempref = NEW_NETCOMMANDREF(msg);

		Bool msgFits = packet->addCommand(tempref);
		tempref->deleteInstance(); // delete the temporary reference.
		tempref = NULL;

		if (!msgFits) {
			NetCommandRef *origref = NEW_NETCOMMANDREF(msg);
			origref->setRelay(relay);
			// the message doesn't fit in a single packet, need to split it up.
			NetPacketList packetList = NetPacket::ConstructBigCommandPacketList(origref);
			NetPacketListIter tempPacketPtr = packetList.begin();

			while (tempPacketPtr != packetList.end()) {
				NetPacket *tempPacket = (*tempPacketPtr);

				NetCommandList *list = tempPacket->getCommandList();
				NetCommandRef *ref1 = list->getFirstMessage();
				while (ref1 != NULL) {
					NetCommandRef *ref2 = m_netCommandList->addMessage(ref1->getCommand());
					ref2->setRelay(relay);

					ref1 = ref1->getNext();
				}

				tempPacket->deleteInstance();
				tempPacket = NULL;
				++tempPacketPtr;

				list->deleteInstance();
				list = NULL;
			}

			origref->deleteInstance();
			origref = NULL;

			return;
		}

		// the message fits in a packet, add to the command list normally.
		NetCommandRef *ref = m_netCommandList->addMessage(msg);

		if (ref != NULL) {

/*
#if ((defined(RTS_DEBUG)) || (defined(_INTERNAL)))
			if (msg->getNetCommandType() == NETCOMMANDTYPE_GAMECOMMAND) {
				DEBUG_LOG(("Connection::sendNetCommandMsg - added game command %d to net command list for frame %d.\n",
					msg->getID(), msg->getExecutionFrame()));
			} else if (msg->getNetCommandType() == NETCOMMANDTYPE_FRAMEINFO) {
				DEBUG_LOG(("Connection::sendNetCommandMsg - added frame info for frame %d\n", msg->getExecutionFrame()));
			}
#endif // RTS_DEBUG || _INTERNAL
*/

			ref->setRelay(relay);
		}
	}
}

void Connection::clearCommandsExceptFrom( Int playerIndex )
{
	NetCommandRef *tmp = m_netCommandList->getFirstMessage();
	while (tmp)
	{
		NetCommandMsg *msg = tmp->getCommand();
		if (msg->getPlayerID() != playerIndex)
		{
			DEBUG_LOG(("Connection::clearCommandsExceptFrom(%d) - clearing a command from %d for frame %d\n",
				playerIndex, tmp->getCommand()->getPlayerID(), tmp->getCommand()->getExecutionFrame()));
			m_netCommandList->removeMessage(tmp);
			NetCommandRef *toDelete = tmp;
			tmp = tmp->getNext();
			toDelete->deleteInstance();
		} else {
			tmp = tmp->getNext();
		}
	}
}

Bool Connection::isQueueEmpty() {
	if (m_netCommandList->getFirstMessage() == NULL) {
		return TRUE;
	}
	return FALSE;
}

void Connection::setQuitting( void )
{
	m_isQuitting = TRUE;
	m_quitTime = timeGetTime();
	DEBUG_LOG(("Connection::setQuitting() at time %d\n", m_quitTime));
}

/**
 * This is the good part. We take all the network commands queued up for this connection,
 * packetize them and put them on the transport's send queue for actual sending.
 */
UnsignedInt Connection::doSend() {
	Int numpackets = 0;
	time_t curtime = timeGetTime();
	Bool couldQueue = TRUE;

	// Do this check first, since it's an important fail-safe
	if (m_isQuitting && curtime > m_quitTime + MaxQuitFlushTime)
	{
		DEBUG_LOG(("Timed out a quitting connection.  Deleting all %d messages\n", m_netCommandList->length()));
		m_netCommandList->reset();
		return 0;
	}

	if ((curtime - m_lastTimeSent) < m_frameGrouping) {
//		DEBUG_LOG(("not sending packet, time = %d, m_lastFrameSent = %d, m_frameGrouping = %d\n", curtime, m_lastTimeSent, m_frameGrouping));
		return 0;
	}
	
	// iterate through all the messages and put them into a packet(s).
	NetCommandRef *msg = m_netCommandList->getFirstMessage();

	while ((msg != NULL) && couldQueue) {
		NetPacket *packet = newInstance(NetPacket);
		packet->init();
		packet->setAddress(m_user->GetIPAddr(), m_user->GetPort());

		Bool notDone = TRUE;

		// add the command messages until either we run out of messages or the packet is full.
		while ((msg != NULL) && notDone) {
			NetCommandRef *next = msg->getNext(); // Need this since msg could be deleted

			time_t timeLastSent = msg->getTimeLastSent();

			if (((curtime - timeLastSent) > m_retryTime) || (timeLastSent == -1)) {
				notDone = packet->addCommand(msg);
				if (notDone) {
					// the msg command was added to the packet.
					if (CommandRequiresAck(msg->getCommand())) {
						if (timeLastSent != -1) {
							++m_numRetries;
						}
						doRetryMetrics();
						msg->setTimeLastSent(curtime);
					} else {
						m_netCommandList->removeMessage(msg);
						msg->deleteInstance();
					}
				}
			}
			msg = next;
		}

		if (msg != NULL) {
			DEBUG_LOG(("didn't finish sending all commands in connection\n"));
		}

		++numpackets;

		/// @todo Make the act of giving the transport object a packet to send more efficient.  Make the transport take a NetPacket object rather than the raw data, thus avoiding an extra memcpy.
		if (packet->getNumCommands() > 0) {
			// If the packet actually has any information to give, give it to the transport object
			// for transmission.
			couldQueue = m_transport->queueSend(packet->getAddr(), packet->getPort(), packet->getData(), packet->getLength());
			m_lastTimeSent = curtime;
		}
		if (packet != NULL) {
			packet->deleteInstance(); // delete the packet now that we're done with it.
		}
	}

	return numpackets;
}

NetCommandRef * Connection::processAck(NetAckStage1CommandMsg *msg) {
	return processAck(msg->getCommandID(), msg->getOriginalPlayerID());
}

NetCommandRef * Connection::processAck(NetAckBothCommandMsg *msg) {
	return processAck(msg->getCommandID(), msg->getOriginalPlayerID());
}

NetCommandRef * Connection::processAck(NetCommandMsg *msg) {
	if (msg->getNetCommandType() == NETCOMMANDTYPE_ACKSTAGE1) {
		NetAckStage1CommandMsg *ackmsg = (NetAckStage1CommandMsg *)msg;
		return processAck(ackmsg);
	}

	if (msg->getNetCommandType() == NETCOMMANDTYPE_ACKBOTH) {
		NetAckBothCommandMsg *ackmsg = (NetAckBothCommandMsg *)msg;
		return processAck(ackmsg);
	}

	return NULL;
}

/**
 * The person we are sending to has ack'd one of the messages we sent him.
 * Take that message off the list of commands to send.
 */
NetCommandRef * Connection::processAck(UnsignedShort commandID, UnsignedByte originalPlayerID) {
	NetCommandRef *temp = m_netCommandList->getFirstMessage();
	while ((temp != NULL) && ((temp->getCommand()->getID() != commandID) || (temp->getCommand()->getPlayerID() != originalPlayerID))) {

		// cycle through the commands till we find the one we need to remove.
		// Need to check for both the command ID and the player ID.
		temp = temp->getNext();
	}
	if (temp == NULL) {
		return NULL;
	}

#if defined(RTS_DEBUG) || defined(_INTERNAL)
	Bool doDebug = FALSE;
	if (temp->getCommand()->getNetCommandType() == NETCOMMANDTYPE_DISCONNECTFRAME) {
		doDebug = TRUE;
	}
#endif

	Int index = temp->getCommand()->getID() % CONNECTION_LATENCY_HISTORY_LENGTH;
	m_averageLatency -= ((Real)(m_latencies[index])) / CONNECTION_LATENCY_HISTORY_LENGTH;
	Real lat = timeGetTime() - temp->getTimeLastSent();
	m_averageLatency += lat / CONNECTION_LATENCY_HISTORY_LENGTH;
	m_latencies[index] = lat;

#if defined(RTS_DEBUG) || defined(_INTERNAL)
	if (doDebug == TRUE) {
		DEBUG_LOG(("Connection::processAck - disconnect frame command %d found, removing from command list.\n", commandID));
	}
#endif
	m_netCommandList->removeMessage(temp);
	return temp;
}

void Connection::setFrameGrouping(time_t frameGrouping) {
	m_frameGrouping = frameGrouping;
//	m_retryTime = frameGrouping * 4;
}

void Connection::doRetryMetrics() {
	static Int numSeconds = 0;
	time_t curTime = timeGetTime();

	if ((curTime - m_retryMetricsTime) > 10000) {
		m_retryMetricsTime = curTime;
		++numSeconds;
//		DEBUG_LOG(("Retries in the last 10 seconds = %d, average latency = %fms\n", m_numRetries, m_averageLatency));
		m_numRetries = 0;
//		m_retryTime = m_averageLatency * 1.5;
	}
}

#if defined(RTS_DEBUG) || (_INTERNAL)
void Connection::debugPrintCommands() {
	NetCommandRef *ref = m_netCommandList->getFirstMessage();
	while (ref != NULL) {
		DEBUG_LOG(("Connection::debugPrintCommands - ID: %d\tType: %s\tRelay: 0x%X for frame %d\n",
			ref->getCommand()->getID(), GetAsciiNetCommandType(ref->getCommand()->getNetCommandType()).str(),
			ref->getRelay(), ref->getCommand()->getExecutionFrame()));
		ref = ref->getNext();
	}
}
#endif
