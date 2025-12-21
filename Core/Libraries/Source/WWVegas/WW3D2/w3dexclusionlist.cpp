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

/* $Header: /Commando/Code/ww3d2/w3dexclusionlist.cpp 1     12/12/02 3:36p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/w3dexclusionlist.cpp                   $*
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


#include "w3dexclusionlist.h"
#include "proto.h"
#include "htree.h"
#include "hanim.h"


W3DExclusionListClass::W3DExclusionListClass(const DynamicVectorClass<StringClass> & names) :
	Names(names)
{
	for (int i=0; i<Names.Count(); i++) {
		NameHash.Insert(Names[i],i);
	}
}

W3DExclusionListClass::~W3DExclusionListClass(void)
{
	NameHash.Remove_All();
}

bool	W3DExclusionListClass::Is_Excluded(PrototypeClass * proto) const
{
	StringClass copy = proto->Get_Name();
	char * root_name = copy.Peek_Buffer();

	// don't preserve munged prototypes
	if (strchr(root_name,'#') != nullptr) {
		return false;
	}

	// chop off the sub-object name if present (
	char * tmp = strchr(root_name,'.');
	if (tmp != nullptr) {
		*tmp = 0;
	}

	return Is_Excluded(root_name);
}


bool	W3DExclusionListClass::Is_Excluded(HTreeClass * htree) const
{
	// plain old name...
	return Is_Excluded(htree->Get_Name());
}


bool	W3DExclusionListClass::Is_Excluded(HAnimClass * hanim) const
{
	// For HAnims, the name to check is the one trailing the '.'
	StringClass copy = hanim->Get_Name();
	const char * root_name = copy.str();

	const char * tmp = strchr(root_name,'.');
	if (tmp) {
		return Is_Excluded(tmp + 1);
	} else {
		return false;  // should never happen...
	}
}


bool W3DExclusionListClass::Is_Excluded(const char * root_name) const
{
	return NameHash.Exists(StringClass(root_name));
}


