module;
#define BOOST_TEST_MODULE PropDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Graphics.Scene.Props.Drawing.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Material;
import Graphics.Scene.Props.Extraction;
import Graphics.Scene.Props.MeshSet;
import Graphics.Scene.Models.SourceRevision;
import Graphics.Scene.Props.MaterialPassQueue;
import Graphics.Scene.Lighting.Environment;
import Assets.Adapters.W3D.Materials;
using namespace Graphics;

import Graphics.Scene.Props.Lighting;
import Graphics.Materials.TextureCoordinates;
import Assets.Images.PixelEncoding;
import Graphics.Resources.Textures.Storage;
import Assets.Math;

BOOST_AUTO_TEST_CASE(geometry_bounds_follow_owned_vertices_and_successful_edits)
{
    PropGeometry geometry;
    const auto check = [&](std::array<float,3> minimum,std::array<float,3> maximum) {
        BOOST_CHECK(geometry.Minimum_Position() == minimum);
        BOOST_CHECK(geometry.Maximum_Position() == maximum);
    };
    check({0,0,0},{0,0,0});
    std::array<PropVertex,3> vertices{};
    vertices[0].position = {2,3,4};
    vertices[1].position = {5,9,7};
    vertices[2].position = {4,6,8};
    const std::array<std::uint32_t,3> indices{0,1,2};
    BOOST_REQUIRE(geometry.Assign(vertices,indices));
    check({2,3,4},{5,9,8});
    for (auto& vertex : vertices) vertex.position = {-7,-2,-5};
    check({2,3,4},{5,9,8});
    BOOST_REQUIRE(geometry.Append(vertices,indices));
    check({-7,-2,-5},{5,9,8});
    vertices[0].position[0] = std::bit_cast<float>(0x7f800000u);
    BOOST_CHECK(!geometry.Assign(vertices,indices));
    BOOST_CHECK(!geometry.Append(vertices,indices));
    check({-7,-2,-5},{5,9,8});
    BOOST_REQUIRE(geometry.Assign({},{}));
    check({0,0,0},{0,0,0});
    vertices[0].position[0] = -7;
    BOOST_REQUIRE(geometry.Append(vertices,indices));
    check({-7,-2,-5},{-7,-2,-5});
    BOOST_REQUIRE(geometry.Append({},{}));
    check({-7,-2,-5},{-7,-2,-5});
}

