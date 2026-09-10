#include "W3DDevice/GameClient/W3DMeshDrawing.h"
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

#include "W3DDevice/GameClient/W3DCollectionRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
import Assets.Adapters.W3D.Collection;
#include "WWLib/chunkio.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "WWDebug/wwdebug.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

//#include "sr.hpp"

W3DCollectionRenderObject::W3DCollectionRenderObject() :
	SnapPoints()
{
    m_children.Initialize(1);
    Update_Obj_Space_Bounding_Volumes();
}

W3DCollectionRenderObject::W3DCollectionRenderObject(const Assets::ModelCollectionDesc & def) :

	SnapPoints()
{
    m_children.Initialize(1);
    Set_Name(def.name.c_str());
    for (const auto& name : def.children) {
        auto child = ChildOwner::Create_No_Add_Ref(W3DAssetCatalog::Get_Instance()->Create_Render_Obj(name.c_str()));
        child->Set_Container(this);
        m_children.Add(0, std::move(child), 0);
    }
    ProxyList = def.proxies;
    SnapPoints = def.snap_points;
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
}

W3DCollectionRenderObject::W3DCollectionRenderObject(const W3DCollectionRenderObject & src) :
	W3DModelGroupRenderObject(src),

	SnapPoints()
{
	*this = src;
}

W3DCollectionRenderObject & W3DCollectionRenderObject::operator = (const W3DCollectionRenderObject & that)
{
    if (this != &that) {
        Free();
        W3DModelGroupRenderObject::operator=(that);
        m_children.Clone_From(that.m_children, [&](const ChildOwner& source) {
            auto child = ChildOwner::Create_No_Add_Ref(source->Clone());
            child->Set_Container(this);
            return child;
        });
        ProxyList = that.ProxyList;
        SnapPoints = that.SnapPoints;
        Update_Sub_Object_Bits();
        Update_Obj_Space_Bounding_Volumes();
    }
    return *this;
}

W3DCollectionRenderObject::~W3DCollectionRenderObject()
{
	Free();
}

W3DRenderObject * W3DCollectionRenderObject::Clone() const
{
	return NEW_REF( W3DCollectionRenderObject, (*this));
}

void W3DCollectionRenderObject::Free()
{
    m_children.Clear([](const ChildOwner& child) { child->Set_Container(nullptr); });
    ProxyList.clear();
    SnapPoints.clear();
}

int W3DCollectionRenderObject::Class_ID()	const
{
	return W3DRenderObject::CLASSID_COLLECTION;
}

int W3DCollectionRenderObject::Get_Num_Polys() const
{
    int count = 0;
    m_children.Visit_All([&](const ChildAttachment& child, int) { count += child.model->Get_Num_Polys(); });
    return count;
}

void W3DCollectionRenderObject::Render(W3DRenderContext & rinfo)
{
    if (!Is_Not_Hidden_At_All()) return;
    if (Are_Sub_Object_Transforms_Dirty()) Update_Sub_Object_Transforms();
    m_children.Visit_All([&](const ChildAttachment& child, int) { Flush_Before_W3D_Object_Draw(*child.model); child.model->Render(rinfo); });
}

void W3DCollectionRenderObject::Set_Transform(const Matrix3D &m)
{
	W3DRenderObject::Set_Transform(m);
	Set_Sub_Object_Transforms_Dirty(true);
}

void W3DCollectionRenderObject::Set_Position(const Vector3 &v)
{
	W3DRenderObject::Set_Position(v);
	Set_Sub_Object_Transforms_Dirty(true);
}

int W3DCollectionRenderObject::Get_Num_Sub_Objects() const
{
	return static_cast<int>(m_children.Count());
}

W3DRenderObject * W3DCollectionRenderObject::Get_Sub_Object(int index) const
{
    const auto* child = index >= 0 ? m_children.At(index) : nullptr;
    WWASSERT(child);
    if (!child) return nullptr;
    auto retained = child->model;
    return retained.Release();
}

int W3DCollectionRenderObject::Add_Sub_Object(W3DRenderObject * subobj)
{
    WWASSERT(subobj);
    auto owner = ChildOwner::Create_Add_Ref(subobj);
    subobj->Set_Container(this);
    subobj->Set_Transform(Get_Transform_No_Validity_Check());
    m_children.Add(0, std::move(owner), 0);
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
    if (Is_In_Scene()) subobj->Notify_Added(Scene);
    return 1;
}

int W3DCollectionRenderObject::Remove_Sub_Object(W3DRenderObject * robj)
{
    if (!robj) return 0;
    const Matrix3D transform = Get_Transform();
    const auto matches = [&](const ChildAttachment& child) { return child.model.Peek() == robj; };
    if (!m_children.Find(matches)) return 0;
    // Collections notify while the child is still present, unlike animated hierarchies.
    if (Is_In_Scene()) robj->Notify_Removed(Scene);
    robj->Set_Container(nullptr);
    robj->Set_Transform(transform);
    auto removed = m_children.Extract_First(matches);
    removed.reset();
    Update_Sub_Object_Bits();
    Update_Obj_Space_Bounding_Volumes();
    return 1;
}

