module;

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Scene.Models.Materials;

import Graphics.Materials.MeshMaterial;
import Graphics.Scene.Models.MaterialSlots;

namespace Graphics {

// Customization owns a collection independently of the mesh's material slots.
// Texture ownership is supplied by the resource layer using this collection.
export template<class TextureOwner>
struct ModelMaterials final
{
    std::vector<std::shared_ptr<MeshMaterial>> materials;
    std::vector<TextureOwner> textures;

    void Reset() noexcept
    {
        materials.clear();
        textures.clear();
    }

    // Each authored entry gets independent material and mapping state. Textures
    // remain shared until customization explicitly replaces a collection entry.
    ModelMaterials Clone(std::uint32_t now) const
    {
        ModelMaterials result;
        result.materials.reserve(materials.size());
        for (const auto& material : materials) {
            result.materials.push_back(material ? material->Clone(now) : nullptr);
        }
        result.textures = textures;
        return result;
    }
};

// An ordered identity mapping retains both sides while replacing mesh slots.
// Repeated source entries select the first destination, as in authored order.
export template<class Owner>
class MaterialResourceRemap final
{
    struct Entry final
    {
        Owner source;
        Owner destination;
    };

public:
    MaterialResourceRemap(std::span<const Owner> source, std::span<const Owner> destination)
    {
        assert(source.size() == destination.size());
        if (source.size() != destination.size()) {
            return;
        }
        m_entries.reserve(source.size());
        for (std::size_t index = 0; index < source.size(); ++index) {
            m_entries.push_back({source[index], destination[index]});
        }
    }

    bool Empty() const noexcept { return m_entries.empty(); }

    template<class Resource>
    const Owner& Find(const Resource* source)
    {
        if (!source) {
            return m_empty;
        }
        if (m_last < m_entries.size() && m_entries[m_last].source &&
            std::to_address(m_entries[m_last].source) == source) {
            return m_entries[m_last].destination;
        }
        for (std::size_t index = 0; index < m_entries.size(); ++index) {
            if (m_entries[index].source && std::to_address(m_entries[index].source) == source) {
                m_last = index;
                return m_entries[index].destination;
            }
        }
        assert(false && "Material resource is absent from the source collection");
        return m_empty;
    }

    // The active geometry prefix may be shorter than allocated slot storage.
    void Remap_Slots(const MaterialSlots<Owner>& source, MaterialSlots<Owner>& destination,
        std::size_t count)
    {
        assert(count <= source.Count() && count <= destination.Count());
        if (count > source.Count() || count > destination.Count()) {
            return;
        }
        for (std::size_t index = 0; index < count; ++index) {
            const auto& owner = *source.Peek(index);
            destination.Set(index, owner ? Find(std::to_address(owner)) : m_empty);
        }
    }

private:
    std::vector<Entry> m_entries;
    std::size_t m_last = 0;
    Owner m_empty{};
};

}
