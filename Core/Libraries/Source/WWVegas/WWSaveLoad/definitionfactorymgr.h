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
 *                     $Archive:: /Commando/Code/wwsaveload/definitionfactorymgr.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 7/07/00 11:55a                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef __DEFINITION_FACTORY_MGR_H
#define __DEFINITION_FACTORY_MGR_H

#include "always.h"
#include "bittype.h"
#include "definitionclassids.h"

class DefinitionFactoryClass;

//////////////////////////////////////////////////////////////////////////////////
//
//	DefinitionFactoryMgrClass
//
//////////////////////////////////////////////////////////////////////////////////
class DefinitionFactoryMgrClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////////////
	static DefinitionFactoryClass *	Find_Factory (uint32 class_id);
	static DefinitionFactoryClass *	Find_Factory (const char *name);
	static void								Register_Factory (DefinitionFactoryClass *factory);
	static void								Unregister_Factory (DefinitionFactoryClass *factory);

	// Class enumeration
	static DefinitionFactoryClass *	Get_First (uint32 superclass_id);
	static DefinitionFactoryClass *	Get_Next (DefinitionFactoryClass *current, uint32 superclass_id);

	// Factory enumeration
	static DefinitionFactoryClass *	Get_First (void);
	static DefinitionFactoryClass *	Get_Next (DefinitionFactoryClass *current);

private:

	/////////////////////////////////////////////////////////////////////
	//	Private methods
	/////////////////////////////////////////////////////////////////////
	static void								Link_Factory (DefinitionFactoryClass *factory);
	static void								Unlink_Factory (DefinitionFactoryClass *factory);

	/////////////////////////////////////////////////////////////////////
	//	Static member data
	/////////////////////////////////////////////////////////////////////
	static DefinitionFactoryClass *	_FactoryListHead;
};


#endif //__DEFINITION_FACTORY_MGR_H
