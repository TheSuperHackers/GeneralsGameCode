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
 *                     $Archive:: /Commando/Code/wwsaveload/persist.h                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/04/01 8:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef PERSIST_H
#define PERSIST_H

#include "always.h"
#include "refcount.h"
#include "postloadable.h"

class PersistFactoryClass;
class ChunkSaveClass;
class ChunkLoadClass;


//////////////////////////////////////////////////////////////////////////////////
//
//	PersistClass
//
// PersistClass defines the interface for an object to the save load system.
// Each concrete derived type of PersistClass must have an associated
// PersistFactoryClass that basically maps a chunk_id to a constructor,
// a save function, a load function, and a on_post_load function (taken from
// PostLoadableClass )
//
//////////////////////////////////////////////////////////////////////////////////
class PersistClass : public PostLoadableClass
{
public:

	virtual const PersistFactoryClass &	Get_Factory (void) const			= 0;
	virtual bool								Save (ChunkSaveClass &csave)		{ return true; }
	virtual bool								Load (ChunkLoadClass &cload)		{ return true; }

};



#endif
