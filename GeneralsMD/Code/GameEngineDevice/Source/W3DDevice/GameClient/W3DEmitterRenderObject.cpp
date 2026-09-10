import Graphics.Frame.RenderClock;
#include "W3DDevice/GameClient/W3DEmitterRenderObject.h"
#include "W3DDevice/GameClient/W3DEmitterParticles.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"

#include "WWMath/quat.h"

W3DEmitterRenderObject::W3DEmitterRenderObject(const Assets::EmitterAssetDesc &description,
    W3DTextureHandle *texture, Graphics::MaterialState shader,
    std::unique_ptr<Vector3Randomizer> position, std::unique_ptr<Vector3Randomizer> velocity)
    : m_emission(description), m_position_randomizer(std::move(position)),
      m_velocity_randomizer(std::move(velocity))
{
    if (m_velocity_randomizer)
        m_velocity_randomizer->Scale(0.001f);
    m_particles.Assign_No_Add_Ref(new W3DEmitterParticles(description, texture, shader, this));
}

W3DEmitterRenderObject::W3DEmitterRenderObject(const W3DEmitterRenderObject &source)
    : W3DRenderObject(source), m_emission(source.m_emission),
      m_position_randomizer(source.m_position_randomizer ? source.m_position_randomizer->Clone() : nullptr),
      m_velocity_randomizer(source.m_velocity_randomizer ? source.m_velocity_randomizer->Clone() : nullptr),
      m_name(source.m_name), m_active(true), m_invisible(source.m_invisible)
{
    m_emission.Prepare_Clone();
    m_particles.Assign_No_Add_Ref(new W3DEmitterParticles(*source.m_particles.Peek(), this));
}

W3DEmitterRenderObject::~W3DEmitterRenderObject()
{
    m_particles->Emitter_Removed();
}

W3DRenderObject *W3DEmitterRenderObject::Clone() const { return new W3DEmitterRenderObject(*this); }

void W3DEmitterRenderObject::Set_Name(const char *name)
{
    if (name)
        m_name = name;
    else
        m_name.reset();
}

Graphics::EmitterTransform W3DEmitterRenderObject::Current_Transform() const
{
    const auto &transform = Get_Transform();
    const Quaternion rotation = Build_Quaternion(transform);
    const Vector3 origin = transform.Get_Translation();
    return {{rotation.X, rotation.Y, rotation.Z, rotation.W}, {origin.X, origin.Y, origin.Z}};
}

void W3DEmitterRenderObject::Notify_Added(W3DScene *scene)
{
    W3DRenderObject::Notify_Added(scene);
    scene->Register(this, W3DScene::ON_FRAME_UPDATE);
    if (!m_first)
        m_active = true;
    m_in_scene = true;
}

void W3DEmitterRenderObject::Notify_Removed(W3DScene *scene)
{
    scene->Unregister(this, W3DScene::ON_FRAME_UPDATE);
    W3DRenderObject::Notify_Removed(scene);
    m_active = false;
    m_in_scene = false;
}

void W3DEmitterRenderObject::On_Frame_Update()
{
    if (m_active && !m_emission.Complete() && m_first) {
        if (!Is_In_Scene())
            return;
        m_particles->Add(Scene);
        m_emission.Set_Previous_Transform(Current_Transform());
        m_first = false;
    }
    if (m_emission.Complete() && Is_In_Scene())
        Scene->Register(this, W3DScene::RELEASE);
}

void W3DEmitterRenderObject::Start()
{
    // Transform evaluation can reenter visibility updates.
    m_active = true;
    m_emission.Set_Previous_Transform(Current_Transform());
    m_emission.Start();
}

void W3DEmitterRenderObject::Reset()
{
    m_active = true;
    m_emission.Set_Previous_Transform(Current_Transform());
    m_emission.Reset();
}

void W3DEmitterRenderObject::Emit(Graphics::EmitterKinematics &particles)
{
    const auto current = Current_Transform();
    if (!m_active || m_emission.Complete()) {
        m_emission.Set_Previous_Transform(current);
        return;
    }
    const auto sample = [](Vector3Randomizer *randomizer) {
        Vector3 value(0, 0, 0);
        if (randomizer)
            randomizer->Get_Vector(value);
        return std::array{value.X, value.Y, value.Z};
    };
    m_emission.Emit(particles, Graphics::Get_Render_Clock().Sync_Delta(), Graphics::Get_Render_Clock().Sync_Time(), current,
        [&] { return sample(m_position_randomizer.get()); },
        [&] { return sample(m_velocity_randomizer.get()); });
}

void W3DEmitterRenderObject::Update_Visibility()
{
    if (Is_Not_Hidden_At_All() && !m_invisible && !m_active && m_in_scene)
        Start();
    else if ((!Is_Not_Hidden_At_All() || m_invisible) && m_active)
        Stop();
}

void W3DEmitterRenderObject::Scale(float scale)
{
    if (m_position_randomizer)
        m_position_randomizer->Scale(scale);
    if (m_velocity_randomizer)
        m_velocity_randomizer->Scale(scale);
    m_emission.Scale(scale);
    m_particles->Scale(scale);
}

void W3DEmitterRenderObject::Set_LOD_Bias(float bias) { m_particles->Set_LOD_Bias(bias); }

void W3DEmitterRenderObject::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere.Init(Vector3(0, 0, 0), 0);
}

void W3DEmitterRenderObject::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box.Center.Set(0, 0, 0);
    box.Extent.Set(0, 0, 0);
}

void W3DEmitterRenderObject::Update_Cached_Bounding_Volumes() const
{
    CachedBoundingSphere.Init(Get_Position(), 0);
    CachedBoundingBox.Center = Get_Position();
    CachedBoundingBox.Extent.Set(0, 0, 0);
    Validate_Cached_Bounding_Volumes();
}

void W3DEmitterRenderObject::Add_Dependencies_To_List(DynamicVectorClass<StringClass> &files,
    bool textures_only)
{
    if (const auto *texture = m_particles->Peek_Texture())
        files.Add(texture->Get_Full_Path());
    W3DRenderObject::Add_Dependencies_To_List(files, textures_only);
}