BOOST_AUTO_TEST_CASE(retained_materials_draw_correctly_after_switches_changes_and_renderer_recreation)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device, shaders));
    const auto target = device.Create_Texture({32,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    BOOST_REQUIRE(target.Is_Valid());
    const auto depth = device.Create_Texture({32,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(depth.Is_Valid());
    const std::array<std::byte,8> texels{
        std::byte{200},std::byte{30},std::byte{60},std::byte{128},
        std::byte{10},std::byte{180},std::byte{40},std::byte{192}};
    const auto texture = device.Create_Texture_Initialized({2,1}, {texels,8});
    BOOST_REQUIRE(texture.Is_Valid());
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,.5f}; vertices[1].position = {1,-1,.5f};
    vertices[2].position = {1,1,.5f}; vertices[3].position = {-1,1,.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto first = renderer.Create_Mesh(vertices, indices);
    const auto second = renderer.Create_Mesh(vertices, indices);
    BOOST_REQUIRE(first.Is_Valid()); BOOST_REQUIRE(second.Is_Valid());
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.depth_write = style.depth_test = false;
    style.samplers[0].Set_Filter(RHISamplerFilter::Point);
    const std::array<std::array<int,4>,8> expected{{
        {200,30,60,128}, {10,180,40,96}, {200,30,60,128}, {10,180,40,96},
        {200,30,60,32}, {10,180,40,48}, {0,0,0,0}, {10,180,40,96}}};
    auto& commands = device.Immediate_Command_List();
    for (unsigned frame = 0; frame < 3; ++frame) {
        if (frame == 1) BOOST_REQUIRE(renderer.Update_Mesh(first, vertices, indices));
        if (frame == 2) {
            renderer.Shutdown();
            BOOST_REQUIRE(renderer.Initialize(device, shaders));
        }
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,32,8}));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        PropParameters a;
        a.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        PropParameters b = a;
        b.uv_transform[0][3] = .75f;
        b.opacity = .5f;
        const auto draw = [&](unsigned stripe, PropMeshHandle mesh, const PropParameters& parameters) {
            BOOST_REQUIRE(commands.Set_Viewport({stripe*4,0,4,8}));
            BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, std::array{texture}));
        };
        draw(0, first, a); draw(1, second, b);
        draw(2, first, a); draw(3, second, b);
        a.opacity = .25f; draw(4, first, a);
        a.uv_transform[0][3] = .75f; draw(5, first, a);
        b.alpha_cutoff = 1; draw(6, second, b);
        b.alpha_cutoff = 0; draw(7, second, b);
        if (frame == 2) {
            BOOST_REQUIRE(renderer.Destroy_Mesh(first));
            BOOST_REQUIRE(renderer.Destroy_Mesh(second));
        }
        std::array<std::byte,32*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32*4));
        for (unsigned y = 0; y < 8; ++y)
            for (unsigned x = 0; x < 32; ++x)
                for (unsigned channel = 0; channel < 4; ++channel)
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*32+x)*4+channel])
                        - expected[x/4][channel], 2);
    }
    renderer.Shutdown(); device.Destroy_Texture(texture);
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(prepared_vertex_colors_retain_quantization_and_alpha_when_drawn)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    BOOST_REQUIRE(target.Is_Valid());
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(depth.Is_Valid());
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    style.cull=RHICullMode::None;
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured=0;
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    struct Case { Assets::Color4f color; std::array<int,4> expected; };
    const Case cases[]{
        {{-1,0.5f,2,0.25f},{0,128,255,64}},
        {{0.25f,0.5f,0.75f,0},{64,128,191,0}},
        {{1,0.125f,0.5f,1},{255,32,128,255}}};
    for (const auto& value : cases) {
        const auto color=Assets::Color_From_ARGB(Assets::Color_To_ARGB(value.color));
        for (auto& vertex : vertices) vertex.color={color.r,color.g,color.b,color.a};
        BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-value.expected[channel],2);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(packed_drawing_colors_retain_gpu_rgb_and_alpha)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.primary_gradient=0;
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    struct Case {
        Assets::PixelEncoding encoding;
        RHITextureFormat storage;
        std::uint32_t color;
        std::array<int,4> expected;
    };
    const Case cases[]{
        {Assets::PixelEncoding::BGRA8,RHITextureFormat::BGRA8_UNorm,0x80f84020,{248,64,32,128}},
        {Assets::PixelEncoding::BGRX8,RHITextureFormat::BGRX8_UNorm,0x80f84020,{248,64,32,255}},
        {Assets::PixelEncoding::BGR565,RHITextureFormat::BGR565_UNorm,0x80f84020,{255,65,33,255}},
        {Assets::PixelEncoding::BGRA5551,RHITextureFormat::BGRA5551_UNorm,0x7ff84020,{255,66,33,0}},
        {Assets::PixelEncoding::BGRA5551,RHITextureFormat::BGRA5551_UNorm,0x80f84020,{255,66,33,255}},
        {Assets::PixelEncoding::BGRA4444,RHITextureFormat::BGRA4444_UNorm,0x80f84020,{255,68,34,136}},
        {Assets::PixelEncoding::Alpha8,RHITextureFormat::A8_UNorm,0x80f84020,{0,0,0,128}}};
    for (const auto& value : cases) {
        const auto packed=Assets::Pack_Image_Color(value.encoding,value.color);
        const auto size=Assets::Pixel_Size(value.encoding);
        std::array<std::byte,4> bytes{};
        for (unsigned i=0;i<size;++i) bytes[i]=std::byte(packed>>(i*8));
        const auto storage=Texture_Storage_Format(value.encoding);
        BOOST_CHECK(storage==value.storage);
        const auto texture=device.Create_Texture_Initialized({1,1,1,storage},
            {std::span(bytes).first(size),size});
        BOOST_REQUIRE(texture.Is_Valid());
        const std::array<RHITextureHandle,4> textures{texture,{},{},{}};
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-value.expected[channel],2);
        device.Destroy_Texture(texture);
    }
    // Authored premultiplication changes material blending, not the BC storage
    // layout. Sample each encoding directly to catch color/alpha block swaps.
    using E=Assets::PixelEncoding;
    for (auto encoding : {E::BC1,E::BC2,E::BC2Premultiplied,E::BC3,E::BC3Premultiplied}) {
        const bool bc1=encoding==E::BC1;
        const bool bc2=encoding==E::BC2 || encoding==E::BC2Premultiplied;
        std::array<std::byte,16> block{};
        const unsigned color_offset=bc1 ? 0 : 8;
        block[color_offset+1]=std::byte{0xf8}; // RGB565 red, endpoint index zero.
        if (bc2) std::fill_n(block.begin(),8,std::byte{0x88});
        else if (!bc1) block[0]=block[1]=std::byte{128};
        const unsigned block_size=bc1 ? 8 : 16;
        const auto texture=device.Create_Texture_Initialized({4,4,1,Texture_Storage_Format(encoding)},
            {std::span(block).first(block_size),block_size});
        BOOST_REQUIRE(texture.Is_Valid());
        const std::array<RHITextureHandle,4> textures{texture,{},{},{}};
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        const std::array expected{255,0,0,bc1 ? 255 : bc2 ? 136 : 128};
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-expected[channel],2);
        device.Destroy_Texture(texture);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(texture_coordinate_modes_and_projection_preserve_drawing_on_both_stages)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    const auto shaders=Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    std::array<std::uint8_t,4*4*4> texels{};
    for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) {
        const unsigned offset=(y*4+x)*4;
        texels[offset]=static_cast<std::uint8_t>(32+x*48);
        texels[offset+1]=static_cast<std::uint8_t>(16+y*48);
        texels[offset+2]=64;
        texels[offset+3]=static_cast<std::uint8_t>(96+y*16+x*4);
    }
    const auto texture=device.Create_Texture_Initialized({4,4},{std::as_bytes(std::span(texels)),16});
    BOOST_REQUIRE(texture.Is_Valid());
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) {
        vertex.normal={0,0,1}; vertex.uv={0.25f,0.125f}; vertex.secondary_uv=vertex.uv;
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    for (auto& sampler : style.samplers) {
        sampler.Set_Filter(RHISamplerFilter::Point);
        sampler.address.fill(RHISamplerAddress::Clamp);
    }
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.primary_gradient=0;
    parameters.detail_color=1; parameters.detail_alpha=1;
    const std::array<RHITextureHandle,4> textures{texture,texture,{},{}};
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    const auto check = [&](unsigned stage,TextureCoordinateMode mode,
        const std::array<float,16>& matrix,unsigned x,unsigned y) {
        parameters.uv_sources[stage*2]=static_cast<float>(mode.source);
        parameters.uv_sources[stage*2+1]=mode.projected ? 1.0f : 0.0f;
        parameters.uv_transform[stage]=matrix;
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])
                -texels[(y*4+x)*4+channel],1);
    };
    const std::array<float,16> projection{0,0,0,0.375f, 0,0,0,0.125f,
        0,0,0,0.875f, 0,0,0,0.5f};
    const std::array modes{TextureProjection::Orthographic,TextureProjection::Perspective,
        TextureProjection::DepthGradient,TextureProjection::NormalGradient};
    const std::array<std::array<unsigned,2>,4> expected{{{1,0},{3,1},{0,3},{0,2}}};
    for (unsigned stage=0;stage<2;++stage) {
        BOOST_TEST_CONTEXT("texture stage " << stage) {
            parameters.secondary_texture=static_cast<float>(stage);
            check(stage,{},Make_Affine_Texture_Transform({1,0,0.125f,0,1,0}),1,0);
            check(stage,{},Make_Affine_Texture_Transform({0,-1,1,1,0,0.125f}),3,1);
            check(stage,{},Make_Affine_Texture_Transform({1,0,0.5f,0,1,0.5f}),3,2);
            for (unsigned index=0;index<modes.size();++index) {
                const auto mode=Projection_Texture_Coordinates(modes[index]);
                const auto matrix=Make_Projection_Texture_Transform(projection,modes[index],0.125f,{0,0,0.625f});
                check(stage,mode,matrix,expected[index][0],expected[index][1]);
            }
            const auto local=Make_Affine_Texture_Transform({1,0,0.125f,0,1,-0.0625f});
            check(stage,Projection_Texture_Coordinates(TextureProjection::Perspective),
                Compose_Texture_Projection(projection,local),3,0);
            // Environment coordinates require both the direction and the
            // affine bias. Reflection Z is -1/3 here; normal Z is +1.
            const std::array<float,16> environment{0,0,0.25f,0.625f, 0,0,0,0.125f,
                0,0,1,0, 0,0,0,1};
            check(stage,{TextureCoordinateSource::CameraNormal,false},environment,3,0);
            check(stage,{TextureCoordinateSource::CameraReflection,false},environment,2,0);
        }
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(detail_material_equations_preserve_rgb_and_alpha_between_draws)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto texture = [&](std::array<std::uint8_t,4> color) {
        const auto handle = device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
        BOOST_REQUIRE(handle.Is_Valid());
        return handle;
    };
    const auto base = texture({64,128,192,64});
    const auto detail = texture({192,64,128,192});
    const auto target = device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    BOOST_REQUIRE(target.Is_Valid());
    const auto depth = device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(depth.Is_Valid());
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropStyle style;
    style.blend=RHIBlendMode::Disabled;
    style.depth_test=false; style.depth_write=false;
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.primary_gradient=0;
    parameters.secondary_texture=1;
    const std::array<RHITextureHandle,4> textures{base,detail,{},{}};
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));

    // Authored order: disabled, detail, multiply, screen, add, subtract in
    // both directions, two alpha blends, signed add, signed double add,
    // double multiply, and add scaled by the base alpha.
    const std::array<std::array<int,3>,13> expected_rgb{{
        {64,128,192}, {192,64,128}, {48,32,96}, {208,160,224},
        {255,192,255}, {0,64,64}, {128,0,0}, {160,80,144},
        {96,112,176}, {129,65,193}, {255,129,255}, {96,64,193},
        {112,144,224}}};
    const std::array<int,4> expected_alpha{64,192,48,208};
    const auto draw_and_check = [&](std::array<int,4> expected) {
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-expected[channel],2);
    };
    for (unsigned color=0;color<expected_rgb.size();++color)
        for (unsigned alpha=0;alpha<expected_alpha.size();++alpha) {
        BOOST_TEST_CONTEXT("detail color " << color << ", alpha " << alpha) {
            parameters.detail_color=static_cast<float>(color);
            parameters.detail_alpha=static_cast<float>(alpha);
            draw_and_check({expected_rgb[color][0],expected_rgb[color][1],
                expected_rgb[color][2],expected_alpha[alpha]});
            // A subsequent material with its detail texture disabled must
            // ignore the previous draw's texture operations and alpha rule.
            parameters.secondary_texture=0;
            draw_and_check({64,128,192,64});
            parameters.secondary_texture=1;
        }
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(base); device.Destroy_Texture(detail);
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(decoded_material_colors_survive_gpu_resource_recreation)
{
    struct ResetEnvironment { ~ResetEnvironment() { Get_Environment_Lighting() = {}; } } reset;
    Get_Environment_Lighting() = {};
    // One source vertex-material info chunk, with distinct RGB channels.
    std::array<std::byte,40> bytes{};
    const auto write = [&](unsigned offset, std::uint32_t value) {
        for (unsigned byte=0;byte<4;++byte) bytes[offset+byte]=std::byte(value>>(8*byte));
    };
    write(0,0x2D); write(4,32);
    write(12,0x00302010); // Ambient: 16,32,48.
    write(16,0x00782850); // Diffuse: 80,40,120.
    write(20,0x00604020); // Specular: 32,64,96.
    write(24,0x00181008); // Emissive: 8,16,24.
    write(28,std::bit_cast<std::uint32_t>(16.0f));
    write(32,std::bit_cast<std::uint32_t>(0.5f));
    Assets::W3D::W3DVertexMaterialData decoded;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Vertex_Material(bytes,decoded));
    const auto rgb = [](const auto& color) { return std::array{color.r,color.g,color.b}; };
    PropMaterial material;
    material.ambient=rgb(decoded.material.ambient_color);
    material.diffuse=rgb(decoded.material.base_color);
    material.specular=rgb(decoded.material.specular_color);
    material.emissive=rgb(decoded.material.emissive_color);
    material.opacity=decoded.material.opacity;
    material.shininess=decoded.material.shininess;
    material.lighting=true;
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) {
        vertex.normal={0,0,1};
        Apply_Prop_Material(vertex,material);
    }
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    const auto shaders=Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    decoded={}; bytes={}; vertices={};
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured=0;
    parameters.scene_ambient={0.5f,0.5f,0.5f,0};
    parameters.light_direction[0]={0,0,1,1};
    parameters.light_diffuse[0]={0.25f,0.25f,0.25f,0};
    PropStyle style;
    style.blend=RHIBlendMode::Disabled;
    auto& commands=device.Immediate_Command_List();
    for (unsigned pass=0;pass<2;++pass) {
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
        BOOST_REQUIRE(commands.Clear({1,0,1,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        const std::array expected{36,42,78,128};
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-expected[channel],2);
        if (pass==0) {
            renderer.Shutdown();
            BOOST_REQUIRE(renderer.Initialize(device,shaders));
        }
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(persistent_mesh_draws_indices_above_16_bits_after_resource_recreation)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const auto target = device.Create_Texture({16,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(target.Is_Valid());
    BOOST_REQUIRE(depth.Is_Valid());

    constexpr std::uint32_t first = 65536;
    std::vector<PropVertex> vertices(first+4);
    vertices[first].position = {-1,-1,0.5f};
    vertices[first+1].position = {1,-1,0.5f};
    vertices[first+2].position = {1,1,0.5f};
    vertices[first+3].position = {-1,1,0.5f};
    for (std::uint32_t i=first;i<vertices.size();++i) vertices[i].color = {0,1,0,1};
    const std::array<std::uint32_t,6> indices{first,first+1,first+2,first,first+2,first+3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    // Submitted geometry owns its data, including after the source is released.
    vertices.clear();
    vertices.shrink_to_fit();
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured = 0;
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    for (unsigned pass=0;pass<2;++pass) {
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,16,8}));
        BOOST_REQUIRE(commands.Clear({1,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,16*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for (unsigned pixel=0;pixel<16*8;++pixel) {
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[pixel*4]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[pixel*4+1]),255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[pixel*4+2]),0u);
        }
        if (pass==0) {
            renderer.Shutdown();
            BOOST_REQUIRE(renderer.Initialize(device,shaders));
        }
    }
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(persistent_mesh_versions_survive_source_changes_and_device_recreation)
{
    for (const bool prepared : {false,true}) {
        GraphicsTestDevice device({true});
        PropRenderer renderer;
        const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
        BOOST_REQUIRE(renderer.Initialize(device,shaders));
        const auto target = device.Create_Texture({16,8,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({16,8,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        std::array<PropVertex,4> vertices{};
        vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
        vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
        for (auto& vertex : vertices) vertex.color = {1,0,0,1};
        const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
        PropMeshSet meshes;
        const auto synchronize = [&](std::span<const std::uint32_t> topology) {
            if (!prepared) return meshes.Synchronize(renderer,0,vertices,topology);
            const std::array<std::span<const std::byte>,2> sources{std::as_bytes(std::span(vertices)),std::as_bytes(topology)};
            const auto found = meshes.Find_Prepared(renderer,0,sources);
            return found.Is_Valid() ? found : meshes.Publish_Prepared(renderer,0,sources,vertices,topology);
        };
        const auto red = synchronize(indices);
        BOOST_REQUIRE(red.Is_Valid());
        BOOST_CHECK(synchronize(indices) == red);
        PropParameters parameters;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        parameters.textured = 0;
        parameters.world[0] = 0.5f;
        parameters.world[3] = -0.5f;
        PropStyle style;
        style.blend = RHIBlendMode::Disabled;
        MaterialPassQueue queue;
        BOOST_REQUIRE(queue.Submit(renderer,red,style,parameters,{}));
        for (auto& vertex : vertices) vertex.color = {0,0,1,1};
        BOOST_CHECK(!renderer.Update_Mesh(red,vertices,indices));
        BOOST_CHECK(!renderer.Append_Mesh(red,vertices,indices));
        const auto blue = synchronize(indices);
        BOOST_REQUIRE(blue.Is_Valid());
        BOOST_CHECK(blue != red);
        const std::array<std::uint32_t,3> invalid{0,1,4};
        BOOST_CHECK(!synchronize(invalid).Is_Valid());
        BOOST_CHECK(synchronize(indices) == blue);
        parameters.world[3] = 0.5f;
        BOOST_REQUIRE(queue.Submit(renderer,blue,style,parameters,{}));
        meshes.Clear();
        auto& commands = device.Immediate_Command_List();
        // Upload once before resource recreation; retained CPU versions must upload
        // again on the first subsequent draw without synchronizing their source.
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,16,8}));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,blue,style,parameters,{}));
        renderer.Shutdown();
        BOOST_REQUIRE(renderer.Initialize(device,shaders));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(queue.Flush(commands));
        std::array<std::byte,16*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for (unsigned x=0;x<16;++x) for (unsigned channel=0;channel<3;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*16+x)*4+channel])
                - (channel == (x<8 ? 0 : 2) ? 255 : 0),1);
        BOOST_CHECK(renderer.Mesh_Geometry(red) == nullptr);
        BOOST_CHECK(renderer.Mesh_Geometry(blue) == nullptr);
        renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(mesh_edits_preserve_vertices_topology_and_rejected_updates)
{
    for (const bool prepared : {false,true}) {
        GraphicsTestDevice device({true});
        PropRenderer renderer;
        const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
        BOOST_REQUIRE(renderer.Initialize(device,shaders));
        const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        std::array<PropVertex,4> vertices{};
        vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
        vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
        const std::array<std::uint32_t,6> full{0,1,2,0,2,3};
        const std::array<std::uint32_t,3> lower{0,1,2}, upper{0,2,3}, invalid{0,1,4};
        PropMeshSet meshes;
        const auto synchronize = [&](std::span<const std::uint32_t> indices) {
            if (!prepared) return meshes.Synchronize(renderer,0,vertices,indices);
            const std::array<std::span<const std::byte>,2> sources{
                std::as_bytes(std::span(vertices)),std::as_bytes(indices)};
            return meshes.Publish_Prepared(renderer,0,sources,vertices,indices);
        };
        PropParameters parameters;
        parameters.textured=0;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        PropStyle style; style.blend=RHIBlendMode::Disabled;
        auto& commands=device.Immediate_Command_List();
        const auto check = [&](PropMeshHandle mesh,std::array<int,4> top_left,std::array<int,4> bottom_right) {
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
            BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
            BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
            BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
            std::array<std::byte,16*16*4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
            for (unsigned channel=0;channel<4;++channel) {
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(2*16+2)*4+channel])-top_left[channel],1);
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(13*16+13)*4+channel])-bottom_right[channel],1);
            }
        };
        for (auto& vertex : vertices) vertex.color={1,0,0,1};
        auto mesh=synchronize(full);
        BOOST_REQUIRE(mesh.Is_Valid());
        check(mesh,{255,0,0,255},{255,0,0,255});
        for (auto& vertex : vertices) vertex.color={0,1,0,1};
        mesh=synchronize(full);
        BOOST_REQUIRE(mesh.Is_Valid());
        check(mesh,{0,255,0,255},{0,255,0,255});
        for (auto& vertex : vertices) vertex.color={0,0,1,1};
        mesh=synchronize(lower);
        BOOST_REQUIRE(mesh.Is_Valid());
        check(mesh,{0,0,0,0},{0,0,255,255});
        mesh=synchronize(upper);
        BOOST_REQUIRE(mesh.Is_Valid());
        check(mesh,{0,0,255,255},{0,0,0,0});
        BOOST_CHECK(!synchronize(invalid).Is_Valid());
        check(mesh,{0,0,255,255},{0,0,0,0});
        renderer.Shutdown();
        BOOST_REQUIRE(renderer.Initialize(device,shaders));
        check(mesh,{0,0,255,255},{0,0,0,0});
        meshes.Clear(); renderer.Shutdown();
        device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}