bool W3DCollectionRenderObject::Cast_Ray(W3DRayCastQuery & raytest)
{
    return m_children.Query_Level(0, [&](const ChildAttachment& child) { return child.model->Cast_Ray(raytest); });
}

bool W3DCollectionRenderObject::Cast_AABox(W3DBoxCastQuery & boxtest)
{
    return m_children.Query_Level(0, [&](const ChildAttachment& child) { return child.model->Cast_AABox(boxtest); });
}

bool W3DCollectionRenderObject::Cast_OBBox(W3DOrientedBoxCastQuery & boxtest)
{
    return m_children.Query_Level(0, [&](const ChildAttachment& child) { return child.model->Cast_OBBox(boxtest); });
}

bool W3DCollectionRenderObject::Intersect_AABox(W3DBoxIntersectionQuery & boxtest)
{
    return m_children.Query_Level(0, [&](const ChildAttachment& child) { return child.model->Intersect_AABox(boxtest); });
}

bool W3DCollectionRenderObject::Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest)
{
    return m_children.Query_Level(0, [&](const ChildAttachment& child) { return child.model->Intersect_OBBox(boxtest); });
}

void W3DCollectionRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere = BoundSphere;
}

void W3DCollectionRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box = BoundBox;
}

int W3DCollectionRenderObject::Snap_Point_Count()
{
	if (!SnapPoints.empty()) {
		return static_cast<int>(SnapPoints.size());
	} else {
		return 0;
	}
}

void W3DCollectionRenderObject::Get_Snap_Point(int index,Vector3 * set)
{
	WWASSERT(set != nullptr);
	if (index >= 0 && static_cast<std::size_t>(index) < SnapPoints.size()) {
		const auto& point = SnapPoints[index];
		set->Set(point.x, point.y, point.z);
	} else {
		set->X = set->Y = set->Z = 0;
	}
}

void W3DCollectionRenderObject::Scale(float scale)
{
    m_children.Visit_All([&](const ChildAttachment& child, int) { child.model->Scale(scale); });
}

void W3DCollectionRenderObject::Scale(float scalex, float scaley, float scalez)
{
    m_children.Visit_All([&](const ChildAttachment& child, int) { child.model->Scale(scalex, scaley, scalez); });
}

void W3DCollectionRenderObject::Update_Obj_Space_Bounding_Volumes()
{
	int i;
	if (static_cast<int>(m_children.Count()) <= 0) {
		BoundSphere = SphereClass();
		BoundBox.Center.Set(0,0,0);
		BoundBox.Extent.Set(0,0,0);
		return;
	}

	Matrix3D tm = Get_Transform();
	Set_Transform(Matrix3D(true));

	// loop through all sub-objects, combining their bounding spheres.
	BoundSphere = m_children.At(0)->model.Peek()->Get_Bounding_Sphere();
	for (i=1; i < static_cast<int>(m_children.Count()); i++) {
		BoundSphere.Add_Sphere(m_children.At(i)->model.Peek()->Get_Bounding_Sphere());
	}

	// loop through the sub-objects, computing a box in the root coordinate
	// system which bounds all of the meshes.  Note that we've set the
	// root coordinate system to identity for this.
	MinMaxAABoxClass box(Vector3(FLT_MAX,FLT_MAX,FLT_MAX),Vector3(-FLT_MAX,-FLT_MAX,-FLT_MAX));

	for (i=0; i < static_cast<int>(m_children.Count()); i++) {
		box.Add_Box(m_children.At(i)->model.Peek()->Get_Bounding_Box());
	}

	BoundBox.Init(box);

   Invalidate_Cached_Bounding_Volumes();

   // Now update the object space bounding volumes of this object's container:
   W3DRenderObject *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();

	Set_Transform(tm);
}

void W3DCollectionRenderObject::Update_Sub_Object_Transforms()
{
    W3DRenderObject::Update_Sub_Object_Transforms();
    m_children.Visit_All([&](const ChildAttachment& child, int) {
        child.model->Set_Transform(Get_Transform_No_Validity_Check());
        child.model->Update_Sub_Object_Transforms();
    });
    Set_Sub_Object_Transforms_Dirty(false);
}

Graphics::ModelFactory<W3DRenderObject> * Load_Collection_Factory(ChunkLoadClass & cload)
{
    std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
    if(cload.Read(bytes.data(),static_cast<unsigned>(bytes.size()))!=bytes.size())return nullptr;
    Assets::ModelCollectionDesc description;
    std::string error;
    if(!Assets::W3D::W3DRead_Model_Collection(bytes,description,error))return nullptr;
    auto data=std::make_shared<const Assets::ModelCollectionDesc>(std::move(description));
    return new Graphics::ModelFactory<W3DRenderObject>(data->name,W3DRenderObject::CLASSID_COLLECTION,[data] { return NEW_REF(W3DCollectionRenderObject,(*data)); });
}

