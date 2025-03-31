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
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shddump.h                           $*
 *                                                                                             *
 *                  $Org Author:: Kenny_m
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 8/05/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
// header file for use in wdump util only

#ifndef SHDDUMP_H
#define SHDDUMP_H

#ifndef SHDCLASSIDS_H
#include "shdclassids.h"
#endif

const char* Shader_ClassIDs[SHDDEF_CLASSID_LAST]=
{
	"SHDDEF_CLASSID_DUMMY",
	"SHDDEF_CLASSID_SIMPLE",
	"SHDDEF_CLASSID_GLOSSMASK",
	"SHDDEF_CLASSID_BUMPSPEC",
	"SHDDEF_CLASSID_BUMPDIFF",
	"SHDDEF_CLASSID_CUBEMAP"
};

struct ShdDef_ChunkStruct
{
	DWORD DefChunkId;
	char	Pad0[6];
	char  DefName[8];
	char	Pad1[2];
	int	SurfaceType;
	DWORD ShdChunkId;
	char	Pad2[6];
	char	TexName[];
};


#endif //SHDDUMP_H
