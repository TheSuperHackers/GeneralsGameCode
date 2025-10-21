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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/SoundChunkIDs.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/27/00 5:43p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef __SOUND_CHUNK_IDS_H
#define __SOUND_CHUNK_IDS_H

#include "saveloadids.h"
#include "definitionclassids.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	These are the chunk IDs that serve as 'globally-unique' persist identifiers for
//	all persist objects inside the editor.  These are used when building the
//	PersistFactoryClass's for definitions.
//
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_SOUND_DEF			= CHUNKID_WWAUDIO_BEGIN,
	CHUNKID_AUDIBLE_SOUND,
	CHUNKID_FILTERED_SOUND,
	CHUNKID_SOUND3D,
	CHUNKID_PSEUDO_SOUND3D,
	CHUNKID_STATIC_SAVELOAD,
	CHUNKID_DYNAMIC_SAVELOAD,
	CHUNKID_LOGICALSOUND,
	CHUNKID_LOGICALLISTENER
};


//////////////////////////////////////////////////////////////////////////////////
//
//	These are the globally-unique class identifiers that the definition system
// uses inside the editor.
//
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CLASSID_SOUND_DEF			= CLASSID_SOUND,
};


#endif //__SOUND_CHUNK_IDS_H
