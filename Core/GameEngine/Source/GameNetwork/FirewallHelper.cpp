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


/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /RedAlert2/NAT.cpp                                          $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 3/15/01 12:00PM                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/crc.h"
#include "Common/UserPreferences.h"
#include "GameNetwork/FirewallHelper.h"
#include "GameNetwork/NAT.h"
#include "GameNetwork/udp.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/GameSpy/GSConfig.h"


FirewallHelperClass *TheFirewallHelper = nullptr;

FirewallHelperClass * createFirewallHelper()
{
	return NEW FirewallHelperClass();
}


/***********************************************************************************************
 * FirewallHelperClass::FirewallHelperClass -- Constructor                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 5:03PM ST : Created                                                               *
 *=============================================================================================*/
/* static */ Int FirewallHelperClass::m_sourcePortPool = 4096;

FirewallHelperClass::FirewallHelperClass(void)
{
	m_currentTry = 0;
	m_numManglers = 0;
	m_numResponses = 0;
	m_packetID = 0;
	m_timeoutLength = 0;
	m_timeoutStart = 0;
	m_behavior = FIREWALL_TYPE_UNKNOWN;
	m_lastBehavior = FIREWALL_TYPE_UNKNOWN;
	m_sourcePortAllocationDelta = 0;
	m_lastSourcePortAllocationDelta = 0;
	Int i = 0;
	for (; i < MAX_SPARE_SOCKETS; ++i) {
		m_spareSockets[i].port = 0;
		m_messages[i].length = 0;
		m_mangledPorts[i] = 0;
		m_sparePorts[i] = 0;
	}

	for (i = 0; i < MAX_NUM_MANGLERS; i++)
	{
		m_manglers[i] = 0;
	}

	m_currentState = DETECTIONSTATE_IDLE;

	m_sourcePortPool = 4096 + ((timeGetTime() / 1000) % 1000); // do this to make sure we don't use the same source
																										// port before a previous connection has had a chance
																										// to time out.
}



/***********************************************************************************************
 * FirewallHelperClass::~FirewallHelperClass -- Destructor                                     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/16/02 BGC : Created                                                                     *
 *=============================================================================================*/
FirewallHelperClass::~FirewallHelperClass()
{
	reset();
}

/***********************************************************************************************
 * FirewallHelperClass::Reset -- Cleans out the object                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/29/01 1:04AM ST : Created                                                               *
 *=============================================================================================*/
void FirewallHelperClass::reset(void)
{
	closeAllSpareSockets();
	m_currentState = DETECTIONSTATE_IDLE;
	for (Int i = 0; i < MAX_SPARE_SOCKETS; ++i) {
		m_messages[i].length = 0;
	}
}




/***********************************************************************************************
 * FirewallHelperClass::Detect_Firewall -- See what our firewall is up to                      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 6:47PM ST : Created                                                               *
 *=============================================================================================*/
Bool FirewallHelperClass::detectFirewall(void)
{
	OptionPreferences pref;

	OptionPreferences::const_iterator it = pref.find("FirewallNeedToRefresh");
	if (it != pref.end()) {
		AsciiString str = it->second;
		if (str.compareNoCase("TRUE") == 0) {
			TheWritableGlobalData->m_firewallBehavior = FIREWALL_TYPE_UNKNOWN;
		}
	}

	if (TheWritableGlobalData->m_firewallBehavior == FIREWALL_TYPE_UNKNOWN) {
		detectFirewallBehavior();

		return FALSE;
	} else {
		DEBUG_LOG(("FirewallHelperClass::detectFirewall - firewall behavior already specified as %d, port allocation delta is %d, skipping detection.", TheWritableGlobalData->m_firewallBehavior, TheWritableGlobalData->m_firewallPortAllocationDelta));
	}

	return TRUE;
}


Bool FirewallHelperClass::behaviorDetectionUpdate()
{
	switch (m_currentState) {
		case DETECTIONSTATE_IDLE:
			return FALSE;
			
		case DETECTIONSTATE_DONE:
			return TRUE;
			
		case DETECTIONSTATE_BEGIN:
			return detectionBeginUpdate();
			
		case DETECTIONSTATE_TEST1:
			return detectionTest1Update();
			
		case DETECTIONSTATE_TEST2:
			return detectionTest2Update();
			
		case DETECTIONSTATE_TEST3:
			return detectionTest3Update();
			
		case DETECTIONSTATE_TEST3_WAITFORRESPONSES:
			return detectionTest3WaitForResponsesUpdate();
			
		case DETECTIONSTATE_TEST4_1:
			return detectionTest4Stage1Update();
			
		case DETECTIONSTATE_TEST4_2:
			return detectionTest4Stage2Update();
			
		case DETECTIONSTATE_TEST5:
			return detectionTest5Update();
			
		default:
			return TRUE;
	}
}

/***********************************************************************************************
 * FHC::getNextTemporarySourcePort -- Get a throwaway source port for temporary use        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    number of ports in sequence to skip                                               *
 *                                                                                             *
 * OUTPUT:   port number                                                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:06PM ST : Created                                                              *
 *=============================================================================================*/
UnsignedShort FirewallHelperClass::getNextTemporarySourcePort(Int skip)
{
	UnsignedShort return_port = (UnsignedShort) m_sourcePortPool;

	/*
	** Try max 256 ports until we find one we can bind to a socket.
	*/
	Int tries = 256;
	if (skip == 0) {
		skip = 1;
	}

	while (tries--) {

		m_sourcePortPool += skip;
		return_port = (UnsignedShort) m_sourcePortPool;

		if (m_sourcePortPool > 65535) {
			m_sourcePortPool = 2048;
		}

		/*
		** Validate the port by trying to bind it to a socket.
		*/
		Bool result = openSpareSocket(return_port);

		if (result) {
			closeSpareSocket(return_port);
			return(return_port);
		} else {
			DEBUG_LOG(("FirewallHelperClass::getNextTemporarySourcePort - failed to open socket on port %d", return_port));
		}
	}

	return(return_port);

}





/***********************************************************************************************
 * FHC::sendToManglerFromPort -- Send to the mangler from the specified port               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Address of mangler server                                                         *
 *           Source port to send *from*                                                        *
 *                                                                                             *
 * OUTPUT:   True if sent OK                                                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:47PM ST : Created                                                              *
 *=============================================================================================*/
