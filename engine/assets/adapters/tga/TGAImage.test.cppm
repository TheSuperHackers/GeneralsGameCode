module;
#define BOOST_TEST_MODULE TGAImageTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <span>
#include <tuple>
#include <vector>
export module Assets.Adapters.TGA.Image.Tests;
import Assets.Adapters.TGA.Image;
import Assets.Images.Preparation;
using namespace Assets;

namespace
{
std::vector<std::byte> Header(unsigned width, unsigned height, unsigned bits, unsigned type, unsigned descriptor = 32)
{
    std::vector<std::byte> bytes(18);
    bytes[2] = std::byte(type); bytes[12] = std::byte(width); bytes[13] = std::byte(width >> 8);
    bytes[14] = std::byte(height); bytes[15] = std::byte(height >> 8);
    bytes[16] = std::byte(bits); bytes[17] = std::byte(descriptor);
    return bytes;
}
std::vector<std::byte> RGBA(const TGAImage& image)
{
    std::vector<PreparedImage> prepared;
    BOOST_REQUIRE(Prepare_Image_Levels(image.View(), PixelEncoding::RGBA8,
        image.info.width, image.info.height, 1, {}, prepared));
    return prepared.front().bytes;
}
}

BOOST_AUTO_TEST_CASE(truecolor_raw_and_packet_images_normalize_all_four_origins)
{
    const std::array<unsigned,4> colors{0x40ff0000,0x8000ff00,0xc00000ff,0xffffffff};
    const std::array<std::array<unsigned,4>,4> orders{{{2,3,0,1},{3,2,1,0},{0,1,2,3},{1,0,3,2}}};
    for (const unsigned bits : {24u,32u}) for (const bool rle : {false,true}) for (unsigned origin=0;origin<4;++origin) {
        auto bytes=Header(2,2,bits,rle ? 10 : 2,origin*16);
        bytes[0]=std::byte{2}; bytes.push_back(std::byte{'I'}); bytes.push_back(std::byte{'D'});
        if (rle) bytes.push_back(std::byte{3});
        for (const unsigned index : orders[origin]) for (unsigned channel=0;channel<bits/8;++channel)
            bytes.push_back(std::byte(colors[index]>>(channel*8)));
        const auto original=bytes;
        TGAImage image;
        BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
        const auto pixels=RGBA(image);
        for (unsigned i=0;i<4;++i) {
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4]),(colors[i]>>16)&255);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4+1]),(colors[i]>>8)&255);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4+2]),colors[i]&255);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4+3]),bits==24 ? 255 : colors[i]>>24);
        }
        BOOST_CHECK(bytes==original);
    }
}

BOOST_AUTO_TEST_CASE(indexed_images_keep_palette_origins_channel_order_and_precision)
{
    for (unsigned palette_bits : {16u,24u,32u}) for (const bool rle : {false,true}) {
        auto bytes=Header(2,1,8,rle ? 9 : 1);
        bytes[1]=std::byte{1}; bytes[3]=std::byte{3}; bytes[5]=std::byte{2}; bytes[7]=std::byte(palette_bits);
        const std::array colors = palette_bits==16 ? std::array{0xfc00u,0x801fu} : std::array{0x80ff0000u,0xff0000ffu};
        for (const unsigned color : colors) for (unsigned channel=0;channel<palette_bits/8;++channel)
            bytes.push_back(std::byte(color>>(channel*8)));
        if (rle) bytes.push_back(std::byte{1});
        bytes.push_back(std::byte{3}); bytes.push_back(std::byte{4});
        TGAImage image;
        BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
        BOOST_CHECK_EQUAL(image.palette.size(),5u*(palette_bits/8));
        const auto pixels=RGBA(image);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]),palette_bits==16 ? 248u : 255u);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[2]),0u);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[3]),palette_bits==32 ? 128u : 255u);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[4]),0u);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[6]),palette_bits==16 ? 248u : 255u);
        bytes.back()=std::byte{2};
        BOOST_CHECK(!Decode_TGA_Image(bytes,image));
        BOOST_CHECK(RGBA(image)==pixels);
    }
}

BOOST_AUTO_TEST_CASE(grayscale_and_alpha_survive_plain_and_repeated_packets)
{
    for (unsigned bits : {8u,16u}) for (const bool rle : {false,true}) {
        auto bytes=Header(16,16,bits,rle ? 11 : 3);
        for (unsigned half=0;half<2;++half) {
            if (rle) bytes.push_back(std::byte{255});
            for (unsigned count=0;count<(rle ? 1u : 128u);++count) {
                bytes.push_back(std::byte(half ? 160 : 80));
                if (bits==16) bytes.push_back(std::byte(half ? 128 : 64));
            }
        }
        TGAImage image;
        BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
        const auto pixels=RGBA(image);
        for (unsigned i=0;i<256;++i) {
            for (unsigned channel=0;channel<3;++channel)
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4+channel]),i<128 ? 80 : 160);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4+3]),bits==8 ? 255 : i<128 ? 64 : 128);
        }
    }
}

