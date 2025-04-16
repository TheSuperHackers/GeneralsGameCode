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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/geometryexporttask.h           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/27/00 5:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef GEOMETRYEXPORTTASK_H
#define GEOMETRYEXPORTTASK_H

#include <string.h>
#include <max.h>
#include "w3d_file.h"
#include "Vector.H"


class GeometryExportContextClass;


/**
** GeometryExportTaskClass
** This abstract base class defines the interface for a geometry export task.
** Derived classes will encapsulate the job of exporting meshes, collision
** boxes, dazzles, etc.  The factory function Create_Task will create the 
** appropriate task for a given INode.
*/
class GeometryExportTaskClass
{
public:

	GeometryExportTaskClass(INode * node,GeometryExportContextClass & context);
	GeometryExportTaskClass(const GeometryExportTaskClass & that);
	virtual ~GeometryExportTaskClass(void);

	virtual void							Export_Geometry(GeometryExportContextClass & context) = 0;

	/*
	** Accessors
	*/
	char *									Get_Name(void)								{ return Name; }
	char *									Get_Container_Name(void)				{ return ContainerName; }
	void										Get_Full_Name(char * buffer,int size);

	int										Get_Bone_Index(void)						{ return BoneIndex; }
	INode *									Get_Object_Node(void)					{ return Node; }
	Matrix3									Get_Export_Transform(void)				{ return ExportSpace; }

	void										Set_Name(char * name)					{ strncpy(Name,name,sizeof(Name)); }
	void										Set_Container_Name(char * name)		{ strncpy(ContainerName,name,sizeof(ContainerName)); }
	/*
	** Unique Name generation. During optimization, new meshes may get created.  When this happens,
	** we have to create a unique name for each one.  The name will be generated from the original
	** mesh's name, the index passed in, and the LOD level of the original mesh.
	*/
	void										Generate_Name(char * root,int index,GeometryExportContextClass & context);


	/*
	** Get vertex normal.  This function should return the normal of a vertex at the
	** specified x,y,z and smoothing_group if one exists.  It is used in the algorithm which
	** smooths the vertex normals on the boundaries of meshes.
	*/
	virtual Point3							Get_Shared_Vertex_Normal(const Point3 & world_space_point,int smgroup) { return Point3(0,0,0); }

	/*
	** Aggregate Model Detection.  An "aggregate" is an external W3D model that we are requesting
	** to be attached to a bone in the model being exported.  In order for our LOD system to work 
	** properly, some special handling of aggregates is required (they must be added into the model
	** as "additional models" rather than being placed in the normal LOD arrays).  This virtual
	** can be used to detect "aggregate" models.
	*/
	virtual bool							Is_Aggregate(void)						{ return false; }
	
	/*
	** Proxy Detection. A "proxy" is a reference (by name) to an external game object that should
	** be instantiated at the specified transform.  Like the aggregates, these had to unfortunately
	** be handled with special cases and therefore have this virtual function devoted solely to them.
	*/
	virtual bool							Is_Proxy(void)								{ return false; }

	/*
	** Virtual Constructor
	*/
	static GeometryExportTaskClass *	Create_Task(INode * node,GeometryExportContextClass & context);

	/*
	** Pre-Export Optimization of a set of geometry export tasks.  This does things like
	** separating multi-material meshes, combining small meshes which are nearby and use the
	** same material, etc.
	*/
	static void Optimize_Geometry(	DynamicVectorClass<GeometryExportTaskClass *> & tasks,
												GeometryExportContextClass & context	);

protected:

	/*
	** Internal RTTI
	*/
	enum 
	{
		MESH							= 0,
		COLLISIONBOX,
		DAZZLE,
		NULLOBJ,
		AGGREGATE,
		PROXY,
	};
	virtual int	Get_Geometry_Type(void) = 0;
			
protected:
	
	char					Name[W3D_NAME_LEN];
	char					ContainerName[W3D_NAME_LEN];
	int					BoneIndex;
	
	Matrix3				ExportSpace;
	TimeValue			CurTime;
	INode *				Node;
};



#endif //GEOMETRYEXPORTTASK_H