Bool FirewallHelperClass::sendToManglerFromPort(UnsignedInt address, UnsignedShort port, UnsignedShort packetID, Bool blitzme)
{
	DEBUG_LOG(("sizeof(ManglerMessage) == %d, sizeof(ManglerData) == %d",
		sizeof(ManglerMessage), sizeof(ManglerData)));

	/*
	** Build the packet to send out.
	*/
	ManglerMessage packet;
	memset(&(packet.data),0x44, sizeof(ManglerData));
	packet.data.NetCommandType = 12; // mangler request.
	packet.data.PacketID = packetID;
	if (blitzme) {
		packet.data.BlitzMe = 1;
	} else {
		packet.data.BlitzMe = 0;
	}
	packet.data.magic = GENERALS_MAGIC_NUMBER;
	packet.data.OriginalPortNumber = port;
/*
	DEBUG_LOG_RAW(("Pre-Adjust Buffer = "));
	for (Int i = 0; i < sizeof(ManglerData); ++i) {
		DEBUG_LOG_RAW(("%02x", *(((unsigned char *)(&(packet.data))) + i)));
	}
	DEBUG_LOG_RAW(("\n"));
*/
	byteAdjust(&(packet.data));
/*
	DEBUG_LOG_RAW(("Pre-CRC Buffer = "));
	for (i = 0; i < sizeof(ManglerData); ++i) {
		DEBUG_LOG_RAW(("%02x", *(((unsigned char *)(&(packet.data))) + i)));
	}
	DEBUG_LOG_RAW(("\n"));
*/
	CRC crc;
	crc.computeCRC((unsigned char *)(&(packet.data.magic)), sizeof(ManglerData) - sizeof(unsigned int));
	packet.data.CRC = htonl(crc.get());

	packet.length = sizeof(ManglerData);

	DEBUG_LOG(("FirewallHelperClass::sendToManglerFromPort - Sending from port %d to %d.%d.%d.%d:%d", (UnsignedInt)port,
		PRINTF_IP_AS_4_INTS(address), MANGLER_PORT));
/*
	DEBUG_LOG_RAW(("Buffer = "));
	for (i = 0; i < sizeof(ManglerData); ++i) {
		DEBUG_LOG_RAW(("%02x", *(((unsigned char *)(&(packet.data))) + i)));
	}
	DEBUG_LOG_RAW(("\n"));
*/
	SpareSocketStruct *spareSocket = findSpareSocketByPort(port);
//	DEBUG_LOG(("PacketID = %u", packetID));
//	DEBUG_LOG(("OriginalPortNumber = %u", port));

	if (spareSocket == nullptr) {
		DEBUG_ASSERTCRASH(spareSocket != nullptr, ("Could not find spare socket for send."));
		DEBUG_LOG(("FirewallHelperClass::sendToManglerFromPort - failed to find the spare socket for port %d", port));
		return FALSE;
	}

	spareSocket->udp->Write((UnsignedByte *) &packet, sizeof(ManglerData), address, MANGLER_PORT);

	return(TRUE);
}


SpareSocketStruct * FirewallHelperClass::findSpareSocketByPort(UnsignedShort port) {
	DEBUG_LOG(("FirewallHelperClass::findSpareSocketByPort - trying to find spare socket with port %d", port));
	for (Int i = 0; i < MAX_SPARE_SOCKETS; ++i) {
		if (m_spareSockets[i].port == port) {
			DEBUG_LOG(("FirewallHelperClass::findSpareSocketByPort - found it!"));
			return &(m_spareSockets[i]);
		}
	}

	DEBUG_LOG(("FirewallHelperClass::findSpareSocketByPort - didn't find it"));
	return nullptr;
}

ManglerMessage * FirewallHelperClass::findEmptyMessage() {
	for (Int i = 0; i < MAX_SPARE_SOCKETS; ++i) {
		if (m_messages[i].length == 0) {
			return &(m_messages[i]);
		}
	}
	return nullptr;
}

void FirewallHelperClass::byteAdjust(ManglerData *data) {
//	for (Int i = 0; i < len/4; ++i) {
//		*buf = htonl(*buf);
//		++buf;
//	}
	data->CRC = htonl(data->CRC);
	data->magic = htons(data->magic);
	data->MyMangledPortNumber = htons(data->MyMangledPortNumber);
	data->OriginalPortNumber = htons(data->OriginalPortNumber);
	data->PacketID = htons(data->PacketID);
}

/***********************************************************************************************
 * FHC::Get_Mangler_Response_On_Port -- Get the manglers response to a specific query           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Packet id of packet we are looking for                                            *
 *                                                                                             *
 * OUTPUT:   Port the mangler saw this packet come from.                                       *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:51PM ST : Created                                                              *
 *=============================================================================================*/
UnsignedShort FirewallHelperClass::getManglerResponse(UnsignedShort packetID, Int time)
{
	ManglerMessage *msg = nullptr;

//	SpareSocketStruct *spareSocket = nullptr;

	sockaddr_in addr;

	Int i = 0;
	for (; i < MAX_SPARE_SOCKETS; ++i) {
		if (m_spareSockets[i].udp != nullptr) {
			ManglerMessage *message = findEmptyMessage();
			if (message == nullptr) {
				break;
			}
			Int retval = m_spareSockets[i].udp->Read((unsigned char *)message, sizeof(ManglerData), &addr);
			if (retval > 0) {
				CRC crc;
				crc.computeCRC((unsigned char *)(&(message->data.magic)), sizeof(ManglerData) - sizeof(unsigned int));
				if (crc.get() != htonl(message->data.CRC)) {
					DEBUG_LOG(("FirewallHelperClass::getManglerResponse - Saw message, CRC mismatch.  Expected CRC %u, computed CRC %u", message->data.CRC, crc.get()));
					continue;
				}
				byteAdjust(&(message->data));
				message->length = retval;
				DEBUG_LOG(("FirewallHelperClass::getManglerResponse - Saw message of %d bytes from mangler %d on port %u", retval, i, m_spareSockets[i].port));
				DEBUG_LOG(("FirewallHelperClass::getManglerResponse - Message has packet ID %d 0x%08X, looking for packet id %d 0x%08X", message->data.PacketID, message->data.PacketID, packetID, packetID));
				if (message->data.PacketID == packetID) {
					DEBUG_LOG(("FirewallHelperClass::getManglerResponse - packet ID's match, returning message"));
					msg = message;
					message->length = 0;
				}
				if (ntohs(message->data.PacketID) == packetID) {
					DEBUG_LOG(("FirewallHelperClass::getManglerResponse - NETWORK BYTE ORDER packet ID's match, returning message"));
					msg = message;
					message->length = 0;
				}
			}
		}
	}

	// See if we have already received it and saved it.
	if (msg == nullptr) {
		for (i = 0; i < MAX_SPARE_SOCKETS; ++i) {
			if ((m_messages[i].length != 0) && (m_messages[i].data.PacketID == packetID)) {
				msg = &(m_messages[i]);
				msg->length = 0;
			}
		}
	}

	if (msg == nullptr) {
		return 0;
	}

	UnsignedShort mangled_port = msg->data.MyMangledPortNumber;
	DEBUG_LOG(("Mangler is seeing packets from port %d as coming from port %d", (UnsignedInt)msg->data.OriginalPortNumber, (UnsignedInt)mangled_port));
	return mangled_port;
}




