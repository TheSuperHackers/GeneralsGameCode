module;
#define BOOST_TEST_MODULE TextureSamplingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <vector>
export module Graphics.Resources.Textures.Sampling.Tests;
import Graphics.Resources.Textures.Sampling;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(authored_overrides_preserve_quality_modes_and_preference_values)
{
    const auto saved = Get_Texture_Sampling_Settings();
    struct Restore { TextureSamplingSettings saved; ~Restore() {
        Set_Texture_Sampling_Mode(static_cast<int>(saved.mode)); Set_Texture_Anisotropy(saved.anisotropy);
    } } restore{saved};
    for (unsigned i = 0; i < TextureSamplingModeNames.size(); ++i)
        BOOST_CHECK(Parse_Texture_Sampling_Mode(TextureSamplingModeNames[i]) == static_cast<TextureSamplingMode>(i));
    BOOST_CHECK(Parse_Texture_Sampling_Mode("bIlInEaR") == TextureSamplingMode::Bilinear);
    BOOST_CHECK(Parse_Texture_Sampling_Mode("unknown") == TextureSamplingMode::None);
    for (const auto level : {1, 2, 3, 7, 8, 15, 16, 32}) {
        Set_Texture_Anisotropy(level);
        const auto actual = Get_Texture_Sampling_Settings().anisotropy;
        BOOST_CHECK(actual == 2 || actual == 4 || actual == 8 || actual == 16);
        if (level >= 2 && level <= 16) BOOST_CHECK_LE(actual, static_cast<unsigned>(level));
    }
    Set_Texture_Sampling_Mode(-1);
    BOOST_CHECK(Get_Texture_Sampling_Settings().mode == TextureSamplingMode::None);
    Set_Texture_Sampling_Mode(99);
    BOOST_CHECK(Get_Texture_Sampling_Settings().mode == TextureSamplingMode::Anisotropic);
    TextureSampling sampling;
    sampling.address[1] = RHISamplerAddress::Clamp;
    for (const auto mode : {TextureSamplingMode::None, TextureSamplingMode::Point,
        TextureSamplingMode::Bilinear, TextureSamplingMode::Trilinear, TextureSamplingMode::Anisotropic}) {
        auto resolved = Resolve_Texture_Sampling(sampling, {mode, 8});
        BOOST_CHECK(resolved.address[1] == RHISamplerAddress::Clamp);
        BOOST_CHECK(resolved.minification == (mode <= TextureSamplingMode::Point ? RHISamplerFilter::Point : RHISamplerFilter::Linear));
        BOOST_CHECK(resolved.mipmap == (mode <= TextureSamplingMode::Bilinear ? RHISamplerFilter::Point : RHISamplerFilter::Linear));
        BOOST_CHECK_EQUAL(resolved.anisotropy, mode == TextureSamplingMode::Anisotropic ? 8 : 1);
        BOOST_CHECK_EQUAL(resolved.max_lod == 0, mode == TextureSamplingMode::None);
        const auto secondary = Resolve_Texture_Sampling(sampling, {mode, 8}, false);
        BOOST_CHECK_EQUAL(secondary.anisotropy, 1);
    }
    sampling.minification = SamplingFilter::Disabled;
    sampling.magnification = SamplingFilter::Fast;
    sampling.mipmap = SamplingFilter::Disabled;
    const auto mixed = Resolve_Texture_Sampling(sampling, {TextureSamplingMode::Anisotropic, 16});
    BOOST_CHECK(mixed.minification == RHISamplerFilter::Point);
    BOOST_CHECK(mixed.magnification == RHISamplerFilter::Linear);
    BOOST_CHECK_EQUAL(mixed.max_lod, 0);
    BOOST_CHECK_EQUAL(mixed.anisotropy, 1);
    BOOST_CHECK(Make_Texture_Sampling(false).mipmap == SamplingFilter::Disabled);
}

