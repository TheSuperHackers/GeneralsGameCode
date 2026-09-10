#include <array>
#include <span>
#include <vector>
#include "W3DDevice/GameClient/WaterMaterial.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
#include "Common/GlobalData.h"

#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"
#include "WWMath/matrix4.h"
#include <algorithm>
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.DrawParameters;
import Graphics.Frame.Runtime;

namespace {
std::array<float,4> Copy_Vector(const Vector4& value) { return {value.X,value.Y,value.Z,value.W}; }
}
WaterMaterialClass::WaterMaterialClass() = default;
WaterMaterialClass::~WaterMaterialClass() = default;
void WaterMaterialClass::Set_Common_Constants(const WaterMaterialParameters& parameters)
{
    m_parameters.shroud_projection = Copy_Vector(parameters.shroud_projection);
    m_parameters.animation = Copy_Vector(parameters.animation);
    m_parameters.camera_position = Copy_Vector(parameters.camera_position);
    m_parameters.displacement_domain = Copy_Vector(parameters.displacement_domain);
    m_parameters.tint = Copy_Vector(parameters.tint);
    m_parameters.effects = Copy_Vector(parameters.effects);
    m_parameters.surface_options = Copy_Vector(parameters.surface_options);
}
void WaterMaterialClass::Set_Frame_Lighting(W3DScene* scene)
{
    const auto& camera = Graphics::Get_Camera_Matrices();
    Matrix4x4 view, projection;
    std::copy_n(camera.view.values.data(),16,&view[0][0]);
    std::copy_n(camera.projection.values.data(),16,&projection[0][0]);
    const Matrix4x4 inverse = (projection * view).Inverse();
    std::copy_n(&inverse[0][0],16,m_parameters.inverse_view_projection.data());
    if (TheGlobalData) {
        const auto& color = TheGlobalData->m_terrainDiffuse[0];
        const auto& direction = TheGlobalData->m_terrainLightPos[0];
        m_parameters.sun_color = {color.red,color.green,color.blue,1};
        m_parameters.environment_frame = Graphics::Water_Environment_Frame({-direction.x,-direction.y,-direction.z});
    }
    m_parameters.fog_state = {};
    if (!scene) return;
    scene->Get_Fog_Range(&m_parameters.fog_state[0],&m_parameters.fog_state[1]);
    m_parameters.fog_state[2] = Graphics::Get_Scene_Draw_Parameters().fog.enabled ? 1.0f : 0.0f;
    const auto& color = scene->Get_Fog_Color();
    m_parameters.fog_color = {color.X,color.Y,color.Z,1};
}
bool WaterMaterialClass::Apply_Underwater(Graphics::RHITextureHandle scene_texture,
    Graphics::RHITextureHandle depth_texture, W3DTextureHandle *caustics_texture,
    W3DTextureHandle *depth_lut_texture, const WaterMaterialParameters &parameters)
{
    m_textures = {};
    m_textures[5] = scene_texture;
    m_textures[8] = depth_texture;
    m_textures[9] = Resolve_Graphics_Texture(caustics_texture);
    m_textures[10] = Resolve_Graphics_Texture(depth_lut_texture);
    if (!m_textures[5].Is_Valid() || !m_textures[8].Is_Valid()
        || !m_textures[9].Is_Valid() || !m_textures[10].Is_Valid()) return false;
    m_style = {};
    m_style.pass = Graphics::WaterPass::Underwater;
    m_style.blend = Graphics::RHIBlendMode::Disabled;
    Set_Common_Constants(parameters);
    return true;
}

bool WaterMaterialClass::Apply_Ocean(W3DTextureHandle *surface_texture,
	Graphics::RHITextureHandle displacement_texture,
	W3DTextureHandle *normal_texture, W3DTextureHandle *foam_texture,
	W3DTextureHandle *reflection_texture, Graphics::RHITextureHandle refraction_texture,
	W3DTextureHandle *environment_texture, W3DTextureHandle *shroud_texture,
	Graphics::RHITextureHandle scene_depth_texture,
	W3DTextureHandle *caustics_texture, W3DTextureHandle *depth_lut_texture,
	const WaterMaterialParameters &parameters, bool additive_blend)
{
	if (!Graphics::Shared_Frame_Device())
	{
		return false;
	}

	m_style.pass = Graphics::WaterPass::Ocean;
	m_textures[0] = Resolve_Graphics_Texture(surface_texture);
	m_textures[1] = displacement_texture;
	m_textures[2] = Resolve_Graphics_Texture(normal_texture);
	m_textures[3] = Resolve_Graphics_Texture(foam_texture);
	m_textures[4] = Resolve_Graphics_Texture(reflection_texture);
	m_textures[5] = refraction_texture;
	m_textures[6] = Resolve_Graphics_Texture(environment_texture);
	m_textures[7] = Resolve_Graphics_Texture(shroud_texture);
	m_textures[8] = scene_depth_texture;
	m_textures[9] = Resolve_Graphics_Texture(caustics_texture);
	m_textures[10] = Resolve_Graphics_Texture(depth_lut_texture);
	Set_Common_Constants(parameters);
    m_parameters.effects[3] = scene_depth_texture.Is_Valid() && m_textures[9].Is_Valid()
        && m_textures[10].Is_Valid() ? 1.0f : 0.0f;
	m_style.blend = additive_blend ? Graphics::RHIBlendMode::Additive : Graphics::RHIBlendMode::Alpha;
    m_style.clamp_texture = false;
	return true;
}

