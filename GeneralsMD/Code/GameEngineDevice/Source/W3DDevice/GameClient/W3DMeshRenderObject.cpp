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

#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshDrawing.h"
#include <assert.h>
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "WWDebug/wwdebug.h"
import Graphics.Materials.MeshMaterial;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Props.Material;
import Graphics.Materials.State;
import Graphics.Scene.Models.Hierarchy;
import Assets.Adapters.W3D.Geometry;
#include "WWMath/tri.h"
#include "WWMath/aaplane.h"
#include "WWLib/chunkio.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "W3DDevice/GameClient/W3DMeshGeometry.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include <WWDebug/wwprofile.h>





W3DMeshRenderObject::W3DMeshRenderObject() :
	Model(nullptr),
	LightEnvironment(nullptr),
	m_alphaOverride(1.0f),
	m_materialPassAlphaOverride(1.0f),
	m_materialPassEmissiveOverride(1.0f),
	m_muzzleFlashDesignation(Graphics::MuzzleFlashDesignation::None)
{
}


W3DMeshRenderObject::W3DMeshRenderObject(const W3DMeshRenderObject & that) :
	W3DRenderObject(that),
	Model(nullptr),
	LightEnvironment(nullptr),
	m_alphaOverride(1.0f),
	m_materialPassAlphaOverride(1.0f),
	m_materialPassEmissiveOverride(1.0f),
	m_muzzleFlashDesignation(that.m_muzzleFlashDesignation)
{
	REF_PTR_SET(Model,that.Model);					// mesh instances share models by default
}


W3DMeshRenderObject & W3DMeshRenderObject::operator = (const W3DMeshRenderObject & that)
{
	if (this != &that) {
		GraphicsInstance.Reset();
		GraphicsSkin.Reset();

		W3DRenderObject::operator = (that);

		REF_PTR_SET(Model,that.Model);				// mesh instances share models by default
		m_muzzleFlashDesignation = that.m_muzzleFlashDesignation;

		// just dont copy the light environment
		LightEnvironment = nullptr;
	}
	return * this;
}


W3DMeshRenderObject::~W3DMeshRenderObject()
{
	Free();
}


bool W3DMeshRenderObject::Contains(const Vector3 &point)
{
	// Transform point to object space and pass on to model
	Vector3 obj_point;
	Matrix3D::Inverse_Transform_Vector(Get_Transform_No_Validity_Check(), point, &obj_point);
	return Model->Contains(obj_point);
}


void W3DMeshRenderObject::Free()
{
	GraphicsInstance.Reset();
	GraphicsSkin.Reset();
	REF_PTR_RELEASE(Model);
}


W3DRenderObject * W3DMeshRenderObject::Clone() const
{
	return NEW_REF( W3DMeshRenderObject, (*this));
}


const char * W3DMeshRenderObject::Get_Name() const
{
	return Model->Get_Name();
}


void W3DMeshRenderObject::Set_Name(const char * name)
{
	Model->Set_Name(name);
}

uint32 W3DMeshRenderObject::Get_W3D_Flags()
{
	return Model->W3dAttributes;
}


const char * W3DMeshRenderObject::Get_User_Text() const
{
	return Model->Get_User_Text();
}


std::shared_ptr<Graphics::ModelMaterials<RefCountPtr<W3DTextureHandle>>> W3DMeshRenderObject::Get_Material_Info()
{
	if (Model) {
		if (Model->MatInfo) {
			return Model->MatInfo;
		}
	}
	return nullptr;
}


W3DMeshResource * W3DMeshRenderObject::Get_Model()
{
	if (Model != nullptr) {
		Model->Add_Ref();
	}
	return Model;
}

