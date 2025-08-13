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
 *                 Project Name : Max2W3D                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/geometryexporttask.cpp         $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/31/00 10:35a                                             $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GeometryExportTaskClass::GeometryExportTaskClass -- Constructor                           *
 *   GeometryExportTaskClass::GeometryExportTaskClass -- Copy Constructor                      *
 *   GeometryExportTaskClass::~GeometryExportTaskClass -- Destructor                           *
 *   GeometryExportTaskClass::Get_Full_Name -- Composes the full name of this robj             *
 *   GeometryExportTaskClass::Create_Task -- virtual constructor for export tasks              *
 *   GeometryExportTaskClass::Optimize_Geometry -- Optimizes the export tasks                  *
 *   GeometryExportTaskClass::Generate_Unique_Name -- create a unique name for this object     *
 *   MeshGeometryExportTaskClass::Is_Single_Material -- Tests if this mesh uses a single mater *
 *   MeshGeometryExportTaskClass::Get_Single_Material -- returns pointer to the material       *
 *   MeshGeometryExportTaskClass::Cache_Single_Material -- updates the cached material pointer *
 *   MeshGeometryExportTaskClass::Split -- Splits into single material meshes                  *
 *   MeshGeometryExportTaskClass::Reduce_To_Single_Material -- deletes polys                   *
 *   MeshGeometryExportTaskClass::Can_Combine -- can this mesh combine with anything           *
 *   MeshGeometryExportTaskClass::Can_Combine_With -- can this mesh be combined with the given *
 *   MeshGeometryExportTaskClass::Combine_Mesh -- Add the given mesh into this mesh            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "geometryexporttask.h"
#include "geometryexportcontext.h"
#include "util.h"
#include "w3dutil.h"
#include "w3dappdata.h"
#include "hiersave.h"
#include "maxworldinfo.h"
#include "meshsave.h"
#include "colboxsave.h"
#include "dazzlesave.h"
#include <bitarray.h>


const int		OPTIMIZATION_FACECOUNT_LIMIT	= 256;			// TODO: what should this number be...
const float		OPTIMIZATION_COMBINING_DISTANCE = 20.0f;		// TODO: need a smarter method for combining...


/**
** MeshGeometryExportTaskClass
** Export task for INodes which are to generate W3D meshes
*/
class MeshGeometryExportTaskClass : public GeometryExportTaskClass
{
public:

	MeshGeometryExportTaskClass(INode * node,GeometryExportContextClass & context) :
		GeometryExportTaskClass(node,context),
		NameDirty(false),
		SingleMtl(NULL)
	{
		/*
		** Copy the export options
		*/
		ExportOptions = *(W3DAppData2Struct::Get_App_Data(Node));

		/*
		** Copy the mesh
		*/
		Object *       obj = Node->EvalWorldState(CurTime).obj;
		TriObject *    tri = (TriObject *)obj->ConvertToType(CurTime, triObjectClassID);
		MeshData = tri->mesh;

		/*
		** Store a pointer to the material if this mesh uses only one material (even inside a Multi-Sub)
		*/
		Update_Cached_Data();
	}

	MeshGeometryExportTaskClass(const MeshGeometryExportTaskClass & that) :
		GeometryExportTaskClass(that),
		MeshData(that.MeshData),
		ExportOptions(that.ExportOptions),
		NameDirty(false)
	{
	}

	virtual ~MeshGeometryExportTaskClass(void)
	{
	}

	virtual void Export_Geometry(GeometryExportContextClass & context)
	{
		/*
		** Create the mesh
		*/
		context.WorldInfo.Set_Current_Task(this);
		context.WorldInfo.Set_Export_Transform(ExportSpace);

		MeshSaveClass * mesh = new MeshSaveClass(	Name,
																ContainerName,
																Node,
																&MeshData,
																ExportSpace,
																ExportOptions,
																context.HTree,
																context.CurTime,
																*context.ProgressMeter,
																context.materialColors,
																context.numMaterialColors,
																context.numHouseColors,
																context.materialColorTexture,
																&context.WorldInfo
																);

		/*
		** Export It
		*/
		mesh->Write_To_File(context.CSave,!context.Options.DisableExportAABTrees);
		delete mesh;
		context.ProgressMeter->Add_Increment();
	};

