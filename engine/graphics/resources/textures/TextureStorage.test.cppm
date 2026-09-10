module;
#define NOMINMAX
#define BOOST_TEST_MODULE TextureStorageTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <limits>
export module Graphics.Resources.Textures.Storage.Tests;
import Graphics.Resources.Textures.Storage;
import Graphics.RHI;
using namespace Graphics;
using E=Assets::PixelEncoding;

BOOST_AUTO_TEST_CASE(storage_selection_preserves_precision_compression_and_editing_layouts)
{
    for (auto encoding : {E::BGRA8,E::BGRX8,E::BGR565,E::BGRA5551,E::BGRA4444,
        E::Alpha8,E::Luminance8,E::LuminanceAlpha88,E::RG8_SNorm}) {
        BOOST_CHECK(Texture_Storage_Format(encoding)!=RHITextureFormat::Unknown);
        BOOST_CHECK(Select_Texture_Encoding(encoding,false,false)==encoding);
    }
    BOOST_CHECK(Select_Texture_Encoding(E::BGR8,false,false)==E::BGRX8);
    BOOST_CHECK(Select_Texture_Encoding(E::BGRA8,false,true)==E::BGRA4444);
    BOOST_CHECK(Select_Texture_Encoding(E::BGRX8,false,true)==E::BGR565);
    BOOST_CHECK(Select_Texture_Encoding(E::BGR8,false,true)==E::BGR565);
    for (auto encoding : {E::BC1,E::BC2,E::BC2Premultiplied,E::BC3,E::BC3Premultiplied}) {
        BOOST_CHECK(Assets::Is_Block_Compressed(encoding));
        BOOST_CHECK_EQUAL(Assets::Pixel_Size(encoding),0);
        BOOST_CHECK(Select_Texture_Encoding(encoding,true,true)==encoding);
        BOOST_CHECK(Select_Texture_Encoding(encoding,true,false)==encoding);
        BOOST_CHECK(Select_Texture_Encoding(encoding,false,false)==(encoding==E::BC1 ? E::BGRX8 : E::BGRA8));
        BOOST_CHECK(Select_Texture_Encoding(encoding,false,true)==(encoding==E::BC1 ? E::BGR565 : E::BGRA4444));
    }
    BOOST_CHECK(Texture_Storage_Format(E::BC1)==RHITextureFormat::BC1_UNorm);
    BOOST_CHECK(Texture_Storage_Format(E::BC2)==RHITextureFormat::BC2_UNorm);
    BOOST_CHECK(Texture_Storage_Format(E::BC2Premultiplied)==RHITextureFormat::BC2_UNorm);
    BOOST_CHECK(Texture_Storage_Format(E::BC3)==RHITextureFormat::BC3_UNorm);
    BOOST_CHECK(Texture_Storage_Format(E::BC3Premultiplied)==RHITextureFormat::BC3_UNorm);
    for (auto encoding : {E::Unknown,E::Indexed8,E::IndexedAlpha88,E::RGB332Alpha8,E::RG5_SNorm_L6}) {
        BOOST_CHECK(Texture_Storage_Format(encoding)==RHITextureFormat::Unknown);
        BOOST_CHECK(Select_Texture_Encoding(encoding,false,false)==E::BGRA8);
    }
    for (auto encoding : {E::BGRA5551,E::BGRA4444,E::Luminance8})
        BOOST_CHECK(Editable_Texture_Encoding(encoding)==E::BGRA8);
    for (auto encoding : {E::BGRA8,E::BGR565,E::BC1,E::BC2,E::BC3,E::Alpha8})
        BOOST_CHECK(Editable_Texture_Encoding(encoding)==encoding);
}

BOOST_AUTO_TEST_CASE(texture_extents_round_and_cap_without_integer_overflow)
{
    const RHITextureLimits limits{16384,2048};
    BOOST_CHECK(Select_Texture_Extent({513,257,3},limits) == (TextureExtent{1024,512,4}));
    BOOST_CHECK(Select_Texture_Extent({0,0,0},limits) == (TextureExtent{1,1,1}));
    BOOST_CHECK(Select_Texture_Extent({1,16384,1},limits) == (TextureExtent{1,16384,1}));
    const auto maximum = std::numeric_limits<unsigned>::max();
    BOOST_CHECK(Select_Texture_Extent({maximum,maximum,maximum},limits) == (TextureExtent{16384,16384,2048}));
    BOOST_CHECK(Select_Texture_Extent({maximum,1,1},{maximum,maximum}) == (TextureExtent{maximum,1,1}));
    BOOST_CHECK(Select_Texture_Extent({8,8,1},{}) == (TextureExtent{0,0,0}));
}

