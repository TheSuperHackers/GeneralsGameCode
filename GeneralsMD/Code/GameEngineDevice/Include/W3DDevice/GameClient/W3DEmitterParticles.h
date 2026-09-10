#pragma once

import Assets.Particles;
import Graphics.Materials.State;
import Graphics.Scene.Particles.EmitterKinematics;
import Graphics.Scene.Particles.EmitterVisualState;
import Graphics.Scene.Particles.EmitterRenderer;
import Graphics.Scene.Particles.EmitterDetail;
#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "WWLib/ref_ptr.h"

class W3DTextureHandle;
class W3DEmitterRenderObject;

// Retains scene membership and texture lifetime while engine components own
// particle state, visual evaluation, detail selection, and drawing.
class W3DEmitterParticles final : public W3DRenderObject
{
public:
    W3DEmitterParticles(const Assets::EmitterAssetDesc &description, W3DTextureHandle *texture,
        Graphics::MaterialState shader, W3DEmitterRenderObject *emitter);
    W3DEmitterParticles(const W3DEmitterParticles &source, W3DEmitterRenderObject *emitter);
    ~W3DEmitterParticles() override;
    W3DRenderObject *Clone() const override;
    int Class_ID() const override { return CLASSID_PARTICLEBUFFER; }
    int Get_Num_Polys() const override { return static_cast<int>(m_detail.Cost()); }
    void Render(W3DRenderContext &info) override;
    void Scale(float scale) override;
    void On_Frame_Update() override;
    void Notify_Added(W3DScene *scene) override;
    void Notify_Removed(W3DScene *scene) override;
    void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
    void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;
    void Prepare_LOD(W3DCamera &camera) override;
    void Increment_LOD() override { m_detail.Increment(); }
    void Decrement_LOD() override { m_detail.Decrement(); }
    float Get_Cost() const override { return m_detail.Cost(); }
    float Get_Value() const override { return m_detail.Value(); }
    float Get_Post_Increment_Value() const override { return m_detail.Next_Value(); }
    void Set_LOD_Level(int level) override { m_detail.Set_Level(level); }
    int Get_LOD_Level() const override { return m_detail.Level(); }
    int Get_LOD_Count() const override { return m_detail.Count(); }
    void Set_LOD_Bias(float bias) override { m_detail.Set_Bias(bias); }
    int Calculate_Cost_Value_Arrays(float area, float *values, float *costs) const override;
    bool Is_Complete() override { return m_emitter == nullptr && m_kinematics.Count() == 0; }
    void Emitter_Removed() noexcept { m_emitter = nullptr; }
    W3DTextureHandle *Peek_Texture() const noexcept { return m_texture.Peek(); }

protected:
    void Update_Cached_Bounding_Volumes() const override;

private:
    void Submit(W3DRenderContext &info);
    void Update_Bounds();
    Graphics::EmitterKinematics m_kinematics;
    Graphics::EmitterVisualState m_visuals;
    Graphics::EmitterRenderer m_renderer;
    Graphics::EmitterDetail m_detail;
    RefCountPtr<W3DTextureHandle> m_texture;
    W3DEmitterRenderObject *m_emitter;
    AABoxClass m_bounds;
    bool m_bounds_dirty = true;
};
