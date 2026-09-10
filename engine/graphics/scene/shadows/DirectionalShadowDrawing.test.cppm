module;
#define BOOST_TEST_MODULE DirectionalShadowDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Graphics.Scene.Shadows.DirectionalDrawing.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Instances;
import Graphics.Scene.Props.SkinPalettes;
import Graphics.Scene.Lighting.Environment;
import Graphics.Scene.Trees.Renderer;
import Graphics.Scene.Terrain.Renderer;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(cascades_draw_off_slice_casters_and_clear_after_resource_resize)
{
    struct ResetEnvironment final {
        ~ResetEnvironment() { Get_Environment_Lighting() = {}; }
    } reset;
    Get_Environment_Lighting() = {};
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    DirectionalShadowRenderer shadows;
    TerrainRenderer terrain;
    PropRenderer receiver;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(shadows.Initialize(device,shaders));
    BOOST_REQUIRE(receiver.Initialize(device,shaders));
    const auto target = device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    const std::array<std::uint8_t,8> cutout_pixels{255,255,255,255,255,255,255,0};
    const auto cutout = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(cutout_pixels)),8});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,-4}; vertices[1].position = {1,-1,-4};
    vertices[2].position = {1,1,-4}; vertices[3].position = {-1,1,-4};
    vertices[0].uv = {0,1}; vertices[1].uv = {1,1};
    vertices[2].uv = {1,0}; vertices[3].uv = {0,0};
    for (auto& vertex : vertices) {
        vertex.normal = {0,0,1};
        vertex.material_ambient = {1,1,1,1};
        vertex.material_diffuse = {1,1,1,1};
        vertex.material_emissive = {0.1f,0.1f,0.1f,0};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    PropParameters parameters;
    parameters.textured = 0;
    parameters.scene_ambient = {0.2f,0.2f,0.2f,0};
    parameters.camera_position = {0,0,0,1};
    parameters.light_direction[0] = {0,0,1,1};
    parameters.light_diffuse[0] = {0.6f,0.6f,0.6f,0};
    parameters.textured = 1;
    parameters.alpha_cutoff = 0.5f;
    const std::array caster_textures{cutout};
    BOOST_REQUIRE(shadows.Add_Caster(vertices,indices,parameters,caster_textures));
    parameters.textured = parameters.alpha_cutoff = 0;
    for (auto& vertex : vertices) vertex.position[2] = -5;
    vertices[1].position[0] = vertices[2].position[0] = 1;
    const auto mesh = receiver.Create_Mesh(vertices,indices);
    auto projection = Matrix4x4::Identity();
    projection.values[10] = -1.0f/9.0f;
    projection.values[11] = -1.0f/9.0f;
    parameters.view_projection = projection.values;
    const View view{Matrix4x4::Identity(),projection,{}, {0,0,32,32,0,1}};
    RenderLight light;
    light.type = RenderLightType::Directional;
    light.flags = RenderLightFlags::Enabled;
    light.direction = {0,0,-1};
    ShadowSettings settings{2,1,10,0.5f,2,128};
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    for (unsigned frame=0;frame<9;++frame) {
        if (frame != 0) {
            shadows.Clear_Casters();
            settings.map_size = 64;
        }
        if (frame == 2) {
            TreeRenderer trees;
            std::array<TreeVertex,4> tree_vertices;
            for (unsigned index=0;index<4;++index) {
                tree_vertices[index].position = vertices[index].position;
                tree_vertices[index].position[2] = -4;
                tree_vertices[index].uv = vertices[index].uv;
                tree_vertices[index].sway = {1,1,-5};
            }
            TreeParameters tree_parameters;
            tree_parameters.sway[0] = {0.5f,0,0,0};
            BOOST_REQUIRE(trees.Add_Shadow_Caster(shadows,tree_vertices,indices,tree_parameters,cutout));
        }
        if (frame >= 3 && frame < 7) {
            if (frame == 3 || frame == 6) {
                BOOST_REQUIRE(terrain.Initialize(device,shaders));
                TerrainCell cell;
                cell.spacing = {1,2};
                if (frame == 6) cell.origin[0] = 1;
                BOOST_REQUIRE(terrain.Set_Cells(std::span(&cell,1)));
            }
            auto world = Matrix4x4::Identity();
            world.values[3] = world.values[7] = -1;
            if (frame == 5) world.values[3] = 0;
            world.values[11] = -4;
            BOOST_REQUIRE(terrain.Add_Shadow_Caster(shadows,world.values));
        }
        if (frame >= 7) {
            // All four vertices lie outside each cascade, but the quad spans
            // its whole volume. Moving the same geometry clear of the volume
            // must then leave the receiver lit.
            const float offset = frame == 8 ? 1000.0f : 0.0f;
            vertices[0].position = {-100+offset,-100,-4};
            vertices[1].position = {100+offset,-100,-4};
            vertices[2].position = {100+offset,100,-4};
            vertices[3].position = {-100+offset,100,-4};
            const std::span<const RHITextureHandle> textures;
            BOOST_REQUIRE(shadows.Add_Caster(vertices,indices,parameters,textures,style));
        }
        BOOST_REQUIRE(shadows.Render(commands,view,light,settings,target,depth,{0,0,32,32}));
        BOOST_CHECK_EQUAL(Get_Environment_Lighting().parameters.shadow_options[0],2);
        // Render restored the caller's target and viewport.
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(receiver.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,32*32*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        for (int channel=0;channel<3;++channel) {
            const unsigned shadow_x = frame >= 5 ? 24 : frame == 2 ? 16 : 8;
            const unsigned clear_x = frame >= 5 ? 4 : 28;
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*32+shadow_x)*4+channel])-(frame != 1 && frame != 8 ? 77 : 230),2);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*32+clear_x)*4+channel])-(frame == 7 ? 77 : 230),2);
        }
    }
    terrain.Shutdown();
    shadows.Shutdown();
    BOOST_CHECK_EQUAL(Get_Environment_Lighting().parameters.shadow_options[0],0);
    receiver.Destroy_Mesh(mesh);
    receiver.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
    device.Destroy_Texture(cutout);
}

