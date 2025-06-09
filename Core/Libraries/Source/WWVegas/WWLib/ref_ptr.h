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
 *                 Project Name : WWLIB                                                        *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwlib/ref_ptr.h                                   $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 6/13/01 2:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef REF_PTR_H
#define REF_PTR_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#include "wwdebug.h"

/*
	RefCountPtr<T> is a smart pointer for reference counted objects.
	
	  RefCountPtr<T> is designed to support objects derived from RefCountClass, although any class
	  supporting the required interface may also be used.

	  The reference counted objects must implement the following interface:
			1. new objects must be created with reference count 1.
			2. Add_Ref() to increase reference count by 1
			3. Release_Ref() to decrease reference count.  The object will be deleted when the reference
				count is reduced to 0.

		RefCountPtr<T> is designed to behave _mostly_ like a normal pointer. However, like most pointer
		wrappers, it does not support:
			1. type hierarchy relationships.  RefCountPtr<Derived> is unrelated to RefCountPtr<Base>.
				However, the templates support conversion from RefCountPtr<Derived> to RefCountPtr<Base>
				There is no support for casting upward (i.e. dynamic cast from RefCountPtr<Base> to RefCountPtr<Derived>
				This may be added in the future.
			2. const'ness.  RefCountPtr<const T> will probably misbehave.
			3. address of pointer.  There is no conversion to T**, nor should there be.
			4. direct conversion to T*.  Please keep object wrappered in RefCountPtr<T> and use dereferencing
				operators (*myptr.f() and/or myptr->f())
				You may also use Peek() to get a raw pointer.  Peek does not modify the reference count of the object

  Using RefCountPtr<T>
		Example of usage :
				
				class MyClass : public RefCountClass
				{
					public:
						static RefCountPtr<T> Create(void)
						{
							return NEW MyClass;
						}

						void Do_Something(void);
						void Do_Something_Else(void);
						bool Is_Nice() const;

					private:
						MyClass(void);
				};

				void MyOtherClass
				{
					public:
						MyOtherClass(const RefCountPtr<T> & my_object) : MyObject(my_object) {}

						void Do_It(void)
						{
							if (MyObject) {
								MyObject->Do_Something();
								*MyObject.Do_Something_Else();
							}
						}

						void Set_Object(const RefCountPtr<T> & object) {
							MyObject = object;
						}

						const RefCountPtr<T> & Get_Object(void) {
							return MyObject;
						}

					private:
						RefCountPtr<T>	MyObject;
				};

				RefCountPtr<T> Get_Nice_One(void)
				{
					do {
						RefCountPtr<T> object = MyClass::Create();
						if (object && object->Is_Nice()) {
							return object;
						}
					}
				}

		In general, a reference counted object should always be wrapped by RefCountPtr<T>.  Since the
		smart pointer provides dereferencing operators (*myptr and myptr->), there is typically no
		need to access the raw pointer.

		Code that consistently uses RefCountPtr should be safer, cleaner and easier to use than code
		than manually manages reference counts.

		RefCountPtr<T> provides a default constructor, which corresponds to a null pointer, and a copy
		constructor, which handles initialization and an assignment operator, which handles pointer
		copying.

		To create a RefCountPtr<T> from a raw pointer, use the global template functions
		Create_NoAddRef should be used when wrapping a pointer that has just been created with NEW
		Create_NoAddRef should be used when wrapping a pointer that has been returned from a "Get" function
			(the function added a reference prior to returning the pointer)
		Create_AddRef should be used when wrapping a pointer that has been returned from a "Peek" function
			(the function did not add a reference prior to returning the pointer).

		Create_NoAddRef and Create_AddRef are provided to allow old code to migrate from manual reference count
		management to RefCountPtr.  New code written with RefCountPtr should rarely if ever use
		Create_NoAddRef and Create_AddRef.

		If it is absolutely necessary to extract the raw pointer, use Peek.  Peek does not add a new
		reference to the object.  Using a Peek'd object after its RefCountPtr has gone out of scope requires
		care and typically requires the client to Add_Ref/Release_Ref the Peek'd object.

		Rewrapping and Peeking reference counted objects is primarily useful when converting old code to
		use RefCountPtr instead of manually managing the reference count.  These two functions are designed 
		for safety, NOT convenience.

		Automatic construction of a RefCountPtr from a raw pointer is enabled if 
		ALLOW_AUTOMATIC_REF_COUNT_PTR_CONSTRUCTION is defined.
		This may be useful when migrating existing code to use RefCountPtr, but is not completely safe,
		since it is not possible to determine if the pointer is being Get'd or Peek'd.
		Please note that the constructor WILL add a reference to the object, which errs on the side
		of leaking references rather than prematurely deleting objects.  Whenever possible, use the
		explicit global Create_* functions rather than the automatic conversion.

		When used...
			1. As a local variable.  use RefCountPtr<T> :
						void My_Function(void)
						{
							RefCountPtr<T> myobject = MyClass::Get();
							if (myobject) {
								myobject->Do_Something();
							}
						}
			2. As a member variable.  use RefCountPtr<T> :
						class MyClass
						{
							private:
								RefCountPtr<T> MyObject;
						};
			3. As a return value.  use RefCountPtr<T> or const RefCountPtr<T> & (member variables)
					class MyClass
					{
						public:
							RefCountPtr<T>				Make_Something(void);
							const RefCountPtr<T> &	Get_Something(void) const {return MyObject;}
						private:
							RefCountPtr<T>				MyObject;
					};
			4. As a function parameter. user const RefCountPtr<T> & or RefCountPtr<T> & (for modifiable parameter)
					void My_Function(const RefCountPtr<T> & my_parameter)
					{
						my_parameter->Do_Something();
					}

					class MyClass
					{
						public:
							void Set(const RefCountPtr<T> & newthing)
							{
								MyThing = newthing;
							}

							// Get using an OUT parameter
							void Get(RefCountPtr<T> & thing)
							{
								thing = MyThing;								
							}

							// Get using a return value.  Preferable to above
							const RefCountPtr<T> & Get(void) const {return MyThing;}

						private:
							RefCountPtr<T> MyThing;
					};
*/


