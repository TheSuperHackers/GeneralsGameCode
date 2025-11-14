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
 *                     $Archive:: /Commando/Code/WWAudio/SoundSceneObj.h        $*
 *																														     *
 *							  $Modtime:: 8/24/01 3:51p                                               $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "WWAudio.h"
#include "bittype.h"
#include "persist.h"
#include "multilist.h"
#include "mutex.h"

/////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////
class SoundCullObjClass;
class SoundSceneClass;
class RenderObjClass;
class Vector3;
class Matrix3D;
class LogicalListenerClass;
class LogicalSoundClass;
class Sound3DClass;
class SoundPseudo3DClass;
class FilteredSoundClass;
class Listener3DClass;
class AudibleSoundClass;


/////////////////////////////////////////////////////////////////////////////////
//	Constants
/////////////////////////////////////////////////////////////////////////////////
const uint32	SOUND_OBJ_DEFAULT_ID	= 0;
const uint32	SOUND_OBJ_START_ID	= 1000000000;


/////////////////////////////////////////////////////////////////////////////////
//
//	SoundSceneObjClass
//
//	Abstract base class to defines an interface for any sound
// object that will be inserted in the culling system.
//
/////////////////////////////////////////////////////////////////////////////////
class SoundSceneObjClass : public MultiListObjectClass, public PersistClass, public RefCountClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public friends
		//////////////////////////////////////////////////////////////////////
		friend class SoundSceneClass;
		friend class WWAudioClass;
		friend class HandleMgrClass;

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		SoundSceneObjClass (void);
		SoundSceneObjClass (const SoundSceneObjClass &src);
		virtual ~SoundSceneObjClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public operators
		//////////////////////////////////////////////////////////////////////
		const SoundSceneObjClass &operator= (const SoundSceneObjClass &src);

		//////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////
		//	Conversion methods
		//////////////////////////////////////////////////////////////////////
		virtual Sound3DClass *			As_Sound3DClass (void) 			{ return NULL; }
		virtual SoundPseudo3DClass *	As_SoundPseudo3DClass (void) 	{ return NULL; }
		virtual FilteredSoundClass *	As_FilteredSoundClass (void) 	{ return NULL; }
		virtual Listener3DClass *		As_Listener3DClass (void) 		{ return NULL; }
		virtual AudibleSoundClass *	As_AudibleSoundClass(void) 	{ return NULL; }

		//////////////////////////////////////////////////////////////////////
		//	Identification methods
		//////////////////////////////////////////////////////////////////////
		virtual uint32			Get_ID (void) const	{ return m_ID; }
		virtual void			Set_ID (uint32 id);

		//////////////////////////////////////////////////////////////////////
		//	Update methods
		//////////////////////////////////////////////////////////////////////
		virtual bool			On_Frame_Update (unsigned int milliseconds);

		//////////////////////////////////////////////////////////////////////
		//	Event handling
		//////////////////////////////////////////////////////////////////////
		virtual void			On_Event (AudioCallbackClass::EVENTS event, uint32 param1 = 0, uint32 param2 = 0);
		virtual void			Register_Callback (AudioCallbackClass::EVENTS events, AudioCallbackClass *callback);

		//////////////////////////////////////////////////////////////////////
		//	Position/direction methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Set_Position (const Vector3 &position)	= 0;
		virtual Vector3		Get_Position (void) const = 0;

		virtual void			Set_Listener_Transform (const Matrix3D &tm) {};
		virtual void			Set_Transform (const Matrix3D &transform) = 0;
		virtual Matrix3D		Get_Transform (void) const = 0;

		//////////////////////////////////////////////////////////////////////
		//	Culling methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Cull_Sound (bool culled = true)	= 0;
		virtual bool			Is_Sound_Culled (void) const		= 0;

		//////////////////////////////////////////////////////////////////////
		//	User data methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Set_User_Data (RefCountClass *user_obj = NULL, uint32 user = 0)	{ REF_PTR_SET (m_UserObj, user_obj); m_UserData = user; }
		virtual uint32			Get_User_Data (void) const														{ return m_UserData; }
		virtual RefCountClass *Peek_User_Obj (void) const													{ return m_UserObj; }

		//////////////////////////////////////////////////////////////////////
		//	Attached object methods
		//////////////////////////////////////////////////////////////////////
		virtual void				Attach_To_Object (RenderObjClass *render_obj, int bone_index = -1);
		virtual void				Attach_To_Object (RenderObjClass *render_obj, const char *bone_name);
		virtual RenderObjClass *Peek_Parent_Object (void)			{ return m_AttachedObject; }
		virtual int					Get_Parent_Bone (void)				{ return m_AttachedBone; }
		virtual void				Apply_Auto_Position (void);

		//////////////////////////////////////////////////////////////////////
		//	Scene integration
		//////////////////////////////////////////////////////////////////////
		virtual void			Add_To_Scene (bool start_playing = true) = 0;
		virtual void			Remove_From_Scene (void) = 0;
		virtual bool			Is_In_Scene (void) const			{ return m_Scene != NULL; }

		//////////////////////////////////////////////////////////////////////
		//	Attenuation settings
		//////////////////////////////////////////////////////////////////////

		//
		//	This is the distance where the sound can not be heard any longer.  (its vol is 0)
		//
		virtual void			Set_DropOff_Radius (float radius = 1) = 0;
		virtual float			Get_DropOff_Radius (void) const = 0;

		//////////////////////////////////////////////////////////////////////
		//	From PersistClass
		//////////////////////////////////////////////////////////////////////
		bool						Save (ChunkSaveClass &csave);
		bool						Load (ChunkLoadClass &cload);

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Handle information
		//////////////////////////////////////////////////////////////////////
		virtual SoundCullObjClass *	Peek_Cullable_Wrapper (void) const					{ return m_PhysWrapper; }
		virtual void						Set_Cullable_Wrapper (SoundCullObjClass *obj)	{ m_PhysWrapper = obj; }

		//////////////////////////////////////////////////////////////////////
		//	Sound object managment
		//////////////////////////////////////////////////////////////////////
		static void				Register_Sound_Object (SoundSceneObjClass *sound_obj);
		static void				Unregister_Sound_Object (SoundSceneObjClass *sound_obj);
		static bool				Find_Sound_Object (uint32 id_to_find, int *index);

		//////////////////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////////////////
		SoundSceneClass *				m_Scene;
		SoundCullObjClass *			m_PhysWrapper;
		AudioCallbackClass *			m_pCallback;
		AudioCallbackClass::EVENTS	m_RegisteredEvents;
		uint32							m_ID;

		RenderObjClass *				m_AttachedObject;
		int								m_AttachedBone;
		uint32							m_UserData;
		RefCountClass *				m_UserObj;

		static DynamicVectorClass<SoundSceneObjClass *>	m_GlobalSoundList;
		static uint32												m_NextAvailableID;
		static CriticalSectionClass							m_IDListMutex;
};


//////////////////////////////////////////////////////////////////////////////
//
//	On_Event
//
//////////////////////////////////////////////////////////////////////////////
__inline void
SoundSceneObjClass::On_Event
(
	AudioCallbackClass::EVENTS	event,
	uint32							param1,
	uint32							param2
)
{
	if ((m_pCallback != NULL) && (m_RegisteredEvents & event)) {

		switch (event)
		{
			case AudioCallbackClass::EVENT_SOUND_STARTED:
				m_pCallback->On_Sound_Started (this);
				break;

			case AudioCallbackClass::EVENT_SOUND_ENDED:
				m_pCallback->On_Sound_Ended (this);
				break;

			case AudioCallbackClass::EVENT_LOGICAL_HEARD:
				m_pCallback->On_Logical_Heard ((LogicalListenerClass *)param1, (LogicalSoundClass *)param2);
				break;
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Register_Callback
//
//////////////////////////////////////////////////////////////////////////////
__inline void
SoundSceneObjClass::Register_Callback
(
	AudioCallbackClass::EVENTS	events,
	AudioCallbackClass *			callback
)
{
	m_RegisteredEvents = events;
	m_pCallback = callback;
	return ;
}
