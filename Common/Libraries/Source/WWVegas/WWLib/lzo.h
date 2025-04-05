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
 *                     $Archive:: /Commando/Code/wwlib/lzo.h                                  $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/19/99 3:35p                                               $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef _LZO_H
#define _LZO_H

#include	"lzoconf.h"
#include "lzo1x.h"

// Macros.

// Maximum size of any LZO compressed chunk expessed in terms of maximum size
// of UNcompressed chunk. NOTE: LZO needs an additional 16 bytes per uncompressed K.
// Note: SKB took this from lzo.h in lol3 code.
//
// (gth) The buffer that you compress to should be the size define by 
// LZO_BUFFER_SIZE(uncompressed_size).  Also, the work buffer should be of
// size: LZO1X_MEM_COMPRESS which is defined in lzo1x.h.

#define LZO_BUFFER_SIZE(s) ((s) + ((((s) / 0x400) + 1) * 16))	


int lzo1x_1_compress ( 	const lzo_byte *in,
								lzo_uint  in_len,
                         lzo_byte *out,
								lzo_uint *out_len,
                         lzo_voidp wrkmem);


int lzo1x_decompress	(  const lzo_byte *in,
								lzo_uint  in_len,
								lzo_byte *out,
								lzo_uint *out_len,
                         lzo_voidp);



//
// LZOCompressor
// Simply wraps the 'C' style lzo compression and decompression functions and 
// hides the work buffer.  So you dont have to worry about the work buffer but
// you do have to manage the compression buffer being large enough to hold the
// worst case compression: LZO_BUFFER_SIZE(uncompressed_size).
//
class LZOCompressor
{
public:

	static int Compress
	(
		const lzo_byte *	in,
		lzo_uint				in_len,
		lzo_byte *			out,
		lzo_uint *			out_len
	);

	static int Decompress
	(
		const lzo_byte *	in,
		lzo_uint				in_len,
		lzo_byte *			out,
		lzo_uint *			out_len
	);

private:

	static lzo_byte WorkBuffer[LZO1X_MEM_COMPRESS + 1];
	static lzo_byte * EOWorkBuffer;
};



#endif
