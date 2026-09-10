module;
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
export module Assets.Adapters.W3D.Aggregate;
export import Assets.ModelAssembly;
import Assets.Adapters.W3D.Chunks;

namespace Assets::W3D {
export inline constexpr std::uint32_t W3DChunkAggregate = 0x00000600u;

// Format classification is adapter metadata, separate from the generic asset.
export struct W3DAggregateDescription final {
    ModelAggregateDesc model;
    std::uint32_t original_class_id=25;
};
export bool W3DRead_Model_Aggregate(W3DByteSpan bytes,W3DAggregateDescription& output,std::string& error)
{
    W3DAggregateDescription next;
    bool header=false,info=false,classification=false;
    error.clear();
    const bool valid=W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        if(chunk.id==0x601) {
            if(header || chunk.payload.size()!=20)return false;
            header=true;
            next.model.name=W3DRead_Fixed_String(chunk.payload,4,16);
        } else if(chunk.id==0x602) {
            if(info || chunk.payload.size()<36)return false;
            info=true;
            next.model.base_model=W3DRead_Fixed_String(chunk.payload,0,32);
            std::uint32_t count=0;W3DRead_U32(chunk.payload,32,count);
            if((chunk.payload.size()-36)%64 || count!=(chunk.payload.size()-36)/64)return false;
            for(std::size_t offset=36;offset<chunk.payload.size();offset+=64) {
                ModelNamedAttachmentDesc attachment;
                attachment.model_name=W3DRead_Fixed_String(chunk.payload,offset,32);
                attachment.bone_name=W3DRead_Fixed_String(chunk.payload,offset+32,32);
                if(attachment.model_name.empty())return false;
                next.model.attachments.push_back(std::move(attachment));
            }
        } else if(chunk.id==0x604) {
            if(classification || chunk.payload.size()!=20)return false;
            classification=true;
            W3DRead_U32(chunk.payload,0,next.original_class_id);
            std::uint32_t flags=0;W3DRead_U32(chunk.payload,4,flags);
            next.model.match_detail_levels=(flags&1)!=0;
        }
        return true;
    });
    if(!valid || !header || !info || next.model.name.empty() || next.model.base_model.empty()) {
        error="invalid W3D model aggregate";
        return false;
    }
    output=std::move(next);
    return true;
}
}
