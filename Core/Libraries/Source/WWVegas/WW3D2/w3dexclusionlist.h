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

/* $Header: /Commando/Code/ww3d2/w3dexclusionlist.h 1     12/12/02 3:36p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/w3dexclusionlist.h                     $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 12/12/02 10:04a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "Vector.h"
#include "wwstring.h"
#include "hashtemplate.h"

class PrototypeClass;
class HTreeClass;
class HAnimClass;


/**
** W3DExclusionListClass
** This class ecapsulates an "exclusion list" which the asset manager and related classes use
** to filter what resources get released.  This is useful between level loads for example.
** The Is_Excluded function uses w3d naming convention assumptions to determine whether the given
** asset name is in the list or is a child of something in the list.
*/

class W3DExclusionListClass
{
public:
	W3DExclusionListClass(const DynamicVectorClass<StringClass> & names);
	~W3DExclusionListClass(void);

	bool	Is_Excluded(PrototypeClass * proto) const;
	bool	Is_Excluded(HTreeClass * htree) const;
	bool	Is_Excluded(HAnimClass * hanim) const;

	bool	Is_Excluded(const char * root_name) const;

protected:


	const DynamicVectorClass<StringClass> &	Names;
	HashTemplateClass<StringClass,int>			NameHash;
};