	/*
	** Naming.  During the optimization phase, sometimes new meshes are created and require
	** new unique names.  These meshes are flagged and then new names are generated prior
	** to exporting.
	*/
	bool	Is_Name_Dirty(void)				{ return NameDirty; }
	void	Set_Name_Dirty(bool onoff)		{ NameDirty = onoff; }

	/*
	** Vertex Normal smoothing support!
	*/
	virtual Point3		Get_Shared_Vertex_Normal(const Point3 & pos,int smgroup);

	/*
	** Optimization functions
	*/
	bool	Is_Single_Material(void);
	Mtl *	Get_Single_Material(void);
	void	Split(DynamicVectorClass<MeshGeometryExportTaskClass *> & simple_meshes);
	void	Reduce_To_Single_Material(int mat_id);
	bool	Can_Combine(void);
	bool	Can_Combine_With(MeshGeometryExportTaskClass * other_mesh);
	void	Combine_Mesh(MeshGeometryExportTaskClass * other_mesh);

protected:

	virtual int			Get_Geometry_Type(void) { return MESH; }
	void					Update_Cached_Data(void);

	Mesh					MeshData;			// Copy of the mesh data to be exported.
	W3DAppData2Struct	ExportOptions;		// Copy of the export options in case we want to change them during optimization
	bool					NameDirty;

	// Cached Data about the Node/Mesh.  Updated by calling Update_Cached_Data whenever the mesh changes.
	Mtl *					SingleMtl;			// Pointer to the single material (if the mesh uses only one, even in a multi-mtl)
	Point3				BoxCenter;			// Center of the bounding box (in object space)
	Point3				BoxExtent;			// Extent of the bounding box (in object space)
	Box3					WorldBounds;		// World-space bounding box
};





/**
** CollisionBoxGeometryExportTaskClass
** Export task for INodes which are to generate W3D AABoxes or OBBoxes
*/
class CollisionBoxGeometryExportTaskClass : public GeometryExportTaskClass
{
public:
	CollisionBoxGeometryExportTaskClass(INode * node,GeometryExportContextClass & context) :
		GeometryExportTaskClass(node,context)
	{
	}

	virtual void Export_Geometry(GeometryExportContextClass & context)
	{
		/*
		** Create the collision box
		*/
		CollisionBoxSaveClass * colbox = new CollisionBoxSaveClass(	Name,
																						ContainerName,
																						Node,
																						ExportSpace,
																						context.CurTime,
																						*context.ProgressMeter);

		/*
		** Export it
		*/
		colbox->Write_To_File(context.CSave);
		delete colbox;
		context.ProgressMeter->Add_Increment();
	};

protected:

	virtual int	Get_Geometry_Type(void) { return COLLISIONBOX; }

};





/**
** DazzleGeometryExportTaskClass
** Export task for INodes which are to generate W3D Dazzle objects
*/
class DazzleGeometryExportTaskClass : public GeometryExportTaskClass
{
public:
	DazzleGeometryExportTaskClass(INode * node,GeometryExportContextClass & context) :
		GeometryExportTaskClass(node,context)
	{
	}

	virtual void Export_Geometry(GeometryExportContextClass & context)
	{
		/*
		** Create the dazzle object
		*/
		DazzleSaveClass * dazzle = new DazzleSaveClass(	Name,
																		ContainerName,
																		Node,
																		ExportSpace,
																		context.CurTime,
																		*context.ProgressMeter);
		/*
		** Export it.
		*/
		dazzle->Write_To_File(context.CSave);
		delete dazzle;
		context.ProgressMeter->Add_Increment();
	};

protected:

	virtual int	Get_Geometry_Type(void) { return DAZZLE; }

};




