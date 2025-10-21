/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/nullrobj.cpp                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "nullrobj.h"
#include "chunkio.h"

NullLoaderClass _NullLoader;




Null3DObjClass::Null3DObjClass(const char * name)
{
	strcpy(Name, name);
}

Null3DObjClass::Null3DObjClass(const Null3DObjClass & src)
{
	strcpy(Name, src.Name);
}

Null3DObjClass & Null3DObjClass::operator = (const Null3DObjClass & that)
{
	strcpy(Name, that.Name);

	RenderObjClass::operator = (that); return *this;
}

int Null3DObjClass::Class_ID(void) const
{
	return CLASSID_NULL;
}

RenderObjClass * Null3DObjClass::Clone(void) const
{
	return NEW_REF( Null3DObjClass, (*this));
}

void Null3DObjClass::Render(RenderInfoClass & rinfo)
{
}

void Null3DObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
   sphere.Center.Set(0,0,0);
	sphere.Radius = 0.0f;
}

void Null3DObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Center.Set(0,0,0);
	box.Extent.Set(0,0,0);
}

/*
** NullPrototypeClass
*/

NullPrototypeClass::NullPrototypeClass (void)
{
	// Note that the other members of the definition are uninitialized..
	// So don't rely on them if the name is "NULL".
	strcpy(Definition.Name, "NULL");
}

NullPrototypeClass::NullPrototypeClass (const W3dNullObjectStruct &null)
{
	Definition = null;
}


/*
** NullLoaderClass
*/

PrototypeClass * NullLoaderClass::Load_W3D (ChunkLoadClass &cload)
{
	W3dNullObjectStruct null;
	cload.Read(&null,sizeof(null));
	return W3DNEW NullPrototypeClass(null);
}
