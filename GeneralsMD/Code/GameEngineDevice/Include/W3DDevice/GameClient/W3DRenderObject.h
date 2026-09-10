#pragma once

#include <cstdint>
#include <memory>

#include "WWLib/ref_ptr.h"
#include "WWMath/aabox.h"
#include "WWMath/matrix3d.h"
#include "WWMath/sphere.h"
#include "WWMath/vector2.h"
#include "WWMath/vector3.h"
#include "W3DDevice/GameClient/W3DSceneQueryMask.h"
#include "WWSaveLoad/persist.h"

import Assets.Cache.Animations;
import Graphics.Scene.Lighting.Local;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.Models.Materials;
import Graphics.Scene.ObjectList;
import Graphics.Scene.RenderObjectBounds;
import Graphics.Scene.RenderObjectDrawing;
import Graphics.Scene.RenderObjectHierarchy;
import Graphics.Scene.RenderObjectLOD;
import Graphics.Scene.RenderObjectState;

class ChunkLoadClass;
class ChunkSaveClass;
class StringClass;
class W3DTextureHandle;
class W3DCamera;
class W3DRenderObject;
class W3DRenderContext;
class W3DScene;
class W3DRayCastQuery;
class W3DBoxCastQuery;
class W3DOrientedBoxCastQuery;
class W3DBoxIntersectionQuery;
class W3DOrientedBoxIntersectionQuery;

template<class T> class DynamicVectorClass;

// Application-owned render hooks remain at the game boundary. The generic
// graphics extraction helper invokes these hooks without knowing their policy.
class W3DRenderHook
{
public:
    W3DRenderHook() = default;
    W3DRenderHook(const W3DRenderHook &) = delete;
    W3DRenderHook &operator=(const W3DRenderHook &) = delete;
    virtual ~W3DRenderHook() = default;

    virtual bool Pre_Render(W3DRenderObject *object, W3DRenderContext &context) = 0;
    virtual void Post_Render(W3DRenderObject *object, W3DRenderContext &context) = 0;
};

