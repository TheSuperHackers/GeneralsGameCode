module;

#define BOOST_TEST_MODULE RibbonPipelineTests

#include <boost/test/included/unit_test.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.Beams.RibbonPipeline.Tests;

import Graphics.Scene.Beams.RibbonPipeline;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Scene.Props.Geometry;
#if defined(_WIN32)
import Graphics.Scene.Props.Renderer;
import Graphics.RHI;
import Graphics.Tests.Device;
#endif

using namespace Graphics;

namespace {

RibbonPoint Make_Line_Point(std::size_t index, float z = -5.0f)
{
    RibbonPoint point;
    point.position = {static_cast<float>(index) * 0.1f - 1.0f, 0, z};
    point.color = {1, 1, 1, 1};
    return point;
}

struct ChunkObservation final {
    std::size_t first_source_point = 0;
    std::size_t source_point_count = 0;
    std::size_t vertex_count = 0;
    std::size_t index_count = 0;
    float first_v = 0;
    float last_v = 0;
};

}

BOOST_AUTO_TEST_CASE(short_lines_and_unsupported_subdivision_levels_are_no_ops_or_rejected)
{
    RibbonPipelineSettings settings;
    RibbonPipeline pipeline;
    bool read = false;
    bool reset = false;
    bool emitted = false;
    const auto read_point = [&](std::size_t) {
        read = true;
        return Make_Line_Point(0);
    };
    const auto random = [] { return std::array<float, 3>{}; };
    const auto reset_random = [&](std::size_t) { reset = true; };
    const auto emit = [&](RibbonPipelineChunk) { emitted = true; };

    settings.subdivision_level = RibbonPipelineMaximumSubdivisionLevel + 1;
    BOOST_CHECK(!pipeline.Build(2, settings, read_point, random, reset_random, emit));
    BOOST_CHECK(!read);
    BOOST_CHECK(!reset);
    BOOST_CHECK(!emitted);

    settings.subdivision_level = 0;
    BOOST_REQUIRE(pipeline.Build(0, settings, read_point, random, reset_random, emit));
    BOOST_REQUIRE(pipeline.Build(1, settings, read_point, random, reset_random, emit));
    BOOST_CHECK(!read);
    BOOST_CHECK(!reset);
    BOOST_CHECK(!emitted);
}

BOOST_AUTO_TEST_CASE(level_zero_chunks_preserve_overlap_tail_global_uvs_and_reader_order)
{
    constexpr std::size_t source_count = 260;
    RibbonPipelineSettings settings;
    settings.subdivision_level = 0;
    settings.width = 2;
    settings.merge_intersections = false;
    settings.texture_mapping = RibbonTextureMapping::Tiled;
    settings.texture_tile_factor = 0.5f;
    settings.use_uniform_color = false;

    std::vector<std::size_t> reads;
    std::vector<std::size_t> resets;
    std::vector<ChunkObservation> chunks;
    unsigned random_calls = 0;
    RibbonPipeline pipeline;
    BOOST_REQUIRE(pipeline.Build(source_count, settings,
        [&](std::size_t index) {
            reads.push_back(index);
            return Make_Line_Point(index);
        },
        [&] {
            ++random_calls;
            return std::array<float, 3>{};
        },
        [&](std::size_t first_source_point) { resets.push_back(first_source_point); },
        [&](RibbonPipelineChunk chunk) {
            chunks.push_back({chunk.first_source_point, chunk.source_point_count,
                chunk.vertices.size(), chunk.indices.size(),
                chunk.vertices.front().uv[1], chunk.vertices.back().uv[1]});
        }));

    const std::vector<std::size_t> expected_reads = [] {
        std::vector<std::size_t> result;
        result.reserve(262);
        for (std::size_t index = 0; index <= 128; ++index) result.push_back(index);
        for (std::size_t index = 128; index <= 256; ++index) result.push_back(index);
        for (std::size_t index = 256; index < 260; ++index) result.push_back(index);
        return result;
    }();
    BOOST_CHECK_EQUAL_COLLECTIONS(reads.begin(), reads.end(), expected_reads.begin(), expected_reads.end());
    const std::vector<std::size_t> expected_resets{0, 128, 256};
    BOOST_CHECK_EQUAL_COLLECTIONS(resets.begin(), resets.end(),
        expected_resets.begin(), expected_resets.end());
    BOOST_CHECK_EQUAL(random_calls, 0u);
    BOOST_REQUIRE_EQUAL(chunks.size(), 3u);

    const std::array<ChunkObservation, 3> expected_chunks{{
        {0, 129, 258, 768, 0, 64},
        {128, 129, 258, 768, 64, 128},
        {256, 4, 8, 18, 128, 129.5f}}};
    for (std::size_t index = 0; index < expected_chunks.size(); ++index) {
        BOOST_CHECK_EQUAL(chunks[index].first_source_point, expected_chunks[index].first_source_point);
        BOOST_CHECK_EQUAL(chunks[index].source_point_count, expected_chunks[index].source_point_count);
        BOOST_CHECK_EQUAL(chunks[index].vertex_count, expected_chunks[index].vertex_count);
        BOOST_CHECK_EQUAL(chunks[index].index_count, expected_chunks[index].index_count);
        BOOST_CHECK_EQUAL(chunks[index].first_v, expected_chunks[index].first_v);
        BOOST_CHECK_EQUAL(chunks[index].last_v, expected_chunks[index].last_v);
    }
}

