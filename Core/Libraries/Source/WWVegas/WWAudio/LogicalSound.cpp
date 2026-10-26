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

////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LogicalSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
LogicalSoundClass::LogicalSoundClass()
  : m_DropOffRadius(1)
  , m_TypeMask(0)
  , m_Position(0, 0, 0)
  , m_IsSingleShot(false)
  , m_OldestListenerTimestamp(0)
  , m_MaxListeners(0)
  , m_NotifyDelayInMS(2000)
  , m_LastNotification(0)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~LogicalSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
LogicalSoundClass::~LogicalSoundClass()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void LogicalSoundClass::Add_To_Scene(bool /*start_playing*/)
{
	SoundSceneClass* scene = WWAudioClass::Get_Instance()->Get_Sound_Scene();
	if ((scene != nullptr) && (m_Scene == nullptr))
	{
		//
		//	Add this sound to the culling system
		//
		m_Scene = scene;
		scene->Add_Logical_Sound(this, m_IsSingleShot);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void LogicalSoundClass::Remove_From_Scene()
{
	if (m_Scene != nullptr)
	{
		//
		//	Remove this sound from the culling system
		//
		m_Scene->Remove_Logical_Sound(this, m_IsSingleShot);
		m_Scene = nullptr;
		m_PhysWrapper = nullptr;
		m_LastNotification = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Notify
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool LogicalSoundClass::Allow_Notify(uint32 timestamp)
{
	bool retval = false;

	if (m_IsSingleShot || timestamp > (m_LastNotification + m_NotifyDelayInMS))
	{
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
bool LogicalSoundClass::On_Frame_Update(unsigned int milliseconds)
{
	//
	// Update the sound's position if its linked to a render object
	//
	Apply_Auto_Position();
	return SoundSceneObjClass::On_Frame_Update(milliseconds);
}
