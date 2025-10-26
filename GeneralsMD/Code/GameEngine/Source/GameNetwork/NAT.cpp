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

// FILE: NAT.cpp /////////////////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland April 2002
// Props to Steve Tall for figuring all the NAT and Firewall behavior patterns out, making my job
// a LOT easier.
// Desc:   Resolves NAT'd IPs and port numbers for the other players in a game.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameNetwork/NAT.h"
#include "GameNetwork/Transport.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameClient/EstablishConnectionsMenu.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/GameSpy/GSConfig.h"


/*
 * In case you're wondering, we do this weird connection pairing scheme
 * to speed up the negotiation process, especially in cases where there
 * are 4 or more players (nodes).  Take for example an 8 player game...
 * In an 8 player game there are 28 connections that need to be negotiated,
 * doing this pairing scheme thing, we can make those 28 connections in the
 * time it would normally take to make 7 connections.  Since each connection
 * could potentially take several seconds, this can be a HUGE time savings.

 * Right now you're probably wondering who this Bryan Cleveland guy is.
 * He's the network coder that got fired when this didn't work.

 * In case you're wondering, this did end up working and Bryan left by
 * his own choice.
 */
//										m_connectionPairs[num nodes]	[round]			 [node index]
/* static */ Int NAT::m_connectionPairs[MAX_SLOTS-1][MAX_SLOTS-1][MAX_SLOTS] =
{
	{	// 2 nodes
		//	node 0	node 1	node 2	node 3	node 4	node 5	node 6	node 7
		{			1,			0,			-1,			-1,			-1,			-1,			-1,			-1},	// round 0
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 1
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 2
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 3
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 4
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 5
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1}		// round 6
	},
	{	// 3 nodes
		//	node 0	node 1	node 2	node 3	node 4	node 5	node 6	node 7
		{			1,			0,			-1,			-1,			-1,			-1,			-1,			-1},	// round 0
		{			2,			-1,			0,			-1,			-1,			-1,			-1,			-1},	// round 1
		{			-1,			2,			1,			-1,			-1,			-1,			-1,			-1},	// round 2
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 3
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 4
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 5
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1}		// round 6
	},
	{	// 4 nodes
		//	node 0	node 1	node 2	node 3	node 4	node 5	node 6	node 7
		{			1,			0,			3,			2,			-1,			-1,			-1,			-1},	// round 0
		{			2,			3,			0,			1,			-1,			-1,			-1,			-1},	// round 1
		{			3,			2,			1,			0,			-1,			-1,			-1,			-1},	// round 2
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 3
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 4
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 5
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1}		// round 6
	},
	{	// 5 nodes
		//	node 0	node 1	node 2	node 3	node 4	node 5	node 6	node 7
		{			2,			4,			0,			-1,			1,			-1,			-1,			-1},	// round 0
		{			-1,			3,			4,			1,			2,			-1,			-1,			-1},	// round 1
		{			3,			2,			1,			0,			-1,			-1,			-1,			-1},	// round 2
		{			4,			-1,			3,			2,			0,			-1,			-1,			-1},	// round 3
		{			1,			0,			-1,			4,			3,			-1,			-1,			-1},	// round 4
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 5
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1}		// round 6
	},
	{	// 6 nodes
		//	node 0	node 1	node 2	node 3	node 4	node 5	node 6	node 7
		{			3,			5,			4,			0,			2,			1,			-1,			-1},	// round 0
		{			2,			4,			0,			5,			1,			3,			-1,			-1},	// round 1
		{			4,			3,			5,			1,			0,			2,			-1,			-1},	// round 2
		{			1,			0,			3,			2,			5,			4,			-1,			-1},	// round 3
		{			5,			2,			1,			4,			3,			0,			-1,			-1},	// round 4
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1},	// round 5
		{			-1,			-1,			-1,			-1,			-1,			-1,			-1,			-1}		// round 6
	},
	{	// 7 nodes
		//	node 0	node 1	node 2	node 3	node 4	node 5	node 6	node 7
		{			-1,			6,			5,			4,			3,			2,			1,			-1},	// round 0
		{			2,			-1,			0,			6,			5,			4,			3,			-1},	// round 1
		{			4,			3,			-1,			1,			0,			6,			5,			-1},	// round 2
		{			6,			5,			4,			-1,			2,			1,			0,			-1},	// round 3
		{			1,			0,			6,			5,			-1,			3,			2,			-1},	// round 4
		{			3,			2,			1,			0,			6,			-1,			4,			-1},	// round 5
		{			5,			4,			3,			2,			1,			0,			-1,			-1}		// round 6
	},
	{	// 8 nodes
		//	node 0	node 1	node 2	node 3	node 4	node 5	node 6	node 7
		{			4,			5,			6,			7,			0,			1,			2,			3},		// round 0
		{			5,			4,			7,			6,			1,			0,			3,			2},		// round 1
		{			3,			6,			5,			0,			7,			2,			1,			4},		// round 2
		{			2,			7,			0,			5,			6,			3,			4,			1},		// round 3
		{			6,			3,			4,			1,			2,			7,			0,			5},		// round 4
		{			1,			0,			3,			2,			5,			4,			7,			6},		// round 5
		{			7,			2,			1,			4,			3,			6,			5,			0}		// round 6
	}
};

/* static */ Int NAT::m_timeBetweenRetries = 500; // .5 seconds between retries sounds good to me.
/* static */ time_t NAT::m_manglerRetryTimeInterval = 300; // sounds good to me.
/* static */ Int NAT::m_maxAllowedManglerRetries = 25; // works for me.
/* static */ time_t NAT::m_keepaliveInterval = 15000; // 15 seconds between keepalive packets seems good.
/* static */ time_t NAT::m_timeToWaitForPort = 15000; // wait for 15 seconds for the other player's port number.
/* static */ time_t NAT::m_timeForRoundTimeout = 15000; // wait for at most 15 seconds for each connection round to finish.

NAT *TheNAT = NULL;

NAT::NAT()
{
	//Added By Sadullah Nader
	//Initializations inserted
	m_beenProbed = FALSE;
	m_connectionPairIndex = 0;
	m_connectionRound = 0;
	m_localIP = 0;
	m_localNodeNumber = 0;
	m_manglerAddress = 0;
	m_manglerRetries = 0;
	m_numNodes = 0;
	m_numRetries = 0;
	m_previousSourcePort = 0;
	for(Int i = 0; i < MAX_SLOTS; i++)
		m_sourcePorts[i] = 0;
	m_spareSocketPort = 0;
	m_startingPortNumber = 0;
	m_targetNodeNumber = 0;
	//
	m_transport = NULL;
	m_slotList = NULL;
	m_roundTimeout = 0;

	m_maxNumRetriesAllowed = 10;
	m_packetID = 0x7f00;
}

NAT::~NAT() {
}

