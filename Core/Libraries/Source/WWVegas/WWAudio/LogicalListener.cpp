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
 *                 Project Name : WWAudio                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/LogicalListener.cpp       $*
 *																															  *
 *							  $Modtime:: 8/14/01 11:45a                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "LogicalListener.h"
#include "WWAudio.h"
#include "SoundScene.h"
#include "SoundChunkIDs.h"
#include "persistfactory.h"


//////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////////////////
uint32		LogicalListenerClass::m_OldestTimestamp	= 0;
uint32		LogicalListenerClass::m_NewestTimestamp	= 1;
float			LogicalListenerClass::m_GlobalScale			= 1.0F;


//////////////////////////////////////////////////////////////////////////////////
//	Static factories
//////////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<LogicalListenerClass, CHUNKID_LOGICALLISTENER> _LogicalListenerPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//	Save/Load constants
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x03270542,
	CHUNKID_BASE_CLASS
};

enum
{
	XXXVARID_DROP_OFF_RADIUS		= 0x01,
	VARID_TYPE_MASK,
	VARID_XXX1,
	VARID_POSITION,
	VARID_SCALE
};


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LogicalListenerClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
LogicalListenerClass::LogicalListenerClass (void)
	:	m_Scale (1),
		m_TypeMask (0),
		m_Position (0, 0, 0),
		m_Timestamp (0)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~LogicalListenerClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
LogicalListenerClass::~LogicalListenerClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
LogicalListenerClass::Add_To_Scene (bool /*start_playing*/)
{
	SoundSceneClass *scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
	if ((scene != NULL) && (m_Scene == NULL)) {

		//
		//	Add this listener to the culling system
		//
		m_Scene = scene;
		scene->Add_Logical_Listener (this);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
LogicalListenerClass::Remove_From_Scene (void)
{
	if (m_Scene != NULL) {

		//
		//	Remove this listener from the culling system
		//
		m_Scene->Remove_Logical_Listener (this);
		m_Scene = NULL;
		m_PhysWrapper = NULL;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
/////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
LogicalListenerClass::Get_Factory (void) const
{
	return _LogicalListenerPersistFactory;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
LogicalListenerClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		SoundSceneObjClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		WRITE_MICRO_CHUNK (csave, VARID_SCALE, m_Scale);
		WRITE_MICRO_CHUNK (csave, VARID_TYPE_MASK, m_TypeMask);
		WRITE_MICRO_CHUNK (csave, VARID_POSITION, m_Position);

	csave.End_Chunk ();
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
LogicalListenerClass::Load (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				PersistClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
			{
				//
				//	Read all the variables from their micro-chunks
				//
				while (cload.Open_Micro_Chunk ()) {
					switch (cload.Cur_Micro_Chunk_ID ()) {

						READ_MICRO_CHUNK (cload, VARID_SCALE, m_Scale);
						READ_MICRO_CHUNK (cload, VARID_TYPE_MASK, m_TypeMask);
						READ_MICRO_CHUNK (cload, VARID_POSITION, m_Position);
					}

					cload.Close_Micro_Chunk ();
				}
			}
			break;
		}

		cload.Close_Chunk ();
	}

	return true;
}

