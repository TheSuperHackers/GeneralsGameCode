module;
#include <array>
#include <bit>
#include <cstdint>
#include <memory>
#include <string>
export module Graphics.Materials.MeshMaterial;
import Graphics.Scene.Props.Material;
import Graphics.Materials.TextureMapping;

namespace Graphics {
// Compare authored material contents without relying on a hash collision or
// struct padding. Names do not affect material sharing. Animation identities
// do: two independently animated materials must not be coalesced.
export struct MeshMaterialKey final {
    std::array<std::uint32_t,14> colors{};
    std::array<PropColorSource,3> color_sources{};
    std::array<std::uint32_t,8> uv_sources{};
    std::array<const TextureMapping*,8> mappings{};
    std::uint32_t flags=0;
    std::uint32_t unique_group=0;
    bool lighting=false;
    bool operator==(const MeshMaterialKey&) const = default;
};

// A mesh owns references to this material; extraction borrows its parameters.
// Copying a mesh may share the material, while an explicit material clone
// restarts its animation independently at the supplied clock time.
export class MeshMaterial final {
public:
    MeshMaterial() { parameters.shininess=0; }
    MeshMaterial(const MeshMaterial&)=delete;
    MeshMaterial& operator=(const MeshMaterial&)=delete;

    PropMaterial parameters;
    std::string name;
    std::array<std::uint32_t,8> uv_sources{0,1,2,3,4,5,6,7};
    std::array<std::shared_ptr<TextureMapping>,8> mappings{};
    std::uint32_t flags=0;

    std::shared_ptr<MeshMaterial> Clone(std::uint32_t now) const
    {
        auto result=std::make_shared<MeshMaterial>();
        result->parameters=parameters;
        result->name=name;
        result->uv_sources=uv_sources;
        result->flags=flags;
        result->m_unique_group=m_unique_group;
        for (unsigned stage=0;stage<mappings.size();++stage)
            if (mappings[stage]) result->mappings[stage]=mappings[stage]->Clone(now);
        return result;
    }
    void Make_Unique() noexcept
    {
        // Material ownership and mutation stay on the scene's owner thread.
        static std::uint32_t next_group=1;
        m_unique_group=next_group++;
    }
    void Reset_Mappings(std::uint32_t now) noexcept
    {
        for (const auto& mapping:mappings) if (mapping) mapping->Reset(now);
    }
    bool Mappings_Need_Normals() const noexcept
    {
        for (const auto& mapping:mappings) if (mapping && mapping->Needs_Normals()) return true;
        return false;
    }
    bool Has_Animated_Mappings() const noexcept
    {
        for (const auto& mapping:mappings) if (mapping && mapping->Is_Time_Variant()) return true;
        return false;
    }
    MeshMaterialKey Content_Key() const noexcept
    {
        const std::array<float,14> colors{
            parameters.diffuse[0],parameters.diffuse[1],parameters.diffuse[2],parameters.opacity,
            parameters.ambient[0],parameters.ambient[1],parameters.ambient[2],
            parameters.specular[0],parameters.specular[1],parameters.specular[2],
            parameters.emissive[0],parameters.emissive[1],parameters.emissive[2],parameters.shininess};
        MeshMaterialKey key;
        key.colors=std::bit_cast<std::array<std::uint32_t,14>>(colors);
        key.color_sources={parameters.diffuse_source,parameters.ambient_source,parameters.emissive_source};
        key.uv_sources=uv_sources;
        key.flags=flags;
        key.lighting=parameters.lighting;
        key.unique_group=m_unique_group;
        for (unsigned stage=0;stage<mappings.size();++stage) key.mappings[stage]=mappings[stage].get();
        return key;
    }
private:
    std::uint32_t m_unique_group=0;
};
}
