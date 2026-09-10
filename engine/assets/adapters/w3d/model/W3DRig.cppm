module;
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>
export module Assets.Adapters.W3D.Rig;
import Assets.Adapters.W3D.Assembly;
import Assets.Adapters.W3D.Chunks;
import Assets.ModelRig;
import Assets.Adapters.W3D.AnimationChannels;

namespace Assets::W3D {
export bool W3DRead_Pose_Keys(W3DByteSpan bytes,std::vector<ModelPoseKey>& result,std::string& error) {
    if(bytes.empty() || bytes.size()%8) { error="invalid W3D pose key payload";return false; }
    std::vector<ModelPoseKey> keys(bytes.size()/8);
    for(std::size_t i=0;i<keys.size();++i) {
        W3DRead_U32(bytes,i*8,keys[i].frame);
        W3DRead_U32(bytes,i*8+4,keys[i].pose_frame);
        if(i && keys[i].frame<=keys[i-1].frame) {
            error="W3D pose key times are not increasing";return false;
        }
    }
    result=std::move(keys);error.clear();return true;
}

export bool W3DRead_Pose_Animation(W3DByteSpan bytes,ModelPoseAnimationDesc& result,std::string& error) {
    W3DByteSpan header,mapping;
    bool has_header=false,has_mapping=false;
    std::vector<W3DByteSpan> channels;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id==0x2c1) { if(has_header)return false;has_header=true;header=chunk.payload; }
        if(chunk.id==0x2c2)channels.push_back(chunk.payload);
        if(chunk.id==0x2c5) { if(has_mapping)return false;has_mapping=true;mapping=chunk.payload; }
        return true;
    }) || !has_header || header.size()!=48 || mapping.size()%4) {
        error="invalid W3D pose animation payload";return false;
    }
    ModelPoseAnimationDesc next;
    next.name=W3DRead_Fixed_String(header,4,16);
    next.skeleton_name=W3DRead_Fixed_String(header,20,16);
    W3DRead_U32(header,36,next.frame_count);W3DRead_F32(header,40,next.frame_rate);
    std::uint32_t count=0;W3DRead_U32(header,44,count);
    if(count!=channels.size()) { error="W3D pose channel count mismatch";return false; }
    for(const auto bytes:channels) {
        W3DByteSpan name,keys;bool has_name=false,has_keys=false;
        if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
            if(chunk.id==0x2c3) { if(has_name)return false;has_name=true;name=chunk.payload; }
            if(chunk.id==0x2c4) { if(has_keys)return false;has_keys=true;keys=chunk.payload; }
            return true;
        }) || !has_name || !has_keys || name.empty()) { error="incomplete W3D pose channel";return false; }
        ModelPoseChannel channel;
        channel.animation_name=W3DRead_Fixed_String(name,0,name.size());
        if(!W3DRead_Pose_Keys(keys,channel.keys,error))return false;
        next.channels.push_back(std::move(channel));
    }
    next.bone_channels.resize(mapping.size()/4);
    for(std::size_t bone=0;bone<next.bone_channels.size();++bone)
        W3DRead_U32(mapping,bone*4,next.bone_channels[bone]);
    if(!Validate_Pose_Animation(next,error))return false;
    result=std::move(next);return true;
}

