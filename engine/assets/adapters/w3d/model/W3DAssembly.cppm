module;
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
export module Assets.Adapters.W3D.Assembly;
export import Assets.ModelAssembly;
import Assets.Adapters.W3D.Chunks;
namespace Assets::W3D {
export inline constexpr std::uint32_t W3DChunkHModel = 0x00000300u;
export inline constexpr std::uint32_t W3DChunkHlod = 0x00000700u;

namespace AssemblyDetail {
bool Array(W3DByteSpan bytes,ModelAssemblyLevel& level,std::string& error) {
    std::uint32_t count=0;bool header=false;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id==0x703) {
            if(header || chunk.payload.size()!=8)return false;
            header=true;W3DRead_U32(chunk.payload,0,count);
            W3DRead_F32(chunk.payload,4,level.maximum_screen_size);
        } else if(chunk.id==0x704) {
            if(chunk.payload.size()!=36)return false;
            ModelAttachmentDesc child;W3DRead_U32(chunk.payload,0,child.bone);
            child.object_name=W3DRead_Fixed_String(chunk.payload,4,32);
            level.children.push_back(std::move(child));
        }
        return true;
    }) || !header || level.children.size()!=count) {
        error="invalid W3D assembly array";return false;
    }
    return true;
}
bool Hlod(W3DByteSpan bytes,ModelAssemblyDesc& result,std::string& error) {
    W3DByteSpan header;bool aggregates=false,proxies=false;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id==0x701) {
            if(!header.empty() || chunk.payload.size()!=40)return false;
            header=chunk.payload;
        } else if(chunk.id==0x702) {
            result.levels.emplace_back();
            if(!Array(chunk.payload,result.levels.back(),error))return false;
        } else if(chunk.id==0x705 || chunk.id==0x706) {
            auto& seen=chunk.id==0x705?aggregates:proxies;
            if(seen)return false;seen=true;
            ModelAssemblyLevel array;
            if(!Array(chunk.payload,array,error))return false;
            (chunk.id==0x705?result.aggregates:result.proxies)=std::move(array.children);
        }
        return true;
    }) || header.empty()) { if(error.empty())error="invalid W3D assembly chunks";return false; }
    std::uint32_t count=0;W3DRead_U32(header,4,count);
    if(result.levels.size()!=count) { error="W3D assembly level count mismatch";return false; }
    result.name=W3DRead_Fixed_String(header,8,16);
    result.skeleton_name=W3DRead_Fixed_String(header,24,16);
    return true;
}
bool Hmodel(W3DByteSpan bytes,ModelAssemblyDesc& result,std::string& error) {
    W3DByteSpan header;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id==0x301) {
            if(!header.empty() || chunk.payload.size()!=40)return false;
            header=chunk.payload;
        }
        return true;
    }) || header.empty()) { error="invalid W3D hierarchy model header";return false; }
    result.name=W3DRead_Fixed_String(header,4,16);
    result.skeleton_name=W3DRead_Fixed_String(header,20,16);
    const auto count=std::to_integer<unsigned>(header[36])|(std::to_integer<unsigned>(header[37])<<8);
    std::uint32_t version=0;W3DRead_U32(header,0,version);
    result.levels.emplace_back();
    bool points=false;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id>=0x302 && chunk.id<=0x304) {
            if(chunk.payload.size()!=18)return false;
            ModelAttachmentDesc child;
            child.object_name=result.name+"."+W3DRead_Fixed_String(chunk.payload,0,16);
            const auto bone=std::to_integer<unsigned>(chunk.payload[16])|(std::to_integer<unsigned>(chunk.payload[17])<<8);
            child.bone=version<0x30000 ? (bone==65535?0:bone+1) : bone;
            if(version>=0x30000 && bone==65535)return false;
            result.levels.front().children.push_back(std::move(child));
        } else if(chunk.id==0x440) {
            if(points || chunk.payload.size()%12)return false;points=true;
            for(std::size_t offset=0;offset<chunk.payload.size();offset+=12) {
                Vector3f point;W3DRead_Vector3(chunk.payload,offset,point);result.snap_points.push_back(point);
            }
        }
        return true;
    }) || result.levels.front().children.size()!=count) {
        error="invalid W3D hierarchy model connections";return false;
    }
    return true;
}
}
// The caller supplies one complete HLOD or HMODEL container payload. Decoding
// and validation are atomic; no child assets are loaded by this adapter.
export bool W3DRead_Model_Assembly(W3DByteSpan bytes,bool hierarchical_lod,
    ModelAssemblyDesc& output,std::string& error) {
    ModelAssemblyDesc next;error.clear();
    if(!(hierarchical_lod?AssemblyDetail::Hlod(bytes,next,error):AssemblyDetail::Hmodel(bytes,next,error)))return false;
    if(!Validate_Model_Assembly(next,error))return false;
    output=std::move(next);return true;
}
}
