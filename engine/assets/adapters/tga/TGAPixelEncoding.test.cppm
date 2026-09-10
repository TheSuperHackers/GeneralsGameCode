module;
#define BOOST_TEST_MODULE TGAPixelEncodingTests
#include <boost/test/included/unit_test.hpp>
export module Assets.Adapters.TGA.PixelEncoding.Tests;
import Assets.Adapters.TGA.PixelEncoding;
using namespace Assets;

BOOST_AUTO_TEST_CASE(source_headers_preserve_channel_layout_and_scalar_size)
{
    BOOST_CHECK(TGA_Pixel_Encoding(32,0,2)==PixelEncoding::BGRA8);
    BOOST_CHECK(TGA_Pixel_Encoding(24,0,2)==PixelEncoding::BGR8);
    BOOST_CHECK(TGA_Pixel_Encoding(16,0,2)==PixelEncoding::BGRA5551);
    BOOST_CHECK(TGA_Pixel_Encoding(8,1,1)==PixelEncoding::Indexed8);
    BOOST_CHECK(TGA_Pixel_Encoding(8,1,9)==PixelEncoding::Indexed8);
    BOOST_CHECK(TGA_Pixel_Encoding(8,0,3)==PixelEncoding::Luminance8);
    BOOST_CHECK(TGA_Pixel_Encoding(8,0,11)==PixelEncoding::Luminance8);
    BOOST_CHECK(TGA_Pixel_Encoding(16,0,3)==PixelEncoding::LuminanceAlpha88);
    BOOST_CHECK(TGA_Pixel_Encoding(16,0,11)==PixelEncoding::LuminanceAlpha88);
    BOOST_CHECK(TGA_Pixel_Encoding(8,0,2)==PixelEncoding::Alpha8);
    for (unsigned bits : {8u,16u,24u,32u})
        BOOST_CHECK_EQUAL(Pixel_Size(TGA_Pixel_Encoding(bits,0,2)),bits/8);
    for (unsigned bits : {0u,1u,15u,48u,64u}) {
        const auto encoding=TGA_Pixel_Encoding(bits,0,2);
        BOOST_CHECK(encoding==PixelEncoding::Unknown);
        BOOST_CHECK_EQUAL(Pixel_Size(encoding),0);
    }
}
