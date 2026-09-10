module;
#define NOMINMAX
#define BOOST_TEST_MODULE ImageBufferTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <vector>
export module Assets.Images.Buffer.Tests;
import Assets.Images.Buffer;
using namespace Assets;

BOOST_AUTO_TEST_CASE(storage_keeps_block_tails_and_rejects_impossible_rows)
{
    for (auto encoding : {PixelEncoding::BGRA8,PixelEncoding::BGR8,PixelEncoding::BC1,PixelEncoding::BC2,PixelEncoding::BC3}) {
        auto image=ImageBuffer::Create(5,3,encoding);
        BOOST_REQUIRE(image);
        const unsigned pitch=Is_Block_Compressed(encoding) ? (encoding==PixelEncoding::BC1 ? 16 : 32) : Pixel_Size(encoding)*5;
        BOOST_CHECK_EQUAL(image->Row_Pitch(),pitch);
        BOOST_CHECK_EQUAL(image->Bytes().size(),pitch*(Is_Block_Compressed(encoding) ? 1 : 3));
    }
    BOOST_CHECK(!ImageBuffer::Create(0,1,PixelEncoding::BGRA8));
    BOOST_CHECK(!ImageBuffer::Create(1,1,PixelEncoding::Unknown));
    BOOST_CHECK(!ImageBuffer::Create(std::numeric_limits<unsigned>::max(),1,PixelEncoding::BGRA8));
}

BOOST_AUTO_TEST_CASE(copy_preserves_overlapping_bytes_and_endpoint_resampling)
{
    auto image=ImageBuffer::Create(7,2,PixelEncoding::Alpha8);
    BOOST_REQUIRE(image);
    for (unsigned i=0;i<14;++i) image->Bytes()[i]=std::byte(i+1);
    BOOST_REQUIRE(Copy_Image_Region(*image,{0,0,6,2},*image,{1,0,7,2}));
    const std::array<unsigned,14> expected{1,1,2,3,4,5,6,8,8,9,10,11,12,13};
    for (unsigned i=0;i<14;++i) BOOST_CHECK_EQUAL(std::to_integer<unsigned>(image->Bytes()[i]),expected[i]);
    auto destination=ImageBuffer::Create(4,3,PixelEncoding::Alpha8);
    BOOST_REQUIRE(destination);
    BOOST_REQUIRE(Copy_Image_Region(*image,{2,0,5,2},*destination,{0,0,4,3}));
    const std::array<unsigned,12> scaled{2,2,3,4,2,2,3,4,9,9,10,11};
    for (unsigned i=0;i<12;++i) BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination->Bytes()[i]),scaled[i]);
}

BOOST_AUTO_TEST_CASE(conversion_and_invalid_regions_preserve_alpha_and_unwritten_pixels)
{
    auto source=ImageBuffer::Create(2,1,PixelEncoding::BGRA4444);
    auto destination=ImageBuffer::Create(4,2,PixelEncoding::BGRA8);
    BOOST_REQUIRE(source); BOOST_REQUIRE(destination);
    source->Bytes()[0]=std::byte{0x23}; source->Bytes()[1]=std::byte{0x81};
    source->Bytes()[2]=std::byte{0x56}; source->Bytes()[3]=std::byte{0x04};
    std::ranges::fill(destination->Bytes(),std::byte{0xcc});
    BOOST_REQUIRE(Copy_Image_Region(*source,{0,0,2,1},*destination,{1,1,3,2}));
    const std::array<unsigned,8> converted{48,32,16,128,96,80,64,0};
    for (unsigned i=0;i<8;++i) BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination->Bytes()[20+i]),converted[i]);
    const std::vector<std::byte> saved(destination->Bytes().begin(),destination->Bytes().end());
    for (const auto region : {ImageRegion{-1,0,1,1},ImageRegion{0,0,3,1},ImageRegion{1,0,1,1},ImageRegion{0,0,1,2}}) {
        BOOST_CHECK(!Copy_Image_Region(*source,region,*destination,{0,0,1,1}));
        BOOST_CHECK(std::ranges::equal(destination->Bytes(),saved));
    }
    BOOST_CHECK(!Copy_Image_Region(*source,{0,0,2,1},*destination,{3,0,5,1}));
    BOOST_CHECK(std::ranges::equal(destination->Bytes(),saved));
}
