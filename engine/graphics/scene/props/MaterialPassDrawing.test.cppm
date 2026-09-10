module;
#define BOOST_TEST_MODULE MaterialPassDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
export module Graphics.Scene.Props.MaterialPassDrawing.Tests;
import Graphics.Materials.ProceduralPass;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.State;
import Graphics.Scene.Props.MaterialPassQueue;
import Graphics.Tests.Device;
using namespace Graphics;

namespace
{

struct CallbackTexture final
{
    RHITextureHandle handle{};
};

struct CallbackState final
{
    CallbackTexture* texture = nullptr;
    MeshMaterial* material = nullptr;
    std::array<float, 16> transform{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};
};

using CallbackPass = ProceduralMaterialPass<std::shared_ptr<CallbackTexture>, CallbackState>;

bool Prepare_Callback_Pass(const CallbackPass& pass, CallbackPass::Description& description)
{
    const auto* state = pass.cull_bounds;
    if (state == nullptr || state->texture == nullptr || !state->texture->handle.Is_Valid()) return false;
    description.shader = MaterialState::Opaque();
    description.material = state->material;
    description.textures[0] = state->texture;
    description.textures[1] = std::to_address(pass.textures[1]);
    description.world_coordinates = true;
    description.world_texture_transform = state->transform;
    return true;
}

}

