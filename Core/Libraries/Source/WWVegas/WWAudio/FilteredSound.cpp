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
 *                     $Archive:: /Commando/Code/WWAudio/FilteredSound.cpp          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/13/01 3:03p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "FilteredSound.h"
#include "WWAudio.h"
#include "SoundScene.h"
#include "SoundChunkIDs.h"
#include "persistfactory.h"
#include "soundhandle.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	Static factories
//
//////////////////////////////////////////////////////////////////////////////////
SimplePersistFactoryClass<FilteredSoundClass, CHUNKID_FILTERED_SOUND> _FilteredSoundPersistFactory;


/////////////////////////////////////////////////////////////////////////////////
//
//	FilteredSoundClass
//
/////////////////////////////////////////////////////////////////////////////////
FilteredSoundClass::FilteredSoundClass (void)
	:	m_hFilter ((HPROVIDER)INVALID_MILES_HANDLE)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FilteredSoundClass
//
////////////////////////////////////////////////////////////////////////////////////////////////
FilteredSoundClass::FilteredSoundClass (const FilteredSoundClass &src)
	:	m_hFilter ((HPROVIDER)INVALID_MILES_HANDLE),
		SoundPseudo3DClass (src)
{
	(*this) = src;
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	~FilteredSoundClass
//
/////////////////////////////////////////////////////////////////////////////////
FilteredSoundClass::~FilteredSoundClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////////////
//
//	operator=
//
////////////////////////////////////////////////////////////////////////////////////////////////
const FilteredSoundClass &
FilteredSoundClass::operator= (const FilteredSoundClass &src)
{
	// Call the base class
	SoundPseudo3DClass::operator= (src);
	return (*this);
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Miles_Handle
//
/////////////////////////////////////////////////////////////////////////////////
void
FilteredSoundClass::Initialize_Miles_Handle (void)
{
	SoundPseudo3DClass::Initialize_Miles_Handle ();
	m_hFilter = WWAudioClass::Get_Instance ()->Get_Reverb_Filter ();
	if ((m_SoundHandle != NULL) &&
		 (m_hFilter != (HPROVIDER)INVALID_MILES_HANDLE)) {

		//
		//	Pass the filter onto the sample
		//
		::AIL_set_sample_processor (m_SoundHandle->Get_HSAMPLE (), DP_FILTER, m_hFilter);

		//
		//	Change the reverb's settings to simulate a 'tinny' effect.
		//
		F32 reverb_level   = 0.3F;
		F32 reverb_reflect = 0.01F;
		F32 reverb_decay   = 0.535F;
		::AIL_set_filter_sample_preference (m_SoundHandle->Get_HSAMPLE (),
														"Reverb level",
														&reverb_level);

		::AIL_set_filter_sample_preference (m_SoundHandle->Get_HSAMPLE (),
														"Reverb reflect time",
														&reverb_reflect);

		::AIL_set_filter_sample_preference (m_SoundHandle->Get_HSAMPLE (),
														"Reverb decay time",
														&reverb_decay);
	}

	Update_Volume ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Update_Volume
//
/////////////////////////////////////////////////////////////////////////////////
void
FilteredSoundClass::Update_Volume (void)
{
	if (m_SoundHandle != NULL) {

		// Determine the listener's position and the sound's position
		SoundSceneClass *scene = WWAudioClass::Get_Instance ()->Get_Sound_Scene ();
		Listener3DClass *listener = scene->Peek_2nd_Listener ();
		if (listener != NULL) {
			Vector3 listener_pos = listener->Get_Position ();
			Vector3 sound_pos = m_Transform.Get_Translation ();

			// Determine a normalized volume from the position
			float distance = (sound_pos - listener_pos).Quick_Length ();
			Update_Pseudo_Volume (distance);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////////
//
//	Get_Factory
//
/////////////////////////////////////////////////////////////////////////////////
const PersistFactoryClass &
FilteredSoundClass::Get_Factory (void) const
{
	return _FilteredSoundPersistFactory;
}

