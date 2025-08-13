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
 *                 Project Name : ww3d2																		  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/animatedsoundmgr.h                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/13/01 6:05p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//
// MBL Update for CNC3 INCURSION - 10.23.2002 - Expanded param handling, Added STOP command
//

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __ANIMATEDSOUNDMGR_H
#define __ANIMATEDSOUNDMGR_H

#include "simplevec.h"
#include "Vector.H"
#include "hashtemplate.h"


//////////////////////////////////////////////////////////////////////
//	Forward declarations
//////////////////////////////////////////////////////////////////////
class HTreeClass;
class HAnimClass;
class Matrix3D;
class SoundLibraryBridgeClass;

//////////////////////////////////////////////////////////////////////
//
//	AnimatedSoundMgrClass
//
//////////////////////////////////////////////////////////////////////
class AnimatedSoundMgrClass
{
public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////

	//
	//	Initialization and shutdown
	//
	static void		Initialize (const char *ini_filename = NULL);
	static void		Shutdown (void);

	//
	//	Sound playback
	//
	static const char*	Get_Embedded_Sound_Name (HAnimClass *anim);
	static float			Trigger_Sound (HAnimClass *anim, float old_frame, float new_frame, const Matrix3D &tm);

	// Bridges E&B code with WW3D.
	static void		Set_Sound_Library(SoundLibraryBridgeClass* library);

private:

	///////////////////////////////////////////////////////////////////
	//	Private data types
	///////////////////////////////////////////////////////////////////
	struct AnimSoundInfo
	{
		AnimSoundInfo() : Frame(0), SoundName(), Is2D(false), IsStop(false) {}
		int			Frame;
		StringClass	SoundName;
		bool			Is2D;
		bool			IsStop;
	};

	typedef AnimSoundInfo								ANIM_SOUND_INFO;

	struct AnimSoundList
	{
		AnimSoundList() : List(), BoneName("root") {}
		~AnimSoundList()
		{
			for (int i = 0; i < List.Count(); i++) {
				delete List[i];
			}
		}
		void	Add_Sound_Info(ANIM_SOUND_INFO* info) {List.Add(info);}

		SimpleDynVecClass<ANIM_SOUND_INFO*>	List;
		StringClass									BoneName;
	};

	typedef AnimSoundList								ANIM_SOUND_LIST;

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	static HashTemplateClass<StringClass, ANIM_SOUND_LIST *> AnimationNameHash;
	static DynamicVectorClass<ANIM_SOUND_LIST *>					AnimSoundLists;

	static SoundLibraryBridgeClass*									SoundLibrary;

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
	static ANIM_SOUND_LIST *	Find_Sound_List (HAnimClass *anim);
};


#endif //__ANIMATEDSOUNDMGR_H
