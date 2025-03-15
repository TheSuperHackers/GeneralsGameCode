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
 *                     $Archive:: wwshade/shdmesh.cpp														$*
 *                                                                                             *
 *                   Org Author:: Jani P                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 07/12/02 10:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *---------------------------------------------------------------------------------------------*/

#include "shdmesh.h"
#include "shdsubmesh.h"
#include "shdrenderer.h"
#include "rinfo.h"
#include "camera.h"
#include "dx8wrapper.h"
#include "wwdebug.h"
#include "wwprofile.h"
#include "mesh.h"
#include "meshmdl.h"

ShdMeshClass::ShdMeshClass() 
: 	Name("UnNamed"),
	LightEnvironment(NULL),
	Applying_Shadow_Map(false)
{
}

ShdMeshClass::ShdMeshClass(const ShdMeshClass & src) 
:	RenderObjClass(src),	
	Name(src.Name),
	LightEnvironment(NULL),
	Applying_Shadow_Map(false)
{
	Free();
	SubMeshes.Resize(src.SubMeshes.Length());
	for (int i=0;i<SubMeshes.Length();++i) 
	{
		SubMeshes[i].Mesh=NULL;
		SubMeshes[i].Renderer=NULL;
		REF_PTR_SET(SubMeshes[i].Mesh,src.SubMeshes[i].Mesh);
	}
}

ShdMeshClass::~ShdMeshClass()
{
	Free();
	// TODO TODO TODO!!!!!
}

RenderObjClass * ShdMeshClass::Clone() const
{
	return new ShdMeshClass(*this);
}

const char * ShdMeshClass::Get_Name() const
{
	return Name;
}

void ShdMeshClass::Set_Name(const char * name)
{
	Name = name;
}

/***********************************************************************************************
 * ShdMeshClass::Free -- Releases all memory/assets in use by this mesh                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/6/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void ShdMeshClass::Free()
{
	for (int i=0;i<SubMeshes.Length();++i)
	{
		REF_PTR_RELEASE(SubMeshes[i].Mesh);
		if (SubMeshes[i].Renderer) 
		{
			delete SubMeshes[i].Renderer;
			SubMeshes[i].Renderer=NULL;
		}
	}
}
 
int ShdMeshClass::Get_Num_Polys() const
{
	int count = 0;
	for (int i=0; i<SubMeshes.Length(); i++) 
	{
		count+= SubMeshes[i].Mesh->Get_Polygon_Count();
	}
	return count;
}

int ShdMeshClass::Get_Num_Vertices(void) const
{
	int count = 0;
	for (int i=0; i<SubMeshes.Length(); i++) 
	{
		count+= SubMeshes[i].Mesh->Get_Vertex_Count();
	}
	return count;
}


void ShdMeshClass::Render(RenderInfoClass& rinfo)
{
	WWPROFILE("ShdMeshClass::Render");
	if (Is_Not_Hidden_At_All() == false) 
	{
		return;
	}

//	DX8_RECORD_MESH_RENDER();

	// TODO: Static sort lists
	Set_Lighting_Environment(rinfo.light_environment);
	const FrustumClass & frustum=rinfo.Camera.Get_Frustum();

	// if rendering shadow remember camera info
	if ((rinfo.Current_Override_Flags()&RenderInfoClass::RINFO_OVERRIDE_SHADOW_RENDERING))
	{
		// is generating shadow map
		Set_Is_Self_Shadowed();

		// set texture projector
		Texture_Projector=rinfo.Texture_Projector;

		Unset_Is_Applying_Shadow_Map();
	}
	else if (Is_Self_Shadowed())
	{
		// is applying shadow map
		Set_Is_Applying_Shadow_Map();
		Unset_Is_Self_Shadowed();
	}
	else
	{
		Unset_Is_Applying_Shadow_Map();
	}


	// TODO: What to do with SKINS?
	if (1)//CollisionMath::Overlap_Test(frustum,Get_Bounding_Box())!=CollisionMath::OUTSIDE ) 
	{
//		bool rendered_something = false;

		// TODO: Override flags, decals and material passes (probably in the submesh rendering)
		for (int i=0;i<SubMeshes.Length();++i) 
		{
			if (!SubMeshes[i].Renderer) 
			{
				SubMeshes[i].Renderer=ShdRendererClass::Peek_Instance()->Register_Mesh(this,SubMeshes[i].Mesh);
			}

			SubMeshes[i].Renderer->Render(rinfo);
		}

		// TODO: RendererDebugger
	}


}

//void ShdMeshClass::Render_Material_Pass(MaterialPassClass * pass,IndexBufferClass * ib)
//{
//
//}

void ShdMeshClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
}

/***********************************************************************************************
 * ShdMeshClass::Cast_Ray -- compute a ray intersection with this mesh                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/17/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool ShdMeshClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	if ((Get_Collision_Type() & raytest.CollisionType) == 0) return false;	
	if ((Is_Translucent()!=0) && (!raytest.CheckTranslucent)) return false;
	if (Is_Animation_Hidden()) return false;
	if (raytest.Result->StartBad) return false;

	Matrix3D world_to_obj;
	Matrix3D world=Get_Transform();

	// if aligned or oriented rotate the mesh so that it's aligned to the ray
/*	if (Model->Get_Flag(MeshModelClass::ALIGNED)) {
			Vector3 mesh_position;
			world.Get_Translation(&mesh_position);
			world.Obj_Look_At(mesh_position,mesh_position - raytest.Ray.Get_Dir(),0.0f);
	} else if (Model->Get_Flag(MeshModelClass::ORIENTED)) {		
			Vector3 mesh_position;
			world.Get_Translation(&mesh_position);
			world.Obj_Look_At(mesh_position,raytest.Ray.Get_P0(),0.0f);
	}
*/
	world.Get_Orthogonal_Inverse(world_to_obj);
	RayCollisionTestClass objray(raytest,world_to_obj);

	for (int i=0;i<SubMeshes.Length();++i) {
		if (SubMeshes[i].Mesh->Cast_Ray(objray)) {
			// transform result back into original coordinate system
			raytest.CollidedRenderObj = this;
			Matrix3D::Rotate_Vector(world,raytest.Result->Normal, &(raytest.Result->Normal));
			if (raytest.Result->ComputeContactPoint) {
				Matrix3D::Transform_Vector(world,raytest.Result->ContactPoint, &(raytest.Result->ContactPoint));
			}
			return true;
		}
	}
	
	return false;
}

