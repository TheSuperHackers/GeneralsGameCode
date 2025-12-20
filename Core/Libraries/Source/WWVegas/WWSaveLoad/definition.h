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
 *                     $Archive:: /Commando/Code/wwsaveload/definition.h                      $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 8/24/01 4:57p                                               $*
 *                                                                                             *
 *                    $Revision:: 20                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
// SKB Remove because of G conflicts with CLASSID_??
#include "definitionclassids.h"
#include "definitionmgr.h"
#include "editable.h"
#include "wwstring.h"

// Forward declarations
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	DefinitionClass
//
//////////////////////////////////////////////////////////////////////////////////
class DefinitionClass : public PersistClass, public EditableClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(DefinitionClass, EditableClass);

	/////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	/////////////////////////////////////////////////////////////////////
	DefinitionClass (void);
	virtual ~DefinitionClass (void);

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////

	// Type identification
	virtual uint32						Get_Class_ID (void) const = 0;
	virtual uint32						Get_ID (void) const;
	virtual void						Set_ID (uint32 id);
	virtual PersistClass *			Create (void) const = 0;

	// Display name methods
	virtual const char *				Get_Name (void) const;
	virtual void						Set_Name (const char *new_name);

	// Validation methods
	virtual bool						Is_Valid_Config (StringClass &message);

	// From PersistClass
	virtual bool						Save (ChunkSaveClass &csave);
	virtual bool						Load (ChunkLoadClass &cload);

	// User data support
	uint32								Get_User_Data (void) const		{ return m_GenericUserData; }
	void									Set_User_Data (uint32 data)	{ m_GenericUserData = data; }

	// Save support
	bool									Is_Save_Enabled (void) const	{ return m_SaveEnabled; }
	void									Enable_Save (bool onoff)		{ m_SaveEnabled = onoff; }

protected:

	/////////////////////////////////////////////////////////////////////
	//	Protected member data
	/////////////////////////////////////////////////////////////////////
	int											m_DefinitionMgrLink;

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	bool											Save_Variables (ChunkSaveClass &csave);
	bool											Load_Variables (ChunkLoadClass &cload);

	/////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////
	StringClass				m_Name;
	uint32					m_ID;
	uint32					m_GenericUserData;
	bool						m_SaveEnabled;

	/////////////////////////////////////////////////////////////////////
	//	Friends
	/////////////////////////////////////////////////////////////////////
	friend class DefinitionMgrClass;
};


/////////////////////////////////////////////////////////////////////
//	DefinitionClass
/////////////////////////////////////////////////////////////////////
inline
DefinitionClass::DefinitionClass (void)
	:	m_ID (0),
		m_SaveEnabled (true),
		m_DefinitionMgrLink (-1)
{
	return ;
}

/////////////////////////////////////////////////////////////////////
//	DefinitionClass
/////////////////////////////////////////////////////////////////////
inline
DefinitionClass::~DefinitionClass (void)
{
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//	Get_Name
//////////////////////////////////////////////////////////////////////////////////
inline const char *
DefinitionClass::Get_Name (void) const
{
	return m_Name;
}

//////////////////////////////////////////////////////////////////////////////////
//	Set_Name
//////////////////////////////////////////////////////////////////////////////////
inline void
DefinitionClass::Set_Name (const char *new_name)
{
	m_Name = new_name;
	return ;
}

//////////////////////////////////////////////////////////////////////////////////
//	Get_ID
//////////////////////////////////////////////////////////////////////////////////
inline uint32
DefinitionClass::Get_ID (void) const
{
	return m_ID;
}

//////////////////////////////////////////////////////////////////////////////////
//	Is_Valid_Config
//////////////////////////////////////////////////////////////////////////////////
inline bool
DefinitionClass::Is_Valid_Config (StringClass &message)
{
	return true;
}
