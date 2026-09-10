module;
#include <array>
export module Graphics.Materials.TextureCoordinates;

namespace Graphics
{
export enum class TextureCoordinateSource : unsigned
{
    UV, CameraPosition, CameraNormal, CameraReflection, WorldPosition
};

export struct TextureCoordinateMode final
{
    TextureCoordinateSource source = TextureCoordinateSource::UV;
    bool projected = false;
};

export enum class TextureProjection : unsigned
{
    Orthographic, Perspective, DepthGradient, NormalGradient
};

// Row-major matrices multiply column vectors. UV inputs are (u,v,0,1);
// all generated coordinate sources carry an implicit homogeneous one.
export constexpr std::array<float,16> Make_Affine_Texture_Transform(
    const std::array<float,6>& rows) noexcept
{
    return {rows[0],rows[1],0,rows[2], rows[3],rows[4],0,rows[5],
        0,0,1,0, 0,0,0,1};
}

export constexpr TextureCoordinateMode Projection_Texture_Coordinates(TextureProjection projection) noexcept
{
    return {projection == TextureProjection::NormalGradient
        ? TextureCoordinateSource::CameraNormal : TextureCoordinateSource::CameraPosition,
        projection == TextureProjection::Perspective};
}

export constexpr std::array<float,16> Make_Projection_Texture_Transform(
    const std::array<float,16>& view_to_texture, TextureProjection projection,
    float gradient_u, const std::array<float,3>& projection_normal) noexcept
{
    if (projection == TextureProjection::Orthographic || projection == TextureProjection::Perspective)
        return view_to_texture;
    std::array<float,16> result{0,0,0,gradient_u, 0,0,0,0, 0,0,1,0, 0,0,0,1};
    if (projection == TextureProjection::DepthGradient) {
        for (unsigned column=0;column<4;++column) result[4+column]=view_to_texture[8+column];
    } else {
        for (unsigned column=0;column<3;++column) result[4+column]=projection_normal[column];
    }
    return result;
}

// Compose a local UV animation after a projection. The projection's fourth
// component supplies the affine translation, including perspective division.
export constexpr std::array<float,16> Compose_Texture_Projection(
    const std::array<float,16>& view_to_texture,
    const std::array<float,16>& texture_transform) noexcept
{
    std::array<float,16> result{};
    for (unsigned row=0;row<4;++row)
        for (unsigned column=0;column<4;++column)
            for (unsigned inner=0;inner<4;++inner)
                result[row*4+column] += texture_transform[row*4+inner]*view_to_texture[inner*4+column];
    return result;
}
}