/***********************************************************************************************
 * FirewallHelperClass::Write_Firewall_Settings -- Save out firewall settings.                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/22/01 10:23PM ST : Created                                                              *
 *=============================================================================================*/
void FirewallHelperClass::writeFirewallBehavior(void)
{
	OptionPreferences pref;

	char num[16];
	num[0] = 0;
	itoa(TheGlobalData->m_firewallBehavior, num, 10);
	AsciiString numstr;
	numstr = num;
	(pref)["FirewallBehavior"] = numstr;

	TheWritableGlobalData->m_firewallPortAllocationDelta = getSourcePortAllocationDelta();
	num[0] = 0;
	itoa(TheGlobalData->m_firewallPortAllocationDelta, num, 10);
	numstr = num;
	(pref)["FirewallPortAllocationDelta"] = numstr;

	pref.write();
}


/***********************************************************************************************
 * FirewallHelperClass::flagNeedToRefresh -- Flag that the next time we log in we need to      *
 *    refresh our firewall settings.                                                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    flag - whether or not to refresh...munkee                                         *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/19/03 4:30PM BGC : Created                                                              *
 *=============================================================================================*/
void FirewallHelperClass::flagNeedToRefresh(Bool flag)
{
	OptionPreferences pref;

	(pref)["FirewallNeedToRefresh"] = flag ? "TRUE" : "FALSE";

	pref.write();
}


/***********************************************************************************************
 * FirewallHelperClass::Read_Firewall_Behavior -- Read in old firewall settings                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/22/01 10:25PM ST : Created                                                              *
 *=============================================================================================*/
void FirewallHelperClass::readFirewallBehavior(void)
{
#if (0)
	m_lastBehavior = (FirewallBehaviorType) ConfigINI.Get_Int("MultiPlayer", "FirewallSettings", FIREWALL_UNKNOWN);
	m_lastSourcePortAllocationDelta = ConfigINI.Get_Int("MultiPlayer", "FirewallDelta", 1);
#endif //(0)
}



/***********************************************************************************************
 * FHC::detectFirewallBehavior -- What is that wacky firewall doing to our packet headers?   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Firewall behavior                                                                 *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 12:30PM ST : Created                                                              *
 *=============================================================================================*/
void FirewallHelperClass::detectFirewallBehavior(/*Bool &canRecord*/)
{
	m_behavior = FIREWALL_TYPE_SIMPLE;

	m_currentState = DETECTIONSTATE_BEGIN;
}

FirewallHelperClass::FirewallBehaviorType FirewallHelperClass::getFirewallBehavior() {
	m_currentState = DETECTIONSTATE_IDLE;
	return m_behavior;
}

Short FirewallHelperClass::getSourcePortAllocationDelta() {
	return m_sourcePortAllocationDelta;
}

/* static */ void FirewallHelperClass::getManglerName(Int manglerIndex, Char *nameBuf)
{
	AsciiString host;
	UnsignedShort port;
	TheGameSpyConfig->getManglerLocation(manglerIndex, host, port);
	strcpy(nameBuf, host.str());
}

