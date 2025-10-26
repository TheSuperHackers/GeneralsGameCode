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
 *                 Project Name : WWAudio.h                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWAudio/PriorityVector.h                                                                                                                                                                                                                                                                                                                               $Modtime:: 7/02/99 11:43a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "Vector.H"

////////////////////////////////////////////////////////////////////
//
//	PriorityVectorClass
//
////////////////////////////////////////////////////////////////////
template<class T>
class PriorityVectorClass : public DynamicVectorClass<T>
{
	using DynamicVectorClass<T>::Vector;
	using DynamicVectorClass<T>::ActiveCount;

	public:

		virtual bool	Process_Head (T &object);
		virtual bool	Add_Low (T const &object);
		virtual bool	Add_High (T const &object);

		/*PriorityVectorClass<T> & operator= (PriorityVectorClass<T> const & rvalue) {
			DynamicVectorClass<T>::operator= (rvalue);
			return(*t8his);
		}*/
};


////////////////////////////////////////////////////////////////////
//
//	Process_Head
//
////////////////////////////////////////////////////////////////////
template <class T>
__inline bool PriorityVectorClass<T>::Process_Head (T &object)
{
	bool retval = false;
	if (Vector != NULL) {

		// Pass the object back to the caller
		object = Vector[0];

		//
		//	Move the head object to the end of the list
		//
		for (int index = 1; index < ActiveCount; index ++) {
			Vector[index - 1] = Vector[index];
		}
		Vector[ActiveCount - 1] = object;

		// Success!
		retval = true;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////
//
//	Add_Low
//
////////////////////////////////////////////////////////////////////
template <class T>
__inline bool PriorityVectorClass<T>::Add_Low (T const &object)
{
	return DynamicVectorClass<T>::Add (object);
}


////////////////////////////////////////////////////////////////////
//
//	Add_High
//
////////////////////////////////////////////////////////////////////
template <class T>
__inline bool PriorityVectorClass<T>::Add_High (T const &object)
{
	return DynamicVectorClass<T>::Add_Head (object);
}
