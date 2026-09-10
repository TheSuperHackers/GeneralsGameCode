#include "W3DDevice/GameClient/W3DRenderObject.h"

#include <array>
#include <cstring>
#include <span>
#include <string_view>

#include "WWLib/Vector.h"
#include "WWLib/wwstring.h"
#include "WWMath/wwmath.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"

import Assets.Identity;

namespace
{

StringClass Filename_From_Asset_Name(const char *asset_name)
{
    StringClass filename;
    if (asset_name == nullptr)
        return filename;

    std::strcpy(filename.Get_Buffer(static_cast<int>(std::strlen(asset_name)) + 5), asset_name);
    if (char *suffix = std::strchr(filename.Peek_Buffer(), '.'))
        *suffix = '\0';
    filename += ".w3d";
    return filename;
}

}

W3DRenderObject::W3DRenderObject()
    : ObjectColor(0),
      CachedBoundingSphere(Vector3(0, 0, 0), 1.0f),
      CachedBoundingBox(Vector3(0, 0, 0), Vector3(1, 1, 1)),
      Transform(true),
      Scene(nullptr),
      Container(nullptr),
      User_Data(nullptr),
      RenderHook(nullptr),
      NativeState()
{
}

W3DRenderObject::W3DRenderObject(const W3DRenderObject &source)
    : RefCountClass(source),
      PersistClass(source),
      Graphics::SceneListMember(source),
      ObjectColor(0),
      CachedBoundingSphere(source.CachedBoundingSphere),
      CachedBoundingBox(source.CachedBoundingBox),
      Transform(source.Get_Transform_No_Validity_Check()),
      m_bounds_valid(source.m_bounds_valid),
      m_collision_type(source.m_collision_type),
      Scene(nullptr),
      Container(nullptr),
      User_Data(nullptr),
      RenderHook(nullptr),
      NativeState(source.NativeState)
{
	// Object scale is a per-instance override and the historical copy
	// constructor deliberately reset it for the new object.
	NativeState.Set_Object_Scale(1.0f);
}

W3DRenderObject &W3DRenderObject::operator=(const W3DRenderObject &source)
{
    if (this == &source)
        return *this;

    Set_Hidden(source.Is_Hidden());
    Set_Animation_Hidden(source.Is_Animation_Hidden());
    Set_Force_Visible(source.Is_Force_Visible());
    Set_Collision_Type(source.Get_Collision_Type());
    Set_Native_Screen_Size(source.Get_Native_Screen_Size());
    NativeState.Set_Transform_Identity(false);
    return *this;
}

W3DRenderObject::~W3DRenderObject()
{
    delete RenderHook;
}

void W3DRenderObject::Add(W3DScene *scene)
{
    WWASSERT(scene != nullptr);
    WWASSERT(Container == nullptr);
    Scene = scene;
    scene->Add_Render_Object(this);
}

bool W3DRenderObject::Remove()
{
    if (Container == nullptr) {
        if (Scene == nullptr)
            return false;
        Scene->Remove_Render_Object(this);
        return true;
    }

    Container->Remove_Sub_Object(this);
    return true;
}

W3DScene *W3DRenderObject::Get_Scene()
{
    if (Scene != nullptr)
        Scene->Add_Ref();
    return Scene;
}

void W3DRenderObject::Set_Container(W3DRenderObject *container)
{
    WWASSERT(container == nullptr || Container == nullptr);
    Container = container;
}

void W3DRenderObject::Set_Transform(const Matrix3D &transform)
{
    Transform = transform;
    NativeState.Set_Transform(Graphics::Import_Affine_Transform(transform));
    Invalidate_Cached_Bounding_Volumes();
}

void W3DRenderObject::Set_Position(const Vector3 &position)
{
    Transform.Set_Translation(position);
    NativeState.Set_Position({position.X, position.Y, position.Z});
    Invalidate_Cached_Bounding_Volumes();
}

const Matrix3D &W3DRenderObject::Get_Transform() const
{
    Validate_Transform();
    Transform = Graphics::Export_Affine_Transform<Matrix3D>(NativeState.Transform());
    return Transform;
}

