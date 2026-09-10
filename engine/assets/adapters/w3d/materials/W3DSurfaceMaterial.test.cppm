module;
#define BOOST_TEST_MODULE W3DSurfaceMaterialTests
#include <boost/test/included/unit_test.hpp>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

export module Assets.Tests.W3DSurfaceMaterial;
import Assets.Adapters.W3D.ShaderMaterials;
import Assets.Adapters.W3D.SurfaceMaterial;
import Assets.Materials;

namespace
{
using namespace Assets;
using namespace Assets::W3D;
W3DShaderMaterial Object(std::string name = "ObjectsAllied.fx")
{
	W3DShaderMaterial source;
	source.version = 1;
	source.shader_name = std::move(name);
	source.properties = {
		{"DiffuseTexture", W3DShaderPropertyType::String, "airfield.dds"},
		{"NormalMap", W3DShaderPropertyType::String, "airfield_nrm.dds"},
		{"SpecMap", W3DShaderPropertyType::String, "airfield_spm.dds"}};
	return source;
}
}

BOOST_AUTO_TEST_CASE(faction_material_preserves_packed_masks_and_fixed_source_defaults)
{
	for (const auto name : {"ObjectsAllied.fx", "ObjectsSoviet.fx", "ObjectsJapan.fx"}) {
		auto source = Object(name);
		source.properties.push_back({"BumpScale", W3DShaderPropertyType::Float, {}, {2.0f}});
		source.properties.push_back({"AlphaTestEnable", W3DShaderPropertyType::Boolean, {}, {}, 0, true});
		MaterialAssetDesc material;
		std::string error;
		BOOST_REQUIRE_MESSAGE(W3DResolve_Surface_Material(source, material, error), error);
		BOOST_CHECK(material.surface.shading_model == MaterialShadingModel::SpecularGlossiness);
		BOOST_TEST(material.primary_texture == "airfield.dds");
		BOOST_TEST(material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Normal)] == "airfield_nrm.dds");
		BOOST_TEST(material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::TeamColor)] == "airfield_spm.dds");
		BOOST_CHECK(material.surface.specular_channel == MaterialTextureChannel::Red);
		BOOST_CHECK(material.surface.team_color_channel == MaterialTextureChannel::Blue);
		BOOST_TEST(material.surface.team_color_multiplier == 2.0f);
		BOOST_TEST(material.surface.normal_scale == 1.5f);
		BOOST_TEST(material.ambient_color.r == 0.1f);
		BOOST_TEST(material.shininess == (std::string(name) == "ObjectsSoviet.fx" ? 45.0f : 50.0f));
		BOOST_CHECK(material.render_mode == MaterialRenderMode::AlphaTest);
		BOOST_TEST(material.surface.alpha_cutoff == 96.0f / 255.0f);
	}
}

BOOST_AUTO_TEST_CASE(allied_tread_preserves_vertex_alpha_texture_offset_semantics)
{
	MaterialAssetDesc material;
	std::string error;
	BOOST_REQUIRE_MESSAGE(W3DResolve_Surface_Material(Object("ObjectsAlliedTread.fx"),material,error),error);
	BOOST_TEST(material.surface.uv_offset_from_vertex_alpha);
	BOOST_TEST(material.surface.team_color_multiplier==2.0f);
	BOOST_REQUIRE(W3DResolve_Surface_Material(Object(),material,error));
	BOOST_TEST(!material.surface.uv_offset_from_vertex_alpha);
}

BOOST_AUTO_TEST_CASE(generic_objects_use_authored_values_without_faction_recoloring)
{
	auto source = Object("ObjectsGeneric.fx");
	source.properties.push_back({"BumpScale", W3DShaderPropertyType::Float, {}, {2.0f}});
	source.properties.push_back({"SpecularExponent", W3DShaderPropertyType::Float, {}, {80.0f}});
	source.properties.push_back({"AmbientColor", W3DShaderPropertyType::Vector3, {}, {.2f, .3f, .4f}});
	MaterialAssetDesc material;
	std::string error;
	BOOST_REQUIRE(W3DResolve_Surface_Material(source, material, error));
	BOOST_TEST(material.surface.normal_scale == 2.0f);
	BOOST_TEST(material.shininess == 80.0f);
	BOOST_TEST(material.ambient_color.g == .3f);
	BOOST_TEST(material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::TeamColor)].empty());
}

BOOST_AUTO_TEST_CASE(unsupported_or_malformed_material_never_publishes_partial_translation)
{
	for (int scenario = 0; scenario < 7; ++scenario) {
		auto source = Object();
		switch (scenario) {
		case 0: source.version = 2; break;
		case 1: source.technique = 2; break;
		case 2: source.shader_name = "water.fx"; break;
		case 3: source.properties.push_back({"FutureEffect", W3DShaderPropertyType::Boolean}); break;
		case 4: source.properties[0].type = W3DShaderPropertyType::Integer; break;
		case 5: source.properties.pop_back(); break;
		case 6: source.properties.push_back(source.properties[0]); break;
		}
		MaterialAssetDesc material; material.name = "keep"; material.primary_texture = "keep.dds";
		std::string error;
		BOOST_TEST(!W3DResolve_Surface_Material(source, material, error));
		BOOST_TEST(!error.empty());
		BOOST_TEST(material.name == "keep");
		BOOST_TEST(material.primary_texture == "keep.dds");
		BOOST_CHECK(material.surface.shading_model == MaterialShadingModel::Legacy);
	}
}

BOOST_AUTO_TEST_CASE(extracted_airfield_fixture_translates_to_expected_atlas_roles)
{
	const char *directory = std::getenv("GENERALS_W3D_SHADER_DIRECTORY");
	if (!directory || !*directory) { BOOST_TEST_MESSAGE("Set GENERALS_W3D_SHADER_DIRECTORY for Evolution fixture coverage"); return; }
	std::size_t matches = 0;
	for (const auto &entry : std::filesystem::directory_iterator(directory)) {
		if (entry.path().extension() != ".bin") continue;
		std::ifstream file(entry.path(), std::ios::binary | std::ios::ate);
		BOOST_REQUIRE(file.good());
		const auto size = file.tellg(); file.seekg(0);
		std::vector<std::byte> bytes(static_cast<std::size_t>(size));
		BOOST_REQUIRE(file.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(bytes.size())));
		std::vector<W3DShaderMaterial> shaders;
		BOOST_REQUIRE(W3DRead_Shader_Materials(bytes, shaders));
		for (const auto &source : shaders) {
			if (source.shader_name != "objectsallied.fx") continue;
			bool airfield = false;
			for (const auto &property : source.properties)
				if (property.name == "DiffuseTexture" && property.texture == "ABNBUB_AllStructuresTextureAtlas") airfield = true;
			if (!airfield) continue;
			MaterialAssetDesc material; std::string error;
			BOOST_REQUIRE_MESSAGE(W3DResolve_Surface_Material(source, material, error), error);
			BOOST_TEST(material.primary_texture == "ABNBUB_AllStructuresTextureAtlas");
			BOOST_TEST(material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Normal)] == "ABNBUB_AllStructuresTextureAtlas_NRM");
			BOOST_TEST(material.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Specular)] == "ABNBUB_AllStructuresTextureAtlas_SPM");
			++matches;
		}
	}
	BOOST_TEST(matches > 0u);
	BOOST_TEST_MESSAGE("Translated " << matches << " authored shared-atlas object materials");
}
