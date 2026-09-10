module;
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
#include <vector>
export module Graphics.Scene.Dazzles.Resources;
import Assets.Dazzles;

namespace Graphics {
export inline constexpr unsigned Invalid_Dazzle_Type = (std::numeric_limits<unsigned>::max)();
export enum class DazzleImage { Glare, Halo, LensFlare };
export template<class TextureOwner>
class DazzleResources final {
    struct Dazzle final {
        Assets::DazzleDefinition definition;
        std::array<TextureOwner, 2> textures;
        unsigned lens_flare = Invalid_Dazzle_Type;
    };
    struct LensFlare final { Assets::LensFlareDefinition definition; TextureOwner texture; };
public:
    void Initialize(Assets::DazzleDefinitions definitions) {
        Clear();
        m_lens_flares.reserve(definitions.lens_flares.size());
        for (auto& definition : definitions.lens_flares) m_lens_flares.push_back({std::move(definition), {}});
        m_dazzles.reserve(definitions.dazzles.size());
        for (auto& definition : definitions.dazzles) {
            unsigned lens_flare = Invalid_Dazzle_Type;
            for (unsigned i = 0; i < m_lens_flares.size(); ++i)
                if (m_lens_flares[i].definition.name == definition.lens_flare) { lens_flare = i; break; }
            m_dazzles.push_back({std::move(definition), {}, lens_flare});
        }
    }
    void Clear() { m_dazzles.clear(); m_lens_flares.clear(); }
    std::size_t Size() const noexcept { return m_dazzles.size(); }
    unsigned Find(std::string_view name) const noexcept {
        for (unsigned i = 0; i < m_dazzles.size(); ++i) if (m_dazzles[i].definition.name == name) return i;
        return Invalid_Dazzle_Type;
    }
    const Assets::DazzleDefinition& Definition(unsigned type) const {
        assert(type < m_dazzles.size());
        return m_dazzles[type].definition;
    }
    std::span<const Assets::LensFlareSprite> Sprites(unsigned type) const {
        assert(type < m_dazzles.size());
        const unsigned index = m_dazzles[type].lens_flare;
        return index == Invalid_Dazzle_Type ? std::span<const Assets::LensFlareSprite>{} : m_lens_flares[index].definition.sprites;
    }
    template<class Acquire>
    auto Texture(unsigned type, DazzleImage image, Acquire&& acquire) {
        assert(type < m_dazzles.size());
        auto& dazzle = m_dazzles[type];
        if (image == DazzleImage::LensFlare) {
            assert(dazzle.lens_flare != Invalid_Dazzle_Type);
            auto& flare = m_lens_flares[dazzle.lens_flare];
            if (!flare.texture) flare.texture = acquire(flare.definition.texture);
            return std::to_address(flare.texture);
        }
        auto& texture = dazzle.textures[image == DazzleImage::Glare ? 0 : 1];
        if (!texture) texture = acquire(image == DazzleImage::Glare ? dazzle.definition.primary_texture : dazzle.definition.halo_texture);
        return std::to_address(texture);
    }
private:
    std::vector<Dazzle> m_dazzles;
    std::vector<LensFlare> m_lens_flares;
};
}