Bool FirewallHelperClass::detectionBeginUpdate() {
//	UnsignedShort mangler_port = MANGLER_PORT;
	 m_packetID = 0x7f00;
	//int current_mangler = 0;

	/*
	** Well, we are going to need some manglers.
	*/

	UnsignedByte mangler_addresses[4][4];

//	Int delta = 0;


	/*
	** If the user specified a particular port to use then we act as if there is no firewall.
	*/
	if (TheWritableGlobalData->m_firewallPortOverride != 0) {
		m_behavior = FIREWALL_TYPE_SIMPLE;
		DEBUG_LOG(("Source port %d specified by user", TheGlobalData->m_firewallPortOverride));

		if (TheGlobalData->m_firewallSendDelay) {
			UnsignedInt addbehavior = FIREWALL_TYPE_NETGEAR_BUG;
			addbehavior |= (UnsignedInt)m_behavior;
			m_behavior = (FirewallBehaviorType) addbehavior;
			DEBUG_LOG(("Netgear bug specified by command line or SendDelay flag"));
		}
		m_currentState = DETECTIONSTATE_DONE;
		return TRUE;
	}



	m_timeoutStart = timeGetTime();
	m_timeoutLength = 5000;
	DEBUG_LOG(("About to call gethostbyname for the mangler address"));
	int namenum = 0;

	do {
		AsciiString host;
		UnsignedShort port;
		TheGameSpyConfig->getManglerLocation(namenum, host, port);
		const char *mangler_name_ptr = host.str();
		DEBUG_LOG(("Looking at %s:%d", host.str(), port));

		/*
		** Use the wolapi supplied mangler info if available.
		*/
//		if (NumManglerServers > namenum) {
//			mangler_name_ptr = &ManglerServerAddress[namenum][0];
//			mangler_port = ManglerServerPort[namenum];
			//current_mangler = CurrentManglerServer;
//			DEBUG_LOG(("Using mangler from servserv"));
//		}
		namenum++;

		if (strlen(mangler_name_ptr) == 0) {
			break;
		}

		/*
		** Do the lookup.
		*/
		struct hostent *host_info = gethostbyname(mangler_name_ptr);

		if (!host_info) {
			DEBUG_LOG(("gethostbyname failed! Error code %d", WSAGetLastError()));
			break;
		}

		/*
		** See if we already have that address in the list.
		*/
		Bool found = FALSE;
		for (Int i=0 ; i<m_numManglers; i++) {
			if (memcmp(mangler_addresses[i], &host_info->h_addr_list[0][0], 4) == 0) {
				found = TRUE;
				break;
			}
		}
		/*
		** Add the address in if we didn't find it.
		*/
		if (!found) {
			Int m = m_numManglers++;
			memcpy(&mangler_addresses[m][0], &host_info->h_addr_list[0][0], 4);
			ntohl((UnsignedInt)mangler_addresses[m]);
			DEBUG_LOG(("Found mangler address at %d.%d.%d.%d", mangler_addresses[m][0], mangler_addresses[m][1], mangler_addresses[m][2], mangler_addresses[m][3]));
		}

	} while ((m_numManglers < MAX_NUM_MANGLERS) && ((timeGetTime() - m_timeoutStart) < m_timeoutLength));


	DEBUG_ASSERTCRASH(m_numManglers > 2, ("not enough mangler addresses found."));
	if (m_numManglers < 3) {
		m_currentState = DETECTIONSTATE_DONE;
		return TRUE;
	}

	for (Int i=0 ; i<m_numManglers ; i++) {
		UnsignedInt temp = 0;
		temp = mangler_addresses[i][3];
		temp += mangler_addresses[i][2] << 8;
		temp += mangler_addresses[i][1] << 16;
		temp += mangler_addresses[i][0] << 24;
		m_manglers[i] = temp;
//		memcpy(&(m_manglers[i]), &mangler_addresses[i][0], 4);
	}

	DEBUG_LOG(("FirewallHelperClass::detectionBeginUpdate - Testing for Netgear bug"));

	/*
	** See if the user specified a netgear firewall - that will save us the trouble.
	*/
	if (TheGlobalData->m_firewallSendDelay) {
		UnsignedInt addbehavior = FIREWALL_TYPE_NETGEAR_BUG;
		addbehavior |= (UnsignedInt)m_behavior;
		m_behavior = (FirewallBehaviorType) addbehavior;
		DEBUG_LOG(("FirewallHelperClass::detectionBeginUpdate - Netgear bug specified by command line or SendDelay flag"));
	} else {
		DEBUG_LOG(("FirewallHelperClass::detectionBeginUpdate - Netgear bug not specified"));
	}

	/*
	** OK, we have our manglers.
	**
	** First test, see if there is any port mangling at all.
	**
	**
	**
	*/

	DEBUG_LOG(("About to start mangler test 1"));
	/*
	** Get a spare port number and create a new socket to bind it to.
	*/
	m_sparePorts[0] = getNextTemporarySourcePort(0);
	if (!openSpareSocket(m_sparePorts[0])) {
		m_currentState = DETECTIONSTATE_DONE;
		return TRUE;
	}

	/*
	** Send to the mangler from this port until we get a response.
	*/
	m_timeoutStart = timeGetTime();
	m_timeoutLength = 6000;

	sendToManglerFromPort(m_manglers[0], m_sparePorts[0], m_packetID);
	m_currentState = DETECTIONSTATE_TEST1;
	return FALSE;
}


Bool FirewallHelperClass::detectionTest1Update() {

	m_mangledPorts[0] = getManglerResponse(m_packetID);

	/*
	** See if we got no response or a non-mangled response.
	*/
	if (m_mangledPorts[0] == 0 || m_mangledPorts[0] == m_sparePorts[0]) {
		if (m_mangledPorts[0] == m_sparePorts[0]) {
			m_sourcePortAllocationDelta = 0;
			DEBUG_LOG(("FirewallHelperClass::detectionTest1Update - Non-mangled response from mangler, quitting test."));
		}
		if ((m_mangledPorts[0] == 0) && ((timeGetTime() - m_timeoutStart) < m_timeoutLength)) {
			// we are still waiting for a response and haven't timed out yet.
			DEBUG_LOG(("FirewallHelperClass::detectionTest1Update - waiting for response from mangler."));
			return FALSE;
		}
		if ((m_mangledPorts[0] == 0) && ((timeGetTime() - m_timeoutStart) >= m_timeoutLength)) {
			// we are still waiting for a response and we timed out.
			DEBUG_LOG(("FirewallHelperClass::detectionTest1Update - timed out waiting for response from mangler."));
		}
		// either we have received a non-mangled response or we timed out waiting for a response.
		closeSpareSocket(m_sparePorts[0]);

		m_currentState = DETECTIONSTATE_DONE;
		return TRUE;
	}

	DEBUG_LOG(("FirewallHelperClass::detectionTest1Update - test 1 complete"));
	/*
	** Test one completed, time to start up the second test.
	**
	** Second test. See if the ports are mangled differently for different destination IPs.
	**
	** We can use the spare socket from the last test and send to a different mangler.
	**
	*/

	/*
	** Send to the mangler from this port until we get a response.
	*/
	m_timeoutStart = timeGetTime();
	m_timeoutLength = 6000;
	m_mangledPorts[1] = 0;
	sendToManglerFromPort(m_manglers[1], m_sparePorts[0], m_packetID+1);

	m_currentState = DETECTIONSTATE_TEST2;
	return FALSE;
}

