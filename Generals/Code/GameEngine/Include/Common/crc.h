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

// CRC.h ///////////////////////////////////////////////////////////////
// A class encapsulating CRC calculation
// Author: Matthew D. Campbell, October 2001

#pragma once

#ifndef _CRC_H_
#define _CRC_H_

#include "Lib/BaseType.h"

#ifdef RTS_DEBUG

//#include "winsock2.h" // for htonl

class CRC
{
public:
	CRC() { crc = 0; }

	void computeCRC( const void *buf, Int len );		///< Compute the CRC for a buffer, added into current CRC
	void clear( void ) { crc = 0; }									///< Clears the CRC to 0
//	UnsignedInt get( void ) { return htonl(crc); }	///< Get the combined CRC
	UnsignedInt get( void );

#if (defined(_MSC_VER) && _MSC_VER < 1300) && RETAIL_COMPATIBLE_CRC
  void set( UnsignedInt v )
  {
    crc = v;
  }
#endif

private:
	void addCRC( UnsignedByte val );									///< CRC a 4-byte block

	UnsignedInt crc;
};

#else

// optimized inline only version
class CRC
{
public:
	CRC(void) { crc=0; }

  /// Compute the CRC for a buffer, added into current CRC
	__forceinline void computeCRC( const void *buf, Int len )
  {
    if (!buf||len<1)
      return;
    
#if !(defined(_MSC_VER) && _MSC_VER < 1300)
    // C++ version left in for reference purposes
	  for (UnsignedByte *uintPtr=(UnsignedByte *)buf;len>0;len--,uintPtr++)
    {
    	int hibit;
    	if (crc & 0x80000000) 
      {
		    hibit = 1;
	    } 
      else 
      {
		    hibit = 0;
	    }

	    crc <<= 1;
	    crc += *uintPtr;
	    crc += hibit;
    }
#else
    // ASM version, verified by comparing resulting data with C++ version data
    unsigned *crcPtr=&crc;
    _asm
    {
      mov esi,[buf]
      mov ecx,[len]
      dec ecx
      mov edi,[crcPtr]
      mov ebx,dword ptr [edi]
      xor eax,eax
    lp:
      mov al,byte ptr [esi]
      shl ebx,1
      inc esi
      adc ebx,eax
      dec ecx
      jns lp
      mov dword ptr [edi],ebx
    };
#endif
  }

  /// Clears the CRC to 0
	void clear( void ) 
  { 
    crc = 0; 
  }									

  ///< Get the combined CRC
	UnsignedInt get( void ) const
  {
    return crc;
  }

#if (defined(_MSC_VER) && _MSC_VER < 1300) && RETAIL_COMPATIBLE_CRC
  void set( UnsignedInt v )
  {
    crc = v;
  }
#endif

private:
	UnsignedInt crc;
};

#endif

#endif // _CRC_H_
