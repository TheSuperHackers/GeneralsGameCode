module;
#define BOOST_TEST_MODULE ParticleTextureDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>
export module Graphics.Scene.Particles.TextureDrawing.Tests;
import Assets.Cache.TextureLoadTask;
import Assets.Adapters.DDS;
import Assets.Math;
import Assets.Images.Preparation;
import Assets.Identity;
import Assets.Textures;
import Graphics.Scene.Particles.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(texture_regions_select_distinct_atlas_cells_after_source_release_and_resize)
{
    const std::array<std::array<unsigned, 4>, 4> colors{{
        {255, 0, 0, 255}, {0, 255, 0, 128}, {255, 255, 255, 0}, {255, 255, 0, 192}}};
    for (bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        for (auto mode : {ParticleEmitterFlags::Billboard, ParticleEmitterFlags::PointSprite, ParticleEmitterFlags::None}) {
            ParticleRenderer renderer;
            BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 4));
            Texture description;
            description.width = description.height = 8;
            description.mip_count = 1;
            description.format = TextureFormat::RGBA8_UNorm;
            description.usage = TextureUsage::Sampled;
            description.row_pitch = 32;
            std::array<std::byte, 8 * 8 * 4> pixels{};
            for (unsigned y = 0; y < 8; ++y)
                for (unsigned x = 0; x < 8; ++x)
                    for (unsigned c = 0; c < 4; ++c)
                        pixels[(y * 8 + x) * 4 + c] = std::byte(colors[(y / 4) * 2 + x / 4][c]);
            const auto texture = renderer.Create_Texture(description, pixels);
            BOOST_REQUIRE(texture.Is_Valid());
            Material material;
            material.shader = renderer.Particle_Shader();
            material.textures[0] = texture;
            const auto material_handle = renderer.Create_Material(material);
            BOOST_REQUIRE(material_handle.Is_Valid());
            ParticleEmitter emitter;
            emitter.position = {0, 0, .5f};
            emitter.flags = ParticleEmitterFlags::Enabled | mode;
            emitter.material = material_handle;
            emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
            emitter.particle_size = mode == ParticleEmitterFlags::PointSprite ? 8.0f : .16f;
            emitter.max_particles = 4;
            const auto handle = renderer.Create_Emitter(emitter);
            BOOST_REQUIRE(handle.Is_Valid());
            {
                ParticleSystem source;
                source.Reserve(1, 4);
                const auto source_handle = source.Create_Emitter(emitter);
                BOOST_REQUIRE(source.Spawn(source_handle, 4));
                auto data = source.Particles();
                const std::array<float, 4> positions{-.75f, -.25f, .25f, .75f};
                const std::array<ParticleTextureRegion, 4> regions{{
                    {0, 0, .5f, .5f}, {.5f, 0, 1, .5f}, {0, .5f, .5f, 1}, {.5f, .5f, 1, 1}}};
                data.position_x = positions;
                data.texture_regions = regions;
                BOOST_REQUIRE(renderer.Append_Particles(handle, data));
            }
            for (unsigned width : {64u, 96u, 64u}) {
                constexpr unsigned height = 32;
                BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {},
                    {0, 0, float(width), float(height), 0, 1}}));
                const auto target = device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
                    static_cast<unsigned>(RHITextureUsage::RenderTarget)});
                const auto depth = device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
                    static_cast<unsigned>(RHITextureUsage::DepthStencil)});
                auto& commands = device.Immediate_Command_List();
                BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
                BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 1));
                BOOST_REQUIRE(renderer.Render(commands, target, depth, {0, 0, width, height}));
                std::vector<std::byte> result(width * height * 4);
                BOOST_REQUIRE(device.Readback_Texture(target, result, width * 4));
                for (unsigned i = 0; i < 4; ++i) {
                    const unsigned center = ((height / 2) * width + (2 * i + 1) * width / 8) * 4;
                    const unsigned alpha = colors[i][3];
                    for (unsigned c = 0; c < 3; ++c) {
                        const int expected = static_cast<int>(colors[i][c] * alpha / 255 + (c == 2 ? 255 - alpha : 0));
                        BOOST_CHECK_SMALL(std::to_integer<int>(result[center + c]) - expected, 2);
                    }
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(result[center + 3]), 255);
                }
                device.Destroy_Texture(target);
                device.Destroy_Texture(depth);
            }
            renderer.Shutdown();
        }
    }
}

