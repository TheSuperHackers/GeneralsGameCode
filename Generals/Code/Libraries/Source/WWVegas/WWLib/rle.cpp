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

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Library/RLE.cpp                              $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 9/24/98 10:05a                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   RLEEngine::Compress -- Compresses a sequence of bytes.                                    * 
 *   RLEEngine::Decompress -- Decompress a sequence of RLE compressed bytes.                   * 
 *   RLEEngine::Line_Compress -- Compress a line of data.                                      * 
 *   RLEEngine::Line_Decompress -- Decompresses a line-compressed RLE data sequence.           * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"RLE.H"
#include	<assert.h>
#include	<stdlib.h>


/*********************************************************************************************** 
 * RLEEngine::Compress -- Compresses a sequence of bytes.                                      * 
 *                                                                                             * 
 *    This routine will compress the sequence of bytes specified by using RLE compression.     * 
 *                                                                                             * 
 * INPUT:   source  -- Pointer to the data to be compressed.                                   * 
 *                                                                                             * 
 *          dest    -- Pointer to the buffer that will hold the compressed data.               * 
 *                                                                                             * 
 *          length  -- The length of the data to compress.                                     * 
 *                                                                                             * 
 * OUTPUT:  Returns with the number of bytes long for the compressed data.                     * 
 *                                                                                             * 
 * WARNINGS:    The compressed data may, in rare instances, be larger than the data it was     * 
 *              compressing. The worst case is 33% larger. Keep that in mind when supplying    * 
 *              the destination buffer to this routine.                                        * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/17/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int RLEEngine::Compress(void const * source, void * dest, int length) const
{
	assert(source != NULL);
	assert(length > 0);

	int outlen = 0;
	unsigned char const * sptr = (unsigned char const *) source;
	unsigned char * dptr = (unsigned char *)dest;
	while (length > 0) {

		/*
		**	Examine each source byte. If the byte is zero (transparent), then
		**	it is recorded as a run. Otherwise it is output without translation.
		*/
		if (*sptr == '\0') {

			/*
			**	Count the number of transparent pixels in this run.
			*/
			int runcount = 0;
			while (sptr[runcount] == '\0' && runcount <= length) {
				runcount++;
			}

			/*
			**	Limit the run to 255 characters maximum.
			*/
			runcount = MIN(runcount, 255);
			if (dptr != NULL) {
				*dptr++ = '\0';
				*dptr++ = (unsigned char)runcount;
			}
			outlen += 2;
			sptr += runcount;
			length -= runcount;

		} else {

			/*
			**	Store the raw byte without any translation.
			*/
			if (dptr != NULL) {
				*dptr++ = *sptr++;
			} else {
				sptr++;
			}
			outlen++;
			length--;
		}
	}

	/*
	**	Return with the number of compressed output bytes.
	*/
	return(outlen);
}


/*********************************************************************************************** 
 * RLEEngine::Line_Compress -- Compress a line of data.                                        * 
 *                                                                                             * 
 *    This routine will compress a sequence of bytes and store the length of the compressed    * 
 *    data at the beginning of the output buffer. By encoding the compressed size in this      * 
 *    fashion, it is possible to build a sequence of compressed bytes (such as with a sprite)  * 
 *    so that each sequence can be quickly traversed.                                          * 
 *                                                                                             * 
 * INPUT:   source   -- Pointer to the source data to be compressed.                           * 
 *                                                                                             * 
 *          dest     -- Pointer to the destination buffer that will hold the length value and  * 
 *                      the compressed data.                                                   * 
 *                                                                                             * 
 *          length   -- The number of source bytes to compress.                                * 
 *                                                                                             * 
 * OUTPUT:  Returns with the number of bytes stored into the output buffer.                    * 
 *                                                                                             * 
 * WARNINGS:   The output data could be larger than the source data by as much as 33% + 2      * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/17/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int RLEEngine::Line_Compress(void const * source, void * dest, int length) const
{
	assert(source != NULL);
	assert(length > 0);

	/*
	**	If an output buffer was specified, then the data is actually compressed
	**	into the buffer.
	*/
	if (dest != NULL) {
		unsigned short * sizeptr = (unsigned short *)dest;
		int complen = Compress(source, sizeptr+1, length) + sizeof(short);
		*sizeptr = (unsigned short)complen;
		return(complen);
	}

	/*
	**	Since no output buffer was specifed, this call merely determins how
	**	many bytes would be consumed in the output buffer.
	*/
	return(Compress(source, NULL, length) + sizeof(short));
}


/*********************************************************************************************** 
 * RLEEngine::Decompress -- Decompress a sequence of RLE compressed bytes.                     * 
 *                                                                                             * 
 *    This will decompress a sequence of RLE compressed bytes.                                 * 
 *                                                                                             * 
 * INPUT:   source   -- Pointer to the RLE compressed data.                                    * 
 *                                                                                             * 
 *          dest     -- Pointer to the buffer that will hold the uncompressed data.            * 
 *                                                                                             * 
 *          length   -- The length of the source RLE data to process.                          * 
 *                                                                                             * 
 * OUTPUT:  Returns with the actual number of bytes stored into the output buffer.             * 
 *                                                                                             * 
 * WARNINGS:   The output buffer must be large enough to hold the decompressed data. This      * 
 *             could be as much as 128 times larger than the source RLE data.                  * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/17/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int RLEEngine::Decompress(void const * source, void * dest, int length) const
{
	assert(source != NULL);
	assert(dest != NULL);
	assert(length > 0);

	unsigned char * dptr = (unsigned char *)dest;
	unsigned char const * sptr = (unsigned char const *)source;

	/*
	**	Process the RLE data normally.
	*/
	while (length > 0) {

		/*
		**	Detect if a zero-run code is present. If so, then dump the desired
		**	number of bytes. Otherwise output the pixel in untranslated form.
		*/
		unsigned char value = *sptr++;
		length--;
		if (value == '\0') {
			int outlen = *sptr++;
			length--;
			while (outlen > 0) {
				*dptr++ = '\0';
				outlen--;
			}

		} else {
			*dptr++ = value;
		}
	}
	
	/*
	**	Return with the number of bytes stored into the output buffer.
	*/
	return(dptr - (unsigned char const *)dest);
}


/*********************************************************************************************** 
 * RLEEngine::Line_Decompress -- Decompresses a line-compressed RLE data sequence.             * 
 *                                                                                             * 
 *    This routine is the counterpart to Line_Compress. It will take a compressed line and     * 
 *    fully decompress it into the destination buffer.                                         * 
 *                                                                                             * 
 * INPUT:   source   -- The pointer to the line compressed RLE data.                           * 
 *                                                                                             * 
 *          dest     -- Pointer to the destination buffer that will hold the decompressed      * 
 *                      data.                                                                  * 
 *                                                                                             * 
 * OUTPUT:  Returns with the number of bytes stored into the destination buffer.               * 
 *                                                                                             * 
 * WARNINGS:   none                                                                            * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/17/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
int RLEEngine::Line_Decompress(void const * source, void * dest) const
{
	assert(source != NULL);
	assert(dest != NULL);

	unsigned short const * sptr = (unsigned short const *)source;

	int datalen = *sptr++;

	/*
	**	Process the RLE data normally.
	*/
	return(Decompress(sptr, dest, datalen - sizeof(short)));
}

