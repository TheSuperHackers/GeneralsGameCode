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
 *                     $Archive:: wwshade/shddefmanager.h                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 5/20/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                          $*
 *                                                                                             *
 * 5/20/02 KM Added save load behavior
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHDDEFMANAGER_H
#define SHDDEFMANAGER_H

#include "always.h"
#include "bittype.h"

class ShdDefClass;
class ShdDefFactoryClass;
class ChunkSaveClass;
class ChunkLoadClass;


/**
** ShdDefManagerClass - This class contains a list of all constructed ShdDefFactories.  
** This class is used to iterate through the shader definition factories in the material
** editor for example.  
*/
class ShdDefManagerClass
{
public:

	static ShdDefFactoryClass *		Find_Factory (uint32 class_id);	
	static ShdDefFactoryClass *		Find_Factory (const char *name);
	static void								Register_Factory (ShdDefFactoryClass *factory);
	static void								Unregister_Factory (ShdDefFactoryClass *factory);

	// Class enumeration
	static ShdDefFactoryClass *		Get_First (uint32 superclass_id);
	static ShdDefFactoryClass *		Get_Next (ShdDefFactoryClass *current, uint32 superclass_id);

	// Factory enumeration
	static ShdDefFactoryClass *		Get_First (void);
	static ShdDefFactoryClass *		Get_Next (ShdDefFactoryClass *current);
	
	// Construction
	static ShdDefClass *					Create_ShdDefClass_Instance(uint32 class_id);

	// save/load
	static void								Save_Shader(ChunkSaveClass& csave, ShdDefClass* shddef);
	static void								Load_Shader(ChunkLoadClass& cload, ShdDefClass** shddef);

private:

	static void								Link_Factory (ShdDefFactoryClass *factory);
	static void								Unlink_Factory (ShdDefFactoryClass *factory);

	static ShdDefFactoryClass *		_FactoryListHead;
};



#endif // SHDDEFMANAGER_H