/***********************************************************************************************
 * ShdMeshClass::Cast_AABox -- cast an AABox against this mesh                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/17/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool ShdMeshClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	for (int i=0;i<SubMeshes.Length();++i) {
		// This function analyses the tranform to call optimized functions in certain cases
		if (SubMeshes[i].Mesh->Cast_World_Space_AABox(boxtest, Get_Transform())) {
			boxtest.CollidedRenderObj = this;
			return true;
		}
	}

	return false;
}

/***********************************************************************************************
 * Cast_OBBox -- Cast an obbox against this mesh                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/17/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool ShdMeshClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	/*
	** transform into the local coordinate system of the mesh.
	*/
	const Matrix3D & tm = Get_Transform();
	Matrix3D world_to_obj;
	tm.Get_Orthogonal_Inverse(world_to_obj);
	OBBoxCollisionTestClass localtest(boxtest,world_to_obj);

	for (int i=0;i<SubMeshes.Length();++i) {
		if (SubMeshes[i].Mesh->Cast_OBBox(localtest)) {

		/*
		** If we hit, transform the result of the test back to the original coordinate system.
		*/
			boxtest.CollidedRenderObj = this;
			Matrix3D::Rotate_Vector(tm,boxtest.Result->Normal, &(boxtest.Result->Normal));
			if (boxtest.Result->ComputeContactPoint) {
				Matrix3D::Transform_Vector(tm,boxtest.Result->ContactPoint, &(boxtest.Result->ContactPoint));
			}
			return true;
		}
	}

	return false;
}

/***********************************************************************************************
 * ShdMeshClass::Intersect_AABox -- test for intersection with given AABox                     *
 *                                                                                             *
 * The AAbox given is assumed to be in world space.  Since meshes aren't generally in world    *
 * space, the test must be transformed into our local coordinate system (which turns it into   *
 * an OBBox...)                                                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool ShdMeshClass::Intersect_AABox(AABoxIntersectionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;

	Matrix3D inv_tm;
	Get_Transform().Get_Orthogonal_Inverse(inv_tm);
	OBBoxIntersectionTestClass local_test(boxtest,inv_tm);
	
	for (int i=0;i<SubMeshes.Length();++i) {
		if (SubMeshes[i].Mesh->Intersect_OBBox(local_test)) return true;
	}
	return false;
}

/***********************************************************************************************
 * ShdMeshClass::Intersect_OBBox -- test for intersection with the given OBBox                 *
 *                                                                                             *
 * The given OBBox is assumed to be in world space so we need to transform it into the mesh's  *
 * local coordinate system.                                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
bool ShdMeshClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;

	Matrix3D inv_tm;
	Get_Transform().Get_Orthogonal_Inverse(inv_tm);
	OBBoxIntersectionTestClass local_test(boxtest,inv_tm);

	for (int i=0;i<SubMeshes.Length();++i) {
		if (SubMeshes[i].Mesh->Intersect_OBBox(local_test)) return true;
	}
	return false;
}

/***********************************************************************************************
 * ShdMeshClass::Add_Dependencies_To_List -- Add dependent files to the list.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/18/99    PDS : Created.                                                                 *
 *   6/05/02	 KJM : Added dependency calculation for shader system
 *=============================================================================================*/
