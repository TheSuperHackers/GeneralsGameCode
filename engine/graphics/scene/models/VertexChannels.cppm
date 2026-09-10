module;
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <vector>
export module Graphics.Scene.Models.VertexChannels;
import Graphics.Scene.Models.SourceRevision;

namespace Graphics {
// Value represents a fully initialized vertex attribute without padding when
// installed. Raw writable channels are not candidates for content deduplication.
export template<class Value>
class VertexChannels final
{
    struct Channel {
        std::vector<Value> values;
        bool installed=false;
        SourceRevision revision;
    };
public:
    std::size_t Count() const noexcept
    {
        std::size_t count=0;
        while(count<channels.size() && channels[count])++count;
        return count;
    }
    bool Empty() const noexcept
    {
        for(const auto& channel:channels)if(channel)return false;
        return true;
    }
    bool Is_Allocated(std::size_t index) const noexcept
    {
        return index < channels.size() && static_cast<bool>(channels[index]);
    }
    void Clear() { channels.clear(); }
    Value* Get(std::size_t index) const
    {
        if(index<channels.size() && channels[index]) channels[index]->revision.Expose_Writable();
        return index<channels.size() && channels[index] ? channels[index]->values.data() : nullptr;
    }
    const Value* Peek(std::size_t index) const noexcept
    {
        return index<channels.size() && channels[index] ? channels[index]->values.data() : nullptr;
    }
    std::uint64_t Revision(std::size_t index) const noexcept
    {
        return index<channels.size() && channels[index] ? channels[index]->revision.Token() : 0;
    }
    void Allocate(std::size_t index,std::size_t count)
    {
        if(index>=channels.size())channels.resize(index+1);
        if(!channels[index]) {
            auto channel=std::make_shared<Channel>();channel->values.resize(count);
            channels[index]=std::move(channel);
        }
    }
    void Set(std::size_t index,std::size_t vertex,const Value& value)
    {
        auto& channel=*channels.at(index);
        channel.revision.Invalidate();
        channel.values.at(vertex)=value;
    }
    Value* Create(std::size_t index,std::size_t count)
    {
        Allocate(index,count);
        return Get(index);
    }
    void Make_Unique(std::size_t index)
    {
        if(index<channels.size() && channels[index] && channels[index].use_count()>1)
            channels[index]=std::make_shared<Channel>(*channels[index]);
    }
    void Share(std::size_t destination,const VertexChannels& source,std::size_t index)
    {
        const auto value=source.channels.at(index);
        if(destination>=channels.size())channels.resize(destination+1);
        channels[destination]=value;
    }
    std::size_t Install(std::span<const Value> values)
    {
        const auto index=Find(values);
        if(index<Count())return index;
        auto channel=std::make_shared<Channel>();
        channel->values.assign(values.begin(),values.end());channel->installed=true;
        Put(index,std::move(channel));return index;
    }
    std::size_t Import(const VertexChannels& source,std::size_t index)
    {
        const auto value=source.channels.at(index);
        for(std::size_t i=0;i<Count();++i)if(channels[i]==value)return i;
        const auto destination=value->installed ? Find(value->values) : Count();
        if(destination==Count())Put(destination,value);
        return destination;
    }
private:
    std::size_t Find(std::span<const Value> values) const
    {
        const auto count=Count();
        for(std::size_t i=0;i<count;++i) {
            const auto& channel=*channels[i];
            if(channel.installed && channel.values.size()==values.size() &&
                (values.empty() || std::memcmp(channel.values.data(),values.data(),values.size_bytes())==0))return i;
        }
        return count;
    }
    void Put(std::size_t index,std::shared_ptr<Channel> value)
    {
        if(index>=channels.size())channels.resize(index+1);
        channels[index]=std::move(value);
    }
    std::vector<std::shared_ptr<Channel>> channels;
};
}
