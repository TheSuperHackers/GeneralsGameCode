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
 *                     $Archive:: /Commando/Code/ww3d2/soundrobj.h                            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 5:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

// TheSuperHackers @build xezon 05/04/2025 Compile in WWAUDIO for Renegade's w3dview tool.
#define noWWAUDIO 1

#if noWWAUDIO // (gth) removing dependency on WWAUDIO

#include "rendobj.h"
#include "wwstring.h"
#include "proto.h"
#include "w3d_file.h"
#include "w3derr.h"
#include "AudibleSound.h"


//////////////////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////////////////
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjClass
//
//	This object is used to trigger a sound effect in the world.  When the object
// is shown, its associated sound is added to the world and played, when the object
// is hidden, the associated sound is stopped and removed from the world.
//
//	This is handy when used in conjunction with the aggregate system for creating
// complex animations.
//
//////////////////////////////////////////////////////////////////////////////////
class SoundRenderObjClass : public RenderObjClass
{
public:

	////////////////////////////////////////////////////////////////
	//	Public flags
	////////////////////////////////////////////////////////////////
	typedef enum
	{
		FLAG_STOP_WHEN_HIDDEN	= 0x00000001,

	} FLAGS;

	///////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////
	SoundRenderObjClass (void);
	SoundRenderObjClass (const SoundRenderObjClass &src);
	virtual ~SoundRenderObjClass (void);

	///////////////////////////////////////////////////////////
	//	Public operators
	///////////////////////////////////////////////////////////
	const SoundRenderObjClass &operator= (const SoundRenderObjClass &src);

	///////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////

	//
	//	From RenderObjClass
	//
	RenderObjClass *	Clone (void) const					{ return W3DNEW SoundRenderObjClass (*this); }
	int					Class_ID (void) const				{ return CLASSID_SOUND; }
	const char *		Get_Name (void) const				{ return Name; }
	void					Set_Name (const char *name)		{ Name = name; }
	void					Render (RenderInfoClass &rinfo)	{ }
	void					On_Frame_Update (void);
	void					Set_Hidden (int onoff);
	void					Set_Visible (int onoff);
	void					Set_Animation_Hidden (int onoff);
	void					Set_Force_Visible (int onoff);
	void					Notify_Added (SceneClass *scene);
	void					Notify_Removed (SceneClass *scene);
	void 					Set_Transform(const Matrix3D &m);
	void 					Set_Position(const Vector3 &v);

	//
	//	SoundRenderObjClass specific
	//
	virtual void						Set_Sound (AudibleSoundDefinitionClass *definition);
	virtual AudibleSoundClass *	Get_Sound (void) const;
	virtual AudibleSoundClass *	Peek_Sound (void) const			{ return Sound; }

	//
	//	Flag support
	//
	uint32					Get_Flags (void) const					{ return Flags; }
	void						Set_Flags (uint32 flags)				{ Flags = flags; }
	bool						Get_Flag (uint32 flag)					{ return bool((Flags & flag) == flag); }
	void						Set_Flag (uint32 flag, bool onoff);


protected:

	///////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////
	virtual void		Update_On_Visibility (void);

private:

	///////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////
	bool						IsInitialized;
	StringClass				Name;
	AudibleSoundClass *	Sound;
	uint32					Flags;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjDefClass
//
//////////////////////////////////////////////////////////////////////////////////
class SoundRenderObjDefClass : public RefCountClass
{
public:

	///////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////
	SoundRenderObjDefClass (void);
	SoundRenderObjDefClass (SoundRenderObjClass &render_obj);
	SoundRenderObjDefClass (const SoundRenderObjDefClass &src);
	virtual ~SoundRenderObjDefClass (void);

	///////////////////////////////////////////////////////////
	//	Public operators
	///////////////////////////////////////////////////////////
	const SoundRenderObjDefClass &operator= (const SoundRenderObjDefClass &src);

	///////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////
	RenderObjClass *				Create (void);
	WW3DErrorType					Load_W3D (ChunkLoadClass &cload);
	WW3DErrorType					Save_W3D (ChunkSaveClass &csave);
	const char *					Get_Name (void) const					{ return Name; }
	void								Set_Name (const char *name)			{ Name = name; }
	SoundRenderObjDefClass *	Clone (void) const						{ return NEW_REF( SoundRenderObjDefClass, (*this) ); }

	//
	//	Initialization
	//
	void								Initialize (SoundRenderObjClass &render_obj);

protected:

	///////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////

	//
	//	Loading methods
	//
	WW3DErrorType					Read_Header (ChunkLoadClass &cload);
	WW3DErrorType					Read_Definition (ChunkLoadClass &cload);

	//
	//	Saving methods
	//
	WW3DErrorType					Write_Header (ChunkSaveClass &csave);
	WW3DErrorType					Write_Definition (ChunkSaveClass &csave);

private:

	///////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////
	uint32								Version;
	StringClass							Name;
	AudibleSoundDefinitionClass 	Definition;
	SoundRenderObjClass::FLAGS		Flags;
};


///////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjPrototypeClass
//
///////////////////////////////////////////////////////////////////////////////////
class SoundRenderObjPrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(SoundRenderObjPrototypeClass)
public:

	///////////////////////////////////////////////////////////
	//	Public constructors/destructors
	///////////////////////////////////////////////////////////
	SoundRenderObjPrototypeClass (SoundRenderObjDefClass *def)
		: Definition (nullptr)													{ Set_Definition (def); }

	///////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////
	const char *					Get_Name(void) const					{ return Definition->Get_Name (); }
	int								Get_Class_ID(void) const			{ return RenderObjClass::CLASSID_SOUND; }
	RenderObjClass *				Create (void)							{ return Definition->Create (); }
	virtual void							DeleteSelf()										{ delete this; }

	SoundRenderObjDefClass	*	Peek_Definition (void) const						{ return Definition; }
	void								Set_Definition (SoundRenderObjDefClass *def)	{ REF_PTR_SET (Definition, def); }

protected:
	virtual ~SoundRenderObjPrototypeClass (void)						{ REF_PTR_RELEASE (Definition); }

private:

	///////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////
	SoundRenderObjDefClass *		Definition;
};


///////////////////////////////////////////////////////////////////////////////////
//
//	SoundRenderObjLoaderClass
//
///////////////////////////////////////////////////////////////////////////////////
class SoundRenderObjLoaderClass : public PrototypeLoaderClass
{
public:
	virtual int						Chunk_Type (void)		{ return W3D_CHUNK_SOUNDROBJ; }
	virtual PrototypeClass *	Load_W3D (ChunkLoadClass &cload);
};


///////////////////////////////////////////////////////////////////////////////////
//	Global variables
///////////////////////////////////////////////////////////////////////////////////
extern SoundRenderObjLoaderClass		_SoundRenderObjLoader;

#endif //noWWAUDIO (gth) removing dependency on wwaudio