// if we're already finished, change to being idle
// if we are negotiating now, check to see if this round is done
// if it is, check to see if we're completely done with all rounds.
// if we are, set state to be done.
// if not go on to the next connection round.
// if we are negotiating still, call the connection update
// check to see if this connection is done for us, or if it has failed.
enum { MS_TO_WAIT_FOR_STATS = 5000 };
NATStateType NAT::update() {
	static UnsignedInt s_startStatWaitTime = 0;
	if (m_NATState == NATSTATE_DONE) {
		m_NATState = NATSTATE_IDLE;
	} else if (m_NATState == NATSTATE_WAITFORSTATS) {
		// check for all stats
		Bool gotAllStats = TRUE;
		Bool timedOut = FALSE;
		for (Int i=0; i<MAX_SLOTS; ++i)
		{
			const GameSpyGameSlot *slot = TheGameSpyGame->getGameSpySlot(i);
			if (slot && slot->isHuman())
			{
				PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(slot->getProfileID());
				if (stats.id == 0)
				{
					gotAllStats = FALSE;
					//DEBUG_LOG(("Failed to find stats for %ls(%d)", slot->getName().str(), slot->getProfileID()));
				}
			}
		}
		// check for timeout.  Timing out is not a fatal error - it just means we didn't get the other
		// player's stats.  We'll see 0/0 as his record, but we can still play him just fine.
		UnsignedInt now = timeGetTime();
		if (now > s_startStatWaitTime + MS_TO_WAIT_FOR_STATS)
		{
			DEBUG_LOG(("Timed out waiting for stats.  Let's just start the dang game."));
			timedOut = TRUE;
		}
		if (gotAllStats || timedOut)
		{
			m_NATState = NATSTATE_DONE;
			TheEstablishConnectionsMenu->endMenu();

			delete TheFirewallHelper;
			TheFirewallHelper = NULL;
		}
	} else if (m_NATState == NATSTATE_DOCONNECTIONPATHS) {
		if (allConnectionsDoneThisRound() == TRUE) {
			// we finished this round, move on to the next one.
			++m_connectionRound;
//			m_roundTimeout = timeGetTime() + TheGameSpyConfig->getRoundTimeout();
			m_roundTimeout = timeGetTime() + m_timeForRoundTimeout;
			DEBUG_LOG(("NAT::update - done with connection round, moving on to round %d", m_connectionRound));

			// we finished that round, now check to see if we're done, or if there are more rounds to go.
			if (allConnectionsDone() == TRUE) {
				// we're all done, time to go back home.
				m_NATState = NATSTATE_WAITFORSTATS;

				// 2/19/03 BGC - we have successfully negotaited a NAT thingy, so our behavior must be correct
				// so therefore we don't need to refresh our NAT even if we previously thought we had to.
				TheFirewallHelper->flagNeedToRefresh(FALSE);

				s_startStatWaitTime = timeGetTime();
				DEBUG_LOG(("NAT::update - done with all connections, woohoo!!"));
				/*
				m_NATState = NATSTATE_DONE;
				TheEstablishConnectionsMenu->endMenu();

				delete TheFirewallHelper;
				TheFirewallHelper = NULL;
				*/
			} else {
				doThisConnectionRound();
			}
		}
		NATConnectionState state = connectionUpdate();

		if (timeGetTime() > m_roundTimeout) {
			DEBUG_LOG(("NAT::update - round timeout expired"));
			setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
			notifyUsersOfConnectionFailed(m_localNodeNumber);
		}

		if (state == NATCONNECTIONSTATE_FAILED) {
			// if we fail
			m_NATState = NATSTATE_FAILED;
			TheEstablishConnectionsMenu->endMenu();
			if (TheFirewallHelper != NULL) {
				// we failed NAT negotiation, perhaps we need to redetect our firewall settings.
				// We don't trust the user to do it for themselves so we force them to do it next time
				// the log in.
				// 2/19/03 - ok, we don't want to do this right away, if the user tries to play in another game
				// before they log out and log back in the game won't have a chance at working.
				// so we need to simply flag it so that when they log out the firewall behavior gets blown away.
				TheFirewallHelper->flagNeedToRefresh(TRUE);
//				TheWritableGlobalData->m_firewallBehavior = FirewallHelperClass::FIREWALL_TYPE_UNKNOWN;
//				TheFirewallHelper->writeFirewallBehavior();

				delete TheFirewallHelper;
				TheFirewallHelper = NULL;
			}
			// we failed to connect, so we don't have to pass on the transport to the network.
			delete m_transport;
			m_transport = NULL;
		}
	}
	return m_NATState;
}


