module;
#include "../../profiling/Tracy.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>
export module Graphics.Scene.Props.MaterialSubmission;
import Graphics.RHI;
import Graphics.Materials.State;
import Graphics.Resources.Textures.Sampling;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.MuzzleFlash;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.MaterialDrawState;
import Graphics.Scene.Surfaces.Geometry;

namespace Graphics {

export class PropMaterialPreparation;
export struct PropMaterialDrawOverrides final {
    float alpha_cutoff = -1;
    int color_write_mask = -1;
    bool deferred_pass = false;
    bool force_multiply = false;
    bool shadow_capture = false;
    bool decal_pass = false;
    MuzzleFlashDesignation muzzle_flash = MuzzleFlashDesignation::None;
    PropMeshHandle mesh{};
    PropMaterialPreparation* preparation = nullptr;
    PropInstanceOwner* instance = nullptr;
    PropSkinPaletteHandle skin{};
};

// Resolvers return a borrowed generation. Submission retains its own reference
// before another resolver or deferred draw can replace the source resource.
export struct PropMaterialTexture final {
    RHITextureHandle texture{};
    TextureSampling sampling{};
};

export struct PropMaterialDrawContext final {
    SceneDrawParameters scene{};
    bool reflection = false;
    bool batchable = false;
    std::uint32_t milliseconds = 0;
    std::optional<std::array<float, 4>> sorting_depth;
};

namespace MaterialSubmissionDetail {
PropStyle Prepare_Style(MaterialState shader, const PropMaterialDrawContext& context,
    const PropMaterialDrawOverrides& overrides,
    const std::array<std::optional<TextureSampling>, 2>& sampling,
    TextureSamplingSettings settings, PropParameters& parameters)
{
    auto style = Resolve_Prop_Material_State(shader, context.scene, context.reflection, parameters);
    if (overrides.alpha_cutoff >= 0 && shader.Get_Alpha_Test() == MaterialState::ALPHATEST_ENABLE)
        parameters.alpha_cutoff = overrides.alpha_cutoff;
    if (overrides.force_multiply) {
        style.source_blend = RHIBlendFactor::DestinationColor;
        style.destination_blend = RHIBlendFactor::SourceColor;
    }
    if (overrides.color_write_mask >= 0)
        style.color_write_mask = static_cast<std::uint8_t>(overrides.color_write_mask);
    for (unsigned stage = 0; stage < 2; ++stage)
        if (sampling[stage]) style.samplers[stage] = Resolve_Texture_Sampling(*sampling[stage], settings, stage == 0);
    return style;
}
}

// A model packet retains material and sampler preparation. Compare
// the complete inputs on use: scene scopes and authored sampling remain mutable.
// Texture generations are borrowed afresh and transferred by submission below.
export class PropMaterialPreparation final {
public:
    PropStyle Apply(MaterialState shader, const PropMaterialDrawContext& context,
        const PropMaterialDrawOverrides& overrides,
        const std::array<std::optional<TextureSampling>, 2>& sampling,
        TextureSamplingSettings settings, PropParameters& parameters)
    {
        const auto& scene = context.scene;
        const bool same_scene = m_valid && m_scene.fog.enabled == scene.fog.enabled
            && m_scene.fog.start == scene.fog.start && m_scene.fog.end == scene.fog.end
            && m_scene.fog.color == scene.fog.color && m_scene.stencil == scene.stencil
            && m_scene.depth_bias == scene.depth_bias && m_scene.color_write_mask == scene.color_write_mask
            && m_scene.wireframe == scene.wireframe;
        if (!m_valid || m_shader != shader || !same_scene || m_reflection != context.reflection
            || m_alpha_cutoff != overrides.alpha_cutoff || m_color_write_mask != overrides.color_write_mask
            || m_force_multiply != overrides.force_multiply
            || m_settings.mode != settings.mode || m_settings.anisotropy != settings.anisotropy
            || !Same_Sampling(m_sampling[0], sampling[0]) || !Same_Sampling(m_sampling[1], sampling[1])) {
            GRAPHICS_PROFILE_SCOPE("Graphics.Mesh.PrepareMaterialState");
            m_style = MaterialSubmissionDetail::Prepare_Style(shader, context, overrides, sampling, settings, parameters);
            m_constants = {parameters.primary_gradient, parameters.secondary_gradient,
                parameters.detail_color, parameters.detail_alpha, parameters.alpha_cutoff};
            m_fog_state = parameters.fog_state;
            m_fog_color = parameters.fog_color;
            m_shader = shader; m_scene = scene; m_reflection = context.reflection;
            m_alpha_cutoff = overrides.alpha_cutoff; m_color_write_mask = overrides.color_write_mask;
            m_force_multiply = overrides.force_multiply; m_sampling = sampling; m_settings = settings;
            m_valid = true;
            ++m_preparations;
        } else {
            parameters.primary_gradient = m_constants[0];
            parameters.secondary_gradient = m_constants[1];
            parameters.detail_color = m_constants[2];
            parameters.detail_alpha = m_constants[3];
            parameters.alpha_cutoff = m_constants[4];
            parameters.fog_state = m_fog_state;
            parameters.fog_color = m_fog_color;
        }
        return m_style;
    }
    std::uint64_t Preparation_Count() const noexcept { return m_preparations; }
private:
    static bool Same_Sampling(const std::optional<TextureSampling>& left,
        const std::optional<TextureSampling>& right) noexcept
    {
        if (left.has_value() != right.has_value()) return false;
        return !left || (left->minification == right->minification && left->magnification == right->magnification
            && left->mipmap == right->mipmap && left->address == right->address);
    }
    MaterialState m_shader{0};
    SceneDrawParameters m_scene;
    PropStyle m_style;
    std::array<float, 5> m_constants{};
    std::array<float, 4> m_fog_state{}, m_fog_color{};
    std::array<std::optional<TextureSampling>, 2> m_sampling;
    TextureSamplingSettings m_settings;
    float m_alpha_cutoff = -1;
    int m_color_write_mask = -1;
    bool m_force_multiply = false;
    bool m_reflection = false;
    bool m_valid = false;
    std::uint64_t m_preparations = 0;
};

namespace MaterialSubmissionDetail {
class TextureTransfers final {
public:
    explicit TextureTransfers(Device& device) : m_device(device) {}
    ~TextureTransfers() {
        if (!m_transferred)
            for (const auto texture : textures)
                if (texture.Is_Valid()) m_device.Destroy_Texture(texture);
    }
    bool Retain(unsigned stage, RHITextureHandle texture) {
        if (!m_device.Retain_Texture(texture)) return false;
        textures[stage] = texture;
        return true;
    }
    void Transfer() noexcept { m_transferred = true; }
    std::array<RHITextureHandle, 2> textures{};
private:
    Device& m_device;
    bool m_transferred = false;
};
}

// Sources and their resolver belong to the asset adapter. Material policy,
// sampling, temporary geometry and successful reference transfer live here.
export template<class Source, class Resolve>
bool Submit_Prop_Material(Device& device, PropRenderer& renderer, PropSubmission& submission,
    std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices,
    MaterialState shader, std::array<Source, 2> sources, Resolve&& resolve,
    PropParameters parameters, const PropMaterialDrawContext& context,
    PropMaterialDrawOverrides overrides = {})
{
    GRAPHICS_PROFILE_SCOPE("Graphics.Mesh.SubmitMaterial");
    if (vertices.empty() || indices.empty()) return false;
    const bool textured = shader.Get_Texturing() != MaterialState::TEXTURING_DISABLE;
    const bool muzzle = overrides.muzzle_flash != MuzzleFlashDesignation::None && sources[0] && textured;
    if (muzzle) sources[1] = sources[0];
    MaterialSubmissionDetail::TextureTransfers transfers(device);
    std::array<std::optional<TextureSampling>, 2> sampling;
    for (unsigned stage = 0; stage < 2; ++stage) {
        if (!sources[stage]) continue;
        // Sampling metadata still applies when the material disables texturing.
        const auto binding = resolve(sources[stage], textured);
        if (!binding) return false;
        sampling[stage] = binding->sampling;
        if (textured && !transfers.Retain(stage, binding->texture)) return false;
    }
    parameters.textured = transfers.textures[0].Is_Valid() ? 1.0f : 0.0f;
    parameters.secondary_texture = transfers.textures[1].Is_Valid() ? 1.0f : 0.0f;
    const auto settings = Get_Texture_Sampling_Settings();
    auto style = overrides.preparation
        ? overrides.preparation->Apply(shader, context, overrides, sampling, settings, parameters)
        : MaterialSubmissionDetail::Prepare_Style(shader, context, overrides, sampling, settings, parameters);
    if (muzzle && transfers.textures[0].Is_Valid())
        Prepare_Muzzle_Flash(parameters, style, overrides.muzzle_flash, context.milliseconds);
    auto phase = context.batchable ? PropDrawPhase::Batchable : PropDrawPhase::Immediate;
    if (overrides.shadow_capture) phase = PropDrawPhase::Shadow;
    else if (overrides.decal_pass) phase = PropDrawPhase::Decal;
    else if (overrides.deferred_pass) phase = PropDrawPhase::Material;
    else if (context.sorting_depth) phase = PropDrawPhase::Transparent;
    const auto mesh = overrides.mesh.Is_Valid() ? overrides.mesh : renderer.Create_Mesh(vertices, indices);
    const auto instance = overrides.instance ? overrides.instance->Update(renderer.Instances(),parameters,overrides.skin) : PropInstanceHandle{};
    const bool drawn = submission.Submit(mesh, style, parameters, transfers.textures, phase,
        context.sorting_depth.value_or(std::array<float, 4>{}),instance);
    if (!overrides.mesh.Is_Valid()) renderer.Destroy_Mesh(mesh);
    if (drawn) transfers.Transfer();
    return drawn;
}

export template<class Source, class Resolve>
bool Submit_Prop_Surface(Device& device, PropRenderer& renderer, PropSubmission& submission,
    std::span<const SurfaceVertex> source, std::span<const std::uint32_t> indices,
    MaterialState shader, std::array<Source, 2> textures, Resolve&& resolve,
    PropParameters parameters, const PropMaterialDrawContext& context)
{
    std::vector<PropVertex> vertices(source.size());
    for (std::size_t i = 0; i < source.size(); ++i) {
        vertices[i].position = source[i].position;
        vertices[i].uv = source[i].uv;
        vertices[i].color = source[i].color;
    }
    return Submit_Prop_Material(device, renderer, submission, vertices, indices,
        shader, textures, resolve, parameters, context);
}
}