Bool FirewallHelperClass::detectionTest2Update() {

	m_mangledPorts[1] = getManglerResponse(m_packetID+1);

	if (m_mangledPorts[1] == 0) {
		if ((timeGetTime() - m_timeoutStart) <= m_timeoutLength) {
			return FALSE;
		}
		DEBUG_LOG(("FirewallHelperClass::detectionTest2Update - timed out waiting for mangler response"));
		m_currentState = DETECTIONSTATE_DONE;
		return TRUE;
	}

	/*
	** We are done with this socket/port
	*/
	closeSpareSocket(m_sparePorts[0]);

	/*
	** See if we got no response or a non-mangled response.
	*/
	if (m_mangledPorts[1] == 0 || m_mangledPorts[1] == m_sparePorts[0]) {
		m_currentState = DETECTIONSTATE_DONE;
		UnsignedInt addBehavior = (UnsignedInt)FIREWALL_TYPE_SIMPLE;
		addBehavior |= (UnsignedInt)m_behavior;
		m_behavior = (FirewallBehaviorType)addBehavior;

		if (m_mangledPorts[1] == 0) {
			DEBUG_LOG(("FirewallHelperClass::detectionTest2Update - got no response from mangler"));
		} else {
			DEBUG_LOG(("FirewallHelperClass::detectionTest2Update - got a mangler response, no port mangling"));
		}
		DEBUG_LOG(("FirewallHelperClass::detectionTest2Update - Setting behavior to SIMPLE, done testing"));
		return TRUE;
	}

	if (m_mangledPorts[0] == m_mangledPorts[1]) {
		DEBUG_LOG(("FirewallHelperClass::detectionTest2Update - port mangling doesn't depend on destination IP, setting to DUMB_MANGLING"));
		UnsignedInt addBehavior = (UnsignedInt)FIREWALL_TYPE_DUMB_MANGLING;
		addBehavior |= (UnsignedInt)m_behavior;
		m_behavior = (FirewallBehaviorType)addBehavior;
	} else {
		DEBUG_LOG(("FirewallHelperClass::detectionTest2Update - port mangling depends on destination IP, setting to SMART_MANGLING"));
		UnsignedInt addBehavior = (UnsignedInt)FIREWALL_TYPE_SMART_MANGLING;
		addBehavior |= (UnsignedInt)m_behavior;
		m_behavior = (FirewallBehaviorType)addBehavior;
	}




	/*
	** Third test.
	**
	** This test tries to detect a pattern in the ports allocated by the NAT.
	** We use several source ports for this one.
	**
	*/

	m_currentTry = 0;
	m_packetID = m_packetID + 10;

	DEBUG_LOG(("FirewallHelperClass::detectionTest2Update - moving on to 3rd test"));

	m_currentState = DETECTIONSTATE_TEST3;
	return FALSE;
}

Bool FirewallHelperClass::detectionTest3Update() {
	/*
	** Try this whole thing a max of 3 times.
	*/
	if (m_currentTry < 3) {
		memset(m_sparePorts, 0, sizeof(m_sparePorts));
		memset(m_mangledPorts, 0, sizeof(m_mangledPorts));

		/*
		** Open a socket for each source port.
		** We should use a non-linear set of source ports so we can detect the NAT32 relative offset
		** case.
		*/
		Int i=0;
		for (; i<NUM_TEST_PORTS ; i++) {
			m_sparePorts[i] = getNextTemporarySourcePort(i);
			if (!openSpareSocket(m_sparePorts[i])) {

				/*
				** Close any spare ports we allocated already before we bail.
				*/
				for (Int j=0 ; j<i ; j++) {
					if (m_sparePorts[j]) {
						closeSpareSocket(m_sparePorts[j]);
					}
				}
				DEBUG_LOG(("FirewallHelperClass::detectionTest3Update - Failed to open all the spare sockets, bailing test"));
				m_currentState = DETECTIONSTATE_DONE;
				return TRUE;
			}
		}

		/*
		** OK, lets get some numbers from the mangler.
		**
		** Keep sending from each port until we get a response to all the sends. There's a implied
		** delay between initial sends due to the timeout in Get_Mangler_Response.
		*/
		m_timeoutStart = timeGetTime();
		m_timeoutLength = 12000;

		DEBUG_LOG(("FirewallHelperClass::detectionTest3Update - Sending to %d manglers", NUM_TEST_PORTS));
		for (i=0 ; i<NUM_TEST_PORTS ; i++) {
			if (m_mangledPorts[i] == 0) {
				sendToManglerFromPort(m_manglers[0], m_sparePorts[i], m_packetID+i);
			}
		}

		m_numResponses = 0;

		m_currentState = DETECTIONSTATE_TEST3_WAITFORRESPONSES;
		DEBUG_LOG(("FirewallHelperClass::detectionTest3Update - Waiting for mangler responses"));
		return FALSE;
	}

	DEBUG_LOG(("FirewallHelperClass::detectionTest3Update - Failed to complete test, quitting"));
	m_currentState = DETECTIONSTATE_DONE;
	return TRUE;
}

