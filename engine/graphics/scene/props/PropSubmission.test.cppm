module;
#define BOOST_TEST_MODULE PropSubmissionTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Graphics.Scene.Props.Submission.Tests;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.Constants;
import Graphics.Scene.Props.Instances;
import Graphics.Scene.Props.SkinPalettes;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(decal_groups_materials_and_transparency_preserve_order_and_ownership)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    DirectionalShadowRenderer shadows;
    PropSubmission submission;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    BOOST_REQUIRE(shadows.Initialize(device,shaders));
    submission.Initialize(device,renderer,shadows);
    const auto target = device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.depth_write = false;
    style.depth_test = false;
    style.source_blend = RHIBlendFactor::SourceAlpha;
    style.destination_blend = RHIBlendFactor::InverseSourceAlpha;
    const std::array<std::array<std::uint8_t,4>,5> colors{{
        {255,0,0,128},{255,255,0,128},{0,255,0,128},{0,0,255,128},{255,255,255,128}}};
    std::array<RHITextureHandle,5> textures{};
    for (unsigned index=0;index<colors.size();++index)
        textures[index] = device.Create_Texture_Initialized({1,1},
            {std::as_bytes(std::span(colors[index])),4});
    submission.Begin_Decal_Group();
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,std::array{textures[0]},PropDrawPhase::Decal));
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,std::array{textures[1]},PropDrawPhase::Decal));
    submission.Begin_Decal_Group();
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,std::array{textures[2]},PropDrawPhase::Decal));
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,std::array{textures[3]},PropDrawPhase::Material));
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,std::array{textures[4]},PropDrawPhase::Transparent));
    BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
    BOOST_REQUIRE(submission.Flush_Materials());
    std::array<float,4> expected{};
    const auto blend = [&](unsigned index) {
        const float alpha = colors[index][3]/255.0f;
        for (unsigned channel=0;channel<4;++channel)
            expected[channel] = colors[index][channel]*alpha+expected[channel]*(1-alpha);
    };
    // Last decal object first; authored material order within each object.
    for (unsigned index : {2u,0u,1u,3u}) blend(index);
    std::array<std::byte,8*8*4> pixels{};
    const auto check = [&] {
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32));
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-expected[channel],2.0f);
    };
    check();
    BOOST_REQUIRE(submission.Flush_Transparent());
    blend(4);
    check();
    BOOST_CHECK(renderer.Mesh_Geometry(mesh) == nullptr);
    std::array<std::byte,4> sample{};
    for (const auto texture : textures) BOOST_CHECK(!device.Readback_Texture(texture,sample,4));
    // Cancellation also releases transferred textures and retained geometry.
    const auto cancelled = renderer.Create_Mesh(vertices,indices);
    const auto texture = device.Create_Texture_Initialized({1,1},
        {std::as_bytes(std::span(colors[0])),4});
    BOOST_REQUIRE(submission.Submit(cancelled,style,parameters,std::array{texture},PropDrawPhase::Material));
    BOOST_REQUIRE(renderer.Destroy_Mesh(cancelled));
    submission.Shutdown();
    BOOST_CHECK(renderer.Mesh_Geometry(cancelled) == nullptr);
    BOOST_CHECK(!device.Readback_Texture(texture,sample,4));
    shadows.Shutdown(); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(shared_texture_handles_survive_source_release_and_multiple_deferred_consumers)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    DirectionalShadowRenderer shadows;
    PropSubmission submission;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device, shaders));
    BOOST_REQUIRE(shadows.Initialize(device, shaders));
    submission.Initialize(device, renderer, shadows);
    const auto target = device.Create_Texture({8, 8, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({8, 8, 1, RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));
    BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 1));
    std::array<PropVertex, 4> vertices{};
    vertices[0].position = {-1, -1, 0.5f}; vertices[1].position = {1, -1, 0.5f};
    vertices[2].position = {1, 1, 0.5f}; vertices[3].position = {-1, 1, 0.5f};
    const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
    const auto mesh = renderer.Create_Mesh(vertices, indices);
    PropParameters parameters;
    parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    PropStyle style;
    style.depth_write = style.depth_test = false;
    style.source_blend = RHIBlendFactor::SourceAlpha;
    style.destination_blend = RHIBlendFactor::InverseSourceAlpha;
    const std::array<std::byte, 4> red{std::byte{255}, {}, {}, std::byte{128}};
    const auto texture = device.Create_Texture_Initialized({1, 1}, {red, 4});
    for (auto phase : {PropDrawPhase::Material, PropDrawPhase::Transparent}) {
        BOOST_REQUIRE(device.Retain_Texture(texture));
        BOOST_REQUIRE(submission.Submit(mesh, style, parameters, std::array{texture}, phase));
    }
    BOOST_REQUIRE(device.Destroy_Texture(texture));
    BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
    BOOST_REQUIRE(submission.Flush_Materials());
    std::array<std::byte, 4> sample{};
    BOOST_REQUIRE(device.Readback_Texture(texture, sample, 4));
    BOOST_REQUIRE(submission.Flush_Transparent());
    std::array<std::byte, 8 * 8 * 4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32));
    const auto center = (4 * 8 + 4) * 4;
    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center]) - 192, 2);
    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[center + 2]) - 63, 2);
    BOOST_CHECK(!device.Readback_Texture(texture, sample, 4));
    submission.Shutdown(); shadows.Shutdown(); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(instance_batches_match_individual_draws_across_growth_state_changes_and_cancellation)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer; DirectionalShadowRenderer shadows; PropSubmission submission;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    submission.Initialize(device, renderer, shadows);
    const auto target = device.Create_Texture({32,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,16}));
    std::array<PropVertex,3> vertices{};
    vertices[0].position={-.2f,-.6f,.5f}; vertices[1].position={.2f,-.6f,.5f}; vertices[2].position={0,.6f,.5f};
    const std::array<std::uint32_t,3> indices{0,1,2};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    PropParameters parameters; parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style; style.cull=RHICullMode::None;
    style.source_blend=RHIBlendFactor::One; style.destination_blend=RHIBlendFactor::Zero;
    std::array<std::byte,32*16*4> expected{}, actual{};
    for (const unsigned count : {2u,17u,3u}) {
        std::vector<std::array<float,16>> worlds(count,parameters.world);
        for (unsigned instance=0;instance<count;++instance) {
            worlds[instance][3]=-.6f+1.2f*instance/(count-1);
            worlds[instance][0]=.75f; worlds[instance][5]=.8f;
        }
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        for (const auto& world : worlds) {
            parameters.world=world;
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        }
        BOOST_REQUIRE(device.Readback_Texture(target,expected,32*4));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        const auto before=commands.Submission_Counts();
        submission.Begin_Batching();
        for (const auto& world : worlds) {
            parameters.world=world;
            BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Batchable));
        }
        BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls,before.draw_calls);
        BOOST_REQUIRE(submission.End_Batching());
        BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls-before.draw_calls,1u);
        BOOST_CHECK_EQUAL(commands.Submission_Counts().triangles-before.triangles,count);
        BOOST_REQUIRE(device.Readback_Texture(target,actual,32*4));
        BOOST_CHECK(actual==expected);
    }
    // A shared-state change splits the run; an immediate draw is a barrier.
    auto before=commands.Submission_Counts();
    submission.Begin_Batching();
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Batchable));
    parameters.opacity=.5f;
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Batchable));
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Immediate));
    BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls-before.draw_calls,3u);
    BOOST_REQUIRE(submission.End_Batching());
    submission.Begin_Batching();
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Batchable));
    BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
    submission.Clear();
    BOOST_CHECK(renderer.Mesh_Geometry(mesh)==nullptr);
    BOOST_REQUIRE(submission.End_Batching());
    submission.Shutdown(); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(deferred_alpha_and_additive_instances_preserve_primitive_order)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer; DirectionalShadowRenderer shadows; PropSubmission submission;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    submission.Initialize(device,renderer,shadows);
    const auto target = device.Create_Texture({32,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::uint8_t,8> texels{255,0,0,128, 0,255,0,128};
    const auto texture = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(texels)),8});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-.6f,-.6f,.5f}; vertices[1].position={.6f,-.6f,.5f};
    vertices[2].position={.6f,.6f,.5f}; vertices[3].position={-.6f,.6f,.5f};
    vertices[0].uv={0,0}; vertices[1].uv={1,0}; vertices[2].uv={1,1}; vertices[3].uv={0,1};
    const auto mesh = renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style; style.depth_write=false;
    style.source_blend=RHIBlendFactor::SourceAlpha;
    style.samplers[0].Set_Filter(RHISamplerFilter::Point);
    style.samplers[0].address.fill(RHISamplerAddress::Clamp);
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,16}));
    for (const bool additive : {false,true}) {
        style.destination_blend = additive ? RHIBlendFactor::One : RHIBlendFactor::InverseSourceAlpha;
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        for (const float x : {-.5f,0.f,.5f}) {
            parameters.world[3]=x;
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
        }
        std::array<std::byte,32*16*4> expected{},actual{};
        BOOST_REQUIRE(device.Readback_Texture(target,expected,32*4));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        const auto before = commands.Submission_Counts();
        for (const float x : {-.5f,0.f,.5f}) {
            parameters.world[3]=x;
            BOOST_REQUIRE(device.Retain_Texture(texture));
            BOOST_REQUIRE(submission.Submit(mesh,style,parameters,std::array{texture},PropDrawPhase::Material));
        }
        BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls,before.draw_calls);
        BOOST_REQUIRE(submission.Flush_Materials());
        BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls-before.draw_calls,1u);
        BOOST_REQUIRE(device.Readback_Texture(target,actual,32*4));
        BOOST_CHECK(actual == expected);
        // Green, green, then red cover the center. Alpha blending makes
        // reversing the instance order observable in the red/green channels.
        const std::array<int,4> center = additive ? std::array<int,4>{128,255,0,192}
            : std::array<int,4>{128,96,0,112};
        for (unsigned channel=0; channel<4; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(actual[(8*32+16)*4+channel])-center[channel],2);
        BOOST_CHECK_EQUAL(std::to_integer<int>(actual[(1*32+1)*4+3]),0);
        submission.Clear();
    }
    renderer.Destroy_Mesh(mesh); submission.Shutdown(); renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

