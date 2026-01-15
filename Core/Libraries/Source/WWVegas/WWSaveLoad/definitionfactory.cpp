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
 *                     $Archive:: /Commando/Code/wwsaveload/definitionfactory.cpp             $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 9/09/99 7:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "definitionfactory.h"
#include "definitionfactorymgr.h"


/////////////////////////////////////////////////////////
//
//	DefinitionFactoryClass
//
/////////////////////////////////////////////////////////
DefinitionFactoryClass::DefinitionFactoryClass (void)
	:	m_NextFactory (nullptr),
		m_PrevFactory (nullptr)
{
	DefinitionFactoryMgrClass::Register_Factory (this);
	return ;
}


/////////////////////////////////////////////////////////
//
//	~DefinitionFactoryClass
//
/////////////////////////////////////////////////////////
DefinitionFactoryClass::~DefinitionFactoryClass (void)
{
	DefinitionFactoryMgrClass::Unregister_Factory (this);
	return ;
}