Bool FirewallHelperClass::detectionTest3WaitForResponsesUpdate() {
	Int i = 0;
	for (; i < NUM_TEST_PORTS; ++i) {
		if (m_mangledPorts[i] == 0) {
			m_mangledPorts[i] = getManglerResponse(m_packetID + i);
			if (m_mangledPorts[i] != 0) {
				++m_numResponses;
			}
		}
	}

	if (m_numResponses < NUM_TEST_PORTS) {
		if ((timeGetTime() - m_timeoutStart) > m_timeoutLength) {
			/*
			** Close down those sockets - we are finished with them.
			*/
			for (Int j=0 ; j<i ; j++) {
				if (m_spareSockets[j].port != 0) {
					closeSpareSocket(m_spareSockets[j].port);
				}
			}
			DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - timed out waiting, bailing test"));
			m_currentState = DETECTIONSTATE_DONE;
			return TRUE;
		}
		return FALSE;
	}

	/*
	** Close down those sockets - we are finished with them.
	*/
	for (Int j=0 ; j<i ; j++) {
		if (m_spareSockets[j].port != 0) {
			closeSpareSocket(m_spareSockets[j].port);
		}
	}

	/*
	** We need at least 4 responses to be sure of the port allocation scheme.
	*/
	if (m_numResponses < 4) {
		if (m_lastSourcePortAllocationDelta != 0 && (int)m_lastBehavior > (int)FIREWALL_TYPE_SIMPLE) {
			/*
			** If the delta we got last time we played looks good then use that.
			*/
			m_sourcePortAllocationDelta = m_lastSourcePortAllocationDelta;
		}
		DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - didn't get enough responses, using %d as the source port allocation delta, finished test", m_sourcePortAllocationDelta));
		m_currentState = DETECTIONSTATE_DONE;
		return TRUE;
	}


	Bool relative_delta = FALSE;
	Bool looks_good = FALSE;
	Int delta = getNATPortAllocationScheme(m_numResponses, m_sparePorts, m_mangledPorts, relative_delta, looks_good);
	DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - getNATPortAllocationScheme returned %d", delta));

	if (delta) {

		/*
		** Hey, we got it!
		*/
		UnsignedInt addbehavior = 0;
		if (relative_delta) {
			DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - detected RELATIVE PORT ALLOCATION"));
			addbehavior = (UnsignedInt)FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION;
		} else {
			DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - detected SIMPLE PORT ALLOCATION"));
			addbehavior = (UnsignedInt)FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION;
		}
		addbehavior |= (UnsignedInt)m_behavior;
		m_behavior = (FirewallBehaviorType) addbehavior;

		m_sourcePortAllocationDelta = delta;
		DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - setting source port delta to %d", delta));
	} else {
		DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - didn't get a delta value"));
		if (m_lastSourcePortAllocationDelta != 0 && (Int)m_lastBehavior > (Int)FIREWALL_TYPE_SIMPLE) {
			/*
			** If the delta we got last time we played looks good then use that.
			*/
			DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - using the port allocation delta we have from before which is %d", m_lastSourcePortAllocationDelta));
			m_sourcePortAllocationDelta = m_lastSourcePortAllocationDelta;
		}
		++m_currentTry;
		m_currentState = DETECTIONSTATE_TEST3;
		return FALSE;
	}

	DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForRepsonsesUpdate - starting 4th test"));
	/*
	** Fourth test.
	**
	** Test to see if the NAT mangles differently per destination port at the same IP.
	*/
	if ((m_behavior & FIREWALL_TYPE_SMART_MANGLING) != 0) {

		if ((m_behavior & FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION) != 0) {

			DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForRepsonsesUpdate - simple port allocation, Testing to see if the NAT mangles differently per destination port at the same IP"));

			/*
			** We need 2 source ports for this.
			*/
			m_sparePorts[0] = getNextTemporarySourcePort(0);
			if (!openSpareSocket(m_sparePorts[0])) {
				m_currentState = DETECTIONSTATE_DONE;
				DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForRepsonsesUpdate - Failed to open first spare port, bailing"));
				return TRUE;
			}

			m_sparePorts[1] = getNextTemporarySourcePort(0);
			if (!openSpareSocket(m_sparePorts[1])) {
				closeSpareSocket(m_sparePorts[0]);
				m_currentState = DETECTIONSTATE_DONE;
				DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForRepsonsesUpdate - Failed to open second spare port, bailing"));
				return TRUE;
			}

			/*
			** Get a reference port.
			*/
			m_timeoutStart = timeGetTime();
			m_timeoutLength = 4000;
			m_mangledPorts[0] = 0;
			m_packetID += 10;

			/*
			** Wait for a response.
			*/
			sendToManglerFromPort(m_manglers[0], m_sparePorts[0], m_packetID);

			m_currentState = DETECTIONSTATE_TEST4_1;
			return FALSE;
		} else {
			/*
			** NAT32 uses different mangled source ports for different destination ports.
			*/
			DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForRepsonsesUpdate - relative port allocation, NAT32 right?"));
			UnsignedInt addbehavior = 0;
			addbehavior = (UnsignedInt)FIREWALL_TYPE_DESTINATION_PORT_DELTA;
			DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForRepsonsesUpdate - adding DESTINATION PORT DELTA to behavior"));
			addbehavior |= (UnsignedInt)m_behavior;
			m_behavior = (FirewallBehaviorType) addbehavior;
		}
	} else {
		DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForResponsesUpdate - We don't have smart mangling, skipping test 4, entering test 5"));
	}

	DEBUG_LOG(("FirewallHelperClass::detectionTest3WaitForRepsonsesUpdate - entering test 5"));

	m_currentState = DETECTIONSTATE_TEST5;
	return FALSE;
}



Bool FirewallHelperClass::detectionTest4Stage1Update() {
	m_mangledPorts[0] = getManglerResponse(m_packetID);

	if (m_mangledPorts[0] == 0) {
		if ((timeGetTime() - m_timeoutStart) > m_timeoutLength) {
			closeSpareSocket(m_sparePorts[0]);
			closeSpareSocket(m_sparePorts[1]);
			m_currentState = DETECTIONSTATE_DONE;
			DEBUG_LOG(("FirewallHelperClass::detectionTest4Stage1Update - timed out waiting for mangler response, quitting"));
			return TRUE;
		}
		return FALSE;
	}

	/*
	** Send out to a different port at that IP.
	** We won't get a response for this.
	*/
	UnsignedInt addr = m_manglers[0];
	UnsignedShort port1 = m_sparePorts[0] + 1;
	sendToManglerFromPort(addr, port1, m_packetID);
	sendToManglerFromPort(addr, port1, m_packetID);
	sendToManglerFromPort(addr, port1, m_packetID);

	/*
	** We can't get a response from a different destination port so the only way to detect
	** this behavior is to check the next mangled port allocation to see if it's double
	** what we would normally expect.
	*/
	m_packetID++;
	m_timeoutStart = timeGetTime();
	m_timeoutLength = 4000;

	sendToManglerFromPort(m_manglers[0], m_sparePorts[1], m_packetID);

	m_currentState = DETECTIONSTATE_TEST4_2;
	return FALSE;
}

Bool FirewallHelperClass::detectionTest4Stage2Update() {
	m_mangledPorts[1] = getManglerResponse(m_packetID);

	if (m_mangledPorts[1] == 0) {
		if ((timeGetTime() - m_timeoutStart) > m_timeoutLength) {
			closeSpareSocket(m_sparePorts[0]);
			closeSpareSocket(m_sparePorts[1]);
			m_currentState = DETECTIONSTATE_DONE;
			DEBUG_LOG(("FirewallHelperClass::detectionTest4Stage2Update - timed out waiting for the second mangler response, quitting"));
			return TRUE;
		}
		return FALSE;
	}

	if (m_mangledPorts[1] != m_mangledPorts[0] + m_sourcePortAllocationDelta) {
		DEBUG_LOG(("FirewallHelperClass::detectionTest4Stage2Update - NAT uses different source ports for different destination ports"));

		UnsignedInt addbehavior = 0;
		addbehavior = (UnsignedInt)FIREWALL_TYPE_DESTINATION_PORT_DELTA;
		addbehavior |= (UnsignedInt)m_behavior;
		m_behavior = (FirewallBehaviorType) addbehavior;
	} else {
		DEBUG_ASSERTCRASH(m_mangledPorts[1] == m_mangledPorts[0] + m_sourcePortAllocationDelta, ("Problem getting the source port deltas."));
		if (m_mangledPorts[1] == m_mangledPorts[0] + m_sourcePortAllocationDelta) {
			DEBUG_LOG(("FirewallHelperClass::detectionTest4Stage2Update - NAT uses the same source port for different destination ports"));
		} else {
			DEBUG_LOG(("FirewallHelperClass::detectionTest4Stage2Update - Unable to complete destination port mangling test"));
			DEBUG_CRASH(("Unable to complete destination port mangling test"));
		}
	}

	m_currentState = DETECTIONSTATE_TEST5;

	return detectionTest5Update();
}