BOOST_AUTO_TEST_CASE(compressed_mips_keep_quality_minimums_and_authored_level_requests)
{
    const RHITextureLimits limits{16384,2048};
    TextureMipSelection selected;
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({256,128,1},9,{0,0,16,true},limits,selected));
    BOOST_CHECK(selected.extent == (TextureExtent{256,128,1}));
    BOOST_CHECK_EQUAL(selected.first_mip,0u);
    BOOST_CHECK_EQUAL(selected.mip_count,6u);
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({256,128,1},9,{0,99,16,true},limits,selected));
    BOOST_CHECK(selected.extent == (TextureExtent{32,16,1}));
    BOOST_CHECK_EQUAL(selected.first_mip,3u);
    BOOST_CHECK_EQUAL(selected.mip_count,3u);
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({256,128,1},9,{0,4,16,false},limits,selected));
    BOOST_CHECK_EQUAL(selected.first_mip,0u);
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({256,128,1},9,{1,4,16,true},limits,selected));
    BOOST_CHECK_EQUAL(selected.first_mip,0u);
    BOOST_CHECK_EQUAL(selected.mip_count,1u);
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({256,128,1},9,{2,4,16,true},limits,selected));
    BOOST_CHECK_EQUAL(selected.first_mip,1u);
    BOOST_CHECK_EQUAL(selected.mip_count,1u);
    // Partial authored chains keep the same tail reservation as full chains.
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({256,128,1},4,{0,0,16,true},limits,selected));
    BOOST_CHECK_EQUAL(selected.mip_count,2u);
}

BOOST_AUTO_TEST_CASE(compressed_mips_fit_device_limits_and_keep_volume_depth)
{
    TextureMipSelection selected;
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({1024,512,64},11,{0,0,1,true},{256,16},selected));
    BOOST_CHECK(selected.extent == (TextureExtent{256,128,16}));
    BOOST_CHECK_EQUAL(selected.first_mip,2u);
    BOOST_CHECK_EQUAL(selected.mip_count,6u);
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({16,1,1},5,{0,4,1,true},{16384,2048},selected));
    BOOST_CHECK(selected.extent == (TextureExtent{16,4,1}));
    BOOST_CHECK_EQUAL(selected.first_mip,0u);
    BOOST_CHECK_EQUAL(selected.mip_count,1u);
    BOOST_REQUIRE(Select_Compressed_Texture_Mips({2,2,1},2,{}, {16384,2048},selected));
    BOOST_CHECK(selected.extent == (TextureExtent{4,4,1}));
    BOOST_CHECK_EQUAL(selected.mip_count,1u);
}

BOOST_AUTO_TEST_CASE(invalid_mip_selection_does_not_publish_or_underflow_counts)
{
    TextureMipSelection selected{{8,8,1},2,3};
    const RHITextureLimits limits{16384,2048};
    BOOST_CHECK(!Select_Compressed_Texture_Mips({8,0,1},4,{},limits,selected));
    BOOST_CHECK(!Select_Compressed_Texture_Mips({8,8,1},0,{},limits,selected));
    BOOST_CHECK(!Select_Compressed_Texture_Mips({8,8,1},33,{},limits,selected));
    BOOST_CHECK(!Select_Compressed_Texture_Mips({8,8,1},4,{}, {},selected));
    BOOST_CHECK(!Select_Compressed_Texture_Mips({32768,32768,1},1,{},limits,selected));
    BOOST_CHECK(!Select_Compressed_Texture_Mips({32768,32768,1},16,{1,0,1,true},limits,selected));
    BOOST_CHECK(selected.extent == (TextureExtent{8,8,1}));
    BOOST_CHECK_EQUAL(selected.first_mip,2u);
    BOOST_CHECK_EQUAL(selected.mip_count,3u);
}