BOOST_AUTO_TEST_CASE(persistent_local_mesh_instances_match_baked_geometry)
{
    struct ResetEnvironment { ~ResetEnvironment() { Get_Environment_Lighting() = {}; } } reset;
    Get_Environment_Lighting() = {};
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    constexpr unsigned extent = 32;
    const auto target = device.Create_Texture({extent,extent,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({extent,extent,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::uint8_t,16> texels{255,128,64,255,64,255,128,0,
        128,64,255,255,255,255,255,255};
    const auto texture = device.Create_Texture_Initialized({2,2},
        {std::as_bytes(std::span(texels)),8});
    std::array<PropVertex,4> local{};
    local[0].position = {-1,-1,0}; local[1].position = {1,-1,0};
    local[2].position = {1,1,0}; local[3].position = {-1,1,0};
    for (auto& vertex : local) {
        vertex.normal = {0.5f,0.25f,1};
        vertex.material_ambient = {1,1,1,1};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto persistent = renderer.Create_Mesh(local,indices);
    BOOST_REQUIRE(persistent.Is_Valid());
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,extent,extent}));
    const std::array<std::array<float,16>,3> transforms{{
        {0,-0.5f,0,0.25f,0.5f,0,0,-0.25f,0,0,1,0.5f,0,0,0,1},
        {-0.5f,0,0,-0.25f,0,0.25f,0,0.25f,0,0,1,0.5f,0,0,0,1},
        {0.25f,0,0,0,0,0.5f,0,0,0,0,0.5f,0.25f,0,0,0,1}}};
    for (const auto& world : transforms) for (bool world_normal : {false,true}) {
        PropParameters parameters;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        parameters.world = world;
        parameters.normal_in_world_space = world_normal ? 1.0f : 0.0f;
        parameters.alpha_cutoff = 0.5f;
        parameters.scene_ambient = {0.125f,0.125f,0.125f,0};
        parameters.light_direction[0] = {0.5f,0.25f,1,1};
        parameters.light_diffuse[0] = {0.5f,0.5f,0.5f,0};
        parameters.uv_sources[0] = 4;
        parameters.uv_transform[0] = {0.5f,0,0,0.5f,0,0.5f,0,0.5f,0,0,1,0,0,0,0,1};
        std::array<std::byte,extent*extent*4> actual{},reference{};
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,persistent,style,parameters,std::array{texture}));
        BOOST_REQUIRE(device.Readback_Texture(target,actual,extent*4));
        auto baked = local;
        for (unsigned i=0;i<baked.size();++i) {
            for (unsigned row=0;row<3;++row) {
                baked[i].position[row] = world[row*4+3];
                baked[i].normal[row] = world_normal ? local[i].normal[row] : 0;
                for (unsigned column=0;column<3;++column) {
                    baked[i].position[row] += world[row*4+column]*local[i].position[column];
                    if (!world_normal)
                        baked[i].normal[row] += world[row*4+column]*local[i].normal[column];
                }
            }
            baked[i].uv = {baked[i].position[0]*0.5f+0.5f,baked[i].position[1]*0.5f+0.5f};
        }
        parameters.world = PropParameters{}.world;
        parameters.uv_sources[0] = 0;
        parameters.uv_transform[0] = PropParameters{}.uv_transform[0];
        const auto mesh = renderer.Create_Mesh(baked,indices);
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{texture}));
        BOOST_REQUIRE(device.Readback_Texture(target,reference,extent*4));
        unsigned visible = 0;
        for (unsigned i=0;i<actual.size();++i) {
            BOOST_CHECK_SMALL(std::to_integer<int>(actual[i])-std::to_integer<int>(reference[i]),1);
            if (i%4 == 3 && actual[i] != std::byte{0}) ++visible;
        }
        BOOST_CHECK_GT(visible,16u);
        BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
    }
    BOOST_REQUIRE(renderer.Destroy_Mesh(persistent));
    renderer.Shutdown();
    device.Destroy_Texture(texture); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(recycled_buffers_preserve_queued_draws_and_live_meshes)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({64,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({64,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured = 0;
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    for (auto& vertex : vertices) vertex.color = {0,0,1,1};
    const auto retained = renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(commands.Set_Viewport({63,0,1,16}));
    BOOST_REQUIRE(renderer.Draw(commands,retained,style,parameters,{}));
    for (unsigned column=0; column<63; ++column) {
        for (auto& vertex : vertices)
            vertex.color = column%2 ? std::array<float,4>{0,1,0,1} : std::array<float,4>{1,0,0,1};
        const auto mesh = renderer.Create_Mesh(vertices,indices);
        BOOST_REQUIRE(commands.Set_Viewport({column,0,1,16}));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
    }
    // Read only after all draws and releases: recycling must not overwrite
    // commands still queued on the GPU or another mesh's live allocation.
    BOOST_REQUIRE(commands.Set_Viewport({63,0,1,16}));
    BOOST_REQUIRE(renderer.Draw(commands,retained,style,parameters,{}));
    std::array<std::byte,64*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,64*4));
    for (unsigned column=0; column<64; ++column) {
        const unsigned channel = column==63 ? 2 : column%2;
        for (unsigned c=0;c<3;++c)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*64+column)*4+c])-(c==channel ? 255 : 0),1);
    }
    BOOST_REQUIRE(renderer.Destroy_Mesh(retained));
    renderer.Shutdown();
    device.Destroy_Texture(target);
    device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(vertex_validation_preserves_binary32_finite_boundaries)
{
    PropGeometry geometry;
    const std::array<std::uint32_t,3> indices{0,1,2};
    std::array<PropVertex,3> vertices{};
    BOOST_REQUIRE(geometry.Assign(vertices,indices));
    for (const std::uint32_t bits : {0u,0x80000000u,1u,0x80000001u,
        0x007fffffu,0x00800000u,0x7f7fffffu,0xff7fffffu,
        0x7f800000u,0xff800000u,0x7fc00000u,0xffc00000u,0x7f800001u}) {
        const float value = std::bit_cast<float>(bits);
        const bool finite = bits != 0x7f800000u && bits != 0xff800000u
            && bits != 0x7fc00000u && bits != 0xffc00000u && bits != 0x7f800001u;
        const auto check = [&](auto member) {
            for (unsigned vertex=0;vertex<vertices.size();++vertex)
                for (unsigned component=0;component<(vertices[vertex].*member).size();++component) {
                    vertices = {};
                    (vertices[vertex].*member)[component] = value;
                    BOOST_CHECK_EQUAL(geometry.Assign(vertices,indices),finite);
                }
        };
        check(&PropVertex::position); check(&PropVertex::color);
        check(&PropVertex::uv); check(&PropVertex::secondary_uv);
        check(&PropVertex::normal); check(&PropVertex::secondary_color);
        check(&PropVertex::material_ambient); check(&PropVertex::material_diffuse);
        check(&PropVertex::material_emissive); check(&PropVertex::material_specular);
    }
}

