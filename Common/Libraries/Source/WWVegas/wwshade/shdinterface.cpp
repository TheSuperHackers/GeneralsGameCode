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
 *                     $Archive:: wwshade/shdinterface.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 6/05/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                          $*
 *                                                                                             *
 *   6/05/02	 KJM : Added texture info functions
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "shdinterface.h"
#include "shddef.h"


//**********************************************************************************************
//! Constructor
/*!
	Base class constructor for shaders.  Saves a pointer to the definition for this shader.

	@param 
*/
ShdInterfaceClass::ShdInterfaceClass(const ShdDefClass * def, int class_id) : 
	Definition(NULL),
	ClassID(class_id)
{ 
	REF_PTR_SET(Definition,def); 
}


//**********************************************************************************************
//! Destructor
/*!
	Destructor for ShdInterfaceClass, releases the definition.
*/
ShdInterfaceClass::~ShdInterfaceClass(void) 
{ 
	REF_PTR_RELEASE(Definition); 
}


//**********************************************************************************************
//! returns a pointer to the definition for this shader
/*!
	@returns	
*/
const ShdDefClass * ShdInterfaceClass::Peek_Definition(void)
{
	return Definition;
}
