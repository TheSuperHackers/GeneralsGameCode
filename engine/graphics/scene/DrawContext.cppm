module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

export module Graphics.Scene.DrawContext;

import Graphics.Scene.Lighting.Local;

namespace Graphics
{

export enum class DrawOverride : std::uint32_t
{
    Default = 0,
    ForceTwoSided = 1,
    ForceSorting = 2,
    AdditionalPassesOnly = 4,
    ShadowRendering = 8
};

export constexpr DrawOverride operator|(DrawOverride left, DrawOverride right) noexcept
{
    return static_cast<DrawOverride>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

// Traversal-local draw state. Passes retain their resources until popped or
// until the context is destroyed; lighting is borrowed for this traversal.
export template<class MaterialPass>
class SceneDrawContext
{
public:
    bool Push_Material_Pass(const std::shared_ptr<MaterialPass> &pass)
    {
        // Preserve the authored traversal limit of 31 additional passes.
        if (m_pass_count == m_passes.size()) {
            ++m_rejected_passes;
            return false;
        }
        m_passes[m_pass_count++] = pass;
        return true;
    }

    bool Pop_Material_Pass() noexcept
    {
        if (m_rejected_passes != 0) {
            --m_rejected_passes;
            return false;
        }
        if (m_pass_count == 0)
            return false;
        m_passes[--m_pass_count].reset();
        return true;
    }

    int Additional_Pass_Count() const noexcept { return static_cast<int>(m_pass_count); }
    MaterialPass *Peek_Additional_Pass(int index) const noexcept
    {
        return index >= 0 && static_cast<unsigned>(index) < m_pass_count
            ? m_passes[index].get() : nullptr;
    }

    bool Push_Override_Flags(DrawOverride flags) noexcept
    {
        if (m_override_level >= m_overrides.size() - 1) {
            ++m_rejected_overrides;
            return false;
        }
        m_overrides[++m_override_level] = flags;
        return true;
    }

    bool Pop_Override_Flags() noexcept
    {
        if (m_rejected_overrides != 0) {
            --m_rejected_overrides;
            return false;
        }
        if (m_override_level == 0)
            return false;
        --m_override_level;
        return true;
    }

    DrawOverride &Current_Override_Flags() noexcept { return m_overrides[m_override_level]; }
    DrawOverride Current_Override_Flags() const noexcept { return m_overrides[m_override_level]; }
    bool Has_Override(DrawOverride flag) const noexcept
    {
        return (static_cast<std::uint32_t>(Current_Override_Flags()) & static_cast<std::uint32_t>(flag)) != 0;
    }

    float fog_scale = 0;
    float fog_start = 0;
    float fog_end = 0;
    float alpha_override = 1;
    float pass_alpha_override = 1;
    float pass_emissive_override = 1;
    LocalLighting *light_environment = nullptr;

private:
    std::array<std::shared_ptr<MaterialPass>, 31> m_passes;
    unsigned m_pass_count = 0;
    unsigned m_rejected_passes = 0;
    std::array<DrawOverride, 32> m_overrides{};
    unsigned m_override_level = 0;
    unsigned m_rejected_overrides = 0;
};

}