void ShdMeshClass::Add_Dependencies_To_List
(
	DynamicVectorClass<StringClass> &file_list,
	bool										textures_only
)
{
	// loop through sub meshes
	for (int i=0;i<SubMeshes.Length();i++) 
	{
		ShdInterfaceClass* shd=SubMeshes[i].Mesh->Peek_Shader();
			
		for (int tidx=0;tidx<shd->Get_Texture_Count();tidx++)
		{
			TextureClass* texture=shd->Peek_Texture(tidx);	
			if (texture)
			{
				file_list.Add(texture->Get_Full_Path());
			}
		}
	}

	RenderObjClass::Add_Dependencies_To_List (file_list, textures_only);
	return ;
}

/***********************************************************************************************
 * ShdMeshClass::Update_Cached_Bounding_Volumes -- default collision sphere.                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/14/2001    NH : Created.                                                                *
 *=============================================================================================*/
void ShdMeshClass::Update_Cached_Bounding_Volumes(void) const
{
	Get_Obj_Space_Bounding_Sphere(CachedBoundingSphere);

	Matrix3D::Transform_Vector(Get_Transform(),CachedBoundingSphere.Center,&CachedBoundingSphere.Center);

	// If we are camera-aligned or -oriented, we don't know which way we are facing at this point,
	// so the box we return needs to contain the sphere. Otherewise do the normal computation.
/*	if (Model->Get_Flag(MeshModelClass::ALIGNED) || Model->Get_Flag(MeshModelClass::ORIENTED)) {
		CachedBoundingBox.Center = CachedBoundingSphere.Center;
		CachedBoundingBox.Extent.Set(CachedBoundingSphere.Radius, CachedBoundingSphere.Radius, CachedBoundingSphere.Radius);
	} else {
*/		Get_Obj_Space_Bounding_Box(CachedBoundingBox);
		CachedBoundingBox.Transform(Get_Transform());
//	}

	Validate_Cached_Bounding_Volumes();
}

/***********************************************************************************************
 * ShdMeshClass::Get_Obj_Space_Bounding_Sphere -- returns obj-space bounding sphere            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void ShdMeshClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	if (SubMeshes.Length()) {
		SubMeshes[0].Mesh->Get_Bounding_Sphere(&sphere);
		// If there are more than one submesh, merge all bounding spheres
		for (int i=1;i<SubMeshes.Length();++i) {
			SphereClass tmp_s;
			SubMeshes[i].Mesh->Get_Bounding_Sphere(&tmp_s);
			sphere+=tmp_s;
		}
	}
	else {
		sphere.Center.Set(0,0,0);
		sphere.Radius = 1.0f;
	}
}

/***********************************************************************************************
 * ShdMeshClass::Get_Obj_Space_Bounding_Box -- returns the obj-space bounding box                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/00    gth : Created.                                                                 *
 *=============================================================================================*/
void ShdMeshClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	if (SubMeshes.Length()) {
		SubMeshes[0].Mesh->Get_Bounding_Box(&box);
		// If there are more than one submesh, merge all bounding boxes
		for (int i=1;i<SubMeshes.Length();++i) {
			AABoxClass tmp_b;
			SubMeshes[i].Mesh->Get_Bounding_Box(&tmp_b);
			
			box.Add_Box(tmp_b);
		}
	}
	else {
		box.Init(Vector3(0,0,0),Vector3(1,1,1));
	}
}

void ShdMeshClass::Init_From_Legacy_Mesh(MeshClass* mesh)
{
	Set_Name(mesh->Get_Name());
	MeshModelClass* model=mesh->Peek_Model();
	if (model) 
	{
		int first_poly=0;
		int sub_mesh_count=0;
		while (first_poly<model->Get_Polygon_Count()) {
			sub_mesh_count++;
			ShdSubMeshClass * sub_mesh = NEW_REF( ShdSubMeshClass, () );
			sub_mesh->Init_From_Legacy_Mesh_Model(model,first_poly);
			SubMeshes.Resize(sub_mesh_count);
			SubMeshes[sub_mesh_count-1].Mesh=sub_mesh;
			SubMeshes[sub_mesh_count-1].Renderer=NULL;
			if (sub_mesh->Get_Visible_Polygon_Count()==0) break;
			first_poly+=sub_mesh->Get_Visible_Polygon_Count();
		}
	}

	// Pull interesting stuff out of the w3d attributes bits
	Set_Collision_Type(mesh->Get_Collision_Type());
	Set_Hidden(mesh->Is_Hidden());
	Set_Translucent(mesh->Is_Translucent());	
}



