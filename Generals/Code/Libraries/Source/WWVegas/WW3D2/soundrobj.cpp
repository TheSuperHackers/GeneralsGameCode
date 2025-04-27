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

#include "AudibleSound.h"
#include "Sound3D.h"
#include "WWAudio.h"
#include "ffactory.h"
#include "WWFILE.H"
#include "chunkio.h"
#include "scene.h"


//////////////////////////////////////////////////////////////////////////////////
//	Global variables
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjLoaderClass		_SoundRenderObjLoader;


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjClass::SoundRenderObjClass (void)
	:	Flags (FLAG_STOP_WHEN_HIDDEN),
		Sound (NULL),
		IsInitialized (false)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjClass::SoundRenderObjClass (const SoundRenderObjClass &src)
	:	Flags (FLAG_STOP_WHEN_HIDDEN),
		Sound (NULL),
		IsInitialized (false)
{
	(*this) = src;
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~SoundRenderObjClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjClass::~SoundRenderObjClass (void)
{
	//
	//	Remove the old sound from the world (if necessary)
	//
	if (Sound != NULL) {
		Sound->Attach_To_Object (NULL);
		Sound->Remove_From_Scene ();
		REF_PTR_RELEASE (Sound);
	}

	return ;
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
	//	Create a definition from the src sound object
	//
	AudibleSoundDefinitionClass definition;
	definition.Initialize_From_Sound (src.Sound);

	//
	//	Create the internal sound object from the definition
	//
	Set_Sound (&definition);

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
SoundRenderObjClass::Set_Sound (AudibleSoundDefinitionClass *definition)
{
	//
	//	Remove the old sound from the world (if necessary)
	//
	if (Sound != NULL) {
		Sound->Remove_From_Scene ();
		REF_PTR_RELEASE (Sound);
	}

	//
	//	Create the sound object from its definition
	//
	if (definition != NULL) {
		Sound = (AudibleSoundClass *)definition->Create ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::On_Frame_Update (void)
{
	//
	//	Stop the sound from playing (if necessary)
	//
	if (	Sound != NULL &&
			Sound->Is_In_Scene () &&
			Sound->Is_Playing () == false)
	{
		Sound->Attach_To_Object (NULL);
		Sound->Remove_From_Scene ();
	}

	return ;
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
		Update_On_Visibilty ();		
	}

	return ;
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
		Update_On_Visibilty ();
	}

	return ;
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
		Update_On_Visibilty ();
	}

	return ;
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
		Update_On_Visibilty ();
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Update_On_Visibilty
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjClass::Update_On_Visibilty (void)
{
	if (Sound == NULL) {
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
			Peek_Scene () != NULL)
	{		
		//
		//	Make sure the sound is properly attached to this render
		// object and then add it to the scene
		//
		Sound->Attach_To_Object (this);
		Sound->Add_To_Scene (true);

	} else if ((Is_Not_Hidden_At_All () == false) || (Peek_Scene () == NULL)) {
		
		//
		//	Remove the sound from the scene (it will stop playing)
		//
		if ((Flags & FLAG_STOP_WHEN_HIDDEN) != 0 || (Peek_Scene () == NULL)) {
			Sound->Attach_To_Object (NULL);
			Sound->Remove_From_Scene ();
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Sound
//
//////////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
SoundRenderObjClass::Get_Sound (void) const
{
	if (Sound != NULL) {
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

	return ;
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

	Update_On_Visibilty ();
	return ;
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

	Update_On_Visibilty ();
	return ;
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
		Update_On_Visibilty ();
	}

	return ;
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
		Update_On_Visibilty ();
	}

	return ;
}


//*********************************************************************************
//
//	Start of SoundRenderObjDefClass
//
//*********************************************************************************


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::SoundRenderObjDefClass (void)
	:	Version (W3D_CURRENT_SOUNDROBJ_VERSION),
		Flags (SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::SoundRenderObjDefClass (SoundRenderObjClass &render_obj)
	:	Version (W3D_CURRENT_SOUNDROBJ_VERSION),
		Flags (SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN)
{
	Initialize (render_obj);
	return ;
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
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
SoundRenderObjDefClass::~SoundRenderObjDefClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
//////////////////////////////////////////////////////////////////////////////////
const SoundRenderObjDefClass &
SoundRenderObjDefClass::operator= (const SoundRenderObjDefClass &src)
{
	Definition	= src.Definition;
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
SoundRenderObjDefClass::Create (void)
{
	//
	//	Allocate and initialize a new instance
	//
	SoundRenderObjClass *render_obj = W3DNEW SoundRenderObjClass;
	render_obj->Set_Name (Name);
	render_obj->Set_Sound (&Definition);
	render_obj->Set_Flags (Flags);

	return render_obj;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Initialize
//
//////////////////////////////////////////////////////////////////////////////////
void
SoundRenderObjDefClass::Initialize (SoundRenderObjClass &render_obj)
{
	//
	//	Copy the settings from the sound object into our definition
	//
	Definition.Initialize_From_Sound (render_obj.Peek_Sound ());

	//
	//	Copy the flags from the render object
	//
	Flags = (SoundRenderObjClass::FLAGS)render_obj.Get_Flags ();

	//
	//	Copy the name
	//
	Name = render_obj.Get_Name ();
	return ;
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
		//
		//	Ask the definition to load its settings from the chunk
		//
		if (Definition.Load (cload)) {
			retval = WW3D_ERROR_OK;
		}		
		
		cload.Close_Chunk ();		
	}

	return retval;
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
		if (Definition.Save (csave)) {
			retval = WW3D_ERROR_OK;
		}
		csave.End_Chunk ();
	}

	return retval;
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
	SoundRenderObjPrototypeClass *prototype = NULL;

	//
	// Create a definition object
	//
	SoundRenderObjDefClass *definition = W3DNEW SoundRenderObjDefClass;
	if (definition != NULL) {
		
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


