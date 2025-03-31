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
 *                 Project Name : WWShade                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwshade/shdloader.cpp                        $*
 *                                                                                             *
 *                   Org Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/27/02 3:51p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * 06/26/02 KM Abstracting shader system
 *---------------------------------------------------------------------------------------------*/

#include "shdloader.h"
#include "shdmesh.h"
#include "mesh.h"

ShdMeshLoaderClass			_ShdMeshLoader;
ShdMeshLegacyLoaderClass	_ShdMeshLegacyLoader;

/***********************************************************************************************
 * ShdMeshLoaderClass::Load -- reads in a shader mesh and creates a prototype for it           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/21/02    KJM : Created.                                                                 *
 *=============================================================================================*/
PrototypeClass* ShdMeshLoaderClass::Load_W3D(ChunkLoadClass& cload)
{
	ShdMeshClass* mesh=NEW_REF(ShdMeshClass, ());

	if (mesh==NULL) 
	{
		return NULL;
	}

	if (mesh->Load_W3D(cload)!=WW3D_ERROR_OK) 
	{
		// if the load failed, delete the mesh
		assert(mesh->Num_Refs() == 1);
		mesh->Release_Ref();
		return NULL;

	}

	// create the prototype and add it to the lists
	PrimitivePrototypeClass * newproto = new PrimitivePrototypeClass(mesh);
	mesh->Release_Ref();
	return newproto;
}


PrototypeClass * ShdMeshLegacyLoaderClass::Load_W3D(ChunkLoadClass & cload)
{
	MeshClass * mesh = NEW_REF( MeshClass, () );

	if (mesh == NULL) {
		return NULL;
	}

	if (mesh->Load_W3D(cload) != WW3D_ERROR_OK) {

		// if the load failed, delete the mesh
		assert(mesh->Num_Refs() == 1);
		mesh->Release_Ref();
		return NULL;

	} else {
		ShdMeshClass* shdmesh=NEW_REF( ShdMeshClass, () );
		shdmesh->Init_From_Legacy_Mesh(mesh);
		mesh->Release_Ref();

		// create the prototype and add it to the lists
		PrimitivePrototypeClass * newproto = W3DNEW PrimitivePrototypeClass(shdmesh);
		shdmesh->Release_Ref();
		return newproto;
	
	}
}