/**
** NullGeometryExportTaskClass
** Export task for INodes which are to generate W3D NULL objects.  Note that this
** does not do anything in the Export_Geometry call, these only create entries in
** any Hierarhcical model or collection object being exported.
*/
class NullGeometryExportTaskClass : public GeometryExportTaskClass
{
public:
	NullGeometryExportTaskClass(INode * node,GeometryExportContextClass & context) :
		GeometryExportTaskClass(node,context)
	{
		memset(ContainerName,0,sizeof(ContainerName));
		memset(Name,0,sizeof(Name));
		strcpy(Name,"NULL");
	}

	virtual void Export_Geometry(GeometryExportContextClass & context)
	{
		context.ProgressMeter->Add_Increment();
	};

protected:

	virtual int	Get_Geometry_Type(void) { return NULLOBJ; }

};


/**
** AggregateGeometryExportTaskClass
** Export task for INodes which are to generate W3D Aggregates.  These are nodes
** that refer to some external W3D object.  This export task doesn't export any
** geometry (similer to the Null export task) and it clears its container name
** because the object to be attached is not a sub-object of the model we are
** currently exporting.
*/
class AggregateGeometryExportTaskClass : public GeometryExportTaskClass
{
public:
	AggregateGeometryExportTaskClass(INode * node,GeometryExportContextClass & context) :
		GeometryExportTaskClass(node,context)
	{
		memset(ContainerName,0,sizeof(ContainerName));
	}

	virtual void Export_Geometry(GeometryExportContextClass & context)
	{
		context.ProgressMeter->Add_Increment();
	};

	virtual bool Is_Aggregate(void)
	{
		return true;
	}

protected:

	virtual int	Get_Geometry_Type(void) { return AGGREGATE; }

};


/**
** ProxyExportTaskClass
** These are used by the Renegade Level Editor to cause game objects to be
** instantiated at the specified transform.  Like aggregates they have to
** be handled specially and therefore have the Is_Proxy member function devoted
** solely to them :-)  Hopefully we don't have any more geometry types which
** have to be handled specially or this is going to get messy again.
*/
class ProxyExportTaskClass : public GeometryExportTaskClass
{
public:

	ProxyExportTaskClass(INode * node,GeometryExportContextClass & context) :
		GeometryExportTaskClass(node,context)
	{
		/*
		** clear the container name
		*/
		memset(ContainerName,0,sizeof(ContainerName));

		/*
		** strip the trailing ~
		*/
		char *tilda	= ::strchr(Name, '~');
		memset(tilda,0,sizeof(Name) - ((int)tilda - (int)Name));

	}

	virtual void Export_Geometry(GeometryExportContextClass & context)
	{
		context.ProgressMeter->Add_Increment();
	};

	virtual bool Is_Proxy(void) { return true; }

protected:

	virtual int	Get_Geometry_Type(void) { return PROXY; }

};





/***********************************************************************************************
**
** Implementations
**
***********************************************************************************************/

/***********************************************************************************************
 * GeometryExportTaskClass::GeometryExportTaskClass -- Constructor                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/2000 gth : Created.                                                                 *
 *=============================================================================================*/
GeometryExportTaskClass::GeometryExportTaskClass(INode * node,GeometryExportContextClass & context) :
	BoneIndex(0),
	ExportSpace(1),
	CurTime(context.CurTime),
	Node(node)
{
	/*
	** Set up the names
	*/
	Set_W3D_Name(Name,Node->GetName());
	Append_Lod_Character(Name,Get_Lod_Level(context.Origin),context.OriginList);
	Set_W3D_Name(ContainerName,context.ModelName);

	/*
	** Set up the bone index and export coordinate system.
	*/
	if (context.HTree != NULL) {
		if (!Is_Skin(node)) {
			context.HTree->Get_Export_Coordinate_System(Node,&BoneIndex,NULL,&ExportSpace);
		} else {
			BoneIndex = 0;
			ExportSpace = context.OriginTransform;
		}
	}
}


/***********************************************************************************************
 * GeometryExportTaskClass::GeometryExportTaskClass -- Copy Constructor                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/2000 gth : Created.                                                                 *
 *=============================================================================================*/
