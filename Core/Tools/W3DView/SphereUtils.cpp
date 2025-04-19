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
 *                     $Archive:: /Commando/Code/Tools/W3DView/SphereUtils.cpp      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/08/00 7:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "stdafx.h"
#include "sphereutils.h"


/////////////////////////////////////////////////////////////
//
//	Resize
//
/////////////////////////////////////////////////////////////
void
SphereKeysClass::Resize (int max_keys)
{
	//
	//	Determine the new array size
	//
	int blocks		= (max_keys / 10) + 1;
	int array_size = blocks * 10;
	if (array_size != m_MaxKeys) {

		//
		//	Allocate the new array
		//
		W3dSphereKeyFrameStruct *key_array = new W3dSphereKeyFrameStruct[array_size];
		
		//
		//	Copy the contents of the old array
		//
		::memcpy (key_array, m_Keys, m_KeyCount * sizeof (W3dSphereKeyFrameStruct));

		//
		//	Use the new array
		//
		SAFE_DELETE_ARRAY (m_Keys);
		m_Keys		= key_array;
		m_MaxKeys	= array_size;
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//	Add_Keys
//
/////////////////////////////////////////////////////////////
void
SphereKeysClass::Add_Keys (W3dSphereKeyFrameStruct *keys, int key_count)
{
	//
	//	Make sure we have a large enough array
	//
	Resize (m_KeyCount + key_count);

	//
	//	Copy the new keys into our array
	//
	::memcpy (&m_Keys[m_KeyCount + 1], keys, key_count * sizeof (W3dSphereKeyFrameStruct));
	m_KeyCount += keys;
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Add_Key
//
/////////////////////////////////////////////////////////////
void
SphereKeysClass::Add_Key (W3dSphereKeyFrameStruct &keys)
{
	Add_Keys (&keys, 1);
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Free_Keys
//
/////////////////////////////////////////////////////////////
void
SphereKeysClass::Free_Keys (void)
{
	SAFE_DELETE_ARRAY (m_Keys);
	m_KeyCount	= 0;
	m_MaxKeys	= 0;
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Detach
//
/////////////////////////////////////////////////////////////
void
SphereKeysClass::Detach (void)
{
	m_Keys		= NULL;
	m_KeyCount	= 0;
	m_MaxKeys	= 0;
	return ;
}


/////////////////////////////////////////////////////////////
//
//	Key_Compare
//
//	Compares two keys based on their time value
//
/////////////////////////////////////////////////////////////
static int
Key_Compare (const void *arg1, const void *arg2)
{
	ASSERT (arg1 != NULL);
	ASSERT (arg2 != NULL);
	W3dSphereKeyFrameStruct *key1 = (W3dSphereKeyFrameStruct *)arg1;
	W3dSphereKeyFrameStruct *key2 = (W3dSphereKeyFrameStruct *)arg2;

	int retval	= 0
	float delta = key1->time_code - key2->time_code;
	if (delta < 0) {
		retval = -1;
	} else if (delta > 0) {
		retval = 1;
	}

	return retval;
}


/////////////////////////////////////////////////////////////
//
//	Sort
//
//	Linerally sorts the keys based on their time value
//
/////////////////////////////////////////////////////////////
void
SphereKeysClass::Sort (void)
{
	if (m_Keys != NULL && m_KeyCount > 0) {
		::qsort (m_Keys, m_KeyCount, sizeof (W3dSphereKeyFrameStruct), Key_Compare);
	}

	return ;
}