// update transport, check for PROBE packets from our target.
// check to see if its time to PROBE our target
// MANGLER:
// if we are talking to the mangler, check to see if we got a response
// if we didn't get a response, check to see if its time to send another packet to it
NATConnectionState NAT::connectionUpdate() {

	GameSlot *targetSlot = NULL;
	if (m_targetNodeNumber >= 0) {
		targetSlot = m_slotList[m_connectionNodes[m_targetNodeNumber].m_slotIndex];
	} else {
		return m_connectionStates[m_localNodeNumber];
	}

	if (m_beenProbed == FALSE) {
		if (timeGetTime() >= m_nextPortSendTime) {
//			sendMangledPortNumberToTarget(m_previousSourcePort, targetSlot);
			sendMangledPortNumberToTarget(m_sourcePorts[m_targetNodeNumber], targetSlot);
//			m_nextPortSendTime = timeGetTime() + TheGameSpyConfig->getRetryInterval();
			m_nextPortSendTime = timeGetTime() + m_timeBetweenRetries;
		}
	}

	// check to see if its time to send out our keepalives.
	if (timeGetTime() >= m_nextKeepaliveTime) {
		for (UnsignedInt node = 0; node < m_numNodes; ++node) {
			if (m_myConnections[node] == TRUE) {
				// we've made this connection, send a keepalive.
				Int slotIndex = m_connectionNodes[node].m_slotIndex;
				GameSlot *slot = m_slotList[slotIndex];
				DEBUG_ASSERTCRASH(slot != NULL, ("Trying to send keepalive to a NULL slot"));
				if (slot != NULL) {
					UnsignedInt ip = slot->getIP();
					DEBUG_LOG(("NAT::connectionUpdate - sending keep alive to node %d at %d.%d.%d.%d:%d", node,
											PRINTF_IP_AS_4_INTS(ip), slot->getPort()));
					m_transport->queueSend(ip, slot->getPort(), (const unsigned char *)"KEEPALIVE", strlen("KEEPALIVE") + 1);
				}
			}
		}
//		m_nextKeepaliveTime = timeGetTime() + TheGameSpyConfig->getKeepaliveInterval();
		m_nextKeepaliveTime = timeGetTime() + m_keepaliveInterval;
	}

	m_transport->update();

	// check to see if we've been probed.
	for (Int i = 0; i < MAX_MESSAGES; ++i) {
		if (m_transport->m_inBuffer[i].length > 0) {
#ifdef DEBUG_LOGGING
			UnsignedInt ip = m_transport->m_inBuffer[i].addr;
#endif
			DEBUG_LOG(("NAT::connectionUpdate - got a packet from %d.%d.%d.%d:%d, length = %d",
									PRINTF_IP_AS_4_INTS(ip), m_transport->m_inBuffer[i].port, m_transport->m_inBuffer[i].length));
			UnsignedByte *data = m_transport->m_inBuffer[i].data;
			if (memcmp(data, "PROBE", strlen("PROBE")) == 0) {
				Int fromNode = atoi((char *)data + strlen("PROBE"));
				DEBUG_LOG(("NAT::connectionUpdate - we've been probed by node %d.", fromNode));

				if (fromNode == m_targetNodeNumber) {
					DEBUG_LOG(("NAT::connectionUpdate - probe was sent by our target, setting connection state %d to done.", m_targetNodeNumber));
					setConnectionState(m_targetNodeNumber, NATCONNECTIONSTATE_DONE);

					if (m_transport->m_inBuffer[i].addr != targetSlot->getIP()) {
						UnsignedInt fromIP = m_transport->m_inBuffer[i].addr;
#ifdef DEBUG_LOGGING
						UnsignedInt slotIP = targetSlot->getIP();
#endif
						DEBUG_LOG(("NAT::connectionUpdate - incomming packet has different from address than we expected, incoming: %d.%d.%d.%d expected: %d.%d.%d.%d",
												PRINTF_IP_AS_4_INTS(fromIP),
												PRINTF_IP_AS_4_INTS(slotIP)));
						targetSlot->setIP(fromIP);
					}
					if (m_transport->m_inBuffer[i].port != targetSlot->getPort()) {
						DEBUG_LOG(("NAT::connectionUpdate - incoming packet came from a different port than we expected, incoming: %d expected: %d",
												m_transport->m_inBuffer[i].port, targetSlot->getPort()));
						targetSlot->setPort(m_transport->m_inBuffer[i].port);
						m_sourcePorts[m_targetNodeNumber] = m_transport->m_inBuffer[i].port;
					}
					notifyUsersOfConnectionDone(m_targetNodeNumber);
				}

				m_transport->m_inBuffer[i].length = 0;
			}
			if (memcmp(data, "KEEPALIVE", strlen("KEEPALIVE")) == 0) {
				// keep alive packet, just toss it.
				DEBUG_LOG(("NAT::connectionUpdate - got keepalive from %d.%d.%d.%d:%d",
										PRINTF_IP_AS_4_INTS(ip), m_transport->m_inBuffer[i].port));
				m_transport->m_inBuffer[i].length = 0;
			}
		}
	}

	// we are waiting for our target to tell us that they have received our probe.
	if (m_connectionStates[m_localNodeNumber] == NATCONNECTIONSTATE_WAITINGFORRESPONSE) {
		// check to see if it's time to probe our target.
		if ((m_timeTillNextSend != -1) && (m_timeTillNextSend <= timeGetTime())) {
			if (m_numRetries > m_maxNumRetriesAllowed) {
				DEBUG_LOG(("NAT::connectionUpdate - too many retries, connection failed."));
				setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);

				notifyUsersOfConnectionFailed(m_localNodeNumber);
			} else {
				DEBUG_LOG(("NAT::connectionUpdate - trying to send another probe (#%d) to our target", m_numRetries+1));
				// Send a probe.
				sendAProbe(targetSlot->getIP(), targetSlot->getPort(), m_localNodeNumber);
//				m_timeTillNextSend = timeGetTime() + TheGameSpyConfig->getRetryInterval();
				m_timeTillNextSend = timeGetTime() + m_timeBetweenRetries;

				// tell the target they've been probed. In other words, our port is open.
				notifyTargetOfProbe(targetSlot);

				++m_numRetries;
			}
		}
	}

	// we are waiting for a response from the mangler to tell us what port we're using.
	if (m_connectionStates[m_localNodeNumber] == NATCONNECTIONSTATE_WAITINGFORMANGLERRESPONSE) {
		UnsignedShort mangledPort = 0;
		if (TheFirewallHelper != NULL) {
			mangledPort = TheFirewallHelper->getManglerResponse(m_packetID);
		}
		if (mangledPort != 0) {
			// we got a response.  now we need to start probing (unless of course we have a netgear)
			processManglerResponse(mangledPort);

			// we know there is a firewall helper if we got here.
			TheFirewallHelper->closeSpareSocket(m_spareSocketPort);
			m_spareSocketPort = 0;
		} else {
			if (timeGetTime() >= m_manglerRetryTime) {
				++m_manglerRetries;
//				if (m_manglerRetries > TheGameSpyConfig->getMaxManglerRetries()) {
				if (m_manglerRetries > m_maxAllowedManglerRetries) {
					// we couldn't communicate with the mangler, just use our non-mangled
					// port number and hope that works.
					DEBUG_LOG(("NAT::connectionUpdate - couldn't talk with the mangler using default port number"));
					sendMangledPortNumberToTarget(getSlotPort(m_connectionNodes[m_localNodeNumber].m_slotIndex), targetSlot);
					m_sourcePorts[m_targetNodeNumber] = getSlotPort(m_connectionNodes[m_localNodeNumber].m_slotIndex);
					setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORRESPONSE);
				} else {
					if (TheFirewallHelper != NULL) {
						DEBUG_LOG(("NAT::connectionUpdate - trying to send to the mangler again. mangler address: %d.%d.%d.%d, from port: %d, packet ID:%d",
							PRINTF_IP_AS_4_INTS(m_manglerAddress), m_spareSocketPort, m_packetID));
						TheFirewallHelper->sendToManglerFromPort(m_manglerAddress, m_spareSocketPort, m_packetID);
					}
//					m_manglerRetryTime = TheGameSpyConfig->getRetryInterval() + timeGetTime();
					m_manglerRetryTime = m_manglerRetryTimeInterval + timeGetTime();
				}
			}
		}
	}

	if (m_connectionStates[m_localNodeNumber] == NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT) {
		if (timeGetTime() > m_timeoutTime) {
			DEBUG_LOG(("NAT::connectionUpdate - waiting too long to get the other player's port number, failed."));
			setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);

			notifyUsersOfConnectionFailed(m_localNodeNumber);
		}
	}

	return m_connectionStates[m_localNodeNumber];
}

