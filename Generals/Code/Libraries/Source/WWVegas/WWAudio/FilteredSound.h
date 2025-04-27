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
 *                     $Archive:: /Commando/Code/WWAudio/FilteredSound.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/11/00 12:01p                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __FILTERED_SOUND_H
#define __FILTERED_SOUND_H


#include "SoundPseudo3D.h"


/////////////////////////////////////////////////////////////////////////////////
//
//	FilteredSoundClass
//
//	Sound object that applies the specified 'reverb' filter so as to sound 'tinny'.
//
/////////////////////////////////////////////////////////////////////////////////
class FilteredSoundClass : public SoundPseudo3DClass
{
	public:

		//////////////////////////////////////////////////////////////////////
		//	Public constructors/destructors
		//////////////////////////////////////////////////////////////////////
		FilteredSoundClass (const FilteredSoundClass &src);
		FilteredSoundClass (void);
		virtual ~FilteredSoundClass (void);

		//////////////////////////////////////////////////////////////////////
		//	Public operators
		//////////////////////////////////////////////////////////////////////
		const FilteredSoundClass &operator= (const FilteredSoundClass &src);

		//////////////////////////////////////////////////////////////////////
		//	Identification methods
		//////////////////////////////////////////////////////////////////////
		virtual SOUND_CLASSID	Get_Class_ID (void) { return CLASSID_FILTERED; }

		//////////////////////////////////////////////////////////////////////
		//	Conversion methods
		//////////////////////////////////////////////////////////////////////		
		virtual FilteredSoundClass *	As_FilteredSoundClass (void) { return this; }

		//////////////////////////////////////////////////////////////////////
		//	Volume control
		//////////////////////////////////////////////////////////////////////
		virtual void				Update_Volume (void);

		// From PersistClass
		const PersistFactoryClass &	Get_Factory (void) const;

	protected:

		//////////////////////////////////////////////////////////////////////
		//	Handle information
		//////////////////////////////////////////////////////////////////////				
		virtual void				Initialize_Miles_Handle (void);

	private:

		//////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////
		HPROVIDER    m_hFilter;
};


#endif //__FILTERED_SOUND_H