BOOST_AUTO_TEST_CASE(level_seven_chunks_reset_external_noise_and_keep_each_seam)
{
    constexpr std::size_t source_count = 130;
    constexpr std::size_t chunk_count = source_count - 1;
    constexpr std::size_t calls_per_chunk = 127;
    RibbonPipelineSettings settings;
    settings.subdivision_level = RibbonPipelineMaximumSubdivisionLevel;
    settings.noise_amplitude = 0;
    settings.width = 2;
    settings.merge_intersections = false;

    struct NoiseEvent final {
        std::size_t chunk = 0;
        unsigned call = 0;
    };
    std::vector<std::size_t> reads;
    std::vector<std::size_t> resets;
    std::vector<NoiseEvent> noise_events;
    std::vector<ChunkObservation> chunks;
    std::size_t current_chunk = 0;
    unsigned call_in_chunk = 0;
    RibbonPipeline pipeline;
    BOOST_REQUIRE(pipeline.Build(source_count, settings,
        [&](std::size_t index) {
            reads.push_back(index);
            return Make_Line_Point(index);
        },
        [&] {
            noise_events.push_back({current_chunk, call_in_chunk});
            ++call_in_chunk;
            return std::array<float, 3>{1, 2, 3};
        },
        [&](std::size_t first_source_point) {
            resets.push_back(first_source_point);
            current_chunk = resets.size() - 1;
            call_in_chunk = 0;
        },
        [&](RibbonPipelineChunk chunk) {
            chunks.push_back({chunk.first_source_point, chunk.source_point_count,
                chunk.vertices.size(), chunk.indices.size(), 0, 0});
        }));

    BOOST_REQUIRE_EQUAL(reads.size(), chunk_count * 2);
    for (std::size_t chunk = 0; chunk < chunk_count; ++chunk) {
        BOOST_CHECK_EQUAL(reads[chunk * 2], chunk);
        BOOST_CHECK_EQUAL(reads[chunk * 2 + 1], chunk + 1);
    }
    BOOST_REQUIRE_EQUAL(resets.size(), chunk_count);
    for (std::size_t chunk = 0; chunk < chunk_count; ++chunk)
        BOOST_CHECK_EQUAL(resets[chunk], chunk);

    BOOST_REQUIRE_EQUAL(noise_events.size(), chunk_count * calls_per_chunk);
    for (std::size_t chunk = 0; chunk < chunk_count; ++chunk) {
        for (unsigned call = 0; call < calls_per_chunk; ++call) {
            const auto event = noise_events[chunk * calls_per_chunk + call];
            BOOST_CHECK_EQUAL(event.chunk, chunk);
            BOOST_CHECK_EQUAL(event.call, call);
        }
    }
    BOOST_REQUIRE_EQUAL(chunks.size(), chunk_count);
    for (const auto &chunk : chunks) {
        BOOST_CHECK_EQUAL(chunk.source_point_count, 2u);
        BOOST_CHECK_EQUAL(chunk.vertex_count, 258u);
        BOOST_CHECK_EQUAL(chunk.index_count, 768u);
    }
}

