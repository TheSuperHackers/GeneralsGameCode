import Graphics.Materials.State;
#include <array>
#include <bit>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
import Assets.Adapters.W3D.Particles;
import Assets.Images.PixelEncoding;

#include "W3DDevice/GameClient/W3DEmitterLoader.h"
#include "W3DDevice/GameClient/W3DEmitterRenderObject.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "WWLib/chunkio.h"
#include "WWLib/ref_ptr.h"

namespace {

std::unique_ptr<Vector3Randomizer> Create_Randomizer(const Assets::EmitterRandomizerDesc& data)
{
    const auto& d = data.dimensions;
    switch (data.kind) {
    case Assets::EmitterRandomizerKind::SolidBox:
        return std::make_unique<Vector3SolidBoxRandomizer>(Vector3(d.x,d.y,d.z));
    case Assets::EmitterRandomizerKind::SolidSphere:
        return std::make_unique<Vector3SolidSphereRandomizer>(d.x);
    case Assets::EmitterRandomizerKind::HollowSphere:
        return std::make_unique<Vector3HollowSphereRandomizer>(d.x);
    case Assets::EmitterRandomizerKind::SolidCylinder:
        return std::make_unique<Vector3SolidCylinderRandomizer>(d.x,d.y);
    default: return {};
    }
}

Graphics::MaterialState Create_Shader(const Assets::EmitterShaderDesc& data)
{
    Graphics::MaterialState shader;
    shader.Set_Depth_Compare(static_cast<Graphics::MaterialState::DepthCompareType>(data.depth_compare));
    shader.Set_Depth_Mask(data.depth_write ? Graphics::MaterialState::DEPTH_WRITE_ENABLE : Graphics::MaterialState::DEPTH_WRITE_DISABLE);
    shader.Set_Color_Mask(Graphics::MaterialState::COLOR_WRITE_ENABLE);
    shader.Set_Fog_Func(Graphics::MaterialState::FOG_DISABLE);
    shader.Set_Dst_Blend_Func(static_cast<Graphics::MaterialState::DstBlendFuncType>(data.destination_blend));
    switch (data.source_blend) {
    case Assets::EmitterBlendFactor::Zero: shader.Set_Src_Blend_Func(Graphics::MaterialState::SRCBLEND_ZERO); break;
    case Assets::EmitterBlendFactor::One: shader.Set_Src_Blend_Func(Graphics::MaterialState::SRCBLEND_ONE); break;
    case Assets::EmitterBlendFactor::SourceAlpha: shader.Set_Src_Blend_Func(Graphics::MaterialState::SRCBLEND_SRC_ALPHA); break;
    case Assets::EmitterBlendFactor::OneMinusSourceAlpha: shader.Set_Src_Blend_Func(Graphics::MaterialState::SRCBLEND_ONE_MINUS_SRC_ALPHA); break;
    default: WWASSERT(false); break;
    }
    shader.Set_Primary_Gradient(static_cast<Graphics::MaterialState::PriGradientType>(data.primary_gradient));
    shader.Set_Secondary_Gradient(static_cast<Graphics::MaterialState::SecGradientType>(data.secondary_gradient));
    shader.Set_Texturing(data.texturing ? Graphics::MaterialState::TEXTURING_ENABLE : Graphics::MaterialState::TEXTURING_DISABLE);
    shader.Set_Alpha_Test(data.alpha_test ? Graphics::MaterialState::ALPHATEST_ENABLE : Graphics::MaterialState::ALPHATEST_DISABLE);
    // The material pass consumes authored detail functions through these slots.
    shader.Set_Post_Detail_Color_Func(static_cast<Graphics::MaterialState::DetailColorFuncType>(data.detail_color_function));
    shader.Set_Post_Detail_Alpha_Func(static_cast<Graphics::MaterialState::DetailAlphaFuncType>(data.detail_alpha_function));
    return shader;
}

W3DEmitterRenderObject* Create_Emitter(const Assets::EmitterAssetDesc& data)
{
    auto position = Create_Randomizer(data.creation_volume);
    auto velocity = Create_Randomizer(data.velocity_random);
    if (!position || !velocity) return nullptr;
	    auto* catalog = W3DAssetCatalog::Get_Instance();
    RefCountPtr<W3DTextureHandle> texture;
	if (catalog != nullptr && !data.texture_name.empty())
	        texture.Assign_No_Add_Ref(catalog->Get_Texture(data.texture_name.c_str(),MIP_LEVELS_ALL,Assets::PixelEncoding::Unknown));
    Graphics::MaterialState shader = Create_Shader(data.shader);
    if (data.texture_blend_policy != Assets::EmitterTextureBlendPolicy::Authored) {
        shader = Graphics::MaterialState::AdditiveSprite();
        if (data.texture_blend_policy == Assets::EmitterTextureBlendPolicy::AlphaSpriteWhenTextureHasAlpha
            && texture.Peek() && Assets::Has_Explicit_Pixel_Alpha(texture->Get_Texture_Format()))
            shader = Graphics::MaterialState::AlphaSprite();
    }
	    if (catalog != nullptr && catalog->Get_Fog_On_Load()) shader.Enable_Fog_For_Blend();
    auto* emitter = new W3DEmitterRenderObject(data, texture.Peek(), shader,
        std::move(position), std::move(velocity));
    emitter->Set_Name(data.name.c_str());
    return emitter;
}
}

Graphics::ModelFactory<W3DRenderObject>* Load_ParticleEmitter_Factory(ChunkLoadClass& source)
{
    std::vector<std::byte> bytes(source.Cur_Chunk_Length());
    if (source.Read(bytes.data(),static_cast<unsigned>(bytes.size())) != bytes.size()) return nullptr;
    Assets::EmitterAssetDesc description;
    std::string error;
    if (!Assets::W3D::W3DRead_Emitter(bytes,description,error)) return nullptr;
    auto data = std::make_shared<const Assets::EmitterAssetDesc>(std::move(description));
    return new Graphics::ModelFactory<W3DRenderObject>(data->name,W3DRenderObject::CLASSID_PARTICLEEMITTER,
        [data] { return Create_Emitter(*data); });
}
