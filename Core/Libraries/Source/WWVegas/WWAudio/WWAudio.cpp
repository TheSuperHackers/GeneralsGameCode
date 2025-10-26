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
 *                     $Archive:: /Commando/Code/WWAudio/WWAudio.cpp                          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/14/01 2:27p                                               $*
 *                                                                                             *
 *                    $Revision:: 52                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "always.h"
#include <windows.h>
#include "WWAudio.h"
#include "wwdebug.h"
#include "Utils.h"
#include "realcrc.h"
#include "SoundBuffer.h"
#include "AudibleSound.h"
#include "Sound3D.h"
#include "RAWFILE.H"
#include "SoundScene.h"
#include "SoundPseudo3D.h"
#include "ffactory.h"
#include "registry.h"
#include "Threads.h"
#include "LogicalSound.h"
#include "LogicalListener.h"
#include "definitionclassids.h"
#include "wwmemlog.h"
#include "wwprofile.h"


#ifdef G_CODE_BASE
#include "../WWLib/argv.h"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////////////////////////////////////
WWAudioClass *WWAudioClass::_theInstance = NULL;
HANDLE WWAudioClass::_TimerSyncEvent = NULL;


////////////////////////////////////////////////////////////////////////////////////////////////
//	Registry value names
////////////////////////////////////////////////////////////////////////////////////////////////
const char *VALUE_NAME_IS_STEREO			= "stereo";
const char *VALUE_NAME_BITS				= "bits";
const char *VALUE_NAME_HERTZ				= "hertz";
const char *VALUE_NAME_DEVICE_NAME		= "device name";
const char *VALUE_NAME_MUSIC_ENABLED	= "music enabled";
const char *VALUE_NAME_SOUND_ENABLED	= "sound enabled";
const char *VALUE_NAME_MUSIC_VOL			= "music volume";
const char *VALUE_NAME_SOUND_VOL			= "sound volume";


