module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module Assets.Adapters.W3D.SurfaceMaterial;

import Assets.Adapters.W3D.ShaderMaterials;
import Assets.Materials;
import Assets.Math;

namespace Assets::W3D
{

namespace SurfaceMaterialDetail
{
bool Equal_Name(std::string_view left, std::string_view right)
{
	return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(),
		[](char a, char b) {
			return (a >= 'A' && a <= 'Z' ? a + ('a' - 'A') : a) == b;
		});
}

bool Read_Color(const W3DShaderProperty &property, Color4f &color)
{
	if (property.type != W3DShaderPropertyType::Vector3 && property.type != W3DShaderPropertyType::Vector4)
		return false;
	const std::size_t count = property.type == W3DShaderPropertyType::Vector3 ? 3 : 4;
	for (std::size_t i = 0; i < count; ++i)
		if (!std::isfinite(property.values[i]) || property.values[i] < 0)
			return false;
	color = {property.values[0], property.values[1], property.values[2], count == 4 ? property.values[3] : 1};
	return true;
}
}

// Translate source material semantics, not executable FX programs. Supported
// object variants share EA's Objects.fxh. Specialized damage, water and
// effect programs need their own translations and fail explicitly here.
// Reference: electronicarts/CnC_Modding_Support, Red Alert 3/Shaders/Objects.fxh.
export bool W3DResolve_Surface_Material(const W3DShaderMaterial &source,
	MaterialAssetDesc &result, std::string &error)
{
	error.clear();
	const auto fail = [&](std::string reason) {
		error = "W3D shader '" + source.shader_name + "': " + std::move(reason);
		return false;
	};
	if (source.version != 1)
		return fail("unsupported material version " + std::to_string(source.version));
	if (source.technique != 0)
		return fail("unsupported material technique " + std::to_string(source.technique));
	const bool allied_tread = SurfaceMaterialDetail::Equal_Name(source.shader_name, "objectsalliedtread.fx");
	const bool allied = allied_tread || SurfaceMaterialDetail::Equal_Name(source.shader_name, "objectsallied.fx");
	const bool soviet = SurfaceMaterialDetail::Equal_Name(source.shader_name, "objectssoviet.fx");
	const bool japan = SurfaceMaterialDetail::Equal_Name(source.shader_name, "objectsjapan.fx");
	const bool generic = SurfaceMaterialDetail::Equal_Name(source.shader_name, "objectsgeneric.fx");
	if (!allied && !soviet && !japan && !generic)
		return fail("unsupported shader program");

	MaterialAssetDesc material;
	material.name = source.shader_name;
	material.scope = MaterialScope::Model;
	material.surface.shading_model = MaterialShadingModel::SpecularGlossiness;
	material.surface.normal_scale = generic ? 1.0f : 1.5f;
	material.surface.uv_offset_from_vertex_alpha = allied_tread;
	material.surface.specular_channel = MaterialTextureChannel::Red;
	material.surface.team_color_channel = MaterialTextureChannel::Blue;
	material.surface.team_color_multiplier = 2.0f;
	material.surface.alpha_cutoff = 96.0f / 255.0f;
	material.ambient_color = generic ? Color4f{0.4f, 0.4f, 0.4f, 1} : Color4f{0.1f, 0.1f, 0.1f, 1};
	material.specular_color = {0.8f, 0.8f, 0.8f, 1};
	material.shininess = soviet ? 45.0f : 50.0f;
	std::vector<std::string_view> names;
	for (const auto &property : source.properties) {
		if (std::find(names.begin(), names.end(), property.name) != names.end())
			return fail("duplicate property '" + property.name + "'");
		names.push_back(property.name);
		const auto invalid = [&]() { return fail("invalid property '" + property.name + "'"); };
		if (property.name == "DiffuseTexture" || property.name == "NormalMap" || property.name == "SpecMap") {
			if (property.type != W3DShaderPropertyType::String || property.texture.empty())
				return invalid();
			if (property.name == "DiffuseTexture") material.primary_texture = property.texture;
			else if (property.name == "NormalMap")
				material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Normal)] = property.texture;
			else {
				material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Specular)] = property.texture;
				if (!generic)
					material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::TeamColor)] = property.texture;
			}
		} else if (property.name == "AlphaTestEnable") {
			if (property.type != W3DShaderPropertyType::Boolean) return invalid();
			material.render_mode = property.boolean ? MaterialRenderMode::AlphaTest : MaterialRenderMode::Opaque;
		} else if (property.name == "BumpScale" || property.name == "SpecularExponent" || property.name == "EnvMult") {
			if (property.type != W3DShaderPropertyType::Float || !std::isfinite(property.values[0]) || property.values[0] < 0)
				return invalid();
			// Faction shaders compile these material parameters as constants.
			// EnvMult is declared but unused in Objects.fxh's object passes.
			if (generic && property.name == "BumpScale") material.surface.normal_scale = property.values[0];
			if (generic && property.name == "SpecularExponent") material.shininess = property.values[0];
		} else if (property.name == "AmbientColor" || property.name == "DiffuseColor" || property.name == "SpecularColor") {
			Color4f color;
			if (!SurfaceMaterialDetail::Read_Color(property, color)) return invalid();
			if (generic) {
				if (property.name == "AmbientColor") material.ambient_color = color;
				if (property.name == "DiffuseColor") material.base_color = color;
				if (property.name == "SpecularColor") material.specular_color = color;
			}
		} else {
			return fail("unsupported property '" + property.name + "'");
		}
	}
	if (material.primary_texture.empty() ||
		material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Normal)].empty() ||
		material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Specular)].empty())
		return fail("diffuse, normal and specular maps are required");
	if (!Validate_Material_Surface(material.surface))
		return fail("invalid surface parameters");
	result = std::move(material);
	return true;
}

}
