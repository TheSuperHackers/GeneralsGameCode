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
 *                     $Archive:: /Commando/Code/WWAudio/AudibleSound.h                       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 12:08p                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SOUNDOBJ_H
#define __SOUNDOBJ_H

#pragma warning (push, 3)
#include "mss.h"
#pragma warning (pop)

//#include <malloc.h>
#include "vector3.h"
#include "matrix3d.h"
#include "refcount.h"
#include "RAWFILE.H"
#include "SoundSceneObj.h"
#include "Vector.H"
#include "wwstring.h"
#include "definition.h"


/////////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////////
class SoundBufferClass;
class RenderObjClass;
class LogicalSoundClass;
class AudibleSoundDefinitionClass;
class Sound3DClass;
class SoundPseudo3DClass;
class FilteredSoundClass;
class Listener3DClass;
class SoundHandleClass;


/////////////////////////////////////////////////////////////////////////////////
//
//	Typedefs
//
typedef unsigned long MILES_HANDLE;

typedef enum
{
	INFO_OBJECT_PTR	= 0,
	INFO_MAX
} HANDLE_USER_INDEX;


/////////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
/////////////////////////////////////////////////////////////////////////////////
const MILES_HANDLE INVALID_MILES_HANDLE			= (MILES_HANDLE)-1;
const int INFINITE_LOOPS								= 0;


