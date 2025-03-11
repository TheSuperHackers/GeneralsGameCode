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
 *                     $Archive:: /G/wwlib/lcw.cpp                                            $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             * 
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   LCW_Comp -- Performes LCW compression on a block of data.                                 * 
 *   LCW_Uncomp -- Decompress an LCW encoded data block.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"lcw.h"

/***************************************************************************
 * LCW_Uncomp -- Decompress an LCW encoded data block.                     *
 *                                                                         *
 * Uncompress data to the following codes in the format b = byte, w = word *
 * n = byte code pulled from compressed data.                              *
 *                                                                         *
 *   Command code, n        |Description                                   *
 * ------------------------------------------------------------------------*
 * n=0xxxyyyy,yyyyyyyy      |short copy back y bytes and run x+3 from dest *
 * n=10xxxxxx,n1,n2,...,nx+1|med length copy the next x+1 bytes from source*
 * n=11xxxxxx,w1            |med copy from dest x+3 bytes from offset w1   *
 * n=11111111,w1,w2         |long copy from dest w1 bytes from offset w2   *
 * n=11111110,w1,b1         |long run of byte b1 for w1 bytes              *
 * n=10000000               |end of data reached                           *
 *                                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      void * source ptr                                                  *
 *      void * destination ptr                                             *
 *      unsigned long length of uncompressed data                          *
 *                                                                         *
 *                                                                         *
 * OUTPUT:                                                                 *
 *     unsigned long # of destination bytes written                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *     3rd argument is dummy. It exists to provide cross-platform          *
 *      compatibility. Note therefore that this implementation does not    *
 *      check for corrupt source data by testing the uncompressed length.  *
 *                                                                         *
 * HISTORY:                                                                *
 *    03/20/1995 IML : Created.                                            *
 *=========================================================================*/
int LCW_Uncomp(void const * source, void * dest, unsigned long )
{
	unsigned char * source_ptr, * dest_ptr, * copy_ptr;
	unsigned char op_code, data;
	unsigned count;
	unsigned * word_dest_ptr;
	unsigned word_data;

	/* Copy the source and destination ptrs. */
	source_ptr = (unsigned char*) source;
	dest_ptr   = (unsigned char*) dest;

	for (;;) {

		/* Read in the operation code. */
		op_code = *source_ptr++;

		if (!(op_code & 0x80)) {

			/* Do a short copy from destination. */
			count = (op_code >> 4) + 3;
			copy_ptr = dest_ptr - ((unsigned) *source_ptr++ + (((unsigned) op_code & 0x0f) << 8));

			while (count--) *dest_ptr++ = *copy_ptr++;

		} else {

			if (!(op_code & 0x40)) {

				if (op_code == 0x80) {

					/* Return # of destination bytes written. */
					return ((unsigned long) (dest_ptr - (unsigned char*) dest));

				} else {

					/* Do a medium copy from source. */
					count = op_code & 0x3f;

					while (count--) *dest_ptr++ = *source_ptr++;
				}

			} else {

				if (op_code == 0xfe) {

					/* Do a long run. */
					count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
					word_data = data = *(source_ptr + 2);
					word_data  = (word_data << 24) + (word_data << 16) + (word_data << 8) + word_data;
					source_ptr += 3;

					copy_ptr = dest_ptr + 4 - ((unsigned) dest_ptr & 0x3);
					count -= (copy_ptr - dest_ptr);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;

					word_dest_ptr = (unsigned*) dest_ptr;

					dest_ptr += (count & 0xfffffffc);

					while (word_dest_ptr < (unsigned*) dest_ptr) {
						*word_dest_ptr		= word_data;
						*(word_dest_ptr + 1) = word_data;
						word_dest_ptr += 2;
					}

					copy_ptr = dest_ptr + (count & 0x3);
					while (dest_ptr < copy_ptr) *dest_ptr++ = data;

				} else {

					if (op_code == 0xff) {

						/* Do a long copy from destination. */
						count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						copy_ptr = (unsigned char*) dest + *(source_ptr + 2) + ((unsigned) *(source_ptr + 3) << 8);
						source_ptr += 4;

						while (count--) *dest_ptr++ = *copy_ptr++;

					} else {

						/* Do a medium copy from destination. */
						count = (op_code & 0x3f) + 3;
						copy_ptr = (unsigned char*) dest + *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						source_ptr += 2;

						while (count--) *dest_ptr++ = *copy_ptr++;
					}
				}
			}
		}
	}
}


#if defined(_MSC_VER)