BOOST_AUTO_TEST_CASE(appended_ranges_preserve_queued_draws_and_reject_invalid_geometry)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({16,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured = 0;
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    const auto mesh = renderer.Create_Mesh({},{});
    for (unsigned column=0;column<16;++column) {
        for (auto& vertex : vertices)
            vertex.color = column%2 ? std::array<float,4>{0,1,0,1} : std::array<float,4>{1,0,0,1};
        auto invalid = vertices;
        invalid.back().material_specular[3] = std::bit_cast<float>(0x7fc00000u);
        BOOST_CHECK(!renderer.Append_Mesh(mesh,invalid,indices));
        const std::array<std::uint32_t,3> invalid_indices{0,1,4};
        BOOST_CHECK(!renderer.Append_Mesh(mesh,vertices,invalid_indices));
        BOOST_REQUIRE(renderer.Append_Mesh(mesh,vertices,indices));
        BOOST_REQUIRE(commands.Set_Viewport({column,0,1,8}));
        BOOST_REQUIRE(renderer.Draw_Range(commands,mesh,style,parameters,{},column*6,6));
    }
    std::array<std::byte,16*8*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    for (unsigned column=0;column<16;++column) for (unsigned channel=0;channel<3;++channel)
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*16+column)*4+channel])-(channel==column%2 ? 255 : 0),1);
    BOOST_REQUIRE(renderer.Destroy_Mesh(mesh));
    renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(extraction_workspaces_reset_defaults_and_isolate_nested_leases)
{
    PropExtractionCache cache;
    for (unsigned pass=0;pass<3;++pass) {
        auto outer = cache.Acquire();
        auto source = outer.Workspace().Prepare_Source(3);
        const auto defaults = std::bit_cast<std::array<std::uint32_t,sizeof(PropVertex)/sizeof(std::uint32_t)>>(PropVertex{});
        for (const auto& vertex : source)
            BOOST_CHECK((std::bit_cast<std::array<std::uint32_t,sizeof(PropVertex)/sizeof(std::uint32_t)>>(vertex.Make_Vertex()) == defaults));
        source[0].position = {1,2,3};
        source[0].normal = {0.5f,0,0.25f};
        auto& batch = outer.Workspace().Batch();
        BOOST_REQUIRE(batch.Begin(3,3));
        for (unsigned index=0;index<3;++index)
            BOOST_REQUIRE(batch.Append(index,[&](unsigned vertex) { return source[vertex].Make_Vertex(); }));
        {
            auto inner = cache.Acquire();
            auto other = inner.Workspace().Prepare_Source(1024);
            other[0].position = {4,5,6};
            BOOST_CHECK((source[0].position == std::array<float,3>{1,2,3}));
            BOOST_CHECK((batch.Vertices()[0].position == source[0].position));
            BOOST_CHECK((batch.Vertices()[0].normal == source[0].normal));
            if (pass == 1) cache.Clear();
        }
        BOOST_CHECK_EQUAL(batch.Indices().size(),3u);
        BOOST_CHECK((batch.Vertices()[0].position == source[0].position));
        std::vector<std::size_t> extracted;
        source = outer.Workspace().Prepare_Source(2,[&](PropSourceVertex& vertex,std::size_t index) {
            extracted.push_back(index);
            vertex.position={float(index+4),5,6};
            vertex.normal={0,0,1};
        });
        BOOST_CHECK((extracted == std::vector<std::size_t>{0,1}));
        BOOST_REQUIRE_EQUAL(source.size(),2u);
        BOOST_CHECK((source[0].position == std::array<float,3>{4,5,6}));
        BOOST_CHECK((source[1].position == std::array<float,3>{5,5,6}));
        for (const auto& vertex : source)
            BOOST_CHECK((vertex.normal == std::array<float,3>{0,0,1}));
        source = outer.Workspace().Prepare_Source(5,[](PropSourceVertex& vertex,std::size_t index) {
            vertex.position={float(index+1),6,7};
            vertex.normal={0.5f,0.25f,0};
        });
        BOOST_REQUIRE_EQUAL(source.size(),5u);
        BOOST_CHECK((source[4].position == std::array<float,3>{5,6,7}));
        BOOST_CHECK((source[4].normal == std::array<float,3>{0.5f,0.25f,0}));
        extracted.clear();
        source = outer.Workspace().Prepare_Source(0,[&](PropSourceVertex& vertex,std::size_t index) {
            extracted.push_back(index);
            vertex={};
        });
        BOOST_CHECK(source.empty());
        BOOST_CHECK(extracted.empty());
    }
    cache.Clear();
}

