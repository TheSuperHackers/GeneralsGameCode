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
 *                     $Archive:: /Commando/Code/wwsaveload/saveload.cpp                      $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 12/09/01 6:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "saveload.h"
#include "saveloadsubsystem.h"
#include "persist.h"
#include "persistfactory.h"
#include "chunkio.h"
#include "wwdebug.h"
#include "saveloadstatus.h"
#include "wwhack.h"
#include "wwprofile.h"

#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
#include <windows.h>
#include "systimer.h"


SaveLoadSubSystemClass *		SaveLoadSystemClass::SubSystemListHead = nullptr;
PersistFactoryClass *			SaveLoadSystemClass::FactoryListHead = nullptr;
SList<PostLoadableClass>		SaveLoadSystemClass::PostLoadList;
PointerRemapClass					SaveLoadSystemClass::PointerRemapper;



bool SaveLoadSystemClass::Save (ChunkSaveClass &csave,SaveLoadSubSystemClass & subsystem)
{
	bool ok = true;

	if (subsystem.Contains_Data()) {
		csave.Begin_Chunk (subsystem.Chunk_ID ());
		ok &= subsystem.Save (csave);
		csave.End_Chunk ();
	}

	return ok;
}

bool SaveLoadSystemClass::Load (ChunkLoadClass &cload,bool auto_post_load)
{
	WWLOG_PREPARE_TIME_AND_MEMORY("SaveLoadSystemClass::Load");
	PointerRemapper.Reset();
	WWLOG_INTERMEDIATE("PointerRemapper.Reset()");
	bool ok = true;

	// Load each chunk we encounter and link the manager into the PostLoad list
	while (cload.Open_Chunk ()) {
		SaveLoadStatus::Inc_Status_Count();		// Count the sub systems loaded
		SaveLoadSubSystemClass *sys = Find_Sub_System(cload.Cur_Chunk_ID ());
		WWLOG_INTERMEDIATE("Find_Sub_System");
		if (sys != nullptr) {
//WWRELEASE_SAY(("			Name: %s",sys->Name()));
			INIT_SUB_STATUS(sys->Name());
			ok &= sys->Load(cload);
			WWLOG_INTERMEDIATE(sys->Name());
		}
		cload.Close_Chunk();
	}

	// Process all of the pointer remap requests
	PointerRemapper.Process();
	WWLOG_INTERMEDIATE("PointerRemapper.Process()");
	PointerRemapper.Reset();
	WWLOG_INTERMEDIATE("PointerRemapper.Reset()");

	// Call PostLoad on each PersistClass that wanted post-load
	if (auto_post_load) {
		Post_Load_Processing(nullptr);
	}
	WWLOG_INTERMEDIATE("PostLoadProcessing");

	return ok;
}

// Nework update macro for post loader.
#define UPDATE_NETWORK 											\
	if (network_callback) {                            \
		unsigned long time2 = TIMEGETTIME();            \
		if (time2 - time > 20) {                        \
			network_callback();                          \
			time = time2;                                \
		}                                               \
	}                                                  \

bool SaveLoadSystemClass::Post_Load_Processing (void(*network_callback)(void))
{
	unsigned long time = TIMEGETTIME();

	// Call PostLoad on each PersistClass that wanted post-load
	PostLoadableClass * obj = PostLoadList.Remove_Head();
	while (obj) {
		UPDATE_NETWORK;
		obj->On_Post_Load();
		obj->Set_Post_Load_Registered(false);
		obj = PostLoadList.Remove_Head();
	}

	return true;
}

void SaveLoadSystemClass::Register_Sub_System (SaveLoadSubSystemClass * sys)
{
	WWASSERT(sys != nullptr);
	Link_Sub_System(sys);
}


void SaveLoadSystemClass::Unregister_Sub_System (SaveLoadSubSystemClass * sys)
{
	WWASSERT(sys != nullptr);
	Unlink_Sub_System(sys);
}


SaveLoadSubSystemClass * SaveLoadSystemClass::Find_Sub_System (uint32 chunk_id)
{
	// TODO: need a d-s that gives fast searching based on chunk_id!!
	SaveLoadSubSystemClass * sys;
	for ( sys = SubSystemListHead; sys != nullptr; sys = sys->NextSubSystem ) {
		if ( sys->Chunk_ID() == chunk_id ) {
			break;
		}
	}
	return sys;
}

void SaveLoadSystemClass::Register_Persist_Factory(PersistFactoryClass * factory)
{
	WWASSERT(factory != nullptr);
	Link_Factory(factory);
}

