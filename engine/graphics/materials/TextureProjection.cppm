module;
#include <array>
#include <cmath>
export module Graphics.Materials.TextureProjection;
import Graphics.Materials.TextureCoordinates;

namespace Graphics {
// Projection configuration is independent of an animation clock or material
// stage. The owner rebuilds pixel coordinates when its projector changes.
export class TextureProjectionState final {
public:
    TextureProjection type = TextureProjection::Orthographic;
    float gradient_u = 0.5f;
    bool invert_depth = false;

    void Set_Texture_Transform(const std::array<float,16>& transform, float texture_size) noexcept
    {
        m_view_to_texture = transform;
        const float scale = 0.5f * (texture_size - 2.0f) / texture_size;
        const float depth_scale = invert_depth ? -0.5f : 0.5f;
        for (unsigned column=0; column<4; ++column) {
            m_view_to_pixel[column] = scale*transform[column] + scale*transform[12+column];
            m_view_to_pixel[4+column] = -scale*transform[4+column] + scale*transform[12+column];
            m_view_to_pixel[8+column] = depth_scale*transform[8+column] + 0.5f*transform[12+column];
            m_view_to_pixel[12+column] = transform[12+column];
        }
        m_normal = {-transform[8],-transform[9],-transform[10]};
        const float length_squared = m_normal[0]*m_normal[0] + m_normal[1]*m_normal[1] + m_normal[2]*m_normal[2];
        if (length_squared != 0) {
            const float inverse_length = 1.0f/std::sqrt(length_squared);
            for (auto& component : m_normal) component *= inverse_length;
        }
    }

    const std::array<float,16>& Get_Texture_Transform() const noexcept { return m_view_to_texture; }
    std::array<float,16> Transform() const noexcept
    {
        return Make_Projection_Texture_Transform(m_view_to_pixel,type,gradient_u,m_normal);
    }
    TextureCoordinateMode Coordinates() const noexcept { return Projection_Texture_Coordinates(type); }

    // Return homogeneous S,T,Q; perspective division belongs to the consumer.
    std::array<float,3> Compute_Texture_Coordinate(const std::array<float,3>& point) const noexcept
    {
        std::array<float,3> result{};
        constexpr std::array<unsigned,3> rows{0,4,12};
        for (unsigned i=0; i<rows.size(); ++i) {
            const auto row=rows[i];
            result[i]=m_view_to_pixel[row]*point[0] + m_view_to_pixel[row+1]*point[1]
                + m_view_to_pixel[row+2]*point[2] + m_view_to_pixel[row+3];
        }
        return result;
    }
private:
    std::array<float,16> m_view_to_texture{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    std::array<float,16> m_view_to_pixel=m_view_to_texture;
    std::array<float,3> m_normal{};
};
}
