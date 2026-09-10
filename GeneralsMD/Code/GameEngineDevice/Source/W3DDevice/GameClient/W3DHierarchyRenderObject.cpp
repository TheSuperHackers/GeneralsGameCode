#include "W3DDevice/GameClient/W3DMeshDrawing.h"
#include <filesystem>
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

#include "W3DDevice/GameClient/W3DHierarchyRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "WWLib/chunkio.h"
#include <limits>
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "WWMath/sphere.h"
#include <cctype>
#include <cstdlib>
#include <cstring>
import Assets.Adapters.W3D.Assembly;
import Assets.Adapters.W3D.LevelSet;
import Assets.Adapters.W3D.Aggregate;
import Assets.Cache.Animations;

namespace
{
	bool equal_case_insensitive(const char *left, const char *right)
	{
		if (left == nullptr || right == nullptr)
			return left == right;

		while (*left != '\0' && *right != '\0')
		{
			const unsigned char left_character = static_cast<unsigned char>(*left);
			const unsigned char right_character = static_cast<unsigned char>(*right);
			if (std::tolower(left_character) != std::tolower(right_character))
				return false;
			++left;
			++right;
		}

		return *left == *right;
	}
}

Graphics::ModelFactory<W3DRenderObject> *Load_HLod_Factory(ChunkLoadClass& cload)
{
    std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
    if(cload.Read(bytes.data(),static_cast<unsigned>(bytes.size()))!=bytes.size())return nullptr;
    Assets::ModelAssemblyDesc description;std::string error;
    if(!Assets::W3D::W3DRead_Model_Assembly(bytes,true,description,error))return nullptr;
    auto data=std::make_shared<const Assets::ModelAssemblyDesc>(std::move(description));
    return new Graphics::ModelFactory<W3DRenderObject>(data->name,W3DRenderObject::CLASSID_HLOD,[data] { return NEW_REF(W3DHierarchyRenderObject,(*data)); });
}

W3DHierarchyRenderObject::W3DHierarchyRenderObject() :
	W3DAnimatedModelRenderObject(nullptr),
	BoundingBoxIndex(-1),
	SnapPoints(),
	Proxies()
{
}

W3DHierarchyRenderObject::W3DHierarchyRenderObject(const W3DHierarchyRenderObject & src) :
	W3DAnimatedModelRenderObject(src),
	BoundingBoxIndex(-1),
	SnapPoints(),
	Proxies()
{
	*this = src;
}

W3DHierarchyRenderObject::W3DHierarchyRenderObject(const char * name,W3DRenderObject ** lods,int count) :
	W3DAnimatedModelRenderObject(nullptr),
	BoundingBoxIndex(-1),
	SnapPoints(),
	Proxies()
{
	// enforce parameters
	WWASSERT(name != nullptr);
	WWASSERT(lods != nullptr);
	WWASSERT(count > 0);

	// Set the name
	Set_Name(name);

	m_children.Initialize(count);
	m_detail_levels.Initialize(count);

	// Create our Hierarchy from the highest LOD if it is an HModel
	// Otherwise, create a single node tree
	const Graphics::ModelHierarchy * tree = lods[count-1]->Get_Model_Hierarchy();
	if (tree != nullptr) {
		Hierarchy.reset(W3DNEW Graphics::ModelHierarchy(*tree));
	} else {
		Hierarchy.reset(W3DNEW Graphics::ModelHierarchy());
		Hierarchy->Initialize_Default();
	}

	// Ok, now suck the sub-objects out of each LOD model and place them into this HLOD.
	for (int lod_index=0; lod_index < m_children.Level_Count(); lod_index++) {
		W3DRenderObject * lod_obj = lods[lod_index];
		WWASSERT(lod_obj);

		if (	(lod_obj->Class_ID() == W3DRenderObject::CLASSID_HMODEL) ||
				(lod_obj->Class_ID() == W3DRenderObject::CLASSID_HLOD) ||
				(lod_obj->Get_Num_Sub_Objects() > 1) ) {

			// here we insert all sub-objects of this render object into the current LOD array
			while (lod_obj->Get_Num_Sub_Objects() > 0) {

				W3DRenderObject * sub_obj = lod_obj->Get_Sub_Object(0);
				int boneindex = lod_obj->Get_Sub_Object_Bone_Index(sub_obj);
				lod_obj->Remove_Sub_Object(sub_obj);

				Add_Lod_Model(lod_index,sub_obj,boneindex);

				sub_obj->Release_Ref();
			}

		} else {

			// just insert the render object as the sole member of the current LOD array.  This
			// case happens if this level of detail is a simple object such as a mesh or NullRenderObj
			Add_Lod_Model(lod_index,lod_obj,0);
		}
	}

	Recalculate_Static_LOD_Factors();

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its cost and value arrays
	// according to a screen area of 1 pixel.
	int minlod = m_detail_levels.Update(1.0f);

	// Ensure lod is no less than minimum allowed
	if (m_children.Current_Level() < minlod) Set_LOD_Level(minlod);

	// Flag our sub-objects as having dirty transforms
	Set_Sub_Object_Transforms_Dirty(true);

	// Normal render object processing whenever sub-objects are added or removed:
	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
}