BOOST_AUTO_TEST_CASE(retained_caster_handles_survive_frames_and_expire_on_shutdown)
{
    GraphicsTestDevice device({true});
    DirectionalShadowRenderer shadows;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(shadows.Initialize(device,shaders));
    std::array<PropVertex,3> vertices{};
    vertices[0].position = {-1,-1,0};
    vertices[1].position = {1,-1,0};
    vertices[2].position = {0,1,0};
    const std::array<std::uint32_t,3> indices{0,1,2};
    const auto first = shadows.Create_Caster(vertices,indices);
    BOOST_REQUIRE(shadows.Is_Caster_Valid(first));
    PropParameters parameters;
    parameters.textured = 0;
    const PropStyle style;
    const std::span<const RHITextureHandle> textures;
    BOOST_REQUIRE(shadows.Add_Caster(first,parameters,textures,style));
    shadows.Clear_Casters();
    BOOST_CHECK(shadows.Is_Caster_Valid(first));
    shadows.Shutdown();
    BOOST_CHECK(!shadows.Is_Caster_Valid(first));
    BOOST_REQUIRE(shadows.Initialize(device,shaders));
    const auto second = shadows.Create_Caster(vertices,indices);
    BOOST_REQUIRE(shadows.Is_Caster_Valid(second));
    BOOST_CHECK(!shadows.Destroy_Caster(first));
    BOOST_CHECK(shadows.Is_Caster_Valid(second));
    BOOST_CHECK(shadows.Destroy_Caster(second));
}