BOOST_AUTO_TEST_CASE(cloud_projection_uses_height_and_preserves_ambient_emissive_and_local_lights)
{
    struct ResetEnvironment final {
        ~ResetEnvironment() { Get_Environment_Lighting() = {}; }
    } reset;
    Get_Environment_Lighting() = {};
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<std::uint8_t,8> pattern{0,0,0,255,255,255,255,255};
    const auto cloud = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(pattern)),8});
    const std::array<std::uint8_t,8> reversed_pattern{255,255,255,255,0,0,0,255};
    const auto reversed_cloud = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(reversed_pattern)),8});
    auto& environment = Get_Environment_Lighting();
    environment.cloud_texture = cloud;
    environment.parameters.cloud_multiplier = {0,0,0.5f,0};
    environment.parameters.cloud_offset_strength = {0.5f,0.5f,1,1};
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    for (auto& vertex : vertices) {
        vertex.normal = {0,0,1};
        vertex.material_ambient = {1,1,1,1};
        vertex.material_diffuse = {1,1,1,1};
        vertex.material_emissive = {0.1f,0.1f,0.1f,0};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    auto mesh = renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,0,0.5f,0,0,0,1};
    parameters.textured = 0;
    parameters.camera_position = {0,0,10,1};
    parameters.scene_ambient = {0.2f,0.2f,0.2f,0};
    parameters.light_direction[0] = {0,0,1,1};
    parameters.light_diffuse[0] = {0.6f,0.6f,0.6f,0};
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    const auto draw_and_check = [&](int expected) {
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for (int channel=0;channel<3;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+channel])-expected,2);
        BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4+3]),255);
    };
    draw_and_check(77); // Black cloud: ambient and emissive remain.
    draw_and_check(77); // Unchanged environment parameters must remain on the GPU.
    environment.cloud_texture = reversed_cloud;
    draw_and_check(230); // A texture change must bind even with identical parameters.
    environment.cloud_texture = {};
    draw_and_check(230); // Missing clouds disable their effective strength.
    environment.cloud_texture = cloud;
    draw_and_check(77);
    environment.parameters.shadow_options[0] = 5;
    BOOST_CHECK(!renderer.Draw(commands,mesh,style,parameters,{}));
    environment.parameters.shadow_options[0] = 0;
    draw_and_check(77); // Rejected settings do not corrupt the last uploaded values.
    renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    mesh = renderer.Create_Mesh(vertices,indices);
    draw_and_check(77); // Recreated buffers need an upload even when values match.
    for (auto& vertex : vertices) vertex.position[2] = 1.5f;
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    draw_and_check(230); // Same XY, different height: the white cloud texel.
    environment.parameters.cloud_multiplier = {};
    environment.parameters.cloud_offset_strength[0] = 0.25f;
    parameters.light_position[0] = {0,0,1000000,1};
    parameters.light_attenuation[0] = {1,0,0,0};
    draw_and_check(230); // Local lighting does not receive sunlight's clouds.
    environment.parameters.clip_plane = {0,0,1,-2};
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
    std::array<std::byte,16*16*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4+2]),255);
    BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[(8*16+8)*4]),0);
    Get_Environment_Lighting() = {};
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {cloud,reversed_cloud,target,depth}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(shadow_depth_silhouette_attenuates_directional_light_on_receivers)
{
    struct ResetEnvironment final {
        ~ResetEnvironment() { Get_Environment_Lighting() = {}; }
    } reset;
    Get_Environment_Lighting() = {};
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    const auto shadow = device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil) |
        static_cast<std::uint32_t>(RHITextureUsage::ShaderResource)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.25f}; vertices[1].position = {0,-1,0.25f};
    vertices[2].position = {0,1,0.25f}; vertices[3].position = {-1,1,0.25f};
    for (auto& vertex : vertices) {
        vertex.normal = {0,0,1};
        vertex.material_ambient = {1,1,1,1};
        vertex.material_diffuse = {1,1,1,1};
        vertex.material_emissive = {0.1f,0.1f,0.1f,0};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured = 0;
    parameters.camera_position = {0,0,1000000,1};
    parameters.scene_ambient = {0.2f,0.2f,0.2f,0};
    parameters.light_direction[0] = {0,0,1,1};
    parameters.light_diffuse[0] = {0.6f,0.6f,0.6f,0};
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.color_write_mask = 0;
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Depth_Target(shadow));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
    BOOST_REQUIRE(commands.Clear_Depth(1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));

    // Draw a full receiver behind the left-half caster into another target.
    for (auto& vertex : vertices) vertex.position[2] = 0.75f;
    vertices[1].position[0] = vertices[2].position[0] = 1;
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    auto& environment = Get_Environment_Lighting();
    environment.shadow_textures[0] = shadow;
    environment.parameters.shadow_view_projection[0] = parameters.view_projection;
    environment.parameters.shadow_splits = {10,10,10,10};
    environment.parameters.shadow_view_depth = {0,0,1,0};
    environment.parameters.shadow_options = {1,0.0001f,0,0.1f};
    style.color_write_mask = 15;
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
    std::array<std::byte,32*32*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
    for (int channel=0;channel<3;++channel) {
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*32+8)*4+channel])-77,2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*32+24)*4+channel])-230,2);
        const int edge = std::to_integer<int>(pixels[(16*32+15)*4+channel]);
        BOOST_CHECK_GT(edge,77);
        BOOST_CHECK_LT(edge,230);
    }
    Get_Environment_Lighting() = {};
    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    for (const auto texture : {target,depth,shadow}) device.Destroy_Texture(texture);
}