W3DHierarchyRenderObject::W3DHierarchyRenderObject(const Assets::ModelAssemblyDesc & def) :
	W3DAnimatedModelRenderObject(def.skeleton_name.c_str()),
	BoundingBoxIndex(-1),
	SnapPoints(),
	Proxies()
{
	// Set the name
	Set_Name(def.name.c_str());

	// Number of LODs comes from the distlod
	m_children.Initialize(static_cast<int>(def.levels.size()));
	m_detail_levels.Initialize(static_cast<int>(def.levels.size()));

	// Add Models to the ModelArrays
	for (int ilod=0; ilod < static_cast<int>(def.levels.size()); ilod++) {

		m_detail_levels.Set_Maximum_Area(ilod, def.levels[ilod].maximum_screen_size);

		for (int imodel=0; imodel < static_cast<int>(def.levels[ilod].children.size()); imodel++) {

			W3DRenderObject * robj = W3DAssetCatalog::Get_Instance()->Create_Render_Obj(def.levels[ilod].children[imodel].object_name.c_str());
			int boneindex = def.levels[ilod].children[imodel].bone;
			if (robj != nullptr) {
				Add_Lod_Model(ilod,robj,boneindex);
				robj->Release_Ref();
			}
		}
	}

	Recalculate_Static_LOD_Factors();

	// Add aggregates to this model
	for (int iagg=0; iagg<static_cast<int>(def.aggregates.size()); iagg++) {
		W3DRenderObject * robj = W3DAssetCatalog::Get_Instance()->Create_Render_Obj(def.aggregates[iagg].object_name.c_str());
		int boneindex = def.aggregates[iagg].bone;
		if (robj != nullptr) {
			Add_Sub_Object_To_Bone(robj,boneindex);
			robj->Release_Ref();
		}
	}

	// Retain instance data independently of the prototype description.
	Proxies=def.proxies;
	SnapPoints=def.snap_points;

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its cost and value arrays
	// according to a screen area of 1 pixel.
	int minlod = m_detail_levels.Update(1.0f);

	// Ensure lod is no less than minimum allowed
	if (m_children.Current_Level() < minlod) Set_LOD_Level(minlod);

	// Flag our sub-objects as having dirty transforms
	Set_Sub_Object_Transforms_Dirty(true);

	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
}

W3DHierarchyRenderObject & W3DHierarchyRenderObject::operator = (const W3DHierarchyRenderObject & that)
{
    if (this != &that) {
        const int previous_level = m_children.Current_Level();
        Free();
        W3DAnimatedModelRenderObject::operator=(that);
        BoundingBoxIndex = that.BoundingBoxIndex;
        m_detail_levels = that.m_detail_levels;
        m_children.Clone_From(that.m_children, [&](const ChildOwner& source) {
            auto child = ChildOwner::Create_No_Add_Ref(source->Clone());
            child->Set_Container(this);
            if (Is_In_Scene()) child->Notify_Added(Scene);
            return child;
        });
        m_children.Select_Level(previous_level, [](const ChildAttachment&) {}, [](const ChildAttachment&) {});
        Proxies = that.Proxies;
        SnapPoints = that.SnapPoints;
    }
    Recalculate_Static_LOD_Factors();
    const int minimum = m_detail_levels.Update(1.0f);
    if (Get_LOD_Level() < minimum) Set_LOD_Level(minimum);
    Set_Sub_Object_Transforms_Dirty(true);
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
    return *this;
}

W3DHierarchyRenderObject::~W3DHierarchyRenderObject()
{
	Free();
}

void W3DHierarchyRenderObject::Free()
{
    m_children.Clear([](const ChildOwner& child) { child->Set_Container(nullptr); });
    m_detail_levels.Initialize(0);
    SnapPoints.clear();
    Proxies.clear();
}

W3DRenderObject * W3DHierarchyRenderObject::Clone() const
{
	return W3DNEW W3DHierarchyRenderObject(*this);
}