BOOST_AUTO_TEST_CASE(shared_caster_buffers_match_independent_meshes_with_mixed_cutouts_and_culling)
{
    struct ResetEnvironment { ~ResetEnvironment() { Get_Environment_Lighting() = {}; } } reset;
    Get_Environment_Lighting() = {};
    GraphicsTestDevice device({true});
    DirectionalShadowRenderer shadows;
    PropRenderer receiver;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(shadows.Initialize(device,shaders));
    BOOST_REQUIRE(receiver.Initialize(device,shaders));
    constexpr unsigned extent = 64;
    const auto target = device.Create_Texture({extent,extent,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({extent,extent,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::uint8_t,8> cutout_pixels{255,255,255,255,255,255,255,0};
    const auto cutout = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(cutout_pixels)),8});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,-5}; vertices[1].position = {1,-1,-5};
    vertices[2].position = {1,1,-5}; vertices[3].position = {-1,1,-5};
    vertices[0].uv = {0,1}; vertices[1].uv = {1,1};
    vertices[2].uv = {1,0}; vertices[3].uv = {0,0};
    for (auto& vertex : vertices) vertex.material_ambient = {1,1,1,1};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto receiver_mesh = receiver.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.textured = 0;
    parameters.scene_ambient = {0.2f,0.2f,0.2f,0};
    parameters.light_direction[0] = {0,0,1,1};
    parameters.light_diffuse[0] = {0.6f,0.6f,0.6f,0};
    auto projection = Matrix4x4::Identity();
    projection.values[10] = projection.values[11] = -1.0f/9.0f;
    parameters.view_projection = projection.values;
    const View view{Matrix4x4::Identity(),projection,{}, {0,0,extent,extent,0,1}};
    RenderLight light;
    light.type = RenderLightType::Directional;
    light.flags = RenderLightFlags::Enabled;
    light.direction = {0,0,-1};
    ShadowSettings settings{2,1,10,0.5f,2,256};
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    auto& commands = device.Immediate_Command_List();
    // Shrink and regrow the shared storage, with rejected and culled batches
    // between visible ranges. Compare every channel against separate meshes.
    for (unsigned count : {16u,4u,32u}) {
        std::array<std::byte,extent*extent*4> reference{},actual{};
        for (bool shared : {false,true}) {
            shadows.Clear_Casters();
            std::vector<ShadowCasterHandle> retained;
            for (unsigned index=0;index<count;++index) {
                const float left = index%4 == 3 ? 1000 : -1.0f+2.0f*index/count;
                const float right = left+2.0f/count;
                vertices[0].position = {left,-1,-4}; vertices[1].position = {right,-1,-4};
                vertices[2].position = {right,1,-4}; vertices[3].position = {left,1,-4};
                auto material = parameters;
                if (shared) {
                    // Both retained and transient casters carry local geometry.
                    // Their local bounds are outside the light volume, while
                    // the instance transform places visible strips back inside.
                    for (auto& vertex : vertices) vertex.position[0] += 32;
                    material.world[3] = -32;
                }
                material.textured = index%2 ? 1.0f : 0.0f;
                material.alpha_cutoff = index%2 ? 0.5f : 0.0f;
                material.uv_transform[0][3] = index%4 == 1 ? 0.5f : 0.0f;
                const std::array textures{cutout};
                if (!shared || index == 0) {
                    const auto mesh = shadows.Create_Caster(vertices,indices);
                    retained.push_back(mesh);
                    BOOST_REQUIRE(shadows.Add_Caster(mesh,material,textures,style));
                } else if (index%3 == 1) {
                    // Exercise bounds after both appending to an empty mesh and
                    // replacing geometry that was outside the cascade volume.
                    auto initial = vertices;
                    for (auto& vertex : initial) vertex.position[0] += 2048;
                    const auto mesh = receiver.Create_Mesh(initial,indices);
                    if (index%6 == 1) {
                        BOOST_REQUIRE(receiver.Update_Mesh(mesh,{},{}));
                        BOOST_REQUIRE(receiver.Append_Mesh(mesh,vertices,indices));
                    } else {
                        BOOST_REQUIRE(receiver.Update_Mesh(mesh,vertices,indices));
                    }
                    const bool added = shadows.Add_Caster(receiver,mesh,material,textures,style,PropInstanceHandle{});
                    BOOST_REQUIRE(added);
                    // The shadow queue retains this exact version independently
                    // of the source owner and shares its GPU buffers.
                    BOOST_REQUIRE(receiver.Destroy_Mesh(mesh));
                } else {
                    const std::array<std::uint32_t,3> invalid{0,1,4};
                    BOOST_CHECK(!shadows.Add_Caster(vertices,invalid,material,textures,style));
                    BOOST_REQUIRE(shadows.Add_Caster(vertices,indices,material,textures,style));
                }
            }
            BOOST_REQUIRE(shadows.Render(commands,view,light,settings,target,depth,{0,0,extent,extent}));
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(receiver.Draw(commands,receiver_mesh,style,parameters,{}));
            BOOST_REQUIRE(device.Readback_Texture(target,shared ? actual : reference,extent*4));
            shadows.Clear_Casters();
            for (const auto mesh : retained) BOOST_REQUIRE(shadows.Destroy_Caster(mesh));
        }
        BOOST_CHECK(actual == reference);
        const auto shadowed = std::to_integer<int>(actual[(extent/2*extent+1)*4]);
        const auto lit = std::to_integer<int>(actual[(extent/2*extent+extent-1)*4]);
        // Small strips include filtered edges; still require a substantial
        // shadow in addition to the exact comparison against retained meshes.
        BOOST_CHECK_GT(lit - shadowed,64);
        BOOST_CHECK_GT(lit,190);
    }
    shadows.Shutdown(); receiver.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth); device.Destroy_Texture(cutout);
}

