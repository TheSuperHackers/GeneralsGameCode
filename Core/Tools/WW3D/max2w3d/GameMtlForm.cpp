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
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlForm.cpp                $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/23/98 6:21p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "GameMtlForm.h"
#include "gamemtl.h"


GameMtlFormClass::GameMtlFormClass
(
	IMtlParams *	imtl_params,
	GameMtl *		mtl,
	int				pass
)
{
	IParams = imtl_params;
	TheMtl = mtl;
	PassIndex = pass;
}


void GameMtlFormClass::SetThing(ReferenceTarget * target)
{
	assert (target->SuperClassID()==MATERIAL_CLASS_ID);
	assert (target->ClassID()==GameMaterialClassID);

	TheMtl = (GameMtl *)target;
}


ReferenceTarget * GameMtlFormClass::GetThing()
{
	return (ReferenceTarget*)TheMtl;
}


void GameMtlFormClass::DeleteThis()
{
	delete this;
}


// Returns the classID of the object being edited.
Class_ID	GameMtlFormClass::ClassID()
{
	return GameMaterialClassID;
}


void GameMtlFormClass::SetTime(TimeValue t)
{
	// child dialog classes don't have to support
	// the SetTime function.
}