BOOST_AUTO_TEST_CASE(prepared_mip_chain_retains_recolor_and_alpha_in_gpu_drawing)
{
    std::array<std::byte, 8 * 4 * 4> source{};
    for (unsigned y = 0; y < 4; ++y) for (unsigned x = 0; x < 8; ++x)
        BOOST_REQUIRE(Assets::Write_Image_Pixel(std::span(source).subspan((y * 8 + x) * 4, 4),
            Assets::PixelEncoding::BGRA8, x < 4 ? 0xffff0000 : 0xff0000ff));
    std::vector<Assets::PreparedImage> levels;
    BOOST_REQUIRE(Assets::Prepare_Image_Levels({source, 8, 4, 32, Assets::PixelEncoding::BGRA8, {}},
        Assets::PixelEncoding::BGRA8, 8, 4, 4, {120, 0, 0}, levels));
    const auto& last = levels.back();
    BOOST_REQUIRE_EQUAL(last.width, 1u); BOOST_REQUIRE_EQUAL(last.height, 1u);
    std::array<std::byte, 4> rgba{};
    BOOST_REQUIRE(Assets::Convert_Image_Pixel(rgba, Assets::PixelEncoding::RGBA8, last.bytes, last.encoding));
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    for (unsigned cycle = 0; cycle < 2; ++cycle) {
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));
        Texture texture;
        texture.width = texture.height = texture.mip_count = 1; texture.row_pitch = 4;
        texture.format = TextureFormat::RGBA8_UNorm; texture.usage = TextureUsage::Sampled;
        const auto image = renderer.Create_Texture(texture, rgba);
        BOOST_REQUIRE(image.Is_Valid());
        Material material;
        material.shader = renderer.Particle_Shader(); material.textures[0] = image;
        const auto material_handle = renderer.Create_Material(material);
        BOOST_REQUIRE(material_handle.Is_Valid());
        ParticleEmitter emitter;
        emitter.material = material_handle; emitter.color = {1, 1, 1, 1}; emitter.position = {0, 0, 0.5f};
        emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::PointSprite;
        emitter.particle_size = 16; emitter.max_particles = 1;
        emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
        const auto handle = renderer.Create_Emitter(emitter);
        BOOST_REQUIRE(renderer.Spawn(handle, 1));
        const unsigned width = cycle == 0 ? 64 : 96, height = cycle == 0 ? 48 : 72;
        BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {},
            {0, 0, float(width), float(height), 0, 1}}));
        const auto target = device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 1));
        BOOST_REQUIRE(renderer.Render(commands, target, depth, {0, 0, width, height}));
        std::vector<std::byte> pixels(width * height * 4);
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4));
        const unsigned center = ((height / 2) * width + width / 2) * 4;
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center]) - 125, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 1]) - 125, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 2]) - 3, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 3]) - 255, 2);
        renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(reduced_cube_texture_recolor_retains_alpha_through_drawing_and_restart)
{
    std::vector<std::byte> source(128 + 112 * 6);
    const auto put = [&](unsigned offset, unsigned value) {
        for (unsigned i = 0; i < 4; ++i) source[offset + i] = std::byte(value >> (i * 8));
    };
    put(0, 0x20534444); put(4, 124); put(12, 8); put(16, 8); put(28, 4);
    put(76, 32); put(80, 4); put(84, 0x35545844); put(112, 0xfe00);
    // Last cube face, second authored mip. All other subresources stay black.
    const unsigned offset = 128 + 112 * 5 + 64;
    put(offset, 0x00008080); put(offset + 8, 0xf800f800);
    Assets::DDSLayout layout;
    BOOST_REQUIRE(Assets::Read_DDS_Layout(source, source.size(), layout));
    const auto* surface = layout.Surface(1, 5);
    BOOST_REQUIRE(surface);
    std::vector<std::byte> recolored(surface->size);
    BOOST_REQUIRE(Assets::Copy_DDS_Blocks(source, *surface, recolored, surface->row_pitch,
        surface->slice_pitch, layout.compression, {120, 0, 0}));
    Assets::DDSLayout selected;
    selected.compression = layout.compression; selected.mip_count = 1;
    selected.surfaces.push_back(*surface); selected.surfaces[0].offset = 0;
    std::vector<std::byte> rgba;
    BOOST_REQUIRE(Assets::Decode_DDS_Surface(recolored, selected, 0, 0, 0, rgba));
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    for (unsigned cycle = 0; cycle < 2; ++cycle) {
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));
        Texture texture;
        texture.width = texture.height = 4; texture.mip_count = 1;
        texture.row_pitch = 16; texture.format = TextureFormat::RGBA8_UNorm; texture.usage = TextureUsage::Sampled;
        const auto image = renderer.Create_Texture(texture, rgba);
        BOOST_REQUIRE(image.Is_Valid());
        Material material;
        material.shader = renderer.Particle_Shader(); material.textures[0] = image;
        const auto material_handle = renderer.Create_Material(material);
        BOOST_REQUIRE(material_handle.Is_Valid());
        ParticleEmitter emitter;
        emitter.material = material_handle;
        emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::PointSprite;
        emitter.position = {0, 0, 0.5f}; emitter.color = {1, 1, 1, 1};
        emitter.particle_size = 16; emitter.max_particles = 1;
        emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
        const auto handle = renderer.Create_Emitter(emitter);
        BOOST_REQUIRE(renderer.Spawn(handle, 1));
        const unsigned width = cycle == 0 ? 64 : 96, height = cycle == 0 ? 48 : 72;
        BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {},
            {0, 0, float(width), float(height), 0, 1}}));
        const auto target = device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 1));
        BOOST_REQUIRE(renderer.Render(commands, target, depth, {0, 0, width, height}));
        std::vector<std::byte> pixels(width * height * 4);
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4));
        const unsigned center = ((height / 2) * width + width / 2) * 4;
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center]), 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 1]) - 125, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 2]) - 127, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 3]) - 255, 2);
        renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(restart_invalidates_particle_resources_and_preserves_alpha_after_target_resize)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    TextureHandle previous_texture;
    MaterialHandle previous_material;
    ParticleEmitterHandle previous_emitter;
    for (unsigned cycle = 0; cycle < 2; ++cycle) {
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 2, 2));
        Texture description;
        description.width = description.height = description.mip_count = 1;
        description.format = TextureFormat::RGBA8_UNorm;
        description.usage = TextureUsage::Sampled;
        description.row_pitch = 4;
        const unsigned recolored = Assets::Shift_Color_ARGB(0x40ff0000, {120, 0, 0});
        const std::array<std::byte, 4> pixel{std::byte(recolored >> 16), std::byte(recolored >> 8),
            std::byte(recolored), std::byte(recolored >> 24)};
        const auto texture = renderer.Create_Texture(description, pixel);
        BOOST_REQUIRE(texture.Is_Valid());
        Material material;
        material.shader = renderer.Particle_Shader();
        material.textures[0] = texture;
        const auto material_handle = renderer.Create_Material(material);
        BOOST_REQUIRE(material_handle.Is_Valid());
        ParticleEmitter emitter;
        emitter.material = material_handle;
        emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::PointSprite;
        emitter.color = {1, 1, 1, 1};
        emitter.position = {0, 0, 0.5f};
        emitter.particle_size = 16;
        emitter.max_particles = 1;
        emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
        const auto handle = renderer.Create_Emitter(emitter);
        BOOST_REQUIRE(handle.Is_Valid());
        if (cycle != 0) {
            BOOST_CHECK(!renderer.Destroy_Texture(previous_texture));
            BOOST_CHECK(!renderer.Destroy_Material(previous_material));
            BOOST_CHECK(!renderer.Destroy_Emitter(previous_emitter));
        }
        BOOST_REQUIRE(renderer.Spawn(handle, 1));
        const unsigned width = cycle == 0 ? 64 : 96;
        const unsigned height = cycle == 0 ? 48 : 72;
        const auto target = device.Create_Texture({width, height, 1, RHITextureFormat::RGBA8_UNorm,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({width, height, 1, RHITextureFormat::D32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {},
            {0, 0, float(width), float(height), 0, 1}}));
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 1));
        BOOST_REQUIRE(renderer.Render(commands, target, depth, {0, 0, width, height}));
        std::vector<std::byte> pixels(width * height * 4);
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, width * 4));
        const unsigned center = ((height / 2) * width + width / 2) * 4;
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 1]) - 64, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 2]) - 191, 2);
        previous_texture = texture;
        previous_material = material_handle;
        previous_emitter = handle;
        renderer.Shutdown();
        device.Destroy_Texture(target);
        device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(point_sprite_retains_pixel_size_texture_alpha_and_viewport_origin)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));
    Texture description;
    description.width = description.height = description.mip_count = 1;
    description.format = TextureFormat::RGBA8_UNorm;
    description.usage = TextureUsage::Sampled;
    description.row_pitch = 4;
    const std::array<std::byte,4> pixel{std::byte{255},std::byte{0},std::byte{0},std::byte{128}};
    const auto texture = renderer.Create_Texture(description, pixel);
    BOOST_REQUIRE(texture.Is_Valid());
    Material material;
    material.shader = renderer.Particle_Shader();
    material.textures[0] = texture;
    const auto material_handle = renderer.Create_Material(material);
    BOOST_REQUIRE(material_handle.Is_Valid());
    ParticleEmitter emitter;
    emitter.material = material_handle;
    emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::PointSprite;
    emitter.color = {1,1,1,1};
    emitter.position = {0,0,0.5f};
    emitter.particle_size = 16;
    emitter.max_particles = 1;
    emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
    const auto handle = renderer.Create_Emitter(emitter);
    BOOST_REQUIRE(handle.Is_Valid());
    BOOST_REQUIRE(renderer.Spawn(handle,1));
    const auto target = device.Create_Texture({128,96,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({128,96,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    for (const RHIViewport viewport : {RHIViewport{0,0,64,64}, RHIViewport{16,8,96,80}}) {
        BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(),Matrix4x4::Identity(),{},
            {float(viewport.x),float(viewport.y),float(viewport.width),float(viewport.height),0,1}}));
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
        BOOST_REQUIRE(renderer.Render(commands,target,depth,viewport));
        std::vector<std::byte> pixels(128*96*4);
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,128*4));
        const int center_x = int(viewport.x+viewport.width/2);
        const int center_y = int(viewport.y+viewport.height/2);
        for (int y=center_y-10;y<center_y+10;++y)
            for (int x=center_x-10;x<center_x+10;++x) {
                const bool inside = x>=center_x-8 && x<center_x+8 && y>=center_y-8 && y<center_y+8;
                const unsigned offset = (y*128+x)*4;
                BOOST_TEST_CONTEXT("pixel " << x << "," << y) {
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset])-(inside?128:0),2);
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+2])-(inside?127:255),2);
                }
            }
    }
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(ground_aligned_particles_preserve_corner_depth)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY),1,1));
    ParticleEmitter emitter;
    emitter.flags = ParticleEmitterFlags::Enabled;
    emitter.material = renderer.Default_Material();
    emitter.particle_size = 0.5f;
    emitter.position = {0,0,0};
    emitter.max_particles = 1;
    emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
    const auto handle = renderer.Create_Emitter(emitter);
    BOOST_REQUIRE(renderer.Spawn(handle,1));
    auto view = Matrix4x4::Identity();
    constexpr float cosine = 0.70710678f;
    view.values[5] = cosine;
    view.values[6] = -cosine;
    view.values[9] = cosine;
    view.values[10] = cosine;
    view.values[11] = 0.5f;
    BOOST_REQUIRE(renderer.Set_View({view,Matrix4x4::Identity(),{}, {0,0,64,64,0,1}}));
    const auto target = device.Create_Texture({64,64,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({64,64,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Clear({0,0,1,1},0.5f));
    BOOST_REQUIRE(renderer.Render(commands,target,depth,{0,0,64,64}));
    std::array<std::byte,64*64*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
    // World +Y is behind the opaque depth; -Y is in front. A constant
    // center depth incorrectly draws both halves over the occluder.
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(26*64+32)*4]),0);
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(38*64+32)*4]),255);
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(alpha_test_preserves_authored_cutoff_boundary)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY),1,1));
    const auto target = device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(),Matrix4x4::Identity(),{}, {0,0,32,32,0,1}}));
    auto& commands = device.Immediate_Command_List();
    for (const unsigned alpha : {95u,96u,110u}) {
        renderer.Reset_Particles();
        Texture description;
        description.width = description.height = description.mip_count = 1;
        description.format = TextureFormat::RGBA8_UNorm;
        description.usage = TextureUsage::Sampled;
        description.row_pitch = 4;
        const std::array<std::byte,4> pixel{std::byte{255},std::byte{0},std::byte{0},std::byte(alpha)};
        const auto texture = renderer.Create_Texture(description,pixel);
        Material material;
        material.shader = renderer.Particle_Shader();
        material.textures[0] = texture;
        const auto material_handle = renderer.Create_Material(material);
        ParticleEmitter emitter;
        emitter.material = material_handle;
        emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::PointSprite | ParticleEmitterFlags::AlphaTest;
        emitter.position = {0,0,0.5f};
        emitter.particle_size = 16;
        emitter.max_particles = 1;
        emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
        const auto handle = renderer.Create_Emitter(emitter);
        BOOST_REQUIRE(renderer.Spawn(handle,1));
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
        BOOST_REQUIRE(renderer.Render(commands,target,depth,{0,0,32,32}));
        std::array<std::byte,32*32*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(16*32+16)*4]),alpha >= 96 ? 255 : 0);
        renderer.Destroy_Emitter(handle);
        renderer.Destroy_Material(material_handle);
        renderer.Destroy_Texture(texture);
    }
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(effect_textures_can_exceed_one_gpu_binding_page)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    constexpr unsigned count = 130;
    constexpr unsigned width = 136, height = 64;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), count, count));
    BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {}, {0,0,width,height,0,1}}));
    Texture description;
    description.width = description.height = description.mip_count = 1;
    description.format = TextureFormat::RGBA8_UNorm;
    description.usage = TextureUsage::Sampled;
    description.row_pitch = 4;
    for (unsigned i = 0; i < count; ++i) {
        BOOST_TEST_CONTEXT("effect texture " << i) {
            const std::array<std::byte,4> pixel{std::byte(i+1), std::byte(255-i), std::byte{0}, std::byte{255}};
            const auto texture = renderer.Create_Texture(description, pixel);
            BOOST_REQUIRE(texture.Is_Valid());
            Material material;
            material.shader = renderer.Particle_Shader();
            material.textures[0] = texture;
            const auto material_handle = renderer.Create_Material(material);
            BOOST_REQUIRE(material_handle.Is_Valid());
            ParticleEmitter emitter;
            emitter.material = material_handle;
            emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard;
            emitter.color = {1,1,1,1};
            emitter.position = {2.0f*(4+(i%17)*8)/width-1, 1-2.0f*(4+(i/17)*8)/height, 0.5f};
            emitter.particle_size = 0.025f;
            emitter.max_particles = 1;
            emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
            const auto handle = renderer.Create_Emitter(emitter);
            BOOST_REQUIRE(handle.Is_Valid());
            BOOST_REQUIRE(renderer.Spawn(handle, 1));
        }
    }
    const auto target = device.Create_Texture({width,height,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({width,height,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Clear({0,0,1,1}, 1));
    BOOST_REQUIRE(renderer.Render(commands, target, depth, {0,0,width,height}));
    std::vector<std::byte> pixels(width*height*4);
    BOOST_REQUIRE(device.Readback_Texture(target, pixels, width*4));
    for (unsigned i = 0; i < count; ++i) {
        BOOST_TEST_CONTEXT("drawn effect " << i) {
            const unsigned offset = ((4+(i/17)*8)*width+4+(i%17)*8)*4;
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset])-int(i+1), 2);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+1])-int(255-i), 2);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[offset+2]), 0);
        }
    }
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(compressed_particle_texture_retains_color_and_cutout)
{
    // Two BC3 blocks: transparent green followed by opaque green. The game
    // names compressed assets with .tga as well as .dds, so decode by content.
    std::vector<std::byte> encoded(160);
    const auto put=[&](unsigned offset,std::uint32_t value) {
        for (unsigned i=0;i<4;++i) encoded[offset+i]=static_cast<std::byte>(value>>(i*8));
    };
    put(0,0x20534444); put(4,124); put(12,4); put(16,8);
    put(76,32); put(80,4); put(84,0x35545844);
    put(136,0x07e007e0); put(152,0x07e007e0);
    encoded[144]=std::byte{255}; encoded[145]=std::byte{255};
    const auto loaded=Assets::Load_Texture_Asset({Assets::AssetType::Texture,"smoke.tga"},
        [&](const Assets::AssetIdentity&) { return encoded; });
    BOOST_REQUIRE(loaded.Succeeded()); BOOST_REQUIRE(loaded.asset->Has_Pixels());
    BOOST_CHECK_EQUAL(std::to_integer<int>(loaded.asset->Pixels()[3]),0);
    BOOST_CHECK_EQUAL(std::to_integer<int>(loaded.asset->Pixels()[19]),255);
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY),2,2));
    const RHIViewport viewport{0,0,128,72};
    BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(),Matrix4x4::Identity(),{},{0,0,128,72,0,1}}));
    Texture description;
    description.width=loaded.asset->Width(); description.height=loaded.asset->Height();
    description.mip_count=1; description.format=TextureFormat::RGBA8_UNorm;
    description.usage=TextureUsage::Sampled; description.row_pitch=loaded.asset->Row_Pitch();
    const auto texture=renderer.Create_Texture(description,loaded.asset->Pixels());
    BOOST_REQUIRE(texture.Is_Valid());
    Material material;
    material.shader=renderer.Particle_Shader(); material.textures[0]=texture;
    material.parameters.values[0]=material.parameters.values[1]=material.parameters.values[2]=material.parameters.values[3]=1;
    const auto material_handle=renderer.Create_Material(material);
    BOOST_REQUIRE(material_handle.Is_Valid());
    ParticleEmitter emitter;
    emitter.material=material_handle;
    emitter.flags=ParticleEmitterFlags::Enabled|ParticleEmitterFlags::Billboard;
    emitter.color={1,1,1,1}; emitter.particle_size=0.8f; emitter.max_particles=1;
    emitter.pipeline=renderer.Pipeline_For_Flags(emitter.flags);
    const auto handle=renderer.Create_Emitter(emitter);
    BOOST_REQUIRE(handle.Is_Valid()); BOOST_REQUIRE(renderer.Spawn(handle,1));
    const auto target=device.Create_Texture({128,72,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({128,72,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Render(commands,target,depth,viewport));
    std::vector<std::byte> pixels(128*72*4);
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,128*4));
    const auto check=[&](unsigned x,std::array<int,4> expected) {
        for (unsigned c=0;c<4;++c) {
            BOOST_TEST_CONTEXT("x=" << x << " channel=" << c) {
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(36*128+x)*4+c])-expected[c],2);
            }
        }
    };
    check(40,{0,0,255,255}); check(88,{0,255,0,255});
    // Consecutive particles share an instanced draw even when their textures
    // differ. Each instance must retain its own texture and sampled alpha.
    Texture red_description;
    red_description.width=red_description.height=red_description.mip_count=1;
    red_description.format=TextureFormat::RGBA8_UNorm;
    red_description.usage=TextureUsage::Sampled; red_description.row_pitch=4;
    const std::array<std::byte,4> red_pixel{std::byte{255},std::byte{0},std::byte{0},std::byte{128}};
    const auto red_texture=renderer.Create_Texture(red_description,red_pixel);
    BOOST_REQUIRE(red_texture.Is_Valid());
    material.textures[0]=red_texture;
    const auto red_material=renderer.Create_Material(material);
    BOOST_REQUIRE(red_material.Is_Valid());
    emitter.material=red_material; emitter.position={-0.375f,0,0}; emitter.particle_size=0.1f;
    const auto red_emitter=renderer.Create_Emitter(emitter);
    BOOST_REQUIRE(red_emitter.Is_Valid()); BOOST_REQUIRE(renderer.Spawn(red_emitter,1));
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Render(commands,target,depth,viewport));
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,128*4));
    check(40,{128,0,127,255}); check(88,{0,255,0,255});
    BOOST_REQUIRE(renderer.Destroy_Emitter(red_emitter));
    BOOST_REQUIRE(renderer.Destroy_Material(red_material));
    BOOST_REQUIRE(renderer.Destroy_Texture(red_texture));
    BOOST_REQUIRE(renderer.Destroy_Emitter(handle));
    BOOST_REQUIRE(renderer.Destroy_Material(material_handle));
    BOOST_REQUIRE(renderer.Destroy_Texture(texture));
    renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(ground_and_billboard_particles_preserve_authored_texture_orientation)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    ParticleRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 1, 1));
    const std::array<std::array<std::uint8_t,4>,4> colors{{
        {255,0,0,255}, {0,255,0,255}, {0,0,255,255}, {255,255,0,255}}};
    std::array<std::byte,8*8*4> source{};
    for (unsigned y=0;y<8;++y)
        for (unsigned x=0;x<8;++x)
            for (unsigned channel=0;channel<4;++channel)
                source[(y*8+x)*4+channel]=std::byte(colors[(y/4)*2+x/4][channel]);
    Texture description;
    description.width=description.height=8; description.mip_count=1;
    description.format=TextureFormat::RGBA8_UNorm; description.usage=TextureUsage::Sampled;
    description.row_pitch=8*4;
    const auto texture=renderer.Create_Texture(description,source);
    BOOST_REQUIRE(texture.Is_Valid());
    Material material;
    material.shader=renderer.Particle_Shader(); material.textures[0]=texture;
    const auto material_handle=renderer.Create_Material(material);
    BOOST_REQUIRE(material_handle.Is_Valid());
    const auto target=device.Create_Texture({64,64,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({64,64,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(),Matrix4x4::Identity(),{}, {0,0,64,64,0,1}}));
    for (const bool billboard : {false,true}) {
        ParticleEmitter emitter;
        emitter.material=material_handle; emitter.flags=ParticleEmitterFlags::Enabled;
        if (billboard) emitter.flags=emitter.flags|ParticleEmitterFlags::Billboard;
        emitter.position={0,0,0.5f}; emitter.particle_size=0.625f; emitter.max_particles=1;
        emitter.pipeline=renderer.Pipeline_For_Flags(emitter.flags);
        const auto handle=renderer.Create_Emitter(emitter);
        BOOST_REQUIRE(handle.Is_Valid()); BOOST_REQUIRE(renderer.Spawn(handle,1));
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
        BOOST_REQUIRE(renderer.Render(commands,target,depth,{0,0,64,64}));
        std::array<std::byte,64*64*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
        for (unsigned quadrant=0;quadrant<4;++quadrant) {
            const unsigned x=quadrant%2 ? 42 : 22;
            const unsigned y=quadrant/2 ? 42 : 22;
            // Original ground vertex (+X,+Y) has UV(0,0); billboards
            // assign UV(0,0) to their upper-left view-space vertex.
            const auto& expected=colors[billboard ? quadrant : quadrant^1u];
            for (unsigned channel=0;channel<4;++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*64+x)*4+channel])-expected[channel],2);
        }
        BOOST_REQUIRE(renderer.Destroy_Emitter(handle));
    }
    renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(mixed_particle_blends_preserve_black_borders_and_zero_alpha_flashes)
{
    GraphicsTestDevice device({true});
    ParticleRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_PARTICLE_SHADER_DIRECTORY), 3, 3));
    BOOST_REQUIRE(renderer.Set_View({Matrix4x4::Identity(), Matrix4x4::Identity(), {}, {0,0,96,32,0,1}}));
    Texture desc;
    desc.width = desc.height = 8; desc.mip_count = 1;
    desc.format = TextureFormat::RGBA8_UNorm; desc.usage = TextureUsage::Sampled; desc.row_pitch = 32;
    for (unsigned i = 0; i < 3; ++i) {
        std::array<std::byte,256> pixels{};
        for (unsigned y = 0; y < 8; ++y) for (unsigned x = 0; x < 8; ++x) {
            const auto offset = (y*8+x)*4;
            const bool center = x >= 3 && x < 5 && y >= 3 && y < 5;
            pixels[offset] = std::byte(center ? 255 : 0);
            pixels[offset+3] = std::byte(i == 1 ? (center ? 128 : 0) : 255);
        }
        const auto texture = renderer.Create_Texture(desc, pixels);
        BOOST_REQUIRE(texture.Is_Valid());
        Material material; material.shader = renderer.Particle_Shader(); material.textures[0] = texture;
        ParticleEmitter emitter;
        emitter.material = renderer.Create_Material(material);
        emitter.flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::PointSprite;
        if (i != 1) emitter.flags = emitter.flags | ParticleEmitterFlags::Additive;
        emitter.pipeline = renderer.Pipeline_For_Flags(emitter.flags);
        emitter.position = {float(i*32+16)/48-1, 0, 0.5f};
        emitter.color = {1,1,1,i == 1 ? 1.0f : 0.0f};
        emitter.particle_size = 24; emitter.max_particles = 1;
        const auto handle = renderer.Create_Emitter(emitter);
        BOOST_REQUIRE(renderer.Spawn(handle,1));
    }
    const auto target = device.Create_Texture({96,32,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({96,32,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    for (unsigned frame = 0; frame < 2; ++frame) {
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
        BOOST_REQUIRE(renderer.Render(commands,target,depth,{0,0,96,32}));
        std::vector<std::byte> pixels(96*32*4);
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,96*4));
        for (unsigned i = 0; i < 3; ++i) {
            const auto center = (16*96+i*32+16)*4;
            const auto border = (16*96+i*32+6)*4;
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center])-(i == 1 ? 128 : 255),2);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center+2])-(i == 1 ? 127 : 255),2);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[border]),0);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[border+2]),255);
        }
    }
    renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
