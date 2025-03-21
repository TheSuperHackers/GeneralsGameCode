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
 *                     $Archive:: /Commando/Library/LCWPIPE.H                                 $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef LCWPIPE_H
#define LCWPIPE_H

#include	"PIPE.H"


/*
**	Performs LCW compression/decompression on the data stream that is piped through this
**	class. The data is compressed in blocks so of small enough size to be compressed
**	quickly and large enough size to get decent compression rates.
*/
class LCWPipe : public Pipe
{
	public:
		typedef enum CompControl {
			COMPRESS,
			DECOMPRESS
		} CompControl;

		LCWPipe(CompControl, int blocksize=1024*8);
		virtual ~LCWPipe(void);

		virtual int Flush(void);
		virtual int Put(void const * source, int slen);

	private:
		/*
		**	This tells the pipe if it should be decompressing or compressing the data stream.
		*/
		CompControl Control;

		/*
		**	The number of bytes accumulated into the staging buffer.
		*/
		int Counter;

		/*
		**	Pointer to the working buffer that compression/decompression will use.
		*/
		char * Buffer;
		char * Buffer2;

		/*
		**	The working block size. Data will be compressed in chunks of this size.
		*/
		int BlockSize;

		/*
		**	LCW compression requires a safety margin when decompressing over itself. This
		**	margin is only for the worst case situation (very rare).
		*/
		int SafetyMargin;

		/*
		**	Each block has a header of this format.
		*/
		struct {
			unsigned short CompCount;		// Size of data block (compressed).
			unsigned short UncompCount;	// Bytes of uncompressed data it represents.
		} BlockHeader;

		LCWPipe(LCWPipe & rvalue);
		LCWPipe & operator = (LCWPipe const & pipe);
};


#endif