class DummyPtrType;

template <class T>
class RefCountPtr
{
	public:

		// Creates a RefCountPtr<T> and does not increment the reference counter of the passed object.
		// Is generally used for objects returned by operator new and "Get" functions.
		static RefCountPtr<T> Create_NoAddRef(T *t)
		{
			WWASSERT(t == NULL || t->Num_Refs() >= 1);
			return RefCountPtr<T>(t, RefCountPtr<T>::GET);
		}

		// Creates a RefCountPtr<T> and increments the reference counter of the passed object.
		// Is generally used for objects returned by "Peek" functions.
		static RefCountPtr<T> Create_AddRef(T *t)
		{
			return RefCountPtr<T>(t, RefCountPtr<T>::PEEK);
		}

		RefCountPtr(void) 
			: Referent(0)
		{
		}

#ifdef ALLOW_AUTOMATIC_REF_COUNT_PTR_CONSTRUCTION
		RefCountPtr(T * referent)
			: Referent(referent)
		{
			if (Referent) {
				Referent->Add_Ref();
			}
		}
#else
		// This allows construction of the smart pointer from 0 (null)
		// Without allows unwanted conversions from T * (and related types, including void *)
		RefCountPtr(DummyPtrType * dummy)
			: Referent(0)
		{
			WWASSERT(dummy == 0);
		}
#endif

		template <class RHS>
			RefCountPtr(const RefCountPtr<RHS> & rhs)
			: Referent(rhs.Peek())
		{
			if (Referent) {
				Referent->Add_Ref();
			}
		}

		RefCountPtr(const RefCountPtr & rhs)
			: Referent(rhs.Referent)
		{
			if (Referent) {
				Referent->Add_Ref();
			}
		}

#ifdef ALLOW_AUTOMATIC_REF_COUNT_PTR_CONSTRUCTION
		const RefCountPtr<T> & operator =(T * object)
		{
			if (Referent == object) {
				return *this;
			}

			Referent = object;

			if (Referent) {
				Referent->Add_Ref();
			}

			return *this;
		}
#else
		const RefCountPtr<T> & operator =(DummyPtrType * dummy_ptr)
		{
			if (Referent) {
				Referent->Release_Ref();
			}

			Referent = 0;

			return *this;
		}
#endif