void W3DMeshRenderObject::Scale(float scale)
{
	if (scale==1.0f) return;

	Vector3 sc;
	sc.X = sc.Y = sc.Z = scale;
	Make_Unique();
	Model->Make_Geometry_Unique();
	Model->Scale(sc);

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   W3DRenderObject *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


void W3DMeshRenderObject::Scale(float scalex, float scaley, float scalez)
{
	// scale the surrender mesh model
	Vector3 sc;
	sc.X = scalex;
	sc.Y = scaley;
	sc.Z = scalez;
	Make_Unique();
	Model->Make_Geometry_Unique();
	Model->Scale(sc);

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   W3DRenderObject *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


void	W3DMeshRenderObject::Get_Deformed_Vertices(Vector3 *dst_vert, Vector3 *dst_norm)
{
	WWASSERT(Model->Get_Flag(W3DMeshGeometry::SKIN));
	Model->get_deformed_vertices(dst_vert,dst_norm,Container->Get_Model_Hierarchy());
}


void W3DMeshRenderObject::Get_Deformed_Vertices(Vector3 *dst_vert)
{
	WWASSERT(Model->Get_Flag(W3DMeshGeometry::SKIN));
	WWASSERT(Container != nullptr);
	WWASSERT(Container->Get_Model_Hierarchy() != nullptr);

	Model->get_deformed_vertices(dst_vert,Container->Get_Model_Hierarchy());
}

int W3DMeshRenderObject::Get_Num_Polys() const
{
	if (Model) {
		int num_passes=Model->Get_Pass_Count();
		WWASSERT(num_passes>0);
		int poly_count=Model->Get_Polygon_Count();
		return num_passes*poly_count;
	} else {
		return 0;
	}
}


void W3DMeshRenderObject::Render(W3DRenderContext & rinfo)
{
    WWPROFILE("Mesh::Render");
    if (!Is_Not_Hidden_At_All()) return;
    const unsigned sort_level=static_cast<unsigned>(Model->Get_Sort_Level());
    if (Graphics::Get_Scene_Draw_Queue().Is_Enabled()
        && sort_level != static_cast<unsigned>(Assets::W3D::W3DMeshSortLevelNone)) {
        Set_Lighting_Environment(rinfo.light_environment);
        m_alphaOverride=rinfo.alpha_override;
        m_materialPassAlphaOverride=rinfo.pass_alpha_override;
        m_materialPassEmissiveOverride=rinfo.pass_emissive_override;
        Graphics::Get_Scene_Draw_Queue().Enqueue<Extract_Ordered_Draw>(sort_level, *this);
        return;
    }
    if (!Model->Get_Flag(W3DMeshGeometry::SKIN)
        && CollisionMath::Overlap_Test(rinfo.Camera.Get_Frustum(),Get_Bounding_Box())==CollisionMath::OUTSIDE) return;
    if (sort_level == static_cast<unsigned>(Assets::W3D::W3DMeshSortLevelNone)) {
        Set_Lighting_Environment(rinfo.light_environment);
        m_alphaOverride=rinfo.alpha_override;
        m_materialPassAlphaOverride=rinfo.pass_alpha_override;
        m_materialPassEmissiveOverride=rinfo.pass_emissive_override;
    }
    [[maybe_unused]] const bool drawn=Draw_W3D_Mesh(*this,rinfo,
        {m_alphaOverride,m_materialPassAlphaOverride,m_materialPassEmissiveOverride});
    WWASSERT(drawn);

}


void W3DMeshRenderObject::Replace_Texture(W3DTextureHandle* texture,W3DTextureHandle* new_texture)
{
	Model->Replace_Texture(texture,new_texture);
}


void W3DMeshRenderObject::Replace_VertexMaterial(Graphics::MeshMaterial* vmat,const std::shared_ptr<Graphics::MeshMaterial>& new_vmat)
{
	Model->Replace_VertexMaterial(vmat,new_vmat);
}


void W3DMeshRenderObject::Make_Unique(bool force_meshmdl_clone)
{
	// Usually we will not clone the mesh model if it is already unique - force_meshmdl_clone will
	// force it to be cloned in any case. This is used in some special situations, for example if we
	// want to change this mesh and it may have already been rendered, we need to clone the mesh
	// model regardless of whether there is another mesh using it.
	if (Model->Num_Refs()==1 && !force_meshmdl_clone) return;

	W3DMeshResource *newmesh=NEW_REF(W3DMeshResource,(*Model));
	REF_PTR_SET(Model,newmesh);
	REF_PTR_RELEASE(newmesh);
}

bool W3DMeshRenderObject::Load_W3D(ChunkLoadClass & cload)
{
	Vector3 boxmin,boxmax;

	/*
	** Make sure this mesh is "empty"
	*/
	Free();

	/*
	** Create empty MaterialInfo and Model
	*/
	Model = NEW_REF(W3DMeshResource,());
	if (Model == nullptr) {
		WWDEBUG_SAY(("W3DMeshRenderObject::Load - Failed to allocate model"));
		return false;
	}

	/*
	** Create and read in the model...
	*/
	if (!Model->Load_W3D(cload)) {
		Free();
		return false;
	}

	/*
	** Pull interesting stuff out of the w3d attributes bits
	*/
	int col_bits = (Model->W3dAttributes & Assets::W3D::W3DMeshAttributeCollisionTypeMask)
        >> Assets::W3D::W3DMeshAttributeCollisionTypeShift;
	Set_Collision_Type( col_bits << 1 );
	Set_Hidden(Model->W3dAttributes & Assets::W3D::W3DMeshAttributeHidden);

	/*
	** Indicate whether this mesh is translucent.  The mesh is considered translucent
	** if sorting has been enabled (alpha blending on pass 0) or if pass0 contains alpha-test.
	** This flag is mainly being used by visibility preprocessing code in Renegade.
	*/
	int is_translucent = Model->Get_Flag(W3DMeshResource::SORT);
	int is_alpha = 0;	//keep track of alpha pieces that require sorting (including static sort lists).
	int is_additive = 0;

	if (Model->Has_Shader_Array(0)) {
		for (int i=0; i<Model->Get_Polygon_Count(); i++) {
			Graphics::MaterialState shader = Model->Get_Shader(i,0);
			is_translucent |= (shader.Get_Alpha_Test() == Graphics::MaterialState::ALPHATEST_ENABLE);
			is_alpha |= (shader.Get_Dst_Blend_Func() != Graphics::MaterialState::DSTBLEND_ZERO ||
									shader.Get_Src_Blend_Func() != Graphics::MaterialState::SRCBLEND_ONE) && (shader.Get_Alpha_Test() != Graphics::MaterialState::ALPHATEST_ENABLE);
			is_additive |= (shader.Get_Dst_Blend_Func() == Graphics::MaterialState::DSTBLEND_ONE &&
									shader.Get_Src_Blend_Func() == Graphics::MaterialState::SRCBLEND_ONE);
		}
	} else {
		Graphics::MaterialState shader = Model->Get_Single_Shader(0);
		is_translucent |= (shader.Get_Alpha_Test() == Graphics::MaterialState::ALPHATEST_ENABLE);
		is_alpha |= (shader.Get_Dst_Blend_Func() != Graphics::MaterialState::DSTBLEND_ZERO ||
									shader.Get_Src_Blend_Func() != Graphics::MaterialState::SRCBLEND_ONE) && (shader.Get_Alpha_Test() != Graphics::MaterialState::ALPHATEST_ENABLE);
		is_additive |= (shader.Get_Dst_Blend_Func() == Graphics::MaterialState::DSTBLEND_ONE &&
									shader.Get_Src_Blend_Func() == Graphics::MaterialState::SRCBLEND_ONE);
	}
	Set_Translucent(is_translucent);
	Set_Alpha(is_alpha);
	Set_Additive(is_additive);

	return true;

}


bool W3DMeshRenderObject::Cast_Ray(W3DRayCastQuery & raytest)
{
	if ((Get_Collision_Type() & raytest.CollisionType) == 0) return false;
	//Modified for 'Generals' so we could select trees but filter out headlight beams, etc. -MW
	if (raytest.CheckTranslucent && Is_Alpha()!=0)
		return false;
	if (Is_Hidden() && !raytest.CheckHidden) return false;
	if (Is_Animation_Hidden()) return false;
	if (raytest.Result->StartBad) return false;

	Matrix3D world_to_obj;
	Matrix3D world=Get_Transform();

	// if aligned or oriented rotate the mesh so that it's aligned to the ray
	if (Model->Get_Flag(W3DMeshResource::ALIGNED)) {
			Vector3 mesh_position;
			world.Get_Translation(&mesh_position);
			world.Obj_Look_At(mesh_position,mesh_position - raytest.Ray.Get_Dir(),0.0f);
	} else if (Model->Get_Flag(W3DMeshResource::ORIENTED)) {
			Vector3 mesh_position;
			world.Get_Translation(&mesh_position);
			world.Obj_Look_At(mesh_position,raytest.Ray.Get_P0(),0.0f);
	}

	world.Get_Inverse(world_to_obj);
	W3DRayCastQuery objray(raytest,world_to_obj);

	WWASSERT(Model);

	bool hit = Model->Cast_Ray(objray);

	// transform result back into original coordinate system
	if (hit) {
		raytest.CollidedRenderObj = this;
		Matrix3D::Rotate_Vector(world,raytest.Result->Normal, &(raytest.Result->Normal));
		if (raytest.Result->ComputeContactPoint) {
			Matrix3D::Transform_Vector(world,raytest.Result->ContactPoint, &(raytest.Result->ContactPoint));
		}
	}

	return hit;
}


bool W3DMeshRenderObject::Cast_AABox(W3DBoxCastQuery & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	WWASSERT(Model);

	// This function analyses the transform to call optimized functions in certain cases
	bool hit = Model->Cast_World_Space_AABox(boxtest, Get_Transform());

	if (hit) {
		boxtest.CollidedRenderObj = this;
	}

	return hit;
}


bool W3DMeshRenderObject::Cast_OBBox(W3DOrientedBoxCastQuery & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;
	if (boxtest.Result->StartBad) return false;

	/*
	** transform into the local coordinate system of the mesh.
	*/
	const Matrix3D & tm = Get_Transform();
	Matrix3D world_to_obj;
	tm.Get_Orthogonal_Inverse(world_to_obj);
	W3DOrientedBoxCastQuery localtest(boxtest,world_to_obj);

	WWASSERT(Model);

	bool hit = Model->Cast_OBBox(localtest);

	/*
	** If we hit, transform the result of the test back to the original coordinate system.
	*/
	if (hit) {
		boxtest.CollidedRenderObj = this;
		Matrix3D::Rotate_Vector(tm,boxtest.Result->Normal, &(boxtest.Result->Normal));
		if (boxtest.Result->ComputeContactPoint) {
			Matrix3D::Transform_Vector(tm,boxtest.Result->ContactPoint, &(boxtest.Result->ContactPoint));
		}
	}

	return hit;
}


bool W3DMeshRenderObject::Intersect_AABox(W3DBoxIntersectionQuery & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;

	Matrix3D inv_tm;
	Get_Transform().Get_Orthogonal_Inverse(inv_tm);
	W3DOrientedBoxIntersectionQuery local_test(boxtest,inv_tm);
	WWASSERT(Model);
	return Model->Intersect_OBBox(local_test);
}


bool W3DMeshRenderObject::Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest)
{
	if ((Get_Collision_Type() & boxtest.CollisionType) == 0) return false;

	Matrix3D inv_tm;
	Get_Transform().Get_Orthogonal_Inverse(inv_tm);
	W3DOrientedBoxIntersectionQuery local_test(boxtest,inv_tm);
	WWASSERT(Model);
	return Model->Intersect_OBBox(local_test);
}


void W3DMeshRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	if (Model) {
		Model->Get_Bounding_Sphere(&sphere);
	} else {
		sphere.Center.Set(0,0,0);
		sphere.Radius = 1.0f;
	}
}


