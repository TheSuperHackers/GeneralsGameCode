module;
#define BOOST_TEST_MODULE W3DRigTests
#include <boost/test/included/unit_test.hpp>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <span>
#include <vector>
export module Assets.Adapters.W3D.Rig.Tests;
import Assets.Adapters.W3D.Rig;
import Assets.Adapters.W3D.Chunks;
import Assets.ModelRig;
using Bytes=std::vector<std::byte>;
namespace {
void U32(Bytes& b,std::uint32_t x) { for(unsigned i=0;i<4;++i)b.push_back(std::byte(x>>(8*i))); }
void U16(Bytes& b,std::uint16_t x) { b.push_back(std::byte(x));b.push_back(std::byte(x>>8)); }
void F32(Bytes& b,float x) { U32(b,std::bit_cast<std::uint32_t>(x)); }
void Name(Bytes& b,const char* n,unsigned width=16) { unsigned i=0;for(;n[i]&&i<width;++i)b.push_back(std::byte(n[i]));for(;i<width;++i)b.push_back(std::byte{}); }
void Chunk(Bytes& b,unsigned id,const Bytes& p,bool children=false) { U32(b,id);U32(b,unsigned(p.size())|(children?0x80000000u:0));b.insert(b.end(),p.begin(),p.end()); }
Bytes Hierarchy(unsigned parent=0) {
    Bytes header;U32(header,0x40001);Name(header,"RIG");U32(header,2);for(int i=0;i<3;++i)F32(header,0);
    Bytes pivots;
    for(unsigned i=0;i<2;++i) {
        Name(pivots,i?"HINGE":"ROOT");U32(pivots,i?parent:0xffffffffu);
        F32(pivots,i?3:0);for(int j=0;j<8;++j)F32(pivots,0);F32(pivots,1);
    }
    Bytes body,result;Chunk(body,0x101,header);Chunk(body,0x102,pivots);Chunk(result,0x100,body,true);return result;
}
void Animation(Bytes& result,bool truncate=false) {
    Bytes header;U32(header,0x40001);Name(header,"OPEN");Name(header,"RIG");U32(header,41);U32(header,30);
    Bytes channel;for(auto v:{10,11,1,2,1,0})U16(channel,std::uint16_t(v));F32(channel,2);if(!truncate)F32(channel,5);
    Bytes bits;for(auto v:{10,11,0,1})U16(bits,std::uint16_t(v));bits.push_back(std::byte{1});bits.push_back(std::byte{2});
    Bytes body;Chunk(body,0x201,header);Chunk(body,0x202,channel);Chunk(body,0x203,bits);Chunk(result,0x200,body,true);
}
}
BOOST_AUTO_TEST_CASE(retains_rig_channels_and_visibility_defaults) {
    auto bytes=Hierarchy();Animation(bytes);Assets::ModelRigDesc rig;std::string error;
    BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Model_Rig(bytes,rig,error),error);
    BOOST_REQUIRE_EQUAL(rig.bones.size(),2u);BOOST_TEST(rig.bones[1].translation.x==3);
    BOOST_REQUIRE_EQUAL(rig.animations.size(),1u);const auto& clip=rig.animations[0];
    BOOST_TEST(clip.frame_count==41u);BOOST_TEST(clip.frame_rate==30);
    BOOST_REQUIRE_EQUAL(clip.channels.size(),2u);BOOST_TEST(clip.channels[0].first_frame==10u);
    BOOST_TEST(clip.channels[0].samples[1][0]==5);BOOST_TEST(clip.channels[1].default_visible);
    BOOST_TEST(clip.channels[1].samples[0][0]==0);BOOST_TEST(clip.channels[1].samples[1][0]==1);
}
BOOST_AUTO_TEST_CASE(rejects_cycles_duplicate_chunks_and_truncation_without_partial_output) {
    Assets::ModelRigDesc rig;rig.skeleton_name="preserved";std::string error;
    BOOST_TEST(!Assets::W3D::W3DRead_Model_Rig(Hierarchy(1),rig,error));
    auto bytes=Hierarchy();auto duplicate=bytes;bytes.insert(bytes.end(),duplicate.begin(),duplicate.end());
    BOOST_TEST(!Assets::W3D::W3DRead_Model_Rig(bytes,rig,error));
    bytes=Hierarchy();Animation(bytes,true);BOOST_TEST(!Assets::W3D::W3DRead_Model_Rig(bytes,rig,error));
    BOOST_TEST(rig.skeleton_name=="preserved");
}
BOOST_AUTO_TEST_CASE(empty_compressed_clip_is_ready_and_preserves_playback_rate) {
    Bytes header;U32(header,1);Name(header,"RUN");Name(header,"RIG");U32(header,80);U16(header,30);U16(header,1);
    Bytes body,bytes;Chunk(body,0x281,header);Chunk(bytes,0x280,body,true);
    Assets::ModelRigDesc rig;std::string error;BOOST_REQUIRE(Assets::W3D::W3DRead_Model_Rig(bytes,rig,error));
    BOOST_TEST(rig.animations[0].channels_available);BOOST_TEST(rig.animations[0].frame_rate==30);
}
BOOST_AUTO_TEST_CASE(hierarchy_payload_preserves_old_root_indices_and_full_width_names) {
    auto file=Hierarchy();
    Bytes body(file.begin()+8,file.end());
    // Change only the version: both authored bones now follow an inserted root.
    body[10]=std::byte{2};body[8]=std::byte{0};
    Assets::ModelRigDesc rig;std::string error;
    BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Hierarchy(body,rig,error),error);
    BOOST_REQUIRE_EQUAL(rig.bones.size(),3u);
    BOOST_TEST(rig.bones[0].name=="RootTransform");
    BOOST_TEST(rig.bones[0].parent==Assets::ModelRootParent);
    BOOST_TEST(rig.bones[1].parent==0u);BOOST_TEST(rig.bones[2].parent==1u);
    BOOST_TEST(rig.bones[2].translation.x==3.f);
    const std::string full_name="1234567890ABCDEF";
    for(unsigned i=0;i<16;++i)body[12+i]=std::byte(full_name[i]);
    BOOST_REQUIRE(Assets::W3D::W3DRead_Hierarchy(body,rig,error));
    BOOST_TEST(rig.skeleton_name==full_name);
}
BOOST_AUTO_TEST_CASE(hierarchy_rejects_partial_or_invalid_payloads_without_replacing_loaded_rig) {
    const auto file=Hierarchy();const Bytes body(file.begin()+8,file.end());
    Assets::ModelRigDesc rig;rig.skeleton_name="retained";rig.bones={{"retained"}};
    std::string error;
    for(std::size_t size=0;size<body.size();++size) {
        BOOST_TEST(!Assets::W3D::W3DRead_Hierarchy({body.data(),size},rig,error));
        BOOST_TEST(rig.skeleton_name=="retained");BOOST_TEST(rig.bones.size()==1u);
    }
    auto duplicate=body;duplicate.insert(duplicate.end(),body.begin(),body.end());
    BOOST_TEST(!Assets::W3D::W3DRead_Hierarchy(duplicate,rig,error));
    auto invalid=body;
    // The second pivot cannot parent itself, even after old root insertion.
    invalid[52+60+16]=std::byte{1};
    BOOST_TEST(!Assets::W3D::W3DRead_Hierarchy(invalid,rig,error));
    invalid[8]=std::byte{0};invalid[10]=std::byte{2};
    BOOST_TEST(!Assets::W3D::W3DRead_Hierarchy(invalid,rig,error));
    invalid=body;invalid[28]=std::byte{255};
    BOOST_TEST(!Assets::W3D::W3DRead_Hierarchy(invalid,rig,error));
    BOOST_TEST(rig.skeleton_name=="retained");
}
BOOST_AUTO_TEST_CASE(retail_hierarchies_use_the_same_bounded_parser_as_runtime) {
    const auto* directory=std::getenv("GENERALS_W3D_ANIMATION_TEST_DIRECTORY");
    if(!directory) { BOOST_TEST_MESSAGE("Retail hierarchy corpus not configured"); return; }
    unsigned hierarchies=0,bones=0,animations=0,channels=0,malformed_containers=0;
    for(const auto& entry:std::filesystem::directory_iterator(directory)) {
        const auto extension=entry.path().extension().string();
        if(!entry.is_regular_file() || (extension!=".w3d" && extension!=".W3D"))continue;
        std::ifstream input(entry.path(),std::ios::binary|std::ios::ate);
        const auto size=input.tellg();BOOST_REQUIRE(size>=0);input.seekg(0);
        Bytes bytes(static_cast<std::size_t>(size));input.read(reinterpret_cast<char*>(bytes.data()),size);
        BOOST_REQUIRE(input.good());
        const bool valid_file=Assets::W3D::W3DVisit_Chunks(bytes,[&](const Assets::W3D::W3DChunkView& chunk) {
            if(chunk.id==0x200 || chunk.id==0x280) {
                if(!Assets::W3D::W3DVisit_Chunks(chunk.payload,[](const auto&) { return true; }))return false;
                Assets::ModelAnimationDesc clip;std::string error;
                const bool decoded=Assets::W3D::W3DRead_Animation(chunk.payload,chunk.id==0x280,clip,error);
                BOOST_REQUIRE_MESSAGE(decoded,entry.path().string()+": "+error);
                ++animations;channels+=unsigned(clip.channels.size());return true;
            }
            if(chunk.id!=0x100)return true;
            Assets::ModelRigDesc rig;std::string error;
            BOOST_REQUIRE_MESSAGE(Assets::W3D::W3DRead_Hierarchy(chunk.payload,rig,error),entry.path().string()+": "+error);
            ++hierarchies;bones+=unsigned(rig.bones.size());return true;
        });
        if(!valid_file) {
            // The same five malformed animation containers are documented by
            // the channel corpus test. Hierarchy decoding failures remain fatal.
            const auto name=entry.path().filename();
            BOOST_REQUIRE_MESSAGE(name=="UISabotr_idel.w3d" || name=="UISabotr_Jump.w3d"
                || name=="UISabotr_Left.w3d" || name=="UISabotr_Right.w3d" || name=="UISabotr_Up.w3d",
                entry.path().string()+": invalid chunk stream");
            ++malformed_containers;
        }
    }
    BOOST_TEST(hierarchies>0u);BOOST_TEST(bones>=hierarchies);
    BOOST_TEST_MESSAGE("Decoded hierarchies="<<hierarchies<<", bones="<<bones);
    BOOST_TEST(animations>0u);BOOST_TEST(channels>0u);
    BOOST_TEST_MESSAGE("Decoded animations="<<animations<<", channels="<<channels);
    BOOST_TEST_MESSAGE("Known malformed containers rejected="<<malformed_containers);
}

