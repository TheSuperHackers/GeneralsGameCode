module;
#include "../../profiling/Tracy.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module Graphics.Scene.Models.Hierarchy;
export import Graphics.Scene.Models.Animation;
export import Graphics.Scene.AffineTransform;
import Graphics.Memory.AlignedAllocator;
import Assets.ModelRig;

namespace Graphics {
export struct BoneMotion final {
    std::array<float,3> translation{};
    std::array<float,4> orientation{0,0,0,1};
    bool translate=false;
    bool rotate=false;
    bool set_visibility=false;
    bool visible=true;
};

// A model instance owns flat bone metadata, pose buffers and control state.
// Animation sources supply samples; this component owns hierarchy evaluation.
export class ModelHierarchy final {
public:
    ModelHierarchy() { Initialize_Default(); }

    explicit ModelHierarchy(const Assets::ModelRigDesc& rig) {
        std::string error;
        const bool initialized=Initialize(rig,error);
        assert(initialized);
    }

    bool Initialize(const Assets::ModelRigDesc& rig,std::string& error) {
        if(!Assets::Validate_Model_Rig(rig,error) || rig.bones.empty()) return false;
        for(std::size_t i=1;i<rig.bones.size();++i) {
            if(rig.bones[i].parent==Assets::ModelRootParent) {
                error="model hierarchy requires one object root";return false;
            }
        }
        AlignedVector<SkeletonBone> bones;
        std::vector<std::string> names;
        bones.reserve(rig.bones.size());names.reserve(rig.bones.size());
        for(const auto& bone:rig.bones) {
            auto rest=Affine_Identity();
            Translate_Affine(rest,{bone.translation.x,bone.translation.y,bone.translation.z});
            rest=Multiply_Affine(rest,Quaternion_Affine(bone.rotation));
            bones.push_back({bone.parent,rest});names.push_back(bone.name);
        }
        m_name=rig.skeleton_name;m_bones=std::move(bones);m_names=std::move(names);
        m_pose.Initialize(m_bones.size());
        std::fill(m_pose.World_Transforms().begin(),m_pose.World_Transforms().end(),Affine_Identity());
        m_controls.assign(m_bones.size(),ControlState{});
        m_visible.assign(m_bones.size(),1);m_scale=1;
        error.clear();return true;
    }

    void Initialize_Default() {
        m_name.clear();m_names={"RootTransform"};
        m_bones={{Invalid_Bone_Index,Affine_Identity()}};
        m_pose.Initialize(1);m_pose.World_Transforms()[0]=Affine_Identity();
        m_controls.assign(1,ControlState{});m_visible.assign(1,1);m_scale=1;
    }

    const char* Name() const noexcept { return m_name.c_str(); }
    int Bone_Count() const noexcept { return static_cast<int>(m_bones.size()); }
    const char* Bone_Name(int bone) const { assert(Valid(bone));return m_names[bone].c_str(); }
    int Parent_Index(int bone) const {
        assert(Valid(bone));const auto parent=m_bones[bone].parent;
        return parent==Invalid_Bone_Index ? -1 : static_cast<int>(parent);
    }
    int Bone_Index(std::string_view name) const {
        const auto lower=[](char value) { return value>='A'&&value<='Z' ? char(value-'A'+'a') : value; };
        for(std::size_t i=0;i<m_names.size();++i) {
            if(m_names[i].size()==name.size() && std::equal(name.begin(),name.end(),m_names[i].begin(),
                [&](char a,char b) { return lower(a)==lower(b); })) return static_cast<int>(i);
        }
        return 0;
    }
    const RenderTransform& World_Transform(int bone) const { assert(Valid(bone));return m_pose.World_Transforms()[bone]; }
    bool Visible(int bone) const { assert(Valid(bone));return m_visible[bone]!=0; }

    void Scale(float factor) {
        if(factor==1)return;
        for(auto& bone:m_bones)for(unsigned row=0;row<3;++row)bone.rest_transform.matrix[row*4+3]*=factor;
        m_scale*=factor;
    }
    void Capture(int bone) { assert(Valid(bone));m_controls[bone].captured=true; }
    void Release(int bone) { assert(Valid(bone));m_controls[bone].captured=false; }
    bool Is_Captured(int bone) const { assert(Valid(bone));return m_controls[bone].captured; }
    void Control(int bone,const RenderTransform& delta,bool world_translation=false) {
        assert(Valid(bone));assert(Is_Captured(bone));
        m_controls[bone].delta=delta;m_controls[bone].world_translation=world_translation;
    }

    void Evaluate_Rest(const RenderTransform& root) {
        GRAPHICS_PROFILE_SCOPE("Graphics.Models.EvaluateRestHierarchy");
        Evaluate(root,[](int) { BoneMotion motion;motion.set_visibility=true;return motion; });
    }

    template<class Sampler>
    void Evaluate(const RenderTransform& root,Sampler&& sample) {
        auto world=m_pose.World_Transforms();world[0]=root;m_visible[0]=1;
        for(std::size_t i=1;i<m_bones.size();++i) {
            auto transform=Multiply_Affine(world[m_bones[i].parent],m_bones[i].rest_transform);
            const BoneMotion motion=sample(static_cast<int>(i));
            if(motion.translate) {
                auto translation=motion.translation;
                if(m_scale!=1)for(auto& component:translation)component*=m_scale;
                Translate_Affine(transform,translation);
            }
            if(motion.rotate)transform=Multiply_Affine(transform,Quaternion_Affine(motion.orientation));
            if(motion.set_visibility)m_visible[i]=motion.visible;
            const auto& control=m_controls[i];
            if(control.captured) {
                auto delta=control.delta;
                if(control.world_translation)delta.matrix[3]=delta.matrix[7]=delta.matrix[11]=0;
                transform=Multiply_Affine(transform,delta);
                if(control.world_translation)for(unsigned row=0;row<3;++row)
                    transform.matrix[row*4+3]+=control.delta.matrix[row*4+3];
                m_visible[i]=1;
            }
            world[i]=transform;
        }
    }

    // Queries deliberately omit capture state and never replace the published
    // pose. Walk ancestors from the requested bone to retain query composition.
    template<class Sampler>
    bool Evaluate_Bone(int bone,const RenderTransform& root,Sampler&& sample,RenderTransform& result) const {
        result=Affine_Identity();if(!Valid(bone))return false;
        for(int current=bone;Parent_Index(current)!=-1;current=Parent_Index(current)) {
            auto motion=sample(current);
            for(unsigned row=0;row<3;++row)motion.matrix[row*4+3]*=m_scale;
            result=Multiply_Affine(Multiply_Affine(m_bones[current].rest_transform,motion),result);
        }
        result=Multiply_Affine(root,result);return true;
    }

private:
    struct ControlState {
        RenderTransform delta=Affine_Identity();
        bool captured=false;
        bool world_translation=false;
    };
    bool Valid(int bone) const noexcept { return bone>=0&&std::size_t(bone)<m_bones.size(); }
    std::string m_name;
    std::vector<std::string> m_names;
    AlignedVector<SkeletonBone> m_bones;
    Pose m_pose;
    AlignedVector<ControlState> m_controls;
    std::vector<std::uint8_t> m_visible;
    float m_scale=1;
};
}
