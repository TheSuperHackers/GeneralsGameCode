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
*     RefPtr<> and RefPtrConst<> are const-friendly, polymorphic reference
*     counting smart pointers.
*
*     The pointed-to class must be derived from RefCount.
*
*     RefPtr<X> replaces X*
*     RefPtrConst<X> replaces const X*
*
*     Dynamic_Cast<X> replaces dynamic_cast<X*> and dynamic_cast<X&>
*     Reinterpret_Cast<X> replaces reinterpret_cast<X*> and reinterpret_cast<X&>
*     Const_Cast<X> replaces const_cast<X*> and const_cast<X&>
*
*     IsValid() replaces (x != NULL)
*
*     Member function Attach() or assigning RefPtr<X>() will NULL a pointer.
*
*     Generally, RefPtr<> and RefPtrConst<> behave like their raw pointer
*     counterparts, except of course they are reference counted and will delete
*     the pointed-to object when the last reference is lost. The major
*     syntatical differences are the use of RefPtrConst<> to represent a pointer
*     to a constant object (I found it impossible to represent this within rc_ptr)
*     and the use of the upper-case cast functions (it is not possible to overload
*     these built-in functions).
*
*     An explicit goal of this class is to completely avoid the "new" and "delete"
*     operators in client code. The constructors for these pointers are private;
*     they are friends of the pointed-to class. This forces the use of Factory
*     Method functions (or similar) in the pointed-to class. Pointed-to classes
*     should make the constructor protected or private to disallow clients from
*     creating an instance with "new". If this is done, it becomes very difficult
*     for the client to accidentally leak objects and/or misuse the pointed-to
*     class or the reference counting pointers.
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

#ifndef REFPTR_H
#define REFPTR_H

#include "Visualc.h"
#include "RefCounted.h"
#include <stddef.h>
#include <assert.h>

template<typename Type> class RefPtr;
template<typename Type> class RefPtrConst;

class RefPtrBase
	{
	public:
		inline bool operator==(const RefPtrBase& rhs) const
			{return (mRefObject == rhs.mRefObject);}

		inline bool operator!=(const RefPtrBase& rhs) const
			{return !operator==(rhs);}

		inline bool IsValid(void) const
			{return (mRefObject != NULL);}

		inline void Detach(void)
			{
			if (IsValid())
				{
				mRefObject->Release();
				mRefObject = NULL;
				}
			}

	protected:
		RefPtrBase()
			: mRefObject(NULL)
			{}

		RefPtrBase(RefCounted* object)
			: mRefObject(object)
			{
			assert((mRefObject == NULL) || (mRefObject->mRefCount == 0));

			if (IsValid())
				{
				mRefObject->AddReference();
				}
			}

		RefPtrBase(const RefPtrBase& object)
			: mRefObject(object.mRefObject)
			{
			assert(false); // why is this being called?

			if (IsValid())
				{
				mRefObject->AddReference();
				}
			}

		virtual ~RefPtrBase()
			{Detach();}

		const RefPtrBase& operator=(const RefPtrBase&);

		inline RefCounted* const GetRefObject(void)
			{return mRefObject;}

		inline const RefCounted* const GetRefObject(void) const
			{return mRefObject;}

		inline void Attach(RefCounted* object)
			{
			// If objects are different
			if (object != mRefObject)
				{
				// Add reference to new object
				if (object != NULL)
					{
					object->AddReference();
					}

				// Release reference to old object
				Detach();

				// Assign new object
				mRefObject = object;
				}
			}

	private:
		RefCounted* mRefObject;

		template<typename Derived>
		friend RefPtr<Derived> Dynamic_Cast(RefPtrBase&);

		template<typename Type>
		friend RefPtr<Type> Reinterpret_Cast(RefPtrBase&);
	};