// this is the function that starts the NAT/firewall negotiation process.
// after calling this, you should call the update function untill it returns
// NATSTATE_DONE.
void NAT::establishConnectionPaths() {
	DEBUG_LOG(("NAT::establishConnectionPaths - entering"));
	m_NATState = NATSTATE_DOCONNECTIONPATHS;
	DEBUG_LOG(("NAT::establishConnectionPaths - using %d as our starting port number", m_startingPortNumber));
	if (TheEstablishConnectionsMenu == NULL) {
		TheEstablishConnectionsMenu = NEW EstablishConnectionsMenu;
	}
	TheEstablishConnectionsMenu->initMenu();

	if (TheFirewallHelper == NULL) {
		TheFirewallHelper = createFirewallHelper();
	}

	DEBUG_ASSERTCRASH(m_slotList != NULL, ("NAT::establishConnectionPaths - don't have a slot list"));
	if (m_slotList == NULL) {
		return;
	}

	// determine how many nodes we have.
	m_numNodes = 0;
	Int i = 0;
	for (; i < MAX_SLOTS; ++i) {
		if (m_slotList[i] != NULL) {
			if (m_slotList[i]->isHuman()) {
				DEBUG_LOG(("NAT::establishConnectionPaths - slot %d is %ls", i, m_slotList[i]->getName().str()));
				++m_numNodes;
			}
		}
	}
	DEBUG_LOG(("NAT::establishConnectionPaths - number of nodes: %d", m_numNodes));

	if (m_numNodes < 2)
	{
		// just start the game - there isn't anybody to which to connect. :P
		m_NATState = NATSTATE_DONE;
		return;
	}

	m_connectionRound = 0;
	m_connectionPairIndex = m_numNodes - 2;
	Bool connectionAssigned[MAX_SLOTS];

	for (i = 0; i < MAX_SLOTS; ++i) {
		m_connectionNodes[i].m_slotIndex = -1;
		connectionAssigned[i] = FALSE;
		m_sourcePorts[i] = 0;
	}

	m_previousSourcePort = 0;

// check for netgear bug behavior.
// as an aside, if there are more than 2 netgear bug firewall's in the game,
// it probably isn't going to work so well.  stupid netgear.

// nodes with a netgear bug behavior need to be matched up first. This prevents
// the NAT table from being reset for connections to other nodes.  This also happens
// to be the reason why I call them "nodes" rather than "slots" or "players" as the
// ordering has to be messed with to get the netgears to make love, not war.
	DEBUG_LOG(("NAT::establishConnectionPaths - about to set up the node list"));
	DEBUG_LOG(("NAT::establishConnectionPaths - doing the netgear stuff"));
	UnsignedInt otherNetgearNum = -1;
	for (i = 0; i < MAX_SLOTS; ++i) {
		if ((m_slotList != NULL) && (m_slotList[i] != NULL)) {
			if ((m_slotList[i]->getNATBehavior() & FirewallHelperClass::FIREWALL_TYPE_NETGEAR_BUG) != 0) {
				if (otherNetgearNum == -1) {
					// this is the start of a new pair, put it in as the first non -1 node connection pair thing.
					Int nodeindex = 0;
					while ((m_connectionPairs[m_connectionPairIndex][0][nodeindex] == -1) || (m_connectionNodes[nodeindex].m_slotIndex != -1)) {
						++nodeindex;
					}
					m_connectionNodes[nodeindex].m_slotIndex = i;
					m_connectionNodes[nodeindex].m_behavior = m_slotList[i]->getNATBehavior();
					connectionAssigned[i] = TRUE;
					otherNetgearNum = nodeindex;
					DEBUG_LOG(("NAT::establishConnectionPaths - first netgear in pair. assigning node %d to slot %d (%ls)", nodeindex, i, m_slotList[i]->getName().str()));
				} else {
					// this is the second in the pair of netgears, pair this up with the other one
					// for the first round.
					Int nodeindex = 0;
					while (m_connectionPairs[m_connectionPairIndex][0][nodeindex] != otherNetgearNum) {
						++nodeindex;
					}
					m_connectionNodes[nodeindex].m_slotIndex = i;
					m_connectionNodes[nodeindex].m_behavior = m_slotList[i]->getNATBehavior();
					connectionAssigned[i] = TRUE;
					otherNetgearNum = -1;
					DEBUG_LOG(("NAT::establishConnectionPaths - second netgear in pair. assigning node %d to slot %d (%ls)", nodeindex, i, m_slotList[i]->getName().str()));
				}
			}
		}
	}

	// fill in the rest of the nodes with the remaining slots.
	DEBUG_LOG(("NAT::establishConnectionPaths - doing the non-Netgear nodes"));
	for (i = 0; i < MAX_SLOTS; ++i) {
		if (connectionAssigned[i] == TRUE) {
			continue;
		}
		if (m_slotList[i] == NULL) {
			continue;
		}
		if (!(m_slotList[i]->isHuman())) {
			continue;
		}
		// find the first available connection node for this slot.
		Int nodeindex = 0;
		while (m_connectionNodes[nodeindex].m_slotIndex != -1) {
			++nodeindex;
		}
		DEBUG_LOG(("NAT::establishConnectionPaths - assigning node %d to slot %d (%ls)", nodeindex, i, m_slotList[i]->getName().str()));
		m_connectionNodes[nodeindex].m_slotIndex = i;
		m_connectionNodes[nodeindex].m_behavior = m_slotList[i]->getNATBehavior();
		connectionAssigned[i] = TRUE;
	}

// sanity check
#if defined(RTS_DEBUG)
	for (i = 0; i < m_numNodes; ++i) {
		DEBUG_ASSERTCRASH(connectionAssigned[i] == TRUE, ("connection number %d not assigned", i));
	}
#endif

	// find the local node number.
	for (i = 0; i < m_numNodes; ++i) {
		if (m_connectionNodes[i].m_slotIndex == TheGameSpyGame->getLocalSlotNum()) {
			m_localNodeNumber = i;
			DEBUG_LOG(("NAT::establishConnectionPaths - local node is %d", m_localNodeNumber));
			break;
		}
	}

	// set up the names in the connection window.
	Int playerNum = 0;
	for (i = 0; i < MAX_SLOTS; ++i) {
		while ((i < MAX_SLOTS) && (m_slotList[i] != NULL) && !(m_slotList[i]->isHuman())) {
			++i;
		}
		if (i >= MAX_SLOTS) {
			break;
		}
		if (i != TheGameSpyGame->getLocalSlotNum()) {
			TheEstablishConnectionsMenu->setPlayerName(playerNum, m_slotList[i]->getName());
			TheEstablishConnectionsMenu->setPlayerStatus(playerNum, NATCONNECTIONSTATE_WAITINGTOBEGIN);
			++playerNum;
		}
	}

//	m_roundTimeout = timeGetTime() + TheGameSpyConfig->getRoundTimeout();
	m_roundTimeout = timeGetTime() + m_timeForRoundTimeout;

	// make the connections for this round.
	// this song is cool.
	doThisConnectionRound();
}

void NAT::attachSlotList(GameSlot *slotList[], Int localSlot, UnsignedInt localIP) {
	m_slotList = slotList;
	m_localIP = localIP;
	m_transport = new Transport;
	DEBUG_LOG(("NAT::attachSlotList - initting the transport socket with address %d.%d.%d.%d:%d",
							PRINTF_IP_AS_4_INTS(m_localIP), getSlotPort(localSlot)));

	m_startingPortNumber = NETWORK_BASE_PORT_NUMBER + ((timeGetTime() / 1000) % 20000);
	DEBUG_LOG(("NAT::attachSlotList - using %d as the starting port number", m_startingPortNumber));
	generatePortNumbers(slotList, localSlot);
	m_transport->init(m_localIP, getSlotPort(localSlot));
}

Int NAT::getSlotPort(Int slot) {
//	return (slot + m_startingPortNumber);
	if (m_slotList[slot] != NULL) {
		return m_slotList[slot]->getPort();
	}
	return 0;
}

void NAT::generatePortNumbers(GameSlot *slotList[], Int localSlot) {
	for (UnsignedInt i = 0; i < (UnsignedInt)MAX_SLOTS; ++i) {
		if (slotList[i] != NULL) {
			if ((i == localSlot) && (TheWritableGlobalData->m_firewallPortOverride != 0)) {
				slotList[i]->setPort(TheWritableGlobalData->m_firewallPortOverride);
			} else {
				slotList[i]->setPort(i + m_startingPortNumber);
			}
		}
	}
}

Transport * NAT::getTransport() {
	return m_transport;
}

