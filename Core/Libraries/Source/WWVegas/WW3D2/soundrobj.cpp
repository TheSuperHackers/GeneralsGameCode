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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/soundrobj.cpp                          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/16/02 10:21a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "soundrobj.h"

#if noWWAUDIO //(gth) removing dependency on wwaudio

#include "WWAudio/AudibleSound.h"
#include "WWAudio/Sound3D.h"
#include "WWAudio/WWAudio.h"
#include "WWLib/ffactory.h"
#include "WWLib/WWFILE.h"
#include "WWLib/chunkio.h"
#include "scene.h"


//////////////////////////////////////////////////////////////////////////////////
//	Global variables
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjLoaderClass		_SoundRenderObjLoader;


//////////////////////////////////////////////////////////////////////////////////
//	Save/Load constants of W3D_CHUNK_SOUNDROBJ_DEFINITION
//////////////////////////////////////////////////////////////////////////////////
namespace SOUND_DEFINITION_SAVELOAD
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

	//
	//	CHUNKID_BASE_CLASS holds the ID and name that the sound had in Renegade's definition database.
	//
	enum
	{
		CHUNKID_BASE_VARIABLES	= 0x00000100,
	};

	enum
	{
		VARID_BASE_INSTANCEID	= 0x01,
		VARID_BASE_XXX_PARENTID,
		VARID_BASE_NAME,
	};
}


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjClass::SoundRenderObjClass ()
	:	Flags (FLAG_STOP_WHEN_HIDDEN),
		Sound (nullptr),
		IsInitialized (false)
{
}


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjClass::SoundRenderObjClass (const SoundRenderObjClass &src)
	:	Flags (FLAG_STOP_WHEN_HIDDEN),
		Sound (nullptr),
		IsInitialized (false)
{
	(*this) = src;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~SoundRenderObjClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjClass::~SoundRenderObjClass ()
{
	//
	//	Remove the old sound from the world (if necessary)
	//
	if (Sound != nullptr) {
		Sound->Attach_To_Object (nullptr);
		Sound->Remove_From_Scene ();
		REF_PTR_RELEASE (Sound);
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////////////////////////////
const SoundRenderObjClass &
SoundRenderObjClass::operator= (const SoundRenderObjClass &src)
{
	//
	//	Create the internal sound object from the settings of the src sound object
	//
	SoundRenderObjDefClass *definition = NEW_REF (SoundRenderObjDefClass, (src));
	AudibleSoundClass *sound = definition->Create_Sound ();
	Set_Sound (sound);
	REF_PTR_RELEASE (sound);
	REF_PTR_RELEASE (definition);

	//
	//	Copy the other misc settings
	//
	Name = src.Name;
	return *this;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_Sound
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Sound (AudibleSoundClass *sound)
{
	//
	//	Remove the old sound from the world (if necessary)
	//
	if (Sound != nullptr) {
		Sound->Remove_From_Scene ();
	}

	REF_PTR_SET (Sound, sound);
}


//////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::On_Frame_Update ()
{
	//
	//	Stop the sound from playing (if necessary)
	//
	if (	Sound != nullptr &&
			Sound->Is_In_Scene () &&
			Sound->Is_Playing () == false)
	{
		Sound->Attach_To_Object (nullptr);
		Sound->Remove_From_Scene ();
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_Hidden
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Hidden (int onoff)
{
	int before = Is_Not_Hidden_At_All ();
	RenderObjClass::Set_Hidden (onoff);

	//
	//	If we've changed state, then update the visibility
	//
	if (IsInitialized == false || Is_Not_Hidden_At_All () != before) {
		IsInitialized = true;
		Update_On_Visibility ();
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_Visible
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Visible (int onoff)
{
	int before = Is_Not_Hidden_At_All ();
	RenderObjClass::Set_Visible (onoff);

	//
	//	If we've changed state, then update the visibility
	//
	if (IsInitialized == false || Is_Not_Hidden_At_All () != before) {
		IsInitialized = true;
		Update_On_Visibility ();
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_Animation_Hidden
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Animation_Hidden (int onoff)
{
	int before = Is_Not_Hidden_At_All ();
	RenderObjClass::Set_Animation_Hidden (onoff);

	//
	//	If we've changed state, then update the visibility
	//
	if (IsInitialized == false || Is_Not_Hidden_At_All () != before) {
		IsInitialized = true;
		Update_On_Visibility ();
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_Force_Visible
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Force_Visible (int onoff)
{
	int before = Is_Not_Hidden_At_All ();
	RenderObjClass::Set_Force_Visible (onoff);

	//
	//	If we've changed state, then update the visibility
	//
	if (IsInitialized == false || Is_Not_Hidden_At_All () != before) {
		IsInitialized = true;
		Update_On_Visibility ();
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Update_On_Visibility
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Update_On_Visibility ()
{
	if (Sound == nullptr) {
		return ;
	}

	//
	//	Ensure our transform is correct
	//
	Validate_Transform ();

	//
	// Either add the sound object to the sound scene
	// or remove it from the sound scene depending
	// on the visibility state of the render object.
	//
	if (	Is_Not_Hidden_At_All () &&
			Sound->Is_In_Scene () == false &&
			Peek_Scene () != nullptr)
	{
		//
		//	Make sure the sound is properly attached to this render
		// object and then add it to the scene
		//
		Sound->Attach_To_Object (this);
		Sound->Add_To_Scene (true);

	} else if ((Is_Not_Hidden_At_All () == false) || (Peek_Scene () == nullptr)) {

		//
		//	Remove the sound from the scene (it will stop playing)
		//
		if ((Flags & FLAG_STOP_WHEN_HIDDEN) != 0 || (Peek_Scene () == nullptr)) {
			Sound->Attach_To_Object (nullptr);
			Sound->Remove_From_Scene ();
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Sound
//
//////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
SoundRenderObjClass::Get_Sound () const
{
	if (Sound != nullptr) {
		Sound->Add_Ref ();
	}

	return Sound;
}


//////////////////////////////////////////////////////////////////////////////
//
//	Set_Flag
//
//////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Flag (uint32 flag, bool onoff)
{
	Flags &= ~flag;
	if (onoff) {
		Flags |= flag;
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Notify_Added
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Notify_Added (SceneClass *scene)
{
	RenderObjClass::Notify_Added (scene);
	scene->Register (this, SceneClass::ON_FRAME_UPDATE);

	Update_On_Visibility ();
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Notify_Removed
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Notify_Removed (SceneClass *scene)
{
	scene->Unregister (this, SceneClass::ON_FRAME_UPDATE);
	RenderObjClass::Notify_Removed (scene);

	Update_On_Visibility ();
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_Transform
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Transform (const Matrix3D &tm)
{
	RenderObjClass::Set_Transform (tm);

	if (IsInitialized == false) {
		IsInitialized = true;
		Update_On_Visibility ();
	}
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_Position
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Set_Position (const Vector3 &pos)
{
	RenderObjClass::Set_Position (pos);

	if (IsInitialized == false) {
		IsInitialized = true;
		Update_On_Visibility ();
	}
}


//*********************************************************************************
//
//	Start of SoundRenderObjDefClass
//
//*********************************************************************************


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundSettingsStruct
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::SoundSettingsStruct::SoundSettingsStruct ()
	:	Priority (0.5F),
		Volume (1.0F),
		Pan (0.5F),
		LoopCount (1),
		DropOffRadius (40.0F),
		MaxVolRadius (20.0F),
		Is3D (true),
		Type (AudibleSoundClass::TYPE_SOUND_EFFECT),
		StartOffset (0),
		PitchFactor (1.0F),
		LogicalTypeMask (0),
		LogicalNotifyDelay (2),
		LogicalDropOffRadius (-1.0F),
		CreateLogical (false),
		AttenuationSphereColor (0, 0.75F, 0.75F),
		DefinitionID (0)
{
}


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::SoundRenderObjDefClass ()
	:	Version (W3D_CURRENT_SOUNDROBJ_VERSION),
		Flags (SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN)
{
}


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::SoundRenderObjDefClass (const SoundRenderObjClass &render_obj)
	:	Version (W3D_CURRENT_SOUNDROBJ_VERSION),
		Flags (SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN)
{
	Initialize (render_obj);
}


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::SoundRenderObjDefClass (const SoundRenderObjDefClass &src)
	:	Version (W3D_CURRENT_SOUNDROBJ_VERSION),
		Flags (SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN)
{
	(*this) = src;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::~SoundRenderObjDefClass ()
{
}


//////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////////////////////////////
const SoundRenderObjDefClass &
SoundRenderObjDefClass::operator= (const SoundRenderObjDefClass &src)
{
	Settings		= src.Settings;
	Version		= src.Version;
	Name			= src.Name;
	return (*this);
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create
//
//////////////////////////////////////////////////////////////////////////////////
RenderObjClass *
SoundRenderObjDefClass::Create ()
{
	//
	//	Allocate and initialize a new instance
	//
	SoundRenderObjClass *render_obj = W3DNEW SoundRenderObjClass;
	render_obj->Set_Name (Name);

	AudibleSoundClass *sound = Create_Sound ();
	render_obj->Set_Sound (sound);
	REF_PTR_RELEASE (sound);

	render_obj->Set_Flags (Flags);

	return render_obj;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Create_Sound
//
//////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
SoundRenderObjDefClass::Create_Sound () const
{
	AudibleSoundClass *new_sound = nullptr;

	//
	//	If this is a relative path, strip it off and assume
	// the current directory is set correctly.
	//
	StringClass real_filename(Settings.Filename,true);
	const char *dir_delimiter = ::strrchr (Settings.Filename, '\\');
	if (dir_delimiter != nullptr && Settings.Filename.Get_Length () > 2 && Settings.Filename[1] != ':') {
		real_filename = (dir_delimiter + 1);
	}

	//
	//	Should we create a 2D or 3D sound?
	//
	if (Settings.Is3D) {
		new_sound = WWAudioClass::Get_Instance ()->Create_3D_Sound (real_filename, CLASSID_3D);
	} else {
		new_sound = WWAudioClass::Get_Instance ()->Create_Sound_Effect (real_filename);
	}

	//
	//	Did we successfully create the sound?
	//
	if (new_sound != nullptr) {

		//
		//	Configure the sound
		//
		new_sound->Set_Type ((AudibleSoundClass::SOUND_TYPE)Settings.Type);
		new_sound->Set_Priority (Settings.Priority);
		new_sound->Set_Volume (Settings.Volume);
		new_sound->Set_Loop_Count (Settings.LoopCount);
		new_sound->Set_DropOff_Radius (Settings.DropOffRadius);
		new_sound->Set_Start_Offset (Settings.StartOffset);
		new_sound->Set_Pitch_Factor (Settings.PitchFactor);

		if (Settings.Is3D) {
			((Sound3DClass *)new_sound)->Set_Max_Vol_Radius (Settings.MaxVolRadius);
		}
	}

	return new_sound;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjDefClass::Initialize (const SoundRenderObjClass &render_obj)
{
	//
	// Read the settings from the sound object
	//
	AudibleSoundClass *sound = render_obj.Peek_Sound ();
	if (sound != nullptr) {
		Sound3DClass *sound_3d	= sound->As_Sound3DClass ();

		//
		//	Choose defaults for the values that we can't get from
		// the sound.
		//
		Settings.LogicalDropOffRadius	= -1.0F;
		Settings.LogicalNotifyDelay		= 2;
		Settings.LogicalTypeMask			= 0;
		Settings.CreateLogical				= false;
		Settings.Pan							= 0.5F;
		Settings.DisplayText				= "";

		//
		//	Copy the values that we can from the sound object
		//
		Settings.Filename			= sound->Get_Filename ();
		Settings.DropOffRadius	= sound->Get_DropOff_Radius ();
		Settings.Priority			= sound->Peek_Priority ();
		Settings.Is3D				= (sound_3d != nullptr);
		Settings.Type				= sound->Get_Type ();
		Settings.LoopCount			= sound->Get_Loop_Count ();
		Settings.Volume				= sound->Get_Volume ();
		Settings.StartOffset		= sound->Get_Start_Offset ();
		Settings.PitchFactor		= sound->Get_Pitch_Factor ();

		if (sound_3d != nullptr) {
			Settings.MaxVolRadius = sound_3d->Get_Max_Vol_Radius ();
		}
	}

	//
	//	Copy the flags from the render object
	//
	Flags = (SoundRenderObjClass::FLAGS)render_obj.Get_Flags ();

	//
	//	Copy the name
	//
	Name = render_obj.Get_Name ();
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
///////////////////////////////////////////////////////////////////////////////////
WW3DErrorType
SoundRenderObjDefClass::Load_W3D (ChunkLoadClass &cload)
{
	WW3DErrorType retval = WW3D_ERROR_LOAD_FAILED;

	//
	// Attempt to read the different sections of the definition
	//
	if ((Read_Header (cload) == WW3D_ERROR_OK) &&
		 (Read_Definition (cload) == WW3D_ERROR_OK))
	{
		retval = WW3D_ERROR_OK;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_W3D
//
//////////////////////////////////////////////////////////////////////////////////
WW3DErrorType
SoundRenderObjDefClass::Save_W3D (ChunkSaveClass &csave)
{
	WW3DErrorType retval = WW3D_ERROR_SAVE_FAILED;

	//
	// Begin a chunk that identifies a sound render object
	//
	if (csave.Begin_Chunk (W3D_CHUNK_SOUNDROBJ) == TRUE) {

		//
		// Attempt to save the different sections of the aggregate definition
		//
		if ((Write_Header (csave) == WW3D_ERROR_OK) &&
			 (Write_Definition (csave) == WW3D_ERROR_OK))
		{
			retval = WW3D_ERROR_OK;
		}

		csave.End_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Header
//
///////////////////////////////////////////////////////////////////////////////////
WW3DErrorType
SoundRenderObjDefClass::Read_Header (ChunkLoadClass &cload)
{
	WW3DErrorType retval = WW3D_ERROR_LOAD_FAILED;

	//
	// Is this the header chunk?
	//
	if (cload.Open_Chunk () &&
	    (cload.Cur_Chunk_ID () == W3D_CHUNK_SOUNDROBJ_HEADER))
	{
		//
		//	Read the header from the chunk
		//
		W3dSoundRObjHeaderStruct header = { 0 };
		if (cload.Read (&header, sizeof (header)) == sizeof (header)) {

			//
			// Copy the names from the header structure
			//
			Name			= header.Name;
			Version		= header.Version;
			Flags			= (SoundRenderObjClass::FLAGS)header.Flags;
			retval		= WW3D_ERROR_OK;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Definition
//
///////////////////////////////////////////////////////////////////////////////////
WW3DErrorType
SoundRenderObjDefClass::Read_Definition (ChunkLoadClass &cload)
{
	WW3DErrorType retval = WW3D_ERROR_LOAD_FAILED;

	//
	// Is this the right chunk?
	//
	if (cload.Open_Chunk () &&
	    (cload.Cur_Chunk_ID () == W3D_CHUNK_SOUNDROBJ_DEFINITION))
	{
		using namespace SOUND_DEFINITION_SAVELOAD;

		while (cload.Open_Chunk ()) {
			switch (cload.Cur_Chunk_ID ()) {

				case CHUNKID_VARIABLES:
					Read_Settings (cload);
					break;

				case CHUNKID_BASE_CLASS:
					Read_Base_Class (cload);
					break;
			}

			cload.Close_Chunk ();
		}

		retval = WW3D_ERROR_OK;
		cload.Close_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Settings
//
///////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjDefClass::Read_Settings (ChunkLoadClass &cload)
{
	using namespace SOUND_DEFINITION_SAVELOAD;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_PRIORITY,					Settings.Priority)
			READ_MICRO_CHUNK (cload, VARID_VOLUME,						Settings.Volume)
			READ_MICRO_CHUNK (cload, VARID_PAN,							Settings.Pan)
			READ_MICRO_CHUNK (cload, VARID_LOOP_COUNT,				Settings.LoopCount)
			READ_MICRO_CHUNK (cload, VARID_DROP_OFF,					Settings.DropOffRadius)
			READ_MICRO_CHUNK (cload, VARID_MAX_VOL,					Settings.MaxVolRadius)
			READ_MICRO_CHUNK (cload, VARID_TYPE,						Settings.Type)
			READ_MICRO_CHUNK (cload, VARID_IS3D,						Settings.Is3D)
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_FILENAME,		Settings.Filename)
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_DISPLAY_TEXT,	Settings.DisplayText)
			READ_MICRO_CHUNK (cload, VARID_LOGICAL_MASK,				Settings.LogicalTypeMask)
			READ_MICRO_CHUNK (cload, VARID_LOGICAL_DELAY,			Settings.LogicalNotifyDelay)
			READ_MICRO_CHUNK (cload, VARID_CREATE_LOGICAL,			Settings.CreateLogical)
			READ_MICRO_CHUNK (cload, VARID_LOGICAL_DROP_OFF,		Settings.LogicalDropOffRadius)
			READ_MICRO_CHUNK (cload, VARID_SPHERE_COLOR,				Settings.AttenuationSphereColor)
			READ_MICRO_CHUNK (cload, VARID_START_OFFSET,				Settings.StartOffset);
			READ_MICRO_CHUNK (cload, VARID_PITCH_FACTOR,				Settings.PitchFactor);
		}

		cload.Close_Micro_Chunk ();
	}
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Read_Base_Class
//
///////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjDefClass::Read_Base_Class (ChunkLoadClass &cload)
{
	using namespace SOUND_DEFINITION_SAVELOAD;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_BASE_VARIABLES:
				while (cload.Open_Micro_Chunk ()) {
					switch (cload.Cur_Micro_Chunk_ID ()) {

						READ_MICRO_CHUNK (cload, VARID_BASE_INSTANCEID, Settings.DefinitionID)
						READ_MICRO_CHUNK_WWSTRING (cload, VARID_BASE_NAME, Settings.DefinitionName)
					}

					cload.Close_Micro_Chunk ();
				}
				break;
		}

		cload.Close_Chunk ();
	}
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Write_Header
//
///////////////////////////////////////////////////////////////////////////////////
WW3DErrorType
SoundRenderObjDefClass::Write_Header (ChunkSaveClass &csave)
{
	WW3DErrorType retval = WW3D_ERROR_SAVE_FAILED;

	//
	// Begin a chunk that identifies the aggregate
	//
	if (csave.Begin_Chunk (W3D_CHUNK_SOUNDROBJ_HEADER) == TRUE) {

		//
		// Fill the header structure
		//
		W3dSoundRObjHeaderStruct header = { 0 };
		header.Version	= W3D_CURRENT_AGGREGATE_VERSION;
		header.Flags	= Flags;
		::lstrcpyn (header.Name, (const char *)Name, sizeof (header.Name));
		header.Name[sizeof (header.Name) - 1] = 0;

		//
		// Write the header out to the chunk
		//
		if (csave.Write (&header, sizeof (header)) == sizeof (header)) {
			retval = WW3D_ERROR_OK;
		}

		// End the header chunk
		csave.End_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Write_Definition
//
///////////////////////////////////////////////////////////////////////////////////
WW3DErrorType
SoundRenderObjDefClass::Write_Definition (ChunkSaveClass &csave)
{
	WW3DErrorType retval = WW3D_ERROR_SAVE_FAILED;

	//
	// Save the definition to its own chunk
	//
	if (csave.Begin_Chunk (W3D_CHUNK_SOUNDROBJ_DEFINITION) == TRUE) {
		using namespace SOUND_DEFINITION_SAVELOAD;

		csave.Begin_Chunk (CHUNKID_VARIABLES);
		Write_Settings (csave);
		csave.End_Chunk ();

		csave.Begin_Chunk (CHUNKID_BASE_CLASS);
		Write_Base_Class (csave);
		csave.End_Chunk ();

		retval = WW3D_ERROR_OK;
		csave.End_Chunk ();
	}

	return retval;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Write_Settings
//
///////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjDefClass::Write_Settings (ChunkSaveClass &csave)
{
	using namespace SOUND_DEFINITION_SAVELOAD;

	//
	//	Save the audible variables
	//
	WRITE_MICRO_CHUNK (csave, VARID_PRIORITY,						Settings.Priority)
	WRITE_MICRO_CHUNK (csave, VARID_VOLUME,						Settings.Volume)
	WRITE_MICRO_CHUNK (csave, VARID_PAN,							Settings.Pan)
	WRITE_MICRO_CHUNK (csave, VARID_LOOP_COUNT,					Settings.LoopCount)
	WRITE_MICRO_CHUNK (csave, VARID_DROP_OFF,						Settings.DropOffRadius)
	WRITE_MICRO_CHUNK (csave, VARID_MAX_VOL,						Settings.MaxVolRadius)
	WRITE_MICRO_CHUNK (csave, VARID_TYPE,							Settings.Type)
	WRITE_MICRO_CHUNK (csave, VARID_IS3D,							Settings.Is3D)
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_FILENAME,			Settings.Filename)
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_DISPLAY_TEXT,	Settings.DisplayText)
	WRITE_MICRO_CHUNK (csave, VARID_START_OFFSET,				Settings.StartOffset);
	WRITE_MICRO_CHUNK (csave, VARID_PITCH_FACTOR,				Settings.PitchFactor);

	//
	//	Save the logical variables
	//
	WRITE_MICRO_CHUNK (csave, VARID_LOGICAL_MASK,				Settings.LogicalTypeMask)
	WRITE_MICRO_CHUNK (csave, VARID_LOGICAL_DELAY,				Settings.LogicalNotifyDelay)
	WRITE_MICRO_CHUNK (csave, VARID_CREATE_LOGICAL,				Settings.CreateLogical)
	WRITE_MICRO_CHUNK (csave, VARID_LOGICAL_DROP_OFF,			Settings.LogicalDropOffRadius)
	WRITE_MICRO_CHUNK (csave, VARID_SPHERE_COLOR,				Settings.AttenuationSphereColor)
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Write_Base_Class
//
///////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjDefClass::Write_Base_Class (ChunkSaveClass &csave)
{
	using namespace SOUND_DEFINITION_SAVELOAD;

	csave.Begin_Chunk (CHUNKID_BASE_VARIABLES);
	WRITE_MICRO_CHUNK (csave, VARID_BASE_INSTANCEID, Settings.DefinitionID);
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_BASE_NAME, Settings.DefinitionName);
	csave.End_Chunk ();
}


//*********************************************************************************
//
//	Start of SoundRenderObjDefClass
//
//*********************************************************************************


///////////////////////////////////////////////////////////////////////////////////
//
//	Load_W3D
//
///////////////////////////////////////////////////////////////////////////////////
PrototypeClass *
SoundRenderObjLoaderClass::Load_W3D (ChunkLoadClass &cload)
{
	SoundRenderObjPrototypeClass *prototype = nullptr;

	//
	// Create a definition object
	//
	SoundRenderObjDefClass *definition = W3DNEW SoundRenderObjDefClass;
	if (definition != nullptr) {

		//
		// Ask the definition object to load the sound data
		//
		if (definition->Load_W3D (cload) == WW3D_ERROR_OK) {

			//
			// Success!  Create a prototype from the definition
			//
			prototype = W3DNEW SoundRenderObjPrototypeClass (definition);
		}

		REF_PTR_RELEASE (definition);
	}

	return prototype;
}

#endif // noWWAUDIO