////////////////////////////////////////////////////////////////////////////////////////////////
//	Local inlines
////////////////////////////////////////////////////////////////////////////////////////////////
__inline bool
WWAudioClass::Is_OK_To_Give_Handle (const AudibleSoundClass &sound_obj)
{
	bool is_ok = false;
	AudibleSoundClass::SOUND_TYPE type = sound_obj.Get_Type ();
	if (((type == AudibleSoundClass::TYPE_SOUND_EFFECT) && m_AreSoundEffectsEnabled) ||
		 ((type == AudibleSoundClass::TYPE_MUSIC) && m_IsMusicEnabled)) {
		is_ok = true;
	}
	return is_ok;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	WWAudioClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
WWAudioClass::WWAudioClass (void)
	: m_Driver2D (NULL),
	  m_Driver3D (NULL),
	  m_PlaybackRate (44100),
	  m_PlaybackBits (16),
	  m_PlaybackStereo (true),
	  m_ReverbFilter ((HPROVIDER)INVALID_MILES_HANDLE),
	  m_UpdateTimer (-1),
	  m_Driver3DPseudo (NULL),
	  m_MusicVolume (DEF_MUSIC_VOL),
	  m_SoundVolume (DEF_SFX_VOL),
	  m_MaxCacheSize (DEF_CACHE_SIZE * 1024),
	  m_CurrentCacheSize (0),
	  m_Max2DSamples (DEF_2D_SAMPLE_COUNT),
	  m_Max3DSamples (DEF_3D_SAMPLE_COUNT),
	  m_Max2DBufferSize (DEF_MAX_2D_BUFFER_SIZE),
	  m_Max3DBufferSize (DEF_MAX_3D_BUFFER_SIZE),
	  m_SoundScene (NULL),
	  m_IsMusicEnabled (true),
	  m_AreSoundEffectsEnabled (true),
	  m_FileFactory (NULL),
	  m_EffectsLevel (0),
	  m_ReverbRoomType (ENVIRONMENT_GENERIC)
{
	::InitializeCriticalSection (&MMSLockClass::_MSSLockCriticalSection);

	//
	// Start Miles Sound System
	//
	AIL_startup ();
	_theInstance = this;
	_TimerSyncEvent = ::CreateEvent (NULL, TRUE, FALSE, "WWAUDIO_TIMER_SYNC");

	//
	// Set some default values
	//
	Set_Sound_Effects_Volume ();
	Set_Music_Volume ();

	// Create a new sound scene to manage our 3D sounds...
	m_SoundScene = W3DNEW SoundSceneClass;
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~WWAudioClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
WWAudioClass::~WWAudioClass (void)
{
	//
	//	Make sure the delayed-release thread is terminated
	// before we exit (otherwise the process will crash).
	//
	WWAudioThreadsClass::End_Delayed_Release_Thread ();

	Shutdown ();
	_theInstance = NULL;
	::CloseHandle(_TimerSyncEvent);
	_TimerSyncEvent = NULL;

	::DeleteCriticalSection (&MMSLockClass::_MSSLockCriticalSection);

	//
	//	Free the list of logical "types".
	//
	Reset_Logical_Types ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Cache
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Flush_Cache (void)
{
	// Loop through all the hash indicies
	for (int hash_index = 0; hash_index < MAX_CACHE_HASH; hash_index ++) {

		// Loop through all the buffers at this hash index and free them all
		for (int index = 0; index < m_CachedBuffers[hash_index].Count (); index ++) {
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];

			// Free the buffer data
			SAFE_FREE (info.string_id);
			REF_PTR_RELEASE (info.buffer);
		}

		// Remove all the entries for this hash index
		m_CachedBuffers[hash_index].Delete_All ();
	}

	m_CurrentCacheSize = 0;
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Open_2D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
WWAudioClass::DRIVER_TYPE_2D
WWAudioClass::Open_2D_Device (LPWAVEFORMAT format)
{
	MMSLockClass lock;

	//
	//	Store the playback settings for future reference
	//
	m_PlaybackRate		= format->nSamplesPerSec;
	m_PlaybackBits		= (format->nAvgBytesPerSec << 3) / (format->nChannels * format->nSamplesPerSec);
	m_PlaybackStereo	= bool(format->nChannels > 1);

	//
	// Assume we will open the DirectSound driver
	//
	DRIVER_TYPE_2D type = DRIVER2D_DSOUND;

	// First close the current 2D device and take
	// all the sound handles away from the sound objects.
	Close_2D_Device ();

	AIL_set_preference (AIL_LOCK_PROTECTION, NO);

	// Try to use DirectSound if possible
	S32 success = ::AIL_set_preference (DIG_USE_WAVEOUT, FALSE);
	WWASSERT (success == AIL_NO_ERROR);

	// Open the driver
	success = ::AIL_waveOutOpen (&m_Driver2D, NULL, 0, format);

	// Do we need to switch from direct sound to waveout?
	if ((success == AIL_NO_ERROR) &&
		 (m_Driver2D != NULL) &&
		 (m_Driver2D->emulated_ds == TRUE)) {
		::AIL_waveOutClose (m_Driver2D);
		success = 2;
		WWDEBUG_SAY (("WWAudio: Detected 2D DirectSound emulation, switching to WaveOut."));
   }

	// If we couldn't open the direct sound device, then use the
	// default wave out device
	if (success != AIL_NO_ERROR) {

		// Try to use the default wave out driver
		success = ::AIL_set_preference (DIG_USE_WAVEOUT, TRUE);
		WWASSERT (success == AIL_NO_ERROR);

		// Open the driver
		success = ::AIL_waveOutOpen (&m_Driver2D, NULL, 0, format);
		WWASSERT (success == AIL_NO_ERROR);
		type = (success == AIL_NO_ERROR) ? DRIVER2D_WAVEOUT : DRIVER2D_ERROR;
	}

	// Allocate all the available handles if we were successful
	if (success == AIL_NO_ERROR) {
		Allocate_2D_Handles ();
		ReAssign_2D_Handles ();
	} else {
		Close_2D_Device ();
		WWDEBUG_SAY (("WWAudio: Error initializing 2D device."));
	}

	// Return the opened device type
	return type;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Open_2D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
WWAudioClass::DRIVER_TYPE_2D
WWAudioClass::Open_2D_Device
(
	bool stereo,
	int bits,
	int hertz
)
{
	// Build a wave format structure from the params
	PCMWAVEFORMAT wave_format = { 0 };
	wave_format.wf.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.wf.nChannels = stereo ? 2 : 1;
	wave_format.wf.nSamplesPerSec = hertz;
	wave_format.wf.nAvgBytesPerSec = (wave_format.wf.nChannels * wave_format.wf.nSamplesPerSec * bits) >> 3;
	wave_format.wf.nBlockAlign = (wave_format.wf.nChannels * bits) >> 3;
	wave_format.wBitsPerSample = bits;

	DRIVER_TYPE_2D type = DRIVER2D_ERROR;
	while (((type = Open_2D_Device ((LPWAVEFORMAT)&wave_format)) == DRIVER2D_ERROR) &&
			 (wave_format.wf.nSamplesPerSec >= 11025)) {

		//
		//	Cut the playback rate in half and try again
		//
		wave_format.wf.nSamplesPerSec = wave_format.wf.nSamplesPerSec >> 1;
		wave_format.wf.nAvgBytesPerSec = (wave_format.wf.nChannels * wave_format.wf.nSamplesPerSec * bits) >> 3;
		wave_format.wf.nBlockAlign = (wave_format.wf.nChannels * bits) >> 3;
	}

	// Pass this structure onto the function that actually opens the device
	return type;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Close_2D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Close_2D_Device (void)
{
	MMSLockClass lock;

	bool retval = false;

	//
	//	Free any 2D sound handles
	//
	Remove_2D_Sound_Handles ();
	Release_2D_Handles ();

	//
	// Do we have an open driver handle to close?
	//
	if (m_Driver2D != NULL) {

		//
		// Close the driver
		//
		::AIL_waveOutClose (m_Driver2D);
		m_Driver2D = NULL;
		retval = true;
	}

	//
	// Return the true/false result code
	//
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
WWAudioClass::Get_Sound_Buffer (const char *filename, bool is_3d)
{
	//
	// Try to find the buffer in our cache, otherwise create a new buffer.
	//
	SoundBufferClass *buffer = Find_Cached_Buffer (filename);
	if (buffer == NULL) {
		FileClass *file = Get_File (filename);
		if (file != NULL && file->Is_Available ()) {
			buffer = Create_Sound_Buffer (*file, filename, is_3d);
		} else {
			static int count = 0;
			if ( count++ < 10 ) {
				WWDEBUG_SAY(( "Sound \"%s\" not found", filename ));
			}
		}
		Return_File (file);
	}

	return buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
WWAudioClass::Get_Sound_Buffer (FileClass &file, const char *string_id, bool is_3d)
{
	WWMEMLOG(MEM_SOUND);

	//
	// Try to find the buffer in our cache, otherwise create a new buffer.
	//
	SoundBufferClass *buffer = Find_Cached_Buffer (string_id);
	if (buffer == NULL) {
		buffer = Create_Sound_Buffer (file, string_id, is_3d);
	}

	return buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Find_Cached_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
WWAudioClass::Find_Cached_Buffer (const char *string_id)
{
	SoundBufferClass *sound_buffer = NULL;

	// Param OK?
	WWASSERT (string_id != NULL);
	if (string_id != NULL) {

		//
		// Determine which index in our hash table to use
		//
		int hash_index = ::CRC_Stringi (string_id) & CACHE_HASH_MASK;

		//
		// Loop through all the buffers at this hash index and try to find
		// one that matches the requested name
		//
		for (int index = 0; index < m_CachedBuffers[hash_index].Count (); index ++) {

			//
			// Is this the sound buffer we were looking for?
			//
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];
			if (::lstrcmpi (info.string_id, string_id) == 0) {
				sound_buffer = info.buffer;
				sound_buffer->Add_Ref ();
				break;
			}
		}
	}

	//
	// Return a pointer to the cached sound buffer
	//
	return sound_buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Cache_Space
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Free_Cache_Space (int bytes)
{
	int bytes_freed = 0;

	// Loop through all the hash indicies
	for (int hash_index = 0;
		  (hash_index < MAX_CACHE_HASH) && (bytes_freed < bytes);
		  hash_index ++) {

		// Loop through all the buffers at this hash index
		for (int index = 0;
			  (index < m_CachedBuffers[hash_index].Count ()) && (bytes_freed < bytes);
			  index ++) {

			// Can we free this cached buffer?
			CACHE_ENTRY_STRUCT &info = m_CachedBuffers[hash_index][index];
			if ((info.buffer != NULL) && (info.buffer->Num_Refs () == 1)) {

				// Add the size of this buffer to our count of bytes freed
				bytes_freed += info.buffer->Get_Raw_Length ();

				// Free the buffer data
				SAFE_FREE (info.string_id);
				REF_PTR_RELEASE (info.buffer);

				// Remove this entry from the hash table
				m_CachedBuffers[hash_index].Delete (index);
				index --;
			}
		}
	}

	// Make sure to recompute out current cache size
	m_CurrentCacheSize -= bytes_freed;
	WWASSERT (m_CurrentCacheSize >= 0);

	// Return true if we freed enough bytes in the cache
	return (bytes_freed >= bytes);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Cache_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Cache_Buffer
(
	SoundBufferClass *buffer,
	const char *string_id
)
{
	// Assume failure
	bool retval = false;

	// Params OK?
	WWASSERT (buffer != NULL);
	WWASSERT (string_id != NULL);
	if ((buffer != NULL) &&
		 (string_id != NULL) &&
		 (buffer->Get_Raw_Length () < (U32)(m_MaxCacheSize / 2))) {

		// Attempt to free space in the cache (if needed)
		int space_needed = (m_CurrentCacheSize + buffer->Get_Raw_Length ()) - (int)m_MaxCacheSize;
		if (space_needed > 0) {
			Free_Cache_Space (space_needed);
		}

		// Do we have enough space in the cache for this buffer?
		space_needed = (m_CurrentCacheSize + buffer->Get_Raw_Length ()) - (int)m_MaxCacheSize;
		if (space_needed <= 0) {

			// Determine which index in our hash table to use
			int hash_index = ::CRC_Stringi (string_id) & CACHE_HASH_MASK;

			//
			// Add this buffer to the hash table at the given index.
			//	Note:  The assignment operator caused by the Add call
			//			will add a reference to the sound buffer.
			//
			CACHE_ENTRY_STRUCT info;
			info.string_id = (char *)string_id;
			info.buffer = buffer;
			m_CachedBuffers[hash_index].Add (info);

			// Update our current cache size
			m_CurrentCacheSize += buffer->Get_Raw_Length ();
			retval = true;
		}
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
WWAudioClass::Create_Sound_Buffer
(
	FileClass &	file,
	const char *string_id,
	bool			is_3d
)
{
	SoundBufferClass *sound_buffer = NULL;

	//
	//	Determine how large this buffer can be
	//
	int max_size = is_3d ? m_Max3DBufferSize : m_Max2DBufferSize;

	//
	// Create a streaming sound buffer object if the
	// file is too large to preload.
	//
	if (file.Size () > max_size) {
		sound_buffer = W3DNEW StreamSoundBufferClass;
	} else {
		sound_buffer = W3DNEW SoundBufferClass;
	}
	SET_REF_OWNER(sound_buffer);

	//
	// Create a new sound buffer from the provided file
	//
	bool success = sound_buffer->Load_From_File (file);
	sound_buffer->Set_Filename (string_id);
	WWASSERT (success);

	// If we were successful in creating the sound buffer, then
	// try to cache it as well, otherwise free the buffer and return NULL.
	if (success && (string_id != NULL)) {
		Cache_Buffer (sound_buffer, string_id);
	} else if (success == false) {
		REF_PTR_RELEASE (sound_buffer);
	}

	// Return a pointer to the new sound buffer
	return sound_buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundBufferClass *
WWAudioClass::Create_Sound_Buffer
(
	unsigned char *	file_image,
	unsigned long		bytes,
	const char *		string_id,
	bool					is_3d
)
{
	// Create a new sound buffer from the provided file
	SoundBufferClass *sound_buffer = W3DNEW SoundBufferClass;
	SET_REF_OWNER(sound_buffer);

	//
	//	Initialize the sound from this piece of memory
	//
	bool success = sound_buffer->Load_From_Memory (file_image, bytes);
	sound_buffer->Set_Filename (string_id);
	WWASSERT (success);

	// If we were successful in creating the sound buffer, then
	// try to cache it as well, otherwise free the buffer and return NULL.
	if (success && (string_id != NULL)) {
		Cache_Buffer (sound_buffer, string_id);
	} else if (success == false) {
		REF_PTR_RELEASE (sound_buffer);
	}

	// Return a pointer to the new sound buffer
	return sound_buffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound_Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound_Effect (FileClass &file, const char *string_id)
{
	// Create a new sound object
	AudibleSoundClass *sound_obj = W3DNEW AudibleSoundClass;
	if (Is_Disabled () == false) {

		// Try to find the buffer in our cache, otherwise create a new buffer.
		SoundBufferClass *buffer = Get_Sound_Buffer (file, string_id, false);

		// Pass the actual sound data onto the sound object
		sound_obj->Set_Buffer (buffer);
		REF_PTR_RELEASE (buffer);
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound_Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound_Effect (const char *filename)
{
	// Assume failure
	AudibleSoundClass *sound_obj = NULL;
	if (Is_Disabled () == false) {

		// Param OK?
		WWASSERT (filename != NULL);
		if (filename != NULL) {

			// Create a file object and pass it onto the appropriate function
			FileClass *file = Get_File (filename);
			if (file && file->Is_Available()) {
				sound_obj = Create_Sound_Effect (*file, filename);
			} else {
				WWDEBUG_SAY(( "Sound %s not found", filename ));
			}
			Return_File (file);

		}
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound_Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound_Effect
(
	const char *	string_id,
	unsigned char *raw_wave_data,
	unsigned long	bytes
)
{
	// Create a new sound object
	AudibleSoundClass *sound_obj = W3DNEW AudibleSoundClass;
	if (Is_Disabled () == false) {

		// Try to find the buffer in our cache, otherwise create a new buffer.
		SoundBufferClass *buffer = Find_Cached_Buffer (string_id);
		if (buffer == NULL) {
			buffer = Create_Sound_Buffer (raw_wave_data, bytes, string_id, false);
		}

		// Pass the actual sound data onto the sound object
		sound_obj->Set_Buffer (buffer);
		REF_PTR_RELEASE (buffer);
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Create_3D_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
Sound3DClass *
WWAudioClass::Create_3D_Sound (FileClass &file, const char *string_id, int classid_hint)
{
	Sound3DClass *sound_obj = NULL;
	if (Is_Disabled () == false) {

		// Try to find the buffer in our cache, otherwise create a new buffer.
		SoundBufferClass *buffer = Get_Sound_Buffer (file, string_id, true);

		//
		// What type of sound object should we create? A true 3D sound or one of
		// our pseudo-3d sounds?  (volume and panning only)
		//
		if (	classid_hint == CLASSID_PSEUDO3D ||
				Validate_3D_Sound_Buffer (buffer) == false)
		{
			sound_obj = W3DNEW SoundPseudo3DClass;
			sound_obj->Set_Buffer (buffer);
		} else if (buffer != NULL) {
			sound_obj = W3DNEW Sound3DClass;
			sound_obj->Set_Buffer (buffer);
		}

		REF_PTR_RELEASE (buffer);
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_3D_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
Sound3DClass *
WWAudioClass::Create_3D_Sound
(
	const char *	filename,
	int				classid_hint
)
{
	WWMEMLOG(MEM_SOUND);

	// Assume failure
	Sound3DClass *sound_obj = NULL;
	if (Is_Disabled () == false) {

		// Param OK?
		WWASSERT (filename != NULL);
		if (filename != NULL) {

			// Try to find the buffer in our cache, otherwise create a new buffer.
			SoundBufferClass *buffer = Get_Sound_Buffer (filename, true);

			//
			// What type of sound object should we create? A true 3D sound or one of
			// our pseudo-3d sounds?  (volume and panning only)
			//
			if (	classid_hint == CLASSID_PSEUDO3D ||
					Validate_3D_Sound_Buffer (buffer) == false)
			{
				sound_obj = W3DNEW SoundPseudo3DClass;
				sound_obj->Set_Buffer (buffer);
			} else if (buffer != NULL) {
				sound_obj = W3DNEW Sound3DClass;
				sound_obj->Set_Buffer (buffer);
			} else {
				static int count = 0;
				if ( count++ < 10 ) {
					WWDEBUG_SAY(( "Sound File not Found \"%s\"", filename ));
				}
			}

			REF_PTR_RELEASE (buffer);
		}
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_3D_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
Sound3DClass *
WWAudioClass::Create_3D_Sound
(
	const char *		string_id,
	unsigned char *	raw_wave_data,
	unsigned long		bytes,
	int					classid_hint
)
{
	Sound3DClass *sound_obj = NULL;
	if (Is_Disabled () == false) {

		//
		// Try to find the buffer in our cache, otherwise create a new buffer.
		//
		SoundBufferClass *buffer = Find_Cached_Buffer (string_id);
		if (buffer == NULL) {
			buffer = Create_Sound_Buffer (raw_wave_data, bytes, string_id, true);
		}

		//
		// What type of sound object should we create? A true 3D sound or one of
		// our pseudo-3d sounds?  (volume and panning only)
		//
		if (	classid_hint == CLASSID_PSEUDO3D ||
				Validate_3D_Sound_Buffer (buffer) == false)
		{
			sound_obj = W3DNEW SoundPseudo3DClass;
			sound_obj->Set_Buffer (buffer);
		} else if (buffer != NULL) {
			sound_obj = W3DNEW Sound3DClass;
			sound_obj->Set_Buffer (buffer);
		}

		REF_PTR_RELEASE (buffer);
	}

	// Return a pointer to the sound effect
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound
(
	int				definition_id,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	AudibleSoundClass *sound = NULL;

	//
	//	Find the definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Definition (definition_id);
	if (definition != NULL ) {

		//
		//	Make sure this is really a sound definition
		//
		WWASSERT (definition->Get_Class_ID () == CLASSID_SOUND);
		if (definition->Get_Class_ID () == CLASSID_SOUND) {
			AudibleSoundDefinitionClass *sound_def = reinterpret_cast<AudibleSoundDefinitionClass *> (definition);

			//
			//	Create an instance of the sound
			//
			sound = sound_def->Create_Sound (classid_hint);
			if (sound != NULL) {
				sound->Set_User_Data (user_obj, user_data);
			}
		}
	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Sound
(
	const char *	def_name,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	AudibleSoundClass *sound = NULL;

	//
	//	Find the definition
	//
	DefinitionClass *definition = DefinitionMgrClass::Find_Typed_Definition (def_name, CLASSID_SOUND, true);
	if (definition != NULL ) {

		//
		//	Make sure this is really a sound definition
		//
		WWASSERT (definition->Get_Class_ID () == CLASSID_SOUND);
		if (definition->Get_Class_ID () == CLASSID_SOUND) {
			AudibleSoundDefinitionClass *sound_def = reinterpret_cast<AudibleSoundDefinitionClass *> (definition);

			//
			//	Create an instance of the sound
			//
			sound = sound_def->Create_Sound (classid_hint);
			if (sound != NULL) {
				sound->Set_User_Data (user_obj, user_data);
			}
		}
	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Continuous_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Continuous_Sound
(
	int				definition_id,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (definition_id, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () != INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating a continuous sound with a finite loop count!"));
		}
	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Instant_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioClass::Create_Instant_Sound
(
	int					definition_id,
	const Matrix3D &	tm,
	RefCountClass *	user_obj,
	uint32				user_data,
	int					classid_hint
)
{
	int sound_id = 0;

	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (definition_id, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () == INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating an instant sound %s with an infinite loop count!",sound->Get_Definition()->Get_Name()));
		}

		sound_id = sound->Get_ID ();
		sound->Set_Transform (tm);
		sound->Add_To_Scene ();
		sound->Release_Ref ();
	}

	return sound_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Continuous_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Create_Continuous_Sound
(
	const char *	def_name,
	RefCountClass *user_obj,
	uint32			user_data,
	int				classid_hint
)
{
	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (def_name, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () != INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating a continuous sound with a finite loop count!"));
		}

	}

	return sound;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Instant_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioClass::Create_Instant_Sound
(
	const char *		def_name,
	const Matrix3D &	tm,
	RefCountClass *	user_obj,
	uint32				user_data,
	int					classid_hint
)
{
	int sound_id = 0;

	//
	//	Create an instance of the sound and play it
	//
	AudibleSoundClass *sound = Create_Sound (def_name, user_obj, user_data, classid_hint);
	if (sound != NULL) {

		if (sound->Get_Loop_Count () == INFINITE_LOOPS) {
			WWDEBUG_SAY (("Audio Error:  Creating an instant sound %s with an infinite loop count!",sound->Get_Definition()->Get_Name()));
		}

		sound_id = sound->Get_ID ();
		sound->Set_Transform (tm);
		sound->Add_To_Scene ();
		sound->Release_Ref ();
	}

	return sound_id;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Flush_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Flush_Playlist (void)
{
	// First make sure to free any completed sounds
	Free_Completed_Sounds ();

	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist.Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[index];
		REF_PTR_RELEASE (sound_obj);
	}

	// Free the list structure
	m_Playlist.Delete_All ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Completed_Sounds
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Free_Completed_Sounds (void)
{
	if (m_CompletedSounds.Count () > 0) {

		// Loop through all the entries in the completed sounds list
		for (int index = 0; index < m_CompletedSounds.Count (); index ++) {
			AudibleSoundClass *sound_obj = m_CompletedSounds[index];

         WWASSERT(sound_obj != NULL); //TSS 05/24/99

			// Remove this sound from the playlist
			bool found = false;
			for (int play_index = 0; (play_index < m_Playlist.Count ()) && !found; play_index ++) {
				if (m_Playlist[play_index] == sound_obj) {

					// Free our hold on this sound object
					m_Playlist.Delete (play_index);
					REF_PTR_RELEASE (sound_obj);
					found = true;
				}
			}
		}

		// Free the list structure
		m_CompletedSounds.Delete_All ();

		// Try to give a play-handle back to a sound that was priority-bumped.
		Reprioritize_Playlist ();
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Playlist_Entry
//
////////////////////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
WWAudioClass::Get_Playlist_Entry (int index) const
{
	AudibleSoundClass *sound_obj = NULL;

	// Params OK?
	WWASSERT (index >= 0 && index < m_Playlist.Count ());
	if ((index >= 0) && (index < m_Playlist.Count ())) {
		m_Playlist[index]->Add_Ref ();
		m_Playlist[index];
	}

	// Return a pointer to the sound object
	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_To_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Add_To_Playlist (AudibleSoundClass *sound)
{
	// Assume failure
	bool retval = false;

	WWASSERT (sound != NULL);
	if (sound != NULL) {

		// Loop through all the entries in the playlist
		bool already_added = false;
		for (int index = 0; (index < m_Playlist.Count ()) && (already_added == false); index ++) {
			already_added = (sound == m_Playlist[index]);
		}

		// Add this sound to our playlist
		if (already_added == false) {
			sound->Add_Ref ();
			m_Playlist.Add (sound);
		}
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_From_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Remove_From_Playlist (AudibleSoundClass *sound_obj)
{
	// Assume failure
	bool retval = false;

	WWASSERT (sound_obj != NULL);
	if (sound_obj != NULL) {

		// Loop through all the entries in the playlist
		int index = 0;
		for (; (index < m_Playlist.Count ()) && (retval == false); index ++) {

			// Is this the entry we are looking for?
			if (sound_obj == m_Playlist[index]) {

				// Add this sound to the 'completed' list
				m_CompletedSounds.Add (sound_obj);
				retval = true;
			}
		}

		//
		// Notify any callbacks that this sound is ending...
		//
		if (sound_obj->Get_Loop_Count () != INFINITE_LOOPS) {
			for (index = 0; index < m_EOSCallbackList.Count (); index ++) {
				uint32 user_data				= NULL;
				LPFNEOSCALLBACK callback	= m_EOSCallbackList.Get_Callback (index, &user_data);
				if (callback != NULL) {
					(*callback) (sound_obj, user_data);
				}
			}
		}
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Sound_In_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Is_Sound_In_Playlist (AudibleSoundClass *sound_obj)
{
	// Assume failure
	bool retval = false;

	// Loop through all the entries in the playlist
	for (int index = 0; (index < m_Playlist.Count ()) && (retval == false); index ++) {
		if (sound_obj == m_Playlist[index]) {
			retval = true;
		}
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Reprioritize_Playlist
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Reprioritize_Playlist (void)
{
	AudibleSoundClass *sound_to_get_handle = NULL;
	float hightest_priority = 0;

	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist.Count (); index ++) {

		// Is this the highest priority without a miles handle?
		AudibleSoundClass *sound_obj = m_Playlist[index];
		if ((sound_obj->Get_Miles_Handle () == NULL) &&
			 (sound_obj->Is_Sound_Culled () == false) &&
			 (sound_obj->Get_Priority () > hightest_priority))
		{
			// This is now the highest priority sound effect without
			// a play-handle.
			sound_to_get_handle = sound_obj;
			hightest_priority = sound_obj->Get_Priority ();
		}
	}

	// Get a new handle for this sound if necessary
	if (sound_to_get_handle != NULL) {
		sound_to_get_handle->Allocate_Miles_Handle ();
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::On_Frame_Update (unsigned int milliseconds)
{
	//
	// Free any sounds we completed last frame
	//
	Free_Completed_Sounds ();

	if (m_SoundScene != NULL) {
		m_SoundScene->On_Frame_Update (milliseconds);
		m_SoundScene->Collect_Logical_Sounds (milliseconds);
	}

	//
	// Loop through all the entries in the playlist
	//
	for (int index = 0; index < m_Playlist.Count (); index ++) {

		//
		// Update this sound object
		//
		AudibleSoundClass *sound_obj = m_Playlist[index];
		sound_obj->On_Frame_Update (milliseconds);
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Release_2D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Release_2D_Handles (void)
{
	MMSLockClass lock;

	// Release our hold on all the samples we've allocated
	for (int index = 0; index < m_2DSampleHandles.Count (); index ++) {
		HSAMPLE sample = m_2DSampleHandles[index];
		if (sample != NULL) {
			::AIL_release_sample_handle (sample);
		}
	}

	m_2DSampleHandles.Delete_All ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_2D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allocate_2D_Handles (void)
{
	MMSLockClass lock;

	// Start fresh
	Release_2D_Handles ();

	if (m_Driver2D != NULL) {

		// Attempt to allocate our share of 2D sample handles
		for (int index = 0; index < m_Max2DSamples; index ++) {
			HSAMPLE sample = ::AIL_allocate_sample_handle (m_Driver2D);
			if (sample != NULL) {
				::AIL_set_sample_user_data (sample, INFO_OBJECT_PTR, NULL);
				m_2DSampleHandles.Add (sample);
			}
		}

		// Record our actual number of available 2D sample handles
		m_Max2DSamples = m_2DSampleHandles.Count ();
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_2D_Sample
//
////////////////////////////////////////////////////////////////////////////////////////////
HSAMPLE
WWAudioClass::Get_2D_Sample (const AudibleSoundClass &sound_obj)
{
	if (Is_OK_To_Give_Handle (sound_obj) == false) {
		return (HSAMPLE)INVALID_MILES_HANDLE;
	}

	MMSLockClass lock;

	float lowest_priority					= sound_obj.Get_Priority ();
	float lowest_runtime_priority			= sound_obj.Get_Runtime_Priority ();
	AudibleSoundClass *lowest_pri_sound = NULL;
	HSAMPLE lowest_pri_sample				= NULL;
	HSAMPLE free_sample						= (HSAMPLE)INVALID_MILES_HANDLE;

	// Loop through all the available sample handles and try to find
	// one that isn't being used to play a sound.
	bool found = false;
	for (int index = 0; (index < m_2DSampleHandles.Count ()) && !found; index ++) {

		HSAMPLE sample = m_2DSampleHandles[index];
		if (sample != NULL) {

			// Get a pointer to the object that is currently using this sample
			AudibleSoundClass *sound_obj = (AudibleSoundClass *)::AIL_sample_user_data (sample, INFO_OBJECT_PTR);
			if (sound_obj == NULL) {

				// Return this sample handle to the caller
				free_sample = sample;
				found = true;
			} else {

				//
				//	Determine if this sound's priority is lesser then the sound we want to play.
				// This is done by comparing both the designer-specified priority and the current
				// runtime priority (which is calculated by distance to the listener).
				//
				float priority				= sound_obj->Get_Priority ();
				float runtime_priority	= sound_obj->Get_Runtime_Priority ();
				if (	(priority < lowest_priority) ||
						(priority == lowest_priority && runtime_priority <= lowest_runtime_priority))
				{
					lowest_priority			= priority;
					lowest_pri_sound			= sound_obj;
					lowest_pri_sample			= sample;
					lowest_runtime_priority = runtime_priority;
				}
			}
		}
	}

	// Steal the sample handle from the lower priority
	// sound and return the handle to the caller.
	if ((found == false) && (lowest_pri_sound != NULL)) {
		lowest_pri_sound->Free_Miles_Handle ();
		free_sample = lowest_pri_sample;
	}

	// Return the free sample handle if we found one
	return free_sample;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_3D_Sample
//
////////////////////////////////////////////////////////////////////////////////////////////////
H3DSAMPLE
WWAudioClass::Get_3D_Sample (const Sound3DClass &sound_obj)
{
	if (Is_OK_To_Give_Handle (sound_obj) == false) {
		return (H3DSAMPLE)INVALID_MILES_HANDLE;
	}

	MMSLockClass lock;

	float lowest_priority					= sound_obj.Get_Priority ();
	float lowest_runtime_priority			= sound_obj.Get_Runtime_Priority ();
	AudibleSoundClass *lowest_pri_sound = NULL;
	H3DSAMPLE lowest_pri_sample			= NULL;
	H3DSAMPLE free_sample					= (H3DSAMPLE)INVALID_MILES_HANDLE;


	// Loop through all the available sample handles and try to find
	// one that isn't being used to play a sound.
	bool found = false;
	for (int index = 0; (index < m_3DSampleHandles.Count ()) && !found; index ++) {

		H3DSAMPLE sample = m_3DSampleHandles[index];
		if (sample != NULL) {

			// Get a pointer to the object that is currently using this sample
			AudibleSoundClass *sound_obj = (AudibleSoundClass *)::AIL_3D_object_user_data (sample, INFO_OBJECT_PTR);
			if (sound_obj == NULL) {

				// Return this sample handle to the caller
				free_sample = sample;
				found = true;
			} else {

				//
				//	Determine if this sound's priority is lesser then the sound we want to play.
				// This is done by comparing both the designer-specified priority and the current
				// runtime priority (which is calculated by distance to the listener).
				//
				float priority				= sound_obj->Get_Priority ();
				float runtime_priority	= sound_obj->Get_Runtime_Priority ();
				if (	(priority < lowest_priority) ||
						(priority == lowest_priority && runtime_priority <= lowest_runtime_priority))
				{
					lowest_priority			= priority;
					lowest_pri_sound			= sound_obj;
					lowest_pri_sample			= sample;
					lowest_runtime_priority = runtime_priority;
				}
			}
		}
	}

	// Steal the sample handle from the lower priority
	// sound and return the handle to the caller.
	if ((found == false) && (lowest_pri_sound != NULL)) {
		lowest_pri_sound->Free_Miles_Handle ();
		free_sample = lowest_pri_sample;
	}

	// Return the free sample handle if we found one
	return free_sample;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Listener_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
H3DPOBJECT
WWAudioClass::Get_Listener_Handle (void)
{
	MMSLockClass lock;
	return ::AIL_3D_open_listener (m_Driver3D);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Build_3D_Driver_List
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Build_3D_Driver_List (void)
{
	MMSLockClass lock;

	HPROENUM next = HPROENUM_FIRST;
	HPROVIDER provider = NULL;
	char *name = NULL;
	while (::AIL_enumerate_3D_providers (&next, &provider, &name) > 0) {

		// Can we successfully open this provider?
		if (::AIL_open_3D_provider (provider) == M3D_NOERR) {
			DRIVER_INFO_STRUCT *info = W3DNEW DRIVER_INFO_STRUCT;
			info->driver = provider;
			info->name = ::strdup (name);
			m_Driver3DList.Add (info);
			::AIL_close_3D_provider (provider);
		} else {
			char *error_info = ::AIL_last_error ();
			WWDEBUG_SAY (("WWAudio: Unable to open %s.", name));
			WWDEBUG_SAY (("WWAudio: Reason %s.", error_info));
		}
	}

	// Attempt to select one of the known drivers (in the following order).
	if ((Select_3D_Device (DRIVER3D_A3D) == false) &&
		 (Select_3D_Device (DRIVER3D_EAX) == false) &&
		 (Select_3D_Device (DRIVER3D_D3DSOUND) == false) &&
		 (Select_3D_Device (DRIVER3D_DOLBY) == false)) {

		// Couldn't select a known driver, so just use the first possible.
		if (m_Driver3DList.Count () > 0) {
			Select_3D_Device ((int)0);
			//DRIVER_INFO_STRUCT *info = m_Driver3DList[0];
			//m_Driver3D = info->driver;
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_3D_Driver_List
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Free_3D_Driver_List (void)
{
	MMSLockClass lock;

	//
	//	Remove all the handles
	//
	Remove_3D_Sound_Handles ();
	Release_3D_Handles ();

	//
	// Loop through all the driver entries and free them all
	//
	for (int index = 0; index < m_Driver3DList.Count (); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];
		if (info != NULL) {

			//
			// Free the information we have stored with this driver
			//
			if (info->name != NULL) {
				::free (info->name);
			}
			delete info;
		}
	}

	if (m_Driver3D != NULL) {
		::AIL_close_3D_provider (m_Driver3D);
		m_Driver3D = NULL;
	}

	//
	// Clear the list
	//
	m_Driver3DList.Delete_All ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Select_3D_Device (const char *device_name)
{
	bool retval = false;

	//
	// Loop through all the drivers until we've found the one we want
	//
	for (int index = 0; index < m_Driver3DList.Count (); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];
		if (info != NULL) {

			//
			//	Is this the device we were looking for?
			//
			if (::lstrcmpi (info->name, device_name) == 0) {
				retval = Select_3D_Device (device_name, info->driver);
				break;
			}
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Select_3D_Device (const char *device_name, HPROVIDER provider)
{
	bool retval = false;
	if ((provider != NULL) && (provider != m_Driver3D)) {

		//
		//	Remove all the handles
		//
		Remove_3D_Sound_Handles ();
		Release_3D_Handles ();

		//
		// Close the previous driver if needs be
		//
		if (m_Driver3D != NULL) {
			::AIL_close_3D_provider (m_Driver3D);
			m_Driver3D = NULL;
		}

		//
		// Select this device and re-allocate all handles
		//
		if (::AIL_open_3D_provider (provider) == M3D_NOERR) {
			m_Driver3D = provider;
			m_SoundScene->Initialize ();
			Allocate_3D_Handles ();
			AIL_set_3D_speaker_type (provider, AIL_3D_2_SPEAKER);

			//
			//	Adjust the effects level to 1.0 if this is an EAX based driver
			//
			StringClass lower_name = device_name;
			::strlwr (lower_name.Peek_Buffer ());
			if (::strstr (device_name, "eax") != 0) {
				m_EffectsLevel = 1.0F;
			} else {
				m_EffectsLevel = 0.0F;
			}
		}

		retval = true;
	}

	// Return true if we successfully selected the device
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Select_3D_Device (int index)
{
	bool retval = false;

	//
	// Index valid?
	//
	if ((index >= 0) && (index < m_Driver3DList.Count ())) {
		Select_3D_Device (m_Driver3DList[index]->name, m_Driver3DList[index]->driver);
		WWDEBUG_SAY (("WWAudio: Selecting 3D sound device: %s.", m_Driver3DList[index]->name));
		retval = true;
	}

	//
	// Return true if we successfully selected the device
	//
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Select_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Select_3D_Device (DRIVER_TYPE_3D type)
{
	// Return true if we successfully selected the device
	return Select_3D_Device (Find_3D_Device (type));
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Find_3D_Device
//
////////////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioClass::Find_3D_Device (DRIVER_TYPE_3D type)
{
	// Determine which substring to search for in the
	// name of the driver.
	const char *sub_string = "RSX";
	switch (type) {
		case DRIVER3D_D3DSOUND:
			sub_string = "DirectSound";
			break;

		case DRIVER3D_EAX:
			sub_string = "EAX";
			break;

		case DRIVER3D_A3D:
			sub_string = "A3D";
			break;

		case DRIVER3D_PSEUDO:
			sub_string = "Fast";
			break;

		case DRIVER3D_DOLBY:
			sub_string = "Dolby";
			break;
	}

	// Loop through all the driver entries and free them all
	int driver_index = -1;
	for (int index = 0; (index < m_Driver3DList.Count ()) && (driver_index == -1); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];
		if ((info != NULL) && (info->name != NULL)) {

			// Is this the driver we were looking for?
			if (::strstr (info->name, sub_string) != NULL) {
				driver_index = index;
			}
		}
	}

	// Return -1 if not found, otherwise the 0 based index
	return driver_index;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_3D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allocate_3D_Handles (void)
{
	MMSLockClass lock;

	// Start fresh
	Release_3D_Handles ();

	if (m_Driver3D != NULL) {

		// Attempt to allocate our share of 3D sample handles
		for (int index = 0; index < m_Max3DSamples; index ++) {
			H3DSAMPLE sample = ::AIL_allocate_3D_sample_handle (m_Driver3D);
			if (sample != NULL) {
				::AIL_set_3D_object_user_data (sample, INFO_OBJECT_PTR, NULL);
				m_3DSampleHandles.Add (sample);
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Release_3D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Release_3D_Handles (void)
{
	MMSLockClass lock;

	//
	// Release our hold on all the samples we've allocated
	//
	for (int index = 0; index < m_3DSampleHandles.Count (); index ++) {
		H3DSAMPLE sample = m_3DSampleHandles[index];
		if (sample != NULL) {
			::AIL_release_3D_sample_handle (sample);
		}
	}

	m_3DSampleHandles.Delete_All ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Validate_3D_Sound_Buffer
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Validate_3D_Sound_Buffer (SoundBufferClass *buffer)
{
	bool retval = false;

	//
	// 3D sound buffer MUST be uncompressed mono WAV data
	//
	if ((buffer != NULL) &&
		 (buffer->Get_Channels () == 1) &&
		 (buffer->Get_Type () == WAVE_FORMAT_PCM) &&
		 (buffer->Is_Streaming () == false))
	{
		retval = true;
	}

	// Return a true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReAssign_2D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::ReAssign_2D_Handles (void)
{
	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist.Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[index];

		// If this is a 2D sound effect, then force it to 'get' a new
		// sound handle.
		if ((sound_obj->Get_Class_ID () == CLASSID_2D) ||
			 (sound_obj->Get_Class_ID () == CLASSID_PSEUDO3D) ||
			 (sound_obj->Get_Class_ID () == CLASSID_2DTRIGGER))
		{
			sound_obj->Free_Miles_Handle ();
			sound_obj->Allocate_Miles_Handle ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	ReAssign_3D_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::ReAssign_3D_Handles (void)
{
	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist.Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[index];

		// If this is a 3D sound effect, then force it to 'get' a new
		// sound handle.
		if (sound_obj->Get_Class_ID () == CLASSID_3D) {
			sound_obj->Free_Miles_Handle ();
			sound_obj->Allocate_Miles_Handle ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_2D_Sound_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Remove_2D_Sound_Handles (void)
{
	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist.Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[index];

		// If this is a 2D sound effect, then take its handle away
		if (sound_obj->Get_Class_ID () == CLASSID_2D) {
			sound_obj->Free_Miles_Handle ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Remove_3D_Sound_Handles
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Remove_3D_Sound_Handles (void)
{
	// Loop through all the entries in the playlist
	for (int index = 0; index < m_Playlist.Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[index];

		// If this is a 3D sound effect, then force take its handle away
		if (sound_obj->Get_Class_ID () == CLASSID_3D) {
			sound_obj->Free_Miles_Handle ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Sound_Effects_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Sound_Effects_Volume (float volume)
{
	m_SoundVolume = volume;
	m_SoundVolume = min (1.0F, m_SoundVolume);
	m_SoundVolume = max (0.0F, m_SoundVolume);

	// Update all the currently playing 'Sound Effects' to
	// reflect this new volume
	for (int index = 0; index < m_Playlist.Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[index];
		if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
			sound_obj->Update_Volume ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Music_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Set_Music_Volume (float volume)
{
	m_MusicVolume = volume;
	m_MusicVolume = min (1.0F, m_MusicVolume);
	m_MusicVolume = max (0.0F, m_MusicVolume);

	// Update all currently playing music to
	// reflect this new volume
	for (int index = 0; index < m_Playlist.Count (); index ++) {
		AudibleSoundClass *sound_obj = m_Playlist[index];
		if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
			sound_obj->Update_Volume ();
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Is_Disabled
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Is_Disabled (void) const
{
	static bool _firsttime = true;
	static bool _disabled = false;

	if (_firsttime) {
		_firsttime = false;

		#ifdef G_CODE_BASE
		//
		// Use command line arguement.
		//
		ArgvClass	argv;
		if (argv.Find("-NOAUDIO")) {
			_disabled = true;
		}
		#endif

		//
		//	Read the disabled key from the registry
		//
		RegistryClass registry ("SOFTWARE\\Westwood\\WWAudio");
		if (registry.Is_Valid ()) {
			if (registry.Get_Int ("Disabled", 0) == 1) {
				_disabled = true;
				WWDEBUG_SAY (("WWAudio: Audio system disabled in registry."));
			}
		}
	}

	return _disabled;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Initialize (const char *registry_subkey_name)
{
	WWMEMLOG(MEM_SOUND);

	if (Is_Disabled () == false) {

		//
		//	Initialize the audio system from the registry settings
		//
		Load_From_Registry (registry_subkey_name);

		//
		//	Grab the first (and only) filter for use with our 'tinny' effect.
		//
		HPROENUM next = HPROENUM_FIRST;
		char *name = NULL;
		if (::AIL_enumerate_filters (&next, &m_ReverbFilter, &name) == 0) {
			m_ReverbFilter = (HPROVIDER)INVALID_MILES_HANDLE;
		}
	}

	//
	//	Register the file callbacks so we can support streaming from MIX files...
	//
	::AIL_set_file_callbacks (File_Open_Callback, File_Close_Callback,
		File_Seek_Callback, File_Read_Callback);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Initialize
(
	bool stereo,
	int bits,
	int hertz
)
{
	// Open the default 2D device, then build a list of 3D
	// devices and open the default.
	if (Is_Disabled () == false) {

		Open_2D_Device (stereo, bits, hertz);
		Build_3D_Driver_List ();

		//
		//	Grab the first (and only) filter for use with our 'tinny' effect.
		//
		HPROENUM next = HPROENUM_FIRST;
		char *name = NULL;
		if (::AIL_enumerate_filters (&next, &m_ReverbFilter, &name) == 0) {
			m_ReverbFilter = (HPROVIDER)INVALID_MILES_HANDLE;
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Shutdown
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Shutdown (void)
{
	//
	// If there is a timer running, then stop the timer...
	//
	if (m_UpdateTimer != -1) {

		// Kill the timer
		::AIL_stop_timer (m_UpdateTimer);
		::AIL_release_timer_handle (m_UpdateTimer);
		m_UpdateTimer = -1;

		// Wait for the timer callback function to end
		::WaitForSingleObject (_TimerSyncEvent, 20000);
		::CloseHandle (_TimerSyncEvent);
		_TimerSyncEvent = NULL;
	}

	//
	// Stop all sounds from playing
	//
	Flush_Playlist ();
	if (m_SoundScene != NULL) {
		m_SoundScene->Flush_Scene ();
	}

	//
	// Free all our cached sound buffers
	//
	Flush_Cache ();

	//
	// Close-out our hold on any driver resources
	//
	Remove_2D_Sound_Handles ();
	Remove_3D_Sound_Handles ();
	Release_2D_Handles ();
	Release_3D_Handles ();
	Free_3D_Driver_List ();
	SAFE_DELETE (m_SoundScene);
	Close_2D_Device ();

	//
	// Shutdown Miles Sound System
	//
	::AIL_shutdown ();
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Register_EOS_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Register_EOS_Callback (LPFNEOSCALLBACK callback, DWORD user_param)
{
	m_EOSCallbackList.Add_Callback (callback, user_param);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	UnRegister_EOS_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::UnRegister_EOS_Callback (LPFNEOSCALLBACK callback)
{
	m_EOSCallbackList.Remove_Callback (callback);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Register_Text_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Register_Text_Callback (LPFNTEXTCALLBACK callback, DWORD user_param)
{
	m_TextCallbackList.Add_Callback (callback, user_param);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	UnRegister_Text_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::UnRegister_Text_Callback (LPFNTEXTCALLBACK callback)
{
	m_TextCallbackList.Remove_Callback (callback);
	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Fire_Text_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Fire_Text_Callback (AudibleSoundClass *sound_obj, const StringClass &text)
{
	if (text.Get_Length () > 0) {

		//
		//	Loop over all the text-callbacks that have been registered
		//
		for (int index = 0; index < m_TextCallbackList.Count (); index ++) {
			uint32 user_data				= 0L;
			LPFNTEXTCALLBACK callback	= m_TextCallbackList.Get_Callback (index, &user_data);
			if (callback != NULL) {

				//
				//	Fire the notification
				//
				(*callback) (sound_obj, text, user_data);
			}
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Sound_Effects
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allow_Sound_Effects (bool onoff)
{
	// Is the state changing?
	if (m_AreSoundEffectsEnabled != onoff) {
		m_AreSoundEffectsEnabled = onoff;

		// Update all the currently playing 'Sound Effects' to
		// reflect this new state.
		if (m_AreSoundEffectsEnabled) {
			for (int index = 0; index < m_Playlist.Count (); index ++) {
				AudibleSoundClass *sound_obj = m_Playlist[index];
				if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
					sound_obj->Allocate_Miles_Handle ();
				}
			}
		} else {
			for (int index = 0; index < m_Playlist.Count (); index ++) {
				AudibleSoundClass *sound_obj = m_Playlist[index];
				if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_SOUND_EFFECT) {
					sound_obj->Free_Miles_Handle ();
				}
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allow_Music
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Allow_Music (bool onoff)
{
	// Is the state changing?
	if (m_IsMusicEnabled != onoff) {
		m_IsMusicEnabled = onoff;

		// Update all the currently playing 'music tracks' to
		// reflect this new state.
		if (m_IsMusicEnabled) {
			for (int index = 0; index < m_Playlist.Count (); index ++) {
				AudibleSoundClass *sound_obj = m_Playlist[index];
				if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
					sound_obj->Stop (false);
					sound_obj->Play ();
				}
			}
		} else {
			for (int index = 0; index < m_Playlist.Count (); index ++) {
				AudibleSoundClass *sound_obj = m_Playlist[index];
				if (sound_obj->Get_Type () == AudibleSoundClass::TYPE_MUSIC) {
					sound_obj->Free_Miles_Handle ();
				}
			}
		}
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Simple_Play_2D_Sound_Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Simple_Play_2D_Sound_Effect
(
	const char *filename,
	float priority,
	float volume
)
{
	bool retval = false;
	AudibleSoundClass *sound = Create_Sound_Effect (filename);
	if (sound != NULL) {
		sound->Set_Priority (priority);
		sound->Set_Loop_Count (1);
		sound->Set_Volume(volume);
		sound->Play ();
		sound->Release_Ref ();
		sound = NULL;
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Simple_Play_2D_Sound_Effect
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Simple_Play_2D_Sound_Effect
(
	FileClass &file,
	float priority,
	float volume
)
{
	bool retval = false;
	AudibleSoundClass *sound = Create_Sound_Effect (file);
	if (sound != NULL) {
		sound->Set_Priority (priority);
		sound->Set_Loop_Count (1);
		sound->Set_Volume(volume);
		sound->Play ();
		sound->Release_Ref ();
		sound = NULL;
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_File
//
////////////////////////////////////////////////////////////////////////////////////////////////
FileClass *
WWAudioClass::Get_File (LPCTSTR filename)
{
	FileClass *file = NULL;
	if (m_FileFactory != NULL) {
		file = m_FileFactory->Get_File (filename);
	} else {
		file = _TheFileFactory->Get_File(filename);
	}

	// Return a pointer to the file
	return file;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Return_File
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Return_File (FileClass *file)
{
	if (m_FileFactory != NULL) {
		m_FileFactory->Return_File (file);
	} else {
		SAFE_DELETE (file);
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Logical_Sound
//
////////////////////////////////////////////////////////////////////////////////////////////
LogicalSoundClass *
WWAudioClass::Create_Logical_Sound (void)
{
	return W3DNEW LogicalSoundClass;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Logical_Listener
//
////////////////////////////////////////////////////////////////////////////////////////////
LogicalListenerClass *
WWAudioClass::Create_Logical_Listener (void)
{
	return W3DNEW LogicalListenerClass;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Add_Logical_Type
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Add_Logical_Type (int id, LPCTSTR display_name)
{
	m_LogicalTypes.Add (LOGICAL_TYPE_STRUCT (id, display_name));
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Reset_Logical_Types
//
////////////////////////////////////////////////////////////////////////////////////////////
void
WWAudioClass::Reset_Logical_Types (void)
{
	m_LogicalTypes.Delete_All ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Logical_Type
//
////////////////////////////////////////////////////////////////////////////////////////////
int
WWAudioClass::Get_Logical_Type (int index, StringClass &name)
{
	int type_id = 0;

	WWASSERT (index >= 0 && index < m_LogicalTypes.Count ());
	if (index >= 0 && index < m_LogicalTypes.Count ()) {
		type_id	= m_LogicalTypes[index].id;
		name		= m_LogicalTypes[index].display_name;
	}

	return type_id;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Find_Sound_Object
//
////////////////////////////////////////////////////////////////////////////////////////////
SoundSceneObjClass *
WWAudioClass::Find_Sound_Object (uint32 sound_obj_id)
{
	SoundSceneObjClass *sound_obj = NULL;

	//
	//	Lookup the sound object and return it to the caller
	//
	int index = 0;
	if (SoundSceneObjClass::Find_Sound_Object (sound_obj_id, &index)) {
		sound_obj = SoundSceneObjClass::m_GlobalSoundList[index];
	}

	return sound_obj;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Load_From_Registry (const char *subkey_name)
{
	bool retval = true;

	StringClass device_name;
	bool is_stereo = true;
	int bits = 16;
	int hertz = 44100;

	//
	//	Load the settings from the registry
	//
	if (Load_From_Registry (subkey_name, device_name, is_stereo, bits, hertz,
			m_AreSoundEffectsEnabled, m_IsMusicEnabled, m_SoundVolume, m_MusicVolume))
	{
		//
		//	Close any open devices
		//
		Free_3D_Driver_List ();
		Close_2D_Device ();

		//
		//	Open the 2D device as specified
		//
		Open_2D_Device (is_stereo, bits, hertz);

		//
		//	Find and open the 3D device specified
		//
		Build_3D_Driver_List ();
		Select_3D_Device (device_name);
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Load_From_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Load_From_Registry
(
	const char *	subkey_name,
	StringClass &	device_name,
	bool &			is_stereo,
	int &				bits,
	int &				hertz,
	bool &			sound_enabled,
	bool &			music_enabled,
	float &			sound_volume,
	float &			music_volume
)
{
	bool retval = false;

	//
	//	Attempt to open the registry key
	//
	RegistryClass registry (subkey_name);
	if (registry.Is_Valid ()) {

		//
		//	Read the device name into a string object
		//
		char temp_buffer[256] = { 0 };
		registry.Get_String (VALUE_NAME_DEVICE_NAME, temp_buffer, sizeof (temp_buffer));
		device_name = temp_buffer;

		//
		//	Read the 2D settings
		//
		is_stereo	= (registry.Get_Int (VALUE_NAME_IS_STEREO, true) == 1);
		bits			= registry.Get_Int (VALUE_NAME_BITS, 16);
		hertz			= registry.Get_Int (VALUE_NAME_HERTZ, 44100);

		//
		//	Read the sound/music enabled settings
		//
		music_enabled	= (registry.Get_Int (VALUE_NAME_MUSIC_ENABLED, 1) == 1);
		sound_enabled	= (registry.Get_Int (VALUE_NAME_SOUND_ENABLED, 1) == 1);

		//
		//	Read the volume information
		//
		music_volume	= registry.Get_Int (VALUE_NAME_MUSIC_VOL, 100) / 100.0F;
		sound_volume	= registry.Get_Int (VALUE_NAME_SOUND_VOL, 100) / 100.0F;
		music_volume	= WWMath::Clamp (music_volume, 0, 1.0F);
		sound_volume	= WWMath::Clamp (sound_volume, 0, 1.0F);

		retval		= true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_To_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Save_To_Registry (const char *subkey_name)
{
	StringClass device_name;

	//
	// Get the name of the current 3D driver
	//
	for (int index = 0; index < m_Driver3DList.Count (); index ++) {
		DRIVER_INFO_STRUCT *info = m_Driver3DList[index];

		//
		//	Is this the device we were looking for?
		//
		if (info != NULL && info->driver == m_Driver3D) {
			device_name = info->name;
			break;
		}
	}

	//
	//	Save these settings to the registry
	//
	return Save_To_Registry (subkey_name, device_name, m_PlaybackStereo, m_PlaybackBits, m_PlaybackRate,
				m_AreSoundEffectsEnabled, m_IsMusicEnabled, m_SoundVolume, m_MusicVolume);
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_To_Registry
//
////////////////////////////////////////////////////////////////////////////////////////////
bool
WWAudioClass::Save_To_Registry
(
	const char *			subkey_name,
	const StringClass &	device_name,
	bool						is_stereo,
	int						bits,
	int						hertz,
	bool						sound_enabled,
	bool						music_enabled,
	float						sound_volume,
	float						music_volume

)
{
	bool retval = false;

	//
	//	Attempt to open the registry key
	//
	RegistryClass registry (subkey_name);
	if (registry.Is_Valid ()) {

		//
		//	Save the settings to the registry
		//
		registry.Set_String (VALUE_NAME_DEVICE_NAME, device_name);
		registry.Set_Int (VALUE_NAME_IS_STEREO, is_stereo);
		registry.Set_Int (VALUE_NAME_BITS, bits);
		registry.Set_Int (VALUE_NAME_HERTZ, hertz);
		registry.Set_Int (VALUE_NAME_MUSIC_ENABLED,	music_enabled);
		registry.Set_Int (VALUE_NAME_SOUND_ENABLED,	sound_enabled);
		registry.Set_Int (VALUE_NAME_MUSIC_VOL,		music_volume * 100);
		registry.Set_Int (VALUE_NAME_SOUND_VOL,		sound_volume * 100);

		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Open_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
U32 AILCALLBACK
WWAudioClass::File_Open_Callback (char const *filename, void **file_handle)
{
	U32 retval = false;

	if (Get_Instance () != NULL) {

		//
		//	Open the file
		//
		FileClass *file = Get_Instance ()->Get_File (filename);
		if (file != NULL && file->Open ()) {
			(*file_handle) = (void *)file;
			retval = true;
		}
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Close_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
void AILCALLBACK
WWAudioClass::File_Close_Callback (void *file_handle)
{
	if (Get_Instance () != NULL) {

		//
		//	Close the file (if necessary)
		//
		FileClass *file = reinterpret_cast<FileClass *> (file_handle);
		if (file != NULL) {
			Get_Instance ()->Return_File (file);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Seek_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
S32 AILCALLBACK
WWAudioClass::File_Seek_Callback (void *file_handle, S32 offset, U32 type)
{
	S32 retval = 0;

	//
	//	Convert the handle to a file handle type
	//
	FileClass *file = reinterpret_cast<FileClass *> (file_handle);
	if (file != NULL) {

		//
		//	Convert the Miles seek type to one of our own
		//
		int seek_type = SEEK_CUR;
		switch (type)
		{
			case AIL_FILE_SEEK_BEGIN:
				seek_type = SEEK_SET;
				break;

			case AIL_FILE_SEEK_CURRENT:
				seek_type = SEEK_CUR;
				break;

			case AIL_FILE_SEEK_END:
				seek_type = SEEK_END;
				break;
		}

		//
		//	Perform the seek
		//
		retval = file->Seek (offset, seek_type);
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////
//
//	File_Read_Callback
//
////////////////////////////////////////////////////////////////////////////////////////////
U32 AILCALLBACK
WWAudioClass::File_Read_Callback (void *file_handle, void *buffer, U32 bytes)
{
	U32 retval = 0;

	//
	//	Convert the handle to a file handle type
	//
	FileClass *file = reinterpret_cast<FileClass *> (file_handle);
	if (file != NULL) {

		//
		//	Read the bytes from the file
		//
		retval = file->Read (buffer, bytes);
	}

	return retval;
}
