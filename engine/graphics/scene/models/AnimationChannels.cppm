module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
export module Graphics.Scene.Models.AnimationChannels;
import Assets.ModelRig;

namespace Graphics {
export struct PoseKeyInterval final {
    std::uint32_t first=0,second=0;
    float fraction=0;
};
// Prepared keys are nonempty and strictly increasing. Selection is independent
// of the last object, view, or direction of playback that sampled the clip.
export PoseKeyInterval Select_Pose_Interval(std::span<const Assets::ModelPoseKey> keys,float frame) {
    if(keys.empty() || !std::isfinite(frame))return {};
    if(frame<0 || keys.size()==1)return {keys.front().pose_frame,keys.front().pose_frame,0};
    if(frame>=keys.back().frame)return {keys.back().pose_frame,keys.back().pose_frame,0};
    auto next=std::upper_bound(keys.begin(),keys.end(),frame,
        [](float time,const auto& key) { return time<key.frame; });
    // A nonnegative time preceding the first key extrapolates that interval.
    if(next==keys.begin())++next;
    const auto& first=*(next-1);
    return {first.pose_frame,next->pose_frame,(frame-first.frame)/(next->frame-first.frame)};
}

export struct AnimationChannelInterval final {
    std::array<float,4> first{};
    std::array<float,4> second{};
    float fraction=0;
};
namespace ChannelDetail {
std::array<float,4> Default(const Assets::ModelAnimationChannel& channel) {
    if(channel.component==Assets::ModelChannelComponent::Rotation) return {0,0,0,1};
    if(channel.component==Assets::ModelChannelComponent::Visibility) return {float(channel.default_visible),0,0,0};
    return {};
}
}

// Integer sampling of consecutive frames. The raw animation adapter keeps its
// existing wrap and quaternion interpolation policy around these source values.
export std::array<float,4> Read_Animation_Frame(const Assets::ModelAnimationChannel& channel,std::int64_t frame) {
    if(channel.samples.empty()) return ChannelDetail::Default(channel);
    const auto index=frame-std::int64_t(channel.first_frame);
    if(index<0) return channel.hold_endpoints ? channel.samples.front() : ChannelDetail::Default(channel);
    if(std::uint64_t(index)>=channel.samples.size())
        return channel.hold_endpoints ? channel.samples.back() : ChannelDetail::Default(channel);
    return channel.samples[std::size_t(index)];
}

export void Copy_Animation_Frame(const Assets::ModelAnimationChannel& channel,std::int64_t frame,float* output) {
    const auto sample=Read_Animation_Frame(channel,frame);
    const unsigned width=channel.component==Assets::ModelChannelComponent::Rotation ? 4 : 1;
    std::copy_n(sample.begin(),width,output);
}

// Timed curves retain sparse keys and authored steps. No mutable sampling cache
// is shared between instances, reverse playback, or the main/reflected views.
export AnimationChannelInterval Select_Animation_Interval(const Assets::ModelAnimationChannel& channel,float frame) {
    const auto fallback=ChannelDetail::Default(channel);
    if(!std::isfinite(frame) || channel.samples.empty()) return {fallback,fallback,0};
    if(channel.key_frames.empty()) {
        // Bound before converting a potentially very large floating-point frame.
        if(frame<0) { const auto value=Read_Animation_Frame(channel,-1); return {value,value,0}; }
        if(double(frame)>double(channel.first_frame)+double(channel.samples.size())) {
            const auto value=channel.hold_endpoints ? channel.samples.back() : fallback;
            return {value,value,0};
        }
        const auto first=static_cast<std::int64_t>(std::floor(frame));
        const auto a=Read_Animation_Frame(channel,first);
        const auto b=Read_Animation_Frame(channel,first+1);
        return {a,b,channel.component==Assets::ModelChannelComponent::Visibility ? 0.f : frame-float(first)};
    }
    const auto end=std::upper_bound(channel.key_frames.begin(),channel.key_frames.end(),frame);
    if(end==channel.key_frames.begin()) {
        const auto value=channel.hold_endpoints ? channel.samples.front() : fallback;
        return {value,value,0};
    }
    const auto first=std::size_t(end-channel.key_frames.begin()-1);
    if(end==channel.key_frames.end() || channel.component==Assets::ModelChannelComponent::Visibility
        || (!channel.step_into_key.empty() && channel.step_into_key[first+1]))
        return {channel.samples[first],channel.samples[first],0};
    const float time0=float(channel.key_frames[first]),time1=float(channel.key_frames[first+1]);
    return {channel.samples[first],channel.samples[first+1],(frame-time0)/(time1-time0)};
}

export float Sample_Animation_Scalar(const Assets::ModelAnimationChannel& channel,float frame) {
    const auto interval=Select_Animation_Interval(channel,frame);
    return interval.first[0]+(interval.second[0]-interval.first[0])*interval.fraction;
}
export bool Sample_Animation_Visibility(const Assets::ModelAnimationChannel& channel,float frame) {
    return Select_Animation_Interval(channel,frame).first[0]!=0;
}
}
