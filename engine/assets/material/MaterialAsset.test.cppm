module;

#define BOOST_TEST_MODULE GeneralsAssetsMaterialTests

#include <boost/test/included/unit_test.hpp>
#include <limits>

export module Assets.Tests.MaterialAsset;

import Assets.Handles;
import Assets.Identity;
import Assets.Materials;

BOOST_AUTO_TEST_CASE(material_runtime_data_keeps_typed_texture_dependencies)
{
	const Assets::TextureAssetHandle primary(2, 1);
	const Assets::TextureAssetHandle secondary(3, 1);
	const Assets::MaterialAsset material(
		{Assets::AssetType::Material, "materials/paint"},
		{"paint", "textures/paint.tga", "textures/mask.tga"},
		primary,
		secondary);

	BOOST_CHECK(material.Name() == "paint");
	BOOST_CHECK(material.Primary_Texture() == primary);
	BOOST_CHECK(material.Secondary_Texture() == secondary);
	BOOST_CHECK(material.Surface().shading_model == Assets::MaterialShadingModel::Legacy);
	BOOST_CHECK(!material.Surface_Texture(Assets::MaterialTextureRole::Normal).Is_Valid());
}

BOOST_AUTO_TEST_CASE(surface_maps_and_parameters_are_immutable_after_publication)
{
	using namespace Assets;
	MaterialAssetDesc description;
	description.name = "airfield";
	description.surface.shading_model = MaterialShadingModel::SpecularGlossiness;
	description.surface.normal_scale = 2.0f;
	description.surface_textures[static_cast<std::size_t>(MaterialTextureRole::Normal)] = "airfield_nrm.dds";
	MaterialSurfaceTextureHandles handles{};
	handles[static_cast<std::size_t>(MaterialTextureRole::Normal)] = TextureAssetHandle(9, 3);
	handles[static_cast<std::size_t>(MaterialTextureRole::Specular)] = TextureAssetHandle(10, 2);
	const MaterialAsset material({AssetType::Material, "airfield"}, description, {}, {}, handles);
	description.surface.normal_scale = 0.0f;
	handles.fill({});
	BOOST_CHECK(material.Surface().shading_model == MaterialShadingModel::SpecularGlossiness);
	BOOST_TEST(material.Surface().normal_scale == 2.0f);
	BOOST_CHECK(material.Surface_Texture(MaterialTextureRole::Normal) == TextureAssetHandle(9, 3));
	BOOST_CHECK(material.Surface_Texture(MaterialTextureRole::Specular) == TextureAssetHandle(10, 2));
	BOOST_CHECK(!material.Surface_Texture(MaterialTextureRole::Count).Is_Valid());
}

BOOST_AUTO_TEST_CASE(surface_validation_rejects_invalid_values_without_changing_legacy_defaults)
{
	using namespace Assets;
	MaterialSurfaceParameters parameters;
	BOOST_TEST(Validate_Material_Surface(parameters));
	parameters.shading_model = static_cast<MaterialShadingModel>(255);
	BOOST_TEST(!Validate_Material_Surface(parameters));
	parameters = {}; parameters.normal_scale = std::numeric_limits<float>::infinity();
	BOOST_TEST(!Validate_Material_Surface(parameters));
	parameters = {}; parameters.roughness = -0.1f;
	BOOST_TEST(!Validate_Material_Surface(parameters));
	parameters = {}; parameters.metallic = 1.01f;
	BOOST_TEST(!Validate_Material_Surface(parameters));
	parameters = {}; parameters.alpha_cutoff = std::numeric_limits<float>::quiet_NaN();
	BOOST_TEST(!Validate_Material_Surface(parameters));
	parameters = {}; parameters.specular_channel = static_cast<MaterialTextureChannel>(255);
	BOOST_TEST(!Validate_Material_Surface(parameters));
	parameters = {}; parameters.team_color_channel = MaterialTextureChannel::RGB;
	BOOST_TEST(!Validate_Material_Surface(parameters));
	parameters = {}; parameters.team_color_multiplier = -1.0f;
	BOOST_TEST(!Validate_Material_Surface(parameters));
}

BOOST_AUTO_TEST_CASE(material_runtime_retains_independent_lighting_colors)
{
	Assets::MaterialAssetDesc description;
	description.name = "paint";
	description.base_color = {0.1f, 0.2f, 0.3f, 1};
	description.ambient_color = {0.2f, 0.3f, 0.4f, 1};
	description.specular_color = {0.3f, 0.4f, 0.5f, 1};
	description.emissive_color = {0.4f, 0.5f, 0.6f, 1};
	const Assets::MaterialAsset material({Assets::AssetType::Material, "paint"}, description, {}, {});
	BOOST_CHECK_EQUAL(material.Base_Color().r, 0.1f);
	BOOST_CHECK_EQUAL(material.Ambient_Color().g, 0.3f);
	BOOST_CHECK_EQUAL(material.Specular_Color().b, 0.5f);
	BOOST_CHECK_EQUAL(material.Emissive_Color().r, 0.4f);
}
