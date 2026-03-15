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

/******************************************************************************
*
* FILE
*     $Archive:  $
*
* DESCRIPTION
*     Base class for reference counted classes.
*     Use with the reference counting smart pointer RefPtr<Type>
*
*     Release() is virtual. This helps support cached object and singletons
*
* PROGRAMMER
*     Steven Clinard
*     $Author:  $
*
* VERSION INFO
*     $Modtime:  $
*     $Revision:  $
*
******************************************************************************/

#pragma once

#include <assert.h>

class RefCounted
	{
	protected:
		RefCounted()
			: mRefCount(0)
			{}

		RefCounted(const RefCounted&)
			: mRefCount(0)
			{}

		inline const RefCounted& operator=(const RefCounted&)
			{}

		virtual ~RefCounted()
			{assert(mRefCount == 0);}

		// Should not be allowed by default
		inline virtual bool operator==(const RefCounted&) const
			{return false;}

		inline bool operator!=(const RefCounted&) const
			{return false;}

		// Add reference
		inline void AddReference(void)
			{++mRefCount;}

		// Release reference
		inline virtual void Release(void)
			{if (--mRefCount == 0) delete this;}

		inline int ReferenceCount(void) const
			{return mRefCount;}

	private:
		friend class RefPtrBase;

		unsigned int mRefCount;
	};