namespace {
class ConstantUploadDevice final : public Device {
public:
    GraphicsTestDevice native{ {true} };
    std::vector<std::size_t> uploads;
    bool reject=false;
    bool Is_Valid() const noexcept override { return native.Is_Valid(); }
    RHIBufferHandle Create_Buffer(const RHIBuffer& value) override { return native.Create_Buffer(value); }
    RHIBufferHandle Create_Buffer_Initialized(const RHIBuffer& value, std::span<const std::byte> bytes) override {
        uploads.push_back(bytes.size());
        return reject ? RHIBufferHandle{} : native.Create_Buffer_Initialized(value,bytes);
    }
    RHITextureHandle Create_Texture(const RHITexture& value) override { return native.Create_Texture(value); }
    RHIPipelineHandle Create_Pipeline(const RHIPipeline& value) override { return native.Create_Pipeline(value); }
    bool Destroy_Buffer(RHIBufferHandle value) noexcept override { return native.Destroy_Buffer(value); }
    bool Destroy_Texture(RHITextureHandle value) noexcept override { return native.Destroy_Texture(value); }
    bool Destroy_Pipeline(RHIPipelineHandle value) noexcept override { return native.Destroy_Pipeline(value); }
    CommandList& Immediate_Command_List() noexcept override { return native.Immediate_Command_List(); }
    SwapChain& Get_Swap_Chain() noexcept override { return native.Get_Swap_Chain(); }
    bool Begin_Frame() noexcept override { return native.Begin_Frame(); }
    bool End_Frame() noexcept override { return native.End_Frame(); }
    bool Update_Buffer(RHIBufferHandle buffer, std::uint32_t offset, std::span<const std::byte> bytes) noexcept override {
        uploads.push_back(bytes.size());
        return !reject && native.Update_Buffer(buffer,offset,bytes);
    }
};
}
BOOST_AUTO_TEST_CASE(instance_streams_skip_unchanged_uploads_and_retry_rejected_replacements)
{
    ConstantUploadDevice device;
    PropInstanceBinding first, second;
    const PropParameters parameters;
    std::vector<std::array<float,16>> worlds(2,parameters.world);
    worlds[1][3] = 4;
    BOOST_REQUIRE(first.Prepare(device,worlds));
    worlds[0][3] = -2;
    BOOST_REQUIRE(second.Prepare(device,worlds));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{128,128}));
    device.uploads.clear();
    BOOST_REQUIRE(second.Prepare(device,worlds));
    worlds[0][3] = 0;
    BOOST_REQUIRE(first.Prepare(device,worlds));
    BOOST_CHECK(device.uploads.empty());
    const auto original = first.Buffer();
    worlds[1][3] = 8;
    device.reject = true;
    BOOST_CHECK(!first.Prepare(device,worlds));
    BOOST_CHECK(first.Buffer() == original);
    device.reject = false;
    BOOST_REQUIRE(first.Prepare(device,worlds));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{128,128}));
    device.uploads.clear();
    worlds.resize(17,parameters.world);
    device.reject = true;
    BOOST_CHECK(!first.Prepare(device,worlds));
    BOOST_CHECK(first.Buffer() == original);
    device.reject = false;
    BOOST_REQUIRE(first.Prepare(device,worlds));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{1088,1088}));
    device.uploads.clear();
    BOOST_REQUIRE(first.Prepare(device,worlds));
    BOOST_CHECK(device.uploads.empty());
    worlds.resize(3);
    BOOST_REQUIRE(first.Prepare(device,worlds));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{192}));
    first.Shutdown(device); second.Shutdown(device);
    device.uploads.clear();
    BOOST_REQUIRE(first.Prepare(device,worlds));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{192}));
    first.Shutdown(device);
}