/////////////////////////////////////////////////////////////////////////////////
//
//	AudibleSoundClass
//
//	Base class defining a common interface to all sound object types.
//
/////////////////////////////////////////////////////////////////////////////////
class AudibleSoundClass : public SoundSceneObjClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Friend classes
		//////////////////////////////////////////////////////////////////////
		friend class WWAudioClass;

		//////////////////////////////////////////////////////////////////////
		//	Public data types
		//////////////////////////////////////////////////////////////////////
		typedef enum
		{
			TYPE_MUSIC				= 0,
			TYPE_SOUND_EFFECT,
			TYPE_COUNT
		} SOUND_TYPE;

		typedef enum
		{
			STATE_STOPPED			= 0,
			STATE_PLAYING,
			STATE_PAUSED,
			STATE_COUNT
		} SOUND_STATE;

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		AudibleSoundClass (const AudibleSoundClass &src);
		AudibleSoundClass (void);
		virtual ~AudibleSoundClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public operators
		//////////////////////////////////////////////////////////////////////
		const AudibleSoundClass &operator= (const AudibleSoundClass &src);

		//////////////////////////////////////////////////////////////////////
		//	Identification methods
		//////////////////////////////////////////////////////////////////////
		virtual SOUND_CLASSID	Get_Class_ID (void) const	{ return CLASSID_2D; }
		virtual SOUND_TYPE		Get_Type (void) const		{ return m_Type; }
		virtual void				Set_Type (SOUND_TYPE type)	{ m_Type = type; }

		//////////////////////////////////////////////////////////////////////
		//	Conversion methods
		//////////////////////////////////////////////////////////////////////
		virtual AudibleSoundClass *	As_AudibleSoundClass(void) 	{ return this; }

		//////////////////////////////////////////////////////////////////////
		//	Update methods
		//////////////////////////////////////////////////////////////////////
		virtual bool				On_Frame_Update (unsigned int milliseconds = 0);

		//////////////////////////////////////////////////////////////////////
		//	State control methods
		//////////////////////////////////////////////////////////////////////
		virtual bool				Play (bool alloc_handle = true);
		virtual bool				Pause (void);
		virtual bool				Resume (void);
		virtual bool				Stop (bool remove_from_playlist = true);
		virtual void				Seek (unsigned long milliseconds);
		virtual SOUND_STATE		Get_State (void) const	{ return m_State; }

		// The timestamp represents when the sound started playing
		virtual unsigned long	Get_Timestamp (void) const { return m_Timestamp; }

		//////////////////////////////////////////////////////////////////////
		//	Informational methods
		//////////////////////////////////////////////////////////////////////
		virtual LPCTSTR			Get_Filename (void) const;
		virtual bool				Is_Playing (void) const  {return(Get_State() == STATE_PLAYING);}

		//////////////////////////////////////////////////////////////////////
		//	Pan control
		//////////////////////////////////////////////////////////////////////

		//
		//	Note:  Pan values are normalized values from 0 (hard left) to
		// 1.0F (hard right).  Dead center is 0.5F.
		//
		virtual float				Get_Pan (void);
		virtual void				Set_Pan (float pan = 0.5F);

		//////////////////////////////////////////////////////////////////////
		//	Volume control
		//////////////////////////////////////////////////////////////////////
		virtual float				Get_Volume (void);
		virtual void				Set_Volume (float volume = 1.0F);
		virtual void				Update_Volume (void)						{ Set_Volume (m_Volume); }

		//////////////////////////////////////////////////////////////////////
		//	Loop control
		//////////////////////////////////////////////////////////////////////
		virtual int					Get_Loop_Count (void) const			{ return m_LoopCount; }
		virtual int					Get_Loops_Left (void) const;

		// Note:  Use the INFINITE_LOOPS constant for an infinite number of loops. (i.e. for use w/ music)
		virtual void				Set_Loop_Count (int count = 1);

		//////////////////////////////////////////////////////////////////////
		//	Priority control
		//////////////////////////////////////////////////////////////////////
		virtual float				Get_Priority (void) const					{ return m_Priority; }
		virtual float				Peek_Priority (void) const					{ return m_Priority; }
		virtual void				Set_Priority (float priority = 0.5F);

		virtual float				Get_Runtime_Priority (void) const		{ return m_RuntimePriority; }
		virtual void				Set_Runtime_Priority (float priority)	{ m_RuntimePriority = priority; }

		//////////////////////////////////////////////////////////////////////
		//	Playback rate control
		//////////////////////////////////////////////////////////////////////
		virtual int					Get_Playback_Rate (void);
		virtual void				Set_Playback_Rate (int rate_in_hz);
		
		virtual float				Get_Pitch_Factor (void)						{ return m_PitchFactor; }
		virtual void				Set_Pitch_Factor (float factor);

		//////////////////////////////////////////////////////////////////////
		//	Buffer position manipulation
		//////////////////////////////////////////////////////////////////////

		//
		//	Note:  The duration is in milliseconds.  The play position
		// can either be set as a normalized value from 0 to 1 or a millisecond
		// offset from the start of the sound.
		//
		virtual unsigned long	Get_Duration (void) const								{ return m_Length; }
		virtual unsigned long	Get_Play_Position (void) const						{ return m_CurrentPosition; }
		virtual void				Set_Play_Position (float position)					{ Seek (position * m_Length); }
		virtual void				Set_Play_Position (unsigned long milliseconds)	{ Seek (milliseconds); }

		virtual void				Set_Start_Offset (float offset)						{ m_StartOffset = offset; }
		virtual float				Get_Start_Offset (void) const							{ return m_StartOffset; }

		//////////////////////////////////////////////////////////////////////
		//	Position/direction methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Set_Position (const Vector3 &position);
		virtual Vector3		Get_Position (void) const							{ return m_Transform.Get_Translation (); }

		virtual void			Set_Listener_Transform (const Matrix3D &tm)	{ m_ListenerTransform = tm; }
		virtual void			Set_Transform (const Matrix3D &transform);
		virtual Matrix3D		Get_Transform (void) const							{ return m_Transform; }

		//////////////////////////////////////////////////////////////////////
		//	Culling methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Cull_Sound (bool culled = true);
		virtual bool			Is_Sound_Culled (void) const { return m_IsCulled; }

		//////////////////////////////////////////////////////////////////////
		//	Scene integration
		//////////////////////////////////////////////////////////////////////
		virtual void			Add_To_Scene (bool start_playing = true);
		virtual void			Remove_From_Scene (void);

		//////////////////////////////////////////////////////////////////////
		//	Attenuation settings
		//////////////////////////////////////////////////////////////////////

		//
		//	This is the distance where the sound can not be heard any longer.  (its vol is 0)
		//
		virtual void			Set_DropOff_Radius (float radius = 1);
		virtual float			Get_DropOff_Radius (void) const	{ return m_DropOffRadius; }

		//////////////////////////////////////////////////////////////////////
		//	Update methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Set_Dirty (bool dirty = true)		{ m_bDirty = dirty; }
		virtual bool			Is_Dirty (void) const				{ return m_bDirty; }

		//////////////////////////////////////////////////////////////////////
		//	Definition managmenent
		//////////////////////////////////////////////////////////////////////
		virtual void									Set_Definition (AudibleSoundDefinitionClass *def) { m_Definition = def; }
		virtual AudibleSoundDefinitionClass *	Get_Definition (void) { return m_Definition; }

		//////////////////////////////////////////////////////////////////////
		//	Conversion methods
		//////////////////////////////////////////////////////////////////////
		virtual void					Re_Sync (AudibleSoundClass &src);
		virtual void					Free_Conversion (void);
		virtual void					Convert_To_Filtered (void);
		virtual AudibleSoundClass *As_Converted_Format (void);

		//////////////////////////////////////////////////////////////////////
		//	From PersistClass
		//////////////////////////////////////////////////////////////////////
		const PersistFactoryClass &	Get_Factory (void) const;
		bool									Save (ChunkSaveClass &csave);
		bool									Load (ChunkLoadClass &cload);

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////
		virtual float				Determine_Real_Volume (void) const;

		//////////////////////////////////////////////////////////////////////
		//	Handle information
		//////////////////////////////////////////////////////////////////////
		virtual SoundHandleClass *	Get_Miles_Handle (void) const			{ return m_SoundHandle; }
		virtual void				Set_Miles_Handle (MILES_HANDLE handle = INVALID_MILES_HANDLE);
		virtual void				Free_Miles_Handle (void);
		virtual void				Initialize_Miles_Handle (void);
		virtual void				Allocate_Miles_Handle (void);

		//////////////////////////////////////////////////////////////////////
		//	Buffer information
		//////////////////////////////////////////////////////////////////////
		virtual SoundBufferClass *	Get_Buffer (void) const;
		virtual SoundBufferClass *	Peek_Buffer (void) const;
		virtual void					Set_Buffer (SoundBufferClass *buffer);

		//////////////////////////////////////////////////////////////////////
		//	Loop methods
		//////////////////////////////////////////////////////////////////////
		virtual void				Restart_Loop (void) {}
		virtual void				Update_Play_Position (void);

		//////////////////////////////////////////////////////////////////////
		//	Event handling
		//////////////////////////////////////////////////////////////////////
		virtual void				On_Loop_End (void);

		//////////////////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////////////////
		SoundHandleClass *	m_SoundHandle;
		unsigned long			m_Timestamp;
		SOUND_STATE				m_State;
		SOUND_TYPE				m_Type;

		// Buffer information
		SoundBufferClass *	m_Buffer;

		// Cached settings
		float						m_RuntimePriority;
		float						m_Priority;
		float						m_Volume;
		float						m_Pan;
		int						m_LoopCount;
		int						m_LoopsLeft;

		// Offset and length information (in milliseconds)
		unsigned long			m_Length;
		unsigned long			m_CurrentPosition;
		float						m_StartOffset;
		float						m_PitchFactor;

		// 3D scene information
		Matrix3D					m_ListenerTransform;
		Matrix3D					m_Transform;
		Matrix3D					m_PrevTransform;
		bool						m_IsCulled;
		bool						m_bDirty;
		float						m_DropOffRadius;

		// Conversion data
		AudibleSoundClass *	m_pConvertedFormat;

		// Definition pointer
		AudibleSoundDefinitionClass *	m_Definition;

		//	Logical sound information
		LogicalSoundClass *				m_LogicalSound;
};


