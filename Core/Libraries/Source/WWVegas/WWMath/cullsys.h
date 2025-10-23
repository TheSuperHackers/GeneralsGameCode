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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/cullsys.h                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/08/01 6:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "wwdebug.h"
#include "stdlib.h"
#include "refcount.h"
#include "aabox.h"

class CullableClass;
class CullSystemClass;
class FrustumClass;

/*
** CullLinkClass
** This class will serve as a base class for the various types of linkage information
** that the Cullable instances will need.  Each CullableClass will have a pointer to
** a class derived from CullLinkClass where the different culling systems can store
** things.
*/
class CullLinkClass
{
public:
	WWINLINE CullLinkClass(CullSystemClass * system)								{ System = system; WWASSERT(System); }
	virtual ~CullLinkClass(void)												{ WWASSERT(System == NULL); }

	WWINLINE void					Set_Culling_System(CullSystemClass * sys)		{ System = sys; }
	WWINLINE CullSystemClass * Get_Culling_System(void)							{ return System; }

protected:
	CullSystemClass * System;
};


/*
** CullableClass
** This is the base class for any object that can be inserted into a culling system
** This class provides an axis aligned bounding box and some linkage variables which
** allow it to be processed by any culling system.
*/
class CullableClass : public RefCountClass
{
public:

	CullableClass(void);
	virtual ~CullableClass(void);

	/*
	** Access to the culling box for this object.  When you set the cull box, you are
	** basically guaranteeing that the object is contained within the given box.  The
	** object will automatically be updated in whatever culling system it is currently
	** contained in (if any)
	*/
	WWINLINE const AABoxClass & Get_Cull_Box(void) const							{ return CullBox; }
	void								Set_Cull_Box(const AABoxClass & box,bool just_loaded = false);

	/*
	** These functions are used by various culling systems to manage the linkage
	** pointers.  *The average user should NEVER call these*
	*/
	void								Set_Culling_System(CullSystemClass * sys);
	CullSystemClass *				Get_Culling_System(void) const;
	WWINLINE void					Set_Cull_Link(CullLinkClass * c)					{ CullLink = c; }
	WWINLINE CullLinkClass *	Get_Cull_Link(void) const							{ return CullLink; }

private:

	WWINLINE void					Set_Next_Collected(CullableClass * c)			{ NextCollected = c; }
	WWINLINE CullableClass *	Get_Next_Collected(void)							{ return NextCollected; }

	/*
	** Culling Data
	** Each object can be linked into various types of culling systems.
	** Each culling system can use its own linkage data structure (derived
	** from CullLinkClass) to keep track of the object.  The CullData pointer
	** will point to one of the culling link objects and NULL
	** if its not in any system.
	*/
	CullLinkClass *				CullLink;

	/*
	** Bounding Box
	** Any objects derived from Cullable should update the bounding box
	** whenever the object moves or changes size.  In order to do this,
	** call Set_Cull_Box...
	*/
	AABoxClass						CullBox;

	/*
	** NextCollected
	** This pointer is used by the culling system to keep a singly linked
	** list of cullable object that have been "collected".
	*/
	CullableClass *				NextCollected;

	// Not Implemented:
	CullableClass(const CullableClass & src);
	CullableClass & operator = (const CullableClass & src);

	friend class CullSystemClass;
};




/*
** CullSystemClass
** Base class of any culling system.  This interface exists so that things can
** be shuffled around without having explicit knowledge of what system they are in.
*/
class CullSystemClass
{
public:

	CullSystemClass(void);
	virtual ~CullSystemClass(void);

	/*
	** Collect_Objects.  Updates the internal collection list with the
	** objects that overlap the given primitive.
	** WARNING: This builds an internal list that is only valid until
	** another list is built, only one list can be valid at any time.
	** WARNING: Always call Reset_Collection if you want to start a
	** fresh collection!
	*/
	void					Reset_Collection(void);
	virtual void		Collect_Objects(const Vector3 & point)					= 0;
	virtual void		Collect_Objects(const AABoxClass & box)				= 0;
	virtual void		Collect_Objects(const OBBoxClass & box)				= 0;
	virtual void		Collect_Objects(const FrustumClass & frustum)		= 0;

	/*
	** This object has moved or changed size, update it
	*/
	virtual void		Update_Culling(CullableClass * obj)						= 0;

protected:

	/*
	** Iterate through the collected objects
	*/
	CullableClass *	Get_First_Collected_Object_Internal(void);
	CullableClass *	Get_Next_Collected_Object_Internal(CullableClass * obj);
	CullableClass *	Peek_First_Collected_Object_Internal(void);
	CullableClass *	Peek_Next_Collected_Object_Internal(CullableClass * obj);

	/*
	** Build the list of collected objects
	*/
	void					Add_To_Collection(CullableClass * obj);

	/*
	** Pointer to the head of the current collection of objects
	*/
	CullableClass *	CollectionHead;

	friend class CullableClass;
};
