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
 *                     $Archive:: /Commando/Code/wwsaveload/twiddler.h                        $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 6/27/00 2:34p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "definition.h"
#include "definitionclassids.h"


//////////////////////////////////////////////////////////////////////////////////
//
//	TwiddlerClass
//
//////////////////////////////////////////////////////////////////////////////////
class TwiddlerClass : public DefinitionClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(TwiddlerClass, DefinitionClass);

	/////////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	/////////////////////////////////////////////////////////////////////
	TwiddlerClass (void);
	virtual ~TwiddlerClass (void);

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////

	//
	// Type identification
	//
	uint32								Get_Class_ID (void) const	{ return CLASSID_TWIDDLERS; }
	PersistClass *						Create (void) const;

	//
	// From PersistClass
	//
	bool									Save (ChunkSaveClass &csave);
	bool									Load (ChunkLoadClass &cload);
	const PersistFactoryClass &	Get_Factory (void) const;

	//
	//	Twiddler specific
	//
	virtual DefinitionClass *		Twiddle (void) const;
	virtual uint32						Get_Indirect_Class_ID (void) const;
	virtual void						Set_Indirect_Class_ID (uint32 class_id);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	bool								Save_Variables (ChunkSaveClass &csave);
	bool								Load_Variables (ChunkLoadClass &cload);

	/////////////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////////////
	uint32							m_IndirectClassID;
	DynamicVectorClass<int>		m_DefinitionList;
};


/////////////////////////////////////////////////////////////////////
//	Get_Indirect_Class_ID
/////////////////////////////////////////////////////////////////////
inline uint32
TwiddlerClass::Get_Indirect_Class_ID (void) const
{
	return m_IndirectClassID;
}


/////////////////////////////////////////////////////////////////////
//	Set_Indirect_Class_ID
/////////////////////////////////////////////////////////////////////
inline void
TwiddlerClass::Set_Indirect_Class_ID (uint32 class_id)
{
	m_IndirectClassID = class_id;
	return ;
}