Bool FirewallHelperClass::detectionTest5Update() {
	/*
	** We have done all the tests we *have* to. There's other info that it would be nice to know though.
	**
	** Test for the netgear bug behavior.
	*/
#if (0)
// moved to before test 1.  Moved because this flag could be specified for another firewall
// for testing purposes and never get this far because it has behavior that doesn't require
// all the tests to be performed.
// BGC 10/1/02
	DEBUG_LOG(("FirewallHelperClass::detectionTest5Update - Testing for Netgear bug"));

	/*
	** See if the user specified a netgear firewall - that will save us the trouble.
	*/
	if (TheGlobalData->m_firewallSendDelay) {
		UnsignedInt addbehavior = FIREWALL_TYPE_NETGEAR_BUG;
		addbehavior |= (UnsignedInt)m_behavior;
		m_behavior = (FirewallBehaviorType) addbehavior;
		DEBUG_LOG(("FirewallHelperClass::detectionTest5Update - Netgear bug specified by command line or SendDelay flag"));
	} else {
		DEBUG_LOG(("FirewallHelperClass::detectionTest5Update - Netgear bug not specified"));
	}
#endif // #if (0)

	DEBUG_LOG_RAW(("FirewallHelperClass::detectionTest5Update - All done, behavior is: "));

	if ((m_behavior & FIREWALL_TYPE_SIMPLE) != 0) {
		DEBUG_LOG_RAW((" FIREWALL_TYPE_SIMPLE "));
	}
	if ((m_behavior & FIREWALL_TYPE_DUMB_MANGLING) != 0) {
		DEBUG_LOG_RAW((" FIREWALL_TYPE_DUMB_MANGLING "));
	}
	if ((m_behavior & FIREWALL_TYPE_SMART_MANGLING) != 0) {
		DEBUG_LOG_RAW((" FIREWALL_TYPE_SMART_MANGLING "));
	}
	if ((m_behavior & FIREWALL_TYPE_NETGEAR_BUG) != 0) {
		DEBUG_LOG_RAW((" FIREWALL_TYPE_NETGEAR_BUG "));
	}
	if ((m_behavior & FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION) != 0) {
		DEBUG_LOG_RAW((" FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION "));
	}
	if ((m_behavior & FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION) != 0) {
		DEBUG_LOG_RAW((" FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION "));
	}
	if ((m_behavior & FIREWALL_TYPE_DESTINATION_PORT_DELTA) != 0) {
		DEBUG_LOG_RAW((" FIREWALL_TYPE_DESTINATION_PORT_DELTA "));
	}

	DEBUG_LOG_RAW(("\n"));

	m_currentState = DETECTIONSTATE_DONE;
	return TRUE;
}


/***********************************************************************************************
 * FHC::Get_NAT_Port_Allocation_Scheme -- Find out how a NAT is allocating ports               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Number of ports we should analyze                                                 *
 *           List of original port numbers                                                     *
 *           List of mangled port numbers                                                      *
 *           relative_delta (out) Is the delta relative to the original port number?           *
 *           looks_good (out) Do all the values point to the same delta?                       *
 *                                                                                             *
 * OUTPUT:   Port allocation delta                                                             *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/01 4:45PM ST : Created                                                               *
 *=============================================================================================*/
Int FirewallHelperClass::getNATPortAllocationScheme(Int numPorts, UnsignedShort *originalPorts, UnsignedShort *mangledPorts, Bool &relativeDelta, Bool &looksGood)
{
	DEBUG_ASSERTCRASH(numPorts > 3, ("numPorts too small"));

	DEBUG_LOG(("Looking for port allocation pattern in originalPorts %d, %d, %d, %d", originalPorts[0], originalPorts[1], originalPorts[2], originalPorts[3]));

	/*
	** Sort the mangled ports into order - should be easier to detect patterns.
	** Stupid bubble sort will do. original_ports may be out of oder after the sort.
	*/
	for (Int x=0 ; x<numPorts ; x++) {
		for (Int y=0 ; y<numPorts-1 ; y++) {
			if (mangledPorts[y] > mangledPorts[y+1]) {
				Int temp = mangledPorts[y];
				mangledPorts[y] = mangledPorts[y+1];
				mangledPorts[y+1] = temp;
				temp = originalPorts[y];
				originalPorts[y] = originalPorts[y+1];
				originalPorts[y+1] = temp;
			}
		}
	}

	/*
	** Now start looking for patterns in the port numbers. Possible patterns include.
	**
	** Incremental. Port numbers are allocated incrementally.
	** Every 'n' ports. NAT adds 'n' port numbers when allocating ports.
	**
	** Also, schemes may be absolute or relative to the original port number.
	*/

	/*
	** 1. Check for absolute sequential allocation.
	*/
	if (mangledPorts[1] - mangledPorts[0] == 1) {
		if (mangledPorts[2] - mangledPorts[1] == 1) {
			if (mangledPorts[3] - mangledPorts[2] == 1) {
				DEBUG_LOG(("Incremental port allocation detected"));
				relativeDelta = FALSE;
				looksGood = TRUE;
				return(1);
			}
		}
	}

	/*
	** 2. Check for semi sequential.
	*/
	if (mangledPorts[1] - mangledPorts[0] == 2) {
		if (mangledPorts[2] - mangledPorts[1] == 2) {
			if (mangledPorts[3] - mangledPorts[2] == 2) {
				DEBUG_LOG(("Semi-incremental port allocation detected"));
				relativeDelta = FALSE;
				looksGood = TRUE;
				return(2);
			}
		}
	}

	Int diff1 = mangledPorts[1] - mangledPorts[0];
	Int diff2 = mangledPorts[2] - mangledPorts[1];
	Int diff3 = mangledPorts[3] - mangledPorts[2];


	/*
	** 3. Check for absolute scheme skipping 'n' ports.
	*/
	if (diff1 == diff2 && diff2 == diff3) {
		DEBUG_LOG(("Looks good for absolute allocation sequence delta of %d", diff1));
		relativeDelta = FALSE;
		looksGood = TRUE;
		return(diff1);
	}

	if (diff1 == diff2) {
		DEBUG_LOG(("Probable absolute allocation sequence delta of %d", diff1));
		relativeDelta = FALSE;
		looksGood = FALSE;
		return(diff1);
	}

	if (diff2 == diff3) {
		DEBUG_LOG(("Probable absolute allocation sequence delta of %d", diff2));
		relativeDelta = FALSE;
		looksGood = FALSE;
		return(diff2);
	}




	/*
	** Insert more tests here if we can think of any!!!!!
	*/



	/*
	** 4. Check for relative scheme skipping 'n' ports. NAT32 behaves this way, it skips 100 ports
	** each time.
	*/
	for (Int i=0 ; i<numPorts ; i++) {
		mangledPorts[i] -= originalPorts[i];
	}

	diff1 = mangledPorts[1] - mangledPorts[0];
	diff2 = mangledPorts[2] - mangledPorts[1];
	diff3 = mangledPorts[3] - mangledPorts[2];

	/*
	** Look for a linear pattern.
	*/
	if (diff1 == diff2 && diff2 == diff3) {
		/*
		** Return a -ve result to indicate that port mangling is relative.
		*/
		DEBUG_LOG(("Looks good for a relative port range delta of %d", diff1));
		relativeDelta = TRUE;
		looksGood = TRUE;
		return(diff1);
	}

	/*
	** Look for a broken pattern. Maybe the NAT skipped a whole range.
	*/
	if (diff1 == diff2 || diff1 == diff3) {
		DEBUG_LOG(("Detected probable broken relative port range delta of %d", diff1));
		relativeDelta = TRUE;
		looksGood = FALSE;
		return(diff1);
	}

	if (diff2 == diff3) {
		DEBUG_LOG(("Detected probable broken relative port range delta of %d", diff2));
		relativeDelta = TRUE;
		looksGood = FALSE;
		return(diff2);
	}


	/*
	** Aw hell, I don't know what it is.
	*/
	looksGood = FALSE;
	relativeDelta = FALSE;
	return(0);
}






