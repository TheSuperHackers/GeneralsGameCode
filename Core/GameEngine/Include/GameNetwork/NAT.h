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

// FILE: NAT.h /////////////////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland April 2002
// Desc:   Resolves NAT'd IPs and port numbers for the other players in a game.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lib/BaseType.h"
#include "GameNetwork/NetworkInterface.h"
#include "GameNetwork/FirewallHelper.h"

class Transport;
class GameSlot;

enum NATStateType CPP_11(: Int) {
	NATSTATE_IDLE,
	NATSTATE_DOCONNECTIONPATHS,
	NATSTATE_WAITFORSTATS,
	NATSTATE_DONE,
	NATSTATE_FAILED
};

enum NATConnectionState CPP_11(: Int) {
	NATCONNECTIONSTATE_NOSTATE,
	NATCONNECTIONSTATE_WAITINGTOBEGIN,
//	NATCONNECTIONSTATE_NETGEARDELAY,
	NATCONNECTIONSTATE_WAITINGFORMANGLERRESPONSE,
	NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT,
	NATCONNECTIONSTATE_WAITINGFORRESPONSE,
	NATCONNECTIONSTATE_DONE,
	NATCONNECTIONSTATE_FAILED
};

struct ConnectionNodeType {
	FirewallHelperClass::tFirewallBehaviorType m_behavior;	///< the NAT/Firewall behavior of this node.
	UnsignedInt m_slotIndex; ///< the player list index of this node.
};

class NAT {
public:
	NAT();
	virtual ~NAT();

	NATStateType update();

	void attachSlotList(GameSlot **slotList, Int localSlot, UnsignedInt localIP);
	void establishConnectionPaths();

	Int getSlotPort(Int slot);
	Transport * getTransport();	///< return the newly created Transport layer that has all the connections and whatnot.

	// Notification messages from GameSpy
	void processGlobalMessage(Int slotNum, const char *options);

protected:
	NATConnectionState connectionUpdate(); ///< the update function for the connections.
	void sendMangledSourcePort(); ///< starts the process to get the next mangled source port.
	void processManglerResponse(UnsignedShort mangledPort);

	Bool allConnectionsDoneThisRound();
	Bool allConnectionsDone();

	void generatePortNumbers(GameSlot **slotList, Int localSlot); ///< generate all of the slots' port numbers to be used.

	void doThisConnectionRound();	///< compute who will connect with who for this round.
	void setConnectionState(Int nodeNumber, NATConnectionState state); ///< central point for changing a connection's state.
	void sendAProbe(UnsignedInt ip, UnsignedShort port, Int fromNode);	///< send a "PROBE" packet to this IP and port.
	void notifyTargetOfProbe(GameSlot *targetSlot);
	void notifyUsersOfConnectionDone(Int nodeIndex);
	void notifyUsersOfConnectionFailed(Int nodeIndex);
	void sendMangledPortNumberToTarget(UnsignedShort mangledPort, GameSlot *targetSlot);

	void probed(Int nodeNumber);
	void gotMangledPort(Int nodeNumber, UnsignedShort mangledPort);
	void gotInternalAddress(Int nodeNumber, UnsignedInt address);
	void connectionComplete(Int slotIndex);
	void connectionFailed(Int slotIndex);

	Transport *m_transport;
	GameSlot **m_slotList;
	NATStateType m_NATState;
	Int m_localNodeNumber;	///< The node number of the local player.
	Int m_targetNodeNumber;	///< The node number of the player we are connecting to this round.
	UnsignedInt m_localIP;	///< The IP of the local computer.
	UnsignedInt m_numNodes;	///< The number of players we have to connect together.
	UnsignedInt m_connectionRound; ///< The "round" of connections we are currently on.

	Int m_numRetries;
	Int m_maxNumRetriesAllowed;

	UnsignedShort m_packetID;
	UnsignedShort m_spareSocketPort;
	time_t m_manglerRetryTime;
	Int m_manglerRetries;
	UnsignedShort m_previousSourcePort;

	Bool m_beenProbed; ///< have I been notified that I've been probed this round?

	UnsignedInt m_manglerAddress;

	time_t m_timeTillNextSend; ///< The number of milliseconds till we send to the other guy's port again.
	NATConnectionState m_connectionStates[MAX_SLOTS]; ///< connection states for this round for all the nodes.

	ConnectionNodeType m_connectionNodes[MAX_SLOTS]; ///< info regarding the nodes that are being connected.

	UnsignedShort m_sourcePorts[MAX_SLOTS]; ///< the source ports that the other players communicate to us on.

	Bool m_myConnections[MAX_SLOTS]; ///< keeps track of all the nodes I've connected to. For keepalive.
	time_t m_nextKeepaliveTime; ///< the next time we will send out our keepalive packets.

	static Int m_connectionPairs[MAX_SLOTS-1][MAX_SLOTS-1][MAX_SLOTS];
	Int m_connectionPairIndex;

	UnsignedShort m_startingPortNumber; ///< the starting port number for this game. The slots all get port numbers with their port numbers based on this number.
																			///< this is done so that games that are played right after each other with the same players in the same
																			///< slot order will not use the old source port allocation scheme in case their NAT
																			///< hasn't timed out that connection.

	time_t m_nextPortSendTime; ///< Last time we sent our mangled port number to our target this round.

	time_t m_timeoutTime; ///< the time at which we will time out waiting for the other player's port number.
	time_t m_roundTimeout;	///< the time at which we will time out this connection round.

 static Int m_timeBetweenRetries; // 1 second between retries sounds good to me.
 static time_t m_manglerRetryTimeInterval; // sounds good to me.
 static Int m_maxAllowedManglerRetries; // works for me.
 static time_t m_keepaliveInterval; // 15 seconds between keepalive packets seems good.
 static time_t m_timeToWaitForPort; // wait for ten seconds for the other player's port number.
 static time_t m_timeForRoundTimeout; // wait for at most ten seconds for each connection round to finish.

};

extern NAT *TheNAT;
