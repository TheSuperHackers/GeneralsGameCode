module;
#define BOOST_TEST_MODULE ImagePreparationTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <vector>
export module Assets.Images.Preparation.Tests;
import Assets.Images.Preparation;
using namespace Assets;

BOOST_AUTO_TEST_CASE(mips_preserve_source_alpha_filtering_and_each_previous_level)
{
    std::array<std::byte, 8 * 4 * 4> bytes{};
    for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 8; ++x)
        BOOST_REQUIRE(Write_Image_Pixel(std::span(bytes).subspan((y * 8 + x) * 4, 4), PixelEncoding::BGRA8,
            0xff000000 | ((x < 4 ? 255u : 0u) << 16) | (x < 4 ? 0 : 255)));
    const auto original = bytes;
    std::vector<PreparedImage> levels;
    BOOST_REQUIRE(Prepare_Image_Levels({bytes, 8, 4, 32, PixelEncoding::BGRA8, {}}, PixelEncoding::BGRA8,
        8, 4, 4, {}, levels));
    BOOST_CHECK(bytes == original);
    BOOST_REQUIRE_EQUAL(levels.size(), 4u);
    BOOST_CHECK_EQUAL(levels[1].width, 4u); BOOST_CHECK_EQUAL(levels[1].height, 2u);
    BOOST_CHECK_EQUAL(levels[2].width, 2u); BOOST_CHECK_EQUAL(levels[2].height, 1u);
    BOOST_CHECK_EQUAL(levels[3].width, 1u); BOOST_CHECK_EQUAL(levels[3].height, 1u);
    unsigned color;
    BOOST_REQUIRE(Read_Image_Pixel(levels[1].bytes, PixelEncoding::BGRA8, color));
    BOOST_CHECK_EQUAL(color, 0xfcfc0000u);
    BOOST_REQUIRE(Read_Image_Pixel(std::span(levels[2].bytes).subspan(4), PixelEncoding::BGRA8, color));
    BOOST_CHECK_EQUAL(color, 0xfc0000fcu);
    BOOST_REQUIRE(Read_Image_Pixel(levels[3].bytes, PixelEncoding::BGRA8, color));
    BOOST_CHECK_EQUAL(color, 0xfc7e007eu);
}

BOOST_AUTO_TEST_CASE(resize_and_recolor_are_applied_once_before_mip_filtering)
{
    const std::array<std::byte, 8> source{std::byte{0}, std::byte{0}, std::byte{255}, std::byte{64},
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{128}};
    std::vector<PreparedImage> levels;
    BOOST_REQUIRE(Prepare_Image_Levels({source, 2, 1, 8, PixelEncoding::BGRA8, {}}, PixelEncoding::RGBA8,
        4, 2, 3, {120, 0, 0}, levels));
    unsigned color;
    BOOST_REQUIRE(Read_Image_Pixel(levels[0].bytes, PixelEncoding::RGBA8, color)); BOOST_CHECK_EQUAL(color, 0x4000ff00u);
    BOOST_REQUIRE(Read_Image_Pixel(std::span(levels[0].bytes).subspan(12), PixelEncoding::RGBA8, color));
    BOOST_CHECK_EQUAL(color, 0x80ff0000u);
    BOOST_REQUIRE(Read_Image_Pixel(levels[2].bytes, PixelEncoding::RGBA8, color));
    BOOST_CHECK_EQUAL(color, 0x607e7e00u);
    std::vector<std::byte> padded(48, std::byte{0xcd});
    BOOST_REQUIRE(Copy_Prepared_Image(levels[0], padded, 24));
    BOOST_CHECK(padded[16] == std::byte{0xcd}); BOOST_CHECK(padded[47] == std::byte{0xcd});
    BOOST_CHECK(!Copy_Prepared_Image(levels[0], padded, 15));
}

BOOST_AUTO_TEST_CASE(paletted_source_converts_before_generating_mips)
{
    const std::array<std::byte, 2> indices{std::byte{0}, std::byte{1}};
    const std::array<std::byte, 8> palette{std::byte{0}, std::byte{255}, std::byte{0}, std::byte{0},
        std::byte{0}, std::byte{255}, std::byte{0}, std::byte{128}};
    std::vector<PreparedImage> levels;
    BOOST_REQUIRE(Prepare_Image_Levels({indices, 2, 1, 2, PixelEncoding::Indexed8, {palette, PixelEncoding::BGRA8}},
        PixelEncoding::BGRA4444, 2, 1, 2, {}, levels));
    unsigned color;
    BOOST_REQUIRE(Read_Image_Pixel(levels[1].bytes, PixelEncoding::BGRA4444, color));
    BOOST_CHECK_EQUAL(color, 0x4000f000u);
}

