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
 *                 Project Name : WWSaveLoad                                                   *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwsaveload/definitionfactorymgr.cpp               $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/16/00 11:42a                                             $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "definitionfactorymgr.h"
#include "definitionfactory.h"
#include "wwdebug.h"

#ifdef _UNIX
#include "osdep.h"
#endif

////////////////////////////////////////////////////////////////////////////
//	Static member initialization
////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *DefinitionFactoryMgrClass::_FactoryListHead = 0;


////////////////////////////////////////////////////////////////////////////
//
//	Find_Factory
//
////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
DefinitionFactoryMgrClass::Find_Factory (uint32 class_id)
{
	DefinitionFactoryClass *factory = 0;

	//
	//	Loop through all the factories and see if we can
	// find the one who owns the corresponding class-id.
	//
	for (	DefinitionFactoryClass *curr_factory = _FactoryListHead;
			(factory == 0) && (curr_factory != 0);
			curr_factory = curr_factory->m_NextFactory) {

		//
		//	Is this the factory we were looking for?
		//
		if (curr_factory->Get_Class_ID () == class_id) {
			factory = curr_factory;
		}
	}

	return factory;
}


////////////////////////////////////////////////////////////////////////////
//
//	Find_Factory
//
////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
DefinitionFactoryMgrClass::Find_Factory (const char *name)
{
	DefinitionFactoryClass *factory = 0;

	//
	//	Loop through all the factories and see if we can
	// find the one who owns the corresponding class-id.
	//
	for (	DefinitionFactoryClass *curr_factory = _FactoryListHead;
			(factory == 0) && (curr_factory != 0);
			curr_factory = curr_factory->m_NextFactory) {

		//
		//	Is this the factory we were looking for?
		//
		if (::stricmp (curr_factory->Get_Name (), name) == 0) {
			factory = curr_factory;
		}
	}

	return factory;
}


////////////////////////////////////////////////////////////////////////////
//
//	Get_First
//
////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
DefinitionFactoryMgrClass::Get_First (uint32 superclass_id)
{
	DefinitionFactoryClass *factory = 0;

	//
	//	Loop through all the factories and see if we can
	// find the next one that belongs to the given superclass
	//
	for (	DefinitionFactoryClass *curr_factory = _FactoryListHead;
			(factory == 0) && (curr_factory != 0);
			curr_factory = curr_factory->m_NextFactory) {

		//
		//	Is this the factory we were looking for?
		//
		if (::SuperClassID_From_ClassID (curr_factory->Get_Class_ID ()) == superclass_id) {
			factory = curr_factory;
		}
	}

	return factory;
}


////////////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
DefinitionFactoryMgrClass::Get_Next
(
	DefinitionFactoryClass *curr_factory,
	uint32						superclass_id
)
{
	DefinitionFactoryClass *factory = 0;

	//
	//	Loop through all the factories and see if we can
	// find the next one that belongs to the given superclass
	//
	while ((factory == NULL) && ((curr_factory = curr_factory->m_NextFactory) != NULL)) {

		//
		//	Is this the factory we were looking for?
		//
		if (::SuperClassID_From_ClassID (curr_factory->Get_Class_ID ()) == superclass_id) {
			factory = curr_factory;
		}
	}

	return factory;
}


////////////////////////////////////////////////////////////////////////////
//
//	Get_First
//
////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
DefinitionFactoryMgrClass::Get_First (void)
{
	return _FactoryListHead;
}


////////////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
////////////////////////////////////////////////////////////////////////////
DefinitionFactoryClass *
DefinitionFactoryMgrClass::Get_Next (DefinitionFactoryClass *curr_factory)
{
	DefinitionFactoryClass *factory = 0;

	//
	//	Simply return the next factory in the chain
	//
	if (curr_factory != NULL) {
		factory = curr_factory->m_NextFactory;
	}

	return factory;
}


////////////////////////////////////////////////////////////////////////////
//
//	Register_Factory
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionFactoryMgrClass::Register_Factory (DefinitionFactoryClass *factory)
{
	WWASSERT (factory->m_NextFactory == 0);
	WWASSERT (factory->m_PrevFactory == 0);
	Link_Factory (factory);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Unregister_Factory
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionFactoryMgrClass::Unregister_Factory (DefinitionFactoryClass *factory)
{
	WWASSERT (factory != 0);
	Unlink_Factory (factory);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Link_Factory
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionFactoryMgrClass::Link_Factory (DefinitionFactoryClass *factory)
{
	WWASSERT (factory->m_NextFactory == 0);
	WWASSERT (factory->m_PrevFactory == 0);

	// Adding this factory in front of the current head of the list
	factory->m_NextFactory = _FactoryListHead;

	// If the list wasn't empty, link the next factory back to this factory
	if (factory->m_NextFactory != 0) {
		factory->m_NextFactory->m_PrevFactory = factory;
	}

	// Point the head of the list at this factory now
	_FactoryListHead = factory;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Unlink_Factory
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionFactoryMgrClass::Unlink_Factory (DefinitionFactoryClass *factory)
{
	WWASSERT(factory != 0);

	// Handle the factory's prev pointer:
	if (factory->m_PrevFactory == 0) {

		// this factory is the head
		WWASSERT (_FactoryListHead == factory);
		_FactoryListHead = factory->m_NextFactory;

	} else {

		// link it's prev with it's next
		factory->m_PrevFactory->m_NextFactory = factory->m_NextFactory;

	}

	// Handle the factory's next pointer if its not at the end of the list:
	if (factory->m_NextFactory != 0) {

		factory->m_NextFactory->m_PrevFactory = factory->m_PrevFactory;

	}

	// factory is now un-linked
	factory->m_NextFactory = 0;
	factory->m_PrevFactory = 0;
	return ;
}