/*********************************************************************************************** 
 * LCW_Comp -- Performes LCW compression on a block of data.                                   * 
 *                                                                                             * 
 *    This routine will compress a block of data using the LCW compression method. LCW has     * 
 *    the primary characteristic of very fast uncompression at the expense of very slow        * 
 *    compression times.                                                                       * 
 *                                                                                             * 
 * INPUT:   source   -- Pointer to the source data to compress.                                * 
 *                                                                                             * 
 *          dest     -- Pointer to the destination location to store the compressed data       * 
 *                      to.                                                                    * 
 *                                                                                             * 
 *          datasize -- The size (in bytes) of the source data to compress.                    * 
 *                                                                                             * 
 * OUTPUT:  Returns with the number of bytes of output data stored into the destination        * 
 *          buffer.                                                                            * 
 *                                                                                             * 
 * WARNINGS:   Be sure that the destination buffer is big enough. The maximum size required    * 
 *             for the destination buffer is (datasize + datasize/128).                        * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/20/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
/*ARGSUSED*/
int LCW_Comp(void const * source, void * dest, int datasize)
{
	int retval = 0;
#ifdef _WINDOWS
	long inlen = 0;
	long a1stdest = 0;
	long a1stsrc = 0;
	long lenoff = 0;
	long ndest = 0;
	long count = 0;
	long matchoff = 0;
	long end_of_data =0;
#ifdef _DEBUG
	inlen = inlen;
	a1stdest = a1stdest;
	a1stsrc = a1stsrc;
	lenoff = lenoff;
	ndest = ndest;
	count = count;
	matchoff = matchoff;
	end_of_data = end_of_data;
#endif

	__asm {
		cld			// make sure all string commands are forward
		mov	edi,[dest]
		mov	esi,[source]
		mov	edx,[datasize]		// get length of data to compress

// compress data to the following codes in the format b = byte, w = word
// n = byte code pulled from compressed data
//   Bit field of n		command		description
// n=0xxxyyyy,yyyyyyyy		short run	back y bytes and run x+3
// n=10xxxxxx,n1,n2,...,nx+1	med length	copy the next x+1 bytes
// n=11xxxxxx,w1			med run		run x+3 bytes from offset w1
// n=11111111,w1,w2		long run	run w1 bytes from offset w2
// n=10000000			end		end of data reached

		mov	ebx,esi
		add	ebx,edx
		mov	[end_of_data],ebx
		mov	[inlen],1	//; set the in-length flag
		mov	[a1stdest],edi	//; save original dest offset for size calc
		mov	[a1stsrc],esi	//; save offset of first byte of data
		mov	[lenoff],edi	//; save the offset of the legth of this len
		sub	eax,eax
		mov	al,081h		//; the first byte is always a len
		stosb			//; write out a len of 1
		lodsb			//; get the byte
		stosb			//; save it
	}

loopstart:
	__asm {
		mov	[ndest],edi	//; save offset of compressed data
		mov	edi,[a1stsrc]	//; get the offset to the first byte of data
		mov	[count],1	//; set the count of run to 0
	}
searchloop:
	__asm {
		sub	eax,eax
		mov	al,[esi]	//; get the current byte of data
		cmp	al,[esi+64]
		jne	short notrunlength

		mov	ebx,edi

		mov	edi,esi
		mov	ecx,[end_of_data]
		sub	ecx,edi
		repe	scasb
		dec	edi
		mov	ecx,edi
		sub	ecx,esi
		cmp	ecx,65
		jb	short notlongenough

		mov	[inlen],0	//; clear the in-length flag
//		mov	[DWORD PTR inlen],0	//; clear the in-length flag
		mov	esi,edi
		mov	edi,[ndest]	//; get the offset of our compressed data

		mov	ah,al
		mov	al,0FEh
		stosb
		xchg	ecx,eax
		stosw
		mov	al,ch
		stosb

		mov	[ndest],edi	//; save offset of compressed data
		mov	edi,ebx
		jmp	searchloop
	}
notlongenough:
	__asm {
		mov	edi,ebx
	}
notrunlength:
oploop:
	__asm {
		mov	ecx,esi		//; get the address of the last byte +1
		sub	ecx,edi		//; get the total number of bytes left to comp
		jz	short searchdone

		repne	scasb		//; look for a match
		jne	short searchdone	//; if we don't find one we're done

		mov	ebx,[count]
		mov	ah,[esi+ebx-1]
		cmp	ah,[edi+ebx-2]

		jne	oploop

		mov	edx,esi		//; save this spot for the next search
		mov	ebx,edi		//; save this spot for the length calc
		dec	edi		//; back up one for compare
		mov	ecx,[end_of_data]		//; get the end of data
		sub	ecx,esi		//; sub current source for max len

		repe	cmpsb		//; see how many bytes match

		jne	short notend	//; if found mismatch then di - bx = match count

		inc	edi		//; else cx = 0 and di + 1 - bx = match count
	}
notend:
	__asm {
		mov	esi,edx		//; restore si
		mov	eax,edi		//; get the dest
		sub	eax,ebx		//; sub the start for total bytes that match
		mov	edi,ebx		//; restore dest
		cmp	eax,[count]	//; see if its better than before
		jb	searchloop	//; if not keep looking

		mov	[count],eax	//; if so keep the count
		dec	ebx		//; back it up for the actual match offset
		mov	[matchoff],ebx //; save the offset for later
		jmp	searchloop	//; loop until we searched it all
	}
searchdone:
	__asm {
		mov	ecx,[count]	//; get the count of the longest run
		mov	edi,[ndest]	//; get the offset of our compressed data
		cmp	ecx,2		//; see if its not enough run to matter
		jbe	short lenin		//; if its 0,1, or 2 its too small

		cmp	ecx,10		//; if not, see if it would fit in a short
		ja	short medrun	//; if not, see if its a medium run

		mov	eax,esi		//; if its short get the current address
		sub	eax,[matchoff] //; sub the offset of the match
		cmp	eax,0FFFh	//; if its less than 12 bits its a short
		ja	short medrun	//; if its not, its a medium
	}
//shortrun:
	__asm {
		sub	ebx,ebx
		mov	bl,cl		//; get the length (3-10)
		sub	bl,3		//; sub 3 for a 3 bit number 0-7
		shl	bl,4		//; shift it left 4
		add	ah,bl		//; add in the length for the high nibble
		xchg	ah,al		//; reverse the bytes for a word store
		jmp	short srunnxt	//; do the run fixup code
	}
medrun:
	__asm {
		cmp	ecx,64		//; see if its a short run
		ja	short longrun	//; if not, oh well at least its long

		sub	cl,3		//; back down 3 to keep it in 6 bits
		or	cl,0C0h		//; the highest bits are always on
		mov	al,cl		//; put it in al for the stosb
		stosb			//; store it
		jmp	short medrunnxt //; do the run fixup code
	}
lenin:
	__asm {
		cmp	[inlen],0	//; is it doing a length?
//		cmp	[DWORD PTR inlen],0	//; is it doing a length?
		jnz	short len	//; if so, skip code
	}
lenin1:
	__asm {
		mov	[lenoff],edi	//; save the length code offset
		mov	al,80h		//; set the length to 0
		stosb			//; save it
	}
len:
	__asm {
		mov	ebx,[lenoff]	//; get the offset of the length code
		cmp	[ebx],0BFh	//; see if its maxed out
//		cmp	[BYTE PTR ebx],0BFh	//; see if its maxed out
		je	lenin1	//; if so put out a new len code
	}
//stolen:
	__asm {
		inc	[ebx] //; inc the count code
//		inc	[BYTE PTR ebx] //; inc the count code
		lodsb			//; get the byte
		stosb			//; store it
		mov	[inlen],1	//; we are now in a length so save it
//		mov	[DWORD PTR inlen],1	//; we are now in a length so save it
		jmp	short nxt	//; do the next code
	}
longrun:
	__asm {
		mov	al,0ffh		//; its a long so set a code of FF
		stosb			//; store it

		mov	eax,[count]	//; send out the count
		stosw			//; store it
	}
medrunnxt:
	__asm {
		mov	eax,[matchoff] //; get the offset
		sub	eax,[a1stsrc]	//; make it relative tot he start of data
	}
srunnxt:
	__asm {
		stosw			//; store it
		//; this code common to all runs
		add	esi,[count]	//; add in the length of the run to the source
		mov	[inlen],0	//; set the in leght flag to false
//		mov	[DWORD PTR inlen],0	//; set the in leght flag to false
	}
nxt:
	__asm {
		cmp	esi,[end_of_data]		//; see if we did the whole pic
		jae	short outofhere		//; if so, cool! were done

		jmp	loopstart
	}
outofhere:
	__asm {
		mov	ax,080h		//; remember to send an end of data code
		stosb			//; store it
		mov	eax,edi		//; get the last compressed address
		sub	eax,[a1stdest]	//; sub the first for the compressed size
		mov	[retval],eax
	}
#endif
	return(retval);
}
#endif