void W3DHierarchyRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	//
	//	Do we have a bounding box mesh?
	//
	int count = static_cast<int>(m_children.Level(m_children.Level_Count() - 1).size());
	if (BoundingBoxIndex >= 0 && BoundingBoxIndex < count) {

		W3DRenderObject *mesh = m_children.Level(m_children.Level_Count() - 1)[BoundingBoxIndex].model.Peek();
		AABoxClass box_query;
		if (mesh != nullptr && mesh->Class_ID() == W3DRenderObject::CLASSID_OBBOX) {
			mesh->Get_Obj_Space_Bounding_Box(box_query);

			//
			//	Determine what the box's transform 'should' be this frame.
			// Note:  We do this because some animation types don't update
			// unless they are visible.
			//
			Matrix3D box_tm;
			Simple_Evaluate_Bone (m_children.Level(m_children.Level_Count() - 1)[BoundingBoxIndex].bone, &box_tm);

			//
			//	Convert the OBBox from its coordinate system to the coordinate
			// system of the HLOD.
			//
			Matrix3D world_to_hlod_tm;
			Matrix3D box_to_hlod_tm;

			Get_Transform ().Get_Orthogonal_Inverse (world_to_hlod_tm);
			Matrix3D::Multiply(world_to_hlod_tm,box_tm,&box_to_hlod_tm);

			box_to_hlod_tm.Transform_Center_Extent_AABox(	box_query.Center,
																																																		box_query.Extent,
																																																		&box.Center,&box.Extent);
		}

	} else {
		W3DAnimatedModelRenderObject::Get_Obj_Space_Bounding_Box (box);
	}
}

void W3DHierarchyRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	AABoxClass box;
	Get_Obj_Space_Bounding_Box(box);
	sphere.Center = box.Center;
	sphere.Radius = box.Extent.Length();
}

const SphereClass &W3DHierarchyRenderObject::Get_Bounding_Sphere() const
{
	if (BoundingBoxIndex >= 0) {
		//
		//	Get the bounding sphere in local coordinates
		//
		SphereClass sphere;
		Get_Obj_Space_Bounding_Sphere (sphere);

		//
		//	Transform the sphere into world coords and return the sphere
		//
#ifdef ALLOW_TEMPORARIES
		CachedBoundingSphere.Center = Get_Transform () * sphere.Center;
#else
		Get_Transform().mulVector3(sphere.Center, CachedBoundingSphere.Center);
#endif
		CachedBoundingSphere.Radius = sphere.Radius;
	} else {
		W3DAnimatedModelRenderObject::Get_Bounding_Sphere ();
	}

	return CachedBoundingSphere;
}

const AABoxClass &W3DHierarchyRenderObject::Get_Bounding_Box() const
{
	if (BoundingBoxIndex >= 0) {

		//
		//	Get the bounding box in local coordinates
		//
		AABoxClass box;
		Get_Obj_Space_Bounding_Box (box);

		//
		//	Transform the bounding box to world coordinates
		//
		Get_Transform().Transform_Center_Extent_AABox(	box.Center,
																		box.Extent,
																		&CachedBoundingBox.Center,
																		&CachedBoundingBox.Extent	);
	} else {
		W3DAnimatedModelRenderObject::Get_Bounding_Box ();
	}

	return CachedBoundingBox;
}

void W3DHierarchyRenderObject::Set_Max_Screen_Size(int lod_index, float size)
{
    WWASSERT(lod_index >= 0 && lod_index < m_children.Level_Count());
    if (lod_index < 0 || lod_index >= m_children.Level_Count()) return;
    m_detail_levels.Set_Maximum_Area(lod_index, size);
    Recalculate_Static_LOD_Factors();
    const int minimum = m_detail_levels.Update(1.0f);
    if (Get_LOD_Level() < minimum) Set_LOD_Level(minimum);
}

float W3DHierarchyRenderObject::Get_Max_Screen_Size(int lod_index) const
{
    WWASSERT(lod_index >= 0 && lod_index < m_children.Level_Count());
    return lod_index >= 0 && lod_index < m_children.Level_Count()
        ? m_detail_levels.Maximum_Area(lod_index) : (std::numeric_limits<float>::max)();
}

int W3DHierarchyRenderObject::Get_Lod_Count() const
{
	return m_children.Level_Count();
}

void W3DHierarchyRenderObject::Set_LOD_Bias(float bias)
{
    assert(bias > 0.0f);
    bias = MAX(bias, 0.0f);
    m_detail_levels.Set_Bias(bias);
    for (const auto& child : m_children.Additional()) child.model->Set_LOD_Bias(bias);
}

int W3DHierarchyRenderObject::Get_Lod_Model_Count(int lod_index) const
{
	int count = 0;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < m_children.Level_Count());
	if ((lod_index >= 0) && (lod_index < m_children.Level_Count())) {

		// Get the number of models in this Lod
		count = static_cast<int>(m_children.Level(lod_index).size());
	}

	// Return the number of models that compose this Lod
	return count;
}

W3DRenderObject *W3DHierarchyRenderObject::Peek_Lod_Model(int lod_index, int model_index) const
{
	W3DRenderObject *pmodel = nullptr;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < m_children.Level_Count());
	if ((lod_index >= 0) &&
		 (lod_index < m_children.Level_Count()) &&
		 (model_index < static_cast<int>(m_children.Level(lod_index).size()))) {

		// Get a pointer to the requested model
		pmodel = m_children.Level(lod_index)[model_index].model.Peek();
	}

	// Return a pointer to the requested model
	return pmodel;
}

