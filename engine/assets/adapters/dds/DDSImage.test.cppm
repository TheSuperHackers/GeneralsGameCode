module;
#define BOOST_TEST_MODULE DDSImageTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <vector>
export module Assets.Adapters.DDS.Tests;
import Assets.Adapters.DDS;

namespace
{
void Write(std::vector<std::byte>& bytes, std::size_t offset, std::uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i) bytes[offset + i] = std::byte(value >> (i * 8));
}
std::vector<std::byte> Image(unsigned width, unsigned height, unsigned mips,
    unsigned payload, unsigned format = 0x31545844, unsigned caps = 0, unsigned depth = 0)
{
    std::vector<std::byte> bytes(128 + payload);
    Write(bytes, 0, 0x20534444); Write(bytes, 4, 124);
    Write(bytes, 12, height); Write(bytes, 16, width); Write(bytes, 24, depth); Write(bytes, 28, mips);
    Write(bytes, 76, 32); Write(bytes, 80, 4); Write(bytes, 84, format); Write(bytes, 112, caps);
    return bytes;
}
}

BOOST_AUTO_TEST_CASE(rectangular_mips_retain_complete_authored_block_tail)
{
    auto bytes = Image(16, 4, 5, 32 + 16 + 8 + 8 + 8);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
    const unsigned widths[]{16, 8, 4, 2, 1}, heights[]{4, 2, 1, 1, 1};
    const unsigned offsets[]{128, 160, 176, 184, 192};
    for (unsigned i = 0; i < 5; ++i) {
        const auto* surface = layout.Surface(i);
        BOOST_REQUIRE(surface);
        BOOST_CHECK_EQUAL(surface->width, widths[i]); BOOST_CHECK_EQUAL(surface->height, heights[i]);
        BOOST_CHECK_EQUAL(surface->offset, offsets[i]);
        Write(bytes, offsets[i], 0xf800f800);
        std::vector<std::byte> pixels;
        BOOST_REQUIRE(Assets::Decode_DDS_Surface(bytes, layout, i, 0, 0, pixels));
        BOOST_CHECK_EQUAL(pixels.size(), std::size_t(widths[i]) * heights[i] * 4);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]), 255u);
    }
    BOOST_CHECK(!layout.Surface(5)); BOOST_CHECK(!layout.Surface(0, 1));
}

BOOST_AUTO_TEST_CASE(cube_faces_keep_full_mip_strides_when_selecting_reduced_levels)
{
    auto bytes = Image(8, 8, 4, 56 * 6, 0x31545844, 0xfe00);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
    for (unsigned face = 0; face < 6; ++face) {
        const auto* surface = layout.Surface(2, face);
        BOOST_REQUIRE(surface); BOOST_CHECK_EQUAL(surface->offset, 128 + face * 56 + 40);
        Write(bytes, surface->offset, face % 2 ? 0x07e007e0 : 0xf800f800);
        std::vector<std::byte> pixels;
        BOOST_REQUIRE(Assets::Decode_DDS_Surface(bytes, layout, 2, face, 0, pixels));
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[face % 2]), 255u);
    }
    auto partial = Image(4, 4, 1, 16, 0x31545844, 0x200 | (1 << 10) | (1 << 15));
    BOOST_REQUIRE(Assets::Read_DDS_Layout(partial, partial.size(), layout));
    BOOST_REQUIRE(layout.Surface(0, 5)); BOOST_CHECK_EQUAL(layout.Surface(0, 5)->offset, 136u);
    BOOST_CHECK(!layout.Surface(0, 1));
}

BOOST_AUTO_TEST_CASE(volume_slices_and_mips_use_independent_pitches)
{
    auto bytes = Image(8, 4, 4, 16 * 4 + 8 * 2 + 8 + 8, 0x31545844, 0x200000, 4);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
    BOOST_CHECK_EQUAL(layout.Surface(0)->size, 64u);
    BOOST_CHECK_EQUAL(layout.Surface(1)->offset, 192u);
    BOOST_CHECK_EQUAL(layout.Surface(1)->depth, 2u);
    Write(bytes, 200, 0x001f001f);
    std::vector<std::byte> pixels;
    BOOST_REQUIRE(Assets::Decode_DDS_Surface(bytes, layout, 1, 0, 1, pixels));
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[2]), 255u);
    BOOST_CHECK(!Assets::Decode_DDS_Surface(bytes, layout, 1, 0, 2, pixels));
    std::vector<std::byte> padded(64, std::byte{0xcc});
    BOOST_REQUIRE(Assets::Copy_DDS_Blocks(bytes, *layout.Surface(1), padded, 16, 32, layout.compression));
    BOOST_CHECK(padded[32] == std::byte{0x1f}); BOOST_CHECK(padded[8] == std::byte{0xcc});
    BOOST_CHECK(padded[48] == std::byte{0xcc});
    BOOST_CHECK(!Assets::Copy_DDS_Blocks(bytes, *layout.Surface(1), padded, 7, 32, layout.compression));
}