// Render objects provide the legacy game-facing surface while storing common
// transform, visibility, bounds, hierarchy, and LOD state in Graphics.
// Asset-specific drawing and collision behavior remains in derived adapters.
class W3DRenderObject : public RefCountClass, public PersistClass,
    public Graphics::SceneListMember
{
public:
    enum { USER_DATA_MATERIAL_OVERRIDE = 0x01234567 };

    struct Material_Override
    {
        Material_Override() : Struct_ID(USER_DATA_MATERIAL_OVERRIDE), customUVOffset(0, 0) {}
        int Struct_ID;
        Vector2 customUVOffset;
    };

    enum
    {
        CLASSID_UNKNOWN = 0xFFFFFFFF,
        CLASSID_MESH = 0,
        CLASSID_HMODEL,
        CLASSID_DISTLOD,
        CLASSID_PREDLODGROUP,
        CLASSID_TILEMAP,
        CLASSID_IMAGE3D,
        CLASSID_LINE3D,
        CLASSID_BITMAP2D,
        CLASSID_CAMERA,
        CLASSID_DYNAMESH,
        CLASSID_DYNASCREENMESH,
        CLASSID_TEXTDRAW,
        CLASSID_FOG,
        CLASSID_LAYERFOG,
        CLASSID_LIGHT,
        CLASSID_PARTICLEEMITTER,
        CLASSID_PARTICLEBUFFER,
        CLASSID_SCREENPOINTGROUP,
        CLASSID_VIEWPOINTGROUP,
        CLASSID_WORLDPOINTGROUP,
        CLASSID_TEXT2D,
        CLASSID_TEXT3D,
        CLASSID_NULL,
        CLASSID_COLLECTION,
        CLASSID_FLARE,
        CLASSID_HLOD,
        CLASSID_AABOX,
        CLASSID_OBBOX,
        CLASSID_SEGLINE,
        CLASSID_SPHERE,
        CLASSID_RING,
        CLASSID_BOUNDFOG,
        CLASSID_DAZZLE,
        CLASSID_SOUND,
        CLASSID_SEGLINETRAIL,
        CLASSID_LAND,
        CLASSID_SHDMESH,
        CLASSID_LAST = 0x0000FFFF
    };

    enum AnimMode
    {
        ANIM_MODE_MANUAL = 0,
        ANIM_MODE_LOOP,
        ANIM_MODE_ONCE,
        ANIM_MODE_LOOP_PINGPONG,
        ANIM_MODE_LOOP_BACKWARDS,
        ANIM_MODE_ONCE_BACKWARDS,
        ANIM_MODE_COUNT
    };

    static constexpr float AT_MIN_LOD = Graphics::Render_Object_Min_LOD_Value;
    static constexpr float AT_MAX_LOD = Graphics::Render_Object_Max_LOD_Value;

    W3DRenderObject();
    W3DRenderObject(const W3DRenderObject &source);
    W3DRenderObject &operator=(const W3DRenderObject &source);
    ~W3DRenderObject() override;

    virtual W3DRenderObject *Clone() const = 0;
    virtual int Class_ID() const { return CLASSID_UNKNOWN; }
    virtual const char *Get_Name() const { return "UNNAMED"; }
    virtual void Set_Name(const char *) {}
    virtual const char *Get_Base_Model_Name() const { return nullptr; }
    virtual void Set_Base_Model_Name(const char *) {}
    virtual int Get_Num_Polys() const { return 0; }
    virtual bool Get_Light_Description(Graphics::MaterialLightSource &) const { return false; }

    virtual void Render(W3DRenderContext &context) = 0;
    virtual void On_Frame_Update() {}
    virtual void Restart() {}

    void Add(W3DScene *scene);
    bool Remove();
    W3DScene *Get_Scene();
    W3DScene *Peek_Scene() const { return Scene; }
    virtual void Set_Container(W3DRenderObject *container);
    virtual void Validate_Transform() const;
    W3DRenderObject *Get_Container() const { return Container; }

    virtual void Set_Transform(const Matrix3D &transform);
    virtual void Set_Position(const Vector3 &position);
    const Matrix3D &Get_Transform() const;
    const Matrix3D &Get_Transform(bool &is_transform_identity) const;
    const Matrix3D &Get_Transform_No_Validity_Check() const;
    const Matrix3D &Get_Transform_No_Validity_Check(bool &is_transform_identity) const
    {
        is_transform_identity = NativeState.Is_Transform_Identity();
        return Get_Transform_No_Validity_Check();
    }
    bool Is_Transform_Identity() const;
    bool Is_Transform_Identity_No_Validity_Check() const { return NativeState.Is_Transform_Identity(); }
    Vector3 Get_Position() const;

    virtual void Notify_Added(W3DScene *scene);
    virtual void Notify_Removed(W3DScene *scene);

    virtual int Get_Num_Sub_Objects() const { return 0; }
    virtual W3DRenderObject *Get_Sub_Object(int) const { return nullptr; }
    virtual int Add_Sub_Object(W3DRenderObject *) { return 0; }
    virtual int Remove_Sub_Object(W3DRenderObject *) { return 0; }
    virtual W3DRenderObject *Get_Sub_Object_By_Name(const char *name, int *index = nullptr) const;
    virtual int Get_Num_Sub_Objects_On_Bone(int) const { return 0; }
    virtual W3DRenderObject *Get_Sub_Object_On_Bone(int, int) const { return nullptr; }
    virtual int Get_Sub_Object_Bone_Index(W3DRenderObject *) const { return 0; }
    virtual int Get_Sub_Object_Bone_Index(int, int) const { return 0; }
    virtual int Add_Sub_Object_To_Bone(W3DRenderObject *, int) { return 0; }
    virtual int Add_Sub_Object_To_Bone(W3DRenderObject *object, const char *bone_name);
    virtual int Remove_Sub_Objects_From_Bone(int bone_index);
    virtual int Remove_Sub_Objects_From_Bone(const char *bone_name);
    virtual void Update_Sub_Object_Transforms();

    virtual void Set_Animation() {}
    virtual void Set_Animation(Assets::AnimationAssetHandle, float, int = ANIM_MODE_MANUAL) {}
    virtual void Set_Animation(Assets::AnimationAssetHandle, float,
        Assets::AnimationAssetHandle, float, float) {}
    virtual Assets::AnimationAssetHandle Peek_Animation() { return nullptr; }
    virtual int Get_Num_Bones() { return 0; }
    virtual const char *Get_Bone_Name(int) { return nullptr; }
    virtual int Get_Bone_Index(const char *) { return 0; }
    virtual Matrix3D Get_Bone_Transform(const char *) { return Get_Transform(); }
    virtual Matrix3D Get_Bone_Transform(int) { return Get_Transform(); }
    virtual void Capture_Bone(int) {}
    virtual void Release_Bone(int) {}
    virtual bool Is_Bone_Captured(int) const { return false; }
    virtual void Control_Bone(int, const Matrix3D &, bool = false) {}
    virtual const Graphics::ModelHierarchy *Get_Model_Hierarchy() const { return nullptr; }

    virtual bool Cast_Ray(W3DRayCastQuery &) { return false; }
    virtual bool Cast_AABox(W3DBoxCastQuery &) { return false; }
    virtual bool Cast_OBBox(W3DOrientedBoxCastQuery &) { return false; }
    virtual bool Intersect_AABox(W3DBoxIntersectionQuery &) { return false; }
    virtual bool Intersect_OBBox(W3DOrientedBoxIntersectionQuery &) { return false; }

    virtual const SphereClass &Get_Bounding_Sphere() const;
    virtual const AABoxClass &Get_Bounding_Box() const;
    virtual void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const;
    virtual void Get_Obj_Space_Bounding_Box(AABoxClass &box) const;
    virtual void Update_Obj_Space_Bounding_Volumes() {}

    virtual void Prepare_LOD(W3DCamera &) {}
    virtual void Recalculate_Static_LOD_Factors() {}
    virtual void Increment_LOD() {}
    virtual void Decrement_LOD() {}
    virtual float Get_Cost() const;
    virtual float Get_Value() const { return AT_MIN_LOD; }
    virtual float Get_Post_Increment_Value() const { return AT_MAX_LOD; }
    virtual void Set_LOD_Level(int) {}
    virtual int Get_LOD_Level() const { return 0; }
    virtual int Get_LOD_Count() const { return 1; }
    virtual void Set_LOD_Bias(float) {}
    virtual int Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const;
    virtual W3DRenderObject *Get_Current_LOD() { Add_Ref(); return this; }

    virtual bool Build_Dependency_List(DynamicVectorClass<StringClass> &file_list,
        bool recursive = true);
    virtual bool Build_Texture_List(DynamicVectorClass<StringClass> &file_list,
        bool recursive = true);

    virtual std::shared_ptr<Graphics::ModelMaterials<RefCountPtr<W3DTextureHandle>>> Get_Material_Info()
    {
        return nullptr;
    }
    virtual void Set_User_Data(void *value, bool recursive = false);
    virtual void *Get_User_Data() { return User_Data; }
    virtual int Get_Num_Snap_Points() { return 0; }
    virtual void Get_Snap_Point(int, Vector3 *) {}
    virtual float Get_Screen_Size(W3DCamera &camera);
    virtual void Scale(float) {}
    virtual void Scale(float, float, float) {}
    virtual void Set_ObjectScale(float scale);
    float Get_ObjectScale() const { return NativeState.Object_Scale(); }
    void Set_ObjectColor(unsigned int color) { ObjectColor = color; }
    unsigned int Get_ObjectColor() const { return ObjectColor; }
    virtual int Get_Sort_Level() const { return 0; }
    virtual void Set_Sort_Level(int) {}

    virtual int Is_Really_Visible() { return NativeState.Has_Flag(Graphics::RenderObjectFlags::Visible)
        && NativeState.Has_Flag(Graphics::RenderObjectFlags::NotHidden)
        && NativeState.Has_Flag(Graphics::RenderObjectFlags::NotAnimationHidden); }
    virtual int Is_Not_Hidden_At_All() { return NativeState.Has_Flag(Graphics::RenderObjectFlags::NotHidden)
        && NativeState.Has_Flag(Graphics::RenderObjectFlags::NotAnimationHidden); }
    virtual int Is_Visible() const { return NativeState.Has_Flag(Graphics::RenderObjectFlags::Visible); }
    virtual void Set_Visible(int onoff);
    virtual int Is_Hidden() const { return !NativeState.Has_Flag(Graphics::RenderObjectFlags::NotHidden); }
    virtual void Set_Hidden(int onoff);
    virtual int Is_Animation_Hidden() const { return !NativeState.Has_Flag(Graphics::RenderObjectFlags::NotAnimationHidden); }
    virtual void Set_Animation_Hidden(int onoff);
    virtual int Is_Force_Visible() const { return NativeState.Has_Flag(Graphics::RenderObjectFlags::ForceVisible); }
    virtual void Set_Force_Visible(int onoff);
    virtual int Is_Translucent() const { return NativeState.Has_Flag(Graphics::RenderObjectFlags::Translucent); }
    virtual void Set_Translucent(int onoff);
    virtual int Is_Alpha() const { return NativeState.Has_Flag(Graphics::RenderObjectFlags::Alpha); }
    virtual void Set_Alpha(int onoff);
    virtual int Is_Additive() const { return NativeState.Has_Flag(Graphics::RenderObjectFlags::Additive); }
    virtual void Set_Additive(int onoff);
    virtual int Get_Collision_Type() const { return m_collision_type; }
    virtual void Set_Collision_Type(int type);
    virtual bool Is_Complete() { return false; }
    virtual bool Is_In_Scene() { return Scene != nullptr; }
    virtual float Get_Native_Screen_Size() const { return NativeState.Native_Screen_Size(); }
    virtual void Set_Native_Screen_Size(float screensize);
    void Set_Sub_Objects_Match_LOD(int onoff);
    int Is_Sub_Objects_Match_LOD_Enabled() { return NativeState.Has_Flag(Graphics::RenderObjectFlags::SubObjectsMatchLod); }
    void Set_Sub_Object_Transforms_Dirty(bool onoff);
    bool Are_Sub_Object_Transforms_Dirty() { return NativeState.Has_Flag(Graphics::RenderObjectFlags::SubObjectTransformsDirty); }
    void Set_Ignore_LOD_Cost(bool onoff);
    bool Is_Ignoring_LOD_Cost() { return NativeState.Has_Flag(Graphics::RenderObjectFlags::IgnoreLodCost); }
    void Set_Is_Self_Shadowed() { NativeState.Set_Flag(Graphics::RenderObjectFlags::SelfShadowed, true); }
    void Unset_Is_Self_Shadowed() { NativeState.Set_Flag(Graphics::RenderObjectFlags::SelfShadowed, false); }
    int Is_Self_Shadowed() const { return NativeState.Has_Flag(Graphics::RenderObjectFlags::SelfShadowed); }

    const PersistFactoryClass &Get_Factory() const override;
    bool Save(ChunkSaveClass &) override;
    bool Load(ChunkLoadClass &) override;

    W3DRenderHook *Get_Render_Hook() { return RenderHook; }
    void Set_Render_Hook(W3DRenderHook *hook);

protected:
    virtual void Add_Dependencies_To_List(DynamicVectorClass<StringClass> &file_list,
        bool textures_only = false);
    virtual void Update_Cached_Bounding_Volumes() const;
    virtual void Update_Sub_Object_Bits();

    bool Bounding_Volumes_Valid() const { return m_bounds_valid; }
    void Invalidate_Cached_Bounding_Volumes() const { m_bounds_valid = false; }
    void Validate_Cached_Bounding_Volumes() const { m_bounds_valid = true; }

    enum
    {
        SCENE_QUERY_MASK = 0x000000FF,
        DEFAULT_BITS = SCENE_QUERY_ALL
    };

    unsigned int ObjectColor;
    mutable SphereClass CachedBoundingSphere;
    mutable AABoxClass CachedBoundingBox;
    mutable Matrix3D Transform;
    mutable bool m_bounds_valid = false;
    int m_collision_type = SCENE_QUERY_ALL;
    W3DScene *Scene;
    W3DRenderObject *Container;
    void *User_Data;
    W3DRenderHook *RenderHook;
    mutable Graphics::RenderObjectState NativeState;
};

bool Extract_Ordered_Draw(W3DRenderObject &object, void *context);