BOOST_AUTO_TEST_CASE(detail_materials_shroud_depth_and_background_drawing)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto texture = [&](std::array<std::uint8_t,4> color) {
        return device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(color)),4});
    };
    const auto base = texture({128,64,32,255});
    const auto detail = texture({128,128,128,128});
    const auto shroud = texture({128,128,128,255});
    const auto target = device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    std::array<PropVertex,4> vertices{};
    vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
    vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh = renderer.Create_Mesh(vertices,indices);
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.secondary_texture = 1;
    PropStyle style;
    style.cull = RHICullMode::None;
    const std::array<RHITextureHandle,4> textures{base,detail,{},shroud};
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    const auto check = [&](std::array<int,4> expected) {
        std::array<std::byte,16*16*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,16*4));
        for(int c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8*16+8)*4+c])-expected[c],2);
    };
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    parameters.detail_color = 2;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({64,32,16,255});
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,true));
    check({32,16,8,255});
    parameters.detail_color = 4;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({255,192,160,255});
    parameters.detail_color = 6;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({0,64,96,255});
    parameters.detail_color = 11;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({128,64,32,255});
    parameters.detail_alpha = 2;
    parameters.alpha_cutoff = 0.75f;
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,true));
    check({0,0,255,255});
    parameters.alpha_cutoff = 0;
    style.depth_test = false; style.depth_write = false;
    BOOST_REQUIRE(commands.Clear({0,0,1,1},0));
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({128,64,32,128});
    // Interpolate normals before lighting. A center normal between opposite
    // diagonal directions must still face the light with full intensity.
    for (unsigned i=0;i<vertices.size();++i) {
        vertices[i].normal = {i==0 || i==3 ? -0.8f : 0.8f,0,0.6f};
        vertices[i].material_ambient = {1,1,1,1};
        vertices[i].material_diffuse = {1,1,1,0.4f};
    }
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    parameters = {};
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position = {0,0,10,1};
    parameters.textured = 0;
    parameters.light_direction[0] = {0,0,1,1};
    parameters.light_diffuse[0] = {0.5f,0.5f,0.5f,0};
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({127,127,127,102});
    // Emissive effects use material opacity even when their color channel
    // carries an alpha-vector mask. Zero diffuse/ambient must not blacken RGB.
    const auto lit_vertices=vertices;
    for (auto& vertex : vertices) {
        vertex.color={0.1f,0.8f,0.3f,0.8f};
        vertex.material_diffuse={0,0,0,0.25f};
        vertex.material_ambient={0,0,0,1};
        vertex.material_emissive={0.4f,0.2f,0.1f,0};
        vertex.material_specular={0,0,0,0};
    }
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({102,51,26,64});
    style.source_blend=RHIBlendFactor::SourceAlpha;
    style.destination_blend=RHIBlendFactor::InverseSourceAlpha;
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({26,13,198,207});
    style.source_blend=RHIBlendFactor::One;
    style.destination_blend=RHIBlendFactor::One;
    BOOST_REQUIRE(commands.Clear({0.1f,0.1f,0.1f,0},1));
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,textures,false));
    check({128,77,51,64});
    style.destination_blend=RHIBlendFactor::Zero;
    vertices=lit_vertices;
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    // A translated texture matrix must choose the other texel without changing
    // geometry. This covers scrolling/stepped material mapper uniforms.
    const std::array<std::uint8_t,8> texels{255,0,0,255,0,255,0,255};
    const auto mapped_texture = device.Create_Texture_Initialized({2,1},{std::as_bytes(std::span(texels)),8});
    BOOST_REQUIRE(mapped_texture.Is_Valid());
    parameters = {};
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.primary_gradient = 0;
    parameters.uv_transform[0][3] = 0.25f;
    style.samplers[0].Set_Filter(Graphics::RHISamplerFilter::Point);
    auto mapped_textures = textures;
    mapped_textures[0] = mapped_texture;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,mapped_textures,false));
    check({255,0,0,255});
    parameters.uv_transform[0][3] = 0.75f;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,mapped_textures,false));
    check({0,255,0,255});
    const std::array<std::uint8_t,16> grid{
        255,0,0,255, 0,255,0,255, 0,0,255,255, 255,255,255,255};
    const auto grid_texture=device.Create_Texture_Initialized({2,2},{std::as_bytes(std::span(grid)),8});
    BOOST_REQUIRE(grid_texture.Is_Valid());
    mapped_textures[0]=grid_texture;
    parameters.uv_transform[0][3]=1.25f;
    parameters.uv_transform[0][7]=1.25f;
    style.samplers[0].address[1]=RHISamplerAddress::Clamp;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,mapped_textures,false));
    check({0,0,255,255});
    style.samplers[0].address[0]=RHISamplerAddress::Clamp;
    style.samplers[0].address[1]=RHISamplerAddress::Wrap;
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,mapped_textures,false));
    check({0,255,0,255});
    for(auto& vertex:vertices) {
        const auto position=vertex.position;
        vertex=PropVertex{}; vertex.position=position;
    }
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    parameters=PropParameters{}; parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    style.cull=RHICullMode::Back; style.front_counter_clockwise=true;
    BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,{},false));
    check({255,255,255,255});
    parameters.view_projection[0]=-1;
    style.front_counter_clockwise=false;
    BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
    BOOST_REQUIRE(Draw_Prop(renderer,commands,mesh,style,parameters,{},false));
    check({255,255,255,255});
    // Screen material luminance ignores RGB tint but retains modulated alpha.
    for (auto& vertex : vertices) vertex.color={0.2f,0.7f,0.1f,0.5f};
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    parameters=PropParameters{};
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.texture_luminance=1;
    style=PropStyle{};
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::span(textures).first(1)));
    check({80,80,80,128});
    // Decal passes inherit a negative depth bias so coplanar geometry survives
    // a strict depth comparison. A positive bias must remain occluded.
    for (auto& vertex : vertices) vertex.color={1,0,0,1};
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    parameters=PropParameters{}; parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    style=PropStyle{}; style.cull=RHICullMode::None;
    style.depth_comparison=RHIComparison::Less;
    BOOST_REQUIRE(commands.Clear({0,0,1,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
    check({0,0,255,255});
    style.depth_bias=-16;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
    check({255,0,0,255});
    BOOST_REQUIRE(commands.Clear({0,0,1,1},0.5f));
    style.depth_bias=16;
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
    check({0,0,255,255});
    // Wireframe must leave triangle interiors untouched while drawing edges.
    style.depth_bias=0; style.depth_test=false; style.depth_write=false;
    style.wireframe=true;
    BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
    std::array<std::byte,16*16*4> wire_pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,wire_pixels,16*4));
    BOOST_CHECK_EQUAL(std::to_integer<int>(wire_pixels[(4*16+8)*4]),0);
    unsigned edge_pixels=0;
    for (unsigned i=0;i<16*16;++i)
        if (std::to_integer<int>(wire_pixels[i*4])>0) ++edge_pixels;
    BOOST_CHECK_GT(edge_pixels,0u);
    BOOST_CHECK_LT(edge_pixels,128u);
    device.Destroy_Texture(grid_texture);
    device.Destroy_Texture(mapped_texture);
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    for (auto handle : {base,detail,shroud,target,depth}) device.Destroy_Texture(handle);
}