bool WaterMaterialClass::Apply_Surface(W3DTextureHandle *surface_texture,
	W3DTextureHandle *normal_texture, W3DTextureHandle *foam_texture,
	W3DTextureHandle *edge_texture, W3DTextureHandle *reflection_texture,
	Graphics::RHITextureHandle refraction_texture, W3DTextureHandle *environment_texture,
	W3DTextureHandle *shroud_texture, Graphics::RHITextureHandle scene_depth_texture,
	const WaterMaterialParameters &parameters, bool additive_blend)
{
	if (!Graphics::Shared_Frame_Device())
	{
		return false;
	}

	m_style.pass = Graphics::WaterPass::Surface;
	m_textures[0] = Resolve_Graphics_Texture(surface_texture);
	m_textures[1] = Resolve_Graphics_Texture(normal_texture);
	m_textures[2] = Resolve_Graphics_Texture(foam_texture);
	m_textures[3] = Resolve_Graphics_Texture(edge_texture);
	m_textures[4] = Resolve_Graphics_Texture(reflection_texture);
	m_textures[5] = refraction_texture;
	m_textures[6] = Resolve_Graphics_Texture(environment_texture);
	m_textures[7] = Resolve_Graphics_Texture(shroud_texture);
	m_textures[8] = scene_depth_texture;
	Set_Common_Constants(parameters);
	m_style.blend = additive_blend ? Graphics::RHIBlendMode::Additive : Graphics::RHIBlendMode::Alpha;
    m_style.clamp_texture = false;
	return true;
}

bool WaterMaterialClass::Apply_Track(W3DTextureHandle *wave_texture)
{
	if (!Graphics::Shared_Frame_Device())
	{
		return false;
	}

	m_style.pass = Graphics::WaterPass::Track;
	for (unsigned stage = 0; stage < 8; ++stage)
	{
		m_textures[stage] = Resolve_Graphics_Texture(stage == 0 ? wave_texture : nullptr);
	}
	m_style.blend = Graphics::RHIBlendMode::Alpha;
    m_style.clamp_texture = false;
	return true;
}

bool WaterMaterialClass::Apply_Sky(W3DTextureHandle *texture, bool alpha_blend,
	bool clamp_texture)
{
	if (!Graphics::Shared_Frame_Device())
	{
		return false;
	}

	m_style.pass = Graphics::WaterPass::Sky;
	for (unsigned stage = 0; stage < 8; ++stage)
	{
		m_textures[stage] = Resolve_Graphics_Texture(stage == 0 ? texture : nullptr);
	}
	m_style.blend = alpha_blend ? Graphics::RHIBlendMode::Alpha : Graphics::RHIBlendMode::Disabled;
    m_style.clamp_texture = clamp_texture;
	return true;
}


void WaterMaterialClass::Shutdown() { m_textures = {}; }
bool WaterMaterialClass::ReacquireResources() { return Graphics::Shared_Frame_Device() != nullptr; }

bool WaterMaterialClass::Draw(Graphics::WaterMeshHandle mesh, const Matrix4x4& world, bool wireframe)
{
    auto* device = Graphics::Shared_Frame_Device();
    if (!device) return false;
    std::copy_n(&world[0][0], 16, m_parameters.world.data());
    m_parameters.view = Graphics::Get_Camera_Matrices().view.values;
    m_parameters.projection = Graphics::Get_Camera_Matrices().projection.values;
    auto style = m_style;
    style.wireframe = wireframe;
    return Graphics::Get_Water_Renderer().Draw(device->Immediate_Command_List(),mesh,style,m_parameters,m_textures);
}

bool WaterMaterialClass::Draw_Patches(Graphics::WaterMeshHandle mesh, const Graphics::OceanPatchGrid& grid)
{
    auto* device = Graphics::Shared_Frame_Device();
    if (!device) return false;
    m_parameters.view = Graphics::Get_Camera_Matrices().view.values;
    m_parameters.projection = Graphics::Get_Camera_Matrices().projection.values;
    return Graphics::Get_Water_Renderer().Draw_Patches(
        device->Immediate_Command_List(),mesh,m_style,m_parameters,m_textures,grid);
}

bool Upload_Water_Geometry(Graphics::WaterMeshHandle& mesh,
    std::span<const WaterSurfaceVertex> source, std::span<const unsigned short> source_indices, bool triangle_strip)
{
    std::vector<Graphics::WaterVertex> vertices;
    vertices.reserve(source.size());
    for (const auto& input : source) {
        Graphics::WaterVertex vertex;
        vertex.position = {input.x,input.y,input.z};
        vertex.normal = {input.nx,input.ny,input.nz};
        vertex.uv = {input.u1,input.v1};
        vertex.secondary_uv = {input.u2,input.v2};
        vertex.color = {((input.diffuse>>16)&255)/255.0f,((input.diffuse>>8)&255)/255.0f,
            (input.diffuse&255)/255.0f,((input.diffuse>>24)&255)/255.0f};
        vertices.push_back(vertex);
    }
    std::vector<std::uint32_t> indices(source_indices.begin(),source_indices.end());
    if (triangle_strip) indices = Graphics::Expand_Water_Strip(indices);
    auto& renderer = Graphics::Get_Water_Renderer();
    if (mesh.Is_Valid()) return renderer.Update_Mesh(mesh,vertices,indices);
    mesh = renderer.Create_Mesh(vertices,indices);
    return mesh.Is_Valid();
}

