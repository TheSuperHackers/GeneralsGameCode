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
 *                     $Archive:: /G/wwlib/iff.h                                              $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 9/23/99 1:46p                                               $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef IFF_H
#define IFF_H

#ifndef _WINDOWS
#define __cdecl
#endif

#include	"BUFF.H"
#include	<stddef.h>

#define LZW_SUPPORTED			FALSE

/*=========================================================================*/
/* Iff and Load Picture system defines and enumerations							*/
/*=========================================================================*/

#define 	MAKE_ID(a,b,c,d)			((long) ((long) d << 24) | ((long) c << 16) | ((long) b <<  8) | (long)(a))
#define	IFFize_WORD(a)			Reverse_Word(a)
#define	IFFize_LONG(a)			Reverse_Long(a)


//lint -strong(AJX,PicturePlaneType)
typedef enum {
	BM_AMIGA,	// Bit plane format (8K per bitplane).
	BM_MCGA,		// Byte per pixel format (64K).

	BM_DEFAULT=BM_MCGA	// Default picture format.
} PicturePlaneType;

/*
**	This is the compression type code.  This value is used in the compressed
**	file header to indicate the method of compression used.  Note that the
**	LZW method may not be supported.
*/
//lint -strong(AJX,CompressionType)
typedef enum {
	NOCOMPRESS,		// No compression (raw data).
	LZW12,			// LZW 12 bit codes.
	LZW14,			// LZW 14 bit codes.
	HORIZONTAL,		// Run length encoding (RLE).
	LCW				// Westwood proprietary compression.
} CompressionType;

/*
**	Compressed blocks of data must start with this header structure.
**	Note that disk based compressed files have an additional two
**	leading bytes that indicate the size of the entire file.
*/
//lint -strong(AJX,CompHeaderType)
typedef struct {
	char	Method;		// Compression method (CompressionType).
	char	pad;			// Reserved pad byte (always 0).
	long	Size;			// Size of the uncompressed data.
	short	Skip;			// Number of bytes to skip before data.
} CompHeaderType;


/*=========================================================================*/
/* The following prototypes are for the file: IFF.CPP								*/
/*=========================================================================*/

int __cdecl Open_Iff_File(char const *filename);
void __cdecl Close_Iff_File(int fh);
unsigned long __cdecl Get_Iff_Chunk_Size(int fh, long id);
unsigned long __cdecl Read_Iff_Chunk(int fh, long id, void *buffer, unsigned long maxsize);
void __cdecl Write_Iff_Chunk(int file, long id, void *buffer, long length);


/*=========================================================================*/
/* The following prototypes are for the file: LOADPICT.CPP						*/
/*=========================================================================*/

//int __cdecl Load_Picture(char const *filename, BufferClass& scratchbuf, BufferClass& destbuf, unsigned char *palette=NULL, PicturePlaneType format=BM_DEFAULT);


/*=========================================================================*/
/* The following prototypes are for the file: LOAD.CPP							*/
/*=========================================================================*/

unsigned long __cdecl Load_Data(char const *name, void *ptr, unsigned long size);
unsigned long __cdecl Write_Data(char const *name, void *ptr, unsigned long size);
//void * __cdecl Load_Alloc_Data(char const *name, MemoryFlagType flags);
unsigned long __cdecl Load_Uncompress(char const *file, Buffer & uncomp_buff, Buffer & dest_buff, void *reserved_data=NULL);
unsigned long __cdecl Uncompress_Data(void const *src, void *dst);
void __cdecl Set_Uncomp_Buffer(int buffer_segment, int size_of_buffer);

/*=========================================================================*/
/* The following prototypes are for the file: WRITELBM.CPP						*/
/*=========================================================================*/

//bool Write_LBM_File(int lbmhandle, BufferClass& buff, int bitplanes, unsigned char *palette);



/*========================= Assembly Functions ============================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================================*/
/* The following prototypes are for the file: PACK2PLN.ASM						*/
/*=========================================================================*/

extern void __cdecl Pack_2_Plane(void *buffer, void * pageptr, int planebit);

/*=========================================================================*/
/* The following prototypes are for the file: LCWCOMP.ASM						*/
/*=========================================================================*/

extern unsigned long __cdecl LCW_Compress(void *source, void *dest, unsigned long length);

/*=========================================================================*/
/* The following prototypes are for the file: LCWUNCMP.ASM						*/
/*=========================================================================*/

extern unsigned long __cdecl LCW_Uncompress(void *source, void *dest, unsigned long length);

#ifdef __cplusplus
}
#endif
/*=========================================================================*/



#endif //IFF_H
