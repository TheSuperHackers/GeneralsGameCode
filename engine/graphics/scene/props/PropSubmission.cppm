module;
#include "../../profiling/Tracy.h"
#include <array>
#include <span>
#include <vector>
export module Graphics.Scene.Props.Submission;
export import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.MaterialPassQueue;
import Graphics.Scene.Props.TransparentGeometry;
import Graphics.Scene.Shadows.DirectionalRenderer;

namespace Graphics {
export enum class PropDrawPhase { Immediate, Batchable, Material, Decal, Transparent, Shadow };

export struct PropDrawSettings final {
    bool lighting = true;
    bool force_multiply = false;
};

// Scene submission owns mesh versions, deferred ordering, and transferred
// texture references. Adapters translate state; they do not own draw queues.
export class PropSubmission final {
public:
    PropSubmission() = default;
    PropSubmission(const PropSubmission&) = delete;
    PropSubmission& operator=(const PropSubmission&) = delete;

    void Initialize(Device& device,PropRenderer& renderer,DirectionalShadowRenderer& shadows)
    {
        Shutdown();
        m_device = &device; m_renderer = &renderer; m_shadows = &shadows;
    }
    void Shutdown() noexcept
    {
        Clear();
        m_batch_depth=0;
        m_device = nullptr; m_renderer = nullptr; m_shadows = nullptr;
    }
    void Clear() noexcept
    {
        m_opaque.Clear(); Release(m_batch_textures);
        Clear_Shadows();
        if (m_renderer != nullptr) m_transparent.Clear(*m_renderer);
        m_materials.Clear(); m_decals.Clear();
        Release(m_frame_textures);
    }
    void Clear_Shadows() noexcept
    {
        if (m_shadows != nullptr) m_shadows->Clear_Casters();
        Release(m_shadow_textures);
    }
    void Begin_Batching(RHIViewport viewport = {}) noexcept {
        if (m_batch_depth==0) m_opaque.Set_Viewport(viewport);
        ++m_batch_depth;
    }
    bool Flush_Batches() {
        if (m_opaque.Empty()) return true;
        const bool success = m_device && m_opaque.Flush(m_device->Immediate_Command_List());
        if (!success) m_opaque.Clear();
        Release(m_batch_textures);
        return success;
    }
    bool End_Batching() {
        const bool success = Flush_Batches();
        if (m_batch_depth) --m_batch_depth;
        return success;
    }
    void Begin_Decal_Group() { m_decals.Begin_Group(); }

    // On success, texture handles transfer to this submission. On failure the
    // caller still owns them. Geometry is borrowed for immediate draws and
    // retained through drawing/cancellation for every deferred phase.
    bool Submit(PropMeshHandle mesh,PropStyle style,const PropParameters& parameters,
        std::span<const RHITextureHandle> textures,PropDrawPhase phase,
        const std::array<float,4>& camera_depth = {}, PropInstanceHandle instance = {})
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.Submit");
        if (m_device == nullptr || m_renderer == nullptr || textures.size() > PropTextureCount) return false;
        const bool batched = phase == PropDrawPhase::Batchable && m_batch_depth != 0
            && MaterialPassQueue::Can_Batch_Opaque(style);
        bool submitted = false;
        switch (phase) {
        case PropDrawPhase::Batchable:
            if (batched) {
                submitted = m_opaque.Submit(*m_renderer,mesh,style,parameters,textures,instance);
                break;
            }
            [[fallthrough]];
        case PropDrawPhase::Immediate:
            if (!Flush_Batches()) return false;
            submitted = instance.Is_Valid()
                ? m_renderer->Draw_Record(m_device->Immediate_Command_List(),mesh,style,parameters,textures,instance)
                : m_renderer->Draw(m_device->Immediate_Command_List(),mesh,style,parameters,textures);
            break;
        case PropDrawPhase::Material:
            submitted = m_materials.Submit(*m_renderer,mesh,style,parameters,textures,instance);
            break;
        case PropDrawPhase::Decal:
            style.depth_bias = 8;
            submitted = m_decals.Submit(*m_renderer,mesh,style,parameters,textures,instance);
            break;
        case PropDrawPhase::Transparent:
            submitted = m_transparent.Submit(*m_renderer,mesh,style,parameters,textures,camera_depth,instance);
            break;
        case PropDrawPhase::Shadow:
            submitted = m_shadows->Add_Caster(*m_renderer,mesh,parameters,textures,style,instance);
            break;
        }
        if (!submitted) return false;
        for (const auto texture : textures) if (texture.Is_Valid()) {
            if (batched) m_batch_textures.push_back(texture);
            else if (phase == PropDrawPhase::Immediate || phase == PropDrawPhase::Batchable) m_device->Destroy_Texture(texture);
            else if (phase == PropDrawPhase::Shadow) m_shadow_textures.push_back(texture);
            else m_frame_textures.push_back(texture);
        }
        return true;
    }
    bool Flush_Materials()
    {
        if (m_device == nullptr) return false;
        auto& commands = m_device->Immediate_Command_List();
        const bool opaque = Flush_Batches();
        const bool decals = m_decals.Flush_Groups_Reversed(commands);
        const bool materials = m_materials.Flush(commands);
        return opaque && decals && materials;
    }
    bool Flush_Transparent()
    {
        if (m_device == nullptr) return false;
        const bool materials = Flush_Materials();
        const bool transparent = m_transparent.Flush(*m_renderer,m_device->Immediate_Command_List());
        Clear();
        return materials && transparent;
    }
private:
    void Release(std::vector<RHITextureHandle>& textures) noexcept
    {
        if (m_device != nullptr)
            for (const auto texture : textures) m_device->Destroy_Texture(texture);
        textures.clear();
    }
    Device* m_device = nullptr;
    PropRenderer* m_renderer = nullptr;
    DirectionalShadowRenderer* m_shadows = nullptr;
    MaterialPassQueue m_opaque;
    std::vector<RHITextureHandle> m_batch_textures;
    unsigned m_batch_depth=0;
    MaterialPassQueue m_materials;
    MaterialPassQueue m_decals;
    TransparentGeometry m_transparent;
    std::vector<RHITextureHandle> m_frame_textures;
    std::vector<RHITextureHandle> m_shadow_textures;
};
// A scene owns each batching interval and flushes before changing targets or
// drawing a different feature. Nested traversals keep their parent's interval.
export class PropBatchScope final {
public:
    explicit PropBatchScope(PropSubmission& submission,RHIViewport viewport = {}) : m_submission(submission) {
        m_submission.Begin_Batching(viewport);
    }
    ~PropBatchScope() { m_submission.End_Batching(); }
    PropBatchScope(const PropBatchScope&) = delete;
    PropBatchScope& operator=(const PropBatchScope&) = delete;
private:
    PropSubmission& m_submission;
};
namespace {
PropSubmission g_submission;
PropDrawSettings g_settings;
}
export PropSubmission& Get_Prop_Submission() noexcept { return g_submission; }
export PropDrawSettings& Get_Prop_Draw_Settings() noexcept { return g_settings; }
}
