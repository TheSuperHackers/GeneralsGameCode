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


#include <Utility/iostream_adapter.h>

#include <signal.h>
#ifdef _WIN32
#include <process.h> // *MUST* be included before ANY Wnet/Wlib headers if _REENTRANT is defined
#endif

#include "mangler.h"
#include "crc.h"

#include <configfile.h>
#include "threadfac.h"

#include "endian.h"

#include "xtime.h"
#include <filed.h>
#include <wstring.h>
#include <wdebug.h>
#include <udp.h>

// ST - 2/1/01 12:46PM
bool BigEndian = false;

unsigned long ResolveIP(const char *Server)
{
  char serverName[100];
  struct hostent *serverStruct;
  struct in_addr *serverNode;

  if (Server == nullptr)
  {
	  ERRMSG("Can't resolve null");
	  return 0;
  }

  if (isdigit(Server[0]))
    return ( ntohl(inet_addr(Server)) );

  strcpy(serverName, Server);

  serverStruct = gethostbyname(Server);
  if (serverStruct == nullptr)
  {
	  ERRMSG("Can't resolve " << Server);
	  return 0;
  }
  serverNode = (struct in_addr *) serverStruct->h_addr;
  return ( ntohl(serverNode->s_addr) );
}

void DisplayHelp(const char *prog)
{
	cout << "Usage: " << prog << " <config file>" << endl;
	exit(0);
}

int main(int argc, char **argv)
{
	ConfigFile config;
	FILE*	conf;

	if( argc <= 1 )
	{
		// No args - use a default config file
		if ((conf = fopen("manglertest.cfg", "r")) == nullptr) {
			cout << "Cannot open mangler.cfg for reading." << endl;
			DisplayHelp(argv[0]);
		}
		config.readFile(conf);
		fclose(conf);
	}
	else if( argc == 2 && (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "?") == 0 ||
		 strcmp(argv[1], "-h") == 0) )
		DisplayHelp(argv[0]);
	else if( argc == 2 )
	{
		// Use a user-supplied config file
		if ((conf = fopen(argv[1], "r")) == nullptr) {
			cout << "Cannot open " << argv[1] << " for reading." << endl;
			DisplayHelp(argv[0]);
		}
		config.readFile(conf);
		fclose(conf);
	}

	// ----- LOGGING -----
	// Setup debugging & logging output
  	Wstring output_file("manglertest.log");
	config.getString("LOGFILE", output_file);
  	Wstring backup_file;
  	backup_file = output_file;
	backup_file += ".bak";
  	rename(output_file.get(),backup_file.get());  // save the old file
  	FileD   output_device(output_file.get());
  	MsgManager::setAllStreams(&output_device);
	DBGMSG("DBG working...");
	INFMSG("INF working...");
	WRNMSG("WRN working...");


	//
	// See if our processor is big or little endian. Network order is big endian.
	// ST - 2/1/01 12:11PM
	//
	if (htonl(0x12345678) == 0x12345678) {
		BigEndian = true;
	}


	// ----- Initialize Winsock -----
#ifdef _WIN32
	WORD verReq = MAKEWORD(2, 2);
	WSADATA wsadata;

	int err = WSAStartup(verReq, &wsadata);
	if (err != 0) {
		ERRMSG("Winsock Init failed.");
		return 1;
	}

	if ((LOBYTE(wsadata.wVersion) != 2) || (HIBYTE(wsadata.wVersion) !=2)) {
		ERRMSG("Winsock DLL is not 2.2");
		WSACleanup();
		ERRMSG("Winsock Init failed.");
		return 1;
	}
	INFMSG("Winsock Init done.");
#endif


	// Set up a UDP listener
	uint8  *buff=new uint8[1024];
	int     retval;
	UDP     udp;
	int     port = 4321;
	config.getInt("MANGLERPORT", port);

	int localport = 4444;
	config.getInt("CLIENTPORT", localport);
	retval = udp.Bind((uint32)0,(uint16)localport);
	DBGMSG("Bind returned " << retval);

	//-----------------------------------------------------------------------------------------
	const int packet_size = sizeof(ManglerData);
	INFMSG("sizeof(packet) == " << packet_size);

	unsigned char buf[packet_size];
	memset(buf, 0x44, packet_size);  // init to something known for memory dumps :)
	struct sockaddr_in addr;

	int doBlitz = 0;
	config.getInt("BLITZ", doBlitz);
	if (doBlitz)
	{
		INFMSG("Requsting port blitz");
	}

	unsigned char *theAddr;
	fd_set fdset;
	unsigned long server_addr;
	Wstring manglername = "localhost";
	config.getString("MANGLERIP", manglername);
	server_addr = ResolveIP(manglername.get());
	if (!server_addr)
	{
		ERRMSG("Cannot resolve mangler server IP");
		return 1;
	}

	ManglerData *packet = (ManglerData *)buf;
	packet->NetCommandType = 12;
	packet->packetID = 9999;
	packet->BlitzMe = doBlitz;
	packet->magic = htons((unsigned short)0xf00d);
	Build_Packet_CRC(buf, packet_size);
	DBGMSG("Writing to " << manglername.get() << ":" << port);
	udp.Write(buf,packet_size,server_addr, 4321);

	retval = udp.Wait(5, 0, fdset);
	if (retval)
	{
		DBGMSG("Wait returned " << retval);
		retval = udp.Read(buf, packet_size, &addr);        // Wait until there is something on the socket
		if (retval > 0)
		{
			theAddr = (unsigned char *)&(addr.sin_addr.s_addr);
			if (retval != packet_size)
			{
				WRNMSG("Recieved mis-sized packet (" << retval << " bytes) from " << theAddr[0] << "." << theAddr[1] << "." << theAddr[2] << "." << theAddr[3] << ":" << addr.sin_port);
			}
			else
			{
				int packetCommand;
				packetCommand = packet->NetCommandType;
				if (!Passes_CRC_Check(buf, packet_size))
				{
					WRNMSG("CRC error!");
				}
				else if (packetCommand != 13)
				{
					WRNMSG("Returned packet had command type " << packetCommand);
				}
				else
				{
					int addr[4];
					unsigned short retPort;
					retPort = htons(packet->MyMangledPortNumber);
					addr[0] = packet->MyMangledAddress[0];
					addr[1] = packet->MyMangledAddress[1];
					addr[2] = packet->MyMangledAddress[2];
					addr[3] = packet->MyMangledAddress[3];
					DBGMSG("Saw " << addr[0] << "." << addr[1] << "." << addr[2] << "." << addr[3] << ":" << retPort);
				}
			}
		}
	}
	else
	{
		DBGMSG("Wait timed out");
	}


	return 0;
}


