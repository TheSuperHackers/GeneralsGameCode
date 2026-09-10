#pragma once
#include <bit>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <utility>
namespace AssemblyTestData {
using Bytes=std::vector<std::byte>;
inline void U32(Bytes& bytes,std::uint32_t value) { for(unsigned i=0;i<4;++i)bytes.push_back(std::byte(value>>(i*8))); }
inline void Name(Bytes& bytes,const char* name,unsigned count) {
    for(unsigned i=0;i<count;++i) { bytes.push_back(std::byte(*name));if(*name)++name; }
}
inline void Chunk(Bytes& bytes,unsigned id,const Bytes& body) {
    U32(bytes,id);U32(bytes,static_cast<unsigned>(body.size()));bytes.insert(bytes.end(),body.begin(),body.end());
}
inline Bytes Collection(unsigned optional_chunks=2) {
    Bytes bytes,header;
    U32(header,0x40002);Name(header,"COLLECTION",16);U32(header,2);U32(header,0);U32(header,0);
    Chunk(bytes,0x421,header);
    for(const char* name:{"Left","Right"}) {
        Bytes child;for(const char* c=name;*c;++c)child.push_back(std::byte(*c));child.push_back(std::byte(0));
        Chunk(bytes,0x422,child);
    }
    if(optional_chunks>0) {
        Bytes proxy;U32(proxy,0x10000);
        for(float value:{0.f,1.f,0.f, -1.f,0.f,0.f, 0.f,0.f,1.f, -.5f,0.f,0.f})U32(proxy,std::bit_cast<unsigned>(value));
        U32(proxy,4);Name(proxy,"LEFT",4);Chunk(bytes,0x423,proxy);
    }
    if(optional_chunks>1) {
        Bytes points;
        for(float value:{1.f,2.f,3.f})U32(points,std::bit_cast<unsigned>(value));
        Chunk(bytes,0x440,points);
    }
    return bytes;
}
inline Bytes LevelSet(unsigned count=2) {
    Bytes bytes,header;
    U32(header,0x10000);Name(header,"LEVELS",16);U32(header,count);Chunk(bytes,0x401,header);
    for(unsigned i=0;i<count;++i) {
        Bytes level;Name(level,i==0?"HIGH":"LOW",32);
        U32(level,std::bit_cast<unsigned>(float(i)));U32(level,std::bit_cast<unsigned>(float(i+10)));
        Chunk(bytes,0x402,level);
    }
    return bytes;
}
inline Bytes Aggregate(bool classification=true) {
    Bytes bytes,header,info;
    U32(header,0x10002);Name(header,"AGGREGATE",16);Chunk(bytes,0x601,header);
    Name(info,"BASE",32);U32(info,2);
    Name(info,"Left",32);Name(info,"LEFT",32);
    Name(info,"Right",32);Name(info,"RIGHT",32);Chunk(bytes,0x602,info);
    if(classification) {
        Bytes metadata;U32(metadata,23);U32(metadata,1);U32(metadata,0);U32(metadata,0);U32(metadata,0);
        Chunk(bytes,0x604,metadata);
    }
    return bytes;
}
inline Bytes Array(float screen,const std::vector<std::pair<const char*,unsigned>>& children) {
    Bytes bytes,header;U32(header,static_cast<unsigned>(children.size()));U32(header,std::bit_cast<unsigned>(screen));Chunk(bytes,0x703,header);
    for(const auto& child:children) { Bytes item;U32(item,child.second);Name(item,child.first,32);Chunk(bytes,0x704,item); }
    return bytes;
}
inline Bytes Hlod() {
    Bytes bytes,header;U32(header,0x10000);U32(header,2);Name(header,"MODEL",16);Name(header,"RIG",16);Chunk(bytes,0x701,header);
    Chunk(bytes,0x702,Array(100,{{"MODEL.Left",1},{"MODEL.Right",2}}));
    Chunk(bytes,0x702,Array(200,{{"MODEL.Low",2}}));
    Chunk(bytes,0x705,Array(0,{{"MODEL.Root",0}}));
    Chunk(bytes,0x706,Array(0,{{"SOCKET",2}}));return bytes;
}
inline Bytes Hmodel(bool insert_root) {
    Bytes bytes,header;U32(header,insert_root?0x20000:0x30000);Name(header,"MODEL",16);Name(header,"RIG",16);
    header.push_back(std::byte(3));header.resize(40);Chunk(bytes,0x301,header);
    unsigned index=0;
    for(const auto& child:std::vector<std::pair<const char*,unsigned>>{{"Left",1},{"Right",2},{"Root",0}}) {
        Bytes item;Name(item,child.first,16);
        const auto bone=insert_root?(child.second?child.second-1:65535):child.second;
        item.push_back(std::byte(bone));item.push_back(std::byte(bone>>8));Chunk(bytes,0x302+index++,item);
    }
    Bytes point;for(float value:{1.f,2.f,3.f})U32(point,std::bit_cast<unsigned>(value));Chunk(bytes,0x440,point);
    return bytes;
}
}