BOOST_AUTO_TEST_CASE(constant_updates_follow_frequency_and_retry_failed_uploads)
{
    ConstantUploadDevice device;
    PropConstantBindings constants;
    PropMaterialBinding material;
    BOOST_REQUIRE(constants.Initialize(device));
    PropParameters parameters;
    std::array<RHIBindlessResource,4> resources{};
    const auto prepare=[&] { return constants.Prepare_Resources(device,parameters,material,resources); };
    BOOST_REQUIRE(prepare());
    BOOST_CHECK((device.uploads==std::vector<std::size_t>{192,464,288,64}));
    device.uploads.clear(); BOOST_REQUIRE(prepare()); BOOST_CHECK(device.uploads.empty());
    parameters.world[3]=2; BOOST_REQUIRE(prepare());
    BOOST_CHECK((device.uploads==std::vector<std::size_t>{64}));
    device.uploads.clear(); parameters.fog_color[0]=.5f; BOOST_REQUIRE(prepare());
    BOOST_CHECK((device.uploads==std::vector<std::size_t>{192}));
    device.uploads.clear(); parameters.light_direction[0][0]=.5f; BOOST_REQUIRE(prepare());
    BOOST_CHECK((device.uploads==std::vector<std::size_t>{464}));
    device.uploads.clear(); parameters.opacity=.5f; BOOST_REQUIRE(prepare());
    BOOST_CHECK((device.uploads==std::vector<std::size_t>{288}));
    device.uploads.clear(); parameters.world[3]=3; device.reject=true;
    BOOST_CHECK(!prepare()); device.reject=false; BOOST_REQUIRE(prepare());
    BOOST_CHECK((device.uploads==std::vector<std::size_t>{64,64}));
    constants.Shutdown(device); material.Shutdown(device);
    BOOST_REQUIRE(constants.Initialize(device));
    device.uploads.clear(); BOOST_REQUIRE(prepare());
    BOOST_CHECK((device.uploads==std::vector<std::size_t>{192,464,288,64}));
    constants.Shutdown(device); material.Shutdown(device);
}