/*********************************************************************************************** 
 * ShdMeshClass::Load -- creates a shader mesh out of a shader mesh chunk in a .w3d file       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * 																														  * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/21/2002 KM  : Created.                                                                 * 
 *=============================================================================================*/
WW3DErrorType ShdMeshClass::Load_W3D(ChunkLoadClass& cload)
{
	//	Open the first chunk, it should be the shader mesh name
	cload.Open_Chunk();
	
	if (cload.Cur_Chunk_ID()!=W3D_CHUNK_SHDMESH_NAME)
	{
		WWDEBUG_SAY(("Invalid format shader mesh.\n"));
		return WW3D_ERROR_LOAD_FAILED;
	}

	// read name
	char buf[MAX_PATH];
	cload.Read(buf,cload.Cur_Chunk_Length());
	cload.Close_Chunk();
	Set_Name(buf);

	// open header
	W3dShdMeshHeaderStruct hdr;
	cload.Open_Chunk();
	if 
	(
		cload.Read
		(
			&hdr,
			sizeof(W3dShdMeshHeaderStruct)
		)!=sizeof(W3dShdMeshHeaderStruct)
	)
	{
		return WW3D_ERROR_LOAD_FAILED;
	}
	cload.Close_Chunk();

	// Process the header

	// Set Bounding Info
	Vector3 min(hdr.BoxMin.X,hdr.BoxMin.Y,hdr.BoxMin.Z);
	Vector3 max(hdr.BoxMax.X,hdr.BoxMax.Y,hdr.BoxMax.Z);

	CachedBoundingBox.Init_Min_Max(min,max);

	Vector3 cntr(hdr.SphCenter.X,hdr.SphCenter.Y,hdr.SphCenter.Z);
	CachedBoundingSphere.Init(cntr,hdr.SphRadius);

	// Flags (todo)

	// user text
	

	// next are the sub meshes
	Free();
	SubMeshes.Resize(hdr.NumSubMeshes);
	int i=0;
	for (;i<SubMeshes.Length(); ) 
	{
		cload.Open_Chunk();

		if (cload.Cur_Chunk_ID()==W3D_CHUNK_SHDMESH_USER_TEXT)
		{
			// todo
			cload.Read(buf,cload.Cur_Chunk_Length());
		}
		else
		{
			ShdSubMeshClass* ssmesh=NEW_REF(ShdSubMeshClass,());
			if (ssmesh==NULL) 
			{
				WWDEBUG_SAY(("ShdMeshClass::Load_W3D - Failed to allocate sub mesh\r\n"));
				return WW3D_ERROR_LOAD_FAILED;
			}

			SubMeshes[i].Mesh=ssmesh;
			SubMeshes[i].Renderer=NULL;
			REF_PTR_SET(SubMeshes[i].Mesh,SubMeshes[i].Mesh);

			ssmesh->Load_W3D(cload);
			
			// assign each sub-mesh with a name in the format: <parentmesh>.<index>
			StringClass tmp;
			tmp.Format("%s.%d",Name,i);
			ssmesh->Set_Name(tmp);
			
			i++;
		}

		cload.Close_Chunk();
	}

	// Pull interesting stuff out of the w3d attributes bits
	int col_bits = (hdr.Attributes & W3D_MESH_FLAG_COLLISION_TYPE_MASK) >> W3D_MESH_FLAG_COLLISION_TYPE_SHIFT;
	Set_Collision_Type( col_bits << 1 );
	Set_Hidden(hdr.Attributes & W3D_MESH_FLAG_HIDDEN);
	
	for (i=0;i<SubMeshes.Length(); i++) { 
		bool shadow = (hdr.Attributes & W3D_MESH_FLAG_CAST_SHADOW) == W3D_MESH_FLAG_CAST_SHADOW;
		SubMeshes[i].Mesh->Set_Flag(MeshGeometryClass::CAST_SHADOW,shadow);
	}
	
	// Indicate whether this mesh is translucent.  The mesh is considered translucent
	// if sorting has been enabled (alpha blending on pass 0) or if pass0 contains alpha-test.
	// This flag is used to determine if the mesh can cast a geometric shadow.
	bool is_translucent = false;
	for (i=0;i<SubMeshes.Length(); i++) { 
		if (SubMeshes[i].Mesh) {
			if (SubMeshes[i].Mesh->Is_Sorting()) {
				Set_Translucent(true);
			}
			ShdInterfaceClass * shader = SubMeshes[i].Mesh->Peek_Shader();
			if ((shader) && (!shader->Is_Opaque())) {
				Set_Translucent(true);
			}
		}
	}

	return WW3D_ERROR_OK;
}


int ShdMeshClass::Get_Sub_Mesh_Count(void) const
{
	return SubMeshes.Length();
}

ShdSubMeshClass * ShdMeshClass::Peek_Sub_Mesh(int i) const
{
	return SubMeshes[i].Mesh;
}