BOOST_AUTO_TEST_CASE(indexed_batches_preserve_seams_material_changes_and_blended_pixels)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({16,16,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({16,16,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,16,16}));
    std::array<PropVertex,6> source{};
    source[0].position={-1,-1,0.5f}; source[1].position={1,-1,0.5f};
    source[2].position={1,1,0.5f}; source[3].position={-1,1,0.5f};
    source[4]=source[0]; source[5]=source[2];
    source[4].color={0,0,1,0.5f}; source[5].color={0,1,0,0.5f};
    source[4].uv={1,0}; source[5].uv={0,1};
    source[4].normal={0.5f,0,0.75f}; source[5].normal={0,0.5f,0.75f};
    // Shared indices and distinct vertices at the same position coexist.
    const std::array<std::uint32_t,12> triangles{0,1,2,0,2,3,4,5,3,0,1,2};
    PropExtractionCache extraction_cache;
    PropParameters parameters;
    parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.depth_write=false;
    for (unsigned material=0;material<2;++material) {
        auto workspace = extraction_cache.Acquire();
        const auto compact_source = workspace.Workspace().Prepare_Source(source.size(),[&](PropSourceVertex& vertex,std::size_t index) {
            vertex.position=source[index].position;
            if (material == 0) vertex.normal=source[index].normal;
            else vertex.normal={0,0,1};
        });
        auto& batch = workspace.Workspace().Batch();
        // An underestimated reservation must still grow without losing any
        // triangles, vertex seams, or material changes.
        BOOST_REQUIRE(batch.Begin(source.size(),material == 0 ? triangles.size() : 3));
        std::array<unsigned,6> extractions{};
        const auto extract=[&](unsigned index) {
            ++extractions[index];
            auto vertex=compact_source[index].Make_Vertex();
            vertex.color=source[index].color;
            vertex.uv=source[index].uv;
            vertex.material_diffuse={1,material==0 ? 0.3f : 0.8f,0.5f,0.4f};
            return vertex;
        };
        for (const auto index : triangles) BOOST_REQUIRE(batch.Append(index,extract));
        BOOST_CHECK_EQUAL(batch.Vertices().size(),6u);
        BOOST_CHECK_EQUAL(batch.Indices().size(),triangles.size());
        for (const auto count : extractions) BOOST_CHECK_EQUAL(count,1u);
        BOOST_CHECK(!batch.Append(6,extract));
        std::array<PropVertex,12> expanded{};
        std::array<std::uint32_t,12> sequential{};
        for (unsigned i=0;i<triangles.size();++i) {
            expanded[i]=source[triangles[i]];
            if (material == 1) expanded[i].normal={0,0,1};
            expanded[i].material_diffuse={1,material==0 ? 0.3f : 0.8f,0.5f,0.4f};
            sequential[i]=i;
            BOOST_CHECK((std::bit_cast<std::array<std::uint32_t,sizeof(PropVertex)/sizeof(std::uint32_t)>>(batch.Vertices()[batch.Indices()[i]])
                == std::bit_cast<std::array<std::uint32_t,sizeof(PropVertex)/sizeof(std::uint32_t)>>(expanded[i])));
            BOOST_CHECK(batch.Vertices()[batch.Indices()[i]].uv == expanded[i].uv);
            BOOST_CHECK(batch.Vertices()[batch.Indices()[i]].color == expanded[i].color);
        }
        const auto compact=renderer.Create_Mesh(batch.Vertices(),batch.Indices());
        const auto reference=renderer.Create_Mesh(expanded,sequential);
        std::array<std::byte,16*16*4> actual{},expected{};
        BOOST_REQUIRE(commands.Clear({0.2f,0.1f,0.3f,0.2f},1));
        BOOST_REQUIRE(renderer.Draw(commands,reference,style,parameters,{}));
        BOOST_REQUIRE(device.Readback_Texture(target,expected,16*4));
        BOOST_REQUIRE(commands.Clear({0.2f,0.1f,0.3f,0.2f},1));
        BOOST_REQUIRE(renderer.Draw(commands,compact,style,parameters,{}));
        BOOST_REQUIRE(device.Readback_Texture(target,actual,16*4));
        BOOST_CHECK(actual == expected);
        renderer.Destroy_Mesh(compact); renderer.Destroy_Mesh(reference);
    }
    renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(material_color_sources_preserve_lit_and_prelit_drawing)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    PropParameters parameters;
    parameters.textured=0;
    parameters.scene_ambient={1,1,1,0};
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style; style.blend=RHIBlendMode::Disabled;
    std::array<PropVertex,4> source{};
    source[0].position={-1,-1,0.5f}; source[1].position={1,-1,0.5f};
    source[2].position={1,1,0.5f}; source[3].position={-1,1,0.5f};
    for (auto& vertex : source) {
        vertex.color={0.1f,0.2f,0.3f,0.4f};
        vertex.secondary_color={0.3f,0.1f,0.2f,0.7f};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const std::array sources{PropColorSource::Material,PropColorSource::PrimaryColor,PropColorSource::SecondaryColor};
    for (bool lit : {false,true}) for (auto diffuse : sources)
        for (auto ambient : sources) for (auto emissive : sources) {
        PropMaterial material;
        material.lighting=lit; material.diffuse_source=diffuse;
        material.ambient_source=ambient; material.emissive_source=emissive;
        material.diffuse={0.6f,0.5f,0.4f}; material.ambient={0.05f,0.07f,0.09f};
        material.emissive={0.08f,0.06f,0.04f}; material.opacity=0.6f;
        material.specular={0.2f,0.3f,0.4f}; material.shininess=19;
        auto vertices=source;
        for (auto& vertex : vertices) Apply_Prop_Material(vertex,material);
        BOOST_CHECK(vertices[0].color == source[0].color);
        BOOST_CHECK(vertices[0].position == source[0].position);
        BOOST_CHECK_EQUAL(vertices[0].material_specular[3],19);
        const auto select=[&](PropColorSource choice,const std::array<float,3>& value,unsigned channel) {
            return choice==PropColorSource::Material ? value[channel]
                : choice==PropColorSource::PrimaryColor ? source[0].color[channel] : source[0].secondary_color[channel];
        };
        std::array<float,4> expected{};
        for (unsigned channel=0;channel<3;++channel) {
            BOOST_CHECK_EQUAL(vertices[0].material_diffuse[channel],lit ? select(diffuse,material.diffuse,channel) : material.diffuse[channel]);
            expected[channel]=lit ? select(ambient,material.ambient,channel)+select(emissive,material.emissive,channel)
                : source[0].color[channel]*material.diffuse[channel];
        }
        expected[3]=lit ? diffuse==PropColorSource::PrimaryColor ? 0.4f
            : diffuse==PropColorSource::SecondaryColor ? 0.7f : 0.6f : 0.4f*0.6f;
        const auto mesh=renderer.Create_Mesh(vertices,indices);
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(float(std::to_integer<int>(pixels[(4*8+4)*4+channel]))-expected[channel]*255,1.1f);
        renderer.Destroy_Mesh(mesh);
    }
    renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(prepared_mesh_inputs_use_owned_contents_and_channel_boundaries)
{
    PropRenderer renderer;
    PropMeshSet meshes;
    std::array<PropVertex,3> vertices{};
    const std::array<std::uint32_t,3> indices{0,1,2};
    std::array<std::byte,4> input{std::byte{1},std::byte{2},std::byte{3},std::byte{4}};
    const auto bytes = std::span<const std::byte>(input);
    std::array sources{bytes.first(2),bytes.subspan(2)};
    const auto mesh = meshes.Publish_Prepared(renderer,0,sources,vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    BOOST_CHECK(meshes.Find_Prepared(renderer,0,sources) == mesh);
    // Equal bytes at a different address remain a hit.
    auto copy = input;
    const auto copied = std::span<const std::byte>(copy);
    BOOST_CHECK(meshes.Find_Prepared(renderer,0,std::array{copied.first(2),copied.subspan(2)}) == mesh);
    BOOST_CHECK(!meshes.Find_Prepared(renderer,1,sources).Is_Valid());
    BOOST_CHECK(!meshes.Find_Prepared(renderer,0,std::array{bytes.first(1),bytes.subspan(1)}).Is_Valid());
    for (auto& value : input) {
        const auto saved = value;
        value = std::byte{9};
        BOOST_CHECK(!meshes.Find_Prepared(renderer,0,sources).Is_Valid());
        value = saved;
        BOOST_CHECK(meshes.Find_Prepared(renderer,0,sources) == mesh);
    }
    // Publishing through the direct path invalidates the preparation snapshot.
    vertices[0].color = {0,1,0,1};
    BOOST_REQUIRE(meshes.Synchronize(renderer,0,vertices,indices).Is_Valid());
    BOOST_CHECK(!meshes.Find_Prepared(renderer,0,sources).Is_Valid());
    const std::span<const std::byte> empty;
    const std::array with_empty{bytes,empty};
    const auto empty_channel_mesh=meshes.Publish_Prepared(renderer,0,with_empty,vertices,indices);
    BOOST_REQUIRE(empty_channel_mesh.Is_Valid());
    BOOST_CHECK(meshes.Find_Prepared(renderer,0,with_empty)==empty_channel_mesh);
    BOOST_CHECK(!meshes.Find_Prepared(renderer,0,std::array{empty,bytes}).Is_Valid());
    const std::array empty_channels{empty,empty};
    const auto empty_mesh=meshes.Publish_Prepared(renderer,0,empty_channels,vertices,indices);
    BOOST_REQUIRE(empty_mesh.Is_Valid());
    BOOST_CHECK(meshes.Find_Prepared(renderer,0,empty_channels)==empty_mesh);
    BOOST_CHECK(!meshes.Find_Prepared(renderer,0,std::array{empty}).Is_Valid());
    BOOST_CHECK(!meshes.Find_Prepared(renderer,0,{}).Is_Valid());
    meshes.Clear();
}

BOOST_AUTO_TEST_CASE(instance_prepared_meshes_retain_distinct_poses_through_deferred_drawing)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    PropParameters parameters;
    parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.blend=RHIBlendMode::Disabled;
    style.cull=RHICullMode::None;
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const std::array<std::array<float,2>,3> extents{{{-1,-0.4f},{-0.3f,0.3f},{0.4f,1}}};
    const std::array<std::array<float,4>,3> colors{{{1,0,0,1},{0,1,0,1},{0,0,1,1}}};
    std::array<PropMeshSet,3> owners;
    std::array<PropMeshHandle,3> handles;
    std::array<std::array<PropVertex,4>,3> poses;
    std::array<PropVertex,4> scratch;
    for (std::size_t instance=0;instance<owners.size();++instance) {
        const auto [left,right]=extents[instance];
        scratch[0].position={left,-1,0.5f}; scratch[1].position={right,-1,0.5f};
        scratch[2].position={right,1,0.5f}; scratch[3].position={left,1,0.5f};
        for (auto& vertex : scratch) vertex.color=colors[instance];
        poses[instance]=scratch;
        const std::array sources{std::as_bytes(std::span<const PropVertex>(scratch))};
        handles[instance]=owners[instance].Publish_Prepared(renderer,0,sources,scratch,indices);
        BOOST_REQUIRE(handles[instance].Is_Valid());
    }
    MaterialPassQueue queue;
    for (std::size_t instance=0;instance<owners.size();++instance) {
        // The shared extraction scratch now contains the last instance's pose.
        // A second view of each instance must still find its own saved version.
        const std::array sources{std::as_bytes(std::span<const PropVertex>(poses[instance]))};
        BOOST_REQUIRE(owners[instance].Find_Prepared(renderer,0,sources)==handles[instance]);
        BOOST_REQUIRE(queue.Submit(renderer,handles[instance],style,parameters,{}));
        owners[instance].Clear();
        BOOST_CHECK(renderer.Mesh_Geometry(handles[instance])!=nullptr);
    }
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(queue.Flush(commands));
    std::array<std::byte,8*8*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
    const std::array<unsigned,3> columns{1,4,6};
    for (unsigned instance=0;instance<3;++instance)
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(float(std::to_integer<int>(pixels[(4*8+columns[instance])*4+channel]))
                - colors[instance][channel]*255,1.1f);
    for (const auto handle : handles) BOOST_CHECK(renderer.Mesh_Geometry(handle)==nullptr);
    renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(material_edits_update_persistent_mesh_rgb_and_opacity)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    PropParameters parameters;
    parameters.textured=0; parameters.scene_ambient={1,1,1,0};
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style; style.blend=RHIBlendMode::Disabled;
    std::array<PropVertex,4> source{};
    source[0].position={-1,-1,0.5f}; source[1].position={1,-1,0.5f};
    source[2].position={1,1,0.5f}; source[3].position={-1,1,0.5f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(source,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropMaterial material;
    material.diffuse={0.25f,0.5f,0.75f}; material.opacity=0.4f;
    material.shininess=0;
    const auto check=[&](std::array<int,4> expected) {
        auto vertices=source;
        for (auto& vertex : vertices) Apply_Prop_Material(vertex,material);
        BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+4)*4+channel])-expected[channel],2);
    };
    check({64,128,191,102});
    material.lighting=true; material.ambient={0.2f,0.3f,0.4f};
    material.emissive={0.1f,0.1f,0.1f}; material.opacity=0.7f;
    check({77,102,128,179});
    material.ambient_source=PropColorSource::PrimaryColor;
    material.diffuse_source=PropColorSource::PrimaryColor;
    material.emissive={0,0,0};
    for (auto& vertex : source) vertex.color={0.6f,0.2f,0.4f,0.25f};
    check({153,51,102,64});
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}