BOOST_AUTO_TEST_CASE(nonzero_noise_preserves_recursive_random_application_order)
{
    RibbonPipelineSettings settings;
    settings.subdivision_level = 2;
    settings.noise_amplitude = 0.5f;
    settings.width = 2;
    settings.merge_intersections = false;

    const std::array<float, 3> offsets{0.2f, -0.2f, 0.4f};
    unsigned random_calls = 0;
    std::vector<PropVertex> vertices;
    RibbonPipeline pipeline;
    BOOST_REQUIRE(pipeline.Build(2, settings,
        [](std::size_t index) {
            RibbonPoint point = Make_Line_Point(index);
            point.position[0] = index == 0 ? -1.0f : 1.0f;
            return point;
        },
        [&] {
            const float offset = random_calls < offsets.size() ? offsets[random_calls] : 0;
            ++random_calls;
            return std::array<float, 3>{offset, 0, 0};
        },
        [](std::size_t) {},
        [&](RibbonPipelineChunk chunk) {
            vertices.assign(chunk.vertices.begin(), chunk.vertices.end());
        }));

    BOOST_CHECK_EQUAL(random_calls, 3u);
    BOOST_REQUIRE_EQUAL(vertices.size(), 10u);
    const std::array<float, 5> expected_points{-1.0f, -0.5f, 0.1f, 0.65f, 1.0f};
    for (std::size_t point = 0; point < expected_points.size(); ++point) {
        BOOST_CHECK_CLOSE(vertices[point * 2].position[0], expected_points[point], 0.001f);
        BOOST_CHECK_CLOSE(vertices[point * 2 + 1].position[0], expected_points[point], 0.001f);
    }
}

