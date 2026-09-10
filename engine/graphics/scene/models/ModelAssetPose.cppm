module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
export module Graphics.Scene.Models.AssetPose;
export import Graphics.Scene.Models.AnimationBlend;
import Assets.ModelRig;
import Graphics.Scene.Models.AnimationChannels;

namespace Graphics {
namespace AssetPoseDetail {
RenderTransform Compose(const std::array<float,4>& q,const std::array<float,3>& t={}) {
    const float scale=2/(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
    const float x=q[0],y=q[1],z=q[2],w=q[3];
    return {{1-scale*(y*y+z*z),scale*(x*y-z*w),scale*(x*z+y*w),t[0],
             scale*(x*y+z*w),1-scale*(x*x+z*z),scale*(y*z-x*w),t[1],
             scale*(x*z-y*w),scale*(y*z+x*w),1-scale*(x*x+y*y),t[2],0,0,0,1}};
}
}
export class ModelAssetPose final {
public:
    bool Initialize(const Assets::ModelRigDesc& rig,std::string& error) {
        if(!Assets::Validate_Model_Rig(rig,error)) return false;
        if(rig.bones.empty()) { error="pose requires a resolved skeleton"; return false; }
        std::vector<SkeletonBone> bones; bones.reserve(rig.bones.size());
        for(const auto& bone:rig.bones)
            bones.push_back({bone.parent,AssetPoseDetail::Compose(bone.rotation,{bone.translation.x,bone.translation.y,bone.translation.z})});
        Skeleton skeleton(bones,{});
        if(!skeleton.Is_Valid()) { error="invalid prepared skeleton"; return false; }
        m_rig=rig; m_skeleton=std::move(skeleton); m_pose.Initialize(bones.size());
        m_translation.resize(bones.size()); m_rotation.resize(bones.size()); m_visible.resize(bones.size());
        return Rest();
    }
    bool Rest() {
        if(!m_skeleton.Is_Valid()) return false;
        const auto bones=m_skeleton.Bones();
        for(std::size_t i=0;i<bones.size();++i) m_pose.Local_Transforms()[i]=bones[i].rest_transform;
        std::fill(m_visible.begin(),m_visible.end(),1);
        return m_pose.Evaluate(m_skeleton,m_pose.Local_Transforms());
    }
    // Frame numbers let the caller retain manual door states and original
    // playback timing. No source lookup or allocation occurs while sampling.
    bool Evaluate(std::size_t clip_index,float frame,bool loop=false) {
        if(clip_index>=m_rig.animations.size() || !std::isfinite(frame)) return false;
        const auto& clip=m_rig.animations[clip_index];
        if(!clip.channels_available || clip.skeleton_name!=m_rig.skeleton_name) return false;
        if(loop) { frame=std::fmod(frame,float(clip.frame_count)); if(frame<0) frame+=float(clip.frame_count); }
        else frame=std::clamp(frame,0.f,float(clip.frame_count-1));
        const auto first=static_cast<std::uint32_t>(std::floor(frame));
        const auto second=loop ? (first+1)%clip.frame_count : std::min(first+1,clip.frame_count-1);
        const float fraction=frame-float(first);
        std::fill(m_translation.begin(),m_translation.end(),std::array<float,3>{});
        std::fill(m_rotation.begin(),m_rotation.end(),AssetPoseDetail::Compose({0,0,0,1}));
        std::fill(m_visible.begin(),m_visible.end(),1);
        for(const auto& channel:clip.channels) {
            using Component=Assets::ModelChannelComponent;
            if(channel.component>=Component::RotationX) return false;
            auto interval=Select_Animation_Interval(channel,frame);
            if(loop && second==0 && channel.component!=Component::Visibility) {
                interval={Select_Animation_Interval(channel,float(first)).first,
                    Select_Animation_Interval(channel,0).first,fraction};
            }
            if(channel.component==Component::Visibility) {
                m_visible[channel.bone]=interval.first[0]!=0;
            } else if(channel.component==Component::Rotation) {
                if(!Blend_Transforms(AssetPoseDetail::Compose(interval.first),AssetPoseDetail::Compose(interval.second),interval.fraction,m_rotation[channel.bone])) return false;
            } else {
                m_translation[channel.bone][static_cast<unsigned>(channel.component)]=interval.first[0]+(interval.second[0]-interval.first[0])*interval.fraction;
            }
        }
        const auto bones=m_skeleton.Bones();
        for(std::size_t bone=0;bone<bones.size();++bone) {
            auto delta=m_rotation[bone];
            for(unsigned i=0;i<3;++i) delta.matrix[i*4+3]=m_translation[bone][i];
            auto& output=m_pose.Local_Transforms()[bone];
            for(unsigned r=0;r<4;++r) for(unsigned c=0;c<4;++c) {
                float value=0;for(unsigned k=0;k<4;++k)value+=bones[bone].rest_transform.matrix[r*4+k]*delta.matrix[k*4+c];
                output.matrix[r*4+c]=value;
            }
        }
        return m_pose.Evaluate(m_skeleton,m_pose.Local_Transforms());
    }
    bool Bone_Transform(std::size_t bone,RenderTransform& output) const {
        if(bone>=m_pose.World_Transforms().size()) return false;
        output=m_pose.World_Transforms()[bone];return true;
    }
    bool Visible(std::size_t bone) const { return bone<m_visible.size() && m_visible[bone]!=0; }
    std::string_view Skeleton_Name() const { return m_rig.skeleton_name; }
    std::size_t Bone_Count() const { return m_skeleton.Bone_Count(); }
private:
    Assets::ModelRigDesc m_rig;
    Skeleton m_skeleton;
    Pose m_pose;
    std::vector<std::array<float,3>> m_translation;
    std::vector<RenderTransform> m_rotation;
    std::vector<unsigned char> m_visible;
};
}
