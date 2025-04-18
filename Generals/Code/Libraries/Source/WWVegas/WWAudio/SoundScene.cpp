/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /Commando/Code/WWAudio/SoundScene.cpp        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/14/01 2:55p                                               $*
 *                                                                                             *
 *                    $Revision:: 32                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "SoundScene.h"
#include "SoundCullObj.h"
#include "LogicalSound.h"
#include "LogicalListener.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "wwprofile.h"
#include "Threads.h"
#include "wwmemlog.h"


DEFINE_AUTO_POOL(SoundSceneClass::AudibleInfoClass, 64);


//////////////////////////////////////////////////////////////////////////////////
//	Generic constants
//////////////////////////////////////////////////////////////////////////////////
const int MAX_LOGICAL_LISTENER_UPDATES_PER_FRAME		= 4;


//////////////////////////////////////////////////////////////////////////////////
//	Save/Load constants
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x00000100,
	CHUNKID_STATIC_SOUNDS,
	CHUNKID_DYNAMIC_SOUNDS
};

enum
{
	VARID_MIN_DIM				= 0x01,
	VARID_MAX_DIM,
};


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SoundSceneClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundSceneClass::SoundSceneClass (void)
	:	m_Listener (NULL),
		m_2ndListener (NULL),
		m_MinExtents (-500, -500, -500),
		m_MaxExtents (500, 500, 500),
		m_IsBatchMode (false)
{	
	WWMEMLOG(MEM_SOUND);
	m_Listener = W3DNEW Listener3DClass;
	m_DynamicCullingSystem.Re_Partition (m_MinExtents, m_MaxExtents, 100.00F);	
	m_LogicalCullingSystem.Re_Partition (m_MinExtents, m_MaxExtents, 100.00F);
	m_ListenerCullingSystem.Re_Partition (m_MinExtents, m_MaxExtents, 40.00F);
	m_StaticCullingSystem.Re_Partition ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~SoundSceneClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundSceneClass::~SoundSceneClass (void)
{
	REF_PTR_RELEASE (m_Listener);
	REF_PTR_RELEASE (m_2ndListener);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Re_Partition
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Re_Partition
(
	const Vector3 &min_dimension,
	const Vector3 &max_dimension
)
{
	m_DynamicCullingSystem.Re_Partition (min_dimension, max_dimension, 100.00F);
	m_LogicalCullingSystem.Re_Partition (min_dimension, max_dimension, 100.00F);
	m_ListenerCullingSystem.Re_Partition (min_dimension, max_dimension, 40.00F);
	m_StaticCullingSystem.Re_Partition ();

	m_MinExtents = min_dimension;
	m_MaxExtents = max_dimension;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Collect_Logical_Sounds
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Collect_Logical_Sounds (int listener_count)
{
	WWPROFILE ("Collect_Logical_Sounds");

	uint32 timestamp = ::GetTickCount ();

	//
	//	Determine how many listeners to process
	//
	int count	= listener_count;
	int max		= MAX_LOGICAL_LISTENER_UPDATES_PER_FRAME;
	if ((count < 0) || (count > max)) {
		count = max;
	}

	PriorityMultiListIterator<LogicalListenerClass> priority_queue (&m_LogicalListeners);
	LogicalListenerClass *listener = NULL;
	
	//
	//	Loop over as many of the listeners as we want to process this
	// frame.
	//
	for (	int total = 0;
			total < count && priority_queue.Process_Head (&listener);
			total ++)
	{
		LogicalListenerClass::Set_Oldest_Timestamp (listener->Get_Timestamp ());
		listener->Set_Timestamp (LogicalListenerClass::Get_New_Timestamp ());
		listener->On_Frame_Update ();
	
		//
		// Collect a list of the sounds this listener can hear.
		//
		Vector3 position = listener->Get_Position ();
		m_LogicalCullingSystem.Reset_Collection ();
		m_LogicalCullingSystem.Collect_Objects (position);

		//
		//	Now loop through the list of sounds this listener can hear
		// and notify their callback.
		//
		SoundCullObjClass * cull_obj;
		for (	cull_obj = m_LogicalCullingSystem.Get_First_Collected_Object();
				cull_obj != NULL;
				cull_obj = m_LogicalCullingSystem.Get_Next_Collected_Object (cull_obj))
		{
			//
			// Get a pointer to the current 'cull-sound' object.
			//
			LogicalSoundClass *sound_obj = (LogicalSoundClass *)cull_obj->Peek_Sound_Obj ();

			//
			//	Test this sound against the scale associated with the current listener to
			// see if the listener can really "hear" the sound.
			//
			const Vector3 &sound_pos	= cull_obj->Get_Bounding_Box ().Center;
			Vector3 listener_pos			= listener->Get_Position ();
			float dropoff_radius			= sound_obj->Get_DropOff_Radius ();
			float scale						= listener->Get_Effective_Scale ();
			float test_radius2			= (dropoff_radius * scale) * (dropoff_radius * scale);
			if ((listener_pos - sound_pos).Length2 () <= test_radius2) {
				
				//
				//	Is the sound ready to notify?
				//
				if (sound_obj->Allow_Notify (timestamp)) {
					listener->On_Event (AudioCallbackClass::EVENT_LOGICAL_HEARD, (uint32)listener, (uint32)sound_obj);
				}
			}
		}
	}

	//
	//	Loop through and remove any single shot sounds that have
	// been completely processed
	//
	MultiListIterator<LogicalSoundClass> single_shot_it (&m_SingleShotLogicalSounds);
	for (single_shot_it.First (); !single_shot_it.Is_Done (); single_shot_it.Next ()) {
		LogicalSoundClass *sound_obj = single_shot_it.Peek_Obj ();

		//
		//	Remove this sound if its been completely processed
		//
		if (sound_obj->Get_Listener_Timestamp () <= LogicalListenerClass::Get_Oldest_Timestamp ()) {
			sound_obj->Remove_From_Scene ();
			single_shot_it.Remove_Current_Object ();
			single_shot_it.Prev ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Collect_Audible_Sounds
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Collect_Audible_Sounds
(
	Listener3DClass *	listener,
	COLLECTED_SOUNDS &list
)
{
	WWPROFILE ("Collect_Audible_Sounds");

	//
	// Collect a list of the audible dynamic sounds
	//
	Vector3 listener_pos = listener->Get_Position ();
	m_DynamicCullingSystem.Reset_Collection ();
	m_DynamicCullingSystem.Collect_Objects (listener_pos);

	//
	// Collect a list of the audible static sounds
	//
	m_StaticCullingSystem.Reset_Collection ();
	m_StaticCullingSystem.Collect_Objects (listener_pos);

	//
	// Loop through all the dynamic sounds that are currently audible and make sure
	// they are 'really' audible.  The culling systems just check bounding boxes
	// but we need to be able to check attenuation spheres.
	//
	SoundCullObjClass * cull_obj = NULL;	
	for (	cull_obj = m_DynamicCullingSystem.Get_First_Collected_Object();
			cull_obj != NULL;
			cull_obj = m_DynamicCullingSystem.Get_Next_Collected_Object(cull_obj))
	{
		// Get a pointer to the current 'cull-sound' object
		AudibleSoundClass *sound_obj = (AudibleSoundClass *)cull_obj->Peek_Sound_Obj ();

		// Perform a quick sphere-cull check to make sure this
		// sound should really be audible
		Vector3 pos = sound_obj->Get_Position ();
		float radius = sound_obj->Get_DropOff_Radius ();
		float radius2 = radius * radius;
		float length2 = (pos - listener_pos).Length2 ();
		if (length2 <= radius2) {
			
			AudibleInfoClass *audible_info = new AudibleInfoClass (sound_obj, length2);
			list.Add (audible_info);
			
			//
			// Update this sound's runtime priority based on its distance			
			// from the sound emitter.
			//
			float length	= (pos - listener_pos).Quick_Length ();
			float priority	= (length > 0) ? 1 - (length / radius) : 1.0F;
			sound_obj->Set_Runtime_Priority (priority);
		}
	}

	//
	// Loop through all the static sounds that are currently audible and make sure
	// they are 'really' audible.  The culling systems just check bounding boxes
	// but we need to be able to check attenuation spheres.
	//	
	for (	cull_obj = m_StaticCullingSystem.Get_First_Collected_Object();
			cull_obj != NULL;
			cull_obj = m_StaticCullingSystem.Get_Next_Collected_Object(cull_obj))
	{
		AudibleSoundClass *sound_obj = (AudibleSoundClass *)cull_obj->Peek_Sound_Obj ();
		
		// Perform a quick sphere-cull check to make sure this
		// sound should really be audible
		Vector3 pos = sound_obj->Get_Position ();
		float radius = sound_obj->Get_DropOff_Radius ();
		float radius2 = radius * radius;
		float length2 = (pos - listener_pos).Length2 ();
		if (length2 <= radius2) {
			
			AudibleInfoClass *audible_info = new AudibleInfoClass (sound_obj, length2);
			list.Add (audible_info);

			//
			// Update this sound's runtime priority based on its distance
			// from the sound emitter.
			//
			float length	= (pos - listener_pos).Quick_Length ();
			float priority	= (length > 0) ? 1 - (length / radius) : 1.0F;
			sound_obj->Set_Runtime_Priority (priority);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
//	Note:  This method could be made more efficient by using another data structure besides
// linked lists.  However the differece may be negligable due to the low density of 3D sounds
// that are audible at once.
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::On_Frame_Update (unsigned int milliseconds)
{
	WWPROFILE ("On_Frame_Update");

	COLLECTED_SOUNDS auxiliary_sounds;
	COLLECTED_SOUNDS primary_sounds;

	//
	//	First, collect any auxiliary sounds that are audible
	//
	if (m_2ndListener != NULL) {
		m_2ndListener->On_Frame_Update (milliseconds);
		Collect_Audible_Sounds (m_2ndListener, auxiliary_sounds);
	}

	//
	// Update the listener's position/velocity, etc
	//
	m_Listener->On_Frame_Update (milliseconds);
	
	//
	//	Collect the primary sounds that are audible
	//	
	Collect_Audible_Sounds (m_Listener, primary_sounds);

	//
	//	Loop through the auxiliary sounds and make sure
	//	sounds are only played once, either primary or auxiliary.
	//
	MultiListIterator<AudibleInfoClass> aux_iterator (&auxiliary_sounds);
	MultiListIterator<AudibleInfoClass> pri_iterator (&primary_sounds);
	AUDIBLE_SOUND_LIST audible_sounds;

	for (aux_iterator.First (); !aux_iterator.Is_Done (); aux_iterator.Next ()) {
		AudibleInfoClass *aux_info = aux_iterator.Peek_Obj ();
		
		//
		//	Loop through all the primary sounds and remove any
		// that are 'overpowered' by the same sound in the
		// other listener.
		//
		bool found = false;		
		for (pri_iterator.First (); !pri_iterator.Is_Done () && !found; pri_iterator.Next ()) {
			AudibleInfoClass *pri_info = pri_iterator.Peek_Obj ();

			//
			//	Is this sound in both lists?
			//
			found = (aux_info->sound_obj == pri_info->sound_obj);
			if (found) {
				if (aux_info->distance2 < pri_info->distance2) {
					delete pri_info;
					primary_sounds.Remove (pri_info);
				} else {
					delete aux_info;
					auxiliary_sounds.Remove (aux_info);
				}
			}
		}
	}

	//
	//	Add the primary audible sounds into the master list
	//		
	for (pri_iterator.First (); !pri_iterator.Is_Done (); pri_iterator.Next ()) {
		AudibleInfoClass *pri_info = pri_iterator.Peek_Obj ();
		audible_sounds.Add (pri_info->sound_obj);

		//
		//	Let the sound know what it's listener's position is
		//
		pri_info->sound_obj->Set_Listener_Transform (m_Listener->Get_Transform ());
		
		//
		//	Free the audible info object
		//
		pri_iterator.Remove_Current_Object ();	
		pri_iterator.Prev ();
		delete pri_info;
	}

	//
	//	Add the auxiliary audible sounds into the master list
	//
	for (aux_iterator.First (); !aux_iterator.Is_Done (); aux_iterator.Next ()) {
		AudibleInfoClass *aux_info = aux_iterator.Peek_Obj ();
		audible_sounds.Add (aux_info->sound_obj);

		//
		//	Let the sound know what it's listener's position is
		//
		aux_info->sound_obj->Set_Listener_Transform (m_2ndListener->Get_Transform ());
		
		//
		//	Convert the sound to a Pseudo-3D sound that has
		// a 'tinny' filter applied to it.
		//
		/*aux_info.sound_obj->Convert_To_Filtered ();
		AudibleSoundClass *tinny_sound = aux_info.sound_obj->As_Converted_Format ();
		if (tinny_sound != NULL) {
			audible_sounds.Add (tinny_sound);
		}*/
		
		//
		//	Free the audible info object
		//
		aux_iterator.Remove_Current_Object ();	
		aux_iterator.Prev ();
		delete aux_info;
	}

	//
	// Loop through all the sounds that were audible last frame
	// and see if they are still audible this frame.
	//
	MultiListIterator<AudibleSoundClass> audible_iterator (&m_LastSoundsAudible);
	for (audible_iterator.First (); !audible_iterator.Is_Done (); audible_iterator.Next ()) {
		AudibleSoundClass *sound_obj = audible_iterator.Peek_Obj ();

		//
		// Is this sound still audible?
		//
		if (audible_sounds.Is_In_List (sound_obj)) {
			
			//
			//	Make sure the sound is playing, then remove it from
			// the newly-audible list so we don't process it again
			//
			sound_obj->Cull_Sound (false);
			audible_sounds.Remove (sound_obj);
		
		} else {

			//
			// If the sound isn't audible any more then remove
			// it from the list
			//
			audible_iterator.Remove_Current_Object ();
			audible_iterator.Prev ();
			
			//
			//	Make sure we cull the sound
			//
			WWASSERT(sound_obj != NULL);
			sound_obj->Cull_Sound (true);
			sound_obj->Set_Runtime_Priority (0);
		}
	}

	//
	// Loop through all the newly-audible sounds and
	// make sure they are playing.
	//
	MultiListIterator<AudibleSoundClass> newly_audible_it (&audible_sounds);
	for (newly_audible_it.First (); !newly_audible_it.Is_Done (); newly_audible_it.Next ()) {
		AudibleSoundClass *sound_obj = newly_audible_it.Peek_Obj ();
		
		//
		//	Make sure the sound has a valid Miles handle (so it can make noise)
		//
		sound_obj->Cull_Sound (false);

		//
		// If this sound is still in the scene (it may have 'stopped'
		// while it was culled) then start playing it...
		//
		if (sound_obj->Is_In_Scene ()) {
			m_LastSoundsAudible.Add (sound_obj);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Add_Sound
(
	AudibleSoundClass *	sound_obj,
	bool						start_playing
)
{
	WWPROFILE ("Add_Sound");
	WWMEMLOG(MEM_SOUND);

	WWASSERT (sound_obj != NULL);
	if (sound_obj != NULL && sound_obj->Is_In_Scene () == false) {
		bool cull_sound = true;


		// Create a wrapper object for the sound that we can use
		// with the different culling systems.
		SoundCullObjClass *cullable_sound = W3DNEW SoundCullObjClass;		
		cullable_sound->Set_Sound_Obj (sound_obj);
		sound_obj->Set_Cullable_Wrapper (cullable_sound);

		//
		// Add this object to the dynamic culling system
		//
		m_DynamicCullingSystem.Add_Object (cullable_sound);
		m_DynamicSounds.Add (cullable_sound);
		Update_Sound (cullable_sound);

		//
		//	If the listener can hear this sound, then make sure
		// we start it off non-culled
		//
		if (m_IsBatchMode == false) {

			Vector3 listener_pos		= m_Listener->Get_Position ();
			Vector3 sound_pos			= sound_obj->Get_Position ();
			float radius				= sound_obj->Get_DropOff_Radius ();
			float radius2				= radius * radius;

			if (((listener_pos - sound_pos).Length2 ()) < radius2) {
				cull_sound = false;
				m_LastSoundsAudible.Add (sound_obj);
				start_playing = true;
				sound_obj->Set_Listener_Transform (m_Listener->Get_Transform ());
			}
		}

		//
		// Make sure the sound is appropriately culled
		//
		sound_obj->Cull_Sound (cull_sound);

		//
		//	Start the sound playing if requested
		//
		if (start_playing) {
			sound_obj->Play ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_Sound
//
//	Note:  This method should really be rewritten to use a different list type.  Linked lists
// are probably too inefficient (especially if we have 100s or 1000s of sounds)
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Remove_Sound
(
	AudibleSoundClass *sound_obj,
	bool stop_playing
)
{
	WWPROFILE ("Remove_Sound");

	if (sound_obj == NULL) {
		return ;
	}

	//
	// Make sure we remove this sound from the list of last audible sounds.
	//
	if (m_LastSoundsAudible.Is_In_List (sound_obj)) {
		m_LastSoundsAudible.Remove (sound_obj);
	}

	//
	//	Is this sound really in the scene?
	//
	SoundCullObjClass *cull_obj = sound_obj->Peek_Cullable_Wrapper ();
	if (cull_obj != NULL && m_DynamicSounds.Is_In_List (cull_obj)) {
				
		//
		//	Stop playing the sound if necessary
		//
		if (stop_playing) {
			sound_obj->Stop ();
		}

		//
		//	Flush the sound's cull-wrapper since we are removing it from the scene
		//
		sound_obj->Set_Cullable_Wrapper (NULL);

		//
		// Remove this sound from the dynamic culling system
		//
		m_DynamicCullingSystem.Remove_Object (cull_obj);
		m_DynamicSounds.Remove (cull_obj);

		//
		//	Register the sound for deletion at an appropriate time
		//
		WWAudioThreadsClass::Add_Delayed_Release_Object (cull_obj);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Static_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Add_Static_Sound
(
	AudibleSoundClass *	sound_obj,
	bool						start_playing
)
{
	WWPROFILE ("Add_Static_Sound");

	WWASSERT (sound_obj != NULL);
	if (sound_obj != NULL) {

		//
		// Check to see if this sound is already in the scene
		//
		SoundCullObjClass *cull_obj = sound_obj->Peek_Cullable_Wrapper ();
		if (cull_obj == NULL) {

			//
			// Create a wrapper object for the sound that we can use
			// with the different culling systems.
			//
			cull_obj = W3DNEW SoundCullObjClass;
			cull_obj->Set_Sound_Obj (sound_obj);
			sound_obj->Set_Cullable_Wrapper (cull_obj);

			//
			// Add this object to the static culling system
			//
			m_StaticCullingSystem.Add_Object (cull_obj);
			m_StaticSounds.Add (cull_obj);
			Update_Sound (cull_obj);

			//
			//	If the listener can hear this sound, then make sure
			// we start it off non-culled
			//
			bool cull_sound = true;
			if (m_IsBatchMode == false) {

				Vector3 listener_pos		= m_Listener->Get_Position ();
				Vector3 sound_pos			= sound_obj->Get_Position ();
				float radius				= sound_obj->Get_DropOff_Radius ();
				float radius2				= radius * radius;

				if (((listener_pos - sound_pos).Length2 ()) < radius2) {
					cull_sound = false;
					m_LastSoundsAudible.Add (sound_obj);
					start_playing = true;
					sound_obj->Set_Listener_Transform (m_Listener->Get_Transform ());
				}
			}

			//
			// Make sure the sound is appropriately culled
			//
			sound_obj->Cull_Sound (cull_sound);

			//
			//	Start the sound playing if requested
			//
			if (start_playing) {
				sound_obj->Play ();
			}

			//
			//	Add a ref to the static sound object
			//
			//sound_obj->Add_Ref ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_Static_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Remove_Static_Sound
(
	AudibleSoundClass *	sound_obj,
	bool						stop_playing
)
{
	WWPROFILE ("Remove_Static_Sound");

	if (sound_obj == NULL) {
		return ;
	}

	//
	// Make sure we remove this sound from the list of last audible sounds.
	//
	if (m_LastSoundsAudible.Is_In_List (sound_obj)) {
		m_LastSoundsAudible.Remove (sound_obj);
	}

	//
	//	Is this sound really in the scene?
	//
	SoundCullObjClass *cull_obj = sound_obj->Peek_Cullable_Wrapper ();
	if (cull_obj != NULL && m_StaticSounds.Is_In_List (cull_obj)) {
				
		//
		//	Stop playing the sound if necessary
		//
		if (stop_playing) {
			sound_obj->Stop ();
		}

		//
		//	Flush the sound's cull-wrapper since we are removing it from the scene
		//
		sound_obj->Set_Cullable_Wrapper (NULL);

		//
		// Remove this sound from the static culling system
		//
		m_StaticCullingSystem.Remove_Object (cull_obj);
		m_StaticSounds.Remove (cull_obj);

		//
		//	Register the sound for deletion at an appropriate time
		//
		WWAudioThreadsClass::Add_Delayed_Release_Object (cull_obj);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Logical_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Add_Logical_Sound
(
	LogicalSoundClass *	sound_obj,
	bool						single_shot
)
{
	WWPROFILE ("Add_Logical_Sound");

	WWASSERT (sound_obj != NULL);
	if (sound_obj != NULL) {

		//
		//	Check to make sure we don't add this sound twice
		//
		if (Is_Logical_Sound_In_Scene (sound_obj, single_shot) == false) {
			sound_obj->Set_Listener_Timestamp (LogicalListenerClass::Get_Newest_Timestamp ());
			
			//
			// Create a wrapper object for the sound that we can use
			// with the different culling systems.
			//
			SoundCullObjClass *cullable_sound = W3DNEW SoundCullObjClass;
			cullable_sound->Set_Sound_Obj (sound_obj);
			sound_obj->Set_Cullable_Wrapper (cullable_sound);

			//
			// Add this object to the logical culling system
			//
			m_LogicalCullingSystem.Add_Object (cullable_sound);
			
			//
			//	Add this sound to our current sounds list
			//
			if (single_shot) {
				m_SingleShotLogicalSounds.Add (sound_obj);
			} else {				
				m_LogicalSounds.Add (sound_obj);
			}

			//
			//	Keep a reference on this sound object
			//
			sound_obj->Add_Ref ();

			//
			//	Make sure the cull-object has the most up-to-date information
			// about this sound object's bounding volume
			//
			Update_Sound (cullable_sound);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_Logical_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Remove_Logical_Sound
(
	LogicalSoundClass *	sound_obj,
	bool						single_shot,
	bool						remove_from_list
)
{
	WWPROFILE ("Remove_Logical_Sound");

	if (sound_obj == NULL) {
		return ;
	}

	if (single_shot) {

		//
		//	Only do this if the logical sound is really in our list
		//
		if (m_SingleShotLogicalSounds.Is_In_List (sound_obj)) {

			//
			// Remove this sound from logical sound list
			//
			if (remove_from_list) {
				m_SingleShotLogicalSounds.Remove (sound_obj);
			}

			//
			// Remove this sound from the culling system
			//
			SoundCullObjClass *cull_obj = sound_obj->Peek_Cullable_Wrapper ();
			m_LogicalCullingSystem.Remove_Object (cull_obj);	

			//
			// Remove this sound obj's wrapper
			//
			sound_obj->Set_Cullable_Wrapper (NULL);
			WWAudioThreadsClass::Add_Delayed_Release_Object (cull_obj);
			
			//
			//	Release our reference on this object
			//
			REF_PTR_RELEASE (sound_obj);
		}

	} else {

		//
		//	Only do this if the logical sound is really in our list
		//
		if (m_LogicalSounds.Is_In_List (sound_obj)) {

			//
			// Remove this sound from logical sound list
			//
			if (remove_from_list) {
				m_LogicalSounds.Remove (sound_obj);
			}

			//
			// Remove this sound from the culling system
			//
			SoundCullObjClass *cull_obj = sound_obj->Peek_Cullable_Wrapper ();
			m_LogicalCullingSystem.Remove_Object (cull_obj);	

			//
			// Remove this sound obj's wrapper
			//
			sound_obj->Set_Cullable_Wrapper (NULL);
			WWAudioThreadsClass::Add_Delayed_Release_Object (cull_obj);

			//
			//	Release our reference on this object
			//
			REF_PTR_RELEASE (sound_obj);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Logical_Listener
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Add_Logical_Listener (LogicalListenerClass *listener_obj)
{
	WWPROFILE ("Add_Logical_Listener");

	WWASSERT (listener_obj != NULL);
	if (listener_obj != NULL) {

		//
		//	Add the listener to the 'scene' if its in our list
		//
		if (m_LogicalListeners.Is_In_List (listener_obj) == false) {
			listener_obj->Set_Timestamp (LogicalListenerClass::Get_New_Timestamp ());
			m_LogicalListeners.Add_Tail (listener_obj);
			listener_obj->Add_Ref ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_Logical_Listener
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Remove_Logical_Listener (LogicalListenerClass *listener_obj)
{
	WWPROFILE ("Remove_Logical_Listener");

	WWASSERT (listener_obj != NULL);
	if (listener_obj != NULL) {

		//
		//	Remove the listener from the 'scene' if its in our list
		//
		if (m_LogicalListeners.Is_In_List (listener_obj)) {
			m_LogicalListeners.Remove (listener_obj);
			listener_obj->Release_Ref ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Update_Sound (SoundCullObjClass *sound_obj)
{
	if (sound_obj != NULL) {
		sound_obj->Set_Cull_Box(sound_obj->Get_Bounding_Box());
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Initialize (void)
{
	m_Listener->Free_Miles_Handle ();
	m_Listener->Allocate_Miles_Handle ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Sound_In_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneClass::Is_Sound_In_Scene (AudibleSoundClass *sound_obj, bool all)
{
	bool retval = false;

	//
	//	Try to find this sound's cull-object in either the static or dynamic
	// lists.
	//
	SoundCullObjClass *cull_obj = sound_obj->Peek_Cullable_Wrapper ();
	if (cull_obj != NULL) {
		retval = (m_DynamicSounds.Is_In_List (cull_obj) || m_StaticSounds.Is_In_List (cull_obj));
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Logical_Sound_In_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneClass::Is_Logical_Sound_In_Scene
(
	LogicalSoundClass *	sound_obj,
	bool						single_shot
)
{	
	bool retval = false;
	
	//
	//	Check to see if this sound is in either the continuous list or the single shot list.
	//
	if (single_shot == false) {
		retval = m_LogicalSounds.Is_In_List (sound_obj);
	} else {
		retval = m_SingleShotLogicalSounds.Is_In_List (sound_obj);
	}
	
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_Static
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneClass::Save_Static (ChunkSaveClass &csave)
{	
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_MIN_DIM, m_MinExtents);
		WRITE_MICRO_CHUNK (csave, VARID_MAX_DIM, m_MaxExtents);
	csave.End_Chunk ();	
		
	//
	//	Save the list of static sounds that are currently in the scene
	//
	csave.Begin_Chunk (CHUNKID_STATIC_SOUNDS);
		Save_Static_Sounds (csave);
	csave.End_Chunk ();
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_Static_Sounds
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Save_Static_Sounds (ChunkSaveClass &csave)
{
	Re_Partition (m_MinExtents, m_MaxExtents);

	//
	//	Loop over all the static sounds that are in the scene
	// and save each to its own chunk.
	//
	MultiListIterator<SoundCullObjClass> static_iterator (&m_StaticSounds);
	for (static_iterator.First (); !static_iterator.Is_Done (); static_iterator.Next ()) {
		SoundCullObjClass *cull_obj = static_iterator.Peek_Obj ();

		//
		//	Get the sound from its cull object
		//
		AudibleSoundClass *sound_obj	= (AudibleSoundClass *)cull_obj->Peek_Sound_Obj ();
		if (sound_obj != NULL) {

			//
			//	Have the sound's factory save it
			//
			csave.Begin_Chunk (sound_obj->Get_Factory ().Chunk_ID ());
			sound_obj->Get_Factory ().Save (csave, sound_obj);
			csave.End_Chunk ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_Static_Sounds
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Load_Static_Sounds (ChunkLoadClass &cload)
{
	while (cload.Open_Chunk ()) {

		//
		//	Load this sound from the chunk (if possible)
		//
		PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
		if (factory != NULL) {
			AudibleSoundClass *sound_obj = (AudibleSoundClass *)factory->Load (cload);
			if (sound_obj != NULL) {
				sound_obj->Add_To_Scene (true);
				REF_PTR_RELEASE (sound_obj);
			}
		}

		cload.Close_Chunk ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_Static
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneClass::Load_Static (ChunkLoadClass &cload)
{
	m_IsBatchMode = true;

	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_STATIC_SOUNDS:
				Load_Static_Sounds (cload);
				break;

			case CHUNKID_VARIABLES:
			{
				//
				//	Read all the variables from their micro-chunks
				//
				while (cload.Open_Micro_Chunk ()) {
					switch (cload.Cur_Micro_Chunk_ID ()) {
						READ_MICRO_CHUNK (cload, VARID_MIN_DIM, m_MinExtents);
						READ_MICRO_CHUNK (cload, VARID_MAX_DIM, m_MaxExtents);			
					}

					cload.Close_Micro_Chunk ();
				}
			}
			break;
		}

		cload.Close_Chunk ();
	}
	
	Re_Partition (m_MinExtents, m_MaxExtents);
	On_Frame_Update (0);
	m_IsBatchMode = false;
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_Dynamic
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneClass::Save_Dynamic (ChunkSaveClass &csave)
{	
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_Dynamic
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
SoundSceneClass::Load_Dynamic (ChunkLoadClass &cload)
{
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Flush_Scene (void)
{
	RefMultiListClass<SoundCullObjClass> temp_static;
	RefMultiListClass<SoundCullObjClass> temp_dynamic;

	//
	//	Build temporary lists of the static and dynamic sounds
	//
	MultiListIterator<SoundCullObjClass> static_iterator (&m_StaticSounds);
	for (static_iterator.First (); !static_iterator.Is_Done (); static_iterator.Next ()) {
		SoundCullObjClass *cull_obj = static_iterator.Peek_Obj ();
		temp_static.Add (cull_obj);
	}

	MultiListIterator<SoundCullObjClass> dynamic_iterator (&m_DynamicSounds);
	for (dynamic_iterator.First (); !dynamic_iterator.Is_Done (); dynamic_iterator.Next ()) {
		SoundCullObjClass *cull_obj = dynamic_iterator.Peek_Obj ();
		temp_dynamic.Add (cull_obj);
	}

	//
	//	Remove all the static sounds from the scene
	//
	RefMultiListIterator<SoundCullObjClass> temp_static_it (&temp_static);
	for (temp_static_it.First (); !temp_static_it.Is_Done (); temp_static_it.Next ()) {
		SoundCullObjClass *cull_obj = temp_static_it.Peek_Obj ();
		
		AudibleSoundClass *sound_obj	= (AudibleSoundClass *)cull_obj->Peek_Sound_Obj ();
		if (sound_obj != NULL) {
			Remove_Static_Sound (sound_obj);
		}
	}

	//
	//	Remove all the dynamic sounds from the scene
	//
	RefMultiListIterator<SoundCullObjClass> temp_dynamic_it (&temp_dynamic);
	for (temp_dynamic_it.First (); !temp_dynamic_it.Is_Done (); temp_dynamic_it.Next ()) {
		SoundCullObjClass *cull_obj = temp_dynamic_it.Peek_Obj ();
		
		AudibleSoundClass *sound_obj	= (AudibleSoundClass *)cull_obj->Peek_Sound_Obj ();
		if (sound_obj != NULL) {
			Remove_Sound (sound_obj);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_2nd_Listener
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundSceneClass::Set_2nd_Listener (Listener3DClass *listener)
{
	if (m_2ndListener != NULL) {
		m_2ndListener->On_Removed_From_Scene ();
	}

	if (listener != NULL) {
		listener->On_Added_To_Scene ();
	}

	REF_PTR_SET (m_2ndListener, listener);
	return ;
}
