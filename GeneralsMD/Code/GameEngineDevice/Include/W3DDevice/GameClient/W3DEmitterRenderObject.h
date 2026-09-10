#pragma once

#include <memory>
#include <optional>
#include <string>
import Assets.Particles;
import Graphics.Materials.State;
import Graphics.Scene.Particles.EmitterEmission;
import Graphics.Scene.Particles.EmitterKinematics;
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "WWLib/ref_ptr.h"
#include "WWMath/v3_rnd.h"

class W3DTextureHandle;
class W3DEmitterParticles;

// Scene ownership and visibility of an emitter are independent of its particles.
class W3DEmitterRenderObject final : public W3DRenderObject
{
public:
    W3DEmitterRenderObject(const Assets::EmitterAssetDesc &description, W3DTextureHandle *texture,
        Graphics::MaterialState shader, std::unique_ptr<Vector3Randomizer> position,
        std::unique_ptr<Vector3Randomizer> velocity);
    W3DEmitterRenderObject(const W3DEmitterRenderObject &source);
    ~W3DEmitterRenderObject() override;
    W3DRenderObject *Clone() const override;
    int Class_ID() const override { return CLASSID_PARTICLEEMITTER; }
    const char *Get_Name() const override { return m_name ? m_name->c_str() : nullptr; }
    void Set_Name(const char *name) override;
    void Notify_Added(W3DScene *scene) override;
    void Notify_Removed(W3DScene *scene) override;
    void Render(W3DRenderContext &) override {}
    void Restart() override { Start(); }
    void Scale(float scale) override;
    void On_Frame_Update() override;
    void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    void Set_Hidden(int value) override { W3DRenderObject::Set_Hidden(value); Update_Visibility(); }
    void Set_Visible(int value) override { W3DRenderObject::Set_Visible(value); Update_Visibility(); }
    void Set_Animation_Hidden(int value) override { W3DRenderObject::Set_Animation_Hidden(value); Update_Visibility(); }
    void Set_Force_Visible(int value) override { W3DRenderObject::Set_Force_Visible(value); Update_Visibility(); }
    void Set_LOD_Bias(float bias) override;
    bool Is_Complete() override { return m_emission.Complete(); }
    void Add_Dependencies_To_List(DynamicVectorClass<StringClass> &files, bool textures_only) override;

    void Start();
    void Stop() noexcept { m_active = false; }
    void Reset();
    bool Is_Stopped() const noexcept { return !m_active; }
    void Set_Invisible(bool value) noexcept { m_invisible = value; }
    bool Is_Invisible() const noexcept { return m_invisible; }
    void Emit(Graphics::EmitterKinematics &particles);
    std::uint8_t Group() const noexcept { return m_emission.Group(); }

private:
    void Update_Cached_Bounding_Volumes() const override;
    Graphics::EmitterTransform Current_Transform() const;
    void Update_Visibility();
    Graphics::EmitterEmission m_emission;
    std::unique_ptr<Vector3Randomizer> m_position_randomizer;
    std::unique_ptr<Vector3Randomizer> m_velocity_randomizer;
    RefCountPtr<W3DEmitterParticles> m_particles;
    std::optional<std::string> m_name{"ParticleEmitter"};
    bool m_active = false;
    bool m_first = true;
    bool m_in_scene = false;
    bool m_invisible = false;
};