GeometryExportTaskClass::GeometryExportTaskClass(const GeometryExportTaskClass & that) :
	BoneIndex(that.BoneIndex),
	ExportSpace(that.ExportSpace),
	CurTime(that.CurTime),
	Node(that.Node)
{
	Set_W3D_Name(Name,that.Name);
	Set_W3D_Name(ContainerName,that.ContainerName);
}


/***********************************************************************************************
 * GeometryExportTaskClass::~GeometryExportTaskClass -- Destructor                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
GeometryExportTaskClass::~GeometryExportTaskClass(void)
{
}


/***********************************************************************************************
 * GeometryExportTaskClass::Get_Full_Name -- Composes the full name of this robj               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void GeometryExportTaskClass::Get_Full_Name(char * buffer,int size)
{
	char tmp[128];
	memset(tmp,0,sizeof(tmp));
	if (strlen(ContainerName) > 0) {
		strcat(tmp,ContainerName);
		strcat(tmp,".");
	}
	strcat(tmp,Name);

	strncpy(buffer,tmp,size);
}


/***********************************************************************************************
 * GeometryExportTaskClass::Create_Task -- virtual constructor for export tasks                *
 *                                                                                             *
 * Virtual constructor for geometry export tasks.  Will create the proper task                 *
 * type depending on the W3D flag settings.                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/2000 gth : Created.                                                                 *
 *=============================================================================================*/
GeometryExportTaskClass *
GeometryExportTaskClass::Create_Task(INode * node,GeometryExportContextClass & context)
{
	if (!::Is_Geometry(node)) {
		return NULL;
	}

	// NOTE: we *have* to check Is_Proxy first because it is tied to a naming convention
	// rather than an explicit UI setting like the rest of the types.
	if (::Is_Proxy(*node)) {
		return new ProxyExportTaskClass(node,context);
	}

	if (::Is_Normal_Mesh(node) || Is_Camera_Aligned_Mesh(node) || Is_Camera_Oriented_Mesh(node) || Is_Skin(node)) {
		return new MeshGeometryExportTaskClass(node,context);
	}

	if (::Is_Collision_AABox(node) || Is_Collision_OBBox(node)) {
		return new CollisionBoxGeometryExportTaskClass(node,context);
	}

	if (::Is_Null_Object(node)) {
		return new NullGeometryExportTaskClass(node,context);
	}

	if (::Is_Dazzle(node)) {
		return new DazzleGeometryExportTaskClass(node,context);
	}

	if (::Is_Aggregate(node)) {
		return new AggregateGeometryExportTaskClass(node,context);
	}

	return NULL;
}


