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
 *                     $Archive:: /Commando/Code/WWAudio/SoundSceneObj.cpp      $*
 *																														     *
 *							  $Modtime:: 8/24/01 5:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "SoundSceneObj.h"
#include "camera.h"
#include "rendobj.h"
#include "persistfactory.h"
#include "SoundChunkIDs.h"
#include "Utils.h"


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
	VARID_ATTACHED_OBJ		= 0x01,
	VARID_ATTACHED_BONE,
	VARID_USER_DATA,
	VARID_USER_OBJ,
	VARID_ID
};


//////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////////////////
DynamicVectorClass<SoundSceneObjClass *>	SoundSceneObjClass::m_GlobalSoundList;
uint32			SoundSceneObjClass::m_NextAvailableID	= SOUND_OBJ_START_ID;
CriticalSectionClass	SoundSceneObjClass::m_IDListMutex;


//////////////////////////////////////////////////////////////////////////////////
//	Mutex managment
//////////////////////////////////////////////////////////////////////////////////
/*
class HandleMgrClass
{
public:
	HandleMgrClass (void)	{ SoundSceneObjClass::m_IDListMutex = ::CreateMutex (nullptr, FALSE, nullptr); }
	~HandleMgrClass (void)	{ ::CloseHandle (SoundSceneObjClass::m_IDListMutex); }

};

HandleMgrClass _GlobalMutexHandleMgr;
*/

