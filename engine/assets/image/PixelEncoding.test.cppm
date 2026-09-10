module;
#define BOOST_TEST_MODULE PixelEncodingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
export module Assets.Images.PixelEncoding.Tests;
import Assets.Images.PixelEncoding;
using namespace Assets;

BOOST_AUTO_TEST_CASE(pixel_metadata_distinguishes_scalar_alpha_and_compressed_blocks)
{
    BOOST_CHECK_EQUAL(Pixel_Size(PixelEncoding::IndexedAlpha88),2);
    BOOST_CHECK_EQUAL(Pixel_Alpha_Bits(PixelEncoding::BGRA8),8);
    BOOST_CHECK_EQUAL(Pixel_Alpha_Bits(PixelEncoding::BGRA4444),4);
    BOOST_CHECK_EQUAL(Pixel_Alpha_Bits(PixelEncoding::BGRA5551),1);
    BOOST_CHECK_EQUAL(Pixel_Alpha_Bits(PixelEncoding::BGRX8),0);
    BOOST_CHECK(!Has_Explicit_Pixel_Alpha(PixelEncoding::BGRX8));
    BOOST_CHECK(!Has_Explicit_Pixel_Alpha(PixelEncoding::Indexed8));
    BOOST_CHECK(Has_Explicit_Pixel_Alpha(PixelEncoding::IndexedAlpha88));
    for (auto encoding : {PixelEncoding::BC1,PixelEncoding::BC2,PixelEncoding::BC2Premultiplied,
        PixelEncoding::BC3,PixelEncoding::BC3Premultiplied}) {
        BOOST_CHECK(Is_Block_Compressed(encoding));
        BOOST_CHECK_EQUAL(Pixel_Size(encoding),0);
        BOOST_CHECK_EQUAL(Pixel_Alpha_Bits(encoding),0);
        BOOST_CHECK_EQUAL(Has_Explicit_Pixel_Alpha(encoding),encoding!=PixelEncoding::BC1);
        std::array<std::byte,16> block{};
        std::uint32_t color=0x12345678;
        BOOST_CHECK(!Read_Image_Pixel(block,encoding,color));
        BOOST_CHECK_EQUAL(color,0x12345678);
    }
}

BOOST_AUTO_TEST_CASE(solid_colors_preserve_packed_channels_alpha_threshold_and_luminance)
{
    const std::array cases{
        std::pair{PixelEncoding::BGR8,0x00f84020u},
        std::pair{PixelEncoding::BGRA8,0x80f84020u},
        std::pair{PixelEncoding::BGRX8,0xfff84020u},
        std::pair{PixelEncoding::BGR565,0xfa04u},
        std::pair{PixelEncoding::BGRX5551,0xfd04u},
        std::pair{PixelEncoding::BGRA5551,0xfd04u},
        std::pair{PixelEncoding::BGRA4444,0x8f42u},
        std::pair{PixelEncoding::BGRX4444,0xff42u},
        std::pair{PixelEncoding::RGB332,0xe8u},
        std::pair{PixelEncoding::RGB332Alpha8,0x80e8u},
        std::pair{PixelEncoding::Alpha8,0x80u},
        std::pair{PixelEncoding::Luminance8,0x73u},
        std::pair{PixelEncoding::LuminanceAlpha88,0x8073u},
        std::pair{PixelEncoding::LuminanceAlpha44,0x87u}};
    for (const auto& [encoding,expected] : cases)
        BOOST_CHECK_EQUAL(Pack_Image_Color(encoding,0x80f84020),expected);
    for (unsigned alpha : {0u,1u,127u,128u,255u})
        BOOST_CHECK_EQUAL(Pack_Image_Color(PixelEncoding::BGRA5551,(alpha<<24)|0xf84020),
            alpha < 128 ? 0x7d04u : 0xfd04u);
    // Solid drawing colors and image preparation intentionally have different
    // one-bit alpha quantization; preserve the image preparation contract.
    std::array<std::byte,2> bytes{};
    BOOST_REQUIRE(Write_Image_Pixel(bytes,PixelEncoding::BGRA5551,0x01f84020));
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(bytes[1]),0xfd);
    BOOST_REQUIRE(Write_Image_Pixel(bytes,PixelEncoding::RGB332Alpha8,0x80f84020));
    std::uint32_t color=0;
    BOOST_REQUIRE(Read_Image_Pixel(bytes,PixelEncoding::RGB332Alpha8,color));
    BOOST_CHECK_EQUAL(color,0x80e04000);
    for (auto encoding : {PixelEncoding::Unknown,PixelEncoding::Indexed8,PixelEncoding::RG8_SNorm})
        BOOST_CHECK_EQUAL(Pack_Image_Color(encoding,0xffffffff),0);
}

