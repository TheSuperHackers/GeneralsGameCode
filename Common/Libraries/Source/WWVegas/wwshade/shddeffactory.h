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
 *                     $Archive:: wwshade/shddeffactory.h                           $*
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

#ifndef SHDDEFFACTORY_H
#define SHDDEFFACTORY_H

#include "always.h"
#include "bittype.h"

class ShdDefClass;

/*
** NOTE:  For most users, the only thing you need from this module is the REGISTER_SHDDEF(T,ID,NAME) macro
*/


/**
** ShdDefFactoryClass - An instance of this class is used to automatically register
** each unique type of ShdDefClass with the system.  This object is responsible for
** creating shader definitions.  All existing 'DefFactories' can be iterated over 
** and presented to the user in a menu.
*/
class ShdDefFactoryClass 
{
public:
	ShdDefFactoryClass (void);
	virtual ~ShdDefFactoryClass (void);

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	virtual ShdDefClass *			Create (void) const = 0;
	virtual const char *				Get_Name (void) const = 0;
	virtual uint32						Get_Class_ID (void) const = 0;

protected:

	//////////////////////////////////////////////////////////////
	//	Protected member data
	//////////////////////////////////////////////////////////////
	ShdDefFactoryClass *				NextFactory;
	ShdDefFactoryClass *				PrevFactory;

	friend class ShdDefManagerClass;
};


/**
** SimpleShdDefFactoryClass - This template automates the process of creating a ShdDefFactory. 
** For complete ease of use, the associated REGISTER_SHDDEF macro can be used in the cpp file
** of your shader definition.
** Macro useage example (in the cpp file for your shader definition):
** REGISTER_SHDDEF(MyShdDefClass, MYSHDDEF_CLASSID, "Greg's super-fancy shader");
*/
template<class T, int class_id, char *name> class SimpleShdDefFactoryClass : public ShdDefFactoryClass
{
public:
	SimpleShdDefFactoryClass (void) { }
	virtual ~SimpleShdDefFactoryClass (void) { }

	//////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////
	virtual ShdDefClass *		Create (void) const					{ return new T; }
	virtual const char *			Get_Name (void) const				{ return name; }
	virtual uint32					Get_Class_ID (void) const			{ return class_id; }
};


/*
** Use this macro in the .CPP file for your shader definition to automatically link it
** into the shader editing system.
*/
#define REGISTER_SHDDEF(T,ID,NAME)							\
char T ## Name[] = NAME;										\
SimpleShdDefFactoryClass<T,ID,T ## Name> T ## Factory \



#endif //SHDDEFFACTORY_H