BOOST_AUTO_TEST_CASE(malformed_headers_and_payloads_never_publish_partial_layouts)
{
    const auto valid = Image(8, 8, 4, 56);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(valid, valid.size(), layout));
    for (std::size_t size = 0; size < valid.size(); ++size) {
        BOOST_CHECK(!Assets::Read_DDS_Layout(std::span(valid).first(size), size, layout));
        BOOST_CHECK_EQUAL(layout.mip_count, 4u);
    }
    for (const auto field : {0u, 4u, 12u, 16u, 76u, 80u, 84u}) {
        auto invalid = valid; Write(invalid, field, 0);
        BOOST_CHECK(!Assets::Read_DDS_Layout(invalid, invalid.size(), layout));
    }
    auto invalid = valid; Write(invalid, 28, 33);
    BOOST_CHECK(!Assets::Read_DDS_Layout(invalid, invalid.size(), layout));
    Write(invalid, 28, 1); Write(invalid, 16, 0xffffffff); Write(invalid, 12, 0xffffffff);
    Write(invalid, 84, 0x35545844);
    BOOST_CHECK(!Assets::Read_DDS_Layout(invalid, std::size_t(-1), layout));
    BOOST_REQUIRE(Assets::Read_DDS_Layout(std::span(valid).first(128), valid.size(), layout));
    std::vector<std::byte> output{std::byte{123}};
    BOOST_CHECK(!Assets::Decode_DDS_Surface(std::span(valid).first(128), layout, 0, 0, 0, output));
    BOOST_CHECK_EQUAL(output.size(), 1u); BOOST_CHECK(output[0] == std::byte{123});
}

BOOST_AUTO_TEST_CASE(alpha_formats_and_endpoint_recolor_preserve_alpha_and_padding)
{
    for (const unsigned format : {0x31545844u, 0x32545844u, 0x33545844u, 0x34545844u, 0x35545844u}) {
        const bool bc1 = format == 0x31545844;
        const bool bc2 = format == 0x32545844 || format == 0x33545844;
        auto bytes = Image(4, 4, 1, bc1 ? 8 : 16, format);
        if (!bc1) { Write(bytes, 128, bc2 ? 0x88888888 : 0x00008080); Write(bytes, 132, bc2 ? 0x88888888 : 0); }
        Write(bytes, bc1 ? 128 : 136, 0xf800f800);
        Assets::DDSLayout layout;
        BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
        BOOST_CHECK_EQUAL(layout.premultiplied_alpha, format == 0x32545844 || format == 0x34545844);
        std::vector<std::byte> shifted(32, std::byte{0xcc});
        BOOST_REQUIRE(Assets::Copy_DDS_Blocks(bytes, *layout.Surface(0), shifted, 32, 32,
            layout.compression, {120, 0, 0}));
        auto selected = layout; selected.surfaces[0].offset = 0;
        std::vector<std::byte> pixels;
        BOOST_REQUIRE(Assets::Decode_DDS_Surface(shifted, selected, 0, 0, 0, pixels));
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]), 0u);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[1]), 250u);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[3]), bc1 ? 255u : bc2 ? 136u : 128u);
        BOOST_CHECK(shifted[31] == std::byte{0xcc});
        if (!bc1) for (unsigned i = 0; i < 8; ++i) BOOST_CHECK(shifted[i] == bytes[128 + i]);
    }
}

