module;
#include <cmath>
#include <cstddef>
#include <string>
#include <utility>
export module Assets.Adapters.W3D.LevelSet;
export import Assets.ModelAssembly;
import Assets.Adapters.W3D.Chunks;

namespace Assets::W3D {
export inline constexpr std::uint32_t W3DChunkLodModel = 0x00000400u;

export bool W3DRead_Model_Level_Set(W3DByteSpan bytes,ModelLevelSetDesc& output,std::string& error)
{
    ModelLevelSetDesc next;
    W3DByteSpan header;
    error.clear();
    const bool valid=W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id==0x401) {
            if(!header.empty() || chunk.payload.size()!=24)return false;
            header=chunk.payload;
            next.name=W3DRead_Fixed_String(header,4,16);
        } else if(chunk.id==0x402) {
            if(chunk.payload.size()!=40)return false;
            ModelDetailLevelDesc level;
            level.name=W3DRead_Fixed_String(chunk.payload,0,32);
            W3DRead_F32(chunk.payload,32,level.minimum_distance);
            W3DRead_F32(chunk.payload,36,level.maximum_distance);
            if(level.name.empty() || !std::isfinite(level.minimum_distance) || !std::isfinite(level.maximum_distance))return false;
            next.levels.push_back(std::move(level));
        }
        return true;
    });
    if(!valid || header.empty() || next.name.empty() || next.levels.empty()) {
        error="invalid W3D model level set";
        return false;
    }
    const auto count=std::to_integer<unsigned>(header[20])|(std::to_integer<unsigned>(header[21])<<8);
    if(next.levels.size()!=count) {
        error="W3D model level count mismatch";
        return false;
    }
    output=std::move(next);
    return true;
}
}