void W3DMeshRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	if (Model) {
		Model->Get_Bounding_Box(&box);
	} else {
		box.Init(Vector3(0,0,0),Vector3(1,1,1));
	}
}


void W3DMeshRenderObject::Generate_Culling_Tree()
{
	Model->Generate_Culling_Tree();
}


void W3DMeshRenderObject::Add_Dependencies_To_List
(
	DynamicVectorClass<StringClass> &file_list,
	bool										textures_only
)
{
	//
	// Get a pointer to this mesh's material information object
	//
	auto material = Get_Material_Info ();
	if (material != nullptr) {

		//
		// Loop through all the textures and add their filenames to our list
		//
		for (int index = 0; index < static_cast<int>(material->textures.size()); index ++) {

			//
			//	Add this texture's filename to the list
			//
			W3DTextureHandle *texture = material->textures[index].Peek();
			if (texture != nullptr) {
				file_list.Add (texture->Get_Full_Path ());
			}
		}

		//
		// Release our hold on the material information object
		//
		material.reset();
	}

	W3DRenderObject::Add_Dependencies_To_List (file_list, textures_only);
}


void W3DMeshRenderObject::Update_Cached_Bounding_Volumes() const
{
	Get_Obj_Space_Bounding_Sphere(CachedBoundingSphere);

#ifdef ALLOW_TEMPORARIES
	CachedBoundingSphere.Center = Get_Transform() * CachedBoundingSphere.Center;
#else
	Get_Transform().mulVector3(CachedBoundingSphere.Center);
#endif

	// If we are camera-aligned or -oriented, we don't know which way we are facing at this point,
	// so the box we return needs to contain the sphere. Otherwise do the normal computation.
	if (Model->Get_Flag(W3DMeshResource::ALIGNED) || Model->Get_Flag(W3DMeshResource::ORIENTED)) {
		CachedBoundingBox.Center = CachedBoundingSphere.Center;
		CachedBoundingBox.Extent.Set(CachedBoundingSphere.Radius, CachedBoundingSphere.Radius, CachedBoundingSphere.Radius);
	} else {
		Get_Obj_Space_Bounding_Box(CachedBoundingBox);
		CachedBoundingBox.Transform(Get_Transform());
	}

	Validate_Cached_Bounding_Volumes();
}


