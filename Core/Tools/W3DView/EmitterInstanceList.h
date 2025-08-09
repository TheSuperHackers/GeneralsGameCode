/*
**	Command & Conquer Renegade(tm)
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
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/W3DView/EmitterInstanceList.h                                                                                                                                                                                                                                                                                                                                 $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __EMITTER_INSTANCE_LIST_H
#define __EMITTER_INSTANCE_LIST_H

#include "Vector.H"
#include "part_ldr.h"
#include "part_emt.h"


/////////////////////////////////////////////////////////////////////
//
//	EmitterInstanceListClass
//
/////////////////////////////////////////////////////////////////////
class EmitterInstanceListClass : public ParticleEmitterDefClass
{
	public:

		///////////////////////////////////////////////////////
		// Public constructors/destructors
		///////////////////////////////////////////////////////
		EmitterInstanceListClass (void)		{ }
		EmitterInstanceListClass (const EmitterInstanceListClass &src)
			: ParticleEmitterDefClass (src)	{ }

		virtual ~EmitterInstanceListClass (void);

		///////////////////////////////////////////////////////
		// Public methods
		///////////////////////////////////////////////////////
		virtual void			Add_Emitter (ParticleEmitterClass *emitter);
		virtual void			Free_List (void);

		///////////////////////////////////////////////////////
		// Derived overrides
		///////////////////////////////////////////////////////

		//
		//	Note:  The following are settings that can be changed on
		//		the fly.  All other settings are simply cached in the
		//		definition and can be used to create a new prototype loader.
		//

		virtual void			Set_Velocity (const Vector3 &value);
		virtual void			Set_Acceleration (const Vector3 &value);
		virtual void			Set_Burst_Size (unsigned int count);
		virtual void			Set_Outward_Vel (float value);
		virtual void			Set_Vel_Inherit (float value);

		//
		//	Randomizer accessors
		//
		virtual void			Set_Velocity_Random (Vector3Randomizer *randomizer);

		//
		//	Keyframe accessors
		//
		virtual void			Set_Color_Keyframes (ParticlePropertyStruct<Vector3> &keyframes);
		virtual void			Set_Opacity_Keyframes (ParticlePropertyStruct<float> &keyframes);
		virtual void			Set_Size_Keyframes (ParticlePropertyStruct<float> &keyframes);
		virtual void			Set_Rotation_Keyframes (ParticlePropertyStruct<float> &keyframes, float orient_rnd);
		virtual void			Set_Frame_Keyframes (ParticlePropertyStruct<float> &keyframes);
		virtual void			Set_Blur_Time_Keyframes (ParticlePropertyStruct<float> &keyframes);

		virtual void			Get_Color_Keyframes (ParticlePropertyStruct<Vector3> &keyframes) const;
		virtual void			Get_Opacity_Keyframes (ParticlePropertyStruct<float> &keyframes) const;
		virtual void			Get_Size_Keyframes (ParticlePropertyStruct<float> &keyframes) const;


	private:

		///////////////////////////////////////////////////////
		// Private member data
		///////////////////////////////////////////////////////

		DynamicVectorClass<ParticleEmitterClass *>	m_List;
};

#endif //__EMITTER_INSTANCE_LIST_H
