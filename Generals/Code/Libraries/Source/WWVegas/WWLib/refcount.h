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
 *                 Project Name : Commando / G Library                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/refcount.h                             $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/13/01 8:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
 
#ifndef REFCOUNT_H
#define REFCOUNT_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef LISTNODE_H
#include "LISTNODE.H"
#endif

class RefCountClass;


#ifndef NDEBUG

struct ActiveRefStruct
{
	const char *	File;
	int						Line;
};

#define	NEW_REF( C, P )						( (C*)RefCountClass::Set_Ref_Owner( W3DNEW C P, __FILE__, __LINE__ ) )
#define	SET_REF_OWNER( P )				(		RefCountClass::Set_Ref_Owner( P,       __FILE__, __LINE__ ) )

#else

#define	NEW_REF( C, P )					( W3DNEW C P )
#define	SET_REF_OWNER( P )			P

#endif


/*
** Macros for setting and releasing a pointer to a ref counted object.
** If you have a member variable which can be pointed at a ref counted object and
** you want to point it at some object.  You must release whatever it currently points at,
** point it at the new object, and add-ref the new object (if its not null...)
*/
#define REF_PTR_SET(dst,src)	{ if (src) (src)->Add_Ref(); if (dst) (dst)->Release_Ref(); (dst) = (src); }
#define REF_PTR_RELEASE(x)		{ if (x) x->Release_Ref(); x = NULL; }


/*
**  Rules regarding the use of RefCountClass
**
**		If you call a function that returns a pointer to a RefCountClass,
**			you MUST Release_Ref() it
**
**		If a functions calls you, and you return a pointer to a RefCountClass,
**			you MUST Add_Ref() it
**
**		If you call a function and pass a pointer to a RefCountClass,
**			you DO NOT Add_Ref() it
**
**		If a function calls you, and passes you a pointer to a RefCountClass,
**			if you keep the pointer, you MUST Add_Ref() and Release_Ref() it
**			otherwise, you DO NOT Add_Ref() or Release_Ref() it
**
*/

typedef DataNode<RefCountClass *>	RefCountNodeClass;
typedef List<RefCountNodeClass *>	RefCountListClass;

class RefCountClass
{
public:
	
	RefCountClass(void) :
		NumRefs(1)
		#ifndef NDEBUG
		,ActiveRefNode(this)
		#endif
	{
		#ifndef NDEBUG
		Add_Active_Ref(this);
		Inc_Total_Refs(this);
		#endif
	}

	RefCountClass(const RefCountClass & ) : 
		NumRefs(1)		
		#ifndef NDEBUG
		,ActiveRefNode(this)
		#endif
	{ 		
		#ifndef NDEBUG
		Add_Active_Ref(this);
		Inc_Total_Refs(this);
		#endif
	}

	/*
	** Add_Ref, call this function if you are going to keep a pointer
	** to this object.
	*/
#ifdef NDEBUG
	WWINLINE void Add_Ref(void) const							{ NumRefs++; }
#else
	void Add_Ref(void) const;
#endif

	/*
	** Release_Ref, call this function when you no longer need the pointer
	** to this object.
	*/
	WWINLINE void		Release_Ref(void) const					{ 
																				#ifndef NDEBUG
																				Dec_Total_Refs(this);
																				#endif
																				NumRefs--; 
																				assert(NumRefs >= 0); 
																				if (NumRefs == 0) const_cast<RefCountClass*>(this)->Delete_This(); 
																			}


	/*
	** Check the number of references to this object.  
	*/
	int					Num_Refs(void) const						{ return NumRefs; }

	/*
	** Delete_This - this function will be called when the object is being
	** destroyed as a result of its last reference being released.  Its
	** job is to actually destroy the object.
	*/
	virtual void		Delete_This(void)							{ delete this; }

	/*
	** Total_Refs - This static function can be used to get the total number
	** of references that have been made.  Once you've released all of your
	** objects, it should go to zero.  
	*/
	static int			Total_Refs(void)							{ return TotalRefs; }

protected:

	/*
	** Destructor, user should not have access to this...
	*/
	virtual ~RefCountClass(void)
	{
		#ifndef NDEBUG
		Remove_Active_Ref(this);	
		#endif
		assert(NumRefs == 0);
	}

private:

	/*
	** Current reference count of this object
	*/
	mutable int			NumRefs;

	/*
	** Sum of all references to RefCountClass's.  Should equal zero after
	** everything has been released.
	*/
	static int			TotalRefs;

	/*
	** increments the total reference count
	*/
	static void			Inc_Total_Refs(const RefCountClass *);
	
	/*
	** decrements the total reference count
	*/
	static void			Dec_Total_Refs(const RefCountClass *);

public:
	
#ifndef NDEBUG // Debugging stuff

	/*
	** Node in the Active Refs List
	*/
	RefCountNodeClass					ActiveRefNode;
	
	/*
	** Auxiliary Active Ref Data
	*/
	ActiveRefStruct					ActiveRefInfo;	

	/*
	** List of the active referenced objects
	*/
	static RefCountListClass		ActiveRefList;
	
	/*
	** Adds the ref obj pointer to the active ref list
	*/
   static RefCountClass *			Add_Active_Ref(RefCountClass *obj);

	/*
	** Updates the owner file/line for the given ref obj in the active ref list
	*/
	static RefCountClass *			Set_Ref_Owner(RefCountClass *obj,const char * file,int line);

	/*
	** Remove the ref obj from the active ref list
	*/
	static void							Remove_Active_Ref(RefCountClass * obj);

	/*
	**	Confirm the active ref object using the pointer of the refbaseclass as a search key
	*/
	static bool							Validate_Active_Ref(RefCountClass * obj);

#endif

};



#endif