// figure out which port I'll be using.
// send the port number to our target for this round.
// init the m_connectionStates for all players.
void NAT::doThisConnectionRound() {
	DEBUG_LOG(("NAT::doThisConnectionRound - starting process for connection round %d", m_connectionRound));
	// clear out the states from the last round.
	m_targetNodeNumber = -1;

	Int i = 0;
	for (; i < MAX_SLOTS; ++i) {
		setConnectionState(i, NATCONNECTIONSTATE_NOSTATE);
	}

	m_beenProbed = FALSE;
	m_numRetries = 0;

	for (i = 0; i < m_numNodes; ++i) {
		Int targetNodeNumber = m_connectionPairs[m_connectionPairIndex][m_connectionRound][i];
		DEBUG_LOG(("NAT::doThisConnectionRound - node %d needs to connect to node %d", i, targetNodeNumber));
		if (targetNodeNumber != -1) {
			if (i == m_localNodeNumber) {
				m_targetNodeNumber = targetNodeNumber;
				DEBUG_LOG(("NAT::doThisConnectionRound - Local node is connecting to node %d", m_targetNodeNumber));
				UnsignedInt targetSlotIndex = m_connectionNodes[(m_connectionPairs[m_connectionPairIndex][m_connectionRound][i])].m_slotIndex;
				GameSlot *targetSlot = m_slotList[targetSlotIndex];
				GameSlot *localSlot = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex];

				DEBUG_ASSERTCRASH(localSlot != NULL, ("local slot is NULL"));
				DEBUG_ASSERTCRASH(targetSlot != NULL, ("trying to negotiate with a NULL target slot, slot is %d", m_connectionPairs[m_connectionPairIndex][m_connectionRound][i]));
				DEBUG_LOG(("NAT::doThisConnectionRound - Target slot index = %d (%ls)", targetSlotIndex, m_slotList[targetSlotIndex]->getName().str()));
				DEBUG_LOG(("NAT::doThisConnectionRound - Target slot has NAT behavior 0x%8X, local slot has NAT behavior 0x%8X", targetSlot->getNATBehavior(), localSlot->getNATBehavior()));

#if defined(DEBUG_LOGGING)
				UnsignedInt targetIP = targetSlot->getIP();
				UnsignedInt localIP = localSlot->getIP();
#endif

				DEBUG_LOG(("NAT::doThisConnectionRound - Target slot has IP %d.%d.%d.%d  Local slot has IP %d.%d.%d.%d",
							PRINTF_IP_AS_4_INTS(targetIP),
							PRINTF_IP_AS_4_INTS(localIP)));

				if (((targetSlot->getNATBehavior() & FirewallHelperClass::FIREWALL_TYPE_NETGEAR_BUG) == 0) &&
						((localSlot->getNATBehavior() & FirewallHelperClass::FIREWALL_TYPE_NETGEAR_BUG) != 0)) {

					// we have a netgear bug type behavior and the target does not, so we need them to send to us
					// first to avoid having our NAT table reset.

					DEBUG_LOG(("NAT::doThisConnectionRound - Local node has a netgear and the target node does not, need to delay our probe."));
					m_timeTillNextSend = -1;
				}

				// figure out which port number I'm using for this connection
				// this merely starts to talk to the mangler server, we have to keep calling
				// the update function till we get a response.
				DEBUG_LOG(("NAT::doThisConnectionRound - About to attempt to get the next mangled source port"));
				sendMangledSourcePort();
//				m_nextPortSendTime = timeGetTime() + TheGameSpyConfig->getRetryInterval();
				m_nextPortSendTime = timeGetTime() + m_timeBetweenRetries;
//				m_timeoutTime = timeGetTime() + TheGameSpyConfig->getPortTimeout();
				m_timeoutTime = timeGetTime() + m_timeToWaitForPort;
			} else {
				// this is someone else that needs to connect to someone, so wait till they tell us
				// that they're done.
				setConnectionState(i, NATCONNECTIONSTATE_WAITINGFORRESPONSE);
			}
		} else {
			// no one to connect to, so this one is done.
			DEBUG_LOG(("NAT::doThisConnectionRound - node %d has no one to connect to, so they're done", i));
			setConnectionState(i, NATCONNECTIONSTATE_DONE);
		}
	}
}

void NAT::sendAProbe(UnsignedInt ip, UnsignedShort port, Int fromNode) {
	DEBUG_LOG(("NAT::sendAProbe - sending a probe from port %d to %d.%d.%d.%d:%d", getSlotPort(m_connectionNodes[m_localNodeNumber].m_slotIndex),
							PRINTF_IP_AS_4_INTS(ip), port));
	AsciiString str;
	str.format("PROBE%d", fromNode);
	m_transport->queueSend(ip, port, (unsigned char *)str.str(), str.getLength() + 1);
	m_transport->doSend();
}