BOOST_AUTO_TEST_CASE(material_uploads_persist_across_other_meshes_and_retry_changes)
{
    ConstantUploadDevice device;
    PropConstantBindings constants;
    PropMaterialBinding first, second;
    BOOST_REQUIRE(constants.Initialize(device));
    PropParameters first_parameters, second_parameters;
    second_parameters.opacity = .5f;
    std::array<RHIBindlessResource,4> resources{};
    const auto prepare = [&](PropMaterialBinding& material, const PropParameters& parameters) {
        return constants.Prepare_Resources(device, parameters, material, resources);
    };
    BOOST_REQUIRE(prepare(first, first_parameters));
    const auto first_buffer = resources[2].buffer;
    device.uploads.clear();
    BOOST_REQUIRE(prepare(second, second_parameters));
    const auto second_buffer = resources[2].buffer;
    BOOST_CHECK(first_buffer != second_buffer);
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{288}));
    device.uploads.clear();
    for (unsigned frame = 0; frame < 3; ++frame) {
        BOOST_REQUIRE(prepare(first, first_parameters));
        BOOST_CHECK(resources[2].buffer == first_buffer);
        BOOST_REQUIRE(prepare(second, second_parameters));
        BOOST_CHECK(resources[2].buffer == second_buffer);
    }
    BOOST_CHECK(device.uploads.empty());

    first_parameters.uv_transform[1][3] = .25f;
    BOOST_REQUIRE(prepare(first, first_parameters));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{288}));
    device.uploads.clear();
    first_parameters.alpha_cutoff = .125f;
    device.reject = true;
    BOOST_CHECK(!prepare(first, first_parameters));
    device.reject = false;
    BOOST_REQUIRE(prepare(second, second_parameters));
    BOOST_REQUIRE(prepare(first, first_parameters));
    BOOST_REQUIRE(prepare(first, first_parameters));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{288,288}));

    // Rebuilding shared view bindings does not invalidate retained materials.
    constants.Shutdown(device);
    BOOST_REQUIRE(constants.Initialize(device));
    device.uploads.clear();
    BOOST_REQUIRE(prepare(first, first_parameters));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{192,464,64}));
    first.Shutdown(device);
    device.uploads.clear();
    BOOST_REQUIRE(prepare(first, first_parameters));
    BOOST_CHECK((device.uploads == std::vector<std::size_t>{288}));
    BOOST_CHECK(resources[2].buffer != first_buffer);
    constants.Shutdown(device); first.Shutdown(device); second.Shutdown(device);
}

