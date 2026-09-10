module;
#include "../../profiling/Tracy.h"
#include <array>
#include <cstring>
#include <span>

export module Graphics.Scene.Lighting.Environment;
export import Graphics.RHI;

namespace Graphics
{
// Frame input. The scene retains the cloud texture until its last draw and
// clears the input before releasing it. Height participates in the projection.
export struct EnvironmentLightingParameters final
{
    std::array<float,4> cloud_multiplier{};
    std::array<float,4> cloud_offset_strength{};
    std::array<float,4> ambient{};
    std::array<float,4> clip_plane{};
    std::array<std::array<float,16>,4> shadow_view_projection{};
    std::array<float,4> shadow_splits{};
    std::array<float,4> shadow_view_depth{};
    // Cascade count, constant depth bias, slope bias, cascade blend fraction.
    std::array<float,4> shadow_options{0,0.0005f,1.5f,0.1f};
};
static_assert(sizeof(EnvironmentLightingParameters) == 368);

export struct EnvironmentLightingState final
{
    EnvironmentLightingParameters parameters;
    RHITextureHandle cloud_texture{};
    std::array<RHITextureHandle,4> shadow_textures{};
};

export EnvironmentLightingState& Get_Environment_Lighting() noexcept
{
    static EnvironmentLightingState state;
    return state;
}

// Each renderer owns its constant buffer. Shared scene inputs do not retain a
// device pointer or own imported textures across renderer/device restarts.
export class EnvironmentLightingBinding final
{
public:
    bool Initialize(Device& device)
    {
        m_parameters_uploaded = false;
        m_constants = device.Create_Buffer({sizeof(EnvironmentLightingParameters), RHIBufferUsage::Constant});
        return m_constants.Is_Valid();
    }

    void Shutdown(Device& device) noexcept
    {
        if (m_constants.Is_Valid()) device.Destroy_Buffer(m_constants);
        m_constants = {};
        m_parameters_uploaded = false;
    }

    bool Bind(Device& device, CommandList& commands)
    {
        std::array<RHIBindlessResource,6> resources{};
        unsigned count=0;
        return Prepare_Resources(device, resources, count)
            && commands.Set_Bindless_Resources(std::span(resources.data(),count));
    }

    bool Prepare_Resources(Device& device, std::span<RHIBindlessResource> resources, unsigned& count)
    {
        count=0;
        if (resources.size()<6) return false;
        for (auto& resource : resources.first(6)) resource={};
        GRAPHICS_PROFILE_SCOPE("Graphics.Environment.Bind");
        const auto& state = Get_Environment_Lighting();
        auto parameters = state.parameters;
        if (!state.cloud_texture.Is_Valid()) parameters.cloud_offset_strength[3] = 0;
        const auto cascade_count = static_cast<unsigned>(parameters.shadow_options[0]);
        if (cascade_count > state.shadow_textures.size()) return false;
        for (unsigned cascade=0;cascade<cascade_count;++cascade)
            if (!state.shadow_textures[cascade].Is_Valid()) return false;
        // This binding exclusively owns its buffer. Keep the uploaded bytes
        // across draws, but still resolve and bind current texture generations.
        if (!m_parameters_uploaded || std::memcmp(&m_parameters,&parameters,sizeof(parameters)) != 0) {
            if (!device.Update_Buffer(m_constants, 0, std::as_bytes(std::span(&parameters,1)))) return false;
            m_parameters = parameters;
            m_parameters_uploaded = true;
        }
        resources[0].type = RHIResourceType::Material;
        resources[0].constant_buffer_slot = 7;
        resources[0].buffer = m_constants;
        count = 1;
        if (parameters.cloud_offset_strength[3] > 0) {
            resources[count].type = RHIResourceType::Texture;
            resources[count].index = ResourceIndex{15,1};
            resources[count++].texture = state.cloud_texture;
        }
        for (unsigned cascade=0;cascade<cascade_count;++cascade) {
            resources[count].type = RHIResourceType::Texture;
            resources[count].index = ResourceIndex{11+cascade,1};
            resources[count++].texture = state.shadow_textures[cascade];
        }
        return true;
    }

private:
    RHIBufferHandle m_constants{};
    EnvironmentLightingParameters m_parameters{};
    bool m_parameters_uploaded = false;
};
}
