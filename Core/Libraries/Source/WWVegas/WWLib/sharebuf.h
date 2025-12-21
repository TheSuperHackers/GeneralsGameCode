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

/***************************************************************************
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : G                                        *
 *                                                                         *
 *                     $Archive:: /Commando/Code/wwlib/sharebuf.h         $*
 *                                                                         *
 *                      $Author:: Greg_h                                  $*
 *                                                                         *
 *                     $Modtime:: 3/20/01 1:24p                           $*
 *                                                                         *
 *                    $Revision:: 8                                       $*
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"


/*
** SharedBufferClass - a templatized class for buffers which are shared
** between different objects. This is essentially just a C array with a
** refcounted wrapper (also a count).
*/
template <class T>
class ShareBufferClass : public W3DMPO, public RefCountClass
{
	W3DMPO_GLUE(ShareBufferClass)
	public:
		ShareBufferClass(int count, const char* msg);
		ShareBufferClass(const ShareBufferClass & that);
		~ShareBufferClass(void);

		// Get the internal pointer to the array
		// CAUTION! This pointer is not refcounted so only use it in a context
		// where you are keeping a reference to the enclosing ShareBufferClass
		// to avoid the possibility of a dangling pointer.
		T *			Get_Array(void)	{ return Array; }
		int			Get_Count(void)	{ return Count; }

		// Access to the elements in the array
		void			Set_Element(int index, const T & thing);
		const T &	Get_Element(int index) const;
		T &			Get_Element(int index);

		// Clear the memory in this array.
		// CAUTION! Be careful calling this if 'T' is a class.  You could be wiping out
		// virtual function table pointers.  Not a good idea to memset 0 over the top of
		// an array of objects but useful if you are creating an array of some basic type
		// like pointers or ints...
		void			Clear(void);

	protected:

#if defined(RTS_DEBUG)
		const char* Msg;
#endif
		T *			Array;
		int			Count;

		// not implemented!
		ShareBufferClass & operator = (const ShareBufferClass &);
};

template <class T>
ShareBufferClass<T>::ShareBufferClass(int count, const char* msg) :
	Count(count)
#if defined(RTS_DEBUG)
	, Msg(msg)
#endif
{
	assert(Count > 0);
	Array = MSGW3DNEWARRAY(msg) T[Count];
}

template <class T>
ShareBufferClass<T>::ShareBufferClass(const ShareBufferClass<T> & that) :
	Count(that.Count)
{
	assert(Count > 0);
#if defined(RTS_DEBUG)
	Msg = that.Msg;
#endif
	Array = MSGW3DNEWARRAY(Msg) T[Count];
	for (int i=0; i<Count; i++) {
		Array[i] = that.Array[i];
	}
}

template <class T>
ShareBufferClass<T>::~ShareBufferClass(void)
{
	delete[] Array;
	Array = nullptr;
}

template<class T>
void ShareBufferClass<T>::Set_Element(int index,const T & thing)
{
	assert(index >= 0);
	assert(index < Count);
	Array[index] = thing;
}

template<class T>
const T& ShareBufferClass<T>::Get_Element(int index) const
{
	return Array[index];
}

template<class T>
T& ShareBufferClass<T>::Get_Element(int index)
{
	return Array[index];
}

template<class T>
void ShareBufferClass<T>::Clear(void)
{
	memset(Array,0,Count * sizeof(T));
}