BOOST_AUTO_TEST_CASE(packed_truecolor_and_alpha_only_images_retain_authored_bits)
{
    auto bytes=Header(2,1,16,2);
    for (unsigned value : {0x00u,0xfcu,0x1fu,0x00u}) bytes.push_back(std::byte(value));
    TGAImage image;
    BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
    const auto pixels=RGBA(image);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]),248u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[3]),255u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[6]),248u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[7]),0u);
    bytes=Header(2,1,8,2); bytes.push_back(std::byte{32}); bytes.push_back(std::byte{128});
    BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
    BOOST_CHECK(image.info.encoding==PixelEncoding::Alpha8);
    const auto alpha=RGBA(image);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(alpha[3]),32u);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(alpha[7]),128u);
}

BOOST_AUTO_TEST_CASE(interleaved_scanlines_keep_uneven_row_groups)
{
    for (const bool four_way : {false,true}) {
        auto bytes=Header(1,5,8,3,32|(four_way ? 128 : 64));
        const auto order=four_way ? std::array{0,4,1,2,3} : std::array{0,2,4,1,3};
        for (const int value : order) bytes.push_back(std::byte(value));
        TGAImage image;
        BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
        for (unsigned i=0;i<5;++i) BOOST_CHECK_EQUAL(std::to_integer<unsigned>(image.pixels[i]),i);
    }
}

BOOST_AUTO_TEST_CASE(metadata_and_failed_decodes_never_publish_partial_images)
{
    auto valid=Header(2,2,32,10);
    valid.push_back(std::byte{131});
    for (unsigned value : {30u,20u,10u,80u}) valid.push_back(std::byte(value));
    TGAImage image;
    BOOST_REQUIRE(Decode_TGA_Image(valid,image));
    const auto original=image.pixels;
    TGAImageInfo info;
    BOOST_REQUIRE(Read_TGA_Info(std::span(valid).first(18),valid.size(),info));
    BOOST_CHECK_EQUAL(info.width,2u); BOOST_CHECK_EQUAL(info.height,2u);
    for (std::size_t size=0;size<valid.size();++size) {
        BOOST_CHECK(!Decode_TGA_Image(std::span(valid).first(size),image));
        BOOST_CHECK(image.pixels==original);
    }
    auto invalid=valid; invalid[18]=std::byte{132}; // Packet exceeds remaining pixels.
    BOOST_CHECK(!Decode_TGA_Image(invalid,image));
    for (const unsigned field : {1u,2u,16u,17u}) {
        invalid=valid; invalid[field]=std::byte{255};
        BOOST_CHECK(!Decode_TGA_Image(invalid,image));
    }
    invalid=Header(65535,65535,32,10); invalid.resize(32);
    BOOST_CHECK(!Decode_TGA_Image(invalid,image));
    BOOST_CHECK(!Read_TGA_Info(invalid,invalid.size(),info));
    BOOST_CHECK_EQUAL(info.width,2u);
    BOOST_CHECK(image.pixels==original);
}

BOOST_AUTO_TEST_CASE(mixed_packets_can_cross_scanline_boundaries)
{
    auto bytes=Header(3,2,8,11);
    for (unsigned value : {129u,10u,1u,20u,30u,129u,40u}) bytes.push_back(std::byte(value));
    TGAImage image;
    BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
    const std::array<unsigned,6> expected{10,10,20,30,40,40};
    for (unsigned i=0;i<expected.size();++i) BOOST_CHECK_EQUAL(std::to_integer<unsigned>(image.pixels[i]),expected[i]);
}

BOOST_AUTO_TEST_CASE(staged_tga_headers_and_representative_image_decodes)
{
    const char* directory=std::getenv("GENERALS_TGA_DIRECTORY");
    if (!directory) { BOOST_TEST_MESSAGE("Set GENERALS_TGA_DIRECTORY to validate supplied TGA assets."); return; }
    using Key=std::tuple<PixelEncoding,PixelEncoding,bool,bool,bool,unsigned>;
    std::map<Key,std::pair<std::uintmax_t,std::filesystem::path>> representatives;
    unsigned count=0;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension()!=".tga" && entry.path().extension()!=".TGA") continue;
        std::ifstream input(entry.path(),std::ios::binary);
        std::array<std::byte,18> header{};
        input.read(reinterpret_cast<char*>(header.data()),header.size());
        const auto size=entry.file_size();
        TGAImageInfo info;
        BOOST_TEST_CONTEXT(entry.path().string()) {
            BOOST_REQUIRE_EQUAL(input.gcount(),18);
            BOOST_REQUIRE(Read_TGA_Info(header,size,info));
        }
        const Key key{info.encoding,info.palette_encoding,info.top_origin,info.right_origin,
            info.run_length_encoded,info.row_interleave};
        const auto selected=representatives.find(key);
        if (selected==representatives.end() || size<selected->second.first)
            representatives[key]={size,entry.path()};
        ++count;
    }
    BOOST_REQUIRE_GT(count,0u);
    for (const auto& [key,entry] : representatives) {
        std::ifstream input(entry.second,std::ios::binary);
        std::vector<std::byte> bytes(entry.first);
        input.read(reinterpret_cast<char*>(bytes.data()),bytes.size());
        TGAImage image;
        BOOST_TEST_CONTEXT(entry.second.string()) {
            BOOST_REQUIRE_EQUAL(input.gcount(),bytes.size());
            BOOST_REQUIRE(Decode_TGA_Image(bytes,image));
            BOOST_CHECK(image.View().Is_Valid());
        }
    }
    BOOST_TEST_MESSAGE("Validated " << count << " staged TGA headers and decoded " << representatives.size() << " representative images.");
}
