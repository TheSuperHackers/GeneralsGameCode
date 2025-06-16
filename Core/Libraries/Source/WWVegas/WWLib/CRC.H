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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/crc.h                                              $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             * 
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CRC_H
#define CRC_H

#include	<stdlib.h>

// TheSuperHackers @compile feliwir 17/04/2025 include _ltrotl macros
#include <Utility/intrin_compat.h>

/*
**	This is a CRC engine class. It will process submitted data and generate a CRC from it.
**	Well, actually, the value returned is not a true CRC. However, it shares the same strength
**	characteristic and is faster to generate than the traditional CRC. This object is treated like
**	a method class. If it is called as a function (using the function operator), it will return
**	the CRC value. There are other function operators to submit data for processing.
*/
class CRCEngine {
	public:

		// Constructor for CRC engine (it can have an override initial CRC value).
		CRCEngine(long initial=0) : CRC(initial), Index(0) {
			StagingBuffer.Composite = 0;
		};

		// Fetches CRC value.
		long operator() (void) const {return(Value());};

		// Submits one byte sized datum to the CRC accumulator.
		void operator() (char datum);

		// Submits an arbitrary buffer to the CRC accumulator.
		long operator() (void const * buffer, int length);

		// Implicit conversion operator so this object appears like a 'long integer'.
		operator long(void) const {return(Value());};

	protected:

		bool Buffer_Needs_Data(void) const {
			return(Index != 0);
		};

		long Value(void) const {
			if (Buffer_Needs_Data()) {
				return(_lrotl(CRC, 1) + StagingBuffer.Composite);
			}
			return(CRC);
		};

		/*
		**	Current accumulator of the CRC value. This value doesn't take into
		**	consideration any pending data in the staging buffer.
		*/
		long CRC;

		/*
		**	This is the sub index into the staging buffer used to keep track of
		**	partial data blocks as they are submitted to the CRC engine.
		*/
		int Index;

		/*
		**	This is the buffer that holds the incoming partial data. When the buffer
		**	is filled, the value is transformed into the CRC and the buffer is flushed
		**	in preparation for additional data.
		*/
		union {
			long Composite;
			char Buffer[sizeof(long)];
		} StagingBuffer;
};

// the CRC class defines a few static functions for dealing with CRCs a little differently than
// the CRCEngine. This class uses a algorithm that produces CRC values that have a high probability
// of being unique under most circumstances.
// Note: this code was provided by Byon Garrabrant
//
// 12/09/97 EHC - converted from c to c++ static class and added to crc.h and crc.cpp
//
#define CRC32(c,crc) (CRC::_Table[((unsigned long)(crc) ^ (c)) & 0xFFL] ^ (((crc) >> 8) & 0x00FFFFFFL))
class CRC {

	// CRC for poly 0x04C11DB7
	static unsigned long _Table[256];

public:

	// get the CRC of a block of memory
	static unsigned long	Memory( unsigned char *data, unsigned long length, unsigned long crc = 0 );

	// get the CRC of a null-terminated string
	static unsigned long	String( const char *string, unsigned long crc = 0 );
};

#endif