// This utility function recurses throughout the subobjects of a renderobject, and for each
// W3DMeshRenderObject it finds it sets the given MeshModel flag on its model. This is useful for stuff
// like making a RenderObjects' polys sort.
void Set_MeshModel_Flag(W3DRenderObject *robj, int flag, int onoff)
{
	if (robj->Class_ID() == W3DRenderObject::CLASSID_MESH) {
		// Set flag on model (the assumption is that meshes don't have subobjects)
		W3DMeshRenderObject *mesh = (W3DMeshRenderObject *)robj;
		W3DMeshResource *model = mesh->Get_Model();
		model->Set_Flag((W3DMeshResource::FlagsType)flag, onoff != 0);
		model->Release_Ref();
	} else {
		// Recurse to subobjects (if any)
		int num_obj = robj->Get_Num_Sub_Objects();
		W3DRenderObject *sub_obj;
		for (int i = 0; i < num_obj; i++) {
			sub_obj = robj->Get_Sub_Object(i);
			if (sub_obj) {
				Set_MeshModel_Flag(sub_obj, flag, onoff);
				sub_obj->Release_Ref();
			}
		}
	}
}

int W3DMeshRenderObject::Get_Sort_Level() const
{
	if (Model) {
		return (Model->Get_Sort_Level());
	}
	return Assets::W3D::W3DMeshSortLevelNone;
}

void W3DMeshRenderObject::Set_Sort_Level(int level)
{
	if (Model) {
		Model->Set_Sort_Level(level);
	}
}

int W3DMeshRenderObject::Get_Draw_Call_Count() const
{
	if (Model != nullptr) {
		// Report the material texture count for the model.
		if (Model->MatInfo && !Model->MatInfo->textures.empty()) {
			return static_cast<int>(Model->MatInfo->textures.size());
		}

		// Otherwise, return 1
		return 1;

	} else {
		return 0;
	}
}

Graphics::ModelFactory<W3DRenderObject>* Load_Mesh_Factory(ChunkLoadClass& cload)
{
    W3DMeshRenderObject* mesh=NEW_REF(W3DMeshRenderObject,());
    if(!mesh)return nullptr;
    const std::shared_ptr<W3DRenderObject> source(mesh,[](W3DRenderObject* object) { object->Release_Ref(); });
    if(!mesh->Load_W3D(cload))return nullptr;
    return new Graphics::ModelFactory<W3DRenderObject>(mesh->Get_Name(),mesh->Class_ID(),[source] {
        return static_cast<W3DRenderObject*>(SET_REF_OWNER(source->Clone()));
    });
}