W3DRenderObject *W3DHierarchyRenderObject::Get_Lod_Model(int lod_index, int model_index) const
{
	W3DRenderObject *pmodel = nullptr;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < m_children.Level_Count());
	if ((lod_index >= 0) &&
		 (lod_index < m_children.Level_Count()) &&
		 (model_index < static_cast<int>(m_children.Level(lod_index).size()))) {

		// Get a pointer to the requested model
		pmodel = m_children.Level(lod_index)[model_index].model.Peek();
		if (pmodel != nullptr) {
			pmodel->Add_Ref ();
		}
	}

	// Return the number of models that compose this Lod
	return pmodel;
}

int W3DHierarchyRenderObject::Get_Lod_Model_Bone(int lod_index, int model_index) const
{
	int bone_index = 0;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < m_children.Level_Count());
	if ((lod_index >= 0) &&
		 (lod_index < m_children.Level_Count()) &&
		 (model_index < static_cast<int>(m_children.Level(lod_index).size()))) {

		// Get the bone that this model resides on
		bone_index = m_children.Level(lod_index)[model_index].bone;
	}

	// Return the bone that this model resides on
	return bone_index;
}

int W3DHierarchyRenderObject::Get_Additional_Model_Count() const
{
	return static_cast<int>(m_children.Additional().size());
}

W3DRenderObject * W3DHierarchyRenderObject::Peek_Additional_Model (int model_index) const
{
	W3DRenderObject *pmodel = nullptr;

	// Param valid?
	WWASSERT(model_index >= 0);
	WWASSERT(model_index < static_cast<int>(m_children.Additional().size()));
	if ((model_index >= 0) &&
		 (model_index < static_cast<int>(m_children.Additional().size()))) {

		// Get a pointer to the requested model
		pmodel = m_children.Additional()[model_index].model.Peek();
	}

	// Return a pointer to the requested model
	return pmodel;
}

W3DRenderObject * W3DHierarchyRenderObject::Get_Additional_Model (int model_index) const
{
	W3DRenderObject *pmodel = nullptr;

	// Param valid?
	WWASSERT(model_index >= 0);
	WWASSERT(model_index < static_cast<int>(m_children.Additional().size()));
	if ((model_index >= 0) &&
		 (model_index < static_cast<int>(m_children.Additional().size()))) {

		// Get a pointer to the requested model
		pmodel = m_children.Additional()[model_index].model.Peek();
		if (pmodel != nullptr) {
			pmodel->Add_Ref ();
		}
	}

	// Return a pointer to the requested model
	return pmodel;
}

int W3DHierarchyRenderObject::Get_Additional_Model_Bone (int model_index) const
{
	int bone_index = 0;

	// Params valid?
	WWASSERT(model_index >= 0);
	WWASSERT(model_index < static_cast<int>(m_children.Additional().size()));
	if ((model_index >= 0) &&
		 (model_index < static_cast<int>(m_children.Additional().size()))) {

		// Get the bone that this model resides on
		bone_index = m_children.Additional()[model_index].bone;
	}

	// Return the bone that this model resides on
	return bone_index;
}

int W3DHierarchyRenderObject::Get_Num_Polys() const
{
    int count = 0;
    m_children.Visit_Level(m_children.Current_Level(), [&](const ChildAttachment& child, int) {
        if (child.model->Is_Not_Hidden_At_All()) count += child.model->Get_Num_Polys();
    });
    return count;
}

void W3DHierarchyRenderObject::Render(W3DRenderContext & rinfo)
{
    if (!Is_Not_Hidden_At_All()) return;
    W3DAnimatedModelRenderObject::Render(rinfo);
    m_children.Visit_Level(m_children.Current_Level(), [&](const ChildAttachment& child, int level) {
        if (level >= 0 && child.model->Class_ID() == W3DRenderObject::CLASSID_OBBOX) return;
        if (level < 0 && Is_Sub_Objects_Match_LOD_Enabled()) child.model->Set_LOD_Level(Get_LOD_Level());
        Flush_Before_W3D_Object_Draw(*child.model); child.model->Render(rinfo);
    });
}

void W3DHierarchyRenderObject::Set_Transform(const Matrix3D &m)
{
	W3DAnimatedModelRenderObject::Set_Transform(m);
	Set_Sub_Object_Transforms_Dirty(true);
}

void W3DHierarchyRenderObject::Set_Position(const Vector3 &v)
{
	W3DAnimatedModelRenderObject::Set_Position(v);
	Set_Sub_Object_Transforms_Dirty(true);
}

void W3DHierarchyRenderObject::Notify_Added(W3DScene * scene)
{
    W3DRenderObject::Notify_Added(scene);
    m_children.Visit_Level(m_children.Current_Level(), [&](const ChildAttachment& child, int) {
        child.model->Notify_Added(scene);
    });
}

