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
#include "ww3d.h"
#include "wwdebug.h"
#include "SoundBuffer.h"
#include "Utils.h"
#include "SoundScene.h"
#include "FilteredSound.h"
#include "Threads.h"
#include "SoundChunkIDs.h"
#include "simpledefinitionfactory.h"
#include "persistfactory.h"
#include "LogicalSound.h"
#include "definitionclassids.h"
#include "soundstreamhandle.h"
#include "sound2dhandle.h"


//////////////////////////////////////////////////////////////////////////////////
//	Static factories
//////////////////////////////////////////////////////////////////////////////////
DECLARE_DEFINITION_FACTORY(AudibleSoundDefinitionClass, CLASSID_SOUND_DEF, "Sound")	_SoundDefFactory;
SimplePersistFactoryClass<AudibleSoundDefinitionClass, CHUNKID_SOUND_DEF>				_AudibleSoundDefPersistFactory;
SimplePersistFactoryClass<AudibleSoundClass, CHUNKID_AUDIBLE_SOUND>						_AudibleSoundPersistFactory;


//////////////////////////////////////////////////////////////////////////////////
//	Save/Load constants
//////////////////////////////////////////////////////////////////////////////////

namespace AUDIBLE_SOUND_SAVELOAD
{
	enum
	{
		CHUNKID_VARIABLES			= 0x00000100,
		CHUNKID_BASE_CLASS
	};

	enum
	{
		VARID_STATE					= 0x01,
		VARID_TYPE,

		VARID_PRIORITY,
		VARID_VOLUME,
		VARID_PAN,
		VARID_LOOP_COUNT,
		VARID_LOOPS_LEFT,
		VARID_SOUND_LENGTH,
		VARID_CURR_POS,
		VARID_TRANSFORM,
		VARID_PREV_TRANSFORM,
		VARID_IS_CULLED,
		VARID_IS_DIRTY,
		VARID_DROP_OFF,
		VARID_FILENAME,
		VARID_THIS_PTR,
		VARID_START_OFFSET,
		VARID_LISTENER_TRANSFORM,
		VARID_PITCH_FACTOR
	};
}

namespace AUDIBLE_SOUND_DEF_SAVELOAD
{
	enum
	{
		CHUNKID_VARIABLES			= 0x00000100,
		CHUNKID_BASE_CLASS		= 0x00000200,
	};

