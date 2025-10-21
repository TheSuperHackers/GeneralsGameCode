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
 *                     $Archive:: /Commando/Code/wwsaveload/definitionmgr.h                   $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/24/01 5:13p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once


#ifndef __DEFINITION_MGR_H
#define __DEFINITION_MGR_H

#include "always.h"
#include "saveload.h"
#include "saveloadsubsystem.h"
#include "saveloadids.h"
#include "wwdebug.h"
#include "wwstring.h"
#include "hashtemplate.h"
#include "Vector.H"


// Forward declarations
class DefinitionClass;

//////////////////////////////////////////////////////////////////////////////////
//	Global declarations
//////////////////////////////////////////////////////////////////////////////////
extern class DefinitionMgrClass	_TheDefinitionMgr;

//////////////////////////////////////////////////////////////////////////////////
//
//	DefinitionMgrClass
//
//////////////////////////////////////////////////////////////////////////////////
class DefinitionMgrClass : public SaveLoadSubSystemClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	/////////////////////////////////////////////////////////////////////
	DefinitionMgrClass (void);
	~DefinitionMgrClass (void);

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////

	// From SaveLoadSubSystemClass
	virtual uint32					Chunk_ID (void) const;

	// Type identification
	static DefinitionClass *	Find_Definition (uint32 id, bool twiddle = true);
	static DefinitionClass *	Find_Named_Definition (const char *name, bool twiddle = true);
	static DefinitionClass *	Find_Typed_Definition (const char *name, uint32 class_id, bool twiddle = true);
   static void                List_Available_Definitions (void);
   static void                List_Available_Definitions (int superclass_id);
	static uint32					Get_New_ID (uint32 class_id);

	// Definition registration
	static void						Register_Definition (DefinitionClass *definition);
	static void						Unregister_Definition (DefinitionClass *definition);

	//
	// Definition enumeration
	//
	typedef enum
	{
		ID_CLASS			= 1,
		ID_SUPERCLASS,
	} ID_TYPE;

	static DefinitionClass *	Get_First (void);
	static DefinitionClass *	Get_First (uint32 id, ID_TYPE type = ID_CLASS);
	static DefinitionClass *	Get_Next (DefinitionClass *curr_def);
	static DefinitionClass *	Get_Next (DefinitionClass *curr_def, uint32 id, ID_TYPE type = ID_CLASS);

	static void						Free_Definitions (void);

protected:

	/////////////////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////////////////

	// From SaveLoadSubSystemClass
	virtual bool					Contains_Data (void) const;
	virtual bool					Save (ChunkSaveClass &csave);
	virtual bool					Load (ChunkLoadClass &cload);
	virtual const char*			Name (void) const						{ return "DefinitionMgrClass"; }

	// Persistence methods
	bool								Save_Objects (ChunkSaveClass &csave);
	bool								Load_Objects (ChunkLoadClass &cload);
	bool								Save_Variables (ChunkSaveClass &csave);
	bool								Load_Variables (ChunkLoadClass &cload);

private:
	static HashTemplateClass<StringClass, DynamicVectorClass<DefinitionClass*>*>* DefinitionHash;

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	static void						Prepare_Definition_Array (void);
	static int __cdecl			fnCompareDefinitionsCallback (const void *elem1, const void *elem2);

	/////////////////////////////////////////////////////////////////////
	//	Static member data
	/////////////////////////////////////////////////////////////////////
	static DefinitionClass **	_SortedDefinitionArray;
	static int						_MaxDefinitionCount;
	static int						_DefinitionCount;

	/////////////////////////////////////////////////////////////////////
	//	Friend classes
	/////////////////////////////////////////////////////////////////////
	friend class DefinitionClass;
};

/////////////////////////////////////////////////////////////////////
//	Chunk_ID
/////////////////////////////////////////////////////////////////////
inline uint32
DefinitionMgrClass::Chunk_ID (void) const
{
	return CHUNKID_SAVELOAD_DEFMGR;
}

/////////////////////////////////////////////////////////////////////
//	Contains_Data
/////////////////////////////////////////////////////////////////////
inline bool
DefinitionMgrClass::Contains_Data (void) const
{
	return true;  // TODO: check if we have any definitions...
}

/////////////////////////////////////////////////////////////////////
//	Get_First_Definition
/////////////////////////////////////////////////////////////////////
inline DefinitionClass *
DefinitionMgrClass::Get_First (void)
{
	DefinitionClass *definition = NULL;
	if (_DefinitionCount > 0) {
		definition = _SortedDefinitionArray[0];
	}

	return definition;
}


#endif //__DEFINITION_MGR_H
