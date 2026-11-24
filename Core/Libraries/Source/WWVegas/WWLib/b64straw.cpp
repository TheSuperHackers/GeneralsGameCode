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
 *                     $Archive:: /Commando/Library/B64STRAW.cpp                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"b64straw.h"
#include	"base64.h"


// The nature of Base 64 encoding means that the data will increase in size by 30% when encoding and
// decrease in like manner when decoding.
int Base64Straw::Get(void * source, int slen)
{
	int total = 0;

	char * from;
	int fromsize;
	char * to;
	int tosize;

	if (Control == ENCODE) {
		from = PBuffer;
		fromsize = sizeof(PBuffer);
		to = CBuffer;
		tosize = sizeof(CBuffer);
	} else {
		from = CBuffer;
		fromsize = sizeof(CBuffer);
		to = PBuffer;
		tosize = sizeof(PBuffer);
	}

	/*
	**	Process the byte request in code blocks until there are either
	**	no more source bytes available or the request has been fulfilled.
	*/
	while (slen > 0) {

		/*
		**	Transfer any processed bytes available to the request buffer.
		*/
		if (Counter > 0) {
			int len = (slen < Counter) ? slen : Counter;
			memmove(source, &to[tosize-Counter], len);
			Counter -= len;
			slen -= len;
			source = ((char *)source) + len;
			total += len;
		}
		if (slen == 0) break;

		/*
		**	More bytes are needed, so fetch and process another base 64 block.
		*/
		int incount = Straw::Get(from, fromsize);
		if (Control == ENCODE) {
			Counter = Base64_Encode(from, incount, to, tosize);
		} else {
			Counter = Base64_Decode(from, incount, to, tosize);
		}
		if (Counter == 0) break;
	}

	return(total);
}