const Matrix3D &W3DRenderObject::Get_Transform(bool &is_transform_identity) const
{
    Validate_Transform();
    Transform = Graphics::Export_Affine_Transform<Matrix3D>(NativeState.Transform());
    is_transform_identity = NativeState.Is_Transform_Identity();
    return Transform;
}

bool W3DRenderObject::Is_Transform_Identity() const
{
    Validate_Transform();
    return NativeState.Is_Transform_Identity();
}

const Matrix3D &W3DRenderObject::Get_Transform_No_Validity_Check() const
{
    Transform = Graphics::Export_Affine_Transform<Matrix3D>(NativeState.Transform());
    return Transform;
}

void W3DRenderObject::Validate_Transform() const
{
    W3DRenderObject *container = Get_Container();
    bool dirty = false;
    if (container != nullptr) {
        dirty = container->Are_Sub_Object_Transforms_Dirty();
        while (container->Get_Container() != nullptr) {
            dirty = container->Are_Sub_Object_Transforms_Dirty() || dirty;
            container = container->Get_Container();
        }
        if (dirty)
            container->Update_Sub_Object_Transforms();
    }

    if (dirty) {
        Transform = Graphics::Export_Affine_Transform<Matrix3D>(NativeState.Transform());
    }
}

Vector3 W3DRenderObject::Get_Position() const
{
    Validate_Transform();
    return Get_Transform_No_Validity_Check().Get_Translation();
}

void W3DRenderObject::Notify_Added(W3DScene *scene)
{
    Scene = scene;
}

void W3DRenderObject::Notify_Removed(W3DScene *scene)
{
    (void)scene;
    Scene = nullptr;
}

W3DRenderObject *W3DRenderObject::Get_Sub_Object_By_Name(const char *name, int *index) const
{
    if (name == nullptr)
        return nullptr;

    return Graphics::Find_Render_Object_Child_By_Name(
        static_cast<std::size_t>(Get_Num_Sub_Objects()), std::string_view(name),
        [this](std::size_t child_index) { return Get_Sub_Object(static_cast<int>(child_index)); },
        [](W3DRenderObject *child) { child->Release_Ref(); },
        [](const W3DRenderObject &child) {
            const char *child_name = child.Get_Name();
            return child_name == nullptr ? std::string_view{} : std::string_view(child_name);
        }, index);
}

int W3DRenderObject::Add_Sub_Object_To_Bone(W3DRenderObject *object, const char *bone_name)
{
    return Add_Sub_Object_To_Bone(object, Get_Bone_Index(bone_name));
}

int W3DRenderObject::Remove_Sub_Objects_From_Bone(int bone_index)
{
    int remove_count = 0;
    for (int index = Get_Num_Sub_Objects_On_Bone(bone_index) - 1; index >= 0; --index) {
        W3DRenderObject *object = Get_Sub_Object_On_Bone(index, bone_index);
        if (object != nullptr) {
            remove_count += Remove_Sub_Object(object);
            object->Release_Ref();
        }
    }
    return remove_count;
}

int W3DRenderObject::Remove_Sub_Objects_From_Bone(const char *bone_name)
{
    return Remove_Sub_Objects_From_Bone(Get_Bone_Index(bone_name));
}

void W3DRenderObject::Update_Sub_Object_Transforms()
{
}

const SphereClass &W3DRenderObject::Get_Bounding_Sphere() const
{
    if (!Bounding_Volumes_Valid())
        Update_Cached_Bounding_Volumes();
    return CachedBoundingSphere;
}

const AABoxClass &W3DRenderObject::Get_Bounding_Box() const
{
    if (!Bounding_Volumes_Valid())
        Update_Cached_Bounding_Volumes();
    return CachedBoundingBox;
}

void W3DRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Center.Set(0, 0, 0);
    sphere.Radius = 1.0f;
}

void W3DRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Center.Set(0, 0, 0);
    box.Extent.Set(0, 0, 0);
}

