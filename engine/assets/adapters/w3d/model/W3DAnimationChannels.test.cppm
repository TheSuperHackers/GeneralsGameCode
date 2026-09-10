module;
#define BOOST_TEST_MODULE W3DAnimationChannelTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
export module Assets.Adapters.W3D.AnimationChannels.Tests;
import Assets.Adapters.W3D.AnimationChannels;
import Assets.Adapters.W3D.Chunks;
import Assets.ModelRig;
using namespace Assets;
using namespace Assets::W3D;
using Bytes=std::vector<std::byte>;
namespace {
void U16(Bytes& b,unsigned x) { b.push_back(std::byte(x));b.push_back(std::byte(x>>8)); }
void U32(Bytes& b,std::uint32_t x) { for(unsigned i=0;i<4;++i)b.push_back(std::byte(x>>(i*8))); }
void F32(Bytes& b,float x) { U32(b,std::bit_cast<std::uint32_t>(x)); }
Bytes Delta(unsigned count,unsigned width) {
    Bytes b;U32(b,count);U16(b,3);b.push_back(std::byte(width));b.push_back(std::byte(width==4?6:0));F32(b,.25f);
    for(unsigned i=0;i<width;++i) F32(b,float(i));
    const unsigned packets=(count-1+15)/16;
    for(unsigned p=0;p<packets;++p) for(unsigned i=0;i<width;++i) {
        b.push_back(std::byte{8});
        for(unsigned j=0;j<8;++j)b.push_back(std::byte{0x87}); // +7 then -8
    }
    return b;
}
}
BOOST_AUTO_TEST_CASE(raw_vectors_keep_range_axes_quaternions_and_trailing_exporter_bytes) {
    for(unsigned type=0;type<7;++type) {
        const unsigned width=type==6?4:1;
        Bytes b;for(unsigned v:{4u,5u,width,type,9u,0u})U16(b,v);
        for(unsigned i=0;i<width*2;++i)F32(b,float(i+1));
        F32(b,1234); // Authored count, not payload length, defines the samples.
        ModelAnimationChannel channel;std::string error;
        BOOST_REQUIRE(Parse_Animation_Channel(b,AnimationChannelEncoding::RawVector,channel,error));
        BOOST_CHECK_EQUAL(channel.bone,9u);BOOST_CHECK_EQUAL(channel.first_frame,4u);
        BOOST_REQUIRE_EQUAL(channel.samples.size(),2u);
        BOOST_CHECK_EQUAL(channel.samples[1][width-1],float(width*2));
        BOOST_CHECK(!channel.hold_endpoints);
    }
}
BOOST_AUTO_TEST_CASE(visibility_bits_and_timed_steps_retain_authored_values) {
    Bytes bits;for(unsigned v:{3,12,0,2})U16(bits,v);
    bits.push_back(std::byte{1});bits.push_back(std::byte{0x81});bits.push_back(std::byte{2});
    ModelAnimationChannel channel;std::string error;
    BOOST_REQUIRE(Parse_Animation_Channel(bits,AnimationChannelEncoding::RawVisibility,channel,error));
    BOOST_CHECK(channel.default_visible);BOOST_REQUIRE_EQUAL(channel.samples.size(),10u);
    for(unsigned i=0;i<10;++i)BOOST_CHECK_EQUAL(channel.samples[i][0],i==0||i==7||i==9?1.f:0.f);
    Bytes timed;U32(timed,3);U16(timed,4);timed.push_back(std::byte{1});timed.push_back(std::byte{0});
    for(unsigned key:{0u,0x80000004u,8u}) { U32(timed,key);F32(timed,float(key&15)); }
    BOOST_REQUIRE(Parse_Animation_Channel(timed,AnimationChannelEncoding::TimedVector,channel,error));
    BOOST_REQUIRE_EQUAL(channel.key_frames.size(),3u);BOOST_CHECK_EQUAL(channel.key_frames[1],4u);
    BOOST_CHECK_EQUAL(channel.step_into_key[1],1u);BOOST_CHECK(channel.hold_endpoints);
    Bytes timed_bits;U32(timed_bits,2);U16(timed_bits,5);timed_bits.push_back(std::byte{0});timed_bits.push_back(std::byte{1});
    U32(timed_bits,0x80000002u);U32(timed_bits,9);
    BOOST_REQUIRE(Parse_Animation_Channel(timed_bits,AnimationChannelEncoding::TimedVisibility,channel,error));
    BOOST_CHECK_EQUAL(channel.key_frames[0],2u);BOOST_CHECK_EQUAL(channel.samples[0][0],1);
    BOOST_CHECK_EQUAL(channel.samples[1][0],0);
}
BOOST_AUTO_TEST_CASE(delta_decoding_crosses_packet_boundaries_for_every_component) {
    for(unsigned width:{1u,4u}) for(unsigned count:{1u,2u,16u,17u,18u,34u}) {
        ModelAnimationChannel channel;std::string error;
        BOOST_REQUIRE(Parse_Animation_Channel(Delta(count,width),AnimationChannelEncoding::DeltaVector,channel,error));
        BOOST_REQUIRE_EQUAL(channel.samples.size(),count);
        for(unsigned i=0;i<width;++i) {
            float expected=float(i);
            for(unsigned frame=0;frame<count;++frame) {
                if(frame)expected+=frame%2?1.75f:-2.f;
                BOOST_CHECK_EQUAL(channel.samples[frame][i],expected);
            }
        }
    }
}
BOOST_AUTO_TEST_CASE(malformed_channels_do_not_publish_partial_data) {
    ModelAnimationChannel channel;channel.bone=123;std::string error;
    const auto source=Delta(18,4);
    for(std::size_t size=0;size<source.size();++size)
        BOOST_CHECK(!Parse_Animation_Channel({source.data(),size},AnimationChannelEncoding::DeltaVector,channel,error));
    BOOST_CHECK_EQUAL(channel.bone,123u);
    auto invalid=source;invalid[6]=std::byte{255};
    BOOST_CHECK(!Parse_Animation_Channel(invalid,AnimationChannelEncoding::DeltaVector,channel,error));
    invalid=source;for(unsigned i=0;i<4;++i)invalid[i]=std::byte{255};
    BOOST_CHECK(!Parse_Animation_Channel(invalid,AnimationChannelEncoding::DeltaVector,channel,error));
    Bytes timed;U32(timed,2);U16(timed,4);timed.push_back(std::byte{1});timed.push_back(std::byte{0});
    U32(timed,8);F32(timed,1);U32(timed,4);F32(timed,2);
    BOOST_CHECK(!Parse_Animation_Channel(timed,AnimationChannelEncoding::TimedVector,channel,error));
    BOOST_CHECK_EQUAL(channel.bone,123u);
}
BOOST_AUTO_TEST_CASE(real_game_animation_channels_decode_without_dropping_formats) {
    // Optional local corpus: extracted retail Generals/Zero Hour Art/W3D files.
    // No proprietary asset is copied into the repository or required by CI.
    const auto* directory=std::getenv("GENERALS_W3D_ANIMATION_TEST_DIRECTORY");
    if(!directory) { BOOST_TEST_MESSAGE("Retail animation corpus not configured"); return; }
    std::array<unsigned,5> counts{};
    unsigned malformed_containers=0;
    for(const auto& entry:std::filesystem::directory_iterator(directory)) {
        if(!entry.is_regular_file())continue;
        auto extension=entry.path().extension().string();
        if(extension!=".W3D" && extension!=".w3d")continue;
        std::ifstream input(entry.path(),std::ios::binary|std::ios::ate);
        const auto size=input.tellg();BOOST_REQUIRE(size>=0);input.seekg(0);
        Bytes bytes(static_cast<std::size_t>(size));input.read(reinterpret_cast<char*>(bytes.data()),size);
        BOOST_REQUIRE(input.good());
        const bool valid_file=W3DVisit_Chunks(bytes,[&](const W3DChunkView& animation) {
            if(animation.id!=0x200 && animation.id!=0x280)return true;
            unsigned flavor=0;
            return W3DVisit_Chunks(animation.payload,[&](const W3DChunkView& chunk) {
                if(chunk.id==0x281) { BOOST_REQUIRE(chunk.payload.size()>=44); flavor=std::to_integer<unsigned>(chunk.payload[42]);return true; }
                AnimationChannelEncoding encoding;
                if(chunk.id==0x202)encoding=AnimationChannelEncoding::RawVector;
                else if(chunk.id==0x203)encoding=AnimationChannelEncoding::RawVisibility;
                else if(chunk.id==0x282)encoding=flavor==0?AnimationChannelEncoding::TimedVector:AnimationChannelEncoding::DeltaVector;
                else if(chunk.id==0x283)encoding=AnimationChannelEncoding::TimedVisibility;
                else return true;
                ModelAnimationChannel channel;std::string error;
                BOOST_REQUIRE_MESSAGE(Parse_Animation_Channel(chunk.payload,encoding,channel,error),entry.path().string()+": "+error);
                BOOST_CHECK(!channel.samples.empty());++counts[static_cast<unsigned>(encoding)];return true;
            });
        });
        if(!valid_file) {
            // These five supplied exports have corrupt chunk lengths. For
            // example, idel declares an 805306370-byte child with only 81240
            // bytes left in its parent. Keep them rejected; no channel decoder
            // can repair their container boundaries. Other invalid files fail.
            const auto name=entry.path().filename();
            BOOST_REQUIRE_MESSAGE(name=="UISabotr_idel.w3d" || name=="UISabotr_Jump.w3d"
                || name=="UISabotr_Left.w3d" || name=="UISabotr_Right.w3d" || name=="UISabotr_Up.w3d",
                entry.path().string()+": invalid chunk stream");
            ++malformed_containers;
        }
    }
    BOOST_CHECK_GT(counts[0],0u);
    BOOST_CHECK_GT(counts[2]+counts[4],0u);
    BOOST_TEST_MESSAGE("Decoded channels: raw="<<counts[0]<<", bits="<<counts[1]<<", timed="<<counts[2]<<", timed bits="<<counts[3]<<", delta="<<counts[4]);
    BOOST_TEST_MESSAGE("Known malformed containers rejected: "<<malformed_containers);
}