BOOST_AUTO_TEST_CASE(pose_key_decoding_is_bounded_atomic_and_retains_integer_frames) {
    Bytes bytes;
    for(unsigned value:{2u,16777217u,8u,1u})U32(bytes,value);
    std::vector<Assets::ModelPoseKey> keys;std::string error;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Pose_Keys(bytes,keys,error));
    BOOST_CHECK_EQUAL(keys.size(),2);BOOST_CHECK_EQUAL(keys[0].pose_frame,16777217u);
    for(unsigned size=0;size<bytes.size();++size) {
        if(size==8)continue;
        BOOST_CHECK(!Assets::W3D::W3DRead_Pose_Keys(std::span(bytes).first(size),keys,error));
        BOOST_CHECK_EQUAL(keys.size(),2);BOOST_CHECK_EQUAL(keys[1].pose_frame,1);
    }
    for(unsigned time:{0u,2u}) {
        auto invalid=bytes;invalid[8]=std::byte(time);
        BOOST_CHECK(!Assets::W3D::W3DRead_Pose_Keys(invalid,keys,error));
        BOOST_CHECK_EQUAL(keys[0].pose_frame,16777217u);
    }
}

BOOST_AUTO_TEST_CASE(animation_payloads_preserve_old_root_offsets_extensions_and_atomic_failure) {
    Bytes file;Animation(file);
    Assets::W3D::W3DChunkView container;
    BOOST_REQUIRE(Assets::W3D::W3DVisit_Chunks(file,[&](const auto& chunk) { container=chunk;return true; }));
    Bytes bytes(container.payload.begin(),container.payload.end());
    // Header starts after its chunk header. Old exports omit the object root.
    bytes[8]=std::byte{0};bytes[9]=std::byte{0};bytes[10]=std::byte{2};bytes[11]=std::byte{0};
    Chunk(bytes,0x123456,Bytes{std::byte{9}});
    Assets::ModelAnimationDesc clip;std::string error;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Animation(bytes,false,clip,error));
    BOOST_CHECK_EQUAL(clip.channels[0].bone,2);
    BOOST_CHECK_EQUAL(clip.name,"OPEN");
    bytes.pop_back();
    BOOST_CHECK(!Assets::W3D::W3DRead_Animation(bytes,false,clip,error));
    BOOST_CHECK_EQUAL(clip.channels[0].bone,2);
}