BOOST_AUTO_TEST_CASE(read_point_colors_uniform_colors_and_view_world_transform_are_prepared)
{
    const std::array<RibbonPoint, 2> source{{
        RibbonPoint{{-1, 0, -5}, {0.1f, 0.5f, 0.9f, 0.3f}, 91},
        RibbonPoint{{1, 0, -5}, {0.9f, 0.3f, 0.1f, 0.7f}, 92}}};
    RibbonPipelineSettings settings;
    settings.width = 2;
    settings.merge_intersections = false;
    settings.use_uniform_color = false;
    // x' = 2 * (0.5 * x + 0.1) + 0.25 = x + 0.45.
    settings.view[0] = 2;
    settings.view[3] = 0.25f;
    settings.world[0] = 0.5f;
    settings.world[3] = 0.1f;

    RibbonPipeline pipeline;
    std::vector<PropVertex> read_point_vertices;
    BOOST_REQUIRE(pipeline.Build(source.size(), settings,
        [&](std::size_t index) { return source[index]; },
        [] { return std::array<float, 3>{}; },
        [](std::size_t) {},
        [&](RibbonPipelineChunk chunk) {
            read_point_vertices.assign(chunk.vertices.begin(), chunk.vertices.end());
        }));
    BOOST_REQUIRE_EQUAL(read_point_vertices.size(), 4u);
    BOOST_CHECK_CLOSE(read_point_vertices[0].position[0], -0.55f, 0.001f);
    BOOST_CHECK_CLOSE(read_point_vertices[1].position[0], -0.55f, 0.001f);
    BOOST_CHECK_CLOSE(read_point_vertices[2].position[0], 1.45f, 0.001f);
    BOOST_CHECK_CLOSE(read_point_vertices[3].position[0], 1.45f, 0.001f);
    const std::array<float, 4> first_color{
        26.0f / 255.0f, 128.0f / 255.0f, 230.0f / 255.0f, 77.0f / 255.0f};
    const std::array<float, 4> last_color{
        230.0f / 255.0f, 77.0f / 255.0f, 26.0f / 255.0f, 179.0f / 255.0f};
    BOOST_CHECK(read_point_vertices[0].color == first_color);
    BOOST_CHECK(read_point_vertices[1].color == first_color);
    BOOST_CHECK(read_point_vertices[2].color == last_color);
    BOOST_CHECK(read_point_vertices[3].color == last_color);

    settings.use_uniform_color = true;
    settings.uniform_color = {0.2f, 0.4f, 0.6f};
    settings.uniform_opacity = 0.35f;
    std::vector<PropVertex> uniform_vertices;
    BOOST_REQUIRE(pipeline.Build(source.size(), settings,
        [&](std::size_t index) {
            RibbonPoint point = source[index];
            point.color = {
                std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::quiet_NaN(),
                std::numeric_limits<float>::quiet_NaN()};
            return point;
        },
        [] { return std::array<float, 3>{}; },
        [](std::size_t) {},
        [&](RibbonPipelineChunk chunk) {
            uniform_vertices.assign(chunk.vertices.begin(), chunk.vertices.end());
        }));
    BOOST_REQUIRE_EQUAL(uniform_vertices.size(), 4u);
    const std::array<float, 4> uniform_color{
        51.0f / 255.0f, 102.0f / 255.0f, 153.0f / 255.0f, 89.0f / 255.0f};
    for (const auto &vertex : uniform_vertices)
        BOOST_CHECK(vertex.color == uniform_color);
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(pipeline_chunks_draw_distinct_regions_and_both_ribbon_sides)
{
    constexpr unsigned target_width = 128;
    constexpr unsigned target_height = 32;
    constexpr std::size_t source_count = 260;
    for (const bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));

        const auto target = device.Create_Texture({target_width, target_height, 1,
            RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({target_width, target_height, 1,
            RHITextureFormat::D32_Float, static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
        auto &commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Set_Viewport({0, 0, target_width, target_height}));
        BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));

        std::vector<RibbonPoint> points(source_count);
        for (std::size_t index = 0; index < source_count; ++index) {
            RibbonPoint &point = points[index];
            if (index < 128)
                point.position = {-0.9f + static_cast<float>(index) * 0.79f / 127, 0, 0.5f};
            else if (index < 256)
                point.position = {-0.1f + static_cast<float>(index - 128) * 0.19f / 127, 0, 0.5f};
            else
                point.position = {0.11f + static_cast<float>(index - 256) * 0.79f / 3, 0, 0.5f};
            point.color = index < 128 ? std::array<float, 4>{1, 0, 0, 1}
                : index < 256 ? std::array<float, 4>{0, 1, 0, 1}
                : std::array<float, 4>{0, 0, 1, 1};
        }

        RibbonPipelineSettings settings;
        settings.view[0] = 0.5f;
        settings.view[3] = 0.2f;
        settings.width = 0.5f;
        settings.merge_intersections = false;
        settings.use_uniform_color = false;
        RibbonPipeline pipeline;
        std::vector<PropMeshHandle> meshes;
        bool mesh_creation_ok = true;
        BOOST_REQUIRE(pipeline.Build(source_count, settings,
            [&](std::size_t index) { return points[index]; },
            [] { return std::array<float, 3>{}; },
            [](std::size_t) {},
            [&](RibbonPipelineChunk chunk) {
                const PropMeshHandle mesh = renderer.Create_Mesh(chunk.vertices, chunk.indices);
                if (!mesh.Is_Valid()) {
                    mesh_creation_ok = false;
                    return;
                }
                meshes.push_back(mesh);
            }));
        BOOST_REQUIRE(mesh_creation_ok);
        BOOST_REQUIRE_EQUAL(meshes.size(), 3u);
        points.clear();

        PropStyle style;
        style.blend = RHIBlendMode::Disabled;
        style.depth_test = false;
        style.depth_write = false;
        style.cull = RHICullMode::None;
        PropParameters parameters;
        parameters.textured = 0;
        parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 1};
        for (const PropMeshHandle mesh : meshes)
            BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {}));

        std::vector<std::byte> pixels(target_width * target_height * 4);
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, target_width * 4));
        const auto check = [&](unsigned x, unsigned y, std::array<unsigned, 4> expected) {
            const std::size_t offset = (static_cast<std::size_t>(y) * target_width + x) * 4;
            for (unsigned channel = 0; channel < 4; ++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + channel])
                    - static_cast<int>(expected[channel]), 2);
        };
        // The non-identity view moves the three source chunk regions to these
        // screen positions. Samples above and below the centerline exercise
        // the two ribbon sides; the outside sample remains untouched background.
        for (const unsigned y : {14u, 16u, 18u}) {
            check(62, y, {255, 0, 0, 255});
            check(77, y, {0, 255, 0, 255});
            check(94, y, {0, 0, 255, 255});
        }
        check(10, 2, {0, 0, 0, 0});

        for (const PropMeshHandle mesh : meshes) {
            BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
        }
        renderer.Shutdown();
        device.Destroy_Texture(target);
        device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(merged_corner_pipeline_geometry_covers_both_fan_sides)
{
    constexpr unsigned target_width = 64;
    constexpr unsigned target_height = 64;
    for (const bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));

        const auto target = device.Create_Texture({target_width, target_height, 1,
            RHITextureFormat::RGBA8_UNorm, static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({target_width, target_height, 1,
            RHITextureFormat::D32_Float, static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
        auto &commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Set_Viewport({0, 0, target_width, target_height}));
        BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));

        const std::array<RibbonPoint, 4> points{{
            RibbonPoint{{-0.8f, -0.2f, 0.5f}, {1, 0, 0, 1}, 0},
            RibbonPoint{{-0.3f, -0.2f, 0.5f}, {1, 0, 0, 1}, 1},
            RibbonPoint{{0.0f, 0.4f, 0.5f}, {1, 0, 0, 1}, 2},
            RibbonPoint{{0.8f, 0.4f, 0.5f}, {1, 0, 0, 1}, 3}}};
        RibbonPipelineSettings settings;
        settings.width = 0.2f;
        settings.merge_intersections = true;
        settings.merge_abort_factor = 0;
        settings.use_uniform_color = false;
        RibbonPipeline pipeline;
        PropMeshHandle mesh{};
        BOOST_REQUIRE(pipeline.Build(points.size(), settings,
            [&](std::size_t index) { return points[index]; },
            [] { return std::array<float, 3>{}; },
            [](std::size_t) {},
            [&](RibbonPipelineChunk chunk) { mesh = renderer.Create_Mesh(chunk.vertices, chunk.indices); }));
        BOOST_REQUIRE(mesh.Is_Valid());

        PropStyle style;
        style.blend = RHIBlendMode::Disabled;
        style.depth_test = false;
        style.depth_write = false;
        style.cull = RHICullMode::None;
        PropParameters parameters;
        parameters.textured = 0;
        parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 1};
        BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {}));

        std::vector<std::byte> pixels(target_width * target_height * 4);
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, target_width * 4));
        const auto check = [&](unsigned x, unsigned y, std::array<unsigned, 4> expected) {
            const std::size_t offset = (static_cast<std::size_t>(y) * target_width + x) * 4;
            for (unsigned channel = 0; channel < 4; ++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + channel])
                    - static_cast<int>(expected[channel]), 2);
        };
        // These samples lie in the two sides of each arm and in the turn's
        // merged fans. The final sample is outside the complete ribbon.
        check(14, 36, {255, 0, 0, 255});
        check(14, 40, {255, 0, 0, 255});
        check(25, 29, {255, 0, 0, 255});
        check(29, 29, {255, 0, 0, 255});
        check(45, 17, {255, 0, 0, 255});
        check(45, 21, {255, 0, 0, 255});
        check(2, 2, {0, 0, 0, 0});

        BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
        renderer.Shutdown();
        device.Destroy_Texture(target);
        device.Destroy_Texture(depth);
    }
}
#endif
