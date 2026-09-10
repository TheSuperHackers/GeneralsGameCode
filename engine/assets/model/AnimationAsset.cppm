module;
#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>
export module Assets.Animations;
export import Assets.ModelRig;

namespace Assets {
// Prepared clip storage owns samples once. Bone bindings are indices, so copies
// and moves never retain pointers into another clip's channel allocation.
export class AnimationAsset final {
public:
    bool Initialize(ModelAnimationDesc description,std::uint32_t bone_count,std::string& error) {
        if(!bone_count || bone_count>std::uint32_t(std::numeric_limits<int>::max())
            || description.frame_count>std::uint32_t(std::numeric_limits<int>::max())) {
            error="invalid animation dimensions";return false;
        }
        ModelRigDesc validation;
        validation.animations.push_back(std::move(description));
        if(!Validate_Model_Rig(validation,error))return false;
        auto& clip=validation.animations.front();
        std::vector<std::array<std::uint32_t,8>> bindings(bone_count);
        for(auto& binding:bindings)binding.fill(Missing);
        for(std::size_t i=0;i<clip.channels.size();++i) {
            const auto& channel=clip.channels[i];
            // A clip may carry channels for a fuller skeleton export. Keep that
            // source data while binding only bones present in this instance.
            if(channel.bone<bone_count)
                bindings[channel.bone][static_cast<unsigned>(channel.component)]=static_cast<std::uint32_t>(i);
        }
        m_description=std::move(clip);m_bindings=std::move(bindings);
        error.clear();return true;
    }
    const ModelAnimationDesc& Description() const noexcept { return m_description; }
    std::uint32_t Bone_Count() const noexcept { return static_cast<std::uint32_t>(m_bindings.size()); }
    const ModelAnimationChannel* Channel(std::uint32_t bone,ModelChannelComponent component) const noexcept {
        const auto slot=static_cast<unsigned>(component);
        if(bone>=m_bindings.size() || slot>=8)return nullptr;
        const auto index=m_bindings[bone][slot];
        return index==Missing ? nullptr : &m_description.channels[index];
    }
private:
    static constexpr auto Missing=std::numeric_limits<std::uint32_t>::max();
    ModelAnimationDesc m_description;
    std::vector<std::array<std::uint32_t,8>> m_bindings;
};
}
