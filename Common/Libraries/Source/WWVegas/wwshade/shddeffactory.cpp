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
 *                     $Archive:: wwshade/shddeffactory.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 6/01/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "shddeffactory.h"
#include "shddefmanager.h"


/*
**
** ShdDefFactory Implementation
**
*/

//**********************************************************************************************
//! Constructor for ShdDefFactoryClass
/*!
	Automatically registers this instance of the ShdDefFactory with the manager.
*/
ShdDefFactoryClass::ShdDefFactoryClass (void) : 
	NextFactory(NULL), 
	PrevFactory(NULL) 
{ 
	ShdDefManagerClass::Register_Factory (this);
}


//**********************************************************************************************
//! Destructor for ShdDefFactoryClass
/*!
	Automatically un-registers this instance of a ShdDefFactory from the manager.
*/
ShdDefFactoryClass::~ShdDefFactoryClass (void) 
{
	ShdDefManagerClass::Unregister_Factory (this);
}



