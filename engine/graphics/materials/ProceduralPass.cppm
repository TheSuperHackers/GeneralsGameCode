module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

export module Graphics.Materials.ProceduralPass;

import Graphics.Materials.MeshMaterial;
import Graphics.Materials.State;

namespace Graphics
{

// A procedural material pass owns its resource references and exposes only
// borrowed draw inputs while a renderer extracts a description. Cull bounds
// remain borrowed so a caller can update a live projection volume in place.
export template<class TextureOwner, class CullBounds>
struct ProceduralMaterialPass final
{
    using TexturePointer = decltype(std::to_address(std::declval<const TextureOwner&>()));

    struct Description final
    {
        MaterialState shader;
        MeshMaterial* material = nullptr;
        std::array<TexturePointer, 2> textures{};
        bool world_coordinates = false;
        std::array<float, 16> world_texture_transform{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
        std::uint8_t color_write_mask = 15;
    };

    using PrepareFunction = bool (*)(const ProceduralMaterialPass&, Description&);

    // MaterialState's default constructor represents its global default. The
    // retired pass explicitly started with zero bits, so retain that value for
    // an unconfigured native record.
    MaterialState shader{0};
    std::shared_ptr<MeshMaterial> material;
    std::array<TextureOwner, 8> textures{};
    bool enabled_on_translucent = true;
    const CullBounds* cull_bounds = nullptr;
    PrepareFunction prepare = nullptr;

    bool Describe(Description& description) const
    {
        description = Description{};
        if (prepare != nullptr) return prepare(*this, description);
        description.shader = shader;
        description.material = material.get();
        description.textures[0] = std::to_address(textures[0]);
        description.textures[1] = std::to_address(textures[1]);
        return true;
    }
};

}