void W3DRenderObject::Update_Cached_Bounding_Volumes() const
{
    SphereClass local_sphere;
    AABoxClass local_box;
    Get_Obj_Space_Bounding_Sphere(local_sphere);
    Get_Obj_Space_Bounding_Box(local_box);

    const Graphics::RenderObjectBounds local_bounds{
        {{local_sphere.Center.X, local_sphere.Center.Y, local_sphere.Center.Z}, local_sphere.Radius},
        {{local_box.Center.X, local_box.Center.Y, local_box.Center.Z},
            {local_box.Extent.X, local_box.Extent.Y, local_box.Extent.Z}}
    };
    const Graphics::RenderObjectBounds world_bounds =
        Graphics::Transform_Render_Object_Bounds(
            Graphics::Import_Affine_Transform(Get_Transform_No_Validity_Check()),
            local_bounds, NativeState.Object_Scale());

    CachedBoundingSphere.Center.Set(
        world_bounds.sphere.center[0], world_bounds.sphere.center[1], world_bounds.sphere.center[2]);
    CachedBoundingSphere.Radius = world_bounds.sphere.radius;
    CachedBoundingBox.Init(
        Vector3(world_bounds.box.center[0], world_bounds.box.center[1], world_bounds.box.center[2]),
        Vector3(world_bounds.box.extent[0], world_bounds.box.extent[1], world_bounds.box.extent[2]));
    Validate_Cached_Bounding_Volumes();
}

float W3DRenderObject::Get_Cost() const
{
    return Graphics::Base_Render_Object_Cost(static_cast<float>(Get_Num_Polys()));
}

int W3DRenderObject::Calculate_Cost_Value_Arrays(
    float screen_area, float *values, float *costs) const
{
    (void)screen_area;
    return Graphics::Calculate_Base_Render_Object_LOD(
        static_cast<float>(Get_Num_Polys()), std::span<float>(values, 2),
        std::span<float>(costs, 1));
}

bool W3DRenderObject::Build_Dependency_List(
    DynamicVectorClass<StringClass> &file_list, bool recursive)
{
    if (recursive) {
        for (int index = 0; index < Get_Num_Sub_Objects(); ++index) {
            W3DRenderObject *object = Get_Sub_Object(index);
            if (object != nullptr) {
                object->Build_Dependency_List(file_list);
                object->Release_Ref();
            }
        }
    }
    Add_Dependencies_To_List(file_list);
    return file_list.Count() > 0;
}

bool W3DRenderObject::Build_Texture_List(
    DynamicVectorClass<StringClass> &file_list, bool recursive)
{
    if (recursive) {
        for (int index = 0; index < Get_Num_Sub_Objects(); ++index) {
            W3DRenderObject *object = Get_Sub_Object(index);
            if (object != nullptr) {
                object->Build_Texture_List(file_list);
                object->Release_Ref();
            }
        }
    }
    Add_Dependencies_To_List(file_list, true);
    return file_list.Count() > 0;
}

void W3DRenderObject::Add_Dependencies_To_List(
    DynamicVectorClass<StringClass> &file_list, bool textures_only)
{
    if (textures_only)
        return;

    const char *model_name = Get_Name();
    file_list.Add(Filename_From_Asset_Name(model_name));

    const Graphics::ModelHierarchy *hierarchy = Get_Model_Hierarchy();
    if (hierarchy != nullptr) {
        const char *hierarchy_name = hierarchy->Name();
        if (!Assets::Asset_Name_Equals_No_Case(hierarchy_name, model_name))
            file_list.Add(Filename_From_Asset_Name(hierarchy_name));
    }

    if (const char *base_model_name = Get_Base_Model_Name(); base_model_name != nullptr)
        file_list.Add(Filename_From_Asset_Name(base_model_name));
}

void W3DRenderObject::Set_User_Data(void *value, bool recursive)
{
    User_Data = value;
    if (!recursive)
        return;

    for (int index = 0; index < Get_Num_Sub_Objects(); ++index) {
        W3DRenderObject *object = Get_Sub_Object(index);
        if (object != nullptr) {
            object->Set_User_Data(value, true);
            object->Release_Ref();
        }
    }
}