BOOST_AUTO_TEST_CASE(bc1_cutout_and_non_block_aligned_dimensions)
{
    auto bytes = Image(3, 2, 1, 8);
    Write(bytes, 128, 0xffff0000); Write(bytes, 132, 0xffffffff);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
    std::vector<std::byte> pixels;
    BOOST_REQUIRE(Assets::Decode_DDS_Surface(bytes, layout, 0, 0, 0, pixels));
    BOOST_REQUIRE_EQUAL(pixels.size(), 24u);
    for (const auto pixel : pixels) BOOST_CHECK(pixel == std::byte{0});
}

BOOST_AUTO_TEST_CASE(image_expansion_preserves_recolor_alpha_and_pitched_edge_padding)
{
    for (const unsigned format : {0x31545844u, 0x32545844u, 0x33545844u, 0x34545844u, 0x35545844u}) {
        const bool bc1 = format == 0x31545844;
        const bool bc2 = format == 0x32545844 || format == 0x33545844;
        auto bytes = Image(3, 2, 1, bc1 ? 8 : 16, format);
        if (!bc1) { Write(bytes, 128, bc2 ? 0x88888888 : 0x00008080); Write(bytes, 132, bc2 ? 0x88888888 : 0); }
        Write(bytes, bc1 ? 128 : 136, 0xf800f800);
        const auto original = bytes;
        Assets::DDSLayout layout;
        BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
        BOOST_CHECK(Assets::Is_Block_Compressed(Assets::DDS_Pixel_Encoding(layout)));
        for (const auto encoding : {Assets::PixelEncoding::RGBA8, Assets::PixelEncoding::BGRA8}) {
            std::vector<std::byte> destination(96, std::byte{0xcc});
            BOOST_REQUIRE(Assets::Copy_DDS_Image(bytes, layout, 0, 0, encoding, 4, 4, 1,
                destination, 20, 96, {120, 0, 0}));
            for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 4; ++x) {
                const auto offset = y * 20 + x * 4;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[offset]), 0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[offset + 1]), 250u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[offset + 2]), 0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[offset + 3]), bc1 ? 255u : bc2 ? 136u : 128u);
            }
            for (unsigned y = 0; y < 4; ++y) BOOST_CHECK(destination[y * 20 + 16] == std::byte{0xcc});
            BOOST_CHECK(destination[80] == std::byte{0xcc});
        }
        BOOST_CHECK(bytes == original);
    }

    // A nonuniform tail distinguishes repeating edge texels from rescaling.
    auto bytes = Image(3, 2, 1, 8);
    Write(bytes, 128, 0x001ff800); Write(bytes, 132, 0x00000110);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
    std::array<std::byte, 64> destination{};
    BOOST_REQUIRE(Assets::Copy_DDS_Image(bytes, layout, 0, 0, Assets::PixelEncoding::RGBA8,
        4, 4, 1, destination, 16, 64));
    for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 4; ++x) {
        const bool blue = y == 0 ? x == 2 || x == 3 : x == 0;
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[y * 16 + x * 4]), blue ? 0 : 255);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[y * 16 + x * 4 + 2]), blue ? 255 : 0);
    }
}

BOOST_AUTO_TEST_CASE(image_copy_selects_cube_face_mips_and_volume_slices)
{
    for (const bool cube : {true, false}) {
        auto bytes = Image(4, 4, 3, cube ? 6 * 24 : 32, 0x31545844,
            cube ? 0xfe00 : 0x200000, cube ? 0 : 2);
        Assets::DDSLayout layout;
        BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
        for (unsigned face = 0; face < (cube ? 6u : 1u); ++face) for (unsigned mip = 0; mip < 3; ++mip) {
            const auto& surface = *layout.Surface(mip, face);
            for (unsigned z = 0; z < surface.depth; ++z)
                Write(bytes, surface.offset + z * surface.slice_pitch, (face + mip + z) % 2 ? 0x001f001f : 0xf800f800);
            std::vector<std::byte> destination(surface.depth * 80, std::byte{0xcc});
            BOOST_REQUIRE(Assets::Copy_DDS_Image(bytes, layout, mip, face, Assets::PixelEncoding::BGRA8,
                4, 4, surface.depth, destination, 16, 80));
            for (unsigned z = 0; z < surface.depth; ++z) {
                const bool blue = (face + mip + z) % 2 != 0;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[z * 80 + 60]), blue ? 255 : 0);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(destination[z * 80 + 62]), blue ? 0 : 255);
                BOOST_CHECK(destination[z * 80 + 64] == std::byte{0xcc});
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(invalid_image_copy_never_writes_destination)
{
    auto bytes = Image(4, 4, 1, 8);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
    const std::vector<std::byte> sentinel(64, std::byte{0xcc});
    auto destination = sentinel;
    using E = Assets::PixelEncoding;
    for (const auto encoding : {E::Unknown, E::Indexed8, E::BC3})
        BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, encoding, 4, 4, 1, destination, 16, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 1, 0, E::RGBA8, 4, 4, 1, destination, 16, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 1, E::RGBA8, 4, 4, 1, destination, 16, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 4, 4, 2, destination, 16, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 4, 4, 1, destination, 15, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 4, 4, 1, destination, 16, 63));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 4, 4, 1, std::span(destination).first(63), 16, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 0, 0, 1, destination, 16, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 8, 4, 1, destination, 16, 64));
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 4, 4, 1, destination, 16, 64,
        {std::numeric_limits<float>::quiet_NaN(), 0, 0}));
    BOOST_CHECK(!Assets::Copy_DDS_Image(std::span(bytes).first(135), layout, 0, 0, E::RGBA8,
        4, 4, 1, destination, 16, 64));
    layout.surfaces[0].slice_pitch = 0;
    BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, E::RGBA8, 4, 4, 1, destination, 16, 64));
    BOOST_CHECK(destination == sentinel);
}