BOOST_AUTO_TEST_CASE(pose_animation_decodes_dependencies_and_mapping_without_partial_publication) {
    Bytes header;U32(header,1);Name(header,"FACE");Name(header,"RIG");U32(header,9);F32(header,30);U32(header,2);
    Bytes bytes;Chunk(bytes,0x2c1,header);
    for(unsigned index=0;index<2;++index) {
        Bytes channel,name,keys;Name(name,index?"RIG.MOUTH":"RIG.EXPRESSION",20);
        U32(keys,0);U32(keys,index?3:7);U32(keys,8);U32(keys,1);
        Chunk(channel,0x2c3,name);Chunk(channel,0x2c4,keys);Chunk(bytes,0x2c2,channel,true);
    }
    const auto mapping_offset=bytes.size();
    Bytes mapping;U32(mapping,0);U32(mapping,1);U32(mapping,0);Chunk(bytes,0x2c5,mapping);
    Assets::ModelPoseAnimationDesc clip;std::string error;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Pose_Animation(bytes,clip,error));
    BOOST_CHECK_EQUAL(clip.name,"FACE");BOOST_CHECK_EQUAL(clip.frame_rate,30);
    BOOST_CHECK_EQUAL(clip.channels[1].animation_name,"RIG.MOUTH");
    BOOST_CHECK_EQUAL(clip.channels[0].keys[0].pose_frame,7);
    BOOST_CHECK_EQUAL(clip.bone_channels[1],1);
    for(std::size_t size=0;size<bytes.size();++size) {
        if(size==mapping_offset)continue; // mapping is optional, defaulting to channel zero
        BOOST_CHECK(!Assets::W3D::W3DRead_Pose_Animation(std::span(bytes).first(size),clip,error));
        BOOST_CHECK_EQUAL(clip.channels.size(),2);
        BOOST_CHECK_EQUAL(clip.bone_channels[1],1);
    }
    auto invalid=bytes;invalid[mapping_offset+12]=std::byte{2};
    BOOST_CHECK(!Assets::W3D::W3DRead_Pose_Animation(invalid,clip,error));
    invalid=bytes;Chunk(invalid,0x2c1,header);
    BOOST_CHECK(!Assets::W3D::W3DRead_Pose_Animation(invalid,clip,error));
    invalid=bytes;invalid[52]=std::byte{3}; // declared channel count
    BOOST_CHECK(!Assets::W3D::W3DRead_Pose_Animation(invalid,clip,error));
    BOOST_REQUIRE(Assets::W3D::W3DRead_Pose_Animation(std::span(bytes).first(mapping_offset),clip,error));
    BOOST_CHECK(clip.bone_channels.empty());
}
