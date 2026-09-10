module;

#define BOOST_TEST_MODULE GeneralsAssetsMaterialLoadTaskTests

#include <boost/test/included/unit_test.hpp>

export module Assets.Tests.MaterialLoadTask;

import Assets.Cache.MaterialLoadTask;
import Assets.Handles;
import Assets.Identity;
import Assets.Materials;

BOOST_AUTO_TEST_CASE(material_finalize_task_keeps_typed_texture_handles)
{
	const Assets::TextureAssetHandle primary(4, 1);
	const Assets::MaterialLoadResult result = Assets::Finalize_Material_Asset(
		{Assets::AssetType::Material, "materials/paint"},
		{"paint", "textures/paint.tga"},
		primary,
		Assets::TextureAssetHandle::Invalid());

	BOOST_REQUIRE(result.Succeeded());
	BOOST_REQUIRE(result.asset != nullptr);
	BOOST_CHECK(result.asset->Primary_Texture() == primary);
}

BOOST_AUTO_TEST_CASE(surface_material_requires_declared_dependencies_before_publication)
{
	using namespace Assets;
	MaterialAssetDesc description;
	description.name = "surface";
	description.surface.shading_model = MaterialShadingModel::SpecularGlossiness;
	const auto role = static_cast<std::size_t>(MaterialTextureRole::Normal);
	description.surface_textures[role] = "normal.dds";
	const auto missing = Finalize_Material_Asset({AssetType::Material,"surface"}, description, {}, {});
	BOOST_TEST(!missing.Succeeded());
	BOOST_TEST(missing.asset == nullptr);
	MaterialSurfaceTextureHandles handles{};
	handles[role] = TextureAssetHandle(4, 2);
	const auto ready = Finalize_Material_Asset({AssetType::Material,"surface"}, description, {}, {}, handles);
	BOOST_REQUIRE(ready.Succeeded());
	BOOST_CHECK(ready.asset->Surface_Texture(MaterialTextureRole::Normal) == handles[role]);
	description.surface.metallic = 2.0f;
	BOOST_TEST(!Finalize_Material_Asset({AssetType::Material,"surface"}, description, {}, {}, handles).Succeeded());
}
