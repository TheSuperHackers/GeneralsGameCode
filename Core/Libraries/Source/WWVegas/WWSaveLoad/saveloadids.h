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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwsaveload/saveloadids.h                     $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/04/01 8:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef SAVELOADIDS_H
#define SAVELOADIDS_H

//////////////////////////////////////////////////////////////////////////////////
//
//	Chunk ID Ranges.
// Here are the ranges to use for SubSystem and PersistFactory chunk id's
// and also the chunk id's used by the save-load system itself.  If you are
// creating a new library that is going to take advantage of this system,
// create a new range for it here.
//
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_SAVELOAD_BEGIN				= 0x00000100,
		CHUNKID_SAVELOAD_DEFMGR,
		CHUNKID_TWIDDLER,
	CHUNKID_WW3D_BEGIN					= 0x00010000,
	CHUNKID_WWPHYS_BEGIN					= 0x00020000,
	CHUNKID_WWAUDIO_BEGIN				= 0x00030000,
	CHUNKID_COMBAT_BEGIN					= 0x00040000,
	CHUNKID_COMMANDO_EDITOR_BEGIN		= 0x00050000,
	CHUNKID_PHYSTEST_BEGIN				= 0x00060000,
	CHUNKID_COMMANDO_BEGIN				= 0x00070000,
	CHUNKID_WWMATH_BEGIN					= 0x00080000,
	CHUNKID_WWTRANSLATEDB_BEGIN		= 0x00090000
};


#endif
