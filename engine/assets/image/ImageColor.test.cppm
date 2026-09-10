module;
#define NOMINMAX
#define BOOST_TEST_MODULE ImageColorTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <limits>
#include <span>
export module Assets.Images.Color.Tests;
import Assets.Images.Color;
using namespace Assets;

BOOST_AUTO_TEST_CASE(packed_pixel_and_region_writes_preserve_bounds_and_neighboring_alpha)
{
    auto image=ImageBuffer::Create(4,3,PixelEncoding::BGRA8);
    BOOST_REQUIRE(image);
    BOOST_REQUIRE(Fill_Packed_Image_Region(*image,{0,0,4,3},0x80201008));
    BOOST_REQUIRE(Fill_Packed_Image_Region(*image,{1,1,3,2},0x4000ff00));
    BOOST_REQUIRE(Write_Packed_Image_Pixel(*image,3,2,0xff0000ff));
    BOOST_CHECK(!Write_Packed_Image_Pixel(*image,-1,0,0));
    BOOST_CHECK(!Write_Packed_Image_Pixel(*image,4,0,0));
    BOOST_CHECK(!Fill_Packed_Image_Region(*image,{0,0,5,1},0));
    for (unsigned y=0;y<3;++y) for (unsigned x=0;x<4;++x) {
        unsigned pixel=0;
        BOOST_REQUIRE(Read_Image_Pixel(image->Bytes().subspan(y*16+x*4,4),image->Encoding(),pixel));
        const unsigned expected=y==2 && x==3 ? 0xff0000ff : y==1 && (x==1 || x==2) ? 0x4000ff00 : 0x80201008;
        BOOST_CHECK_EQUAL(pixel,expected);
    }
}

BOOST_AUTO_TEST_CASE(rgb_replacement_preserves_alpha_and_packed_channel_truncation)
{
    for (auto encoding : {PixelEncoding::BGRA8,PixelEncoding::BGRX8,PixelEncoding::BGR8,
        PixelEncoding::BGRA4444,PixelEncoding::BGRA5551,PixelEncoding::BGR565}) {
        std::array<std::byte,4> pixel{std::byte{0x23},std::byte{0x81},std::byte{0x45},std::byte{0x67}};
        const auto original=pixel;
        BOOST_REQUIRE(Replace_Image_RGB(pixel,encoding,{1,0.5f,0}));
        unsigned argb=0;
        BOOST_REQUIRE(Read_Image_Pixel(pixel,encoding,argb));
        const unsigned red=encoding==PixelEncoding::BGRA4444 ? 240 : Pixel_Size(encoding)==2 ? 248 : 255;
        const unsigned green=encoding==PixelEncoding::BGRA4444 ? 112 : Pixel_Size(encoding)==2 ?
            (encoding==PixelEncoding::BGR565 ? 124 : 120) : 127;
        BOOST_CHECK_EQUAL((argb>>16)&255,red);
        BOOST_CHECK_EQUAL((argb>>8)&255,green);
        BOOST_CHECK_EQUAL(argb&255,0u);
        if (encoding==PixelEncoding::BGRA8 || encoding==PixelEncoding::BGRX8)
            BOOST_CHECK(pixel[3]==original[3]);
        if (encoding==PixelEncoding::BGRA4444)
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixel[1])&0xf0,0x80u);
        if (encoding==PixelEncoding::BGRA5551)
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixel[1])&0x80,0x80u);
    }
}

BOOST_AUTO_TEST_CASE(invalid_rgb_and_short_pixels_do_not_change_storage)
{
    const std::array<std::byte,4> original{std::byte{1},std::byte{2},std::byte{3},std::byte{4}};
    for (const float channel : {-0.01f,1.01f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
        auto pixel=original;
        BOOST_CHECK(!Replace_Image_RGB(pixel,PixelEncoding::BGRA8,{channel,0,0}));
        BOOST_CHECK(pixel==original);
    }
    auto pixel=original;
    BOOST_CHECK(!Replace_Image_RGB(std::span(pixel).first(3),PixelEncoding::BGRA8,{1,0,0}));
    BOOST_CHECK(!Replace_Image_RGB(pixel,PixelEncoding::BC1,{1,0,0}));
    BOOST_CHECK(pixel==original);
}
