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
 *                     $Archive:: /Commando/Code/wwsaveload/definitionmgr.cpp                 $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 12/10/01 2:37p                                              $*
 *                                                                                             *
 *                    $Revision:: 35                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "definitionmgr.h"
#include "definition.h"
#include "definitionfactory.h"
#include "definitionfactorymgr.h"
#include "definitionclassids.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "wwdebug.h"
#include "wwmemlog.h"
#include "twiddler.h"
#include "wwprofile.h"


//////////////////////////////////////////////////////////////////////////////////
//	Global instance
//////////////////////////////////////////////////////////////////////////////////
DefinitionMgrClass	_TheDefinitionMgr;

//////////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////////
static const int		DEFINTION_LIST_GROW_SIZE	= 1000;
static const uint32	IDRANGE_PER_CLASS				= 10000;

enum
{
	CHUNKID_VARIABLES			= 0x00000100,
	CHUNKID_OBJECTS,
		CHUNKID_OBJECT
};

enum
{
	VARID_NEXTDEFID	= 0x01
};

//////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
//////////////////////////////////////////////////////////////////////////////////
DefinitionClass **	DefinitionMgrClass::_SortedDefinitionArray	= nullptr;
int						DefinitionMgrClass::_DefinitionCount			= 0;
int						DefinitionMgrClass::_MaxDefinitionCount		= 0;
HashTemplateClass<StringClass, DynamicVectorClass<DefinitionClass*>*>* DefinitionMgrClass::DefinitionHash;