////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SoundSceneObjClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundSceneObjClass::SoundSceneObjClass (void)
	:	m_Scene (nullptr),
		m_PhysWrapper (nullptr),
		m_pCallback (nullptr),
		m_AttachedObject (nullptr),
		m_UserData (0),
		m_UserObj (nullptr),
		m_ID (SOUND_OBJ_DEFAULT_ID),
		m_RegisteredEvents (AudioCallbackClass::EVENT_NONE)
{
	m_ID = m_NextAvailableID ++;

	Register_Sound_Object (this);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SoundSceneObjClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundSceneObjClass::SoundSceneObjClass (const SoundSceneObjClass &src)
	:	m_Scene (nullptr),
		m_PhysWrapper (nullptr),
		m_pCallback (nullptr),
		m_AttachedObject (nullptr),
		m_UserData (0),
		m_UserObj (nullptr),
		m_ID (SOUND_OBJ_DEFAULT_ID),
		m_RegisteredEvents (AudioCallbackClass::EVENT_NONE)
{
	m_ID = m_NextAvailableID ++;

	(*this) = src;
	Register_Sound_Object (this);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~SoundSceneObjClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundSceneObjClass::~SoundSceneObjClass (void)
{
	REF_PTR_RELEASE (m_UserObj);
	REF_PTR_RELEASE (m_AttachedObject);
	Unregister_Sound_Object (this);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////////////////////////////////////
const SoundSceneObjClass &
SoundSceneObjClass::operator= (const SoundSceneObjClass &src)
{
	m_Scene					= src.m_Scene;
	m_pCallback				= src.m_pCallback;
	m_RegisteredEvents	= src.m_RegisteredEvents;

	Attach_To_Object (src.m_AttachedObject, src.m_AttachedBone);

	PersistClass::operator= ((const PersistClass &)src);
	return (*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Attach_To_Object
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneObjClass::Attach_To_Object
(
	RenderObjClass *	render_obj,
	const char *		bone_name
)
{
	REF_PTR_SET (m_AttachedObject, render_obj);

	if (m_AttachedObject != nullptr && bone_name != nullptr) {
		m_AttachedBone = m_AttachedObject->Get_Bone_Index (bone_name);
	} else {
		m_AttachedBone = -1;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Attach_To_Object
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneObjClass::Attach_To_Object
(
	RenderObjClass *	render_obj,
	int					bone_index
)
{
	if (m_AttachedObject != render_obj || m_AttachedBone != bone_index) {

		//
		//	Record the attachment
		//
		REF_PTR_SET (m_AttachedObject, render_obj);
		m_AttachedBone = bone_index;

		//
		//	Update the transform
		//
		Apply_Auto_Position ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Apply_Auto_Position
//
//////////////////////////////////////////////////////////////////////////////
void
SoundSceneObjClass::Apply_Auto_Position (void)
{
	// If the sound is attached to an object, then update its transform
	// based on this link.
	if (m_AttachedObject != nullptr) {

		// Determine which transform to use
		Matrix3D transform (1);
		if (m_AttachedBone >= 0) {
			transform = m_AttachedObject->Get_Bone_Transform (m_AttachedBone);
		} else {
			transform = m_AttachedObject->Get_Transform ();

			//
			//	Convert the camera's transform to an object transform
			//
			if (m_AttachedObject->Class_ID () == RenderObjClass::CLASSID_CAMERA) {
				Matrix3D cam_to_world (Vector3 (0, 0, -1), Vector3 (-1, 0, 0), Vector3 (0, 1, 0), Vector3 (0, 0, 0));
#ifdef ALLOW_TEMPORARIES
				transform = transform * cam_to_world;
#else
				transform.postMul(cam_to_world);
#endif
			}
		}

		// Update the sound's transform
		Set_Transform (transform);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneObjClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		PersistClass::Save (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_ATTACHED_OBJ, m_AttachedObject);
		WRITE_MICRO_CHUNK (csave, VARID_ATTACHED_BONE, m_AttachedBone);
		WRITE_MICRO_CHUNK (csave, VARID_USER_DATA, m_UserData);
		WRITE_MICRO_CHUNK (csave, VARID_USER_OBJ, m_UserObj);
		WRITE_MICRO_CHUNK (csave, VARID_ID, m_ID);
	csave.End_Chunk ();
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneObjClass::Load (ChunkLoadClass &cload)
{
	uint32 id = SOUND_OBJ_DEFAULT_ID;

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

						READ_MICRO_CHUNK (cload, VARID_ATTACHED_OBJ, m_AttachedObject);
						READ_MICRO_CHUNK (cload, VARID_ATTACHED_BONE, m_AttachedBone);
						READ_MICRO_CHUNK (cload, VARID_USER_DATA, m_UserData);
						READ_MICRO_CHUNK (cload, VARID_USER_OBJ, m_UserObj);
						READ_MICRO_CHUNK (cload, VARID_ID, id);
					}

					cload.Close_Micro_Chunk ();
				}
			}
			break;
		}

		cload.Close_Chunk ();
	}

	//
	//	Set the ID (this will cause the sound object to
	// be re-inserted in the master sorted list)
	//
	if (id != SOUND_OBJ_DEFAULT_ID) {
		Set_ID (id);
	}

	//
	//	Max sure the next available ID is the largest ID in existence
	//
	m_NextAvailableID = max (m_NextAvailableID, m_ID + 1);

	//
	//	We need to 'swizzle' the attached object pointer.  We saved the pointer's
	// value, and need to map it (hopefully) to the new value.
	//
	if (m_AttachedObject != nullptr) {
		SaveLoadSystemClass::Request_Ref_Counted_Pointer_Remap ((RefCountClass **)&m_AttachedObject);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
//////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneObjClass::On_Frame_Update (unsigned int /*milliseconds*/)
{
	Apply_Auto_Position ();
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_ID
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundSceneObjClass::Set_ID (uint32 id)
{
	//
	//	Remove the sound object from our sorted list
	//
	Unregister_Sound_Object (this);

	//
	//	Change the sound object's ID
	//
	m_ID = id;

	//
	//	Reinsert the sound object in our sorted list
	//
	Register_Sound_Object (this);
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Register_Sound_Object
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundSceneObjClass::Register_Sound_Object (SoundSceneObjClass *sound_obj)
{
	int sound_id = sound_obj->Get_ID ();
	CriticalSectionClass::LockClass lock(m_IDListMutex);

	//
	//	Special case a non-ID
	//
	if (sound_id == SOUND_OBJ_DEFAULT_ID) {
		m_GlobalSoundList.Insert (0, sound_obj);
	} else {

		//
		//	Check to ensure the object isn't already in the list
		//
		int index = 0;
		if (Find_Sound_Object (sound_id, &index) == false) {

			//
			//	Insert the object into the list
			//
			m_GlobalSoundList.Insert (index, sound_obj);
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Unregister_Sound_Object
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundSceneObjClass::Unregister_Sound_Object (SoundSceneObjClass *sound_obj)
{
	CriticalSectionClass::LockClass lock(m_IDListMutex);

	//
	//	Try to find the object in the list
	//
	int index = 0;
	if (Find_Sound_Object (sound_obj->Get_ID (), &index)) {

		//
		//	Remove the object from the list
		//
		m_GlobalSoundList.Delete (index);
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Sound_Object
//
//////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneObjClass::Find_Sound_Object (uint32 id_to_find, int *index)
{
	CriticalSectionClass::LockClass lock(m_IDListMutex);

	bool found		= false;
	(*index)			= 0;
	int min_index	= 0;
	int max_index	= m_GlobalSoundList.Count () - 1;

	//
	//	Keep looping until we've closed the window of possiblity
	//
	bool keep_going = (max_index >= min_index);
	while (keep_going) {

		//
		//	Calculate what slot we are currently looking at
		//
		int curr_index	= min_index + ((max_index - min_index) / 2);
		uint32 curr_id	= m_GlobalSoundList[curr_index]->Get_ID ();

		//
		//	Did we find the right slot?
		//
		if (id_to_find == curr_id) {
			(*index)		= curr_index;
			keep_going	= false;
			found			= true;
		} else {

			//
			//	Stop if we've narrowed the window to one entry
			//
			keep_going = (max_index > min_index);

			//
			//	Move the window to the appropriate side
			// of the test index.
			//
			if (id_to_find < curr_id) {
				max_index	= curr_index - 1;
				(*index)		= curr_index;
			} else {
				min_index	= curr_index + 1;
				(*index)		= curr_index + 1;
			}
		}
	}

	return found;
}