void W3DHierarchyRenderObject::Notify_Removed(W3DScene * scene)
{
    m_children.Visit_Level(m_children.Current_Level(), [&](const ChildAttachment& child, int) {
        child.model->Notify_Removed(scene);
    });
    W3DRenderObject::Notify_Removed(scene);
}

int W3DHierarchyRenderObject::Get_Num_Sub_Objects() const
{
    return static_cast<int>(m_children.Count());
}

W3DRenderObject * W3DHierarchyRenderObject::Get_Sub_Object(int index) const
{
    const auto* child = index >= 0 ? m_children.At(index) : nullptr;
    WWASSERT(child);
    if (!child) return nullptr;
    auto retained = child->model;
    return retained.Release();
}

int W3DHierarchyRenderObject::Add_Sub_Object(W3DRenderObject * subobj)
{
	return Add_Sub_Object_To_Bone(subobj,0);
}

int W3DHierarchyRenderObject::Remove_Sub_Object(W3DRenderObject * removeme)
{
    if (!removeme) return 0;
    auto removed = m_children.Extract_First([&](const ChildAttachment& child) { return child.model.Peek() == removeme; });
    if (!removed) return 0;
    removeme->Set_Container(nullptr);
    if ((removed->level < 0 || removed->level == m_children.Current_Level()) && Is_In_Scene())
        removeme->Notify_Removed(Scene);
    removed.reset();
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
    return 1;
}

int W3DHierarchyRenderObject::Get_Num_Sub_Objects_On_Bone(int boneindex) const
{
    return static_cast<int>(m_children.Count_On_Bone(boneindex));
}

W3DRenderObject * W3DHierarchyRenderObject::Get_Sub_Object_On_Bone(int index,int boneindex) const
{
    const auto* child = index >= 0 ? m_children.On_Bone(index, boneindex) : nullptr;
    if (!child) return nullptr;
    auto retained = child->model;
    return retained.Release();
}

int W3DHierarchyRenderObject::Get_Sub_Object_Bone_Index(W3DRenderObject * subobj) const
{
    const auto* child = m_children.Find([&](const ChildAttachment& entry) { return entry.model.Peek() == subobj; });
    return child ? child->bone : 0;
}

//Custom version of above function for cases where we know the lod/model index. -MW
int W3DHierarchyRenderObject::Get_Sub_Object_Bone_Index(int LodIndex, int ModelIndex)	const
{
	return m_children.Level(LodIndex)[ModelIndex].bone;
}

int W3DHierarchyRenderObject::Add_Sub_Object_To_Bone(W3DRenderObject * subobj,int boneindex)
{
    WWASSERT(subobj);
    if (boneindex < 0 || boneindex >= Hierarchy->Bone_Count()) return 0;
    subobj->Set_LOD_Bias(m_detail_levels.Bias());
    auto owner = ChildOwner::Create_Add_Ref(subobj);
    subobj->Set_Container(this);
    subobj->Set_Animation_Hidden(!Hierarchy->Visible(boneindex));
    m_children.Add_Additional(std::move(owner), boneindex);
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
    Set_Hierarchy_Valid(false);
    Set_Sub_Object_Transforms_Dirty(true);
    if (Is_In_Scene()) subobj->Notify_Added(Scene);
    return 1;
}

void W3DHierarchyRenderObject::Set_Animation()
{
	W3DAnimatedModelRenderObject::Set_Animation();
	Set_Sub_Object_Transforms_Dirty(true);
}

void W3DHierarchyRenderObject::Set_Animation(Assets::AnimationAssetHandle motion,float frame,int mode)
{
	W3DAnimatedModelRenderObject::Set_Animation(motion,frame,mode);
	Set_Sub_Object_Transforms_Dirty(true);
}

void W3DHierarchyRenderObject::Set_Animation
(
	Assets::AnimationAssetHandle motion0,
	float frame0,
	Assets::AnimationAssetHandle motion1,
	float frame1,
	float percentage
)
{
	W3DAnimatedModelRenderObject::Set_Animation(motion0,frame0,motion1,frame1,percentage);
	Set_Sub_Object_Transforms_Dirty(true);
}

bool W3DHierarchyRenderObject::Cast_Ray(W3DRayCastQuery & raytest)
{
    if (Are_Sub_Object_Transforms_Dirty()) Update_Sub_Object_Transforms();
    return m_children.Query_Level(m_children.Level_Count() - 1, [&](const ChildAttachment& child) {
        return child.model->Cast_Ray(raytest);
    });
}

bool W3DHierarchyRenderObject::Cast_AABox(W3DBoxCastQuery & boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) Update_Sub_Object_Transforms();
    return m_children.Query_Level(m_children.Level_Count() - 1, [&](const ChildAttachment& child) {
        return child.model->Cast_AABox(boxtest);
    });
}

bool W3DHierarchyRenderObject::Cast_OBBox(W3DOrientedBoxCastQuery & boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) Update_Sub_Object_Transforms();
    return m_children.Query_Level(m_children.Level_Count() - 1, [&](const ChildAttachment& child) {
        return child.model->Cast_OBBox(boxtest);
    });
}