// The caller supplies the hierarchy container's payload. Publication is atomic:
// rejected bytes leave the previous description intact.
export bool W3DRead_Hierarchy(W3DByteSpan bytes,ModelRigDesc& result,std::string& error) {
    W3DByteSpan header,pivots; bool has_header=false,has_pivots=false;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& c) {
        if(c.id==0x101) { if(has_header) return false; has_header=true; header=c.payload; }
        if(c.id==0x102) { if(has_pivots) return false; has_pivots=true; pivots=c.payload; }
        return true;
    }) || !has_header || !has_pivots || header.size()!=36) { error="invalid W3D hierarchy header"; return false; }
    std::uint32_t count=0,version=0;
    W3DRead_U32(header,0,version); W3DRead_U32(header,20,count);
    const bool insert_root=version<0x30000;
    // All published hierarchy versions use the same pivot record. Pre-3.0
    // records precede the synthetic root used by runtime attachment indices.
    if(!count || count>=std::uint32_t(std::numeric_limits<int>::max())
        || pivots.size()%60 || pivots.size()/60!=count) {
        error="invalid W3D hierarchy pivot count"; return false;
    }
    ModelRigDesc rig;
    rig.skeleton_name=W3DRead_Fixed_String(header,4,16);
    rig.bones.reserve(std::size_t(count)+insert_root);
    if(insert_root) rig.bones.push_back({"RootTransform"});
    for(std::size_t i=0;i<count;++i) {
        const auto p=pivots.subspan(i*60,60); ModelBoneDesc bone;
        bone.name=W3DRead_Fixed_String(p,0,16); W3DRead_U32(p,16,bone.parent);
        if(insert_root) {
            // The on-disk -1 parent becomes the inserted root, not a second root.
            if(bone.parent==ModelRootParent) bone.parent=0;
            else if(bone.parent>=i) { error="invalid W3D hierarchy parent"; return false; }
            else ++bone.parent;
        }
        W3DRead_Vector3(p,20,bone.translation);
        for(unsigned k=0;k<4;++k) W3DRead_F32(p,44+k*4,bone.rotation[k]);
        rig.bones.push_back(std::move(bone));
    }
    if(!Validate_Model_Rig(rig,error)) return false;
    result=std::move(rig); return true;
}
namespace RigDetail {
bool U16(W3DByteSpan bytes,std::size_t offset,std::uint32_t& value) {
    if(offset>bytes.size() || bytes.size()-offset<2) return false;
    value=std::to_integer<unsigned>(bytes[offset])|(std::to_integer<unsigned>(bytes[offset+1])<<8); return true;
}
bool Hlod(W3DByteSpan bytes,ModelRigDesc& rig,std::string& error) {
    ModelAssemblyDesc description;
    if(!W3DRead_Model_Assembly(bytes,true,description,error))return false;
    if(!rig.skeleton_name.empty() && rig.skeleton_name!=description.skeleton_name) {
        error="W3D HLOD hierarchy mismatch";return false;
    }
    rig.skeleton_name=description.skeleton_name;
    for(std::size_t level=0;level<description.levels.size();++level) {
        auto& source=description.levels[level];
        for(auto& attachment:source.children) {
            attachment.lod=static_cast<std::uint32_t>(level);
            attachment.maximum_screen_size=source.maximum_screen_size;
            rig.attachments.push_back(std::move(attachment));
        }
    }
    return true;
}
bool Animation(W3DByteSpan bytes,bool compressed,ModelAnimationDesc& animation,std::string& error) {
    W3DByteSpan header;
    std::vector<W3DChunkView> channels;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& c) {
        if(c.id==(compressed ? 0x281u : 0x201u)) { if(!header.empty()) return false; header=c.payload; }
        else channels.push_back(c);
        return true;
    }) || header.size()!=44) { error="invalid W3D animation payload"; return false; }
    std::uint32_t version=0; W3DRead_U32(header,0,version);
    animation.name=W3DRead_Fixed_String(header,4,16);
    animation.skeleton_name=W3DRead_Fixed_String(header,20,16);
    W3DRead_U32(header,36,animation.frame_count);
    std::uint32_t rate=0,flavor=0;
    if(compressed) { U16(header,40,rate); U16(header,42,flavor); }
    else W3DRead_U32(header,40,rate);
    if(compressed && flavor>1) { error="unsupported W3D animation compression"; return false; }
    animation.frame_rate=float(rate);
    for(const auto& chunk:channels) {
        AnimationChannelEncoding encoding;
        if(!compressed && chunk.id==0x202) encoding=AnimationChannelEncoding::RawVector;
        else if(!compressed && chunk.id==0x203) encoding=AnimationChannelEncoding::RawVisibility;
        else if(compressed && chunk.id==0x282) encoding=flavor==0 ? AnimationChannelEncoding::TimedVector : AnimationChannelEncoding::DeltaVector;
        else if(compressed && chunk.id==0x283) encoding=AnimationChannelEncoding::TimedVisibility;
        else continue; // Unrelated extension chunks do not carry motion channels.
        ModelAnimationChannel channel;
        if(!Parse_Animation_Channel(chunk.payload,encoding,channel,error)) return false;
        if(!compressed && version<0x30000)++channel.bone;
        animation.channels.push_back(std::move(channel));
    }
    animation.channels_available=true;
    return true;
}
}

// Decode a single animation container payload for runtime loading. Validate
// before publication; skeleton binding is a separate preparation step.
export bool W3DRead_Animation(W3DByteSpan bytes,bool compressed,ModelAnimationDesc& result,std::string& error) {
    ModelRigDesc validation;
    validation.animations.emplace_back();
    if(!RigDetail::Animation(bytes,compressed,validation.animations.back(),error)
        || !Validate_Model_Rig(validation,error))return false;
    result=std::move(validation.animations.back());error.clear();return true;
}

// Decode separately from geometry: immutable source data carries named bones,
// LOD attachments and animation channels without importing graphics types.
export bool W3DRead_Model_Rig(W3DByteSpan bytes,ModelRigDesc& result,std::string& error) {
    ModelRigDesc next; bool hierarchy=false,hlod=false;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& c) {
        if(c.id==0x100) {
            ModelRigDesc decoded;
            if(hierarchy || !W3DRead_Hierarchy(c.payload,decoded,error)) return false;
            if(!next.skeleton_name.empty() && next.skeleton_name!=decoded.skeleton_name) {
                error="W3D hierarchy identity mismatch"; return false;
            }
            next.skeleton_name=std::move(decoded.skeleton_name);
            next.bones=std::move(decoded.bones);
            hierarchy=true;
        } else if(c.id==0x700) {
            if(hlod || !RigDetail::Hlod(c.payload,next,error)) return false;
            hlod=true;
        } else if(c.id==0x200 || c.id==0x280) {
            ModelAnimationDesc animation;
            if(!RigDetail::Animation(c.payload,c.id==0x280,animation,error)) return false;
            next.animations.push_back(std::move(animation));
        }
        return true;
    })) { if(error.empty()) error="invalid or duplicate W3D rig chunks"; return false; }
    if(!Validate_Model_Rig(next,error)) return false;
    result=std::move(next); return true;
}
}