template<typename Type> class RefPtr
	: public RefPtrBase
	{
	public:
		RefPtr()
			: RefPtrBase()
			{}

		template<typename Derived>
		RefPtr(const RefPtr<Derived>& derived)
			: RefPtrBase()
			{
			Attach(const_cast<Derived*>(derived.ReferencedObject()));
			}

		RefPtr(const RefPtr<Type>& object)
			: RefPtrBase()
			{
			Attach(const_cast<Type*>(object.ReferencedObject()));
			}

		virtual ~RefPtr()
			{}

		template<typename Derived>
		inline const RefPtr<Type>& operator=(const RefPtr<Derived>& derived)
			{
			Attach(const_cast<Derived*>(derived.ReferencedObject()));
			return *this;
			}

		inline const RefPtr<Type>& operator=(const RefPtr<Type>& object)
			{
			Attach(const_cast<Type*>(object.ReferencedObject()));
			return *this;
			}

		inline Type& operator*() const
			{
			assert(IsValid());
			return *const_cast<Type*>(ReferencedObject());
			}

		inline Type* const operator->() const
			{
			assert(IsValid());
			return const_cast<Type*>(ReferencedObject());
			}

		// These are public mostly because I can't seem to declare rc_ptr<Other> as a friend
		inline Type* const ReferencedObject(void)
			{return reinterpret_cast<Type*>(GetRefObject());}

		inline const Type* const ReferencedObject(void) const
			{return reinterpret_cast<const Type*>(GetRefObject());}

		RefPtr(Type* object)
			: RefPtrBase()
			{
			Attach(object);
			}

		inline const RefPtr<Type>& operator=(Type* object)
			{
			Attach(object);
			return *this;
			}

	private:
		friend RefPtr<Type> Dynamic_Cast(RefPtrBase&);
		friend RefPtr<Type> Reinterpret_Cast(RefPtrBase&);
		friend RefPtr<Type> Const_Cast(RefPtrConst<Type>&);
	};


template<typename Type> class RefPtrConst
	: public RefPtrBase
	{
	public:
		RefPtrConst()
			: RefPtrConst()
			{}

		template<typename Derived>
		RefPtrConst(const RefPtr<Derived>& derived)
			: RefPtrBase()
			{
			Attach(derived.ReferencedObject());
			}

		RefPtrConst(const RefPtr<Type>& object)
			: RefPtrBase()
			{
			Attach(const_cast<Type* const >(object.ReferencedObject()));
			}

		template<typename Derived>
		RefPtrConst(const RefPtrConst<Derived>& derived)
			: RefPtrBase()
			{
			Attach(derived.ReferencedObject());
			}

		RefPtrConst(const RefPtrConst<Type>& object)
			: RefPtrBase()
			{
			Attach(object.ReferencedObject());
			}

		template<typename Derived>
		inline const RefPtrConst<Type>& operator=(const RefPtr<Derived>& derived)
			{
			Attach(derived.ReferencedObject());
			return *this;
			}

		inline const RefPtrConst<Type>& operator=(const RefPtr<Type>& object)
			{
			Attach(object.ReferencedObject());
			return *this;
			}

		template<typename Derived>
		inline const RefPtrConst<Type>& operator=(const RefPtrConst<Derived>& derived)
			{
			Attach(derived.ReferencedObject());
			return *this;
			}

		inline const RefPtrConst<Type>& operator=(const RefPtrConst<Type>& object)
			{
			Attach(object.ReferencedObject());
			return *this;
			}

		virtual ~RefPtrConst()
			{}

		inline const Type& operator*() const
			{
			assert(IsValid());
			return *ReferencedObject();
			}

		inline const Type* const operator->() const
			{
			assert(IsValid());
			return ReferencedObject();
			}

		// This is public mostly because I can't seem to declare rc_ptr<Other> as a friend
		inline const Type* const ReferencedObject() const
			{return reinterpret_cast<const Type*>(GetRefObject());}

		RefPtrConst(const Type* object)
			: RefPtrBase()
			{
			Attach(object);
			}

		const RefPtrConst<Type>& operator=(const Type* object)
			{
			Attach(object);
			}
	};


template<typename Derived>
RefPtr<Derived> Dynamic_Cast(RefPtrBase& base)
	{
	RefPtr<Derived> derived;
	derived.Attach(base.GetRefObject());
	return derived;
	}


template<typename Type>
RefPtr<Type> Reinterpret_Cast(RefPtrBase& rhs)
	{
	RefPtr<Type> object;
	object.Attach(rhs.GetRefObject());
	return object;
	}


template<typename Type>
RefPtr<Type> Const_Cast(RefPtrConst<Type>& rhs)
	{
	RefPtr<Type> object;
	object.Attach(rhs.ReferencedObject());
	return object;
	}

#endif // RC_PTR_H