//////////////////////////////////////////////////////////////////////////////////
//
//	AudibleSoundDefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
class AudibleSoundDefinitionClass : public DefinitionClass
{

public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(AudibleSoundDefinitionClass, DefinitionClass);

	//////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////
	AudibleSoundDefinitionClass (void);
	virtual ~AudibleSoundDefinitionClass (void) { }

	// From DefinitionClass
	virtual uint32								Get_Class_ID (void) const;

	// From PersistClass
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool								Save (ChunkSaveClass &csave);
	virtual bool								Load (ChunkLoadClass &cload);
	virtual PersistClass *					Create (void) const;
	virtual AudibleSoundClass *			Create_Sound (int classid_hint) const;

	// Initialization
	virtual void								Initialize_From_Sound (AudibleSoundClass *sound);

	// Accessors
	virtual const StringClass &			Get_Filename (void) const			{ return m_Filename; }
	virtual const StringClass &			Get_Display_Text (void) const		{ return m_DisplayText; }
	virtual float								Get_Max_Vol_Radius (void) const	{ return m_MaxVolRadius; }
	virtual float								Get_DropOff_Radius (void) const	{ return m_DropOffRadius; }
	virtual const Vector3 &					Get_Sphere_Color (void) const		{ return m_AttenuationSphereColor; }
	virtual float								Get_Volume (void) const				{ return m_Volume; }
	virtual float								Get_Start_Offset (void) const		{ return m_StartOffset; }
	virtual float								Get_Pitch_Factor (void) const		{ return m_PitchFactor; }	

	virtual void								Set_Volume (float volume)			{ m_Volume = volume; }
	virtual void								Set_Max_Vol_Radius (float radius){ m_MaxVolRadius = radius; }
	virtual void								Set_DropOff_Radius (float radius){ m_DropOffRadius = radius; }
	virtual void								Set_Start_Offset (float offset)	{ m_StartOffset = offset; }
	virtual void								Set_Pitch_Factor (float factor)	{ m_PitchFactor = factor; }	

	// Logical sound creation
	virtual LogicalSoundClass *			Create_Logical (void);

protected:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	bool										Save_Variables (ChunkSaveClass &csave);
	bool										Load_Variables (ChunkLoadClass &cload);

	//////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////
	float					m_Priority;
	float					m_Volume;
	float					m_Pan;
	int					m_LoopCount;
	float					m_DropOffRadius;
	float					m_MaxVolRadius;
	bool					m_Is3D;
	StringClass			m_Filename;
	int					m_Type;
	StringClass			m_DisplayText;
	float					m_StartOffset;
	float					m_PitchFactor;

	int					m_LogicalTypeMask;
	float					m_LogicalNotifyDelay;
	float					m_LogicalDropOffRadius;
	bool					m_CreateLogical;

	// Misc UI info
	Vector3				m_AttenuationSphereColor;
};


#endif //__SOUNDOBJ_H