BOOST_AUTO_TEST_CASE(gradient_maps_clamp_edges_and_keep_signed_components_and_luminance)
{
    const std::array<std::byte, 3> source{std::byte{0}, std::byte{8}, std::byte{24}};
    std::vector<PreparedImage> levels;
    BOOST_REQUIRE(Prepare_Image_Levels({source, 3, 1, 3, PixelEncoding::Luminance8, {}},
        PixelEncoding::RG8_SNorm_L8X8, 3, 1, 1, {}, levels));
    const auto& bytes = levels[0].bytes;
    BOOST_CHECK(bytes[0] == std::byte{248}); BOOST_CHECK(bytes[1] == std::byte{0}); BOOST_CHECK(bytes[2] == std::byte{127});
    BOOST_CHECK(bytes[4] == std::byte{232}); BOOST_CHECK(bytes[5] == std::byte{0}); BOOST_CHECK(bytes[6] == std::byte{63});
    BOOST_CHECK(bytes[8] == std::byte{240}); BOOST_CHECK(bytes[9] == std::byte{0}); BOOST_CHECK(bytes[10] == std::byte{63});
    BOOST_REQUIRE(Prepare_Image_Levels({source, 3, 1, 3, PixelEncoding::Luminance8, {}},
        PixelEncoding::RG5_SNorm_L6, 3, 1, 1, {}, levels));
    BOOST_CHECK(levels[0].bytes[0] == std::byte{31}); BOOST_CHECK(levels[0].bytes[1] == std::byte{124});
}

BOOST_AUTO_TEST_CASE(invalid_inputs_do_not_publish_partial_levels)
{
    const std::array<std::byte, 4> source{};
    const ImageView view{source, 1, 1, 4, PixelEncoding::BGRA8, {}};
    std::vector<PreparedImage> levels(1); levels[0].width = 7;
    BOOST_CHECK(!Prepare_Image_Levels(view, PixelEncoding::BGRA8, 1, 1, 2, {}, levels));
    BOOST_CHECK(!Prepare_Image_Levels(view, PixelEncoding::BGRA8, 0, 1, 1, {}, levels));
    BOOST_CHECK(!Prepare_Image_Levels(view, PixelEncoding::Indexed8, 1, 1, 1, {}, levels));
    BOOST_CHECK_EQUAL(levels[0].width, 7u);
}

BOOST_AUTO_TEST_CASE(one_dimensional_textures_can_prepare_more_than_twelve_mips)
{
    std::vector<std::byte> source(8192 * 4, std::byte{255});
    std::vector<PreparedImage> levels;
    BOOST_REQUIRE(Prepare_Image_Levels({source, 8192, 1, 8192 * 4, PixelEncoding::BGRA8, {}},
        PixelEncoding::BGRA8, 8192, 1, 14, {}, levels));
    BOOST_REQUIRE_EQUAL(levels.size(), 14u);
    BOOST_CHECK_EQUAL(levels.back().width, 1u); BOOST_CHECK_EQUAL(levels.back().height, 1u);
    unsigned color;
    BOOST_REQUIRE(Read_Image_Pixel(levels.back().bytes, PixelEncoding::BGRA8, color));
    BOOST_CHECK_EQUAL(color, 0xfcfcfcfcu);
}

BOOST_AUTO_TEST_CASE(copy_accepts_mapped_rows_without_trailing_padding)
{
    for (const unsigned source_pitch : {8u, 12u}) {
    PreparedImage image;
    image.width = 2; image.height = 3; image.row_pitch = source_pitch;
    image.encoding = PixelEncoding::RGBA8;
    image.bytes.resize(source_pitch * 2 + 8, std::byte{123});
    std::array<std::byte, 40> destination{};
    BOOST_CHECK(!Copy_Prepared_Image(image, std::span(destination).first(39), 16));
    for (const auto value : destination) BOOST_CHECK(value == std::byte{0});
    BOOST_REQUIRE(Copy_Prepared_Image(image, destination, 16));
    for (unsigned i = 0; i < destination.size(); ++i)
        BOOST_CHECK(destination[i] == (i % 16 < 8 ? std::byte{123} : std::byte{0}));
    }
}