BOOST_AUTO_TEST_CASE(dense_caster_growth_preserves_every_shadow_across_frames)
{
    struct ResetEnvironment final {
        ~ResetEnvironment() { Get_Environment_Lighting() = {}; }
    } reset;
    Get_Environment_Lighting() = {};
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    DirectionalShadowRenderer shadows;
    PropRenderer receiver;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(shadows.Initialize(device,shaders));
    BOOST_REQUIRE(receiver.Initialize(device,shaders));
    constexpr unsigned extent = 260;
    const auto target = device.Create_Texture({extent,extent,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({extent,extent,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,-5}; vertices[1].position = {1,-1,-5};
    vertices[2].position = {1,1,-5}; vertices[3].position = {-1,1,-5};
    for (auto& vertex : vertices) {
        vertex.normal = {0,0,1};
        vertex.material_ambient = {1,1,1,1};
        vertex.material_diffuse = {1,1,1,1};
        vertex.material_emissive = {0.1f,0.1f,0.1f,0};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto receiver_mesh = receiver.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.textured = 0;
    parameters.scene_ambient = {0.2f,0.2f,0.2f,0};
    parameters.light_direction[0] = {0,0,1,1};
    parameters.light_diffuse[0] = {0.6f,0.6f,0.6f,0};
    auto projection = Matrix4x4::Identity();
    projection.values[10] = projection.values[11] = -1.0f/9.0f;
    parameters.view_projection = projection.values;
    const View view{Matrix4x4::Identity(),projection,{}, {0,0,extent,extent,0,1}};
    RenderLight light;
    light.type = RenderLightType::Directional;
    light.flags = RenderLightFlags::Enabled;
    light.direction = {0,0,-1};
    ShadowSettings settings{2,1,10,0.5f,2,1024};
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    std::array<PropVertex,4> unit_vertices{};
    unit_vertices[0].position = {0,0,-4}; unit_vertices[1].position = {1,0,-4};
    unit_vertices[2].position = {1,1,-4}; unit_vertices[3].position = {0,1,-4};
    const std::array instance_meshes{receiver.Create_Mesh(unit_vertices,indices),receiver.Create_Mesh(unit_vertices,indices)};
    unsigned iteration = 0;
    // Cross 256, 1024 and 4096 allocations, then shrink and regrow. Every
    // caster occupies its own measured pixel; a dropped draw leaves it lit.
    for (const unsigned side : {17u,33u,65u,17u,65u}) {
        const bool instanced = iteration == 1 || iteration >= 3;
        ++iteration;
        shadows.Clear_Casters();
        for (unsigned y=0;y<side;++y) for (unsigned x=0;x<side;++x) {
            const float left = -1.0f + 2.0f*x/side;
            const float bottom = -1.0f + 2.0f*y/side;
            const float width = 2.0f/side;
            vertices[0].position = {left,bottom,-4};
            vertices[1].position = {left+width,bottom,-4};
            vertices[2].position = {left+width,bottom+width,-4};
            vertices[3].position = {left,bottom+width,-4};
            const std::span<const RHITextureHandle> textures;
            if (instanced) {
                auto instance = parameters;
                instance.world[0] = instance.world[5] = width;
                instance.world[3] = left; instance.world[7] = bottom;
                instance.scene_ambient[0] = static_cast<float>(x)/side;
                instance.light_position[1] = {left,bottom,2,1};
                const bool added = shadows.Add_Caster(receiver,instance_meshes[(x+y)%2],instance,textures,style,PropInstanceHandle{});
                BOOST_REQUIRE(added);
            } else BOOST_REQUIRE(shadows.Add_Caster(vertices,indices,parameters,textures,style));
        }
        const auto before = commands.Submission_Counts();
        BOOST_REQUIRE(shadows.Render(commands,view,light,settings,target,depth,{0,0,extent,extent}));
        if (instanced) {
            const auto draws = commands.Submission_Counts().draw_calls-before.draw_calls;
            BOOST_CHECK_GT(draws,0u);
            BOOST_CHECK_LE(draws,settings.cascade_count*2u);
        }
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(receiver.Draw(commands,receiver_mesh,style,parameters,{}));
        std::array<std::byte,extent*extent*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,extent*4));
        for (unsigned y=0;y<side;++y) for (unsigned x=0;x<side;++x) {
            const unsigned px = (2*x+1)*extent/(2*side);
            const unsigned py = (2*y+1)*extent/(2*side);
            BOOST_TEST_CONTEXT("grid " << side << " caster " << x << "," << y) {
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(py*extent+px)*4])-77,2);
            }
        }
    }
    shadows.Shutdown();
    for (const auto mesh : instance_meshes) receiver.Destroy_Mesh(mesh);
    receiver.Destroy_Mesh(receiver_mesh);
    receiver.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(sloped_receivers_do_not_shadow_themselves_between_shadow_texels)
{
    struct ResetEnvironment { ~ResetEnvironment() { Get_Environment_Lighting() = {}; } } reset;
    Get_Environment_Lighting() = {};
    GraphicsTestDevice device({true});
    PropRenderer receiver;
    BOOST_REQUIRE(receiver.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    // A planar surface's depth in the light view varies linearly across X.
    std::array<float,64> shadow_depth{};
    for (unsigned y=0;y<8;++y) for (unsigned x=0;x<8;++x)
        shadow_depth[y*8+x] = 0.5f+0.3f*(2*(x+0.5f)/8-1);
    const auto shadow = device.Create_Texture_Initialized({8,8,1,RHITextureFormat::R32_Float,
        static_cast<unsigned>(RHITextureUsage::ShaderResource)}, {std::as_bytes(std::span(shadow_depth)),8*sizeof(float)});
    BOOST_REQUIRE(shadow.Is_Valid());
    auto& environment = Get_Environment_Lighting();
    environment.shadow_textures[0] = shadow;
    environment.parameters.shadow_options = {1,0.00001f,0,0};
    environment.parameters.shadow_splits[0] = 10;
    environment.parameters.shadow_view_depth = {0,0,0,1};
    auto light_projection = Matrix4x4::Identity();
    light_projection.values[8] = 0.3f;
    light_projection.values[10] = 0;
    light_projection.values[11] = 0.5f;
    environment.parameters.shadow_view_projection[0] = light_projection.values;
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    for (auto& vertex : vertices) {
        vertex.normal = {0,0,1}; vertex.material_ambient = {0,0,0,1};
        vertex.material_diffuse = {1,1,1,1};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = receiver.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.light_direction[0] = {0,0,1,1};
    parameters.light_diffuse[0] = {1,1,1,0};
    parameters.textured = 0;
    PropStyle style; style.blend = RHIBlendMode::Disabled;
    const auto target = device.Create_Texture({64,64,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({64,64,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands = device.Immediate_Command_List();
    for (float camera_offset : {0.0f,0.013f,-0.021f}) {
        auto projection = Matrix4x4::Identity(); projection.values[3] = camera_offset;
        parameters.view_projection = projection.values;
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,64,64}));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(receiver.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,64*64*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
        for (unsigned x=16;x<48;++x)
            BOOST_CHECK_GE(std::to_integer<unsigned>(pixels[(32*64+x)*4]),253u);
    }
    receiver.Shutdown(); environment = {};
    device.Destroy_Texture(shadow); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}



BOOST_AUTO_TEST_CASE(skinned_casters_use_posed_bounds_and_retain_cutout_pose_through_recreation)
{
    struct ResetEnvironment { ~ResetEnvironment() { Get_Environment_Lighting()={}; } } reset;
    Get_Environment_Lighting()={};
    GraphicsTestDevice device({true});
    PropRenderer renderer; DirectionalShadowRenderer shadows;
    const auto shaders=Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders)); BOOST_REQUIRE(shadows.Initialize(device,shaders));
    const auto target=device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    const std::array<std::uint8_t,8> texels{255,255,255,255,255,255,255,0};
    const auto texture=device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(texels)),8});
    const std::array textures{texture};
    std::array<PropVertex,4> vertices{};
    vertices[0].position={99,-1,-4}; vertices[1].position={101,-1,-4};
    vertices[2].position={101,1,-4}; vertices[3].position={99,1,-4};
    vertices[0].uv={0,1}; vertices[1].uv={1,1}; vertices[2].uv={1,0}; vertices[3].uv={0,0};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto caster=renderer.Create_Mesh(vertices,indices);
    for (auto& vertex : vertices) {
        vertex.position[0]-=100; vertex.position[2]=-5;
        vertex.material_ambient={1,1,1,1}; vertex.material_emissive={.1f,.1f,.1f,0};
    }
    const auto receiver=renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.scene_ambient={.2f,.2f,.2f,0}; parameters.light_direction[0]={0,0,1,1};
    parameters.light_diffuse[0]={.6f,.6f,.6f,0};
    auto projection=Matrix4x4::Identity(); projection.values[10]=projection.values[11]=-1.f/9;
    parameters.view_projection=projection.values;
    const View view{Matrix4x4::Identity(),projection,{}, {0,0,32,32,0,1}};
    RenderLight light; light.type=RenderLightType::Directional; light.flags=RenderLightFlags::Enabled; light.direction={0,0,-1};
    ShadowSettings settings{2,1,10,.5f,2,128};
    PropStyle style; style.blend=RHIBlendMode::Disabled;
    PropSkinOwner skin; PropInstanceOwner instance;
    auto& commands=device.Immediate_Command_List();
    for (unsigned frame=0; frame<3; ++frame) {
        shadows.Clear_Casters();
        PropBoneTransform bone{1,0,0,frame==1 ? 100.f : -100.f,0,1,0,0,0,0,1,0};
        auto palette=skin.Update(renderer.Instances().Palettes(),1,[&](std::size_t) -> const auto& { return bone; });
        parameters.textured=1; parameters.alpha_cutoff=.5f;
        const auto record=instance.Update(renderer.Instances(),parameters,palette);
        BOOST_REQUIRE(shadows.Add_Caster(renderer,caster,parameters,textures,style,record));
        // The submitted caster owns the original palette and its posed bounds.
        bone[3]=500;
        palette=skin.Update(renderer.Instances().Palettes(),1,[&](std::size_t) -> const auto& { return bone; });
        instance.Update(renderer.Instances(),parameters,palette);
        if (frame==2) { renderer.Shutdown(); BOOST_REQUIRE(renderer.Initialize(device,shaders)); }
        const auto before=commands.Submission_Counts();
        BOOST_REQUIRE(shadows.Render(commands,view,light,settings,target,depth,{0,0,32,32}));
        if (frame==1) BOOST_CHECK_EQUAL(commands.Submission_Counts().draw_calls,before.draw_calls);
        else BOOST_CHECK(commands.Submission_Counts().draw_calls>before.draw_calls);
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        parameters.textured=parameters.alpha_cutoff=0;
        BOOST_REQUIRE(renderer.Draw(commands,receiver,style,parameters,{}));
        std::array<std::byte,32*32*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,128));
        for (const auto y : {10u,22u}) for (unsigned channel=0; channel<3; ++channel) {
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*32+8)*4+channel])-(frame==1 ? 230 : 77),2);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*32+24)*4+channel])-230,2);
        }
    }
    shadows.Shutdown(); renderer.Destroy_Mesh(caster); renderer.Destroy_Mesh(receiver); renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
