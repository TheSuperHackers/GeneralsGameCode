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
 *                     $Archive:: /Commando/Code/WWAudio/SoundPseudo3D.cpp         $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 12:09p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "SoundPseudo3D.h"
#include "WWAudio.h"
#include "SoundScene.h"
#include "Utils.h"
#include "SoundChunkIDs.h"
#include "persistfactory.h"
#include "soundhandle.h"


//////////////////////////////////////////////////////////////////////////////////
//	Static factories
//////////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<SoundPseudo3DClass, CHUNKID_PSEUDO_SOUND3D> _PseudoSound3DPersistFactory;


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SoundPseudo3DClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundPseudo3DClass::SoundPseudo3DClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SoundPseudo3DClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundPseudo3DClass::SoundPseudo3DClass (const SoundPseudo3DClass &src)
	:	Sound3DClass (src)
{
	(*this) = src;
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	~SoundPseudo3DClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
SoundPseudo3DClass::~SoundPseudo3DClass (void)
{
	Free_Miles_Handle ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////////////////////////////////////
const SoundPseudo3DClass &
SoundPseudo3DClass::operator= (const SoundPseudo3DClass &src)
{
	// Call the base class
	Sound3DClass::operator= (src);
	return (*this);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Set_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundPseudo3DClass::Set_Miles_Handle (MILES_HANDLE handle)
{
	AudibleSoundClass::Set_Miles_Handle (handle);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundPseudo3DClass::Initialize_Miles_Handle (void)
{
	AudibleSoundClass::Initialize_Miles_Handle ();
	Update_Pseudo_Volume ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Pseudo_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundPseudo3DClass::Update_Pseudo_Volume (float distance)
{
	MMSLockClass lock;	

	//
	// Only do this if the sound is really playing
	//
	if (m_SoundHandle != NULL) {
		
		float volume_mod = Determine_Real_Volume ();
		float max_distance = Get_DropOff_Radius ();
		float min_distance = Get_Max_Vol_Radius ();
		float delta = max_distance - min_distance;

		// Determine a normalized volume from the position
		float volume = 1.0F;
		if (distance > min_distance) {
			volume = 1.0F - ((distance - min_distance) / delta);
			volume = min (volume, 1.0F);
			volume = max (volume, 0.0F);			
		}

		// Multiply the 'max' volume with the calculated volume
		volume = volume * volume_mod;

		//
		// Pass the volume on
		//
		m_SoundHandle->Set_Sample_Volume (int(volume * 127.0F));
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Pseudo_Volume
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundPseudo3DClass::Update_Pseudo_Volume (void)
{
	MMSLockClass lock;	

	// Only do this if the sound is really playing
	if (m_SoundHandle != NULL) {
		
		//
		// Find the difference in the sound position and its listener's position
		//		
		Vector3 sound_pos = m_ListenerTransform.Get_Translation () - m_Transform.Get_Translation ();
		float distance = sound_pos.Quick_Length ();

		//
		// Determine a normalized volume from the position
		//
		Update_Pseudo_Volume (distance);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Pseudo_Pan
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundPseudo3DClass::Update_Pseudo_Pan (void)
{
	MMSLockClass lock;	

	//
	// Only do this if the sound is really playing
	//
	if (m_SoundHandle != NULL) {
		
		//
		//	Transform the sound's position into 'listener-space'
		//
		Vector3 sound_pos	= m_Transform.Get_Translation ();
		Vector3 rel_sound_pos;
		Matrix3D::Inverse_Transform_Vector (m_ListenerTransform, sound_pos, &rel_sound_pos);

		//
		//	Calculate a normalized pan from 0 (hard left) to 1.0F (hard right)
		//
		float angle	= WWMath::Atan2 (rel_sound_pos.Y, rel_sound_pos.X);
		float pan	= -WWMath::Fast_Sin (angle);
		pan			= (pan / 2.0F) + 0.5F;

		//
		// Pass the pan on
		//
		m_SoundHandle->Set_Sample_Pan (S32(pan * 127.0F));
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Allocate_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundPseudo3DClass::Allocate_Miles_Handle (void)
{
	AudibleSoundClass::Allocate_Miles_Handle ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Free_Miles_Handle
//
////////////////////////////////////////////////////////////////////////////////////////////////
void
SoundPseudo3DClass::Free_Miles_Handle (void)
{
	AudibleSoundClass::Free_Miles_Handle ();
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Frame_Update
//
////////////////////////////////////////////////////////////////////////////////////////////////
bool
SoundPseudo3DClass::On_Frame_Update (unsigned int milliseconds)
{	
	// If necessary, update the volume based on the distance
	// from the listener
	if (m_SoundHandle != NULL) {
		Update_Pseudo_Volume ();
		Update_Pseudo_Pan ();
	}

	// Allow the base Sound3DClass to process this call
	return Sound3DClass::On_Frame_Update (milliseconds);
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
////////////////////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
SoundPseudo3DClass::Get_Factory (void) const
{	
	return _PseudoSound3DPersistFactory;
}