void SaveLoadSystemClass::Unregister_Persist_Factory(PersistFactoryClass * factory)
{
	WWASSERT(factory != nullptr);
	Unlink_Factory(factory);
}

PersistFactoryClass * SaveLoadSystemClass::Find_Persist_Factory(uint32 chunk_id)
{
	// TODO: need a d-s that gives fast searching based on chunk_id!!
	PersistFactoryClass * fact;
	for ( fact = FactoryListHead; fact != nullptr; fact = fact->NextFactory ) {
		if ( fact->Chunk_ID() == chunk_id ) {
			break;
		}
	}
	return fact;
}

bool SaveLoadSystemClass::Is_Post_Load_Callback_Registered(PostLoadableClass * obj)
{
	// obsolete!
	bool retval = false;

	SLNode<PostLoadableClass> *list_node = nullptr;
	for (	list_node = PostLoadList.Head();
			retval == false && list_node != nullptr;
			list_node = list_node->Next())
	{
		retval = (list_node->Data() == obj);
	}

	return retval;
}

void SaveLoadSystemClass::Register_Post_Load_Callback(PostLoadableClass * obj)
{
	WWASSERT(obj != nullptr);
	if (!obj->Is_Post_Load_Registered()) {
		obj->Set_Post_Load_Registered(true);
		PostLoadList.Add_Head(obj);
	}
}

void SaveLoadSystemClass::Register_Pointer (void *old_pointer, void *new_pointer)
{
	PointerRemapper.Register_Pointer(old_pointer,new_pointer);
}

#ifdef WWDEBUG

void SaveLoadSystemClass::Request_Pointer_Remap (void **pointer_to_convert,const char * file,int line)
{
	PointerRemapper.Request_Pointer_Remap(pointer_to_convert,file,line);
}

void SaveLoadSystemClass::Request_Ref_Counted_Pointer_Remap (RefCountClass **pointer_to_convert,const char * file,int line)
{
	PointerRemapper.Request_Ref_Counted_Pointer_Remap(pointer_to_convert,file,line);
}

#else

void SaveLoadSystemClass::Request_Pointer_Remap (void **pointer_to_convert)
{
	PointerRemapper.Request_Pointer_Remap(pointer_to_convert);
}

void SaveLoadSystemClass::Request_Ref_Counted_Pointer_Remap (RefCountClass **pointer_to_convert)
{
	PointerRemapper.Request_Ref_Counted_Pointer_Remap(pointer_to_convert);
}

#endif

void SaveLoadSystemClass::Link_Sub_System(SaveLoadSubSystemClass * sys)
{
	WWASSERT(sys != nullptr);
	if (sys != nullptr) {
		WWASSERT(sys->NextSubSystem == nullptr);			// sys should never be registered twice!
		sys->NextSubSystem = SubSystemListHead;
		SubSystemListHead = sys;
	}
}

void SaveLoadSystemClass::Unlink_Sub_System(SaveLoadSubSystemClass * sys)
{
	WWASSERT(sys != nullptr);
	SaveLoadSubSystemClass * cursys = SubSystemListHead;
	SaveLoadSubSystemClass * prev = nullptr;

	while (cursys != sys) {
		prev = cursys;
		cursys = cursys->NextSubSystem;
	}

	if (prev == nullptr) {
		SubSystemListHead = sys->NextSubSystem;
	} else {
		prev->NextSubSystem = sys->NextSubSystem;
	}

	sys->NextSubSystem = nullptr;
}


void SaveLoadSystemClass::Link_Factory(PersistFactoryClass * fact)
{
	WWASSERT(fact != nullptr);
	if (fact != nullptr) {
		WWASSERT(fact->NextFactory == nullptr);			// factories should never be registered twice!
		fact->NextFactory = FactoryListHead;
		FactoryListHead = fact;
	}
}

void SaveLoadSystemClass::Unlink_Factory(PersistFactoryClass * fact)
{
	WWASSERT(fact != nullptr);

	PersistFactoryClass * curfact = FactoryListHead;
	PersistFactoryClass * prev = nullptr;

	while (curfact != fact) {
		prev = curfact;
		curfact = curfact->NextFactory;
	}

	if (prev == nullptr) {
		FactoryListHead = fact->NextFactory;
	} else {
		prev->NextFactory = fact->NextFactory;
	}

	fact->NextFactory = nullptr;
}

void Force_Link_WWSaveLoad (void)
{
	FORCE_LINK( Twiddler );
	return ;
}