/***********************************************************************************************
 * FHC::Get_Firewall_Hardness -- How hard is it to connect to this firewall                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Firewall behavior                                                                 *
 *                                                                                             *
 * OUTPUT:   Hardness                                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/01 11:43AM ST : Created                                                              *
 *=============================================================================================*/
Int FirewallHelperClass::getFirewallHardness(FirewallBehaviorType behavior)
{

	Int hardness = 0;


	UnsignedInt fw = (UnsignedInt) behavior;

	if (((UnsignedInt)FIREWALL_TYPE_SIMPLE & fw) != 0) {
		hardness++;
	}

	if (((UnsignedInt)FIREWALL_TYPE_DUMB_MANGLING & fw) != 0) {
		hardness += 2;
	}

	if (((UnsignedInt)FIREWALL_TYPE_SMART_MANGLING & fw) != 0) {
		hardness += 3;
	}

	if (((UnsignedInt)FIREWALL_TYPE_NETGEAR_BUG & fw) != 0) {
		hardness += 10;
	}

	if (((UnsignedInt)FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION & fw) != 0) {
		hardness += 1;
	}

	if (((UnsignedInt)FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION & fw) != 0) {
		hardness += 2;
	}

	return(hardness);
}





/***********************************************************************************************
 * FHC::Get_Firewall_Retries -- How many retries is it likely to take before we connect?       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Firewall behavior                                                                 *
 *                                                                                             *
 * OUTPUT:   Hardness                                                                          *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/01 11:43AM ST : Created                                                              *
 *=============================================================================================*/
Int FirewallHelperClass::getFirewallRetries(FirewallBehaviorType behavior)
{

	Int retries = 2;


	UnsignedInt fw = (UnsignedInt) behavior;

	if (((UnsignedInt)FIREWALL_TYPE_SIMPLE & fw) != 0) {
		retries++;
	}

	if (((UnsignedInt)FIREWALL_TYPE_DUMB_MANGLING & fw) != 0) {
		retries += 1;
	}

	if (((UnsignedInt)FIREWALL_TYPE_SMART_MANGLING & fw) != 0) {
		retries += 1;
	}

	if (((UnsignedInt)FIREWALL_TYPE_NETGEAR_BUG & fw) != 0) {
		//retries += 10;
	}

	if (((UnsignedInt)FIREWALL_TYPE_SIMPLE_PORT_ALLOCATION & fw) != 0) {
		//retries += 1;
	}

	if (((UnsignedInt)FIREWALL_TYPE_RELATIVE_PORT_ALLOCATION & fw) != 0) {
		retries += 5;
	}

	return(retries);
}



/*
 *  openSpareSocket - opens a socket for communication on a specified port.
 *  returns TRUE if successful, FALSE otherwise.
 */
Bool FirewallHelperClass::openSpareSocket(UnsignedShort port) {
	Int i = 0;
	for (; i < MAX_SPARE_SOCKETS; ++i) {
		if (m_spareSockets[i].port == 0) {
			break;
		}
	}

	// don't have room for any more spare sockets.  Fail.
	if (i == MAX_SPARE_SOCKETS) {
		DEBUG_ASSERTCRASH(i < MAX_SPARE_SOCKETS, ("Ran out of spare sockets."));
		return FALSE;
	}

	m_spareSockets[i].udp = NEW UDP();
	if (m_spareSockets[i].udp == nullptr) {
		DEBUG_LOG(("FirewallHelperClass::openSpareSocket - failed to create UDP object"));
		return FALSE;
	}

	if (m_spareSockets[i].udp->Bind((UnsignedInt)0, port) != 0) {
		DEBUG_CRASH(("FirewallHelperClass::openSpareSocket - Failed to init spare socket"));
		return FALSE;
	}

	m_spareSockets[i].port = port;
	DEBUG_LOG(("FirewallHelperClass::openSpareSocket - port %d is open for send", port));
	return TRUE;
}

/*
 *  closeSpareSocket - closes a socket at a specific port.
 */
void FirewallHelperClass::closeSpareSocket(UnsignedShort port) {
	for (Int i = 0; i < MAX_SPARE_SOCKETS; ++i) {
		if (m_spareSockets[i].port == port) {
			delete m_spareSockets[i].udp;
			m_spareSockets[i].udp = nullptr;
			m_spareSockets[i].port = 0;
			break;
		}
	}
}

/*
 *  closeAllSpareSockets - closes all spare sockets, duh.
 */
void FirewallHelperClass::closeAllSpareSockets() {
	for (Int i = 0; i < MAX_SPARE_SOCKETS; ++i) {
		delete (m_spareSockets[i].udp);
		m_spareSockets[i].udp = nullptr;
		m_spareSockets[i].port = 0;
	}
}
