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
 *                     $Archive:: /VSS_Sync/W3DView/EmitterInstanceList.cpp                                                                                                                                                                                                                                                                                                                                $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.h"
#include "EmitterInstanceList.h"
#include "Utils.h"

/////////////////////////////////////////////////////////////////////
//
//	~EmitterInstanceListClass
//
/////////////////////////////////////////////////////////////////////
EmitterInstanceListClass::~EmitterInstanceListClass (void)
{
	Free_List ();
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Free_List
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Free_List (void)
{
	//
	//	Release our hold on each of the emitter pointers
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		MEMBER_RELEASE (m_List[index]);
	}

	m_List.Delete_All ();
	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Add_Emitter
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Add_Emitter (ParticleEmitterClass *emitter)
{
	ASSERT (emitter != NULL);
	if (emitter != NULL) {

		//
		//	If this is the first emitter in the list, then initialize
		// the definition to it's state
		//
		if (m_List.Count () == 0) {
			ParticleEmitterDefClass *def = emitter->Build_Definition ();
			if (def != NULL) {
				ParticleEmitterDefClass::operator= (*def);
				SAFE_DELETE (def);
			}
		}

		//
		//	Add this emitter to the list and put a hold on its reference
		//
		SAFE_ADD_REF (emitter);
		m_List.Add (emitter);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Velocity
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Velocity (const Vector3 &value)
{
	ParticleEmitterDefClass::Set_Velocity (value);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Set_Base_Velocity (value);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Acceleration
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Acceleration (const Vector3 &value)
{
	ParticleEmitterDefClass::Set_Acceleration (value);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Set_Acceleration (value);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Burst_Size
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Burst_Size (unsigned int count)
{
	ParticleEmitterDefClass::Set_Burst_Size (count);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Set_Burst_Size (count);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Outward_Vel
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Outward_Vel (float value)
{
	ParticleEmitterDefClass::Set_Outward_Vel (value);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Set_Outwards_Velocity (value);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Vel_Inherit
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Vel_Inherit (float value)
{
	ParticleEmitterDefClass::Set_Vel_Inherit (value);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Set_Velocity_Inheritance_Factor (value);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Velocity_Random
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Velocity_Random (Vector3Randomizer *randomizer)
{
	ParticleEmitterDefClass::Set_Velocity_Random (randomizer);
	if (randomizer != NULL) {

		//
		//	Pass this setting onto the emitters immediately
		//
		for (int index = 0; index < m_List.Count (); index ++) {
			m_List[index]->Set_Velocity_Randomizer (randomizer->Clone ());
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Color_Keyframes
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Color_Keyframes (ParticlePropertyStruct<Vector3> &keyframes)
{
	//
	//	Make sure tha any value that is supposed to go to zero, really
	//	does even if its got a randomizer.
	//
	if (	(keyframes.Rand.X != 0) ||
			(keyframes.Rand.Y != 0) ||
			(keyframes.Rand.Z != 0))
	{
		for (UINT index = 0; index < keyframes.NumKeyFrames; index ++) {
			if ((keyframes.Values[index].X <= 0.000001F) &&
				 (keyframes.Values[index].Y <= 0.000001F) &&
				 (keyframes.Values[index].Z <= 0.000001F)) {
				keyframes.Values[index].X = -keyframes.Rand.X;
				keyframes.Values[index].Y = -keyframes.Rand.Y;
				keyframes.Values[index].Z = -keyframes.Rand.Z;
			}
		}
	}

	ParticleEmitterDefClass::Set_Color_Keyframes (keyframes);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Reset_Colors (keyframes);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Opacity_Keyframes
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Opacity_Keyframes (ParticlePropertyStruct<float> &keyframes)
{
	//
	//	Make sure tha any value that is supposed to go to zero, really
	//	does even if its got a randomizer.
	//
	if (keyframes.Rand != 0)
	{
		for (UINT index = 0; index < keyframes.NumKeyFrames; index ++) {
			if (keyframes.Values[index] <= 0.000001F) {
				keyframes.Values[index] = -keyframes.Rand;
			}
		}
	}

	ParticleEmitterDefClass::Set_Opacity_Keyframes (keyframes);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Reset_Opacity (keyframes);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////
//
//	Set_Size_Keyframes
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Size_Keyframes (ParticlePropertyStruct<float> &keyframes)
{
	//
	//	Make sure tha any value that is supposed to go to zero, really
	//	does even if its got a randomizer.
	//
	if (keyframes.Rand != 0)
	{
		for (UINT index = 0; index < keyframes.NumKeyFrames; index ++) {
			if (keyframes.Values[index] <= 0.000001F) {
				keyframes.Values[index] = -keyframes.Rand;
			}
		}
	}

	ParticleEmitterDefClass::Set_Size_Keyframes (keyframes);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Reset_Size (keyframes);
	}

	return ;
}



/////////////////////////////////////////////////////////////////////
//
//	Set_Rotation_Keyframes
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Rotation_Keyframes (ParticlePropertyStruct<float> &keyframes, float orient_rnd)
{
	ParticleEmitterDefClass::Set_Rotation_Keyframes (keyframes, orient_rnd);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Reset_Rotations (keyframes, orient_rnd);
	}

	return ;
}

/////////////////////////////////////////////////////////////////////
//
//	Set_Frame_Keyframes
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Frame_Keyframes (ParticlePropertyStruct<float> &keyframes)
{
	ParticleEmitterDefClass::Set_Frame_Keyframes (keyframes);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Reset_Frames (keyframes);
	}

	return ;
}

/////////////////////////////////////////////////////////////////////
//
//	Set_Blur_Time_Keyframes
//
/////////////////////////////////////////////////////////////////////
void
EmitterInstanceListClass::Set_Blur_Time_Keyframes (ParticlePropertyStruct<float> &keyframes)
{
	ParticleEmitterDefClass::Set_Blur_Time_Keyframes (keyframes);

	//
	//	Pass this setting onto the emitters immediately
	//
	for (int index = 0; index < m_List.Count (); index ++) {
		m_List[index]->Reset_Blur_Times (keyframes);
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Color_Keyframes
//
void
EmitterInstanceListClass::Get_Color_Keyframes (ParticlePropertyStruct<Vector3> &keyframes) const
{
	ParticleEmitterDefClass::Get_Color_Keyframes (keyframes);

	//
	//	Normalize the data
	//
	for (UINT index = 0; index < keyframes.NumKeyFrames; index ++) {
		if (keyframes.Values[index].X <= 0.000001F) {
			keyframes.Values[index].X = 0;
		}
		if (keyframes.Values[index].Y <= 0.000001F) {
			keyframes.Values[index].Y = 0;
		}
		if (keyframes.Values[index].Z <= 0.000001F) {
			keyframes.Values[index].Z = 0;
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Opacity_Keyframes
//
void
EmitterInstanceListClass::Get_Opacity_Keyframes (ParticlePropertyStruct<float> &keyframes) const
{
	ParticleEmitterDefClass::Get_Opacity_Keyframes (keyframes);

	//
	//	Normalize the data
	//
	for (UINT index = 0; index < keyframes.NumKeyFrames; index ++) {
		if (keyframes.Values[index] <= 0.000001F) {
			keyframes.Values[index] = 0;
		}
	}
	return ;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	Get_Size_Keyframes
//
void
EmitterInstanceListClass::Get_Size_Keyframes (ParticlePropertyStruct<float> &keyframes) const
{
	ParticleEmitterDefClass::Get_Size_Keyframes (keyframes);

	//
	//	Normalize the data
	//
	for (UINT index = 0; index < keyframes.NumKeyFrames; index ++) {
		if (keyframes.Values[index] <= 0.000001F) {
			keyframes.Values[index] = 0;
		}
	}
	return ;
}