BOOST_AUTO_TEST_CASE(deferred_pass_observes_opaque_depth_and_keeps_submission_order)
{
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.6f}; vertices[1].position={1,-1,0.6f};
    vertices[2].position={1,1,0.6f}; vertices[3].position={-1,1,0.6f};
    for (auto& vertex : vertices) vertex.color={0,1,0,1};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}; parameters.textured=0;
    PropStyle style; style.depth_write=false;
    MaterialPassQueue queue;
    BOOST_REQUIRE(queue.Submit(renderer,vertices,indices,style,parameters,{}));
    // Later opaque geometry occludes the left half. Submission copied the
    // green vertices, so this mutation must not recolor the deferred pass.
    for (auto& vertex : vertices) {
        vertex.position[0]=(vertex.position[0]-1)*0.5f;
        vertex.position[2]=0.4f; vertex.color={1,0,0,1};
    }
    const auto occluder=renderer.Create_Mesh(vertices,indices);
    style.depth_write=true;
    BOOST_REQUIRE(renderer.Draw(commands,occluder,style,parameters,{}));
    BOOST_REQUIRE(queue.Flush(commands)); BOOST_CHECK(queue.Empty());
    const auto check=[&](unsigned x,std::array<int,4> expected) {
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for (unsigned c=0;c<4;++c)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+x)*4+c])-expected[c],2);
    };
    check(3,{255,0,0,255}); check(12,{0,255,0,255});
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    style.depth_test=false; style.depth_write=false;
    style.source_blend=RHIBlendFactor::SourceAlpha;
    style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
    for (auto& vertex : vertices) vertex.color={1,0,0,0.5f};
    BOOST_REQUIRE(queue.Submit(renderer,vertices,indices,style,parameters,{}));
    for (auto& vertex : vertices) vertex.color={0,1,0,0.5f};
    BOOST_REQUIRE(queue.Submit(renderer,vertices,indices,style,parameters,{}));
    BOOST_REQUIRE(queue.Flush(commands)); check(3,{64,128,0,96});
    BOOST_REQUIRE(queue.Submit(renderer,vertices,indices,style,parameters,{}));
    queue.Clear(); BOOST_CHECK(queue.Empty());
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(queue.Flush(commands)); check(3,{0,0,255,255});
    // A projected transition mask writes sampled alpha without replacing RGB.
    // A following ordinary pass must restore its own channel mask.
    const std::array<std::uint8_t,8> mask_pixels{255,0,0,64,0,255,0,192};
    const auto mask=device.Create_Texture_Initialized({2,1},
        {std::as_bytes(std::span(mask_pixels)),8});
    BOOST_REQUIRE(mask.Is_Valid());
    for (auto& vertex : vertices) vertex.uv={vertex.position[0]+1,0.5f};
    parameters.textured=1; parameters.primary_gradient=0;
    style=PropStyle{}; style.color_write_mask=8;
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    const std::array mask_textures{mask};
    BOOST_REQUIRE(queue.Submit(renderer,vertices,indices,style,parameters,mask_textures));
    BOOST_REQUIRE(queue.Flush(commands));
    check(3,{0,0,255,64}); check(6,{0,0,255,192}); check(12,{0,0,255,255});
    parameters.textured=0; parameters.primary_gradient=1;
    style.color_write_mask=15;
    BOOST_REQUIRE(queue.Submit(renderer,vertices,indices,style,parameters,{}));
    BOOST_REQUIRE(queue.Flush(commands)); check(3,{0,255,0,128});
    device.Destroy_Texture(mask);
    renderer.Destroy_Mesh(occluder); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(description_callback_drives_texture_and_transform_after_renderer_recreation)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device, shaders));
    const auto target = device.Create_Texture({8, 8, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({8, 8, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(target.Is_Valid());
    BOOST_REQUIRE(depth.Is_Valid());
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));

    // Each source has two point-sampled texels. The callback selects both the
    // source and the world-space coordinate at draw extraction time.
    const std::array<std::uint8_t, 8> source_a_pixels{255, 0, 0, 255, 0, 255, 0, 255};
    const std::array<std::uint8_t, 8> source_b_pixels{
        128, 255, 0, 128,
        0, 0, 255, 64};
    auto source_a = std::make_shared<CallbackTexture>();
    auto source_b = std::make_shared<CallbackTexture>();
    source_a->handle = device.Create_Texture_Initialized({2, 1},
        {std::as_bytes(std::span(source_a_pixels)), 8});
    source_b->handle = device.Create_Texture_Initialized({2, 1},
        {std::as_bytes(std::span(source_b_pixels)), 8});
    BOOST_REQUIRE(source_a->handle.Is_Valid());
    BOOST_REQUIRE(source_b->handle.Is_Valid());
    const auto source_a_handle = source_a->handle;
    const auto source_b_handle = source_b->handle;
    std::weak_ptr<CallbackTexture> weak_a = source_a;
    std::weak_ptr<CallbackTexture> weak_b = source_b;

    std::array<PropVertex, 4> vertices{};
    vertices[0].position = {-1, -1, 0.5f};
    vertices[1].position = {1, -1, 0.5f};
    vertices[2].position = {1, 1, 0.5f};
    vertices[3].position = {-1, 1, 0.5f};
    for (auto& vertex : vertices) {
        vertex.color = {1, 1, 1, 1};
        vertex.secondary_uv = {0, 0};
    }
    const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
    const auto mesh = renderer.Create_Mesh(vertices, indices);
    BOOST_REQUIRE(mesh.Is_Valid());

    CallbackState state;
    state.texture = source_a.get();
    state.transform[0] = 0;
    state.transform[3] = 0.25f;
    state.transform[5] = 0;
    state.transform[7] = 0.5f;
    CallbackPass pass;
    pass.prepare = &Prepare_Callback_Pass;
    pass.material = std::make_shared<MeshMaterial>();
    pass.material->parameters.opacity = 0.5f;
    state.material = pass.material.get();
    pass.textures[0] = source_a;
    pass.textures[1] = source_b;
    pass.cull_bounds = &state;
    source_a.reset();
    source_b.reset();
    BOOST_CHECK(!weak_a.expired());
    BOOST_CHECK(!weak_b.expired());
    std::weak_ptr<MeshMaterial> weak_material = pass.material;

    PropParameters parameters;
    parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    parameters.textured = 1;
    parameters.primary_gradient = 0;
    parameters.uv_sources[0] = 4;
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.depth_test = false;
    style.depth_write = false;
    style.cull = RHICullMode::None;
    style.samplers[0].Set_Filter(RHISamplerFilter::Point);
    style.samplers[1].Set_Filter(RHISamplerFilter::Point);

    const auto draw_description = [&](const CallbackPass::Description& description, bool use_detail) {
        if (description.textures[0] == nullptr || !description.world_coordinates) return false;
        auto draw_parameters = parameters;
        draw_parameters.opacity = description.material == nullptr ? 1.0f : description.material->parameters.opacity;
        draw_parameters.secondary_texture = use_detail ? 1.0f : 0.0f;
        draw_parameters.detail_color = use_detail ? 2.0f : 0.0f;
        draw_parameters.detail_alpha = use_detail ? 2.0f : 0.0f;
        draw_parameters.uv_sources[2] = 0;
        draw_parameters.uv_sources[3] = 0;
        draw_parameters.uv_transform[0] = description.world_texture_transform;
        if (use_detail && description.textures[1] == nullptr) return false;
        const std::array<RHITextureHandle, 2> textures{
            description.textures[0]->handle,
            description.textures[1] == nullptr ? RHITextureHandle{} : description.textures[1]->handle};
        return renderer.Draw(commands, mesh, style, draw_parameters, textures);
    };
    const auto check_quad_interiors = [&](const std::array<int, 4>& expected) {
        std::array<std::byte, 8 * 8 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 8 * 4));
        for (const auto [x, y] : std::array<std::array<unsigned, 2>, 2>{{{2, 4}, {5, 4}}}) {
            const std::size_t offset = (y * 8 + x) * 4;
            for (unsigned channel = 0; channel < expected.size(); ++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset + channel]) - expected[channel], 2);
        }
    };

    CallbackPass::Description description;
    BOOST_REQUIRE(pass.Describe(description));
    BOOST_REQUIRE(description.material == weak_material.lock().get());
    BOOST_REQUIRE(description.textures[0] == weak_a.lock().get());
    BOOST_REQUIRE(description.textures[1] == weak_b.lock().get());
    BOOST_CHECK_SMALL(description.world_texture_transform[3] - 0.25f, 0.00001f);
    BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
    BOOST_REQUIRE(draw_description(description, false));
    check_quad_interiors({255, 0, 0, 128});

    // The second stage is submitted from the same native description. Its
    // yellow texel preserves the base red while proving both texture slots
    // reach the GPU, including the material opacity.
    BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
    BOOST_REQUIRE(draw_description(description, true));
    check_quad_interiors({128, 0, 0, 64});

    // A second extraction reads the changed transform while the same pass and
    // owned texture remain alive.
    state.transform[3] = 0.75f;
    BOOST_REQUIRE(pass.Describe(description));
    BOOST_CHECK_SMALL(description.world_texture_transform[3] - 0.75f, 0.00001f);
    BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
    BOOST_REQUIRE(draw_description(description, false));
    check_quad_interiors({0, 255, 0, 128});

    // The callback now selects the second owned source. Recreating the
    // renderer must retain the CPU mesh and make the callback-selected draw
    // work with freshly created GPU state.
    state.texture = weak_b.lock().get();
    state.transform[3] = 0.25f;
    renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device, shaders));
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));
    BOOST_REQUIRE(pass.Describe(description));
    BOOST_REQUIRE(description.textures[0] == weak_b.lock().get());
    BOOST_CHECK_SMALL(description.world_texture_transform[3] - 0.25f, 0.00001f);
    BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));
    BOOST_REQUIRE(draw_description(description, false));
    check_quad_interiors({128, 255, 0, 64});

    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    pass.textures = {};
    pass.material.reset();
    BOOST_CHECK(weak_a.expired());
    BOOST_CHECK(weak_b.expired());
    BOOST_CHECK(weak_material.expired());
    device.Destroy_Texture(source_a_handle);
    device.Destroy_Texture(source_b_handle);
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}
