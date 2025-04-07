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
 *                     $Archive:: /Commando/Code/WWAudio/SoundScene.h         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/07/01 6:10p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SOUNDSCENE_H
#define __SOUNDSCENE_H

#include "aabtreecull.h"
#include "gridcull.h"
#include "Listener.h"
#include "Vector.H"
#include "PriorityVector.h"
#include "SoundCullObj.h"
#include "LogicalListener.h"
#include "multilist.h"

// Forward declarations
class RenderObjClass;
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	Typedefs
//
//////////////////////////////////////////////////////////////////////////////////
typedef TypedGridCullSystemClass<SoundCullObjClass>		DynamicSoundCullClass;
typedef TypedAABTreeCullSystemClass<SoundCullObjClass>	StaticSoundCullClass;
typedef MultiListClass<AudibleSoundClass>						AUDIBLE_SOUND_LIST;
typedef MultiListClass<SoundCullObjClass>						SOUND_LIST;
typedef MultiListClass<LogicalSoundClass>						LOGICAL_SOUND_LIST;
typedef MultiListClass<LogicalListenerClass>					LOGICAL_LISTENER_LIST;


/////////////////////////////////////////////////////////////////////////////////
//
//	SoundSceneClass
//
//	Mimics the 'SceneClass' for render objects.  Used to insert 3D sounds into
// a virtual world.  Used to efficiently cull sounds that are too far away
// from the listner to be heard.
//
class SoundSceneClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Friend classes
		//////////////////////////////////////////////////////////////////////
		friend class WWAudioClass;

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		SoundSceneClass (void);
		virtual ~SoundSceneClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Partition methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Re_Partition (const Vector3 &min_dimension, const Vector3 &max_dimension);

		//////////////////////////////////////////////////////////////////////
		//	Logical sound methods
		//////////////////////////////////////////////////////////////////////		
		virtual void			Collect_Logical_Sounds (int listener_count = -1);

		//////////////////////////////////////////////////////////////////////
		//	Listener methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Attach_Listener_To_Obj (RenderObjClass *render_obj, int bone_index = -1) { m_Listener->Attach_To_Object (render_obj, bone_index); }

		virtual void			Set_Listener_Position (const Vector3 &pos)			{ m_Listener->Set_Position (pos); }
		virtual Vector3		Get_Listener_Position (void) const						{ return m_Listener->Get_Position (); }

		virtual void			Set_Listener_Transform (const Matrix3D &transform)	{ m_Listener->Set_Transform (transform); }
		virtual Matrix3D		Get_Listener_Transform (void) const						{ return m_Listener->Get_Transform (); }

		virtual Listener3DClass *Peek_2nd_Listener (void) const						{ return m_2ndListener; }
		virtual void			Set_2nd_Listener (Listener3DClass *listener);

		//////////////////////////////////////////////////////////////////////
		//	Sound insertion
		//////////////////////////////////////////////////////////////////////		
		virtual void			Flush_Scene (void);
		virtual void			Update_Sound (SoundCullObjClass *sound_obj);

		//
		//	These methods are for inserting audible-dynamic sounds into the scene.
		//
		virtual void			Add_Sound (AudibleSoundClass *sound_obj, bool start_playing = true);
		virtual void			Remove_Sound (AudibleSoundClass *sound_obj, bool stop_playing = true);

		//
		//	Static sounds are those that will never change position in the world.
		// These sounds can be more efficiently culled.
		//
		virtual void			Add_Static_Sound (AudibleSoundClass *sound_obj, bool start_playing = true);
		virtual void			Remove_Static_Sound (AudibleSoundClass *sound_obj, bool stop_playing = true);

		//
		//	These methods are for inserting logical sounds and listeners into the scene.
		//
		virtual void			Add_Logical_Sound (LogicalSoundClass *sound_obj, bool single_shot = false);
		virtual void			Remove_Logical_Sound (LogicalSoundClass *sound_obj, bool single_shot = false, bool remove_from_list = true);

		virtual void			Add_Logical_Listener (LogicalListenerClass *listener_obj);
		virtual void			Remove_Logical_Listener (LogicalListenerClass *listener_obj);		

		//////////////////////////////////////////////////////////////////////
		//	Save/Load
		//////////////////////////////////////////////////////////////////////		
		bool						Save_Static (ChunkSaveClass &csave);
		bool						Load_Static (ChunkLoadClass &cload);
		bool						Save_Dynamic (ChunkSaveClass &csave);
		bool						Load_Dynamic (ChunkLoadClass &cload);
		
		bool						Is_Batch_Mode (void) const			{ return m_IsBatchMode; }
		void						Set_Batch_Mode (bool batch_mode)	{ m_IsBatchMode = batch_mode; }

		//////////////////////////////////////////////////////////////////////
		//	Debugging
		//////////////////////////////////////////////////////////////////////		
		bool						Is_Sound_In_Scene (AudibleSoundClass *sound_obj, bool all = true);

	protected:
		
		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		virtual void			On_Frame_Update (unsigned int milliseconds = 0);
		virtual void			Initialize (void);

		virtual bool			Is_Logical_Sound_In_Scene (LogicalSoundClass *sound_obj, bool single_shot = false);

		// Save/load methods
		virtual void			Save_Static_Sounds (ChunkSaveClass &csave);
		virtual void			Load_Static_Sounds (ChunkLoadClass &cload);

		//////////////////////////////////////////////////////////////////////
		//	Collection methods
		//////////////////////////////////////////////////////////////////////		
	public:
		class AudibleInfoClass : public MultiListObjectClass, public AutoPoolClass<AudibleInfoClass, 64>
		{
		public:
			AudibleInfoClass (void)
				:	sound_obj (NULL),
					distance2 (0) { }

			AudibleInfoClass (AudibleSoundClass *obj, float dist2)
				:	sound_obj (obj),
					distance2 (dist2) { }

			AudibleSoundClass *	sound_obj;
			float						distance2;
		};

	protected:
		typedef MultiListClass<AudibleInfoClass>	COLLECTED_SOUNDS;

		virtual void			Collect_Audible_Sounds (Listener3DClass *listener, COLLECTED_SOUNDS &list);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		Listener3DClass *				m_Listener;
		Listener3DClass *				m_2ndListener;		
		AUDIBLE_SOUND_LIST			m_LastSoundsAudible;
		SOUND_LIST						m_DynamicSounds;
		SOUND_LIST						m_StaticSounds;

		LOGICAL_SOUND_LIST			m_LogicalSounds;
		LOGICAL_SOUND_LIST			m_SingleShotLogicalSounds;
		LOGICAL_LISTENER_LIST		m_LogicalListeners;

		DynamicSoundCullClass		m_ListenerCullingSystem;
		DynamicSoundCullClass		m_LogicalCullingSystem;
		DynamicSoundCullClass		m_DynamicCullingSystem;
		StaticSoundCullClass			m_StaticCullingSystem;

		Vector3							m_MinExtents;
		Vector3							m_MaxExtents;

		bool								m_IsBatchMode;
};


#endif //__SOUNDSCENE_H
