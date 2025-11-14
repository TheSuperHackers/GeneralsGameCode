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
 *                     $Archive:: /Commando/Code/WWAudio/SoundPseudo3D.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 12:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "Sound3D.h"


/////////////////////////////////////////////////////////////////////////////////
//
//	SoundPseudo3DClass
//
//	Pseudo-3D objects are not true 3D sounds.  They have the
// same properties as 3D 'sound effects' but do not use 3D
// hardware, are not restricted to mono, uncompressed, WAV data,
// and do not calculate doppler and reverb effects.
//
class SoundPseudo3DClass : public Sound3DClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		SoundPseudo3DClass (const SoundPseudo3DClass &src);
		SoundPseudo3DClass (void);
		virtual ~SoundPseudo3DClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public operators
		//////////////////////////////////////////////////////////////////////
		const SoundPseudo3DClass &operator= (const SoundPseudo3DClass &src);

		//////////////////////////////////////////////////////////////////////
		//	Identification methods
		//////////////////////////////////////////////////////////////////////
		virtual SOUND_CLASSID	Get_Class_ID (void) const	{ return CLASSID_PSEUDO3D; }

		//////////////////////////////////////////////////////////////////////
		//	Conversion methods
		//////////////////////////////////////////////////////////////////////
		virtual SoundPseudo3DClass *	As_SoundPseudo3DClass (void) { return this; }

		//////////////////////////////////////////////////////////////////////
		//	Volume control
		//////////////////////////////////////////////////////////////////////
		virtual void			Update_Volume (void)									{ Update_Pseudo_Volume (); }

		//////////////////////////////////////////////////////////////////////
		//	Position/direction methods
		//////////////////////////////////////////////////////////////////////
		virtual void			Set_Listener_Transform (const Matrix3D &tm)	{ m_ListenerTransform = tm; }
		virtual void			Set_Position (const Vector3 &position)			{ Set_Dirty (); m_Transform.Set_Translation (position); }
		virtual void			Set_Transform (const Matrix3D &transform)		{ Set_Dirty (); m_Transform = transform; }

		//////////////////////////////////////////////////////////////////////
		//	Velocity methods
		//////////////////////////////////////////////////////////////////////

		//
		// The velocity settings are in meters per millisecond.
		//
		virtual void			Set_Velocity (const Vector3 &velocity)			{ }

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
		virtual void			Set_Max_Vol_Radius (float radius = 0)			{ m_MaxVolRadius = radius; }
		virtual float			Get_Max_Vol_Radius (void) const					{ return m_MaxVolRadius; }

		//
		//	This is the distance where the sound can not be heard any longer.  (its vol is 0)
		//
		virtual void			Set_DropOff_Radius (float radius = 1)			{ m_DropOffRadius = radius; }
		virtual float			Get_DropOff_Radius (void) const					{ return m_DropOffRadius; }

		//////////////////////////////////////////////////////////////////////
		//	Volume control
		//////////////////////////////////////////////////////////////////////
		virtual void			Update_Pseudo_Volume (void);
		virtual void			Update_Pseudo_Volume (float distance);

		//////////////////////////////////////////////////////////////////////
		//	Pan control
		//////////////////////////////////////////////////////////////////////
		virtual void			Update_Pseudo_Pan (void);

		// From PersistClass
		const PersistFactoryClass &	Get_Factory (void) const;

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Update methods
		//////////////////////////////////////////////////////////////////////
		virtual bool			On_Frame_Update (unsigned int milliseconds);

		//////////////////////////////////////////////////////////////////////
		//	Handle information
		//////////////////////////////////////////////////////////////////////
		virtual void			Set_Miles_Handle (MILES_HANDLE handle);
		virtual void			Initialize_Miles_Handle (void);
		virtual void			Allocate_Miles_Handle (void);
		virtual void			Free_Miles_Handle (void);

		virtual void			On_Loop_End (void) { Sound3DClass::On_Loop_End (); }

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
};
