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

/* $Header: /Commando/Code/Tools/max2w3d/meshcon.h 26    10/27/00 4:11p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G Tools                                           *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/meshcon.h                      $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/27/00 10:31a                                             $*
 *                                                                                             *
 *                    $Revision:: 26                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef CHUNKIO_H
#include "chunkio.h"
#endif

#ifndef NODELIST_H
#include "nodelist.h"
#endif

#ifndef HIERSAVE_H
#include "hiersave.h"
#endif

#ifndef W3D_FILE
#include "w3d_file.h"
#endif

#ifndef VECTOR_H
#include "Vector.h"
#endif


class GeometryExportTaskClass;
class GeometryExportContextClass;


struct ConnectionStruct
{
	ConnectionStruct(void) : BoneIndex(0),MeshINode(NULL)
	{
		memset(ObjectName,0,sizeof(ObjectName));
	}

	int							BoneIndex;
	char							ObjectName[2*W3D_NAME_LEN];
	INode	*						MeshINode;

	// required by DynamicVectorClass...
	operator == (const ConnectionStruct & that) { return false; }
	operator != (const ConnectionStruct & that) { return !(*this==that); }
};


/**
** MeshConnectionsClass
** This class is the description of a hierarchical model.  It contains an array of "ConnectionStructs" which
** associate pieces of geometry with nodes in a hierarchy tree.
*/
class MeshConnectionsClass
{
public:

	MeshConnectionsClass(	DynamicVectorClass<GeometryExportTaskClass *> sub_objects,
									GeometryExportContextClass & context );

	~MeshConnectionsClass(void);

	/*
	** Get the name of the mesh connections object (will be
	** the name of the runtime HierarchyModel that this
	** object is describing.
	*/
	const char * Get_Name(void) const			{ return Name; }

	/*
	** Get the total number of meshes (of all types).
	*/
	int Get_Sub_Object_Count (void) const		{ return SubObjects.Count(); }
	int Get_Aggregate_Count(void) const			{ return Aggregates.Count(); }
	int Get_Proxy_Count(void) const				{ return ProxyObjects.Count(); }

	/*
	** Retrieve data about the mesh of the given index.
	** out_name - name of the mesh is passed back by setting the char* pointed to by this value.
	** out_boneindex - the index of the bone used is passed back by setting the int pointed to by this value.
	** out_inode - mesh INode is passed by setting the INode* pointed to by this value. If this
	**		parameter is NULL, the value is not passed back.
	*/
	bool Get_Sub_Object_Data(int index, char **out_name, int *out_boneindex, INode **out_inode = NULL);
	bool Get_Aggregate_Data(int index, char **out_name, int *out_boneindex, INode **out_inode = NULL);
	bool Get_Proxy_Data(int index, char **out_name, int *out_boneindex, INode **out_inode = NULL);

	/*
	** Returns the origin node used by this model.
	*/
	INode * Get_Origin (void) const				{ return Origin; }

private:

	TimeValue							CurTime;
	INode *								Origin;

	char									Name[W3D_NAME_LEN];

	// array of SubObjects
	DynamicVectorClass<ConnectionStruct>	SubObjects;
	DynamicVectorClass<ConnectionStruct>	Aggregates;
	DynamicVectorClass<ConnectionStruct>	ProxyObjects;


};