BOOST_AUTO_TEST_CASE(packed_formats_retain_channel_order_and_copy_without_swizzling)
{
    struct Case { PixelEncoding encoding; unsigned encoded, expanded; };
    const Case cases[]{
        {PixelEncoding::BGRA8, 0x80f84020, 0x80f84020}, {PixelEncoding::RGBA8, 0x802040f8, 0x80f84020},
        {PixelEncoding::BGR565, 0xfa04, 0xfff84020}, {PixelEncoding::BGRA5551, 0xfd04, 0xfff84020},
        {PixelEncoding::BGRA4444, 0x8f42, 0x80f04020}, {PixelEncoding::RGB332, 0xe8, 0xffe04000},
        {PixelEncoding::Alpha8, 0x80, 0x80000000}, {PixelEncoding::Luminance8, 0x80, 0xff808080}};
    for (const auto& value : cases) {
        const unsigned size = Pixel_Size(value.encoding);
        std::array<std::byte, 6> encoded; encoded.fill(std::byte{0xcd});
        for (unsigned i = 0; i < size; ++i) encoded[i + 1] = std::byte(value.encoded >> (i * 8));
        unsigned color = 0;
        BOOST_REQUIRE(Read_Image_Pixel(std::span(encoded).subspan(1, size), value.encoding, color));
        BOOST_CHECK_EQUAL(color, value.expanded);
        std::array<std::byte, 6> copy; copy.fill(std::byte{0xcd});
        BOOST_REQUIRE(Convert_Image_Pixel(std::span(copy).subspan(1, size), value.encoding,
            std::span(encoded).subspan(1, size), value.encoding));
        BOOST_CHECK(copy == encoded);
        BOOST_REQUIRE(Write_Image_Pixel(std::span(copy).subspan(1, size), value.encoding, color));
        BOOST_CHECK(copy == encoded);
    }
}

BOOST_AUTO_TEST_CASE(alpha_and_single_byte_outputs_preserve_guard_bytes)
{
    std::array<std::byte, 3> destination{std::byte{0xab}, std::byte{0}, std::byte{0xcd}};
    BOOST_REQUIRE(Write_Image_Pixel(std::span(destination).subspan(1, 1), PixelEncoding::Alpha8, 0x401020ff));
    BOOST_CHECK(destination[1] == std::byte{0x40});
    BOOST_REQUIRE(Write_Image_Pixel(std::span(destination).subspan(1, 1), PixelEncoding::RGB332, 0xffe0a0c0));
    BOOST_CHECK(destination[1] == std::byte{0xf7});
    BOOST_CHECK(destination[0] == std::byte{0xab}); BOOST_CHECK(destination[2] == std::byte{0xcd});
    const auto saved = destination;
    BOOST_CHECK(!Write_Image_Pixel(std::span(destination).subspan(1, 1), PixelEncoding::BGRA8, 0));
    BOOST_CHECK(destination == saved);
}

BOOST_AUTO_TEST_CASE(palette_entries_have_explicit_order_alpha_and_bounds)
{
    const std::array<std::byte, 8> palette{std::byte{0}, std::byte{0}, std::byte{0}, std::byte{0},
        std::byte{17}, std::byte{33}, std::byte{65}, std::byte{129}};
    const std::array<std::byte, 1> index{std::byte{1}};
    unsigned color = 0;
    BOOST_REQUIRE(Read_Image_Pixel(index, PixelEncoding::Indexed8, color, {palette, PixelEncoding::BGRA8}));
    BOOST_CHECK_EQUAL(color, 0x81412111u);
    BOOST_REQUIRE(Read_Image_Pixel(index, PixelEncoding::Indexed8, color, {palette, PixelEncoding::RGBA8}));
    BOOST_CHECK_EQUAL(color, 0x81112141u);
    BOOST_CHECK(!Read_Image_Pixel(index, PixelEncoding::Indexed8, color, {std::span(palette).first(7), PixelEncoding::BGRA8}));
    BOOST_CHECK_EQUAL(color, 0x81112141u);
    BOOST_CHECK(!Read_Image_Pixel(index, PixelEncoding::Indexed8, color, {palette, PixelEncoding::Indexed8}));
}

BOOST_AUTO_TEST_CASE(image_views_validate_dimensions_stride_and_last_row)
{
    std::array<std::byte, 20> bytes{};
    const ImageView valid{bytes, 2, 2, 12, PixelEncoding::BGRA8, {}};
    BOOST_CHECK(valid.Is_Valid());
    auto invalid = valid; invalid.row_pitch = 7; BOOST_CHECK(!invalid.Is_Valid());
    invalid = valid; invalid.width = 0; BOOST_CHECK(!invalid.Is_Valid());
    invalid = valid; invalid.height = 0xffffffff; BOOST_CHECK(!invalid.Is_Valid());
    invalid = valid; invalid.bytes = std::span(bytes).first(19); BOOST_CHECK(!invalid.Is_Valid());
    invalid = valid; invalid.encoding = PixelEncoding::Unknown; BOOST_CHECK(!invalid.Is_Valid());
}
