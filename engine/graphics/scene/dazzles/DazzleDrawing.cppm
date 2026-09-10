module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Dazzles.Drawing;
import Assets.Dazzles;
import Assets.Math;
import Graphics.RHI;
import Graphics.Materials.State;
import Graphics.Scene.Dazzles.State;
import Graphics.Scene.Dazzles.Resources;
import Graphics.Scene.Surfaces.Geometry;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.MaterialSubmission;

namespace Graphics {
export MaterialState Dazzle_Material(DazzleImage image) {
    MaterialState shader;
    shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
    shader.Set_Depth_Mask(MaterialState::DEPTH_WRITE_DISABLE);
    shader.Set_Depth_Compare(image == DazzleImage::Halo ? MaterialState::PASS_LEQUAL : MaterialState::PASS_ALWAYS);
    shader.Set_Dst_Blend_Func(MaterialState::DSTBLEND_ONE);
    shader.Set_Src_Blend_Func(MaterialState::SRCBLEND_ONE);
    shader.Set_Fog_Func(MaterialState::FOG_DISABLE);
    shader.Set_Primary_Gradient(MaterialState::GRADIENT_MODULATE);
    shader.Set_Texturing(MaterialState::TEXTURING_ENABLE);
    return shader;
}

export class DazzleDrawing final {
public:
    void Prepare(const Assets::DazzleDefinition& definition, const DazzleState& state,
        std::span<const Assets::LensFlareSprite> flares, unsigned width, unsigned height)
    {
        assert(width != 0 && height != 0);
        m_vertices.clear();
        m_counts = {};
        const float x_scale = height > width ? float(height)/width : 1;
        const float y_scale = width > height ? float(width)/height : 1;
        const auto color = [](const auto& instance, const auto& authored, float intensity) {
            return Assets::Color_From_ARGB(Assets::Color_To_ARGB({instance[0]*authored[0]*intensity,
                instance[1]*authored[1]*intensity, instance[2]*authored[2]*intensity, 1})).To_Array();
        };
        if (state.intensity > 0) {
            Append_Quad(state.screen_position, definition.scale[0]*state.scale*x_scale*state.size,
                definition.scale[1]*state.scale*y_scale*state.size, {0,0,1,1}, color(state.color, definition.color, state.intensity));
            m_counts[0] = 4;
        }
        if (state.halo != 0) {
            // Halo dimensions are independent of the object's scale.
            Append_Quad(state.screen_position, definition.halo_scale[0]*x_scale, definition.halo_scale[1]*y_scale,
                {0,0,1,1}, color(state.halo_color, definition.halo_color, state.halo));
            m_counts[1] = 4;
        }
        if (state.intensity > 0) {
            const auto& position = state.screen_position;
            const float distance_multiplier = std::sqrt(position[0]*position[0] + position[1]*position[1]) + 1;
            for (const auto& flare : flares) {
                const float size = flare.size*distance_multiplier;
                Append_Quad({flare.location*position[0], flare.location*position[1], position[2]}, size*x_scale,
                    size*y_scale, flare.uv, color(std::array{1.f,1.f,1.f}, flare.color, state.intensity*state.lens_flare_intensity));
            }
            m_counts[2] = flares.size()*4;
        }
        const auto count = (std::max)({m_counts[0], m_counts[1], m_counts[2]});
        assert(count <= (std::numeric_limits<std::uint32_t>::max)());
        m_indices.resize(count/4*6);
        for (std::size_t i = 0; i < count/4; ++i) {
            const auto vertex = static_cast<std::uint32_t>(i*4);
            const std::array triangle_indices{vertex, vertex+1, vertex+2, vertex, vertex+2, vertex+3};
            std::copy(triangle_indices.begin(), triangle_indices.end(), m_indices.begin()+i*6);
        }
    }
    std::span<const SurfaceVertex> Vertices(DazzleImage image) const {
        const auto index = static_cast<unsigned>(image);
        const auto offset = index == 0 ? 0 : index == 1 ? m_counts[0] : m_counts[0]+m_counts[1];
        return std::span<const SurfaceVertex>(m_vertices).subspan(offset, m_counts[index]);
    }
    std::span<const std::uint32_t> Indices(DazzleImage image) const {
        return std::span<const std::uint32_t>(m_indices).first(m_counts[static_cast<unsigned>(image)]/4*6);
    }
    template<class GetTexture, class Resolve>
    bool Draw(Device& device, PropRenderer& renderer, PropSubmission& submission,
        GetTexture&& texture, Resolve&& resolve, const PropMaterialDrawContext& context)
    {
        PropParameters parameters;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        bool success = true;
        // These are immediate authored layers, even when another scene uses
        // a transparent sorting view. Preserve halo, glare, then flare order.
        auto immediate = context;
        immediate.sorting_depth.reset();
        for (const auto image : {DazzleImage::Halo, DazzleImage::Glare, DazzleImage::LensFlare}) {
            const auto vertices = Vertices(image);
            if (vertices.empty()) continue;
            const auto source = texture(image);
            using Source = decltype(texture(image));
            if (!Submit_Prop_Surface(device, renderer, submission, vertices, Indices(image), Dazzle_Material(image),
                std::array<Source, 2>{source, {}}, resolve, parameters, immediate)) success = false;
        }
        return success;
    }
private:
    void Append_Quad(const std::array<float, 3>& position, float x, float y,
        const std::array<float, 4>& uv, const std::array<float, 4>& color) {
        const std::array<std::array<float, 2>, 4> offsets{{{x,-y},{x,y},{-x,y},{-x,-y}}};
        const std::array<std::array<float, 2>, 4> coordinates{{{uv[0],uv[1]},{uv[2],uv[1]},{uv[2],uv[3]},{uv[0],uv[3]}}};
        for (unsigned i = 0; i < 4; ++i) {
            SurfaceVertex vertex;
            vertex.position = {position[0]+offsets[i][0], position[1]+offsets[i][1], position[2]};
            vertex.uv = coordinates[i];
            vertex.color = color;
            m_vertices.push_back(vertex);
        }
    }
    std::vector<SurfaceVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
    std::array<std::size_t, 3> m_counts{};
};
}