/***********************************************************************************************
 * GeometryExportTaskClass::Optimize_Geometry -- Optimizes the export tasks                    *
 *                                                                                             *
 *    This function will attempt to split meshes so that they use only a single material and   *
 *    then try to combine small meshes that use the same material.  Export tasks may be        *
 *    removed and new ones added.                                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 * tasks - dynamic vector of export task pointers.  Some tasks may be deleted, some added      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void GeometryExportTaskClass::Optimize_Geometry
(
	DynamicVectorClass<GeometryExportTaskClass *> & tasks,
	GeometryExportContextClass & context
)
{
	int j=0,i=0;

	/*
	** Pass 1: Extract all mesh geometry tasks from the input task array.
	** NOTE: We're not optimizing Skin meshes so we leave them in the task array.
	*/
	DynamicVectorClass<MeshGeometryExportTaskClass *> meshes;
	while (i<tasks.Count()) {
		if (	(tasks[i]->Get_Geometry_Type() == GeometryExportTaskClass::MESH) &&
				(!Is_Skin(tasks[i]->Get_Object_Node())) )
		{
			/*
			** Add to the mesh array, remove from the tasks array
			*/
			meshes.Add((MeshGeometryExportTaskClass *)(tasks[i]));
			tasks.Delete(i);
		} else {

			/*
			** Leave in the task array and move to the next one.
			*/
			i++;
		}
	}

	/*
	** Pass 2: Split all meshes which use more than one material
	*/
	DynamicVectorClass<MeshGeometryExportTaskClass *> simple_meshes;
	while (meshes.Count() > 0) {

		int cur_index = meshes.Count() - 1;
		MeshGeometryExportTaskClass * cur_mesh = meshes[cur_index];

		/*
		** If this mesh already uses only one material, just transfer it to the simple_meshes array.
		** Otherwise, have it split into new tasks, add them to the simple_meshes array, and delete this task.
		*/
		if (cur_mesh->Is_Single_Material()) {
			simple_meshes.Add(cur_mesh);
		} else {
			cur_mesh->Split(simple_meshes);
			delete cur_mesh;
		}
		meshes.Delete(cur_index);
	}

	/*
	** Pass 3: Combine meshes which satisfy the following
	** - They use the same (single) material
	** - They have fewer than 'x' polygons
	** - They are 'close' to each other
	*/
	i=0;
	while (i < simple_meshes.Count()) {

		if (simple_meshes[i]->Can_Combine()) {

			j=i+1;
			while (j < simple_meshes.Count()) {
				if (simple_meshes[i]->Can_Combine_With(simple_meshes[j])) {

					/*
					** Add mesh 'j' into mesh 'i', delete its task.
					*/
					simple_meshes[i]->Combine_Mesh(simple_meshes[j]);
					delete simple_meshes[j];
					simple_meshes.Delete(j);

					/*
					** If we've just exceeded the max poly count, move to the next mesh
					*/
					if (simple_meshes[i]->Can_Combine() == false) {
						j = simple_meshes.Count();
					}

				} else {

					/*
					** Otherwise, move to the next mesh
					*/
					j++;
				}
			}
		}
		i++;
	}



	/*
	** Generate names for each of the meshes that were created
	*/
	for (i=0; i<simple_meshes.Count(); i++) {
//		if (simple_meshes[i]->Is_Name_Dirty()) {
			simple_meshes[i]->Generate_Name("MESH",i,context);
//		}
	}

	/*
	** Finally, transfer all of the optimized tasks into the big task array
	*/
	for (i=0; i<simple_meshes.Count(); i++) {
		tasks.Add(simple_meshes[i]);
	}
	simple_meshes.Delete_All();

}


/***********************************************************************************************
 * GeometryExportTaskClass::Generate_Name -- create a name for this object                     *
 *                                                                                             *
 * During optimization, we can generate new meshes which need unique names.                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 * index - will be used to create the name, don't re-use the same index within an LOD          *
 * context - geometry export context                                                           *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/23/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void GeometryExportTaskClass::Generate_Name(char * root,int index,GeometryExportContextClass & context)
{
	/*
	** Check if the original had a Renegade "building prefix" in its name.  Building meshes
	** are named with a 2-3 letter user specified prefix followed by either '^' or '#'.  We
	** have to maintain this prefix on the optimized meshes...
	*/
	char prefix[5];
	strncpy(prefix,Name,sizeof(prefix));
	prefix[4] = 0;

	char * interior_prefix = strchr(prefix,'^');
	char * exterior_prefix = strchr(prefix,'#');

	memset(Name,0,sizeof(Name));
	if (interior_prefix != NULL) {
		strncpy(Name,prefix,(int)(interior_prefix - prefix) + 1);
	} else if (exterior_prefix != NULL) {
		strncpy(Name,prefix,(int)(exterior_prefix - prefix) + 1);
	}

	/*
	** Append "root"{index} to the name
	*/
	strcat(Name,root);
	strcat(Name,"{");
	sprintf(Name + strlen(Name),"%04d",index);
	strcat(Name,"}");

	/*
	** Append the LOD index so the generated name doesn't collide with meshes in the same
	** object but in other LOD's
	*/
	sprintf(Name + strlen(Name),"%c",'a'+Get_Lod_Level(context.Origin));
}


