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


#include <Utility/iostream_adapter.h>

#include <signal.h>
#ifdef _WINDOWS
#include <process.h> // *MUST* be included before ANY Wnet/Wlib headers if _REENTRANT is defined
#endif

#include "mangler.h"
#include "crc.h"
#include "endian.h"

#include <configfile.h>
#include "threadfac.h"

#include "xtime.h"
#include <filed.h>
#include <wstring.h>
#include <wdebug.h>
#include <udp.h>

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
		if ((conf = fopen("mangler.cfg", "r")) == NULL) {
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
		if ((conf = fopen(argv[1], "r")) == NULL) {
			cout << "Cannot open " << argv[1] << " for reading." << endl;
			DisplayHelp(argv[0]);
		}
		config.readFile(conf);
		fclose(conf);
	}
	
	// ----- LOGGING -----
	// Setup debugging & logging output
	Wstring output_file("mangler.log");
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
	
	if (htonl(0x12345678) == 0x12345678)
	{
		INFMSG("Host is network-byte-order");
	}
	else
	{
		INFMSG("Host is Intel-byte-order");
	}
	
	
	// ----- Initialize Winsock -----
#ifdef _WINDOWS
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
	UDP     udp2;
	UDP     udp3;
	UDP     udp4;
	int     port = 4321;
	config.getInt("PORT", port);
	uint8 blitz = 0;

	uint32 localIP = 0;
	Wstring hostIPStr = "";
	config.getString("IP", hostIPStr);
	if (hostIPStr.length())
	{
		INFMSG("Binding to "<<hostIPStr.get()<<":"<<port<<"-"<<(port+3));
		localIP = ntohl(inet_addr(hostIPStr.get()));
	}
	else
	{
		INFMSG("Binding to localhost:"<<port<<"-"<<(port+3));
	}

	retval  =  udp.Bind(localIP,(uint16)port);
	retval |= udp2.Bind(localIP,(uint16)port+1);
	retval |= udp3.Bind(localIP,(uint16)port+2);
	retval |= udp4.Bind(localIP,(uint16)port+3);
	if (retval != 0)
	{
		ERRMSG("Couldn't bind - error " << retval);
		exit(1);
	}
	
	unsigned char buf[1024];
	struct sockaddr_in addr;
	int packet_size = sizeof(ManglerData);
	INFMSG("sizeof(packet) == " << packet_size);
	unsigned char *theAddr;
	fd_set fdset;
	while (1)
	{
		retval = udp.Wait(15, 0, fdset);
		if (!retval)
			continue;
		
		//DBGMSG("Wait returned " << retval);
		retval = udp.Read(buf, packet_size, &addr);        // Wait until there is something on the socket
		if (retval > 0)
		{
			ManglerData *packet = (ManglerData *)buf;
			theAddr = (unsigned char *)&(addr.sin_addr.s_addr);
			if (retval != packet_size)
			{
				WRNMSG("Recieved mis-sized packet (" << retval << " bytes) from " << theAddr[0] << "." << theAddr[1] << "." << theAddr[2] << "." << theAddr[3] << ":" << addr.sin_port);
			}
			else
			{
				if (!Passes_CRC_Check(buf, packet_size))
				{
					WRNMSG("Recieved a bad packet - good length!");
					continue;
				}
				packet->NetCommandType = 44;
				packet->MyMangledPortNumber = addr.sin_port; // not changing to host order, cause its in network byte order now, and the game will expect it to stay that way.
				packet->MyMangledAddress[0] = theAddr[0];
				packet->MyMangledAddress[1] = theAddr[1];
				packet->MyMangledAddress[2] = theAddr[2];
				packet->MyMangledAddress[3] = theAddr[3];
				blitz = packet->BlitzMe;
				INFMSG("Packet ID = " << packet->packetID);
				Build_Packet_CRC(buf, packet_size);
				udp.Write(buf,packet_size,ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port));
				INFMSG("Saw " << (int)theAddr[0] << "." << (int)theAddr[1] << "." << (int)theAddr[2] << "." << (int)theAddr[3] << ":" << ntohs(addr.sin_port) << ((blitz)?" Blitzed":"") );
				
				if (blitz)
				{
					udp2.Write(buf,packet_size,ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port)+1);
					udp3.Write(buf,packet_size,ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port)+2);
					udp4.Write(buf,packet_size,ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port)+3);
				}
			}
		}
	}
	
	
	return 0;
}