//////////////////////////////////////////////////////////////////////////////////
//
//	DefinitionMgrClass
//
//////////////////////////////////////////////////////////////////////////////////
DefinitionMgrClass::DefinitionMgrClass (void)
{
	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	~DefinitionMgrClass
//
//////////////////////////////////////////////////////////////////////////////////
DefinitionMgrClass::~DefinitionMgrClass (void)
{
	Free_Definitions ();
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Definition
//
//////////////////////////////////////////////////////////////////////////////////
DefinitionClass *
DefinitionMgrClass::Find_Definition (uint32 id, bool twiddle)
{
	DefinitionClass *definition = nullptr;

	int lower_index	= 0;
	int upper_index	= _DefinitionCount - 1;
	int index			= upper_index / 2;
	bool keep_going	= (_DefinitionCount > 0);

	//
	//	Binary search the list until we've found the definition
	//
	while (keep_going) {

		DefinitionClass *curr_def = _SortedDefinitionArray[index];
		WWASSERT (curr_def != nullptr);

		//
		//	Is this the definition we are looking for?
		//
		if (curr_def->Get_ID () == id) {
			definition	= _SortedDefinitionArray[index];
			keep_going	= false;
		} else if (upper_index <= lower_index + 1) {

			//
			//	When the window get's too small, our divide by two won't catch
			// both entries, so just go ahead and do them both now.
			//
			keep_going	= false;
			if (_SortedDefinitionArray[lower_index]->Get_ID () == id) {
				definition = _SortedDefinitionArray[lower_index];
			} else if (_SortedDefinitionArray[upper_index]->Get_ID () == id) {
				definition = _SortedDefinitionArray[upper_index];
			}

		} else {

			//
			//	Cut our 'window' in half
			//
			if (id > curr_def->Get_ID ()) {
				lower_index = index;
				index += (upper_index - index) / 2;
			} else {
				upper_index = index;
				index -= (index - lower_index) / 2;
			}
		}
	}

	//
	//	Should we twiddle this definition? (Twiddling refers to our randomizing
	//	framework for definitions)
	//
	if (	twiddle &&
			definition != nullptr &&
			definition->Get_Class_ID () == CLASSID_TWIDDLERS)
	{
		definition = ((TwiddlerClass *)definition)->Twiddle ();
	}

	return definition;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Named_Definition
//
//////////////////////////////////////////////////////////////////////////////////
DefinitionClass *
DefinitionMgrClass::Find_Named_Definition (const char *name, bool twiddle)
{
	DefinitionClass *definition = nullptr;

	//
	//	Loop through all the definitions and see if we can
	// find the one with the requested name
	//
	for (int index = 0; index < _DefinitionCount; index ++) {
		DefinitionClass *curr_def = _SortedDefinitionArray[index];

		//
		//	Is this the definition we were looking for?
		//
		if (curr_def != nullptr && ::stricmp (curr_def->Get_Name (), name) == 0) {
			definition = curr_def;
			break;
		}
	}

	//
	//	Should we twiddle this definition? (Twiddling refers to our randomizing
	//	framework for definitions)
	//
	if (	twiddle &&
			definition != nullptr &&
			definition->Get_Class_ID () == CLASSID_TWIDDLERS)
	{
		definition = ((TwiddlerClass *)definition)->Twiddle ();
	}

	return definition;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Find_Typed_Definition
//
//////////////////////////////////////////////////////////////////////////////////

DefinitionClass *
DefinitionMgrClass::Find_Typed_Definition (const char *name, uint32 class_id, bool twiddle)
{
	//
	//	Sanity check
	//
	if (DefinitionHash == nullptr) {
		WWDEBUG_SAY (("DefinitionMgrClass::Find_Typed_Definition () failed due to a null DefinitionHash."));
		return nullptr;
	}

	DefinitionClass *definition = nullptr;

	// Check the hash table first. The hash table is built as we need the definitions, so if definition is not
	// in the table, it will be added there.

	//
	// TSS null deref on this sucker 08/03/01
	//
	WWASSERT(DefinitionHash != nullptr);

	StringClass lower_case_name(name,true);
	_strlwr(lower_case_name.Peek_Buffer());
	DynamicVectorClass<DefinitionClass*>* defs = DefinitionHash->Get(lower_case_name);

	if (defs) {
		for (int i=0;i<defs->Length();++i) {
			DefinitionClass* curr_def=(*defs)[i];
			WWASSERT(curr_def);
			uint32 curr_class_id = curr_def->Get_Class_ID ();
			if (	(curr_class_id == class_id) ||
					(::SuperClassID_From_ClassID (curr_class_id) == class_id) ||
					(twiddle && (curr_def->Get_Class_ID () == CLASSID_TWIDDLERS)))
			{
				definition = curr_def;
				break;
			}
		}
	}

	//
	//	Loop through all the definitions and see if we can
	// find the one with the requested name
	//
	if (!definition) {
		for (int index = 0; index < _DefinitionCount; index ++) {
			DefinitionClass *curr_def = _SortedDefinitionArray[index];
			if (curr_def != nullptr) {

				//
				//	Is this the correct class of definition?
				//
				uint32 curr_class_id = curr_def->Get_Class_ID ();
				if (	(curr_class_id == class_id) ||
						(::SuperClassID_From_ClassID (curr_class_id) == class_id) ||
						(twiddle && (curr_def->Get_Class_ID () == CLASSID_TWIDDLERS)))
				{
					//
					//	Is this the definition we were looking for?
					//
					if (::stricmp (curr_def->Get_Name (), name) == 0) {
						definition = curr_def;
						// Add the definition to the hash table, so that it can be quickly accessed the next time it is needed.
						if (!defs) {
							defs=W3DNEW DynamicVectorClass<DefinitionClass*>;
							DefinitionHash->Insert(lower_case_name,defs);
						}
						defs->Add(definition);
						break;
					}
				}
			}
		}
	}

	//
	//	Should we twiddle this definition? (Twiddling refers to our randomizing
	//	framework for definitions)
	//
	if (	twiddle &&
			definition != nullptr &&
			definition->Get_Class_ID () == CLASSID_TWIDDLERS)
	{
		definition = ((TwiddlerClass *)definition)->Twiddle ();
	}

	return definition;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	List_Available_Definitions
//
//////////////////////////////////////////////////////////////////////////////////
void
DefinitionMgrClass::List_Available_Definitions (void)
{
	//
	//	Loop through all the definitions and print the definition name
	//
	WWDEBUG_SAY(("Available definitions:"));
	for (int index = 0; index < _DefinitionCount; index ++) {
		DefinitionClass *curr_def = _SortedDefinitionArray[index];
		if (curr_def != nullptr) {
			WWDEBUG_SAY(("  >%s<", curr_def->Get_Name ()));
		}
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	List_Available_Definitions
//
//////////////////////////////////////////////////////////////////////////////////
void
DefinitionMgrClass::List_Available_Definitions (int superclass_id)
{
	//
	//	Loop through all the definitions and print the definition name
	//
	WWDEBUG_SAY(("Available superclass definitions for 0x%8X:", superclass_id));
	DefinitionClass *definition = nullptr;
	for (	definition = Get_First (superclass_id, DefinitionMgrClass::ID_SUPERCLASS);
			definition != nullptr;
			definition = Get_Next (definition, superclass_id, DefinitionMgrClass::ID_SUPERCLASS))
	{
		WWDEBUG_SAY(("  >%s<", definition->Get_Name ()));
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_First
//
//////////////////////////////////////////////////////////////////////////////////
DefinitionClass *
DefinitionMgrClass::Get_First (uint32 id, ID_TYPE type)
{
	DefinitionClass *definition = nullptr;

	//
	//	Loop through all the definitions and find the first
	// one that belongs to the requested class
	//
	for (	int index = 0;
			(definition == nullptr) && (index < _DefinitionCount);
			index ++)
	{
		DefinitionClass *curr_def = _SortedDefinitionArray[index];
		if (curr_def != nullptr) {

			//
			//	Is this the definition we were looking for?
			//
			if (	(type == ID_SUPERCLASS) &&
					(::SuperClassID_From_ClassID (curr_def->Get_Class_ID ()) == id)) {
				definition = curr_def;
			} else if (	(type == ID_CLASS) &&
							(curr_def->Get_Class_ID () == id)) {
				definition = curr_def;
			}
		}
	}

	return definition;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
//////////////////////////////////////////////////////////////////////////////////
DefinitionClass *
DefinitionMgrClass::Get_Next
(
	DefinitionClass *	curr_def,
	uint32				id,
	ID_TYPE				type
)
{
	DefinitionClass *definition = nullptr;

	//
	//	Loop through all the definitions and find the first
	// one that belongs to the requested class
	//
	for (	int index = curr_def->m_DefinitionMgrLink + 1;
			(definition == nullptr) && (index < _DefinitionCount);
			index ++)
	{
		DefinitionClass *curr_def = _SortedDefinitionArray[index];
		if (curr_def != nullptr) {

			//
			//	Is this the definition we were looking for?
			//
			if (	(type == ID_SUPERCLASS) &&
					(::SuperClassID_From_ClassID (curr_def->Get_Class_ID ()) == id)) {
				definition = curr_def;
			} else if (	(type == ID_CLASS) &&
							(curr_def->Get_Class_ID () == id)) {
				definition = curr_def;
			}
		}
	}

	return definition;
}


////////////////////////////////////////////////////////////////////////////
//
//	Get_Next
//
////////////////////////////////////////////////////////////////////////////
DefinitionClass *
DefinitionMgrClass::Get_Next (DefinitionClass *curr_def)
{
	WWASSERT (curr_def != nullptr);
	DefinitionClass *definition = nullptr;

	int index = curr_def->m_DefinitionMgrLink + 1;
	if (index < _DefinitionCount) {
		definition = _SortedDefinitionArray[index];
	}

	return definition;
}


////////////////////////////////////////////////////////////////////////////
//
//	Free_Definitions
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionMgrClass::Free_Definitions (void)
{
	// Clear the hash table
	if (DefinitionHash) {
		HashTemplateIterator<StringClass,DynamicVectorClass<DefinitionClass*>*> ite(*DefinitionHash);
		for (ite.First();!ite.Is_Done();ite.Next()) {
			DynamicVectorClass<DefinitionClass*>* defs=ite.Peek_Value();
//			delete ite.Peek_Value();
			delete defs;
		}
		DefinitionHash->Remove_All();
		delete DefinitionHash;
		DefinitionHash=nullptr;
	}

	//
	//	Free the definition array
	//
	if (_SortedDefinitionArray != nullptr) {
		//
		//	Free each of the definition objects
		//
		for (int index = 0; index < _DefinitionCount; index ++) {
			delete _SortedDefinitionArray[index];
		}
		delete [] _SortedDefinitionArray;
		_SortedDefinitionArray = nullptr;
		_MaxDefinitionCount = 0;
		_DefinitionCount = 0;
	}
}


////////////////////////////////////////////////////////////////////////////
//
//	Prepare_Definition_Array
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionMgrClass::Prepare_Definition_Array (void)
{
	if (_DefinitionCount + 1 > _MaxDefinitionCount) {

		//
		//	Allocate a new, bigger array
		//
		int new_size = _MaxDefinitionCount + DEFINTION_LIST_GROW_SIZE;
		DefinitionClass **new_array = W3DNEWARRAY DefinitionClass *[new_size];

		//
		//	Copy the entries from the old array to the new array
		//
		::memcpy (new_array, _SortedDefinitionArray, _DefinitionCount * sizeof (DefinitionClass *));

		//
		//	Free the old array and start using the new array
		//
		delete [] _SortedDefinitionArray;
		_SortedDefinitionArray	= new_array;
		_MaxDefinitionCount		= new_size;
	}
	if (!DefinitionHash) DefinitionHash=W3DNEW HashTemplateClass<StringClass, DynamicVectorClass<DefinitionClass*>*>;

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Register_Definition
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionMgrClass::Register_Definition (DefinitionClass *definition)
{
	WWASSERT (definition != nullptr);
	if (definition != nullptr && definition->m_DefinitionMgrLink == -1 && definition->Get_ID () != 0) {
		//
		//	Make sure the definition array is large enough
		//
		Prepare_Definition_Array ();

		//
		//	Calculate where in the list we should insert this definition
		//
		uint32 id			= definition->Get_ID ();
		int lower_index	= 0;
		int upper_index	= _DefinitionCount - 1;
		int index			= upper_index / 2;
		int insert_index	= _DefinitionCount;
		bool keep_going	= (_DefinitionCount > 0);
		bool is_valid		= true;

		while (keep_going) {

			DefinitionClass *curr_def = _SortedDefinitionArray[index];
			WWASSERT (curr_def != nullptr);

			//
			//	Check to make sure we aren't trying to register a definition
			// that has the same ID as a definition that is already in the list.
			//
			if (curr_def->Get_ID () == id) {
				insert_index	= index;
				keep_going		= false;
				is_valid			= false;
			} else {

				//
				//	Cut our 'window' in half
				//
				if (id > curr_def->Get_ID ()) {
					lower_index = index;
					index += (upper_index - index) / 2;
				} else {
					upper_index = index;
					index -= (index - lower_index) / 2;
				}

				//
				//	If we've narrowed down the window to 2 entries, then quick check
				// the different possibilities.
				//
				if (upper_index <= lower_index + 1) {
					if (_SortedDefinitionArray[upper_index]->Get_ID () <= id) {
						insert_index = upper_index + 1;
					} else if (_SortedDefinitionArray[lower_index]->Get_ID () <= id) {
						insert_index = upper_index;
					} else {
						insert_index = lower_index;
					}
					keep_going = false;
				}
			}
		}

		//WWASSERT (is_valid);
		if (is_valid) {

			//
			//	Re-index all the definitions that got bumped one cell due to this insertion.
			//
			for (index = _DefinitionCount - 1; index >= insert_index; index --) {
				_SortedDefinitionArray[index + 1] = _SortedDefinitionArray[index];
				_SortedDefinitionArray[index + 1]->m_DefinitionMgrLink = index + 1;
			}

			//
			//	Insert this definition into the list
			//
			definition->m_DefinitionMgrLink			= insert_index;
			_SortedDefinitionArray[insert_index]	= definition;
			_DefinitionCount ++;
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//	Unregister_Definition
//
////////////////////////////////////////////////////////////////////////////
void
DefinitionMgrClass::Unregister_Definition (DefinitionClass *definition)
{
	WWASSERT (definition != 0);
	//WWASSERT (definition->m_DefinitionMgrLink >= 0 && definition->m_DefinitionMgrLink < _DefinitionCount);

	if (definition != nullptr && definition->m_DefinitionMgrLink != -1) {

		//
		//	Re-index the definitions that come after this definition in the list
		//
		for (int index = definition->m_DefinitionMgrLink; index < _DefinitionCount - 1; index ++) {
			_SortedDefinitionArray[index] = _SortedDefinitionArray[index + 1];
			_SortedDefinitionArray[index]->m_DefinitionMgrLink = index;
		}

		_SortedDefinitionArray[_DefinitionCount - 1] = nullptr;
		definition->m_DefinitionMgrLink = -1;
		_DefinitionCount --;
	}

	return ;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionMgrClass::Save
(
	ChunkSaveClass &	csave
)
{
	WWMEMLOG(MEM_GAMEDATA);

	bool retval = true;

	//
	//	Create a chunk to contain the class variables we need to serialize.
	//
	csave.Begin_Chunk (CHUNKID_VARIABLES);
	Save_Variables (csave);
	csave.End_Chunk ();

	//
	//	Have the base class write the objects to their own chunk.
	//
	csave.Begin_Chunk (CHUNKID_OBJECTS);
	retval &= Save_Objects (csave);
	csave.End_Chunk ();

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionMgrClass::Load (ChunkLoadClass &cload)
{
	WWMEMLOG(MEM_GAMEDATA);
	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			//
			//	If this is the chunk that contains the class variables, then
			// loop through and read each microchunk
			//
			case CHUNKID_VARIABLES:
				retval &= Load_Variables (cload);
				break;

			//
			//	Load all the definition objects from this chunk
			//
			case CHUNKID_OBJECTS:
				retval &= Load_Objects (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Objects
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionMgrClass::Save_Objects
(
	ChunkSaveClass &	csave
)
{
	bool retval = true;

	//
	//	Loop through all the definition objects
	//
	for (int index = 0; index < _DefinitionCount; index ++) {
		DefinitionClass *definition = _SortedDefinitionArray[index];
		if (definition != nullptr && definition->Is_Save_Enabled ()) {

			//
			//	Save this definition object
			//
			csave.Begin_Chunk (definition->Get_Factory ().Chunk_ID ());
			definition->Get_Factory ().Save (csave, definition);
			csave.End_Chunk ();
		}
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionMgrClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;
	return retval;
}

float _alloc_time;
float _load_time;
float _reg_time;


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Objects
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionMgrClass::Load_Objects (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {

		//
		//	Load this definition from the chunk (if possible)
		//
		PersistFactoryClass *factory = SaveLoadSystemClass::Find_Persist_Factory (cload.Cur_Chunk_ID ());
		if (factory != nullptr) {

			DefinitionClass *definition = (DefinitionClass *)factory->Load (cload);
			if (definition != nullptr) {

				//
				//	Add this definition to our array
				//
				Prepare_Definition_Array ();
				_SortedDefinitionArray[_DefinitionCount ++] = definition;
			}
		}

		cload.Close_Chunk ();
	}

	//
	//	Sort the definitions
	//
	if (_DefinitionCount > 0) {
		::qsort (_SortedDefinitionArray, _DefinitionCount, sizeof (DefinitionClass *), fnCompareDefinitionsCallback);
	}

	//
	//	Assign a mgr link to each definition
	//
	for (int index = 0; index < _DefinitionCount; index ++) {
		_SortedDefinitionArray[index]->m_DefinitionMgrLink = index;
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionMgrClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			case VARID_NEXTDEFID:
				break;
		}

		cload.Close_Micro_Chunk ();
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////
//
//	Get_New_ID
//
/////////////////////////////////////////////////////////////////////
uint32
DefinitionMgrClass::Get_New_ID (uint32 class_id)
{
	uint32 idrange_start = (class_id - DEF_CLASSID_START) * IDRANGE_PER_CLASS;
	uint32 idrange_end	= (idrange_start + IDRANGE_PER_CLASS);

	uint32 new_id = idrange_start + 1;

	//
	//	Try to find the first empty slot in this ID range
	//
	for (int index = 0; index < _DefinitionCount; index ++) {
		DefinitionClass *definition = _SortedDefinitionArray[index];
		if (definition != nullptr) {

			//
			//	Get this definition's ID
			//
			uint32 curr_id = definition->Get_ID ();

			//
			//	Is this id in the range we are looking for?
			//
			if (curr_id >= idrange_start && curr_id < idrange_end) {

				bool is_ok = false;
				if (index < _DefinitionCount - 1) {

					//
					//	Check to see if the next definition in our array leaves a hole in the
					// ID range.
					//
					DefinitionClass *next_definition = _SortedDefinitionArray[index + 1];
					if (next_definition != nullptr && next_definition->Get_ID () > (curr_id + 1)) {
						is_ok = true;
					}

				} else {
					is_ok = true;
				}

				//
				//	Return the new ID
				//
				if (is_ok) {
					new_id = curr_id + 1;
					break;
				}
			}
		}
	}

	return new_id;
}


////////////////////////////////////////////////////////////////
//
//	fnCompareDefinitionsCallback
//
////////////////////////////////////////////////////////////////
int __cdecl
DefinitionMgrClass::fnCompareDefinitionsCallback
(
	const void *elem1,
	const void *elem2
)
{
   WWASSERT (elem1 != nullptr);
   WWASSERT (elem2 != nullptr);
   DefinitionClass *definition1 = *((DefinitionClass **)elem1);
   DefinitionClass *definition2 = *((DefinitionClass **)elem2);

	//
	//	Sort the definitions based on ID
	//
	int result = 0;
	if (definition1->Get_ID () > definition2->Get_ID ()) {
		result = 1;
	} else if (definition1->Get_ID () < definition2->Get_ID ()) {
		result = -1;
	} else {
		result = 0;
	}

   return result;
}