/***********************************************************************************************
 * MeshGeometryExportTaskClass::Update_Cached_Data -- updates the cached material pointer, etc *
 *                                                                                             *
 * This should be called after the meshdata changes                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/23/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void	MeshGeometryExportTaskClass::Update_Cached_Data(void)
{
	SingleMtl = NULL;
	Mtl *	nodemtl = Node->GetMtl();

	/*
	** Set the SingleMtl pointer if this mesh uses only one material (again, even if its in a Multi-Sub)
	*/
	if (nodemtl == NULL) {

		SingleMtl = NULL;

	} else if (nodemtl->NumSubMtls() <= 1) {

		SingleMtl = nodemtl;

	} else {

		int mat_index;
		int face_index;
		int sub_mtl_count = nodemtl->NumSubMtls();
		bool * sub_mtl_flags = new bool[sub_mtl_count];

		/*
		** Initialize each sub-material flag to false (indicates that the material is un-used)
		*/
		for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
			sub_mtl_flags[mat_index] = false;
		}

		/*
		** Set a true for each material actually referenced by the mesh
		*/
		for (face_index=0; face_index<MeshData.getNumFaces(); face_index++) {
			int max_mat_index = MeshData.faces[face_index].getMatID();
			int mat_index = (max_mat_index % sub_mtl_count);
			sub_mtl_flags[mat_index] = true;
		}

		/*
		** Loop over the used materials counting how many 'true's we see
		*/
		int mat_count = 0;
		for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
			if (sub_mtl_flags[mat_index]) {
				SingleMtl = nodemtl->GetSubMtl(mat_index);
				mat_count++;
			}
		}

		if (mat_count > 1) {
			SingleMtl = NULL;
		}
	}

	/*
	** Update the bounding box
	*/
	Point3 boxmin(0,0,0);
	Point3 boxmax(0,0,0);

	if (MeshData.numVerts > 0) {
		boxmin = MeshData.verts[1];
		boxmax = MeshData.verts[0];
		for (int i=0; i<MeshData.numVerts; i++) {
			boxmin.x = MIN(MeshData.verts[i].x,boxmin.x);
			boxmin.y = MIN(MeshData.verts[i].y,boxmin.y);
			boxmin.z = MIN(MeshData.verts[i].z,boxmin.z);

			boxmax.x = MAX(MeshData.verts[i].x,boxmax.x);
			boxmax.y = MAX(MeshData.verts[i].y,boxmax.y);
			boxmax.z = MAX(MeshData.verts[i].z,boxmax.z);
		}
	}

	boxmax += Point3(0.1f,0.1f,0.1f);
	boxmin -= Point3(0.1f,0.1f,0.1f);

	BoxCenter = (boxmax + boxmin) * 0.5f;
	BoxExtent = (boxmax - boxmin) * 0.5f;

	WorldBounds = Box3(boxmin,boxmax) * Node->GetObjectTM(CurTime);
}


/***********************************************************************************************
 * MeshGeometryExportTaskClass::Is_Single_Material -- Tests if this mesh uses a single materia *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool MeshGeometryExportTaskClass::Is_Single_Material(void)
{
	return ((SingleMtl != NULL) || (Node->GetMtl() == NULL));
}


/***********************************************************************************************
 * MeshGeometryExportTaskClass::Get_Single_Material -- returns pointer to the material         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/23/2000 gth : Created.                                                                 *
 *=============================================================================================*/
Mtl* MeshGeometryExportTaskClass::Get_Single_Material(void)
{
	return SingleMtl;
}

