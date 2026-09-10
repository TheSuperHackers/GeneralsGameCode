module;
#define BOOST_TEST_MODULE PropAssetBindingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <algorithm>
#include <cstddef>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
export module Graphics.Scene.Props.AssetBinding.Tests;
import Graphics.Scene.Props.AssetBinding;
import Graphics.Tests.Device;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Assets.Adapters.W3D;
import Assets.Cache;
import Assets.Identity;
import Assets.Models;
import Video.Capture.ImageWriter;
using namespace Graphics;

namespace {
std::vector<std::byte> Read(const std::filesystem::path& path) {
    std::ifstream input(path,std::ios::binary|std::ios::ate);
    if(!input) return {};
    const auto size=input.tellg(); input.seekg(0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    if(!input.read(reinterpret_cast<char*>(bytes.data()),size)) return {};
    return bytes;
}
}

BOOST_AUTO_TEST_CASE(automatically_adapted_native_models_render_with_original_rigs)
{
    const char* directory=std::getenv("GENERALS_W3D_ADAPTED_DIRECTORY");
    const char* textures=std::getenv("GENERALS_W3D_SURFACE_TEXTURES");
    const char* legacy=std::getenv("GENERALS_W3D_LEGACY_TEXTURES");
    if(!directory || !textures) { BOOST_TEST_MESSAGE("Set adapted model directory for batch drawing");return; }
    auto upper=[](std::string value) { for(auto& c:value)c=static_cast<char>(std::toupper(static_cast<unsigned char>(c)));return value; };
    GraphicsTestDevice device({true});PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    constexpr unsigned extent=512;
    const auto target=device.Create_Texture({extent,extent,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({extent,extent,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,extent,extent}));
    unsigned rendered=0;
    std::vector<std::pair<std::string,std::string>> unsupported;
    for(const auto& entry:std::filesystem::recursive_directory_iterator(directory)) {
        const auto& path=entry.path();
        if(!entry.is_regular_file() || upper(path.extension().string())!=".W3D"
            || upper(path.stem().string())!=upper(path.parent_path().filename().string()))continue;
        BOOST_TEST_CONTEXT(path.filename().string()) {
            const auto bytes=Read(path);
            Assets::AssetCache assets([&](const Assets::AssetIdentity& identity) {
                if(identity.type==Assets::AssetType::Model)return bytes;
                auto texture_path=std::filesystem::path(textures)/identity.canonical_name;
                auto data=Read(texture_path);
                if(data.empty()) {texture_path.replace_extension(".dds");data=Read(texture_path);}
                if(data.empty() && legacy) {
                    texture_path=std::filesystem::path(legacy)/identity.canonical_name;
                    texture_path.replace_extension(".dds");data=Read(texture_path);
                }
                return data;
            });
            BOOST_REQUIRE(assets.Register_Model_Adapter(std::make_shared<Assets::W3DAdapter>()));
            const auto handle=assets.Request_Model(path.filename().string());assets.Wait(handle);
            const auto* model=assets.Try_Get_Model(handle);
            if(!model && assets.Get_Error(handle).find("W3D shader 'DefaultW3D.fx': unsupported shader program")!=std::string::npos) {
                // Staging can contain programs whose runtime translator is still
                // pending. Assert explicit rejection and report it separately.
                unsupported.emplace_back(path.filename().string(),assets.Get_Error(handle));
                continue;
            }
            BOOST_REQUIRE_MESSAGE(model,assets.Get_Error(handle));
            BOOST_REQUIRE_EQUAL(model->Skin_Bone_Count(),0u);
            std::string error;ModelAssetPose pose;
            BOOST_REQUIRE_MESSAGE(pose.Initialize(model->Rig(),error),error);
            PropAssetBinding binding;BOOST_REQUIRE_MESSAGE(binding.Load(device,renderer,assets,handle,error),error);
            std::array<float,3> low{1e30f,1e30f,1e30f},high{-1e30f,-1e30f,-1e30f};
            for(const auto& part:model->Submeshes()) {
                const auto label=upper(part.name);
                if(label.find("FX")!=std::string::npos || label.find("HEADLIGHT")!=std::string::npos)continue;
                RenderTransform transform;
                for(const auto& attachment:model->Rig().attachments) {
                    const auto dot=attachment.object_name.find('.');
                    if(attachment.object_name.substr(dot==std::string::npos ? 0 : dot+1)==part.name) {
                        BOOST_REQUIRE(pose.Bone_Transform(attachment.bone,transform));break;
                    }
                }
                for(auto index:model->Indices().subspan(part.first_index,part.index_count)) {
                    const auto p=model->Vertices()[index].position;
                    for(unsigned axis=0;axis<3;++axis) {
                        const auto& m=transform.matrix;
                        const auto value=m[axis*4]*p.x+m[axis*4+1]*p.y+m[axis*4+2]*p.z+m[axis*4+3];
                        low[axis]=(std::min)(low[axis],value);high[axis]=(std::max)(high[axis],value);
                    }
                }
            }
            const float span=(std::max)({high[0]-low[0],high[1]-low[1],high[2]-low[2]});
            BOOST_REQUIRE(span>0);const float s=1.2f/span;
            const float x=(low[0]+high[0])*.5f,y=(low[1]+high[1])*.5f,z=(low[2]+high[2])*.5f;
            PropParameters parameters;
            parameters.view_projection={.7f*s,.7f*s,0,-.7f*s*(x+y),-.35f*s,.35f*s,.8f*s,s*(.35f*x-.35f*y-.8f*z),-.2f*s,.2f*s,-.2f*s,.5f+s*(.2f*x-.2f*y+.2f*z),0,0,0,1};
            parameters.camera_position={300,-450,600,1};parameters.scene_ambient={.7f,.75f,.8f,1};
            parameters.light_direction[0]={.3f,-.5f,.8f,1};parameters.light_diffuse[0]={1,1,1,1};parameters.light_specular[0]={.4f,.4f,.4f,1};
            BOOST_REQUIRE(commands.Clear({.02f,.02f,.02f,1},1));
            for(std::size_t part=0;part<binding.Part_Count();++part)BOOST_REQUIRE(binding.Draw_Part(commands,part,parameters,&pose));
            std::vector<std::byte> pixels(extent*extent*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,extent*4));
            std::size_t visible=0;for(std::size_t i=0;i<pixels.size();i+=4)if(std::to_integer<unsigned>(pixels[i])>25)++visible;
            BOOST_REQUIRE_MESSAGE(visible>600u,"No substantial model image: "<<visible<<" pixels");
            if(const char* preview=std::getenv("GENERALS_W3D_ADAPTED_PREVIEW")) {
                Engine::Video::DecodedVideoFrame frame;frame.width=extent;frame.height=extent;frame.row_pitch=extent*4;
                frame.format=Engine::Video::PixelFormat::RGBA8;frame.pixels=std::move(pixels);
                BOOST_REQUIRE(Engine::Video::Write_Frame_Image(std::filesystem::path(preview)/(path.stem().string()+".png"),frame));
            }
            ++rendered;
        }
    }
    BOOST_REQUIRE(rendered>=6u);
    if(const char* preview=std::getenv("GENERALS_W3D_ADAPTED_PREVIEW")) {
        std::ofstream report(std::filesystem::path(preview)/"validation.json");
        report<<"{\"rendered\":"<<rendered<<",\"unsupported\":[";
        for(std::size_t i=0;i<unsupported.size();++i) {
            if(i)report<<',';
            report<<"{\"file\":"<<std::quoted(unsupported[i].first)<<",\"error\":"<<std::quoted(unsupported[i].second)<<'}';
        }
        report<<"],\"whole_game_compatible\":false}";
        BOOST_REQUIRE(report.good());
    }
    renderer.Shutdown();device.Destroy_Texture(target);device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(weighted_surface_pose_renders_and_survives_deferred_owner_release)
{
    const char* source=std::getenv("GENERALS_W3D_SKIN_RENDER_ASSET");
    const char* textures=std::getenv("GENERALS_W3D_SURFACE_TEXTURES");
    if(!source || !textures) { BOOST_TEST_MESSAGE("Set weighted surface fixture paths"); return; }
    const auto bytes=Read(source);
    Assets::AssetCache assets([&](const Assets::AssetIdentity& identity) {
        return identity.type==Assets::AssetType::Model ? bytes : Read(std::filesystem::path(textures)/identity.canonical_name);
    });
    BOOST_REQUIRE(assets.Register_Model_Adapter(std::make_shared<Assets::W3DAdapter>()));
    const auto handle=assets.Request_Model("weighted_surface.w3d");assets.Wait(handle);
    const auto* model=assets.Try_Get_Model(handle);BOOST_REQUIRE_MESSAGE(model,assets.Get_Error(handle));
    BOOST_REQUIRE(model->Skin_Bone_Count()>0);
    std::string error;ModelAssetPose pose;BOOST_REQUIRE_MESSAGE(pose.Initialize(model->Rig(),error),error);
    BOOST_REQUIRE(!model->Rig().animations.empty());
    GraphicsTestDevice device({true});PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    PropAssetBinding binding;BOOST_REQUIRE_MESSAGE(binding.Load(device,renderer,assets,handle,error),error);
    constexpr unsigned extent=512;
    const auto target=device.Create_Texture({extent,extent,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({extent,extent,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    PropParameters parameters;
    parameters.view_projection={.006f,.004f,0,0,-.003f,.0045f,.006f,-.15f,-.0005f,.00075f,-.001f,.5f,0,0,0,1};
    parameters.camera_position={300,-450,600,1};parameters.scene_ambient={.7f,.75f,.8f,1};
    parameters.light_direction[0]={.3f,-.5f,.8f,1};parameters.light_diffuse[0]={1,1,1,1};parameters.light_specular[0]={.4f,.4f,.4f,1};
    auto& commands=device.Immediate_Command_List();BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,extent,extent}));
    const float last=static_cast<float>(model->Rig().animations[0].frame_count-1);
    std::array<std::vector<std::byte>,2> images;
    for(unsigned sample=0;sample<2;++sample) {
        BOOST_REQUIRE(pose.Evaluate(0,sample ? last : 0));BOOST_REQUIRE(commands.Clear({.02f,.02f,.02f,1},1));
        for(std::size_t part=0;part<binding.Part_Count();++part) BOOST_REQUIRE(binding.Draw_Part(commands,part,parameters,&pose));
        images[sample].resize(extent*extent*4);BOOST_REQUIRE(device.Readback_Texture(target,images[sample],extent*4));
        if(const char* preview=std::getenv("GENERALS_W3D_SKIN_PREVIEW")) {
            Engine::Video::DecodedVideoFrame frame;frame.width=extent;frame.height=extent;frame.row_pitch=extent*4;
            frame.format=Engine::Video::PixelFormat::RGBA8;frame.pixels=images[sample];
            BOOST_REQUIRE(Engine::Video::Write_Frame_Image(std::filesystem::path(preview)/(sample ? "skin_finished.png" : "skin_start.png"),frame));
        }
    }
    std::size_t visible=0,changed=0;
    for(std::size_t i=0;i<images[0].size();i+=4) {
        if(std::to_integer<unsigned>(images[1][i])>25) ++visible;
        if(images[0][i]!=images[1][i] || images[0][i+1]!=images[1][i+1]) ++changed;
    }
    BOOST_TEST(visible>2000u);BOOST_TEST(changed>1000u);
    DirectionalShadowRenderer shadows;PropSubmission submission;submission.Initialize(device,renderer,shadows);
    BOOST_REQUIRE(commands.Clear({.02f,.02f,.02f,1},1));
    for(std::size_t part=0;part<binding.Part_Count();++part)
        BOOST_REQUIRE(binding.Submit_Part(submission,part,parameters,PropDrawPhase::Material,{},&pose));
    BOOST_REQUIRE(pose.Evaluate(0,0));binding.Clear();
    BOOST_REQUIRE(submission.Flush_Transparent());
    std::vector<std::byte> captured(extent*extent*4);BOOST_REQUIRE(device.Readback_Texture(target,captured,extent*4));
    BOOST_TEST(std::equal(captured.begin(),captured.end(),images[1].begin()));
    submission.Shutdown();renderer.Shutdown();device.Destroy_Texture(target);device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(converted_airfield_renders_through_typed_asset_and_prop_bindings)
{
    const char* source=std::getenv("GENERALS_W3D_SURFACE_ASSET");
    const char* textures=std::getenv("GENERALS_W3D_SURFACE_TEXTURES");
    if(!source || !textures) { BOOST_TEST_MESSAGE("Set surface asset/texture paths for the converted airfield draw"); return; }
    const auto bytes=Read(source); BOOST_REQUIRE(!bytes.empty());
    Assets::AssetCache assets([&](const Assets::AssetIdentity& identity) {
        return identity.type==Assets::AssetType::Model ? bytes : Read(std::filesystem::path(textures)/identity.canonical_name);
    });
    BOOST_REQUIRE(assets.Register_Model_Adapter(std::make_shared<Assets::W3DAdapter>()));
    const auto handle=assets.Request_Model("airfield.w3d"); assets.Wait(handle);
    BOOST_REQUIRE_MESSAGE(assets.Try_Get_Model(handle)!=nullptr,assets.Get_Error(handle));
    GraphicsTestDevice device({true}); PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    PropAssetBinding binding; std::string error;
    BOOST_REQUIRE_MESSAGE(binding.Load(device,renderer,assets,handle,error),error);
    BOOST_TEST(binding.Part_Count()==1u); BOOST_TEST(binding.Texture_Count()==3u);
    BOOST_TEST(binding.Part_Name(0)=="USA_AIRFIELD");
    BOOST_TEST(!binding.Load(device,renderer,assets,{},error));
    BOOST_TEST(binding.Part_Count()==1u);
    constexpr unsigned width=1024,height=768;
    const auto target=device.Create_Texture({width,height,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({width,height,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    PropParameters parameters;
    const auto& bounds=assets.Try_Get_Model(handle)->Bounds();
    const std::array center{(bounds.minimum.x+bounds.maximum.x)*.5f,
        (bounds.minimum.y+bounds.maximum.y)*.5f,(bounds.minimum.z+bounds.maximum.z)*.5f};
    const std::array<float,3> right{.789352f,.613941f,0},up{-.31000f,.39857f,.863174f},out{.529929f,-.681337f,.504f};
    const auto dot=[&](auto v) { return v[0]*center[0]+v[1]*center[1]+v[2]*center[2]; };
    parameters.view_projection={right[0]*2/430,right[1]*2/430,0,-dot(right)*2/430,
        up[0]*2/322.5f,up[1]*2/322.5f,up[2]*2/322.5f,-dot(up)*2/322.5f,
        -out[0]/800,-out[1]/800,-out[2]/800,.5f+dot(out)/800,0,0,0,1};
    parameters.camera_position={center[0]+out[0]*600,center[1]+out[1]*600,center[2]+out[2]*600,1};
    parameters.scene_ambient={.7f,.75f,.82f,1};
    parameters.light_direction[0]={-.3f,-.5f,.8124f,1};
    parameters.light_diffuse[0]={.9f,.85f,.8f,1};
    parameters.light_specular[0]={.6f,.6f,.6f,1};
    parameters.surface.team_color={.15f,.35f,.9f,1};
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,width,height}));
    BOOST_REQUIRE(commands.Clear({.06f,.07f,.09f,1},1));
    BOOST_REQUIRE(binding.Draw_Part(commands,0,parameters));
    std::vector<std::byte> pixels(width*height*4);
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,width*4));
    std::size_t visible=0;
    for(std::size_t pixel=0;pixel<pixels.size();pixel+=4)
        if(std::to_integer<int>(pixels[pixel])>40 || std::to_integer<int>(pixels[pixel+1])>40 || std::to_integer<int>(pixels[pixel+2])>40) ++visible;
    BOOST_TEST(visible>30000u);
    if(const char* preview=std::getenv("GENERALS_W3D_SURFACE_PREVIEW")) {
        Engine::Video::DecodedVideoFrame frame;
        frame.width=width; frame.height=height; frame.row_pitch=width*4;
        frame.format=Engine::Video::PixelFormat::RGBA8; frame.pixels=pixels;
        BOOST_REQUIRE(Engine::Video::Write_Frame_Image(preview,frame));
    }
    binding.Clear(); renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(converted_section_door_opens_through_native_rig_and_surface_renderer)
{
    const char* directory=std::getenv("GENERALS_W3D_DOOR_DIRECTORY");
    const char* textures=std::getenv("GENERALS_W3D_SURFACE_TEXTURES");
    if(!directory || !textures) { BOOST_TEST_MESSAGE("Set door and texture paths for animated surface drawing"); return; }
    const auto bytes=Read(std::filesystem::path(directory)/"ABArFrcCmd_A2.W3D");
    Assets::AssetCache assets([&](const Assets::AssetIdentity& identity) {
        return identity.type==Assets::AssetType::Model ? bytes : Read(std::filesystem::path(textures)/identity.canonical_name);
    });
    BOOST_REQUIRE(assets.Register_Model_Adapter(std::make_shared<Assets::W3DAdapter>()));
    const auto handle=assets.Request_Model("ABArFrcCmd_A2.W3D");assets.Wait(handle);
    const auto* model=assets.Try_Get_Model(handle);BOOST_REQUIRE_MESSAGE(model,assets.Get_Error(handle));
    std::string error;ModelAssetPose pose;BOOST_REQUIRE_MESSAGE(pose.Initialize(model->Rig(),error),error);
    GraphicsTestDevice device({true});PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    PropAssetBinding binding;BOOST_REQUIRE_MESSAGE(binding.Load(device,renderer,assets,handle,error),error);
    BOOST_REQUIRE_EQUAL(binding.Part_Count(),3u);
    constexpr unsigned extent=512;
    const auto target=device.Create_Texture({extent,extent,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({extent,extent,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    PropParameters parameters;
    parameters.view_projection={0,1.f/20,0,49.23512f/20,0,0,1.f/20,-15.f/20,-1.f/200,0,0,.1f,0,0,0,1};
    parameters.camera_position={100,-49,30,1};parameters.scene_ambient={.8f,.8f,.8f,1};
    parameters.light_direction[0]={1,0,0,1};parameters.light_diffuse[0]={1,1,1,1};parameters.light_specular[0]={.3f,.3f,.3f,1};
    auto& commands=device.Immediate_Command_List();BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));BOOST_REQUIRE(commands.Set_Viewport({0,0,extent,extent}));
    std::array<std::size_t,2> coverage{};
    for(unsigned sample=0;sample<2;++sample) {
        const auto frame=sample?40.f:0.f;BOOST_REQUIRE(pose.Evaluate(0,frame));BOOST_REQUIRE(commands.Clear({.02f,.02f,.02f,1},1));
        for(std::size_t part=0;part<binding.Part_Count();++part) BOOST_REQUIRE(binding.Draw_Part(commands,part,parameters,&pose));
        std::vector<std::byte> pixels(extent*extent*4);BOOST_REQUIRE(device.Readback_Texture(target,pixels,extent*4));
        for(std::size_t i=0;i<pixels.size();i+=4)if(std::to_integer<unsigned>(pixels[i])>25)++coverage[sample];
        if(const char* preview=std::getenv("GENERALS_W3D_DOOR_PREVIEW")) {
            Engine::Video::DecodedVideoFrame image;image.width=extent;image.height=extent;image.row_pitch=extent*4;
            image.format=Engine::Video::PixelFormat::RGBA8;image.pixels=std::move(pixels);
            BOOST_REQUIRE(Engine::Video::Write_Frame_Image(std::filesystem::path(preview)/(sample?"door_open.png":"door_closed.png"),image));
        }
    }
    BOOST_TEST(coverage[0]>10000u);BOOST_TEST(coverage[1]<coverage[0]*.4);
    DirectionalShadowRenderer shadows;PropSubmission submission;submission.Initialize(device,renderer,shadows);
    BOOST_REQUIRE(pose.Evaluate(0,0));BOOST_REQUIRE(commands.Clear({.02f,.02f,.02f,1},1));
    for(std::size_t part=0;part<binding.Part_Count();++part)
        BOOST_REQUIRE(binding.Submit_Part(submission,part,parameters,PropDrawPhase::Material,{},&pose));
    BOOST_REQUIRE(pose.Evaluate(0,40));binding.Clear();
    BOOST_REQUIRE(submission.Flush_Transparent());
    std::vector<std::byte> captured(extent*extent*4);BOOST_REQUIRE(device.Readback_Texture(target,captured,extent*4));
    std::size_t retained_coverage=0;for(std::size_t i=0;i<captured.size();i+=4)if(std::to_integer<unsigned>(captured[i])>25)++retained_coverage;
    BOOST_TEST(retained_coverage==coverage[0]);
    submission.Shutdown();renderer.Shutdown();device.Destroy_Texture(target);device.Destroy_Texture(depth);
}
