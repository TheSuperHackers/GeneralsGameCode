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


#include <signal.h>
#ifdef _WINDOWS
#include <process.h> // *MUST* be included before ANY Wnet/Wlib headers if _REENTRANT is defined
#endif

#include "crc.h"

#include "configfile.h"
#include "threadfac.h"

#include "endian.h"

#include "xtime.h"
#include <filed.h>
#include <wstring.h>
#include <wdebug.h>
#include <udp.h>




/***************************************************************************
 * Add_CRC -- Adds a value to a CRC                                        *
 *                                                                         *
 * INPUT:                                                                  *
 *    crc      ptr to crc                                                  *
 *    val      value to add                                                *
 *                                                                         *
 * OUTPUT:                                                                 *
 *    none                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *    none                                                                 *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/09/1995 BRR : Created.                                             *
 *=========================================================================*/
void Add_CRC(unsigned long *crc, unsigned char val)
{
	int hibit;

	//cout << "\t\t" << hex << val;
//	val = htonl(val);
	//cout << " / " << hex << val <<endl;

	if ((*crc) & 0x80000000) {
		hibit = 1;
	} else {
		hibit = 0;
	}

	(*crc) <<= 1;
	(*crc) += val;
	(*crc) += hibit;

	//cout << hex << (*crc) <<endl;
}


void Build_Packet_CRC(unsigned char *buf, int len)
{
	if (len < 5)
	{
		DBGMSG("Ack!  Constructing a packet too small to hold a CRC!");
		return;
	}
	if (!buf)
	{
		DBGMSG("Ack!  Constructing a CRC for a void *");
		return;
	}

	*((unsigned long *)buf) = 0;

	unsigned long *crc_ptr = (unsigned long *)buf;
	unsigned char *packetptr = (unsigned char*) (buf+4);

	len -= 4; // look past CRC

	for (int i=0 ; i<len ; i++) {
		Add_CRC (crc_ptr, *packetptr++);
	}
/*
	int leftover = len & 3;
	if (leftover) {
		unsigned long val = 0;
		unsigned char *c = (unsigned char *)packetptr;
		for (int i=0; i<leftover; i++)
		{
			val += (c[i] << (i*8));
		}
		val = htonl(val);
		Add_CRC (crc_ptr, val);
	}
*/
	*crc_ptr = htonl(*crc_ptr);
}


/***********************************************************************************************
 * Passes_CRC_Check -- Checks the CRC for a packet                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    ptr to packet                                                                     *
 *                                                                                             *
 * OUTPUT:   true if packet passes CRC check                                                   *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/5/99  1:26PM ST : Created                                                              *
 *   1/9/2001 2:21PM MDC: Ripped from RA2 (WinsockInterfaceClass in wsproto.cpp/queue.cpp)     *
 *  1/31/2001 4:30PM MDC: Converted to network-byte-order so Sparc boxes can talk with Intels. *
 *   2/1/2001 4:07PM MDC: Converted back to Intel order to avoid messing with C&C packets      *
 *=============================================================================================*/
bool Passes_CRC_Check(unsigned char *buf, int len)
{
	if (len < 5)
	{
		DBGMSG("Recieved packet too small to contain a CRC");
		return false;
	}
	if (!buf)
	{
		DBGMSG("Ack!  Checking a CRC for a void *");
		return false;
	}

	unsigned long crc = 0;

	unsigned long *crc_ptr = &crc;
	unsigned char *packetptr = (unsigned char*) (buf+4);

	len -= 4; // remove the CRC from packet size - just look at payload

	for (int i=0 ; i<len ; i++) {
		Add_CRC (crc_ptr, *packetptr++);
	}
/*
	int leftover = len & 3;
	if (leftover) {
		unsigned long val = 0;
		unsigned char *c = (unsigned char *)packetptr;
		for (int i=0; i<leftover; i++)
		{
			val += (c[i] << (i*8));
		}
		val = htonl(val);
		Add_CRC (crc_ptr, val);
	}
*/
	crc = htonl(crc);

	if (crc == *((unsigned long *)buf)) {
		return (true);
	}

	DBGMSG("Invalid packet CRC");
	return (false);
}
