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
 *                     $Archive:: /Commando/Code/WWAudio/LogicalSound.cpp                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/02/99 10:18a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "LogicalSound.h"
#include "WWAudio.h"
#include "SoundScene.h"
#include "SoundChunkIDs.h"
#include "persistfactory.h"


//////////////////////////////////////////////////////////////////////////////////
//	Static factories
//////////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<LogicalSoundClass, CHUNKID_LOGICALSOUND> _LogicalSoundPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//	Save/Load constants
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x03270459,
	CHUNKID_BASE_CLASS
};

enum
{
	VARID_DROP_OFF_RADIUS		= 0x01,
	VARID_IS_SINGLE_SHOT,
	VARID_TYPE_MASK,
	VARID_XXX1,
	VARID_POSITION,
	VARID_MAX_LISTENERS,
	VARID_NOTIFY_DELAY,
	VARID_LAST_NOTIFY,
};


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LogicalSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
LogicalSoundClass::LogicalSoundClass (void)
	:	m_DropOffRadius (1),
		m_TypeMask (0),
		m_Position (0, 0, 0),
		m_IsSingleShot (false),
		m_OldestListenerTimestamp (0),
		m_MaxListeners (0),
		m_NotifyDelayInMS (2000),
		m_LastNotification (0)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~LogicalSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
LogicalSoundClass::~LogicalSoundClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
LogicalSoundClass::Add_To_Scene (bool /*start_playing*/)
{
	SoundSceneClass *scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
	if ((scene != NULL) && (m_Scene == NULL)) {

		//
		//	Add this sound to the culling system
		//
		m_Scene = scene;
		scene->Add_Logical_Sound (this, m_IsSingleShot);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
LogicalSoundClass::Remove_From_Scene (void)
{
	if (m_Scene != NULL) {

		//
		//	Remove this sound from the culling system
		//
		m_Scene->Remove_Logical_Sound (this, m_IsSingleShot);
		m_Scene					= NULL;
		m_PhysWrapper			= NULL;
		m_LastNotification	= 0;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Notify
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
LogicalSoundClass::Allow_Notify (uint32 timestamp)
{
	bool retval = false;

	if (m_IsSingleShot || timestamp > (m_LastNotification + m_NotifyDelayInMS)) {
		retval = true;
		m_LastNotification = timestamp;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
LogicalSoundClass::On_Frame_Update (unsigned int milliseconds)
{
	//
	// Update the sound's position if its linked to a render object
	//
	Apply_Auto_Position ();
	return SoundSceneObjClass::On_Frame_Update (milliseconds);
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
/////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
LogicalSoundClass::Get_Factory (void) const
{
	return _LogicalSoundPersistFactory;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
LogicalSoundClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		SoundSceneObjClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);

		WRITE_MICRO_CHUNK (csave, VARID_DROP_OFF_RADIUS, m_DropOffRadius);
		WRITE_MICRO_CHUNK (csave, VARID_IS_SINGLE_SHOT, m_IsSingleShot);
		WRITE_MICRO_CHUNK (csave, VARID_TYPE_MASK, m_TypeMask);
		WRITE_MICRO_CHUNK (csave, VARID_POSITION, m_Position);
		WRITE_MICRO_CHUNK (csave, VARID_MAX_LISTENERS, m_MaxListeners);
		WRITE_MICRO_CHUNK (csave, VARID_NOTIFY_DELAY, m_NotifyDelayInMS);
		WRITE_MICRO_CHUNK (csave, VARID_LAST_NOTIFY, m_LastNotification);

	csave.End_Chunk ();
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
LogicalSoundClass::Load (ChunkLoadClass &cload)
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

						READ_MICRO_CHUNK (cload, VARID_DROP_OFF_RADIUS, m_DropOffRadius);
						READ_MICRO_CHUNK (cload, VARID_IS_SINGLE_SHOT, m_IsSingleShot);
						READ_MICRO_CHUNK (cload, VARID_TYPE_MASK, m_TypeMask);
						READ_MICRO_CHUNK (cload, VARID_POSITION, m_Position);
						READ_MICRO_CHUNK (cload, VARID_MAX_LISTENERS, m_MaxListeners);
						READ_MICRO_CHUNK (cload, VARID_NOTIFY_DELAY, m_NotifyDelayInMS);
						READ_MICRO_CHUNK (cload, VARID_LAST_NOTIFY, m_LastNotification);
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

