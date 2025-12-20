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
 *                     $Archive:: /Commando/Code/WWAudio/AudioSaveLoad.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/12/00 4:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "saveloadsubsystem.h"
#include "Vector.h"
#include "bittype.h"


// Singleton instances
extern class StaticAudioSaveLoadClass _StaticAudioSaveLoadSubsystem;
extern class DynamicAudioSaveLoadClass _DynamicAudioSaveLoadSubsystem;


//////////////////////////////////////////////////////////////////////////
//
//	StaticAudioSaveLoadClass
//
//////////////////////////////////////////////////////////////////////////
class StaticAudioSaveLoadClass : public SaveLoadSubSystemClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	StaticAudioSaveLoadClass (void)				{ }
	virtual ~StaticAudioSaveLoadClass (void)	{ }

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	// From SaveLoadSubSystemClass
	virtual uint32				Chunk_ID (void) const;

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	virtual bool				Contains_Data(void) const;
	virtual bool				Save (ChunkSaveClass &csave);
	virtual bool				Load (ChunkLoadClass &cload);
	virtual const char*		Name() const { return "StaticAudioSaveLoadClass"; }
};


//////////////////////////////////////////////////////////////////////////
//
//	DynamicAudioSaveLoadClass
//
//////////////////////////////////////////////////////////////////////////
class DynamicAudioSaveLoadClass : public SaveLoadSubSystemClass
{
public:

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	DynamicAudioSaveLoadClass (void)				{ }
	virtual ~DynamicAudioSaveLoadClass (void)	{ }

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////

	// From SaveLoadSubSystemClass
	virtual uint32				Chunk_ID (void) const;

protected:

	//////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////
	virtual bool				Contains_Data(void) const;
	virtual bool				Save (ChunkSaveClass &csave);
	virtual bool				Load (ChunkLoadClass &cload);
	virtual const char*		Name() const { return "DynamicAudioSaveLoadClass"; }

	//bool							Save_Micro_Chunks (ChunkSaveClass &csave);
	//bool							Load_Micro_Chunks (ChunkLoadClass &cload);
};
