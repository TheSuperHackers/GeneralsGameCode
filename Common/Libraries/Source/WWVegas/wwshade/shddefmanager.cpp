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
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shddefmanager.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 5/20/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                          $*
 *                                                                                             *
 * 5/20/02 KM Added save load behavior
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "w3d_file.h"
#include "chunkio.h"
#include "shddef.h"
#include "shddefmanager.h"
#include "shddeffactory.h"
#include "wwdebug.h"
#include "string.h"


/*
** Static head of the factory list
*/
ShdDefFactoryClass *ShdDefManagerClass::_FactoryListHead = NULL;


//**********************************************************************************************
//! Used to look up the ShdDefFactory for a given class id 
/*!
	@param class_id - class id to look up the factory for
	@returns	the factory if it is found or NULL
*/
ShdDefFactoryClass * 
ShdDefManagerClass::Find_Factory (uint32 class_id)
{
	ShdDefFactoryClass *factory = 0;

	//
	//	Loop through all the factories and see if we can
	// find the one who owns the corresponding class-id.
	//
	for (	ShdDefFactoryClass *curr_factory = _FactoryListHead;
			(factory == 0) && (curr_factory != 0);
			curr_factory = curr_factory->NextFactory) 
	{
		//
		//	Is this the factory we were looking for?
		//
		if (curr_factory->Get_Class_ID () == class_id) {
			factory = curr_factory;
		}
	}

	return factory;
}


//**********************************************************************************************
//! Used to look up the ShdDefFactory for a given named shader def
/*!
	@param	name - name of the desired shader def
	@returns	the factory if it is found or NULL if not
*/
ShdDefFactoryClass * ShdDefManagerClass::Find_Factory (const char *name)
{
	ShdDefFactoryClass * factory = 0;

	//
	//	Loop through all the factories and see if we can
	// find the one who owns the corresponding class-id.
	//
	for (	ShdDefFactoryClass *curr_factory = _FactoryListHead;
			(factory == 0) && (curr_factory != 0);
			curr_factory = curr_factory->NextFactory) {

		//
		//	Is this the factory we were looking for?
		//
		if (::stricmp (curr_factory->Get_Name (), name) == 0) {
			factory = curr_factory;
		}
	}

	return factory;
}


//**********************************************************************************************
//! Registers a factory with the system
/*!
	@param	factory - the factory to register
*/
void ShdDefManagerClass::Register_Factory (ShdDefFactoryClass *factory)
{
	WWASSERT (factory->NextFactory == 0);
	WWASSERT (factory->PrevFactory == 0);
	Link_Factory (factory);
}


//**********************************************************************************************
//! removes a factory from the system
/*!
	@param	factory - the factory to unregister
*/
void ShdDefManagerClass::Unregister_Factory (ShdDefFactoryClass *factory)
{
	WWASSERT (factory != 0);
	Unlink_Factory (factory);
	return ;
}


ShdDefFactoryClass * ShdDefManagerClass::Get_First (void)
{
	return _FactoryListHead;
}

ShdDefFactoryClass * ShdDefManagerClass::Get_Next (ShdDefFactoryClass *current)
{
	//
	//	Simply return the next factory in the chain
	//
	if (current != NULL) {
		return current->NextFactory;
	}

	return _FactoryListHead;
}

void ShdDefManagerClass::Link_Factory (ShdDefFactoryClass *factory)
{
	WWASSERT (factory->NextFactory == 0);
	WWASSERT (factory->PrevFactory == 0);

	// Adding this factory in front of the current head of the list
	factory->NextFactory = _FactoryListHead;
	
	// If the list wasn't empty, link the next factory back to this factory
	if (factory->NextFactory != 0) {
		factory->NextFactory->PrevFactory = factory;
	}

	// Point the head of the list at this factory now
	_FactoryListHead = factory;
	return ;
}

void ShdDefManagerClass::Unlink_Factory (ShdDefFactoryClass *factory)
{
	WWASSERT(factory != 0);

	// Handle the factory's prev pointer:
	if (factory->PrevFactory == 0) {

		// this factory is the head
		WWASSERT (_FactoryListHead == factory);
		_FactoryListHead = factory->NextFactory;
	
	} else {

		// link it's prev with it's next
		factory->PrevFactory->NextFactory = factory->NextFactory;

	}

	// Handle the factory's next pointer if its not at the end of the list:
	if (factory->NextFactory != 0) {
		
		factory->NextFactory->PrevFactory = factory->PrevFactory;

	}

	// factory is now un-linked
	factory->NextFactory = 0;
	factory->PrevFactory = 0;
	return ;
}

ShdDefClass* ShdDefManagerClass::Create_ShdDefClass_Instance(uint32 class_id)
{
	// first look up the factory for this classid
	ShdDefFactoryClass * factory = Find_Factory (class_id);

	if (factory != NULL) 
	{
		return factory->Create();
	}
	else 
	{
		return NULL;
	}
}

void ShdDefManagerClass::Save_Shader(ChunkSaveClass& csave, ShdDefClass* shddef)
{
	csave.Begin_Chunk(W3D_CHUNK_SHDSUBMESH_SHADER_CLASSID);
	uint32 id=shddef->Get_Class_ID();
	csave.Write(&id,sizeof(uint32));
	csave.End_Chunk();

	csave.Begin_Chunk(W3D_CHUNK_SHDSUBMESH_SHADER_DEF);
	shddef->Save(csave);
	csave.End_Chunk();

}

void ShdDefManagerClass::Load_Shader(ChunkLoadClass& cload, ShdDefClass** shddef)
{
	uint32 id;

	cload.Open_Chunk();
	cload.Read(&id,sizeof(uint32));
	cload.Close_Chunk();

	*shddef=Create_ShdDefClass_Instance(id);

	cload.Open_Chunk();
	(*shddef)->Load(cload);
	cload.Close_Chunk();
}