	enum
	{
		VARID_UNUSED1				= 0x01,
		VARID_UNUSED2,
		VARID_PRIORITY,
		VARID_VOLUME,
		VARID_PAN,
		VARID_LOOP_COUNT,
		VARID_DROP_OFF,
		VARID_MAX_VOL,
		VARID_TYPE,
		VARID_IS3D,
		VARID_FILENAME,
		VARID_DISPLAY_TEXT,
		VARID_LOGICAL_MASK,
		VARID_LOGICAL_DELAY,
		VARID_CREATE_LOGICAL,
		VARID_LOGICAL_DROP_OFF,
		VARID_SPHERE_COLOR,
		VARID_START_OFFSET,
		VARID_PITCH_FACTOR
	};
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudibleSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass::AudibleSoundClass (void)
	:	m_Priority (0.5F),
		m_RuntimePriority (0),
		m_SoundHandle (NULL),
		m_Length (0),
		m_CurrentPosition (0),
		m_Timestamp (0),
		m_State (STATE_STOPPED),
		m_Buffer (NULL),
		m_Volume (1.0F),
		m_Pan (0.5F),
		m_LoopCount (1),
		m_LoopsLeft (0),
		m_Type (TYPE_SOUND_EFFECT),
		m_bDirty (true),
		m_DropOffRadius (1),
		m_IsCulled (true),
		m_pConvertedFormat (NULL),
		m_PrevTransform (1),
		m_Transform (1),
		m_ListenerTransform (1),
		m_Definition (NULL),
		m_LogicalSound (NULL),
		m_StartOffset (0),
		m_PitchFactor (1.0F)
{	
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudibleSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass::AudibleSoundClass (const AudibleSoundClass &src)
	:	m_Priority (0.5F),
		m_RuntimePriority (0),
		m_SoundHandle (NULL),
		m_Length (0),
		m_CurrentPosition (0),
		m_Timestamp (0),
		m_State (STATE_STOPPED),
		m_Buffer (NULL),
		m_Volume (1.0F),
		m_Pan (0.5F),
		m_LoopCount (1),
		m_LoopsLeft (0),
		m_Type (TYPE_SOUND_EFFECT),
		m_bDirty (true),
		m_DropOffRadius (1),
		m_IsCulled (true),
		m_pConvertedFormat (NULL),
		m_PrevTransform (1),
		m_Transform (1),
		m_Definition (NULL),
		m_LogicalSound (NULL),
		m_StartOffset (0),
		m_PitchFactor (1.0F)
{
	(*this) = src;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~AudibleSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass::~AudibleSoundClass (void)
{
	m_State = STATE_STOPPED;
	Free_Conversion ();
	REF_PTR_RELEASE (m_LogicalSound);

	//
	//	Delay the release of the buffer (fixes a sync bug
	// with Miles internals).
	//
	if (m_Buffer != NULL) {
		WWAudioThreadsClass::Add_Delayed_Release_Object (m_Buffer);
		m_Buffer = NULL;
	}

	Free_Miles_Handle ();
	return ;
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
	if (m_Buffer != NULL) {
		WWAudioThreadsClass::Add_Delayed_Release_Object (m_Buffer);
		m_Buffer = NULL;
	}
	REF_PTR_SET (m_Buffer, buffer);
	
	// Stop playing if necessary
	bool resume = false;
	if (m_State == STATE_PLAYING) {
		resume = Stop (false);
	}

	// Get the time (in ms) that this buffer will play for...
	if (m_Buffer != NULL) {
		m_Length = m_Buffer->Get_Duration ();
	}

	// Reinitialize the handle with this new data
	Initialize_Miles_Handle ();

	// Resume playing if necessary
	if (resume) {
		Play ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
AudibleSoundClass::Get_Buffer (void) const
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
AudibleSoundClass::Peek_Buffer (void) const
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
	if (alloc_handle && (m_pConvertedFormat == NULL)) {
		Allocate_Miles_Handle ();
	}

	// Let the audio system know this sound is playing
	if (m_State != STATE_PLAYING) {
		WWAudioClass::Get_Instance ()->Add_To_Playlist (this);
		m_State				= STATE_PLAYING;
		m_Timestamp			= ::GetTickCount ();
		m_LoopsLeft			= m_LoopCount;		

		// If we have a valid handle, then start playing the sample
		if (m_SoundHandle != NULL) {
			m_SoundHandle->Start_Sample ();
		}

		m_CurrentPosition	= m_StartOffset * m_Length;
		if (m_CurrentPosition > 0) {
			Seek (m_CurrentPosition);
		}


		// Fire an event
		On_Event (AudioCallbackClass::EVENT_SOUND_STARTED);

		//
		//	Create the associate logical sound (if necessary)
		//
		if (m_LogicalSound == NULL && m_Definition != NULL) {
			m_LogicalSound = m_Definition->Create_Logical ();
		}
		
		//
		//	Add this logical sound to the scene
		//
		if (m_LogicalSound != NULL) {
			m_LogicalSound->Set_User_Data (m_UserObj, m_UserData);
			m_LogicalSound->Set_Transform (m_Transform);
			m_LogicalSound->Add_To_Scene ();
		}

		//
		//	Should we send off the text notification?
		//
		if (m_IsCulled == false && m_Definition != NULL) {
			const StringClass &text = m_Definition->Get_Display_Text ();
			WWAudioClass::Get_Instance ()->Fire_Text_Callback (this, text);
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
AudibleSoundClass::Pause (void)
{
	MMSLockClass lock;

	// Assume failure
	bool retval = false;

	if (m_State == STATE_PLAYING) {

		// Pass the pause request onto miles
		if (m_SoundHandle != NULL) {
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
AudibleSoundClass::Resume (void)
{
	MMSLockClass lock;

	// Assume failure
	bool retval = false;

	if (m_State == STATE_PAUSED) {

		// Pass the resume request onto miles
		if (m_SoundHandle != NULL) {
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
		if (m_SoundHandle != NULL) {
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
		if (m_LogicalSound != NULL && m_LogicalSound->Is_Single_Shot () == false) {
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
		if (m_SoundHandle != NULL) {
			m_SoundHandle->Set_Sample_MS_Position (m_CurrentPosition);
		}
	}

	return ;
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
	if (handle != INVALID_MILES_HANDLE && m_Buffer != NULL) {
		
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

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Initialize_Miles_Handle (void)
{
	MMSLockClass lock;

	// If this sound is already playing, then update its
	// playing position to make sure we really should
	// be playing it... (it will free the miles handle if not)
	if (m_State == STATE_PLAYING) {
		Update_Play_Position ();
	}

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != NULL) {
		
		//
		//	Initialize the handle
		//
		m_SoundHandle->Initialize (m_Buffer);

		//
		// Record the total length of the sample in milliseconds...
		//
		m_SoundHandle->Get_Sample_MS_Position ((S32 *)&m_Length, NULL);

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
		m_SoundHandle->Set_Sample_User_Data (INFO_OBJECT_PTR, (S32)this);
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Free_Miles_Handle (void)
{
	MMSLockClass lock;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != NULL) {

		//
		// Release our hold on this handle
		//
		m_SoundHandle->Set_Sample_User_Data (INFO_OBJECT_PTR, NULL);
		m_SoundHandle->End_Sample ();
		
		//
		// Remove the association between file handle and AudibleSoundClass object
		//
		//m_SoundHandle->Set_Sample_User_Data (INFO_OBJECT_PTR, NULL);
		
		//
		//	Free the sound handle object
		//
		delete m_SoundHandle;
		m_SoundHandle = NULL;
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Pan
//
////////////////////////////////////////////////////////////////////////////////////////////////
float
AudibleSoundClass::Get_Pan (void)
{
	MMSLockClass lock;

	//
	// Do we have a valid sample handle from miles?
	//
	if (m_SoundHandle != NULL) {
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
	if (m_SoundHandle != NULL) {
		m_SoundHandle->Set_Sample_Pan (int(m_Pan * 127.0F));
	}
		
	return ;
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
	if (m_SoundHandle != NULL) {		
		
		if (m_Buffer != NULL) {
			
			//
			//	Get the base rate of the sound and scale our playback rate
			// based on the factor
			//
			int base_rate	= m_Buffer->Get_Rate ();
			int new_rate	= base_rate * m_PitchFactor;
			m_SoundHandle->Set_Sample_Playback_Rate (new_rate);
		}
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Playback_Rate
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
AudibleSoundClass::Get_Playback_Rate (void)
{
	MMSLockClass lock;
	int retval = 0;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != NULL) {
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
	if (m_SoundHandle != NULL) {
		m_SoundHandle->Set_Sample_Playback_Rate (rate_in_hz);
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
float
AudibleSoundClass::Get_Volume (void)
{
	MMSLockClass lock;

	// Do we have a valid sample handle from miles?
	if (m_SoundHandle != NULL) {
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
	if (m_SoundHandle != NULL) {

		// Calculate the 'real' volume to set based on the global volume and the sound
		// effect volume.
		float real_volume = Determine_Real_Volume ();
		m_SoundHandle->Set_Sample_Volume (int(real_volume * 127.0F));
	}
		
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Loops_Left
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
AudibleSoundClass::Get_Loops_Left (void) const
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
	if (m_SoundHandle != NULL) {
		m_SoundHandle->Set_Sample_Loop_Count (m_LoopCount);
	}
		
	return ;
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
	return ;
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

	if (m_pConvertedFormat != NULL) {
		m_pConvertedFormat->Re_Sync (*this);
	}

	//
	//	Move the logical sound with the audible one...
	//
	if (m_LogicalSound != NULL) {
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
AudibleSoundClass::Update_Play_Position (void)
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

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Allocate_Miles_Handle (void)
{
	//
	// If we need to, get a play-handle from the audio system
	//
	if (m_SoundHandle == NULL) {
		Set_Miles_Handle ((MILES_HANDLE)WWAudioClass::Get_Instance ()->Get_2D_Sample (*this));
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Loop_End
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::On_Loop_End (void)
{
	// Determine if the sound is actually finished or still looping
	if ((m_LoopCount != INFINITE_LOOPS) && (m_LoopsLeft < 1)) {

		// Let the audio system know that we are done with this sound
		Stop ();
		if (m_Scene != NULL) {
			Remove_From_Scene ();
		}

		// Fire an event
		On_Event (AudioCallbackClass::EVENT_SOUND_ENDED);

	} else {
		Restart_Loop ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Determine_Real_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
float
AudibleSoundClass::Determine_Real_Volume (void) const
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
AudibleSoundClass::Get_Filename (void) const
{
	LPCTSTR filename = NULL;
	if (m_Buffer != NULL) {
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
		if (m_IsCulled || (m_pConvertedFormat != NULL)) {
			Free_Miles_Handle ();
		} else {
			Allocate_Miles_Handle ();
		}
	}

	return ;
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
	return ;
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
	return ;
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
	if ((scene != NULL) && (m_Scene == NULL)) {
		
		//
		//	Add this sound to the static culling system
		//
		m_Scene = scene;
		scene->Add_Static_Sound (this, start_playing);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Scene
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Remove_From_Scene (void)
{
	if (m_Scene != NULL) {

		//
		//	Remove this sound from the  static culling system
		//
		m_Scene->Remove_Static_Sound (this);
		m_Scene = NULL;
		m_PhysWrapper = NULL;
	}

	return ;
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
	return ;
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

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Conversion
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Free_Conversion (void)
{
	if (m_pConvertedFormat != NULL) {
		m_pConvertedFormat->Stop ();
		REF_PTR_RELEASE (m_pConvertedFormat);
	}

	//
	//	Reacquire a play-handle if necessary
	//
	if ((m_IsCulled == false) && (m_State == STATE_PLAYING)) {
		Allocate_Miles_Handle ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Convert_To_Filtered
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundClass::Convert_To_Filtered (void)
{
	if (m_pConvertedFormat == NULL) {
		
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

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	As_Converted_Format
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
AudibleSoundClass::As_Converted_Format (void)
{
	if (m_pConvertedFormat == NULL) {
		Convert_To_Filtered ();
	}

	return m_pConvertedFormat;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
AudibleSoundClass::Get_Factory (void) const
{	
	return _AudibleSoundPersistFactory;
}


//************************************************************************************************
//*
//*	Start of AudibleSoundDefinitionClass
//*
//************************************************************************************************

////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AudibleSoundDefinitionClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundDefinitionClass::AudibleSoundDefinitionClass (void)
	:	m_Priority (0.5F),
		m_Volume (1.0F),
		m_Pan (0.5F),
		m_LoopCount (1),
		m_DropOffRadius (40.0F),
		m_LogicalDropOffRadius (-1.0F),
		m_MaxVolRadius (20.0F),
		m_Is3D (true),
		m_Type (AudibleSoundClass::TYPE_SOUND_EFFECT),
		m_LogicalTypeMask (0),
		m_LogicalNotifyDelay (2),
		m_CreateLogical (false),
		m_AttenuationSphereColor (0, 0.75F, 0.75F),
		m_StartOffset (0),
		m_PitchFactor (1.0F)
{
	//
	//	Audible sound params
	//
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_SOUND_FILENAME, m_Filename, "Filename");
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_FLOAT, m_DropOffRadius, "Drop-off Radius");
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_FLOAT, m_MaxVolRadius, "Max-Vol Radius");	
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_BOOL, m_Is3D, "Is 3D Sound");
	INT_EDITABLE_PARAM   (AudibleSoundDefinitionClass, m_LoopCount, 0, 1000000);
	FLOAT_EDITABLE_PARAM (AudibleSoundDefinitionClass, m_Volume, 0, 1.0F);
	FLOAT_EDITABLE_PARAM (AudibleSoundDefinitionClass, m_Pan, 0, 1.0F);
	FLOAT_EDITABLE_PARAM (AudibleSoundDefinitionClass, m_Priority, 0, 1.0F);
	ENUM_PARAM (AudibleSoundDefinitionClass, m_Type, ("Sound Effect", AudibleSoundClass::TYPE_SOUND_EFFECT, "Music", AudibleSoundClass::TYPE_MUSIC, 0));
	FLOAT_EDITABLE_PARAM (AudibleSoundDefinitionClass, m_StartOffset, 0, 1.0F);
	FLOAT_EDITABLE_PARAM (AudibleSoundDefinitionClass, m_PitchFactor, 0, 1.0F);
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_STRING, m_DisplayText, "Display Text");

	//
	//	Logical sound params
	//
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_BOOL, m_CreateLogical, "Create Logical Sound");
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_FLOAT, m_LogicalDropOffRadius, "Logical Drop-off Radius");	
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_FLOAT, m_LogicalNotifyDelay, "Logical Notif Delay");		

#ifdef	PARAM_EDITING_ON
	//
	//	Configure the logical type mask enumeration
	//	
	EnumParameterClass *param = W3DNEW EnumParameterClass (&m_LogicalTypeMask);
	param->Set_Name ("Logical Type");
	int count = WWAudioClass::Get_Instance ()->Get_Logical_Type_Count ();
	for (int index = 0; index < count; index ++) {					
		StringClass display_name(0,true);
		int id = WWAudioClass::Get_Instance ()->Get_Logical_Type (index, display_name);
		param->Add_Value (display_name, id);
	}
	GENERIC_EDITABLE_PARAM(AudibleSoundDefinitionClass, param);
#endif
	
	NAMED_EDITABLE_PARAM (AudibleSoundDefinitionClass, ParameterClass::TYPE_COLOR, m_AttenuationSphereColor, "Sphere Color");

	return ;
}

// SKB: Put here because of conficts with CLASSID_???? with other projects.
uint32 AudibleSoundDefinitionClass::Get_Class_ID (void) const
{
	return CLASSID_SOUND; 
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_From_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
AudibleSoundDefinitionClass::Initialize_From_Sound (AudibleSoundClass *sound)
{
	//
	// Read the settings from the sound object
	//
	if (sound != NULL) {
		Sound3DClass *sound_3d	= sound->As_Sound3DClass ();

		//
		//	Choose defaults for the values that we can't get from
		// the sound.
		//
		m_LogicalDropOffRadius	= -1.0F;
		m_LogicalNotifyDelay		= 2;
		m_LogicalTypeMask			= 0;
		m_CreateLogical			= false;
		m_Pan							= 0.5F;
		m_DisplayText				= "";

		//
		//	Copy the values that we can from the sound object
		//
		m_Filename			= sound->Get_Filename ();
		m_DropOffRadius	= sound->Get_DropOff_Radius ();		
		m_Priority			= sound->Peek_Priority ();
		m_Is3D				= (sound_3d != NULL);
		m_Type				= sound->Get_Type ();
		m_LoopCount			= sound->Get_Loop_Count ();
		m_Volume				= sound->Get_Volume ();
		m_StartOffset		= sound->Get_Start_Offset ();
		m_PitchFactor		= sound->Get_Pitch_Factor ();

		if (sound_3d != NULL) {
			m_MaxVolRadius = sound_3d->Get_Max_Vol_Radius ();
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
AudibleSoundDefinitionClass::Get_Factory (void) const
{	
	return _AudibleSoundDefPersistFactory;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundDefinitionClass::Save (ChunkSaveClass &csave)
{
	using namespace AUDIBLE_SOUND_DEF_SAVELOAD;
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	retval &= Save_Variables (csave);
	csave.End_Chunk ();

	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
	retval &= DefinitionClass::Save (csave);
	csave.End_Chunk ();

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundDefinitionClass::Load (ChunkLoadClass &cload)
{
	using namespace AUDIBLE_SOUND_DEF_SAVELOAD;
	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {
			
			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;

			case CHUNKID_BASE_CLASS:
				retval &= DefinitionClass::Load (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundDefinitionClass::Save_Variables (ChunkSaveClass &csave)
{
	using namespace AUDIBLE_SOUND_DEF_SAVELOAD;

	//
	//	Save the audible variables
	//
	WRITE_MICRO_CHUNK (csave, VARID_PRIORITY,						m_Priority)
	WRITE_MICRO_CHUNK (csave, VARID_VOLUME,						m_Volume)
	WRITE_MICRO_CHUNK (csave, VARID_PAN,							m_Pan)
	WRITE_MICRO_CHUNK (csave, VARID_LOOP_COUNT,					m_LoopCount)
	WRITE_MICRO_CHUNK (csave, VARID_DROP_OFF,						m_DropOffRadius)
	WRITE_MICRO_CHUNK (csave, VARID_MAX_VOL,						m_MaxVolRadius)
	WRITE_MICRO_CHUNK (csave, VARID_TYPE,							m_Type)
	WRITE_MICRO_CHUNK (csave, VARID_IS3D,							m_Is3D)
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_FILENAME,			m_Filename)
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_DISPLAY_TEXT,	m_DisplayText)
	WRITE_MICRO_CHUNK (csave, VARID_START_OFFSET,				m_StartOffset);
	WRITE_MICRO_CHUNK (csave, VARID_PITCH_FACTOR,				m_PitchFactor);	

	//
	//	Save the logical variables
	//
	WRITE_MICRO_CHUNK (csave, VARID_LOGICAL_MASK,				m_LogicalTypeMask)
	WRITE_MICRO_CHUNK (csave, VARID_LOGICAL_DELAY,				m_LogicalNotifyDelay)
	WRITE_MICRO_CHUNK (csave, VARID_CREATE_LOGICAL,				m_CreateLogical)
	WRITE_MICRO_CHUNK (csave, VARID_LOGICAL_DROP_OFF,			m_LogicalDropOffRadius)
	WRITE_MICRO_CHUNK (csave, VARID_SPHERE_COLOR,				m_AttenuationSphereColor)
	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundDefinitionClass::Load_Variables (ChunkLoadClass &cload)
{
	using namespace AUDIBLE_SOUND_DEF_SAVELOAD;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			
			READ_MICRO_CHUNK (cload, VARID_PRIORITY,					m_Priority)
			READ_MICRO_CHUNK (cload, VARID_VOLUME,						m_Volume)
			READ_MICRO_CHUNK (cload, VARID_PAN,							m_Pan)
			READ_MICRO_CHUNK (cload, VARID_LOOP_COUNT,				m_LoopCount)
			READ_MICRO_CHUNK (cload, VARID_DROP_OFF,					m_DropOffRadius)
			READ_MICRO_CHUNK (cload, VARID_MAX_VOL,					m_MaxVolRadius)
			READ_MICRO_CHUNK (cload, VARID_TYPE,						m_Type)
			READ_MICRO_CHUNK (cload, VARID_IS3D,						m_Is3D)
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_FILENAME,		m_Filename)			
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_DISPLAY_TEXT,	m_DisplayText)
			READ_MICRO_CHUNK (cload, VARID_LOGICAL_MASK,				m_LogicalTypeMask)
			READ_MICRO_CHUNK (cload, VARID_LOGICAL_DELAY,			m_LogicalNotifyDelay)
			READ_MICRO_CHUNK (cload, VARID_CREATE_LOGICAL,			m_CreateLogical)
			READ_MICRO_CHUNK (cload, VARID_LOGICAL_DROP_OFF,		m_LogicalDropOffRadius)	
			READ_MICRO_CHUNK (cload, VARID_SPHERE_COLOR,				m_AttenuationSphereColor)
			READ_MICRO_CHUNK (cload, VARID_START_OFFSET,				m_StartOffset);
			READ_MICRO_CHUNK (cload, VARID_PITCH_FACTOR,				m_PitchFactor);
		}

		cload.Close_Micro_Chunk ();
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////////////
PersistClass *
AudibleSoundDefinitionClass::Create (void) const
{
	return Create_Sound (CLASSID_3D);
}

//////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound
//
//////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
AudibleSoundDefinitionClass::Create_Sound (int classid_hint) const
{
	AudibleSoundClass *new_sound = NULL;

	//
	//	If this is a relative path, strip it off and assume
	// the current directory is set correctly.
	//
	StringClass real_filename(m_Filename,true);
	const char *dir_delimiter = ::strrchr (m_Filename, '\\');
	if (dir_delimiter != NULL && m_Filename.Get_Length () > 2 && m_Filename[1] != ':') {
		real_filename = (dir_delimiter + 1);
	}

	//
	//	Should we create a 2D or 3D sound?
	//
	if (m_Is3D) {
		new_sound = WWAudioClass::Get_Instance ()->Create_3D_Sound (real_filename, classid_hint);
	} else {
		new_sound = WWAudioClass::Get_Instance ()->Create_Sound_Effect (real_filename);
	}

	//
	//	Did we successfully create the sound?
	//
	if (new_sound != NULL) {
		
		//
		//	Configure the sound
		//
		new_sound->Set_Type ((AudibleSoundClass::SOUND_TYPE)m_Type);
		new_sound->Set_Priority (m_Priority);
		new_sound->Set_Volume (m_Volume);
		new_sound->Set_Loop_Count (m_LoopCount);
		new_sound->Set_DropOff_Radius (m_DropOffRadius);
		new_sound->Set_Definition ((AudibleSoundDefinitionClass *)this);
		new_sound->Set_Start_Offset (m_StartOffset);
		new_sound->Set_Pitch_Factor (m_PitchFactor);		

		if (m_Is3D) {
			((Sound3DClass *)new_sound)->Set_Max_Vol_Radius (m_MaxVolRadius);
		}
	}

	return new_sound;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create_Logical
//
//////////////////////////////////////////////////////////////////////////////////
LogicalSoundClass *
AudibleSoundDefinitionClass::Create_Logical (void)
{
	LogicalSoundClass *logical_sound = NULL;

	if (m_CreateLogical) {
		
		//
		//	Create and configure the logical sound
		//
		logical_sound = W3DNEW LogicalSoundClass;
		logical_sound->Set_Type_Mask (m_LogicalTypeMask);
		logical_sound->Set_Notify_Delay (m_LogicalNotifyDelay);		
		logical_sound->Set_Single_Shot (m_LoopCount != 0);

		//
		//	Use the audible sound's drop-off radius if the logical
		// isn't set...
		//
		if (m_LogicalDropOffRadius < 0) {
			logical_sound->Set_DropOff_Radius (m_DropOffRadius);
		} else {
			logical_sound->Set_DropOff_Radius (m_LogicalDropOffRadius);			
		}
	}

	return logical_sound;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundClass::Save (ChunkSaveClass &csave)
{
	using namespace AUDIBLE_SOUND_SAVELOAD;

	csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		SoundSceneObjClass::Save (csave);
	csave.End_Chunk ();

	uint32 temp_position = 0;
	csave.Begin_Chunk (CHUNKID_VARIABLES);
		WRITE_MICRO_CHUNK (csave, VARID_STATE, m_State);
		WRITE_MICRO_CHUNK (csave, VARID_TYPE, m_Type);

		WRITE_MICRO_CHUNK (csave, VARID_PRIORITY, m_Priority);
		WRITE_MICRO_CHUNK (csave, VARID_VOLUME, m_Volume);
		WRITE_MICRO_CHUNK (csave, VARID_PAN, m_Pan);
		WRITE_MICRO_CHUNK (csave, VARID_LOOP_COUNT, m_LoopCount);
		WRITE_MICRO_CHUNK (csave, VARID_LOOPS_LEFT, m_LoopsLeft);
		WRITE_MICRO_CHUNK (csave, VARID_SOUND_LENGTH, m_Length);
		WRITE_MICRO_CHUNK (csave, VARID_CURR_POS, temp_position);
		WRITE_MICRO_CHUNK (csave, VARID_TRANSFORM, m_Transform);
		WRITE_MICRO_CHUNK (csave, VARID_PREV_TRANSFORM, m_PrevTransform);
		WRITE_MICRO_CHUNK (csave, VARID_IS_CULLED, m_IsCulled);
		WRITE_MICRO_CHUNK (csave, VARID_IS_DIRTY, m_bDirty);			
		WRITE_MICRO_CHUNK (csave, VARID_DROP_OFF, m_DropOffRadius);
		WRITE_MICRO_CHUNK (csave, VARID_START_OFFSET, m_StartOffset);
		WRITE_MICRO_CHUNK (csave, VARID_PITCH_FACTOR, m_PitchFactor);
		WRITE_MICRO_CHUNK (csave, VARID_LISTENER_TRANSFORM, m_ListenerTransform);
		
		if (m_Buffer != NULL) {
			WRITE_MICRO_CHUNK_STRING (csave, VARID_FILENAME, m_Buffer->Get_Filename ());
		}

		AudibleSoundClass *this_ptr = this;
		WRITE_MICRO_CHUNK (csave, VARID_THIS_PTR, this_ptr);

	csave.End_Chunk ();

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
AudibleSoundClass::Load (ChunkLoadClass &cload)
{
	using namespace AUDIBLE_SOUND_SAVELOAD;

	StringClass filename(0,true);
	while (cload.Open_Chunk ()) {		
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_CLASS:
				SoundSceneObjClass::Load (cload);
				break;

			case CHUNKID_VARIABLES:
			{
				//
				//	Read all the variables from their micro-chunks
				//
				while (cload.Open_Micro_Chunk ()) {
					switch (cload.Cur_Micro_Chunk_ID ()) {

						READ_MICRO_CHUNK (cload, VARID_STATE, m_State);
						READ_MICRO_CHUNK (cload, VARID_TYPE, m_Type);

						READ_MICRO_CHUNK (cload, VARID_PRIORITY, m_Priority);
						READ_MICRO_CHUNK (cload, VARID_VOLUME, m_Volume);
						READ_MICRO_CHUNK (cload, VARID_PAN, m_Pan);
						READ_MICRO_CHUNK (cload, VARID_LOOP_COUNT, m_LoopCount);
						READ_MICRO_CHUNK (cload, VARID_LOOPS_LEFT, m_LoopsLeft);
						READ_MICRO_CHUNK (cload, VARID_SOUND_LENGTH, m_Length);
						READ_MICRO_CHUNK (cload, VARID_CURR_POS, m_CurrentPosition);
						READ_MICRO_CHUNK (cload, VARID_TRANSFORM, m_Transform);
						READ_MICRO_CHUNK (cload, VARID_PREV_TRANSFORM, m_PrevTransform);
						READ_MICRO_CHUNK (cload, VARID_IS_CULLED, m_IsCulled);
						READ_MICRO_CHUNK (cload, VARID_IS_DIRTY, m_bDirty);			
						READ_MICRO_CHUNK (cload, VARID_DROP_OFF, m_DropOffRadius);
						READ_MICRO_CHUNK (cload, VARID_START_OFFSET, m_StartOffset);
						READ_MICRO_CHUNK (cload, VARID_PITCH_FACTOR, m_PitchFactor);
						READ_MICRO_CHUNK (cload, VARID_LISTENER_TRANSFORM, m_ListenerTransform);
						
						READ_MICRO_CHUNK_WWSTRING (cload, VARID_FILENAME, filename);
						
						case VARID_THIS_PTR:
						{
							AudibleSoundClass *old_ptr = NULL;
							cload.Read(&old_ptr, sizeof (old_ptr));
							SaveLoadSystemClass::Register_Pointer (old_ptr, this);
						}
						break;
					}

					cload.Close_Micro_Chunk ();
				}
			}
			break;
		}

		cload.Close_Chunk ();
	}

	//
	//	Reconstruct the sound buffer we had before we saved
	//
	if (filename.Get_Length () > 0) {
		bool is_3d = (As_Sound3DClass () != NULL);
		SoundBufferClass *buffer = WWAudioClass::Get_Instance ()->Get_Sound_Buffer (filename, is_3d);
		Set_Buffer (buffer);
		REF_PTR_RELEASE (buffer);
	}

	return true;
}
