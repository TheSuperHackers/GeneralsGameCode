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
 *                     $Archive:: /VSS_Sync/wwmath/cullsys.cpp                                $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/16/00 11:42a                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "cullsys.h"
#include "wwdebug.h"
#include "wwprofile.h"


/*************************************************************************
**
** CullableClass Implementation
**
*************************************************************************/
CullableClass::CullableClass(void) :
	CullLink(NULL),
	NextCollected(NULL)
{
	CullBox.Init(Vector3(0,0,0),Vector3(1,1,1));
}

CullableClass::~CullableClass(void)
{
	// the cull system that contains us is responsible for any culling link
	// so we better be out of it and it should have cleared our pointer before
	// we are deleted.
	WWASSERT(CullLink == NULL);
}

void CullableClass::Set_Cull_Box(const AABoxClass & box,bool just_loaded)
{
	CullBox = box;
	
	WWPROFILE("Cullable::Set_Cull_Box");

	// Just_loaded flag allows us to update the box without notifying the
	// culling system.  Use this when you've saved and loaded the linkage 
	// so you know you're in the right node of the culling system...
	if (!just_loaded) {
		CullSystemClass * sys = Get_Culling_System();
		if (sys != NULL) {
			sys->Update_Culling(this);
		}
	}
}

void CullableClass::Set_Culling_System(CullSystemClass * sys)
{
	if (CullLink) {
		CullLink->Set_Culling_System(sys);
	}
}

CullSystemClass * CullableClass::Get_Culling_System(void) const
{
	if (CullLink) {
		return CullLink->Get_Culling_System();
	}
	return NULL;
}


/*************************************************************************
**
** CullSystemClass Implementation
** 
** The base CullSystemClass mainly contains code for maintaining the
** current collection list and iterating through it.
**
*************************************************************************/
CullSystemClass::CullSystemClass(void) :
	CollectionHead(NULL)
{
}

CullSystemClass::~CullSystemClass(void)
{
}

// NOTE: THE Get_() functions currently are the same as the Peek_() functions (e.g., they do not
// add a Ref). This is wrong and will be fixed.
CullableClass * CullSystemClass::Get_First_Collected_Object_Internal(void)
{
	return CollectionHead;
}

CullableClass * CullSystemClass::Get_Next_Collected_Object_Internal(CullableClass * obj)
{
	if (obj != NULL) {
		return obj->NextCollected;
	}
	return NULL;
}

CullableClass * CullSystemClass::Peek_First_Collected_Object_Internal(void)
{
	return CollectionHead;
}

CullableClass * CullSystemClass::Peek_Next_Collected_Object_Internal(CullableClass * obj)
{
	if (obj != NULL) {
		return obj->NextCollected;
	}
	return NULL;
}

void CullSystemClass::Reset_Collection(void)
{
	CollectionHead = NULL;
}

void CullSystemClass::Add_To_Collection(CullableClass * obj)
{
	WWASSERT(obj != NULL);
	obj->NextCollected = CollectionHead;
	CollectionHead = obj;
}




