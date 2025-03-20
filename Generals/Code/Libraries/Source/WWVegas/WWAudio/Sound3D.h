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
 *                     $Archive:: /Commando/Code/WWAudio/Sound3D.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 12:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __SOUND3DOBJ_H
#define __SOUND3DOBJ_H

#include "AudibleSound.h"
#include "mempool.h"




/////////////////////////////////////////////////////////////////////////////////
//
//	Sound3DClass
//
//	Class defining a 3D sound.  A 3D sound uses position and velocity information
// to determine volume/panning/doppler etc.
//
//	A 3D sound should be added to the SoundScene rather than explicitly played.  The
// SoundScene will determine when a 3D sound is 'in range' and play it...
//
class Sound3DClass : public AudibleSoundClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Friend classes
		//////////////////////////////////////////////////////////////////////
		friend class SoundSceneClass;

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		Sound3DClass (const Sound3DClass &src);
		Sound3DClass (void);
		virtual ~Sound3DClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public operators
		//////////////////////////////////////////////////////////////////////
		const Sound3DClass &operator= (const Sound3DClass &src);


		//////////////////////////////////////////////////////////////////////
		//	Identification methods
		//////////////////////////////////////////////////////////////////////
		virtual SOUND_CLASSID	Get_Class_ID (void) const	{ return CLASSID_3D; }
		virtual void				Make_Static (bool is_static = true)	{ m_IsStatic = is_static; }
		virtual bool				Is_Static (void) const					{ return m_IsStatic; }

		//////////////////////////////////////////////////////////////////////
		//	Conversion methods
		//////////////////////////////////////////////////////////////////////		
		virtual Sound3DClass *	As_Sound3DClass (void) { return this; }

		//////////////////////////////////////////////////////////////////////
		//	State control methods
		//////////////////////////////////////////////////////////////////////
		virtual bool			Play (bool alloc_handle = true);

		//////////////////////////////////////////////////////////////////////
		//	Priority control
		//////////////////////////////////////////////////////////////////////
		virtual float			Get_Priority (void) const			{ if (m_IsCulled) return 0; return m_Priority; }

		//////////////////////////////////////////////////////////////////////
		//	Scene integration
		//////////////////////////////////////////////////////////////////////
		virtual void			Add_To_Scene (bool start_playing = true);
		virtual void			Remove_From_Scene (void);		

		//////////////////////////////////////////////////////////////////////
		//	Position/direction methods
		//////////////////////////////////////////////////////////////////////		
		virtual void			Set_Position (const Vector3 &position);
		virtual Vector3		Get_Position (void) const							{ return m_Transform.Get_Translation (); }

		virtual void			Set_Listener_Transform (const Matrix3D &tm);
		virtual void			Set_Transform (const Matrix3D &transform);
		virtual Matrix3D		Get_Transform (void) const							{ return m_Transform; }
		void						Update_Miles_Transform (void);

		//////////////////////////////////////////////////////////////////////
		//	Velocity methods
		//////////////////////////////////////////////////////////////////////		
		
		//
		// The velocity settings are in meters per millisecond.
		//
		virtual void			Set_Velocity (const Vector3 &velocity);
		virtual Vector3		Get_Velocity (void) const							{ return m_CurrentVelocity; }
		virtual void			Get_Velocity (Vector3 &velocity) const			{ velocity = m_CurrentVelocity; }

		virtual void			Auto_Calc_Velocity (bool autocalc = true)		{ m_bAutoCalcVel = autocalc; }
		virtual bool			Is_Auto_Calc_Velocity_On (void) const			{ return m_bAutoCalcVel; }

		//////////////////////////////////////////////////////////////////////
		//	Attenuation settings
		//////////////////////////////////////////////////////////////////////
		
		//
		// The maximum-volume radius is the distance from the sound-emitter where
		//	it seems as loud as it is going to get.  Volume does not increase after this
		// point.  Volume is linerally interpolated from the DropOff distance to the MaxVol
		// distance. For some objects (like an airplane) the max-vol distance is
		// not 0, but would be 100 or so meters away.
		//
		virtual void			Set_Max_Vol_Radius (float radius = 0);
		virtual float			Get_Max_Vol_Radius (void) const					{ return m_MaxVolRadius; }

		//
		//	This is the distance where the sound can not be heard any longer.  (its vol is 0)
		//
		virtual void			Set_DropOff_Radius (float radius = 1);
		virtual float			Get_DropOff_Radius ()  {return(m_DropOffRadius);}

		// From PersistClass
		const PersistFactoryClass &	Get_Factory (void) const;

		//
		//	From PersistClass
		//
		virtual bool					Save (ChunkSaveClass &csave);
		virtual bool					Load (ChunkLoadClass &cload);

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Handle information
		//////////////////////////////////////////////////////////////////////				
		virtual SoundCullObjClass *	Peek_Cullable_Wrapper (void) const	{ return m_PhysWrapper; }
		virtual void						Set_Cullable_Wrapper (SoundCullObjClass *obj) { m_PhysWrapper = obj; }

		//////////////////////////////////////////////////////////////////////
		//	Update methods
		//////////////////////////////////////////////////////////////////////
		virtual bool					On_Frame_Update (unsigned int milliseconds = 0);		

		//////////////////////////////////////////////////////////////////////
		//	Handle information
		//////////////////////////////////////////////////////////////////////				
		virtual void			Set_Miles_Handle (MILES_HANDLE handle);
		virtual void			Initialize_Miles_Handle (void);
		virtual void			Allocate_Miles_Handle (void);		

		//////////////////////////////////////////////////////////////////////
		//	Event handling
		//////////////////////////////////////////////////////////////////////		
		virtual void			On_Loop_End (void);

		//////////////////////////////////////////////////////////////////////
		//	Protected member data
		//////////////////////////////////////////////////////////////////////
		bool						m_IsTransformInitted;
		bool						m_bAutoCalcVel;
		Vector3					m_CurrentVelocity;		
		float						m_MaxVolRadius;		
		bool						m_IsStatic;
		unsigned int			m_LastUpdate;
};


#endif //__SOUND3DOBJ_H
