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
 *                     $Archive:: /Commando/Code/Library/PCX.cpp                              $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 9/28/98 12:06p                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"pcx.H"
#include <stdlib.h>


/***************************************************************************
 * READ_PCX_FILE -- read a pcx file into a Graphic Buffer                  *
 *                                                                         *
 *	GraphicBufferClass* Read_PCX_File (char* name, char* palette,void *Buff, long size );	*
 *  																								*
 *                                                                         *
 * INPUT: name is a NULL terminated string of the format [xxxx.pcx]        *
 *        palette is optional, if palette != NULL the the color palette of *
 *					 the pcx file will be place in the memory block pointed	   *
 *               by palette.																*
 *			 Buff is optional, if Buff == NULL a new memory Buffer		 		*
 *					 will be allocated, otherwise the file will be placed 		*
 *					 at location pointed by Buffer;										*
 *			Size is the size in bytes of the memory block pointed by Buff		*
 *				  is also optional;															*                                                                         *
 * OUTPUT: on success a pointer to a GraphicBufferClass containing the     *
 *         pcx file, NULL otherwise.                                       *
 *																									*
 * WARNINGS:                                                               *
 *         Appears to be a comment-free zone                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/03/1995 JRJ : Created.                                             *
 *   04/30/1996 ST : Tidied up and modified to use CCFileClass             *
 *=========================================================================*/
#define	POOL_SIZE 2048
#define	READ_CHAR()  *file_ptr++ ; \
							 if ( file_ptr	>= & pool [ POOL_SIZE ]	) { \
								 file_handle.Read (pool, POOL_SIZE ); \
								 file_ptr = pool ; \
							 }
#define	READ_CHARx()  *file_ptr++ ; \
							 if ( file_ptr	>= & pool [ POOL_SIZE ]	) { \
								 file_handle.Read (pool, POOL_SIZE ); \
							 }


Surface * Read_PCX_File(FileClass & file_handle, PaletteClass * palette, void * Buff, long Size)
{
	unsigned					i, j;
	unsigned					rle;
	unsigned					color;
	unsigned					scan_pos;
	char						*file_ptr;
	unsigned					width;
	unsigned					height;
	char						*buffer;
	PCX_HEADER				header;
	char						pool [POOL_SIZE];
	BSurface * pic;

	if (!file_handle.Is_Available()) return (NULL);

	file_handle.Open(FileClass::READ);

	file_handle.Read (&header, sizeof (PCX_HEADER));

	if (header.id != 10 &&  header.version != 5 && header.pixelsize != 8 ) return NULL ;

	width = header.width - header.x + 1;
	height = header.height - header.y + 1;

	if (Buff != NULL) {
    	i = Size / width;
    	height = MIN ((int)(i - 1), (int)height);
		Buffer b(Buff, Size);
    	pic = W3DNEW BSurface(width, height, 1, &b);
    	if (pic == NULL) return NULL ;
	} else {
    	pic = W3DNEW BSurface(width, height, 1);
    	if (pic == NULL) return NULL ;
	}

	buffer = (char *)pic->Lock();
	if (buffer != NULL) {
		file_ptr = pool ;
		file_handle.Read (pool, POOL_SIZE);

		if ( header.byte_per_line != width ) {

			i = 0;
			rle = 0;
			for ( scan_pos = j = 0 ; j < height ; j ++, scan_pos += width ) {
				for ( i = 0 ; i < width ; ) {
					rle = READ_CHAR ();
					if ( rle > 192 ) {
						rle -= 192 ;
						color =	READ_CHAR (); ;
						memset ( buffer + scan_pos + i, color, rle );
						i += rle;
					} else {
						*(buffer+scan_pos + i++ ) = (char)rle;
					}
				}
     		}

			if ( i == width ) rle = READ_CHAR ();
			if ( rle > 192 )  READ_CHARx();

		} else {

			for ( i = 0 ; i < width * height ; ) {
  				rle = READ_CHAR ();
  				rle &= 0xff;
  				if ( rle > 192 ) {
        			rle -= 192 ;
        			color = READ_CHAR ();
  					memset ( buffer + i, color, rle );
        			i += rle ;
     			} else {
					*(buffer + i++) = (char)rle;
				}
			}
		}
		pic->Unlock();
	}

	if ( palette ) {
		file_handle.Seek (- (256 * (int)sizeof(RGB)), SEEK_END );
		file_handle.Read (palette, 256L * sizeof ( RGB ));
	}

	file_handle.Close();
	return pic;
}


