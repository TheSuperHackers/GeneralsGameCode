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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// Override.h /////////////////////////////////////////////////////////////////////////////////////
// Electronic Arts Pacific
// Do Not Distribute

#pragma once

#include "Common/Overridable.h"

/*
	An OVERRIDE is a replacement for a pointer of its contained type, ie, rather than containing
	a LocomotorTemplate*, you would contain an OVERRIDE<LocomotorTemplate>.

	OVERRIDE pretends in all ways (dereference via *, -> and casting to type*) to be a type*, so
	there should be very little code that needs to be rewritten to work with these.

	In order to make something overridable, these are the steps:
		1) Make the desired class derive from Overridable.
		2) Make the container class contain an instance of OVERRIDE<Type>
		3) Make the newOverride function (wherever an override is new'd) request the overridables lastOverride,
			to ensure that no leaks are created.

		See LocomotorTemplate for an example.
*/

template <class T> class OVERRIDE
{
	public:
		// Provide useful constructores to go from a T* to an OVERRIDE<T>
		OVERRIDE(const T *overridable = nullptr);
		// Copy constructor
		OVERRIDE(OVERRIDE<T> &overridable);
		// Operator= for copying from another OVERRIDE and T*
		__inline OVERRIDE &operator=( const OVERRIDE<T>& override );
		__inline OVERRIDE &operator=( const T* overridable );

		// these are the methods which we can use to access data in a pointer. (Dereference*, ->, and cast
		// to T*). They are all overloaded to recurse to the lowest override and use that.
		__inline const T *operator->( void ) const;	// overload const ->
		__inline const T *operator*( void ) const;		// overload const *(dereference operator)
		__inline operator const T*( ) const;	// overload casting to (const T*)

		// this is useful in case you want to get the pointer that this object is actually looking at.
		__inline const T *getNonOverloadedPointer( void ) const;

	private:
		// Because OVERRIDE is meant to live on the object and not in the store, it currently contains
		// a constant pointer. We could change this if it seems weird.
		const T *m_overridable;
};

//-------------------------------------------------------------------------------------------------
template <class T>
OVERRIDE<T>::OVERRIDE(const T *overridable)
{
	m_overridable = overridable;
}

//-------------------------------------------------------------------------------------------------
template <class T>
OVERRIDE<T>::OVERRIDE(OVERRIDE<T> &overridable)
{
	m_overridable = overridable.m_overridable;
}

//-------------------------------------------------------------------------------------------------
template <class T>
OVERRIDE<T> &OVERRIDE<T>::operator=( const OVERRIDE<T>& override )
{
	m_overridable = override.m_overridable;
	return *this;
}

//-------------------------------------------------------------------------------------------------
template <class T>
OVERRIDE<T> &OVERRIDE<T>::operator=(const T* overridable)
{
	m_overridable = overridable;
	return *this;
}

//-------------------------------------------------------------------------------------------------
template <class T>
const T *OVERRIDE<T>::operator->() const
{
	if (!m_overridable)
		return nullptr;
	return (T*) m_overridable->getFinalOverride();
}

//-------------------------------------------------------------------------------------------------
template <class T>
const T *OVERRIDE<T>::operator*() const
{
	if (!m_overridable)
		return nullptr;
	return (T*) m_overridable->getFinalOverride();
}

//-------------------------------------------------------------------------------------------------
template <class T>
const T *OVERRIDE<T>::getNonOverloadedPointer( void ) const
{
	return (T*) m_overridable;
}

//-------------------------------------------------------------------------------------------------
template <class T>
OVERRIDE<T>::operator const T*( ) const
{
	return operator*();
}
