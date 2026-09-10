module;
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
export module Assets.Adapters.W3D.AnimationChannels;
import Assets.Adapters.W3D.Chunks;
import Assets.ModelRig;

namespace Assets::W3D {
export enum class AnimationChannelEncoding { RawVector, RawVisibility, TimedVector, TimedVisibility, DeltaVector };
namespace AnimationChannelDetail {
bool U16(W3DByteSpan bytes,std::size_t offset,std::uint32_t& value) {
    if(offset>bytes.size() || bytes.size()-offset<2) return false;
    value=std::to_integer<unsigned>(bytes[offset])|(std::to_integer<unsigned>(bytes[offset+1])<<8);
    return true;
}
bool Component(std::uint32_t type,std::uint32_t width,ModelAnimationChannel& channel) {
    if(type<=2 && width==1) channel.component=static_cast<ModelChannelComponent>(type);
    else if(type>=3 && type<=5 && width==1)
        channel.component=static_cast<ModelChannelComponent>(static_cast<unsigned>(ModelChannelComponent::RotationX)+type-3);
    else if(type==6 && width==4) channel.component=ModelChannelComponent::Rotation;
    else return false;
    return true;
}
bool Sample(W3DByteSpan bytes,std::size_t offset,unsigned width,std::array<float,4>& sample) {
    for(unsigned i=0;i<width;++i)
        if(!W3DRead_F32(bytes,offset+i*4,sample[i]) || !std::isfinite(sample[i])) return false;
    return true;
}
const std::array<float,256>& Filters() {
    static const auto table=[] {
        std::array<float,256> result{.00000001f,.0000001f,.000001f,.00001f,.0001f,.001f,.01f,.1f,
            1.f,10.f,100.f,1000.f,10000.f,100000.f,1000000.f,10000000.f};
        for(unsigned i=0;i<240;++i) {
            const float ratio=float(i)/240.f;
            const float angle=static_cast<float>(double(90.f*ratio)*double(3.141592654f)/180.0);
            result[i+16]=1.f-std::sin(angle);
        }
        return result;
    }();
    return table;
}
}

// The caller supplies one bounded channel payload. Never publish partial data,
// and tolerate the historical exporter's extra trailing vector/packet bytes.
export bool Parse_Animation_Channel(W3DByteSpan bytes,AnimationChannelEncoding encoding,
    ModelAnimationChannel& output,std::string& error) {
    using namespace AnimationChannelDetail;
    const auto fail=[&] { error="invalid W3D animation channel"; return false; };
    ModelAnimationChannel channel;
    std::uint32_t count=0,width=0,type=0;
    if(encoding==AnimationChannelEncoding::RawVector || encoding==AnimationChannelEncoding::RawVisibility) {
        std::uint32_t first=0,last=0;
        if(!U16(bytes,0,first) || !U16(bytes,2,last) || last<first) return fail();
        channel.first_frame=first; count=last-first+1;
        if(encoding==AnimationChannelEncoding::RawVisibility) {
            if(!U16(bytes,4,type) || !U16(bytes,6,channel.bone) || type!=0
                || bytes.size()<9+(std::size_t(count)+7)/8) return fail();
            channel.component=ModelChannelComponent::Visibility;
            channel.default_visible=bytes[8]!=std::byte{};
            channel.samples.resize(count);
            for(std::size_t i=0;i<count;++i)
                channel.samples[i][0]=(std::to_integer<unsigned>(bytes[9+i/8])>>(i%8))&1;
        } else {
            if(!U16(bytes,4,width) || !U16(bytes,6,type) || !U16(bytes,8,channel.bone)
                || !Component(type,width,channel) || bytes.size()<12+std::size_t(count)*width*4) return fail();
            channel.samples.resize(count);
            for(std::size_t i=0;i<count;++i)
                if(!Sample(bytes,12+i*width*4,width,channel.samples[i])) return fail();
        }
    } else {
        if(bytes.size()<8 || !W3DRead_U32(bytes,0,count) || !count || !U16(bytes,4,channel.bone)) return fail();
        channel.hold_endpoints=true;
        if(encoding==AnimationChannelEncoding::TimedVisibility) {
            // Exporters use both visibility flag values for this chunk kind.
            if(std::to_integer<unsigned>(bytes[6])>1 || std::size_t(count)>(bytes.size()-8)/4) return fail();
            channel.component=ModelChannelComponent::Visibility;
            channel.default_visible=bytes[7]!=std::byte{};
            channel.samples.resize(count); channel.key_frames.resize(count);
            for(std::size_t i=0;i<count;++i) {
                std::uint32_t key=0; W3DRead_U32(bytes,8+i*4,key);
                channel.key_frames[i]=key&0x7fffffffu;
                channel.samples[i][0]=(key>>31)!=0;
            }
        } else {
            width=std::to_integer<unsigned>(bytes[6]); type=std::to_integer<unsigned>(bytes[7]);
            if(!Component(type,width,channel)) return fail();
            if(encoding==AnimationChannelEncoding::TimedVector) {
                const std::size_t stride=(width+1)*4;
                if(std::size_t(count)>(bytes.size()-8)/stride) return fail();
                channel.samples.resize(count); channel.key_frames.resize(count); channel.step_into_key.resize(count);
                for(std::size_t i=0;i<count;++i) {
                    std::uint32_t key=0; W3DRead_U32(bytes,8+i*stride,key);
                    channel.key_frames[i]=key&0x7fffffffu; channel.step_into_key[i]=key>>31;
                    if(!Sample(bytes,12+i*stride,width,channel.samples[i])) return fail();
                }
            } else if(encoding==AnimationChannelEncoding::DeltaVector) {
                float scale=0;
                if(!W3DRead_F32(bytes,8,scale) || !std::isfinite(scale)) return fail();
                const std::size_t base_end=12+width*4;
                const std::size_t packets=(std::size_t(count)-1+15)/16;
                if(bytes.size()<base_end || packets>(bytes.size()-base_end)/(width*9)) return fail();
                channel.samples.resize(count);
                if(!Sample(bytes,12,width,channel.samples.front())) return fail();
                for(std::size_t frame=1;frame<count;++frame) {
                    const auto packet=(frame-1)/16;
                    const auto nibble=(frame-1)%16;
                    for(unsigned component=0;component<width;++component) {
                        const auto offset=base_end+(packet*width+component)*9;
                        const float filter=Filters()[std::to_integer<unsigned>(bytes[offset])]*scale;
                        const auto packed=std::to_integer<unsigned>(bytes[offset+1+nibble/2]);
                        int factor=(packed>>((nibble%2)*4))&15;
                        if(factor>=8) factor-=16;
                        const float delta=float(factor)*filter;
                        channel.samples[frame][component]=channel.samples[frame-1][component]+delta;
                        if(!std::isfinite(channel.samples[frame][component])) return fail();
                    }
                }
            } else return fail();
        }
        for(std::size_t i=1;i<channel.key_frames.size();++i)
            if(channel.key_frames[i]<=channel.key_frames[i-1]) return fail();
    }
    output=std::move(channel); error.clear(); return true;
}
}
