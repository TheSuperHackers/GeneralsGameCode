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
 *                 Project Name : WWShade                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwshade/shdloader.h                          $*
 *                                                                                             *
 *                   Org Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/27/02 3:51p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * 06/26/02 KM Integrating shader system
 *---------------------------------------------------------------------------------------------*/

#ifndef SHDLOADER_H
#define SHDLOADER_H

#ifndef PROTO_H
#include "proto.h"
#endif

class ShdMeshLoaderClass : public PrototypeLoaderClass
{
public:

	virtual int						Chunk_Type(void) { return W3D_CHUNK_SHDMESH; }
	virtual PrototypeClass*		Load_W3D(ChunkLoadClass& cload);
};

/*
** Prototype loader that converts legacy meshes into Shader meshes
*/
class ShdMeshLegacyLoaderClass : public PrototypeLoaderClass
{
public:

	virtual int						Chunk_Type(void) { return W3D_CHUNK_MESH; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};


extern ShdMeshLoaderClass			_ShdMeshLoader;
extern ShdMeshLegacyLoaderClass	_ShdMeshLegacyLoader;

#endif
