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
 *                 Project Name : WWLib                                                        *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/lzo.cpp                                $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/24/01 5:07p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LZOCompressor::Compress -- compress a buffer using LZO                                    *
 *   LZOCompressor::Decompress -- decompress a buffer using LZO                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "lzo.h"
#include "mutex.h"
#include "wwdebug.h"
#include <stdlib.h>

/*
** Work Buffer for the LZOCompressor...
*/
lzo_byte		LZOCompressor::WorkBuffer[LZO1X_MEM_COMPRESS + 1];
lzo_byte *	LZOCompressor::EOWorkBuffer = &(LZOCompressor::WorkBuffer[LZO1X_MEM_COMPRESS + 1]);

static CriticalSectionClass mutex;

#define	BUFFER_OVERRUN_TEST_VALUE	((char)0x7d)


/***********************************************************************************************
 * LZOCompressor::Compress -- compress a buffer using LZO                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
int LZOCompressor::Compress
(
	const lzo_byte * in,
	lzo_uint in_len,
	lzo_byte * out,
	lzo_uint * out_len
)
{
	CriticalSectionClass::LockClass m(mutex);

#ifdef WWDEBUG
	// Debugging code to verify that the work buffer is not overrun...
	*EOWorkBuffer = BUFFER_OVERRUN_TEST_VALUE;
#endif

	int result = lzo1x_1_compress(in,in_len,out,out_len,WorkBuffer);

#ifdef WWDEBUG
	WWASSERT(*EOWorkBuffer == BUFFER_OVERRUN_TEST_VALUE);
#endif

	return result;
}


/***********************************************************************************************
 * LZOCompressor::Decompress -- decompress a buffer using LZO                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/19/99    GTH : Created.                                                                 *
 *=============================================================================================*/
int LZOCompressor::Decompress
(
	const lzo_byte * in,
	lzo_uint in_len,
	lzo_byte * out,
	lzo_uint * out_len
)
{
	CriticalSectionClass::LockClass m(mutex);

	return lzo1x_decompress(in,in_len,out,out_len,nullptr);
}