BOOST_AUTO_TEST_CASE(gpu_sampling_distinguishes_minification_magnification_mips_and_lod_clamps)
{
    GraphicsTestDeviceOptions options; options.use_warp = true;
    GraphicsTestDevice device(options);
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({8, 8, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({8, 8, 1, RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const auto texture = device.Create_Texture({8, 8, 4});
    BOOST_REQUIRE(texture.Is_Valid());
    for (unsigned mip = 0; mip < 4; ++mip) {
        const unsigned size = 8 >> mip;
        std::vector<std::uint8_t> bytes(size * size * 4);
        for (unsigned y = 0; y < size; ++y) for (unsigned x = 0; x < size; ++x) {
            const auto offset = (y * size + x) * 4;
            bytes[offset] = mip == 3 || (mip < 2 && x % 2 == 0) ? 255 : 0;
            bytes[offset + 1] = mip == 3 || (mip < 2 && x % 2 != 0) ? 255 : 0;
            bytes[offset + 2] = mip >= 2 ? 255 : 0;
            bytes[offset + 3] = mip == 0 ? 128 : mip == 1 ? 64 : 192;
        }
        RHITextureUpload upload{std::as_bytes(std::span(bytes)), size * 4}; upload.mip_level = mip;
        BOOST_REQUIRE(device.Update_Texture(texture, upload));
    }
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));
    std::array<PropVertex, 4> vertices{};
    vertices[0].position = {-1, -1, 0.5f}; vertices[1].position = {1, -1, 0.5f};
    vertices[2].position = {1, 1, 0.5f}; vertices[3].position = {-1, 1, 0.5f};
    for (auto& vertex : vertices) vertex.uv = {(vertex.position[0] + 1) * 0.5f, 0.5f};
    const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
    const auto mesh = renderer.Create_Mesh(vertices, indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    const auto draw = [&](RHISamplerDescription sampler, float uv_span, float offset) {
        PropParameters parameters;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        parameters.uv_transform[0][0] = uv_span;
        parameters.uv_transform[0][3] = offset;
        PropStyle style; style.depth_test = style.depth_write = false;
        style.samplers[0] = sampler;
        BOOST_REQUIRE(commands.Clear({0,0,0,0}, 1));
        BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, std::array{texture}));
        std::array<std::uint8_t, 8 * 8 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, std::as_writable_bytes(std::span(pixels)), 32));
        return std::array<int, 4>{pixels[0], pixels[1], pixels[2], pixels[3]};
    };
    RHISamplerDescription sampler;
    sampler.Set_Filter(RHISamplerFilter::Point);
    const auto nearest = draw(sampler, 0, 0.125f);
    sampler.magnification = RHISamplerFilter::Linear;
    const auto magnified = draw(sampler, 0, 0.125f);
    BOOST_CHECK_SMALL(magnified[0] - 128, 2); BOOST_CHECK_SMALL(magnified[1] - 128, 2);
    BOOST_CHECK_GE(std::abs(nearest[0] - magnified[0]), 120);
    BOOST_CHECK_SMALL(magnified[3] - 128, 1);
    sampler.Set_Filter(RHISamplerFilter::Point);
    const auto min_nearest = draw(sampler, 2, 0.125f);
    sampler.minification = RHISamplerFilter::Linear;
    const auto min_linear = draw(sampler, 2, 0.125f);
    BOOST_CHECK_SMALL(min_linear[0] - 128, 2); BOOST_CHECK_SMALL(min_linear[1] - 128, 2);
    BOOST_CHECK_GE(std::abs(min_nearest[0] - min_linear[0]), 120);
    BOOST_CHECK_SMALL(min_linear[3] - 64, 1);
    sampler.Set_Filter(RHISamplerFilter::Point);
    sampler.mipmap = RHISamplerFilter::Linear;
    const float span = std::sqrt(8.0f), offset = 0.125f - span / 16;
    const auto mip_blend = draw(sampler, span, offset);
    BOOST_CHECK_SMALL(mip_blend[0] - 128, 3); BOOST_CHECK_SMALL(mip_blend[2] - 128, 3);
    BOOST_CHECK_SMALL(mip_blend[3] - 128, 3);
    sampler.max_lod = 1;
    const auto clamped = draw(sampler, span, offset);
    BOOST_CHECK_EQUAL(clamped[0], 255); BOOST_CHECK_EQUAL(clamped[2], 0); BOOST_CHECK_EQUAL(clamped[3], 64);
    sampler.min_lod = sampler.max_lod = 2;
    for (const auto anisotropy : {1, 2, 4, 8, 16}) {
        sampler.anisotropy = static_cast<std::uint8_t>(anisotropy);
        const auto coarse = draw(sampler, span, offset);
        BOOST_CHECK_EQUAL(coarse[0], 0); BOOST_CHECK_EQUAL(coarse[2], 255); BOOST_CHECK_EQUAL(coarse[3], 192);
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