/***********************************************************************************************
 * MeshGeometryExportTaskClass::Split -- Splits into single material meshes                    *
 *                                                                                             *
 * This function will create new export tasks and add them to the supplied array.  Each of     *
 * these will be single-material meshes.                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryExportTaskClass::Split(DynamicVectorClass<MeshGeometryExportTaskClass *> & simple_meshes)
{
	assert(!Is_Single_Material());

	Mtl *				nodemtl = Node->GetMtl();

	int mat_index;
	int face_index;
	int sub_mtl_count = nodemtl->NumSubMtls();
	bool * sub_mtl_flags = new bool[sub_mtl_count];

	/*
	** Initialize each sub-material flag to false (indicates that the material is un-used)
	*/
	for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
		sub_mtl_flags[mat_index] = false;
	}

	/*
	** Set a true for each material actually referenced by the mesh
	*/
	for (face_index=0; face_index<MeshData.getNumFaces(); face_index++) {
		int max_mat_index = MeshData.faces[face_index].getMatID();
		int mat_index = (max_mat_index % sub_mtl_count);
		sub_mtl_flags[mat_index] = true;
	}

	/*
	** Loop over the used materials, creating a new MeshGeometryExportTaskClass for each one
	*/
	for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
		if (sub_mtl_flags[mat_index]) {

			MeshGeometryExportTaskClass * new_task = new MeshGeometryExportTaskClass(*this);
			new_task->Reduce_To_Single_Material(mat_index);
			new_task->Set_Name_Dirty(true);
			simple_meshes.Add(new_task);

		}
	}
}


/***********************************************************************************************
 * MeshGeometryExportTaskClass::Reduce_To_Single_Material -- deletes polys                     *
 *                                                                                             *
 *    This function deletes all polys (and subsequent un-used vertices) except for the ones    *
 *    that use the specified material.                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 * mat_id - only faces using this material id will remain in the mesh.                         *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryExportTaskClass::Reduce_To_Single_Material(int mat_id)
{
	int sub_mtl_count = Node->GetMtl()->NumSubMtls();

	BitArray faces_to_delete(MeshData.getNumFaces());
	BitArray verts_to_delete(MeshData.getNumVerts());

	faces_to_delete.ClearAll();
	verts_to_delete.ClearAll();

	for (int i=0; i<MeshData.getNumFaces(); i++) {
		if ((MeshData.faces[i].getMatID() % sub_mtl_count) != mat_id) {
			faces_to_delete.Set(i,true);
		}
	}

	MeshData.DeleteFaceSet(faces_to_delete,&verts_to_delete);
	MeshData.DeleteVertSet(verts_to_delete);

	Update_Cached_Data();
}


/***********************************************************************************************
 * MeshGeometryExportTaskClass::Can_Combine -- can this mesh combine with anything             *
 *                                                                                             *
 *    This will return false if the mesh has multiple materials, too many polys, etc.          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool MeshGeometryExportTaskClass::Can_Combine(void)
{
	/*
	** This mesh can't combine at all if:
	** - it has multiple materials
	** - its polygon count is too high
	** - (Renegade) VIS is enabled (don't want to create really wierd shaped vis-sectors...)
	*/
	if (!Is_Single_Material()) {
		return false;
	}
	if (MeshData.numFaces > OPTIMIZATION_FACECOUNT_LIMIT) {
		return false;
	}
	if (ExportOptions.Is_Vis_Collision_Enabled()) {
		return false;
	}
	return true;
}


/***********************************************************************************************
 * MeshGeometryExportTaskClass::Can_Combine_With -- can this mesh be combined with the given m *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool MeshGeometryExportTaskClass::Can_Combine_With(MeshGeometryExportTaskClass * other_mesh)
{
	/*
	** Does the mesh attach to the same W3D bone that we do?
	*/
	if (other_mesh->BoneIndex != BoneIndex) {
		return false;
	}

	/*
	** Does the mesh use the same (single) material that we do?
	*/
	Mtl * other_mtl = other_mesh->Get_Single_Material();
	if (other_mtl == NULL) {
		return false;
	}

	Mtl * my_mtl = Get_Single_Material();
	if (my_mtl != other_mtl) {
		return false;
	}

	/*
	** Are its relevant W3D options the same as ours?
	*/
	if (ExportOptions.Geometry_Options_Match(other_mesh->ExportOptions)) {
		return false;
	}

	/*
	** Would our combined polygon count be reasonable
	*/
	if (MeshData.numFaces + other_mesh->MeshData.numFaces > OPTIMIZATION_FACECOUNT_LIMIT) {
		return false;
	}

	/*
	** Is the other mesh near me?
	*/
	Point3 my_center = Node->GetObjectTM(CurTime) * BoxCenter;
	Point3 other_center = other_mesh->Node->GetObjectTM(CurTime) * BoxCenter;
	float dist = ::FLength(my_center - other_center);
	if (dist > OPTIMIZATION_COMBINING_DISTANCE) {
		return false;
	}

	return true;
}


