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
 *                     $Archive:: /Commando/Library/PCX.H                                     $* 
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

#ifndef PCX_H
#define PCX_H

#include	"bsurface.h"
#include	"PALETTE.H"
#include	"WWFILE.H"
#include	<string.h>


struct RGB {
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};

struct PCX_HEADER
{
	char	id;
	char	version;
	char	encoding;
	char	pixelsize;
	short	x;
	short	y;
	short	width;
	short	height;
	short	xres;
	short	yres;
	RGB	ega_palette[16];
	char	nothing;
	char	color_planes;
	unsigned short	byte_per_line;
	short	palette_type;
	char	filler[58];
};


Surface * Read_PCX_File(FileClass & file_handle, PaletteClass * palette= NULL, void * buff=NULL, long size=0);
//Surface * Read_PCX_File (char * name, Buffer & Buff, PaletteClass * palette= NULL) ;
bool Write_PCX_File(FileClass & file, Surface & pic, PaletteClass * palette);

#endif