bool W3DHierarchyRenderObject::Intersect_AABox(W3DBoxIntersectionQuery & boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) Update_Sub_Object_Transforms();
    return m_children.Query_Level(m_children.Level_Count() - 1, [&](const ChildAttachment& child) {
        return child.model->Intersect_AABox(boxtest);
    });
}

bool W3DHierarchyRenderObject::Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest)
{
    if (Are_Sub_Object_Transforms_Dirty()) Update_Sub_Object_Transforms();
    return m_children.Query_Level(m_children.Level_Count() - 1, [&](const ChildAttachment& child) {
        return child.model->Intersect_OBBox(boxtest);
    });
}

void W3DHierarchyRenderObject::Prepare_LOD(W3DCamera &camera)
{
    if (!Is_Not_Hidden_At_All()) return;
    const float area = Get_Screen_Size(camera);
    if (m_children.Level_Count() > 1) {
        const int minimum = m_detail_levels.Update(area);
        if (Get_LOD_Level() < minimum) Set_LOD_Level(minimum);
    }
    for (const auto& child : m_children.Additional())
        if (child.model->Is_Not_Hidden_At_All()) child.model->Prepare_LOD(camera);
}

void W3DHierarchyRenderObject::Recalculate_Static_LOD_Factors()
{
    for (int level = 0; level < m_children.Level_Count(); ++level) {
        int polygons = 0;
        for (const auto& child : m_children.Level(level))
            if (child.model->Is_Not_Hidden_At_All()) polygons += child.model->Get_Num_Polys();
        m_detail_levels.Set_Polygon_Count(level, polygons);
    }
}

void W3DHierarchyRenderObject::Increment_LOD()
{
    Set_LOD_Level(m_children.Current_Level() + 1);
}

void W3DHierarchyRenderObject::Decrement_LOD()
{
    Set_LOD_Level(m_children.Current_Level() - 1);
}

float W3DHierarchyRenderObject::Get_Cost() const
{
    return m_detail_levels.Cost(m_children.Current_Level());
}

float W3DHierarchyRenderObject::Get_Value() const
{
    return m_detail_levels.Value(m_children.Current_Level());
}

float W3DHierarchyRenderObject::Get_Post_Increment_Value() const
{
    return m_detail_levels.Value(m_children.Current_Level() + 1);
}

void W3DHierarchyRenderObject::Set_LOD_Level(int lod)
{
    m_children.Select_Level(lod,
        [&](const ChildAttachment& child) { if (Is_In_Scene()) child.model->Notify_Removed(Scene); },
        [&](const ChildAttachment& child) { if (Is_In_Scene()) child.model->Notify_Added(Scene); });
}

int W3DHierarchyRenderObject::Get_LOD_Level() const
{
	return m_children.Current_Level();
}

int W3DHierarchyRenderObject::Get_LOD_Count() const
{
	return m_children.Level_Count();
}

int W3DHierarchyRenderObject::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
    const auto count = static_cast<std::size_t>(m_children.Level_Count());
    return m_detail_levels.Calculate(screen_area, {values, count + 1}, {costs, count});
}

W3DRenderObject * W3DHierarchyRenderObject::Get_Current_LOD()
{
	int count = Get_Lod_Model_Count(m_children.Current_Level());

	if(!count)
		return nullptr;

	return Get_Lod_Model(m_children.Current_Level(), 0);
}

void W3DHierarchyRenderObject::Scale(float scale)
{
    if (scale == 1.0f) return;
    m_children.Visit_All([&](const ChildAttachment& child, int) { child.model->Scale(scale); });
    Hierarchy->Scale(scale);
    Set_Hierarchy_Valid(false);
    W3DRenderObject* container = Get_Container();
    if (container) container->Update_Obj_Space_Bounding_Volumes();
}

int W3DHierarchyRenderObject::Get_Num_Snap_Points()
{
	if (!SnapPoints.empty()) {
		return static_cast<int>(SnapPoints.size());
	} else {
		return 0;
	}
}

void W3DHierarchyRenderObject::Get_Snap_Point(int index,Vector3 * set)
{
	WWASSERT(set != nullptr);
	if (index>=0 && static_cast<std::size_t>(index)<SnapPoints.size()) {
		const auto& point=SnapPoints[index];
		set->Set(point.x,point.y,point.z);
	} else {
		set->X = set->Y = set->Z = 0;
	}
}

void W3DHierarchyRenderObject::Update_Sub_Object_Transforms()
{
    W3DAnimatedModelRenderObject::Update_Sub_Object_Transforms();
    m_children.Visit_All([&](const ChildAttachment& child, int) {
        child.model->Set_Transform(Graphics::Export_Affine_Transform<Matrix3D>(Hierarchy->World_Transform(child.bone)));
        child.model->Set_Animation_Hidden(!Hierarchy->Visible(child.bone));
        child.model->Update_Sub_Object_Transforms();
    });
    Set_Sub_Object_Transforms_Dirty(false);
}

