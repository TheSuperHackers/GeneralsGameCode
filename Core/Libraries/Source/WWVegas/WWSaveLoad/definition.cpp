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
 *                     $Archive:: /Commando/Code/wwsaveload/definition.cpp                    $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/24/01 4:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "definition.h"
#include "chunkio.h"

//////////////////////////////////////////////////////////////////////////////////
//	Constants
//////////////////////////////////////////////////////////////////////////////////
enum
{
	CHUNKID_VARIABLES			= 0x00000100,
};

enum
{
	VARID_INSTANCEID	= 0x01,
	XXX_VARID_PARENTID,
	VARID_NAME,
};


/////////////////////////////////////////////////////////
//
//	Save
//
/////////////////////////////////////////////////////////
bool
DefinitionClass::Save (ChunkSaveClass &csave)
{
	bool retval = true;

	csave.Begin_Chunk (CHUNKID_VARIABLES);
	retval &= Save_Variables (csave);
	csave.End_Chunk ();

	return retval;
}


/////////////////////////////////////////////////////////
//
//	Load
//
/////////////////////////////////////////////////////////
bool
DefinitionClass::Load (ChunkLoadClass &cload)
{
	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_VARIABLES:
				Load_Variables (cload);
				break;
		}

		cload.Close_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Save_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionClass::Save_Variables (ChunkSaveClass &csave)
{
	bool retval = true;

	WRITE_MICRO_CHUNK (csave, VARID_INSTANCEID, m_ID);
	WRITE_MICRO_CHUNK_WWSTRING (csave, VARID_NAME, m_Name);
	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Load_Variables
//
//////////////////////////////////////////////////////////////////////////////////
bool
DefinitionClass::Load_Variables (ChunkLoadClass &cload)
{
	bool retval = true;

	//
	//	Loop through all the microchunks that define the variables
	//
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {

			READ_MICRO_CHUNK (cload, VARID_INSTANCEID, m_ID)
			READ_MICRO_CHUNK_WWSTRING (cload, VARID_NAME, m_Name)
		}

		cload.Close_Micro_Chunk ();
	}

	return retval;
}


//////////////////////////////////////////////////////////////////////////////////
//
//	Set_ID
//
//////////////////////////////////////////////////////////////////////////////////
void
DefinitionClass::Set_ID (uint32 id)
{
	m_ID = id;

	//
	//	If we are registered with the definition manager, then we need to
	// re-link ourselves back into the list
	//
	if (m_DefinitionMgrLink != -1) {
		DefinitionMgrClass::Unregister_Definition (this);
		DefinitionMgrClass::Register_Definition (this);
	}

	return ;
}

