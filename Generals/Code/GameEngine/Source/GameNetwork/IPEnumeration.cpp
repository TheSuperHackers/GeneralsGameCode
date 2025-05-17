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

#include "GameNetwork/IPEnumeration.h"

IPEnumeration::IPEnumeration( void )
{
	m_IPlist = NULL;
	m_isWinsockInitialized = false;
}

IPEnumeration::~IPEnumeration( void )
{
	if (m_isWinsockInitialized)
	{
		WSACleanup();
		m_isWinsockInitialized = false;
	}

	EnumeratedIP *ip = m_IPlist;
	while (ip)
	{
		ip = ip->getNext();
		deleteInstance(m_IPlist);
		m_IPlist = ip;
	}
}

EnumeratedIP * IPEnumeration::getAddresses( void )
{
	if (m_IPlist)
		return m_IPlist;

	if (!m_isWinsockInitialized)
	{
		WORD verReq = MAKEWORD(2, 2);
		WSADATA wsadata;

		int err = WSAStartup(verReq, &wsadata);
		if (err != 0) {
			return NULL;
		}

		if ((LOBYTE(wsadata.wVersion) != 2) || (HIBYTE(wsadata.wVersion) !=2)) {
			WSACleanup();
			return NULL;
		}
		m_isWinsockInitialized = true;
	}

	// get the local machine's host name
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)))
	{
		DEBUG_LOG(("Failed call to gethostname; WSAGetLastError returned %d\n", WSAGetLastError()));
		return NULL;
	}
	DEBUG_LOG(("Hostname is '%s'\n", hostname));
	
	// get host information from the host name
	HOSTENT* hostEnt = gethostbyname(hostname);
	if (hostEnt == NULL)
	{
		DEBUG_LOG(("Failed call to gethostnyname; WSAGetLastError returned %d\n", WSAGetLastError()));
		return NULL;
	}
	
	// sanity-check the length of the IP adress
	if (hostEnt->h_length != 4)
	{
		DEBUG_LOG(("gethostbyname returns oddly-sized IP addresses!\n"));
		return NULL;
	}
	
	// construct a list of addresses
	int numAddresses = 0;
	char *entry;
	while ( (entry = hostEnt->h_addr_list[numAddresses++]) != 0 )
	{
		EnumeratedIP *newIP = newInstance(EnumeratedIP);

		AsciiString str;
		str.format("%d.%d.%d.%d", (unsigned char)entry[0], (unsigned char)entry[1], (unsigned char)entry[2], (unsigned char)entry[3]);

		UnsignedInt testIP = *((UnsignedInt *)entry);
		UnsignedInt ip = ntohl(testIP);

		/*
		ip = *entry++;
		ip <<= 8;
		ip += *entry++;
		ip <<= 8;
		ip += *entry++;
		ip <<= 8;
		ip += *entry++;
		*/

		newIP->setIPstring(str);
		newIP->setIP(ip);

		DEBUG_LOG(("IP: 0x%8.8X / 0x%8.8X (%s)\n", testIP, ip, str.str()));

		// Add the IP to the list in ascending order
		if (!m_IPlist)
		{
			m_IPlist = newIP;
			newIP->setNext(NULL);
		}
		else
		{
			if (newIP->getIP() < m_IPlist->getIP())
			{
				newIP->setNext(m_IPlist);
				m_IPlist = newIP;
			}
			else
			{
				EnumeratedIP *p = m_IPlist;
				while (p->getNext() && p->getNext()->getIP() < newIP->getIP())
				{
					p = p->getNext();
				}
				newIP->setNext(p->getNext());
				p->setNext(newIP);
			}
		}
	}

	return m_IPlist;
}

AsciiString IPEnumeration::getMachineName( void )
{
	if (!m_isWinsockInitialized)
	{
		WORD verReq = MAKEWORD(2, 2);
		WSADATA wsadata;

		int err = WSAStartup(verReq, &wsadata);
		if (err != 0) {
			return NULL;
		}

		if ((LOBYTE(wsadata.wVersion) != 2) || (HIBYTE(wsadata.wVersion) !=2)) {
			WSACleanup();
			return NULL;
		}
		m_isWinsockInitialized = true;
	}

	// get the local machine's host name
	char hostname[256];
	if (gethostname(hostname, sizeof(hostname)))
	{
		DEBUG_LOG(("Failed call to gethostname; WSAGetLastError returned %d\n", WSAGetLastError()));
		return NULL;
	}

	return AsciiString(hostname);
}