		template <class RHS>
		const RefCountPtr<T> & operator =(const RefCountPtr<RHS> & rhs)
		{
			if (rhs.Peek()) {
				rhs.Peek()->Add_Ref();
			}

			if (Referent) {
				Referent->Release_Ref();
			}

			Referent = rhs.Peek();

			return *this;
		}

		const RefCountPtr<T> & operator =(const RefCountPtr & rhs)
		{
			if (rhs.Referent) {
				rhs.Referent->Add_Ref();
			}


			if (Referent) {
				Referent->Release_Ref();
			}

			Referent = rhs.Referent;
			return *this;
		}

		~RefCountPtr(void)
		{
			if (Referent) {
				Referent->Release_Ref();
				Referent = 0;
			}
		}

		// This strange conversion allows us to test pointers against null (0) without
		// providing an unsafe conversion to T * (very unsafe) or bool (which can be silently
		//  converted to int, defeating type-safety in some cases).
		//  The compiler will convert our smart pointer to this raw pointer of an undefined
		// class automatically when trying to compare against 0 or with !my_ptr
		// However, the compiler will not perform conversions from DummyPtrType *
		//  (except to void *, which is probably acceptable).
		operator const DummyPtrType *(void) const
		{
			return (DummyPtrType *)(Referent);
		}

		void Clear(void)
		{
			if (Referent) {
				Referent->Release_Ref();
				Referent = 0;
			}
		}

		T * operator ->(void) const
		{
			return Referent;
		}

		T & operator *(void) const
		{
			WWASSERT(0 != Referent);
			return *Referent;
		}

		// Note : This should typically only be used when mixing code that uses RefCountPtr and 
		//   manually managed ref counts on raw points.
		// Code that consistently uses RefCountPtr should never get a hold of a raw T*
		T * Peek(void) const
		{
			return Referent;
		}

		// Releases the held pointer without changing its reference counter.
		T * Release(void)
		{
			T * p = Referent;
			Referent = 0;
			return p;
		}

	private:
		enum ReferenceHandling { GET, PEEK };

		RefCountPtr(T * referent, ReferenceHandling reference_handling)
			: Referent(referent)
		{
			if (reference_handling == PEEK && 0 != referent) {
				referent->Add_Ref();
			}
		}

		T * Referent;
};

// LHS and RHS should be related or compiler will barf
// this follows same rules as LHS * lhs; RHS * rhs; lhs==rhs;
template <class LHS, class RHS>
bool operator ==(const RefCountPtr<LHS> & lhs, const RefCountPtr<RHS> & rhs)
{
	return lhs.Peek() == rhs.Peek();
}

template <class LHS, class RHS>
bool operator <(const RefCountPtr<LHS> & lhs, const RefCountPtr<RHS> & rhs)
{
	return lhs.Peek() < rhs.Peek();
}

// This comparison allows us to test our smart pointer against 0 using
//  0 == my_ptr
template <class RHS>
bool operator ==(DummyPtrType * dummy, const RefCountPtr<RHS> & rhs)
{
	if (0 != dummy) {
		WWASSERT(0);
		return false;
	}

	return 0 == rhs.Peek();
}

// This comparison allows us to test our smart pointer against 0 using
//  0 != my_ptr
template <class RHS>
bool operator !=(DummyPtrType * dummy, const RefCountPtr<RHS> & rhs)
{
	if (0 != dummy) {
		WWASSERT(0);
		return true;
	}

	return 0 != rhs.Peek();	
}

template <class Derived, class Base>
RefCountPtr<Derived> Static_Cast(const RefCountPtr<Base> & base) 
{
	return RefCountPtr<Derived>::Create_AddRef((Derived *)base.Peek());
}

#endif
