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
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/SphereUtils.h       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/08/00 6:34p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef __SPHERE_UTILS
#define __SPHERE_UTILS

#include "sphereobj.h"

/////////////////////////////////////////////////////////////
//
//	SphereKeysClass
//
/////////////////////////////////////////////////////////////
class SphereKeysClass
{
public:

	/////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	/////////////////////////////////////////////////////////////
	SphereKeysClass (void)
		:	m_Keys (NULL),
			m_KeyCount (0),
			m_MaxKeys (0)		{ }

	virtual ~SphereKeysClass (void)	{ Free_Keys (); }

	/////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////
	W3dSphereKeyFrameStruct *	Detach (void);

	int								Get_Key_Count (void) const { return m_KeyCount; }
	W3dSphereKeyFrameStruct *	Get_Keys (void)				{ return m_Keys; }

	void								Add_Keys (W3dSphereKeyFrameStruct *keys, int key_count);
	void								Add_Key (W3dSphereKeyFrameStruct &key);

	void								Free_Keys (void);

protected:

	/////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////
	void								Resize (int max_keys);

private:

	/////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////
	W3dSphereKeyFrameStruct *	m_Keys;
	int								m_KeyCount;
	int								m_MaxKeys;
};


#endif //__SPHERE_UTILS