BOOST_AUTO_TEST_CASE(retained_instances_batch_distinct_lighting_and_keep_queued_snapshots)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer; DirectionalShadowRenderer shadows; PropSubmission submission;
    BOOST_REQUIRE(renderer.Initialize(device,Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    submission.Initialize(device,renderer,shadows);
    const auto target = device.Create_Texture({32,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,16}));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-.3f,-.5f,.5f}; vertices[1].position={.3f,-.5f,.5f};
    vertices[2].position={.3f,.5f,.5f}; vertices[3].position={-.3f,.5f,.5f};
    for (auto& vertex : vertices) {
        vertex.material_ambient={1,1,1,1}; vertex.material_diffuse={1,1,1,1};
        vertex.material_emissive={0,0,0,0}; vertex.normal={0,0,1};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    PropStyle style; style.depth_test=false; style.depth_write=false;
    PropParameters parameters; parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropInstanceOwner left,right;
    parameters.world[3]=-.5f; parameters.scene_ambient={1,0,0,0};
    const auto red = left.Update(renderer.Instances(),parameters);
    // Material queues preserve order and also instance without depth writes.
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Material,{},red));
    parameters.world[3]=.5f; parameters.scene_ambient={0,1,0,0};
    const auto green = right.Update(renderer.Instances(),parameters);
    BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Material,{},green));
    parameters.world[3]=-.5f; parameters.scene_ambient={0,0,1,0};
    const auto blue = left.Update(renderer.Instances(),parameters);
    BOOST_CHECK(blue != red);
    const auto before = commands.Submission_Counts();
    BOOST_REQUIRE(submission.Flush_Materials());
    BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls-before.draw_calls,1u);
    std::array<std::byte,32*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,128));
    for (const auto y : {5u,10u}) {
        for (const auto x : {6u,9u}) {
            const auto offset=(y*32+x)*4;
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+2]),0u);
        }
        for (const auto x : {22u,25u}) {
            const auto offset=(y*32+x)*4;
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+2]),0u);
        }
    }
    BOOST_CHECK(renderer.Instances().Resolve(red)==nullptr);
    const auto uploaded=renderer.Instances().Uploaded_Bytes();
    BOOST_CHECK(left.Update(renderer.Instances(),parameters)==blue);
    BOOST_REQUIRE(renderer.Instances().Prepare(device));
    BOOST_CHECK_EQUAL(renderer.Instances().Uploaded_Bytes(),uploaded);
    parameters.scene_ambient={.25f,0,1,0};
    BOOST_CHECK(left.Update(renderer.Instances(),parameters)==blue);
    BOOST_REQUIRE(renderer.Instances().Prepare(device));
    BOOST_CHECK_EQUAL(renderer.Instances().Uploaded_Bytes()-uploaded,544u);
    renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device,Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    BOOST_REQUIRE(renderer.Instances().Prepare(device));
    BOOST_CHECK(renderer.Instances().Resolve(blue)!=nullptr);
    submission.Shutdown(); renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(gpu_skinning_matches_deformed_positions_normals_and_uvs_without_geometry_uploads)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    const auto shaders=Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const auto target=device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-.6f,-.5f,.5f}; vertices[1].position={.6f,-.5f,.5f};
    vertices[2].position={.6f,.5f,.5f}; vertices[3].position={-.6f,.5f,.5f};
    vertices[0].uv={0,1}; vertices[1].uv={1,1}; vertices[2].uv={1,0}; vertices[3].uv={0,0};
    for (unsigned index=0; index<vertices.size(); ++index) {
        auto& vertex=vertices[index];
        vertex.normal={.4f,.3f,.8f}; vertex.material_ambient={1,1,1,1};
        vertex.material_specular={.1f,.15f,.2f,4};
        vertex.bone_index=index==1 || index==2 ? 1.f : 0.f;
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    const std::array<std::uint8_t,16> texels{255,128,64,255,64,255,128,128,128,64,255,255,255,255,255,128};
    const auto texture=device.Create_Texture_Initialized({2,2},{std::as_bytes(std::span(texels)),8});
    const std::array textures{texture};
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.scene_ambient={.1f,.05f,.1f,0}; parameters.camera_position={0,0,3,1};
    parameters.light_direction[0]={.4f,.2f,1,1}; parameters.light_diffuse[0]={.6f,.7f,.8f,0};
    parameters.light_specular[0]={.2f,.2f,.2f,0};
    PropStyle style;
    PropSkinOwner skin; PropInstanceOwner instance;
    std::array<PropBoneTransform,2> pose{{
        {1,0,0,-.15f,0,1.2f,0,0,0,0,.7f,0},
        {.7f,-.2f,0,.15f,.15f,.9f,0,.1f,0,0,1.1f,-.05f}}};
    for (unsigned frame=0; frame<3; ++frame) {
        pose[1][3]=.15f-.1f*frame;
        auto deformed=vertices;
        for (auto& vertex : deformed) {
            const auto position=vertex.position, normal=vertex.normal;
            const auto& bone=pose[static_cast<unsigned>(vertex.bone_index)];
            for (unsigned axis=0; axis<3; ++axis) {
                const auto row=axis*4;
                vertex.position[axis]=bone[row]*position[0]+bone[row+1]*position[1]+bone[row+2]*position[2]+bone[row+3];
                vertex.normal[axis]=bone[row]*normal[0]+bone[row+1]*normal[1]+bone[row+2]*normal[2];
            }
        }
        const auto reference=renderer.Create_Mesh(deformed,indices);
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,reference,style,parameters,textures));
        std::array<std::byte,32*32*4> expected{},actual{};
        BOOST_REQUIRE(device.Readback_Texture(target,expected,128));
        renderer.Destroy_Mesh(reference);
        const auto before=renderer.Geometry_Uploaded_Bytes();
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        const auto palette=skin.Update(renderer.Instances().Palettes(),pose.size(),
            [&](std::size_t bone) -> const auto& { return pose[bone]; });
        const auto record=instance.Update(renderer.Instances(),parameters,palette);
        BOOST_REQUIRE(renderer.Draw_Record(commands,mesh,style,parameters,textures,record));
        if (frame!=0) BOOST_CHECK_EQUAL(renderer.Geometry_Uploaded_Bytes(),before);
        else BOOST_CHECK_EQUAL(renderer.Geometry_Uploaded_Bytes()-before,4u*sizeof(PropVertex)+24u);
        BOOST_REQUIRE(device.Readback_Texture(target,actual,128));
        for (unsigned index=0; index<actual.size(); ++index)
            BOOST_CHECK_SMALL(std::to_integer<int>(actual[index])-std::to_integer<int>(expected[index]),2);
        // Both authored triangles contribute, and the exterior stays clear.
        BOOST_CHECK(std::to_integer<unsigned>(actual[(20*32+10)*4+3])>0u);
        BOOST_CHECK(std::to_integer<unsigned>(actual[(10*32+10)*4+3])>0u);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(actual[(1*32+1)*4+3]),0u);
        std::array<float,3> minimum{},maximum{};
        BOOST_REQUIRE(renderer.Mesh_Bounds(mesh,record,minimum,maximum));
        for (const auto& vertex : deformed) for (unsigned axis=0; axis<3; ++axis) {
            BOOST_CHECK(vertex.position[axis]>=minimum[axis]-.00001f);
            BOOST_CHECK(vertex.position[axis]<=maximum[axis]+.00001f);
        }
        const auto uploaded=renderer.Instances().Palettes().Uploaded_Bytes();
        BOOST_CHECK(skin.Update(renderer.Instances().Palettes(),pose.size(),
            [&](std::size_t bone) -> const auto& { return pose[bone]; })==palette);
        BOOST_REQUIRE(renderer.Draw_Record(commands,mesh,style,parameters,textures,record));
        BOOST_CHECK_EQUAL(renderer.Instances().Palettes().Uploaded_Bytes(),uploaded);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(skinned_transparency_sorts_the_retained_pose_after_owner_release_and_recreation)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer; DirectionalShadowRenderer shadows; PropSubmission submission;
    const auto shaders=Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders)); submission.Initialize(device,renderer,shadows);
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth)); BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-.75f,-.75f,.5f}; vertices[1].position={.75f,-.75f,.5f};
    vertices[2].position={.75f,.75f,.5f}; vertices[3].position={-.75f,.75f,.5f};
    for (auto& vertex : vertices) vertex.material_ambient={1,1,1,1};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    PropStyle style; style.depth_test=false; style.depth_write=false;
    style.source_blend=RHIBlendFactor::SourceAlpha; style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
    PropParameters parameters; parameters.textured=0; parameters.opacity=.5f;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    for (unsigned index=0; index<2; ++index) {
        PropSkinOwner skin; PropInstanceOwner instance;
        PropBoneTransform pose{1,0,0,0,0,1,0,0,0,0,1,index==0 ? -.3f : .3f};
        const auto palette=skin.Update(renderer.Instances().Palettes(),1,[&](std::size_t) -> const auto& { return pose; });
        parameters.scene_ambient=index==0 ? std::array<float,4>{1,0,0,0} : std::array<float,4>{0,0,1,0};
        const auto record=instance.Update(renderer.Instances(),parameters,palette);
        BOOST_REQUIRE(submission.Submit(mesh,style,parameters,{},PropDrawPhase::Transparent,{0,0,-1,0},record));
        pose[11]=0;
        skin.Update(renderer.Instances().Palettes(),1,[&](std::size_t) -> const auto& { return pose; });
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown(); BOOST_REQUIRE(renderer.Initialize(device,shaders));
    BOOST_REQUIRE(submission.Flush_Transparent());
    std::array<std::byte,16*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,64));
    for (const auto point : {std::array{5u,10u},std::array{10u,5u}}) {
        const auto offset=(point[1]*16+point[0])*4;
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset])-128,1);
        BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[offset+1]),0);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+2])-64,1);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+3])-96,1);
    }
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[0]),0);
    BOOST_CHECK(renderer.Mesh_Geometry(mesh)==nullptr);
    submission.Shutdown(); renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(nonadjacent_opaque_batches_merge_only_with_disjoint_raster_bounds)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer; DirectionalShadowRenderer shadows; PropSubmission submission;
    BOOST_REQUIRE(renderer.Initialize(device,Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    submission.Initialize(device,renderer,shadows);
    const auto target=device.Create_Texture({64,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({64,32,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    const RHIViewport viewport{0,0,64,32};
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport(viewport));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-.18f,-.65f,.5f}; vertices[1].position={.18f,-.65f,.5f};
    vertices[2].position={.18f,.65f,.5f}; vertices[3].position={-.18f,.65f,.5f};
    for (auto& vertex : vertices) vertex.material_ambient={1,1,1,1};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto a=renderer.Create_Mesh(vertices,indices),b=renderer.Create_Mesh(vertices,indices);
    const std::array meshes{a,b,a};
    PropStyle style;
    std::array<PropParameters,3> parameters;
    for (unsigned draw=0; draw<parameters.size(); ++draw) {
        parameters[draw].textured=0;
        parameters[draw].view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        parameters[draw].scene_ambient[draw]=1;
    }
    // Disjoint A/B/A becomes two draws. Equal-depth overlap must keep all three
    // in order, and a missing viewport disables the nonadjacent search.
    for (unsigned scenario=0; scenario<3; ++scenario) {
        for (unsigned draw=0; draw<parameters.size(); ++draw)
            parameters[draw].world[3]=scenario==1 ? 0.f : (static_cast<float>(draw)-1)*.55f;
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        for (unsigned draw=0; draw<parameters.size(); ++draw)
            BOOST_REQUIRE(renderer.Draw(commands,meshes[draw],style,parameters[draw],{}));
        std::array<std::byte,64*32*4> expected{},actual{};
        BOOST_REQUIRE(device.Readback_Texture(target,expected,256));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        const auto before=commands.Submission_Counts();
        submission.Begin_Batching(scenario==2 ? RHIViewport{} : viewport);
        for (unsigned draw=0; draw<parameters.size(); ++draw)
            BOOST_REQUIRE(submission.Submit(meshes[draw],style,parameters[draw],{},PropDrawPhase::Batchable));
        BOOST_REQUIRE(submission.End_Batching());
        BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls-before.draw_calls,scenario==0 ? 2u : 3u);
        BOOST_REQUIRE(device.Readback_Texture(target,actual,256));
        BOOST_CHECK(actual==expected);
        for (const auto y : {10u,21u}) {
            if (scenario==1) {
                const auto offset=(y*64+32)*4;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(actual[offset]),0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(actual[offset+2]),255u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(actual[offset+3]),255u);
            } else for (unsigned draw=0; draw<3; ++draw) {
                const unsigned x=draw==0 ? 14u : draw==1 ? 32u : 49u;
                const auto offset=(y*64+x)*4;
                for (unsigned channel=0; channel<3; ++channel)
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(actual[offset+channel]),channel==draw ? 255u : 0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(actual[offset+3]),255u);
            }
        }
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(actual[3]),0u);
    }
    submission.Shutdown(); renderer.Destroy_Mesh(a); renderer.Destroy_Mesh(b); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
