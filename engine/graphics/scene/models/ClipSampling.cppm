module;
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
export module Graphics.Scene.Models.ClipSampling;
export import Assets.Cache.Animations;
import Graphics.Scene.Models.AnimationChannels;
import Graphics.Scene.Models.AnimationRotation;
export import Graphics.Scene.AffineTransform;

namespace Graphics {
namespace ClipDetail {
struct Frames { int first,second;float fraction; };
Frames Consecutive(const Assets::AnimationClip& clip,float frame) {
    // Preserve the active runtime's authored-frame conversion, including its
    // half-frame bias. Changing it to floor changes fractional raw playback.
    const auto first=static_cast<int>(frame-.499999f);
    const auto next=first+1;
    return {first,next>=static_cast<int>(clip.frame_count)?0:next,frame-float(first)};
}
bool ValidFrame(float frame) {
    return std::isfinite(frame) && double(frame)>double(std::numeric_limits<int>::min())+2
        && double(frame)<double(std::numeric_limits<int>::max())-2;
}
}
export std::array<float,3> Sample_Clip_Translation(const Assets::AnimationCache& cache,
    Assets::AnimationAssetHandle handle,int bone,float frame) {
    const auto* clip=cache.Resolve(handle);
    if(!clip || bone<0 || std::uint32_t(bone)>=clip->bone_count || !ClipDetail::ValidFrame(frame))return {};
    if(clip->sampling==Assets::AnimationSampling::Pose) {
        const auto channel=clip->pose.bone_channels[bone];
        const auto keys=Select_Pose_Interval(clip->pose.channels[channel].keys,frame);
        const auto a=Sample_Clip_Translation(cache,clip->sources[channel],bone,float(static_cast<std::int32_t>(keys.first)));
        const auto b=Sample_Clip_Translation(cache,clip->sources[channel],bone,float(static_cast<std::int32_t>(keys.second)));
        return {a[0]+(b[0]-a[0])*keys.fraction,a[1]+(b[1]-a[1])*keys.fraction,a[2]+(b[2]-a[2])*keys.fraction};
    }
    std::array<float,3> result{};
    const auto frames=ClipDetail::Consecutive(*clip,frame);
    for(unsigned axis=0;axis<3;++axis) {
        const auto* channel=clip->channels.Channel(bone,static_cast<Assets::ModelChannelComponent>(axis));
        if(!channel)continue;
        if(clip->sampling==Assets::AnimationSampling::Keyed)result[axis]=Sample_Animation_Scalar(*channel,frame);
        else {
            const float a=Read_Animation_Frame(*channel,frames.first)[0];
            if(frames.fraction==0)result[axis]=a;
            else { const float b=Read_Animation_Frame(*channel,frames.second)[0];result[axis]=a+(b-a)*frames.fraction; }
        }
    }
    return result;
}
export std::array<float,4> Sample_Clip_Rotation(const Assets::AnimationCache& cache,
    Assets::AnimationAssetHandle handle,int bone,float frame) {
    const auto* clip=cache.Resolve(handle);
    if(!clip || bone<0 || std::uint32_t(bone)>=clip->bone_count || !ClipDetail::ValidFrame(frame))return {0,0,0,1};
    if(clip->sampling==Assets::AnimationSampling::Pose) {
        const auto channel=clip->pose.bone_channels[bone];
        const auto keys=Select_Pose_Interval(clip->pose.channels[channel].keys,frame);
        return Interpolate_Animation_Rotation(
            Sample_Clip_Rotation(cache,clip->sources[channel],bone,float(static_cast<std::int32_t>(keys.first))),
            Sample_Clip_Rotation(cache,clip->sources[channel],bone,float(static_cast<std::int32_t>(keys.second))),keys.fraction);
    }
    const auto* channel=clip->channels.Channel(bone,Assets::ModelChannelComponent::Rotation);
    if(!channel)return {0,0,0,1};
    if(clip->sampling==Assets::AnimationSampling::Keyed) {
        const auto keys=Select_Animation_Interval(*channel,frame);
        return keys.fraction==0 ? keys.first : Interpolate_Animation_Rotation(keys.first,keys.second,keys.fraction);
    }
    const auto frames=ClipDetail::Consecutive(*clip,frame);
    const auto a=Read_Animation_Frame(*channel,frames.first);
    if(frames.fraction==0)return a;
    const auto b=Read_Animation_Frame(*channel,frames.second);
    if(frames.fraction==1)return b;
    return Interpolate_Animation_Rotation(a,b,frames.fraction);
}
export bool Sample_Clip_Visibility(const Assets::AnimationCache& cache,
    Assets::AnimationAssetHandle handle,int bone,float frame) {
    const auto* clip=cache.Resolve(handle);
    if(!clip || bone<0 || std::uint32_t(bone)>=clip->bone_count || !ClipDetail::ValidFrame(frame))return true;
    if(clip->sampling==Assets::AnimationSampling::Pose)return true;
    const auto* channel=clip->channels.Channel(bone,Assets::ModelChannelComponent::Visibility);
    return !channel || Sample_Animation_Visibility(*channel,float(static_cast<int>(frame)));
}
export RenderTransform Sample_Clip_Transform(const Assets::AnimationCache& cache,
    Assets::AnimationAssetHandle handle,int bone,float frame) {
    const auto* clip=cache.Resolve(handle);
    auto rotation=Sample_Clip_Rotation(cache,handle,bone,frame);
    // Raw transform evaluation historically interpolates at fraction one,
    // whereas raw orientation queries return the second endpoint directly.
    if(clip && clip->sampling==Assets::AnimationSampling::Consecutive && ClipDetail::ValidFrame(frame)) {
        const auto frames=ClipDetail::Consecutive(*clip,frame);
        const auto* channel=clip->channels.Channel(bone,Assets::ModelChannelComponent::Rotation);
        if(channel && frames.fraction==1)rotation=Interpolate_Animation_Rotation(
            Read_Animation_Frame(*channel,frames.first),Read_Animation_Frame(*channel,frames.second),1);
    }
    auto result=Quaternion_Affine(rotation);
    const auto translation=Sample_Clip_Translation(cache,handle,bone,frame);
    for(unsigned axis=0;axis<3;++axis)result.matrix[axis*4+3]=translation[axis];
    return result;
}
}