BOOST_AUTO_TEST_CASE(pitched_destinations_end_at_the_last_occupied_byte)
{
    auto bytes = Image(8, 8, 1, 64, 0x31545844, 0x200000, 2);
    for (unsigned offset = 128; offset < bytes.size(); offset += 8) Write(bytes, offset, 0xf800f800);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(bytes, bytes.size(), layout));
    for (const bool compressed : {true, false}) {
        const auto encoding = compressed ? Assets::PixelEncoding::BC1 : Assets::PixelEncoding::RGBA8;
        const unsigned rows = compressed ? 2 : 8, row_bytes = compressed ? 16 : 32;
        const unsigned row_pitch = 64, slice_pitch = rows * row_pitch + 128;
        const unsigned size = slice_pitch + (rows - 1) * row_pitch + row_bytes;
        std::vector<std::byte> destination(size, std::byte{0xcc});
        BOOST_CHECK(!Assets::Copy_DDS_Image(bytes, layout, 0, 0, encoding, 8, 8, 2,
            std::span(destination).first(size - 1), row_pitch, slice_pitch));
        for (const auto value : destination) BOOST_CHECK(value == std::byte{0xcc});
        BOOST_REQUIRE(Assets::Copy_DDS_Image(bytes, layout, 0, 0, encoding, 8, 8, 2,
            destination, row_pitch, slice_pitch));
        for (unsigned z = 0; z < 2; ++z) for (unsigned row = 0; row < rows; ++row) {
            BOOST_CHECK(destination[z * slice_pitch + row * row_pitch] == (compressed ? std::byte{0} : std::byte{255}));
            if (z == 0 || row + 1 < rows)
                BOOST_CHECK(destination[z * slice_pitch + row * row_pitch + row_bytes] == std::byte{0xcc});
        }
        BOOST_CHECK(destination.back() == (compressed ? std::byte{0} : std::byte{255}));
    }
}

BOOST_AUTO_TEST_CASE(staged_dds_corpus_has_bounded_complete_authored_subresources)
{
    const char* directory = std::getenv("GENERALS_DDS_DIRECTORY");
    if (!directory) { BOOST_TEST_MESSAGE("Set GENERALS_DDS_DIRECTORY to validate externally supplied textures."); return; }
    unsigned count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() != ".dds" && entry.path().extension() != ".DDS") continue;
        std::ifstream input(entry.path(), std::ios::binary);
        std::array<std::byte, 128> header{};
        input.read(reinterpret_cast<char*>(header.data()), header.size());
        Assets::DDSLayout layout;
        BOOST_TEST_CONTEXT(entry.path().string()) {
            BOOST_REQUIRE_EQUAL(input.gcount(), 128);
            BOOST_REQUIRE(Assets::Read_DDS_Layout(header, entry.file_size(), layout));
            BOOST_REQUIRE(layout.Surface(0));
        }
        ++count;
    }
    BOOST_CHECK_GT(count, 0u);
    BOOST_TEST_MESSAGE("Validated " << count << " staged DDS texture layouts.");
}