void W3DHierarchyRenderObject::Update_Obj_Space_Bounding_Volumes()
{
	//
	//	Do we still have a valid bounding box index?
	//
	const auto high_lod = m_children.Level(m_children.Level_Count() - 1);
	int count = static_cast<int>(high_lod.size());
	if (	BoundingBoxIndex < 0 ||
			BoundingBoxIndex >= count ||
			high_lod[BoundingBoxIndex].model.Peek()->Class_ID() != W3DRenderObject::CLASSID_OBBOX)
	{
		BoundingBoxIndex = -1;
	}

	//
	//	Attempt to find an OBBox mesh inside the hierarchy
	//
	int index = static_cast<int>(high_lod.size());
	while (index -- && BoundingBoxIndex == -1) {
		W3DRenderObject *model = high_lod[index].model.Peek();

		//
		//	Is this an OBBox mesh?
		//
		if (model->Class_ID() == W3DRenderObject::CLASSID_OBBOX)
		{
			const char *name = model->Get_Name ();
			const char *name_seg = ::strchr (name, '.');
			if (name_seg != nullptr) {
				name = name_seg + 1;
			}

			//
			//	Does the name match the designator we are looking for?
			//
			if (equal_case_insensitive (name, "BOUNDINGBOX")) {
				BoundingBoxIndex = index;
			}
		}
	}

	int i;
	W3DRenderObject * robj = nullptr;

	// if we don't have any sub objects, just set default bounds
	if (Get_Num_Sub_Objects() <= 0) {
		ObjSphere.Init(Vector3(0,0,0),0);
		ObjBox.Center.Set(0,0,0);
		ObjBox.Extent.Set(0,0,0);
		return;
	}

	// loop through all sub-objects, combining their object-space bounding spheres and boxes.
	// Put our Hierarchy in its base pose at the origin.
	SphereClass sphere;
	AABoxClass obj_aabox;
	MinMaxAABoxClass box;

	Hierarchy->Evaluate_Rest(Graphics::Import_Affine_Transform(Matrix3D(true)));

	robj = Get_Sub_Object(0);
	WWASSERT(robj);

	const Matrix3D & bonetm = Graphics::Export_Affine_Transform<Matrix3D>(Hierarchy->World_Transform(Get_Sub_Object_Bone_Index(robj)));
	robj->Get_Obj_Space_Bounding_Sphere(sphere);
	sphere.Transform(bonetm);
	robj->Get_Obj_Space_Bounding_Box(obj_aabox);

	box.Init(obj_aabox);
	box.Transform(bonetm);

	robj->Release_Ref();

	for (i=1; i<Get_Num_Sub_Objects(); i++) {
		robj = Get_Sub_Object(i);
		WWASSERT(robj);

		const Matrix3D & bonetm = Graphics::Export_Affine_Transform<Matrix3D>(Hierarchy->World_Transform(Get_Sub_Object_Bone_Index(robj)));

		SphereClass tmpsphere;
		robj->Get_Obj_Space_Bounding_Sphere(tmpsphere);
		tmpsphere.Transform(bonetm);
		sphere.Add_Sphere(tmpsphere);

		AABoxClass tmpbox;
		robj->Get_Obj_Space_Bounding_Box(tmpbox);
		tmpbox.Transform(bonetm);
		box.Add_Box(tmpbox);

		robj->Release_Ref();
	}

	ObjSphere = sphere;
	ObjBox = box;

   Invalidate_Cached_Bounding_Volumes();
	Set_Hierarchy_Valid(false);

   // Now update the object space bounding volumes of this object's container:
   W3DRenderObject *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}

void W3DHierarchyRenderObject::Add_Lod_Model(int lod, W3DRenderObject * robj, int boneindex)
{
    WWASSERT(robj);
    if (boneindex >= Hierarchy->Bone_Count()) {
        WWDEBUG_SAY(("ERROR: Model %s tried to use bone %d in skeleton %s. Please re-export!", Get_Name(), boneindex, Hierarchy->Name()));
        boneindex = 0;
    }
    auto owner = ChildOwner::Create_Add_Ref(robj);
    robj->Set_Container(this);
    robj->Set_Transform(Graphics::Export_Affine_Transform<Matrix3D>(Hierarchy->World_Transform(boneindex)));
    if (Is_In_Scene() && lod == m_children.Current_Level()) robj->Notify_Added(Scene);
    m_children.Add(lod, std::move(owner), boneindex);
}

void W3DHierarchyRenderObject::Set_Hidden(int onoff)
{
	//
	//	Loop over all attached models
	//
	int additional_count = static_cast<int>(m_children.Additional().size());
	for (int index = 0; index < additional_count; index ++) {

		//
		//	Is this a particle emitter?
		//
		W3DRenderObject *model = m_children.Additional()[index].model.Peek();
		if (model->Class_ID () == W3DRenderObject::CLASSID_PARTICLEEMITTER) {

			//
			//	Pass the hidden bit onto the emitter
			//
			model->Set_Hidden(onoff);
		}
	}

	W3DAnimatedModelRenderObject::Set_Hidden(onoff);
}