/***********************************************************************************************
 * MeshGeometryExportTaskClass::Combine_Mesh -- Add the given mesh into this mesh              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void MeshGeometryExportTaskClass::Combine_Mesh(MeshGeometryExportTaskClass * other_mesh)
{
	/*
	** Compute the transform from other_mesh's coordinate system to ours so that
	** its polygons can be combined with ours (by calling CombineMeshes)
	*/
	Matrix3 our_tm = Node->GetObjectTM(CurTime);
	Matrix3 his_tm = other_mesh->Node->GetObjectTM(CurTime);
	Matrix3 tm = Inverse(our_tm) * his_tm;

	/*
	** Store our current material index
	*/
	int matid = MeshData.faces[0].getMatID();
	if (Node->GetMtl()->NumSubMtls() > 1) {
		matid = matid % Node->GetMtl()->NumSubMtls();
	}

	/*
	** Combine the meshes
	*/
	Mesh new_mesh;
	::CombineMeshes(new_mesh,MeshData,other_mesh->MeshData,&our_tm,&his_tm,0);
	MeshData = new_mesh;

	/*
	** Set all material ID's
	*/
	for (int i=0; i<MeshData.numFaces; i++) {
		MeshData.faces[i].setMatID(matid);
	}
}

Point3 MeshGeometryExportTaskClass::Get_Shared_Vertex_Normal(const Point3 & world_pos,int smgroup)
{
	const float EPSILON = 0.001f;
	Point3 normal(0,0,0);

	/*
	**	Does the bounding box for this node even contain the point
	**	 we are looking for?
	*/
	if (WorldBounds.Contains(world_pos) != 0) {

		/*
		** Transform the query point into object space
		*/
		Matrix3 tm = Node->GetObjectTM(CurTime);
		Point3 obj_pos = world_pos * Inverse(tm);

		/*
		**	Loop through all the faces in this mesh and find out which ones
		** share the same smoothing group as the vertex we are looking for.
		*/
		for (int face_index = 0; face_index < MeshData.numFaces; face_index ++) {
			Face &maxface = MeshData.faces[face_index];
			int face_smgroup = maxface.getSmGroup();
			if ((face_smgroup & smgroup) || (face_smgroup == smgroup)) {

				/*
				**	Find out if any of the verticies of this face share the
				** same space as the vertex we are looking for.
				*/
				bool found = false;
				for (int vert_index = 0; (vert_index < 3) && !found; vert_index ++) {
					int max_vert_index = maxface.v[vert_index];
					Point3 delta = obj_pos - MeshData.verts[max_vert_index];
					if ((fabs (delta.x) < EPSILON) &&
						 (fabs (delta.y) < EPSILON) &&
						 (fabs (delta.z) < EPSILON))
					{

						/*
						**	Compute the normal for this face
						*/
						Point3 v0 = MeshData.verts[maxface.v[0]];
						Point3 v1 = MeshData.verts[maxface.v[1]];
						Point3 v2 = MeshData.verts[maxface.v[2]];
						Point3 face_normal = (v1-v0)^(v2-v1);
						face_normal = ::Normalize(face_normal);

						/*
						**	Add this face normal to the sum
						*/
						normal.x += face_normal.x;
						normal.y += face_normal.y;
						normal.z += face_normal.z;

						/*
						** Done with this face, look for more
						*/
						found = true;
					}
				}
			}
		}

		/*
		** Transform the "normal" to world space.  Note that this vector isn't
		** normalized because we are basically summing the contributions of each
		** face in each mesh which shares this normal.  The final normal
		** will be normalized in the MeshBuilderClass.
		*/
		tm.NoTrans();
		normal = tm.PointTransform(normal);
	}

	return normal;
}