// find the next mangled source port, and then send it to the other player.
// if this requires talking to the mangler, we'll have to wait till a later update
// to send our port out.
void NAT::sendMangledSourcePort() {
	UnsignedShort sourcePort = getSlotPort(m_connectionNodes[m_localNodeNumber].m_slotIndex);

	FirewallHelperClass::tFirewallBehaviorType fwType = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex]->getNATBehavior();
	GameSlot *targetSlot = m_slotList[m_connectionNodes[m_targetNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(targetSlot != NULL, ("NAT::sendMangledSourcePort - targetSlot is NULL"));
	if (targetSlot == NULL) {
		DEBUG_LOG(("NAT::sendMangledSourcePort - targetSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	GameSlot *localSlot = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(localSlot != NULL, ("NAT::sendMangledSourcePort - localSlot is NULL, WTF?"));
	if (localSlot == NULL) {
		DEBUG_LOG(("NAT::sendMangledSourcePort - localSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	// check to see if the target and I are behind the same NAT
	if (targetSlot->getIP() == localSlot->getIP()) {
#if defined(DEBUG_LOGGING)
		UnsignedInt localip = localSlot->getIP();
		UnsignedInt targetip = targetSlot->getIP();
#endif
		DEBUG_LOG(("NAT::sendMangledSourcePort - target and I are behind the same NAT, no mangling"));
		DEBUG_LOG(("NAT::sendMangledSourcePort - I am %ls, target is %ls, my IP is %d.%d.%d.%d, target IP is %d.%d.%d.%d", localSlot->getName().str(), targetSlot->getName().str(),
								PRINTF_IP_AS_4_INTS(localip),
								PRINTF_IP_AS_4_INTS(targetip)));

		sendMangledPortNumberToTarget(sourcePort, targetSlot);
		m_sourcePorts[m_targetNodeNumber] = sourcePort;
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT);
		// In case you're wondering, we don't set the m_previousSourcePort here because this will be a different source
		// address than what other nodes will likely see (unless of course there are more than
		// two of us behind the same NAT, but we won't worry about that cause theres no mangling
		// in that case anyways)
		return;
	}

	// check to see if we are NAT'd at all.
	if ((fwType == 0) || (fwType == FirewallHelperClass::FIREWALL_TYPE_SIMPLE)) {
		// no mangling, just return the source port
		DEBUG_LOG(("NAT::sendMangledSourcePort - no mangling, just using the source port"));
		sendMangledPortNumberToTarget(sourcePort, targetSlot);
		m_previousSourcePort = sourcePort;
		m_sourcePorts[m_targetNodeNumber] = sourcePort;
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT);
		return;
	}

	// check to see if our NAT keeps the same source port for different destinations.
	// if this is the case, and we've already worked out what our mangled port number is
	// then we don't have to figure it out again.
	if (((fwType & FirewallHelperClass::FIREWALL_TYPE_DESTINATION_PORT_DELTA) == 0) &&
			((fwType & FirewallHelperClass::FIREWALL_TYPE_SMART_MANGLING) == 0)) {
		DEBUG_LOG(("NAT::sendMangledSourcePort - our firewall doesn't NAT based on destination address, checking for old connections from this address"));
		if (m_previousSourcePort != 0) {
			DEBUG_LOG(("NAT::sendMangledSourcePort - Previous source port was %d, using that one", m_previousSourcePort));
			sendMangledPortNumberToTarget(m_previousSourcePort, targetSlot);
			m_sourcePorts[m_targetNodeNumber] = m_previousSourcePort;
			setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT);
			return;
		} else {
			DEBUG_LOG(("NAT::sendMangledSourcePort - Previous source port not found"));
		}
	}

	// At this point we know that our NAT uses some kind of relative port mapping scheme, so we
	// need to talk to the mangler to find out where we are now so we can find out where we'll be on the
	// next port allocation.

	// get the address of the mangler we need to talk to.
	Char manglerName[256];
	FirewallHelperClass::getManglerName(1, manglerName);
	DEBUG_LOG(("NAT::sendMangledSourcePort - about to call gethostbyname for mangler at %s", manglerName));
	struct hostent *hostInfo = gethostbyname(manglerName);

	if (hostInfo == NULL) {
		DEBUG_LOG(("NAT::sendMangledSourcePort - gethostbyname failed for mangler address %s", manglerName));
		// can't find the mangler, we're screwed so just send the source port.
		sendMangledPortNumberToTarget(sourcePort, targetSlot);
		m_sourcePorts[m_targetNodeNumber] = sourcePort;
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT);
		return;
	}

	memcpy(&m_manglerAddress, &(hostInfo->h_addr_list[0][0]), 4);
	m_manglerAddress = ntohl(m_manglerAddress);
	DEBUG_LOG(("NAT::sendMangledSourcePort - mangler %s address is %d.%d.%d.%d", manglerName,
							PRINTF_IP_AS_4_INTS(m_manglerAddress)));

	DEBUG_LOG(("NAT::sendMangledSourcePort - NAT behavior = 0x%08x", fwType));

//	m_manglerRetryTime = TheGameSpyConfig->getRetryInterval() + timeGetTime();
	m_manglerRetryTime = m_manglerRetryTimeInterval + timeGetTime();
	m_manglerRetries = 0;

	if (TheFirewallHelper != NULL) {
		m_spareSocketPort = TheFirewallHelper->getNextTemporarySourcePort(0);
		TheFirewallHelper->openSpareSocket(m_spareSocketPort);
		TheFirewallHelper->sendToManglerFromPort(m_manglerAddress, m_spareSocketPort, m_packetID);
//		m_manglerRetryTime = TheGameSpyConfig->getRetryInterval() + timeGetTime();
		m_manglerRetryTime = m_manglerRetryTimeInterval + timeGetTime();
	}

	setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORMANGLERRESPONSE);
}

void NAT::processManglerResponse(UnsignedShort mangledPort) {
	DEBUG_LOG(("NAT::processManglerResponse - Work out what my NAT'd port will be"));

	GameSlot *targetSlot = m_slotList[m_connectionNodes[m_targetNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(targetSlot != NULL, ("NAT::processManglerResponse - targetSlot is NULL"));
	if (targetSlot == NULL) {
		DEBUG_LOG(("NAT::processManglerResponse - targetSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	Short delta = TheGlobalData->m_firewallPortAllocationDelta;
	UnsignedShort sourcePort = getSlotPort(m_connectionNodes[m_localNodeNumber].m_slotIndex);
	UnsignedShort returnPort = 0;

	FirewallHelperClass::tFirewallBehaviorType fwType = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex]->getNATBehavior();

	if ((fwType & FirewallHelperClass::FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION) != 0) {
		returnPort = mangledPort + delta;
	} else {
		// to steal a line from Steve Tall...
		// Rats. It's a relative mangler. This is much harder. Damn NAT32 guy.
		if (delta == 100) {
			// Special NAT32 section.
			// NAT32 mangles source UDP port by ading 1700 + 100*NAT table index.
			returnPort = mangledPort - m_spareSocketPort;
			returnPort -= 1700;

			returnPort += delta;
			returnPort += sourcePort;
			returnPort += 1700;

		} else if (delta == 0) {
			returnPort = sourcePort;
		} else {
			returnPort = mangledPort / delta;
			returnPort = returnPort * delta;

			returnPort += (sourcePort % delta);
			returnPort += delta;
		}
	}

	// This bit is probably doomed.
	if (returnPort > 65535) {
		returnPort -= 65535;
	}
	if (returnPort < 1024) {
		returnPort += 1024;
	}

	DEBUG_LOG(("NAT::processManglerResponse - mangled port is %d", returnPort));
	m_previousSourcePort = returnPort;

	sendMangledPortNumberToTarget(returnPort, targetSlot);
	m_sourcePorts[m_targetNodeNumber] = returnPort;
	if (targetSlot->getPort() == 0) {
		// we haven't got the target's mangled port number yet, wait for it.
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT);
	} else {
		// in this case we should have already sent a PROBE, so we'll just change the state
		// and leave it at that.
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORRESPONSE);
	}
}

// check to see if we've completed all the rounds
// this is kind of a cheesy way to check, but it works.
Bool NAT::allConnectionsDone() {
	if (m_numNodes == 2) {
		if (m_connectionRound >= 1) {
			return TRUE;
		}
	} else if (m_numNodes == 3) {
		if (m_connectionRound >= 3) {
			return TRUE;
		}
	} else if (m_numNodes == 4) {
		if (m_connectionRound >= 3) {
			return TRUE;
		}
	} else if (m_numNodes == 5) {
		if (m_connectionRound >= 5) {
			return TRUE;
		}
	} else if (m_numNodes == 6) {
		if (m_connectionRound >= 5) {
			return TRUE;
		}
	} else if (m_numNodes == 7) {
		if (m_connectionRound >= 7) {
			return TRUE;
		}
	} else if (m_numNodes == 8) {
		if (m_connectionRound >= 7) {
			return TRUE;
		}
	}
	return FALSE;
}

Bool NAT::allConnectionsDoneThisRound() {
	Bool retval = TRUE;
	for (UnsignedInt i = 0; (i < m_numNodes) && (retval == TRUE); ++i) {
		if ((m_connectionStates[i] != NATCONNECTIONSTATE_DONE) && (m_connectionStates[i] != NATCONNECTIONSTATE_FAILED)) {
			retval = FALSE;
		}
	}
	return retval;
}

// this node's connection for this round has been completed.
void NAT::connectionComplete(Int slotIndex) {
}

// this node's connection for this round has failed.
void NAT::connectionFailed(Int slotIndex) {
}

// I have been probed by the target.
void NAT::probed(Int nodeNumber) {
	GameSlot *localSlot = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(localSlot != NULL, ("NAT::probed - localSlot is NULL, WTF?"));
	if (localSlot == NULL) {
		DEBUG_LOG(("NAT::probed - localSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	if (m_beenProbed == FALSE) {
		m_beenProbed = TRUE;
		DEBUG_LOG(("NAT::probed - just got probed for the first time."));
		if ((localSlot->getNATBehavior() & FirewallHelperClass::FIREWALL_TYPE_NETGEAR_BUG) != 0) {
			DEBUG_LOG(("NAT::probed - we have a NETGEAR and we were just probed for the first time"));
			GameSlot *targetSlot = m_slotList[m_connectionNodes[m_targetNodeNumber].m_slotIndex];
			DEBUG_ASSERTCRASH(targetSlot != NULL, ("NAT::probed - targetSlot is NULL"));
			if (targetSlot == NULL) {
				DEBUG_LOG(("NAT::probed - targetSlot is NULL, failed this connection"));
				setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
				return;
			}

			if (targetSlot->getPort() == 0) {
				setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT);
				DEBUG_LOG(("NAT::probed - still waiting for mangled port"));
			} else {
				DEBUG_LOG(("NAT::probed - sending a probe to %ls", targetSlot->getName().str()));
				sendAProbe(targetSlot->getIP(), targetSlot->getPort(), m_localNodeNumber);
				notifyTargetOfProbe(targetSlot);
				setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORRESPONSE);
			}
		}
	}
}

// got the mangled port for our target for this round.
void NAT::gotMangledPort(Int nodeNumber, UnsignedShort mangledPort) {

	// if we've already finished the connection, then we don't need to process this.
	if (m_connectionStates[m_localNodeNumber] == NATCONNECTIONSTATE_DONE) {
		DEBUG_LOG(("NAT::gotMangledPort - got a mangled port, but we've already finished this connection, ignoring."));
		return;
	}

	GameSlot *targetSlot = m_slotList[m_connectionNodes[m_targetNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(targetSlot != NULL, ("NAT::gotMangledPort - targetSlot is NULL"));
	if (targetSlot == NULL) {
		DEBUG_LOG(("NAT::gotMangledPort - targetSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	GameSlot *localSlot = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(localSlot != NULL, ("NAT::gotMangledPort - localSlot is NULL, WTF?"));
	if (localSlot == NULL) {
		DEBUG_LOG(("NAT::gotMangledPort - localSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	if (nodeNumber != m_targetNodeNumber) {
		DEBUG_LOG(("NAT::gotMangledPort - got a mangled port number for someone that isn't my target. node = %d, target node = %d", nodeNumber, m_targetNodeNumber));
		return;
	}

	targetSlot->setPort(mangledPort);
	DEBUG_LOG(("NAT::gotMangledPort - got mangled port number %d from our target node (%ls)", mangledPort, targetSlot->getName().str()));
	if (((localSlot->getNATBehavior() & FirewallHelperClass::FIREWALL_TYPE_NETGEAR_BUG) == 0) || (m_beenProbed == TRUE) ||
			(((localSlot->getNATBehavior() & FirewallHelperClass::FIREWALL_TYPE_NETGEAR_BUG) != 0) && ((targetSlot->getNATBehavior() & FirewallHelperClass::FIREWALL_TYPE_NETGEAR_BUG) != 0))) {
#ifdef DEBUG_LOGGING
		UnsignedInt ip = targetSlot->getIP();
#endif
		DEBUG_LOG(("NAT::gotMangledPort - don't have a netgear or we have already been probed, or both my target and I have a netgear, send a PROBE. Sending to %d.%d.%d.%d:%d",
								PRINTF_IP_AS_4_INTS(ip), targetSlot->getPort()));

		sendAProbe(targetSlot->getIP(), targetSlot->getPort(), m_localNodeNumber);
		notifyTargetOfProbe(targetSlot);
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_WAITINGFORRESPONSE);
	} else {
		DEBUG_LOG(("NAT::gotMangledPort - we are a netgear, not sending a PROBE yet."));
	}
}

void NAT::gotInternalAddress(Int nodeNumber, UnsignedInt address) {
	GameSlot *targetSlot = m_slotList[m_connectionNodes[nodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(targetSlot != NULL, ("NAT::gotInternalAddress - targetSlot is NULL"));
	if (targetSlot == NULL) {
		return;
	}

	GameSlot *localSlot = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(localSlot != NULL, ("NAT::gotInternalAddress - localSlot is NULL, WTF?"));
	if (localSlot == NULL) {
		return;
	}

	if (nodeNumber != m_targetNodeNumber) {
		DEBUG_LOG(("NAT::gotInternalAddress - got a internal address for someone that isn't my target. node = %d, target node = %d", nodeNumber, m_targetNodeNumber));
		return;
	}

	if (localSlot->getIP() == targetSlot->getIP()) {
		// we have the same IP address, i.e. we are behind the same NAT.
		// I need to talk directly to his internal address.
		DEBUG_LOG(("NAT::gotInternalAddress - target and local players have same external address, using internal address."));
		targetSlot->setIP(address); // use the slot's internal address from now on
	}
}

void NAT::notifyTargetOfProbe(GameSlot *targetSlot) {
	PeerRequest req;
	AsciiString options;
	options.format("PROBED%d", m_localNodeNumber);
	req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
	req.UTM.isStagingRoom = TRUE;
	req.id = "NAT/";
	AsciiString hostName;
	hostName.translate(targetSlot->getName());
	req.nick = hostName.str();
	req.options = options.str();
	TheGameSpyPeerMessageQueue->addRequest(req);
	DEBUG_LOG(("NAT::notifyTargetOfProbe - notifying %ls that we have probed them.", targetSlot->getName().str()));
}

void NAT::notifyUsersOfConnectionDone(Int nodeIndex) {
	GameSlot *localSlot = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(localSlot != NULL, ("NAT::notifyUsersOfConnectionDone - localSlot is NULL, WTF?"));
	if (localSlot == NULL) {
		DEBUG_LOG(("NAT::notifyUsersOfConnectionDone - localSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	PeerRequest req;
	AsciiString options;
	options.format("CONNDONE%d %d", nodeIndex, m_localNodeNumber);

	req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
	req.UTM.isStagingRoom = TRUE;
	req.id = "NAT";
	AsciiString names;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if (m_connectionNodes[m_localNodeNumber].m_slotIndex == i) {
			continue;
		}

		if ((m_slotList[i] == NULL) || (m_slotList[i]->isHuman() == FALSE)) {
			continue;
		}

		AsciiString name;
		name.translate(m_slotList[i]->getName());
		if (names.isNotEmpty())
		{
			names.concat(',');
		}
		names.concat(name);
	}
	req.nick = names.str();
	req.options = options.str();

	DEBUG_LOG(("NAT::notifyUsersOfConnectionDone - sending %s to %s", options.str(), names.str()));
	TheGameSpyPeerMessageQueue->addRequest(req);
}

void NAT::notifyUsersOfConnectionFailed(Int nodeIndex) {
	GameSlot *localSlot = m_slotList[m_connectionNodes[m_localNodeNumber].m_slotIndex];
	DEBUG_ASSERTCRASH(localSlot != NULL, ("NAT::notifyUsersOfConnectionFailed - localSlot is NULL, WTF?"));
	if (localSlot == NULL) {
		DEBUG_LOG(("NAT::notifyUsersOfConnectionFailed - localSlot is NULL, failed this connection"));
		setConnectionState(m_localNodeNumber, NATCONNECTIONSTATE_FAILED);
		return;
	}

	PeerRequest req;
	AsciiString options;
	options.format("CONNFAILED%d", nodeIndex);
/*
	req.peerRequestType = PeerRequest::PEERREQUEST_UTMROOM;
	req.UTM.isStagingRoom = TRUE;
	req.id = "NAT/";
	req.options = options.str();

	DEBUG_LOG(("NAT::notifyUsersOfConnectionFailed - sending %s to room", options.str()));
*/

	req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
	req.UTM.isStagingRoom = TRUE;
	req.id = "NAT";
	AsciiString names;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if (m_connectionNodes[m_localNodeNumber].m_slotIndex == i) {
			continue;
		}

		if ((m_slotList[i] == NULL) || (m_slotList[i]->isHuman() == FALSE)) {
			continue;
		}

		AsciiString name;
		name.translate(m_slotList[i]->getName());
		if (names.isNotEmpty())
		{
			names.concat(',');
		}
		names.concat(name);
	}
	req.nick = names.str();
	req.options = options.str();

	DEBUG_LOG(("NAT::notifyUsersOfConnectionFailed - sending %s to %s", options.str(), names.str()));

	TheGameSpyPeerMessageQueue->addRequest(req);
}

void NAT::sendMangledPortNumberToTarget(UnsignedShort mangledPort, GameSlot *targetSlot) {
	PeerRequest req;
	AsciiString options;
	options.format("PORT%d %d %08X", m_localNodeNumber, mangledPort, m_localIP);

	req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
	req.UTM.isStagingRoom = TRUE;
	req.id = "NAT/";
	AsciiString hostName;
	hostName.translate(targetSlot->getName());
	req.nick = hostName.str();
	req.options = options.str();
	DEBUG_LOG(("NAT::sendMangledPortNumberToTarget - sending \"%s\" to %s", options.str(), hostName.str()));
	TheGameSpyPeerMessageQueue->addRequest(req);
}

void NAT::processGlobalMessage(Int slotNum, const char *options) {
	const char *ptr = options;
	// skip preceding whitespace.
	while (isspace(*ptr)) {
		++ptr;
	}
	DEBUG_LOG(("NAT::processGlobalMessage - got message from slot %d, message is \"%s\"", slotNum, ptr));
	if (!strncmp(ptr, "PROBED", strlen("PROBED"))) {
		// format: PROBED<node number>
		// a probe has been sent at us, if we are waiting because of a netgear or something, we
		// should start sending our own probes.
		Int node = atoi(ptr + strlen("PROBED"));
		if (node == m_targetNodeNumber) {
			// make sure we're being probed by who we're supposed to be probed by.
			probed(node);
		} else {
			DEBUG_LOG(("NAT::processGlobalMessage - probed by node %d, not our target", node));
		}
	} else if (!strncmp(ptr, "CONNDONE", strlen("CONNDONE"))) {
		// format: CONNDONE<node number>
		// we should get the node number of the player who's connection is done from the options
		// and mark that down as part of the connectionStates.
		const char *c = ptr + strlen("CONNDONE");
/*		while (*c != ' ') {
			++c;
		}
		while (*c == ' ') {
			++c;
		}
*/		Int node;
		Int sendingNode;
		sscanf(c, "%d %d\n", &node, &sendingNode);

		if (m_connectionPairs[m_connectionPairIndex][m_connectionRound][node] == sendingNode) {
//			Int node = atoi(ptr + strlen("CONNDONE"));
			DEBUG_LOG(("NAT::processGlobalMessage - got a CONNDONE message for node %d", node));
			if ((node >= 0) && (node <= m_numNodes)) {
				DEBUG_LOG(("NAT::processGlobalMessage - node %d's connection is complete, setting connection state to done", node));
				setConnectionState(node, NATCONNECTIONSTATE_DONE);
			}
		} else {
			DEBUG_LOG(("NAT::processGlobalMessage - got a connection done message that isn't from this round. node: %d sending node: %d", node, sendingNode));
		}
	} else if (!strncmp(ptr, "CONNFAILED", strlen("CONNFAILED"))) {
		// format: CONNFAILED<node number>
		// we should get the node number of the player who's connection failed from the options
		// and mark that down as part of the connectionStates.
		Int node = atoi(ptr + strlen("CONNFAILED"));
		if ((node >= 0) && (node < m_numNodes)) {
			DEBUG_LOG(("NAT::processGlobalMessage - node %d's connection failed, setting connection state to failed", node));
			setConnectionState(node, NATCONNECTIONSTATE_FAILED);
		}
	} else if (!strncmp(ptr, "PORT", strlen("PORT"))) {
		// format: PORT<node number> <port number> <internal IP>
		// we should get the node number and the mangled port number of the client we
		// are supposed to be communicating with and start probing them. No, that was not
		// meant to be a phallic reference, you sicko.
		const char *c = ptr + strlen("PORT");
		Int node = atoi(c);
		while (*c != ' ') {
			++c;
		}
		while (*c == ' ') {
			++c;
		}
		UnsignedInt intport = 0;
		UnsignedInt addr = 0;
		sscanf(c, "%d %X", &intport, &addr);
		UnsignedShort port = (UnsignedShort)intport;

		DEBUG_LOG(("NAT::processGlobalMessage - got port message from node %d, port: %d, internal address: %d.%d.%d.%d", node, port,
								PRINTF_IP_AS_4_INTS(addr)));

		if ((node >= 0) && (node < m_numNodes)) {
			if (port < 1024) {
				// it has to be less than 65535 cause its a short duh.
				DEBUG_ASSERTCRASH(port >= 1024, ("Was passed an invalid port number"));
				port += 1024;
			}
			gotInternalAddress(node, addr);
			gotMangledPort(node, port);
		}
	}
}

void NAT::setConnectionState(Int nodeNumber, NATConnectionState state) {
	m_connectionStates[nodeNumber] = state;

	if (nodeNumber != m_localNodeNumber) {
		return;
	}

	// if this is the case we are starting a new round and we don't know
	// who we're connecting to yet.
	if (m_localNodeNumber == m_targetNodeNumber) {
		return;
	}

	// if this is the start of a new connection round we don't have a
	// target yet.
	if (m_targetNodeNumber == -1) {
		return;
	}

	// find the menu slot of the target node.
	Int slotIndex = m_connectionNodes[m_targetNodeNumber].m_slotIndex;
	Int slot = 0;
	Int i = 0;
	for (; i < MAX_SLOTS; ++i) {
		if (m_slotList[i] != NULL) {
			if (m_slotList[i]->isHuman()) {
				if (i != m_connectionNodes[m_localNodeNumber].m_slotIndex) {
					if (i == slotIndex) {
					 break;
					}
					++slot;
				}
			}
		}
	}
	if (i == MAX_SLOTS) {
		DEBUG_ASSERTCRASH(i < MAX_SLOTS, ("Didn't find the node number in the slot list"));
		return;
	}
	TheEstablishConnectionsMenu->setPlayerStatus(slot, state);
}
