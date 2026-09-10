module;
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
export module Assets.Adapters.W3D.Collection;
export import Assets.ModelAssembly;
import Assets.Adapters.W3D.Chunks;

namespace Assets::W3D {
export inline constexpr std::uint32_t W3DChunkCollection = 0x00000420u;

export bool W3DRead_Model_Collection(W3DByteSpan bytes,ModelCollectionDesc& output,std::string& error)
{
    ModelCollectionDesc next;
    W3DByteSpan header;
    bool points=false;
    error.clear();
    const bool valid=W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id==0x421) {
            if(!header.empty() || chunk.payload.size()!=32)return false;
            header=chunk.payload;
            next.name=W3DRead_Fixed_String(header,4,16);
        } else if(chunk.id==0x422) {
            if(chunk.payload.empty() || chunk.payload.back()!=std::byte(0))return false;
            next.children.push_back(W3DRead_Fixed_String(chunk.payload,0,chunk.payload.size()));
        } else if(chunk.id==0x423) {
            if(chunk.payload.size()<56)return false;
            std::uint32_t length=0;
            W3DRead_U32(chunk.payload,52,length);
            if(length!=chunk.payload.size()-56)return false;
            ModelProxyDesc proxy;
            proxy.name=W3DRead_Fixed_String(chunk.payload,56,length);
            // MAX stores four columns; consumers use three affine rows.
            for(unsigned row=0;row<3;++row)
                for(unsigned column=0;column<4;++column)
                    W3DRead_F32(chunk.payload,4+(column*3+row)*4,proxy.transform[row*4+column]);
            next.proxies.push_back(std::move(proxy));
        } else if(chunk.id==0x440) {
            if(points || chunk.payload.size()%12)return false;
            points=true;
            for(std::size_t offset=0;offset<chunk.payload.size();offset+=12) {
                Vector3f point;
                W3DRead_Vector3(chunk.payload,offset,point);
                next.snap_points.push_back(point);
            }
        }
        return true;
    });
    std::uint32_t count=0;
    if(!valid || header.empty() || !W3DRead_U32(header,20,count) || count!=next.children.size()) {
        error="invalid W3D model collection";
        return false;
    }
    if(!Validate_Model_Collection(next,error))return false;
    output=std::move(next);
    return true;
}
}