BOOST_AUTO_TEST_CASE(preparation_versions_track_alias_writes_and_retained_pointer_edits)
{
    SourceRevision revision;
    SourceRevision alias = revision;
    const auto initial = revision.Token();
    BOOST_REQUIRE_NE(initial,0u);
    PropRenderer renderer;
    PropMeshSet meshes;
    std::array<PropVertex,3> vertices{};
    const std::array<std::uint32_t,3> indices{0,1,2};
    std::array<unsigned,4> channel{1,2,3,4};
    const auto bytes = std::as_bytes(std::span(channel));
    std::array<PropPreparationInput,1> inputs{{{bytes.first(8),revision.Token()}}};
    const auto first = meshes.Publish_Versioned(renderer,0,inputs,vertices,indices);
    BOOST_REQUIRE(first.Is_Valid());
    BOOST_CHECK(meshes.Find_Versioned(renderer,0,inputs)==first);
    inputs[0].bytes = bytes.subspan(8);
    BOOST_CHECK(!meshes.Find_Versioned(renderer,0,inputs).Is_Valid());
    inputs[0].bytes = bytes.first(4);
    BOOST_CHECK(!meshes.Find_Versioned(renderer,0,inputs).Is_Valid());
    inputs[0].bytes = bytes.first(8);
    alias.Invalidate();
    BOOST_CHECK_NE(revision.Token(),initial);
    BOOST_CHECK_EQUAL(alias.Token(),revision.Token());
    inputs[0].revision = revision.Token();
    BOOST_CHECK(!meshes.Find_Versioned(renderer,0,inputs).Is_Valid());
    BOOST_REQUIRE(meshes.Publish_Versioned(renderer,0,inputs,vertices,indices).Is_Valid());

    // Mutable access after publication invalidates every shared reader. The
    // pointer remains usable after a new raw-content snapshot is published.
    alias.Expose_Writable();
    auto* retained_pointer = channel.data();
    inputs[0].revision = revision.Token();
    BOOST_CHECK_EQUAL(inputs[0].revision,0u);
    BOOST_CHECK(!meshes.Find_Versioned(renderer,0,inputs).Is_Valid());
    BOOST_REQUIRE(meshes.Publish_Versioned(renderer,0,inputs,vertices,indices).Is_Valid());
    retained_pointer[1] = 9;
    BOOST_CHECK(!meshes.Find_Versioned(renderer,0,inputs).Is_Valid());
    retained_pointer[1] = 2;
    BOOST_CHECK(meshes.Find_Versioned(renderer,0,inputs).Is_Valid());
    alias.Invalidate();
    BOOST_CHECK_EQUAL(revision.Token(),0u);
    revision.Reset();
    BOOST_CHECK_NE(revision.Token(),0u);
    BOOST_CHECK_NE(revision.Token(),initial);
    BOOST_CHECK_EQUAL(alias.Token(),0u);
}

BOOST_AUTO_TEST_CASE(versioned_meshes_preserve_queued_geometry_color_and_alpha_after_recreation)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    const auto shaders=Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    PropParameters parameters;
    parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.blend=RHIBlendMode::Disabled;
    style.cull=RHICullMode::None;
    std::array<PropVertex,4> vertices{};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    SourceRevision revision;
    SourceRevision alias=revision;
    PropMeshSet meshes;
    MaterialPassQueue queue;
    const auto prepare=[&](float left,float right,std::array<float,4> color) {
        alias.Invalidate();
        vertices[0].position={left,-1,0.5f}; vertices[1].position={right,-1,0.5f};
        vertices[2].position={right,1,0.5f}; vertices[3].position={left,1,0.5f};
        for (auto& vertex:vertices) vertex.color=color;
        const std::array<PropPreparationInput,2> sources{{
            {std::as_bytes(std::span(vertices)),revision.Token()},
            {std::as_bytes(std::span(indices))}}};
        BOOST_CHECK(!meshes.Find_Versioned(renderer,0,sources).Is_Valid());
        const auto handle=meshes.Publish_Versioned(renderer,0,sources,vertices,indices);
        BOOST_REQUIRE(handle.Is_Valid());
        BOOST_CHECK(meshes.Find_Versioned(renderer,0,sources)==handle);
        BOOST_REQUIRE(queue.Submit(renderer,handle,style,parameters,{}));
        return handle;
    };
    const auto first=prepare(-1,-0.25f,{1,0,0,0.25f});
    // Upload the first version before replacement so recreation also covers
    // a previously resident buffer and a new version pending its first draw.
    BOOST_REQUIRE(renderer.Draw(commands,first,style,parameters,{}));
    const auto second=prepare(0.25f,1,{0,1,0,0.75f});
    BOOST_CHECK(first!=second);
    meshes.Clear();
    BOOST_CHECK(renderer.Mesh_Geometry(first)!=nullptr);
    BOOST_CHECK(renderer.Mesh_Geometry(second)!=nullptr);
    renderer.Shutdown();
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
    BOOST_REQUIRE(queue.Flush(commands));
    std::array<std::byte,8*8*4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,8*4));
    const auto check=[&](unsigned column,std::array<int,4> expected) {
        for (unsigned channel=0;channel<4;++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(4*8+column)*4+channel])-expected[channel],2);
    };
    check(1,{255,0,0,64});
    check(6,{0,255,0,191});
    check(4,{0,0,0,0});
    BOOST_CHECK(renderer.Mesh_Geometry(first)==nullptr);
    BOOST_CHECK(renderer.Mesh_Geometry(second)==nullptr);
    renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
