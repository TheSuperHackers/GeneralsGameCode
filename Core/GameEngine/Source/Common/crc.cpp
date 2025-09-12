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

#include "Common/crc.h"
#include "Common/Debug.h"

#ifdef RTS_DEBUG

void CRC::addCRC( UnsignedByte val )
{
	int hibit;

	//cout << "\t\t" << hex << val;
//	val = htonl(val);
	//cout << " / " << hex << val <<endl;


	if (crc & 0x80000000) {
		hibit = 1;
	} else {
		hibit = 0;
	}

	crc <<= 1;
	crc += val;
	crc += hibit;

	//cout << hex << (*crc) <<endl;
}


void CRC::computeCRC( const void *buf, Int len )
{
	if (!buf || len < 1)
	{
		return;
	}

	//crc = 0;

	UnsignedByte *uintPtr = (UnsignedByte *)buf;

	for (int i=0 ; i<len ; i++) {
		addCRC (*(uintPtr++));
	}
	//crc = htonl(crc);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnsignedInt CRC::get( void )
{

	UnsignedInt tcrc = crc;
	return tcrc;

}

#endif