namespace {
W3DRenderObject* Acquire_Aggregate_Model(const std::string& name)
{
    auto* manager=W3DAssetCatalog::Get_Instance();
    auto* object=manager->Create_Render_Obj(name.c_str());
    if(object)return object;
    // Preserve aggregate dependency loading from a standalone working-directory
    // file when the asset manager has not already published that model.
    std::error_code error;
    auto path=std::filesystem::current_path(error);
    if(error)return nullptr;
    path/=name+".w3d";
    if(!std::filesystem::exists(path,error) || error)return nullptr;
    if(!manager->Load_3D_Assets(path.string().c_str()))return nullptr;
    return manager->Create_Render_Obj(name.c_str());
}

W3DRenderObject* Create_Aggregate(const Assets::ModelAggregateDesc& description)
{
        const auto release=[](W3DRenderObject* object) { object->Release_Ref(); };
        using ModelOwner=std::unique_ptr<W3DRenderObject,decltype(release)>;
        ModelOwner model(Acquire_Aggregate_Model(description.base_model),release);
        if(!model)return nullptr;
        for(const auto& attachment:description.attachments) {
            ModelOwner child(Acquire_Aggregate_Model(attachment.model_name),release);
            if(!child)continue;
            model->Add_Sub_Object_To_Bone(child.get(),attachment.bone_name.c_str());
        }
        model->Set_Name(description.name.c_str());
        model->Set_Base_Model_Name(description.base_model.c_str());
        model->Set_Sub_Objects_Match_LOD(description.match_detail_levels);
        return model.release();
    }

W3DRenderObject* Create_Level_Set(const Assets::ModelLevelSetDesc& description)
{
        // Acquire dependencies in authored order. HLOD consumes lowest detail
        // first, so reverse only their placement, not the loading sequence.
        struct Models {
            std::vector<W3DRenderObject*> objects;
            ~Models() { for(auto* object:objects)if(object)object->Release_Ref(); }
        } models;
        models.objects.resize(description.levels.size());
        for(std::size_t i=0;i<description.levels.size();++i) {
            auto* object=W3DAssetCatalog::Get_Instance()->Create_Render_Obj(description.levels[i].name.c_str());
            if(!object)return nullptr;
            models.objects[models.objects.size()-1-i]=object;
        }
        return NEW_REF(W3DHierarchyRenderObject,(description.name.c_str(),models.objects.data(),static_cast<int>(models.objects.size())));
    }

}

Graphics::ModelFactory<W3DRenderObject>* Load_ModelLevels_Factory(ChunkLoadClass& cload)
{
    std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
    if(cload.Read(bytes.data(),static_cast<unsigned>(bytes.size()))!=bytes.size())return nullptr;
    Assets::ModelLevelSetDesc description;
    std::string error;
    if(!Assets::W3D::W3DRead_Model_Level_Set(bytes,description,error))return nullptr;
    auto data=std::make_shared<const Assets::ModelLevelSetDesc>(std::move(description));
    return new Graphics::ModelFactory<W3DRenderObject>(data->name,W3DRenderObject::CLASSID_DISTLOD,[data] { return Create_Level_Set(*data); });
}

Graphics::ModelFactory<W3DRenderObject>* Load_Aggregate_Factory(ChunkLoadClass& cload)
{
    std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
    if(cload.Read(bytes.data(),static_cast<unsigned>(bytes.size()))!=bytes.size())return nullptr;
    Assets::W3D::W3DAggregateDescription description;
    std::string error;
    if(!Assets::W3D::W3DRead_Model_Aggregate(bytes,description,error))return nullptr;
    const auto classification=static_cast<int>(description.original_class_id);
    auto data=std::make_shared<const Assets::ModelAggregateDesc>(std::move(description.model));
    return new Graphics::ModelFactory<W3DRenderObject>(data->name,classification,[data] { return Create_Aggregate(*data); });
}

Graphics::ModelFactory<W3DRenderObject> * Load_HModel_Factory(ChunkLoadClass& cload)
{
    std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
    if(cload.Read(bytes.data(),static_cast<unsigned>(bytes.size()))!=bytes.size())return nullptr;
    Assets::ModelAssemblyDesc description;std::string error;
    if(!Assets::W3D::W3DRead_Model_Assembly(bytes,false,description,error))return nullptr;
    auto data=std::make_shared<const Assets::ModelAssemblyDesc>(std::move(description));
    return new Graphics::ModelFactory<W3DRenderObject>(data->name,W3DRenderObject::CLASSID_HLOD,[data] { return NEW_REF(W3DHierarchyRenderObject,(*data)); });
}