float W3DRenderObject::Get_Screen_Size(W3DCamera &camera)
{
    const Vector3 camera_position = camera.Get_Position();
    const W3DViewport &viewport = camera.Get_Viewport();
    Vector2 view_min, view_max;
    camera.Get_View_Plane(view_min, view_max);
    const SphereClass &sphere = Get_Bounding_Sphere();
    return Graphics::Project_Render_Object_Screen_Size(
        {camera_position.X, camera_position.Y, camera_position.Z},
        {sphere.Center.X, sphere.Center.Y, sphere.Center.Z}, sphere.Radius,
        viewport.Width(), viewport.Height(), view_max.X - view_min.X, view_max.Y - view_min.Y);
}

void W3DRenderObject::Set_ObjectScale(float scale)
{
    NativeState.Set_Object_Scale(scale);
    Invalidate_Cached_Bounding_Volumes();
}

void W3DRenderObject::Set_Visible(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::Visible, onoff != 0);
}

void W3DRenderObject::Set_Hidden(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::NotHidden, onoff == 0);
}

void W3DRenderObject::Set_Animation_Hidden(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::NotAnimationHidden, onoff == 0);
}

void W3DRenderObject::Set_Force_Visible(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::ForceVisible, onoff != 0);
}

void W3DRenderObject::Set_Translucent(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::Translucent, onoff != 0);
}

void W3DRenderObject::Set_Alpha(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::Alpha, onoff != 0);
}

void W3DRenderObject::Set_Additive(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::Additive, onoff != 0);
}

void W3DRenderObject::Set_Collision_Type(int type)
{
    m_collision_type = (type & SCENE_QUERY_MASK) | SCENE_QUERY_ALL;
}

void W3DRenderObject::Set_Native_Screen_Size(float screensize)
{
    NativeState.Set_Native_Screen_Size(screensize);
}

void W3DRenderObject::Set_Sub_Objects_Match_LOD(int onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::SubObjectsMatchLod, onoff != 0);
}

void W3DRenderObject::Set_Sub_Object_Transforms_Dirty(bool onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::SubObjectTransformsDirty, onoff);
}

void W3DRenderObject::Set_Ignore_LOD_Cost(bool onoff)
{
    NativeState.Set_Flag(Graphics::RenderObjectFlags::IgnoreLodCost, onoff);
}

void W3DRenderObject::Set_Render_Hook(W3DRenderHook *hook)
{
    if (RenderHook == hook)
        return;
    delete RenderHook;
    RenderHook = hook;
}

void W3DRenderObject::Update_Sub_Object_Bits()
{
    if (Get_Num_Sub_Objects() == 0)
        return;

    int collision_type = 0;
    const Graphics::RenderObjectFlags child_flags =
        Graphics::Aggregate_Render_Object_Child_Flags(
            static_cast<std::size_t>(Get_Num_Sub_Objects()),
            [this](std::size_t index) {
                return Get_Sub_Object(static_cast<int>(index));
            },
            [](W3DRenderObject *object) { object->Release_Ref(); },
            [&collision_type](const W3DRenderObject &object) {
                collision_type |= object.Get_Collision_Type();
                Graphics::RenderObjectFlags flags = Graphics::RenderObjectFlags::None;
                if (object.Is_Translucent())
                    flags |= Graphics::RenderObjectFlags::Translucent;
                if (object.Is_Alpha())
                    flags |= Graphics::RenderObjectFlags::Alpha;
                if (object.Is_Additive())
                    flags |= Graphics::RenderObjectFlags::Additive;
                return flags;
            });

    Set_Collision_Type(collision_type);
    Set_Translucent(Graphics::Has_Render_Object_Flag(
        child_flags, Graphics::RenderObjectFlags::Translucent));
    Set_Alpha(Graphics::Has_Render_Object_Flag(
        child_flags, Graphics::RenderObjectFlags::Alpha));
    Set_Additive(Graphics::Has_Render_Object_Flag(
        child_flags, Graphics::RenderObjectFlags::Additive));
    if (Container != nullptr)
        Container->Update_Sub_Object_Bits();
}

bool Extract_Ordered_Draw(W3DRenderObject &object, void *context)
{
    if (context == nullptr)
        return false;
    auto &render_context = *static_cast<W3DRenderContext *>(context);
    return Graphics::Extract_Render_Object_Draw(object, render_context);
}
