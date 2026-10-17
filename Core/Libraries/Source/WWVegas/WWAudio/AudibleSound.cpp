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
 *                     $Archive:: /Commando/Code/WWAudio/AudibleSound.cpp                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/27/01 7:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "AudibleSound.h"
#include "WWAudio.h"
#include "WWDebug/wwdebug.h"
#include "SoundBuffer.h"
#include "Utils.h"
#include "SoundScene.h"
#include "FilteredSound.h"
#include "Threads.h"
#include "LogicalSound.h"
#include "soundstreamhandle.h"
#include "sound2dhandle.h"


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudibleSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass::AudibleSoundClass ()
	:	m_Priority (0.5F),
		m_RuntimePriority (0),
		m_SoundHandle (nullptr),
		m_Length (0),
		m_CurrentPosition (0),
		m_Timestamp (0),
		m_State (STATE_STOPPED),
		m_Buffer (nullptr),
		m_Volume (1.0F),
		m_Pan (0.5F),
		m_LoopCount (1),
		m_LoopsLeft (0),
		m_Type (TYPE_SOUND_EFFECT),
		m_bDirty (true),
		m_DropOffRadius (1),
		m_IsCulled (true),
		m_pConvertedFormat (nullptr),
		m_PrevTransform (1),
		m_Transform (1),
		m_ListenerTransform (1),
		m_LogicalSound (nullptr),
		m_StartOffset (0),
		m_PitchFactor (1.0F)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudibleSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass::AudibleSoundClass (const AudibleSoundClass &src)
	:	m_Priority (0.5F),
		m_RuntimePriority (0),
		m_SoundHandle (nullptr),
		m_Length (0),
		m_CurrentPosition (0),
		m_Timestamp (0),
		m_State (STATE_STOPPED),
		m_Buffer (nullptr),
		m_Volume (1.0F),
		m_Pan (0.5F),
		m_LoopCount (1),
		m_LoopsLeft (0),
		m_Type (TYPE_SOUND_EFFECT),
		m_bDirty (true),
		m_DropOffRadius (1),
		m_IsCulled (true),
		m_pConvertedFormat (nullptr),
		m_PrevTransform (1),
		m_Transform (1),
		m_LogicalSound (nullptr),
		m_StartOffset (0),
		m_PitchFactor (1.0F)
{
	(*this) = src;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~AudibleSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass::~AudibleSoundClass ()
{
	m_State = STATE_STOPPED;
	Free_Conversion ();
	REF_PTR_RELEASE (m_LogicalSound);

	//
	//	Delay the release of the buffer (fixes a sync bug
	// with Miles internals).
	//
	if (m_Buffer != nullptr) {
		WWAudioThreadsClass::Add_Delayed_Release_Object (m_Buffer);
		m_Buffer = nullptr;
	}

	Free_Miles_Handle ();
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////////////////////////////////////
const AudibleSoundClass &
AudibleSoundClass::operator= (const AudibleSoundClass &src)
{
	m_Timestamp			= src.m_Timestamp;
	m_Type				= src.m_Type;
	m_LoopCount			= src.m_LoopCount;
	m_LoopsLeft			= src.m_LoopsLeft;
	m_Length				= src.m_Length;
	m_CurrentPosition	= src.m_CurrentPosition;
	m_bDirty				= src.m_bDirty;
	m_DropOffRadius	= src.m_DropOffRadius;
	m_PrevTransform	= src.m_PrevTransform;

	m_State = STATE_STOPPED;
	Set_Buffer (src.m_Buffer);
	m_State				= src.m_State;

	Cull_Sound (src.m_IsCulled);
	Set_Volume (src.m_Volume);
	Set_Pan (src.m_Pan);
	Set_Priority (src.m_Priority);
	Set_Transform (src.m_Transform);
	Set_Listener_Transform (src.m_ListenerTransform);
	return (*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Buffer (SoundBufferClass *buffer)
{
	//
	//	Delay the release of the buffer (fixes a sync bug
	// with Miles internals).
	//
	if (m_Buffer != nullptr) {
		WWAudioThreadsClass::Add_Delayed_Release_Object (m_Buffer);
		m_Buffer = nullptr;
	}
	REF_PTR_SET (m_Buffer, buffer);

	// Stop playing if necessary
	bool resume = false;
	if (m_State == STATE_PLAYING) {
		resume = Stop (false);
	}

	// Get the time (in ms) that this buffer will play for...
	if (m_Buffer != nullptr) {
		m_Length = m_Buffer->Get_Duration ();
	}

	// Reinitialize the handle with this new data
	Initialize_Miles_Handle ();

	// Resume playing if necessary
	if (resume) {
		Play ();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
AudibleSoundClass::Get_Buffer () const
{
	if (m_Buffer) {
		m_Buffer->Add_Ref ();
	}

	return m_Buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Peek_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
AudibleSoundClass::Peek_Buffer () const
{
	return m_Buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Play
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundClass::Play (bool alloc_handle)
{
	MMSLockClass lock;

	// If we don't have a valid handle already, try to get one from miles
	if (alloc_handle && (m_pConvertedFormat == nullptr)) {
		Allocate_Miles_Handle ();
	}

	// Let the audio system know this sound is playing
	if (m_State != STATE_PLAYING) {
		WWAudioClass::Get_Instance ()->Add_To_Playlist (this);
		m_State				= STATE_PLAYING;
		m_Timestamp			= ::GetTickCount ();
		m_LoopsLeft			= m_LoopCount;

		// If we have a valid handle, then start playing the sample
		if (m_SoundHandle != nullptr) {
			m_SoundHandle->Start_Sample ();
		}

		m_CurrentPosition	= m_StartOffset * m_Length;
		if (m_CurrentPosition > 0) {
			Seek (m_CurrentPosition);
		}


		// Fire an event
		On_Event (AudioCallbackClass::EVENT_SOUND_STARTED);

		//
		//	Add this logical sound to the scene
		//
		if (m_LogicalSound != nullptr) {
			m_LogicalSound->Set_User_Data (m_UserObj, m_UserData);
			m_LogicalSound->Set_Transform (m_Transform);
			m_LogicalSound->Add_To_Scene ();
		}
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Pause
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundClass::Pause ()
{
	MMSLockClass lock;

	// Assume failure
	bool retval = false;

	if (m_State == STATE_PLAYING) {

		// Pass the pause request onto miles
		if (m_SoundHandle != nullptr) {
			m_SoundHandle->Stop_Sample ();
		}

		// Remember our new state
		m_State = STATE_PAUSED;
		retval = true;
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Resume
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundClass::Resume ()
{
	MMSLockClass lock;

	// Assume failure
	bool retval = false;

	if (m_State == STATE_PAUSED) {

		// Pass the resume request onto miles
		if (m_SoundHandle != nullptr) {
			m_SoundHandle->Resume_Sample ();
		}

		// Remember our new state
		m_State = STATE_PLAYING;
		retval = true;
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Stop
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundClass::Stop (bool remove_from_playlist)
{
	MMSLockClass lock;

	// Assume failure
	bool retval = false;

	if ((m_State == STATE_PAUSED) ||
		 (m_State == STATE_PLAYING)) {

		// Actually stop the sample from playing
		if (m_SoundHandle != nullptr) {
			m_SoundHandle->Stop_Sample ();
		}

		// Free up the handle we have been using
		Free_Miles_Handle ();
		m_State = STATE_STOPPED;
		retval = true;

		// Reset some of the playing attributes
		m_Timestamp = 0;
		m_CurrentPosition = 0;

		if (remove_from_playlist) {
			WWAudioClass::Get_Instance ()->Remove_From_Playlist (this);
		}

		//
		//	Stop the logical portion of the sound
		//
		if (m_LogicalSound != nullptr && m_LogicalSound->Is_Single_Shot () == false) {
			m_LogicalSound->Remove_From_Scene ();
		}
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Seek
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Seek (unsigned long milliseconds)
{
	MMSLockClass lock;

	if ((milliseconds >= 0) && (milliseconds < m_Length)) {

		// Record our new position and recalculate the 'starting' timestamp
		// from this information
		m_CurrentPosition = milliseconds;
		if (m_State == STATE_PLAYING) {
			m_Timestamp = ::GetTickCount () - m_CurrentPosition;
		}

		// Update the actual sound data if we are playing the sound
		if (m_SoundHandle != nullptr) {
			m_SoundHandle->Set_Sample_MS_Position (m_CurrentPosition);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Miles_Handle (MILES_HANDLE handle)
{
	//
	// Start fresh
	//
	Free_Miles_Handle ();

	//
	//	Is our data valid?
	//
	if (handle != INVALID_MILES_HANDLE && m_Buffer != nullptr) {

		//
		//	Determine which type of sound handle to create, streaming or standard 2D
		//
		if (m_Buffer->Is_Streaming ()) {
			m_SoundHandle = W3DNEW SoundStreamHandleClass;
		} else {
			m_SoundHandle = W3DNEW Sound2DHandleClass;
		}

		//
		//	Configure the sound handle
		//
		m_SoundHandle->Set_Miles_Handle (handle);

		//
		//	Use this new handle
		//
		Initialize_Miles_Handle ();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Initialize_Miles_Handle ()
{
	MMSLockClass lock;

	// If this sound is already playing, then update its
	// playing position to make sure we really should
	// be playing it... (it will free the miles handle if not)
	if (m_State == STATE_PLAYING) {
		Update_Play_Position ();
	}

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != nullptr) {

		//
		//	Initialize the handle
		//
		m_SoundHandle->Initialize (m_Buffer);

		//
		// Record the total length of the sample in milliseconds...
		//
		m_SoundHandle->Get_Sample_MS_Position ((S32 *)&m_Length, nullptr);

		//
		// Pass our cached settings onto miles
		//
		m_SoundHandle->Set_Sample_Volume (0);
		m_SoundHandle->Set_Sample_Pan (int(m_Pan * 127.0F));
		m_SoundHandle->Set_Sample_Loop_Count (m_LoopCount);

		//
		//	Apply the pitch factor (if necessary)
		//
		if (m_PitchFactor != 1.0F) {
			Set_Pitch_Factor (m_PitchFactor);
		}

		// If this sound is already playing (and just now got a handle)
		// then make sure we start it.
		if (m_State == STATE_PLAYING) {
			m_SoundHandle->Start_Sample ();

			// Update the loop count based on the number of loops left
			m_SoundHandle->Set_Sample_Loop_Count (m_LoopsLeft);
		}

		// Seek to the position of the sound where we last left off.
		// For example, this sound could have gotten bumped due to a low priority,
		// but is now back and ready to resume at the position it would have been
		// at if it was never bumped.
		Seek (m_CurrentPosition);

		//
		// Pass the 'real' volume onto miles
		//
		float real_volume = Determine_Real_Volume ();
		m_SoundHandle->Set_Sample_Volume (int(real_volume * 127.0F));

		//
		// Associate this object instance with the handle
		//
		m_SoundHandle->Set_Sample_User_Data (INFO_OBJECT_PTR, (void *)this);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Free_Miles_Handle ()
{
	MMSLockClass lock;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != nullptr) {

		//
		// Release our hold on this handle
		//
		m_SoundHandle->Set_Sample_User_Data (INFO_OBJECT_PTR, nullptr);
		m_SoundHandle->End_Sample ();

		//
		// Remove the association between file handle and AudibleSoundClass object
		//
		//m_SoundHandle->Set_Sample_User_Data (INFO_OBJECT_PTR, nullptr);

		//
		//	Free the sound handle object
		//
		delete m_SoundHandle;
		m_SoundHandle = nullptr;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Pan
//
////////////////////////////////////////////////////////////////////////////////////////////////
float
AudibleSoundClass::Get_Pan ()
{
	MMSLockClass lock;

	//
	// Do we have a valid sample handle from miles?
	//
	if (m_SoundHandle != nullptr) {
		m_Pan = ((float)m_SoundHandle->Get_Sample_Pan ()) / 127.0F;
	}

	return m_Pan;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Pan
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Pan (float pan)
{
	MMSLockClass lock;

	//
	// Cache the normalized pan value
	//
	m_Pan = min (pan, 1.0F);
	m_Pan = max (m_Pan, 0.0F);

	//
	// Do we have a valid sample handle from miles?
	//
	if (m_SoundHandle != nullptr) {
		m_SoundHandle->Set_Sample_Pan (int(m_Pan * 127.0F));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Pitch_Factor
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Pitch_Factor (float factor)
{
	MMSLockClass lock;

	m_PitchFactor = factor;

	//
	// Do we have a valid sample handle from miles?
	//
	if (m_SoundHandle != nullptr) {

		if (m_Buffer != nullptr) {

			//
			//	Get the base rate of the sound and scale our playback rate
			// based on the factor
			//
			int base_rate	= m_Buffer->Get_Rate ();
			int new_rate	= base_rate * m_PitchFactor;
			m_SoundHandle->Set_Sample_Playback_Rate (new_rate);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Playback_Rate
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
AudibleSoundClass::Get_Playback_Rate ()
{
	MMSLockClass lock;
	int retval = 0;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != nullptr) {
		retval = m_SoundHandle->Get_Sample_Playback_Rate ();
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Playback_Rate
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Playback_Rate (int rate_in_hz)
{
	MMSLockClass lock;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != nullptr) {
		m_SoundHandle->Set_Sample_Playback_Rate (rate_in_hz);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
float
AudibleSoundClass::Get_Volume ()
{
	MMSLockClass lock;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != nullptr) {
		m_Volume = ((float)m_SoundHandle->Get_Sample_Volume ()) / 127.0F;
	}

	// Return the current pan value
	return m_Volume;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Volume (float volume)
{
	MMSLockClass lock;

	// Cache the normalized volume value
	m_Volume = min (volume, 1.0F);
	m_Volume = max (m_Volume, 0.0F);

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != nullptr) {

		// Calculate the 'real' volume to set based on the global volume and the sound
		// effect volume.
		float real_volume = Determine_Real_Volume ();
		m_SoundHandle->Set_Sample_Volume (int(real_volume * 127.0F));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Loops_Left
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
AudibleSoundClass::Get_Loops_Left () const
{
	return m_LoopsLeft;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Loop_Count
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Loop_Count (int count)
{
	MMSLockClass lock;

	// Cache the loop count
	m_LoopCount = count;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != nullptr) {
		m_SoundHandle->Set_Sample_Loop_Count (m_LoopCount);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Priority
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Priority (float priority)
{
	MMSLockClass lock;

	// Cache the normalized priority
	m_Priority = min (priority, 1.0F);
	m_Priority = max (m_Priority, 0.0F);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundClass::On_Frame_Update (unsigned int milliseconds)
{
	//
	// Do we need to track this sound's play-progress?
	//
	if ((m_LoopCount != INFINITE_LOOPS) &&
		 (m_State == STATE_PLAYING) &&
		 (m_Length > 0))
	{
		Update_Play_Position ();
	}

	if (m_pConvertedFormat != nullptr) {
		m_pConvertedFormat->Re_Sync (*this);
	}

	//
	//	Move the logical sound with the audible one...
	//
	if (m_LogicalSound != nullptr) {
		m_LogicalSound->Set_Transform (m_Transform);
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Play_Position
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Update_Play_Position ()
{
	// Determine the current offset from the beginning of the sound buffer.
	unsigned long play_time = ::GetTickCount () - m_Timestamp;
	m_CurrentPosition = play_time;

	// Have we gone past the end of a sounds play-time?
	if ((m_CurrentPosition > m_Length) && (m_Length > 0)) {

		// Normalize our position and timestamp information
		m_CurrentPosition = m_CurrentPosition % m_Length;
		m_Timestamp = ::GetTickCount () - m_CurrentPosition;

		// Decrement our count of remaining loops (if necessary)
		if (m_LoopCount != INFINITE_LOOPS) {
			m_LoopsLeft -= (play_time / m_Length);
		}

		// Trigger the 'end loop' event
		On_Loop_End ();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Allocate_Miles_Handle ()
{
	//
	// If we need to, get a play-handle from the audio system
	//
	if (m_SoundHandle == nullptr) {
		Set_Miles_Handle ((MILES_HANDLE)WWAudioClass::Get_Instance ()->Get_2D_Sample (*this));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Loop_End
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::On_Loop_End ()
{
	// Determine if the sound is actually finished or still looping
	if ((m_LoopCount != INFINITE_LOOPS) && (m_LoopsLeft < 1)) {

		// Let the audio system know that we are done with this sound
		Stop ();
		if (m_Scene != nullptr) {
			Remove_From_Scene ();
		}

		// Fire an event
		On_Event (AudioCallbackClass::EVENT_SOUND_ENDED);

	} else {
		Restart_Loop ();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Determine_Real_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
float
AudibleSoundClass::Determine_Real_Volume () const
{
	float volume = m_Volume;

	// Is this a piece of music or is it a sound effect?
	if (m_Type == TYPE_MUSIC) {
		volume = volume * WWAudioClass::Get_Instance ()->Get_Music_Volume ();
	} else if (m_Type == TYPE_SOUND_EFFECT) {
		volume = volume * WWAudioClass::Get_Instance ()->Get_Sound_Effects_Volume ();
	}

	// Return the 'real' volume
	return volume;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Filename
//
////////////////////////////////////////////////////////////////////////////////////////////////
LPCTSTR
AudibleSoundClass::Get_Filename () const
{
	LPCTSTR filename = nullptr;
	if (m_Buffer != nullptr) {
		filename = m_Buffer->Get_Filename ();
	}

	return filename;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Cull_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Cull_Sound (bool culled)
{
	// Is our state changing?
	if (m_IsCulled != culled) {
		m_IsCulled = culled;

		//
		// If this sound is culled, then throw away its play-handle.
		// Otherwise, make sure we have a valid handle.
		//
		//	Note: We also free the handle if a converted form
		// of the sound is currently playing.
		//
		if (m_IsCulled || (m_pConvertedFormat != nullptr)) {
			Free_Miles_Handle ();
		} else {
			Allocate_Miles_Handle ();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Transform (const Matrix3D &transform)
{
	// Update our internal transform
	m_PrevTransform	= m_Transform;
	m_Transform			= transform;
	Set_Dirty ();
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Position
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_Position (const Vector3 &position)
{
	// Update our internal transform
	m_Transform.Set_Translation (position);
	Set_Dirty ();
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Add_To_Scene (bool start_playing)
{
	SoundSceneClass *scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
	if ((scene != nullptr) && (m_Scene == nullptr)) {

		//
		//	Add this sound to the static culling system
		//
		m_Scene = scene;
		scene->Add_Static_Sound (this, start_playing);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Remove_From_Scene ()
{
	if (m_Scene != nullptr) {

		//
		//	Remove this sound from the  static culling system
		//
		m_Scene->Remove_Static_Sound (this);
		m_Scene = nullptr;
		m_PhysWrapper = nullptr;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_DropOff_Radius
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Set_DropOff_Radius (float radius)
{
	m_DropOffRadius = radius;
	Set_Dirty ();
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Re_Sync
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Re_Sync (AudibleSoundClass &src)
{
	m_Timestamp			= src.m_Timestamp;
	m_State				= src.m_State;
	m_Type				= src.m_Type;
	m_LoopCount			= src.m_LoopCount;
	m_LoopsLeft			= src.m_LoopsLeft;
	m_Length				= src.m_Length;
	m_CurrentPosition	= src.m_CurrentPosition;
	m_bDirty				= src.m_bDirty;
	m_DropOffRadius	= src.m_DropOffRadius;
	m_PrevTransform	= src.m_PrevTransform;

	Cull_Sound (src.m_IsCulled);
	Set_Volume (src.m_Volume);
	Set_Pan (src.m_Pan);
	Set_Priority (src.m_Priority);
	Set_Transform (src.m_Transform);
	Set_Listener_Transform (src.m_ListenerTransform);

	if (m_State != STATE_PLAYING) {
		Free_Miles_Handle ();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Conversion
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Free_Conversion ()
{
	if (m_pConvertedFormat != nullptr) {
		m_pConvertedFormat->Stop ();
		REF_PTR_RELEASE (m_pConvertedFormat);
	}

	//
	//	Reacquire a play-handle if necessary
	//
	if ((m_IsCulled == false) && (m_State == STATE_PLAYING)) {
		Allocate_Miles_Handle ();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Convert_To_Filtered
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Convert_To_Filtered ()
{
	if (m_pConvertedFormat == nullptr) {

		//
		//	Make a copy of the sound in its new format
		//
		m_pConvertedFormat = W3DNEW FilteredSoundClass;
		switch (Get_Class_ID ())
		{
			case CLASSID_3D:
				(*m_pConvertedFormat) = (const Sound3DClass &)(*this);
				break;

			case CLASSID_PSEUDO3D:
				(*m_pConvertedFormat) = (const SoundPseudo3DClass &)(*this);
				break;

			case CLASSID_FILTERED:
				(*m_pConvertedFormat) = (const FilteredSoundClass &)(*this);
				break;

			default:
				(*m_pConvertedFormat) = (*this);
				break;
		}

		Free_Miles_Handle ();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	As_Converted_Format
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
AudibleSoundClass::As_Converted_Format ()
{
	if (m_pConvertedFormat == nullptr) {
		Convert_To_Filtered ();
	}

	return m_pConvertedFormat;
}
