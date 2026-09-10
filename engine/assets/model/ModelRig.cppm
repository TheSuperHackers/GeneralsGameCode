module;
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_set>
#include <vector>
export module Assets.ModelRig;
import Assets.Math;

namespace Assets {
export inline constexpr std::uint32_t ModelRootParent = std::numeric_limits<std::uint32_t>::max();
export struct ModelBoneDesc final {
    std::string name;
    std::uint32_t parent=ModelRootParent;
    Vector3f translation{};
    std::array<float,4> rotation{0,0,0,1};
};
export struct ModelAttachmentDesc final {
    std::string object_name;
    std::uint32_t bone=0;
    std::uint32_t lod=0;
    float maximum_screen_size=std::numeric_limits<float>::max();
};
export enum class ModelChannelComponent : std::uint8_t { TranslationX,TranslationY,TranslationZ,Rotation,Visibility,RotationX,RotationY,RotationZ };
export struct ModelAnimationChannel final {
    std::uint32_t bone=0;
    ModelChannelComponent component=ModelChannelComponent::TranslationX;
    std::uint32_t first_frame=0;
    bool default_visible=true;
    std::vector<std::array<float,4>> samples;
    // Empty key times describe consecutive integer frames starting at first_frame.
    // Otherwise each sample has an authored key time and an optional step into it.
    std::vector<std::uint32_t> key_frames;
    std::vector<std::uint8_t> step_into_key;
    bool hold_endpoints=false;
};
export struct ModelAnimationDesc final {
    std::string name;
    std::string skeleton_name;
    std::uint32_t frame_count=0;
    float frame_rate=0;
    bool channels_available=true;
    std::vector<ModelAnimationChannel> channels;
};
// Sparse clip times select integer frames in a separately retained pose clip.
export struct ModelPoseKey final {
    std::uint32_t frame=0;
    std::uint32_t pose_frame=0;
};
export struct ModelPoseChannel final {
    std::string animation_name;
    std::vector<ModelPoseKey> keys;
};
export struct ModelPoseAnimationDesc final {
    std::string name;
    std::string skeleton_name;
    std::uint32_t frame_count=0;
    float frame_rate=0;
    std::vector<ModelPoseChannel> channels;
    // Empty mapping selects channel zero for every bone of the resolved skeleton.
    std::vector<std::uint32_t> bone_channels;
};

export bool Validate_Pose_Animation(const ModelPoseAnimationDesc& animation,std::string& error) {
    const auto fail=[&](const char* message) { error=message;return false; };
    if(animation.name.empty() || animation.skeleton_name.empty() || !animation.frame_count
        || !std::isfinite(animation.frame_rate) || animation.frame_rate<=0 || animation.channels.empty())
        return fail("invalid pose animation header");
    for(const auto& channel:animation.channels) {
        if(channel.animation_name.empty() || channel.keys.empty())return fail("incomplete pose channel");
        for(std::size_t i=1;i<channel.keys.size();++i)
            if(channel.keys[i].frame<=channel.keys[i-1].frame)return fail("pose key times are not increasing");
    }
    for(auto channel:animation.bone_channels)
        if(channel>=animation.channels.size())return fail("invalid bone pose channel");
    error.clear();return true;
}
export struct ModelRigDesc final {
    std::string skeleton_name;
    std::vector<ModelBoneDesc> bones;
    std::vector<ModelAttachmentDesc> attachments;
    std::vector<ModelAnimationDesc> animations;
};

export bool Validate_Model_Rig(const ModelRigDesc& rig,std::string& error) {
    const auto fail=[&](const char* message) { error=message; return false; };
    if((!rig.bones.empty() || !rig.attachments.empty()) && rig.skeleton_name.empty())
        return fail("rig has no skeleton identity");
    std::unordered_set<std::string> names;
    for(std::size_t i=0;i<rig.bones.size();++i) {
        const auto& bone=rig.bones[i];
        // Anonymous pivots are valid indexed attachments in legacy models.
        if(!bone.name.empty() && !names.insert(bone.name).second) return fail("duplicate bone name");
        if(bone.parent!=ModelRootParent && bone.parent>=i) return fail("bone parent is not earlier in the hierarchy");
        if(!std::isfinite(bone.translation.x) || !std::isfinite(bone.translation.y) || !std::isfinite(bone.translation.z))
            return fail("nonfinite bone translation");
        float length=0;
        for(const auto value:bone.rotation) { if(!std::isfinite(value)) return fail("nonfinite bone rotation"); length+=value*value; }
        if(length<.000001f || !std::isfinite(length)) return fail("invalid bone quaternion");
    }
    for(const auto& attachment:rig.attachments) {
        if(attachment.object_name.empty() || (!rig.bones.empty() && attachment.bone>=rig.bones.size())
            || !std::isfinite(attachment.maximum_screen_size) || attachment.maximum_screen_size<0)
            return fail("invalid rig attachment");
    }
    for(const auto& animation:rig.animations) {
        if(animation.name.empty() || animation.skeleton_name.empty() || !animation.frame_count
            || !std::isfinite(animation.frame_rate) || animation.frame_rate<=0)
            return fail("invalid animation header");
        std::unordered_set<std::uint64_t> channels;
        for(const auto& channel:animation.channels) {
            if((!rig.bones.empty() && channel.bone>=rig.bones.size()) || channel.samples.empty()
                || channel.first_frame>=animation.frame_count
                || (channel.key_frames.empty() && channel.samples.size()>animation.frame_count-channel.first_frame)
                || (!channel.key_frames.empty() && (channel.key_frames.size()!=channel.samples.size()
                    || channel.key_frames.back()>=animation.frame_count))
                || (!channel.step_into_key.empty() && (channel.key_frames.empty()
                    || channel.step_into_key.size()!=channel.samples.size())))
                return fail("animation channel is out of range");
            for(std::size_t i=1;i<channel.key_frames.size();++i)
                if(channel.key_frames[i]<=channel.key_frames[i-1]) return fail("animation key times are not increasing");
            const auto component=static_cast<unsigned>(channel.component);
            if(component>static_cast<unsigned>(ModelChannelComponent::RotationZ)
                || !channels.insert((std::uint64_t(channel.bone)<<32)|component).second)
                return fail("duplicate or invalid animation channel");
            for(const auto& sample:channel.samples) {
                float length=0;
                for(float value:sample) { if(!std::isfinite(value)) return fail("nonfinite animation sample"); length+=value*value; }
                if(channel.component==ModelChannelComponent::Rotation && (length<.000001f || !std::isfinite(length)))
                    return fail("invalid animation quaternion");
            }
        }
    }
    error.clear(); return true;
}
}
