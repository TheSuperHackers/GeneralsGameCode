module;

#define BOOST_TEST_MODULE GraphicsDecalDrawTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.DecalDraw.Tests;

import Graphics.Scene.DecalDraw;

using namespace Graphics;

namespace
{
Matrix4x4 Make_Perspective(float near_clip, float far_clip) noexcept
{
	Matrix4x4 projection{};
	projection.values[0] = 1.0f;
	projection.values[5] = 1.0f;
	projection.values[10] = -(far_clip + near_clip) / (far_clip - near_clip);
	projection.values[11] = -2.0f * far_clip * near_clip / (far_clip - near_clip);
	projection.values[14] = -1.0f;
	return projection;
}
}

BOOST_AUTO_TEST_CASE(decal_draw_data_is_indexed_and_deterministically_ordered)
{
	MaterialPool materials;
	const MaterialHandle material = materials.Create();
	RenderScene scene;
	RenderDecal first;
	first.bounds.center = {0.0f, 0.0f, -5.0f};
	first.material = material;
	const DecalHandle first_handle = scene.Create_Decal(first);
	RenderDecal second = first;
	second.bounds.center.z = -10.0f;
	const DecalHandle second_handle = scene.Create_Decal(second);

	const View view{Matrix4x4::Identity(), Make_Perspective(1.0f, 100.0f), {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
	std::array<DecalHandle, 2> visible_storage{};
	VisibleDecalSet visible_decals(visible_storage);
	BOOST_REQUIRE(Build_Visible_Decals(scene, view, visible_decals));

	MeshPool meshes;
	TexturePool textures;
	SamplerPool samplers;
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));

	std::array<DecalDrawData, 2> draw_storage{};
	DecalDrawSet draw_set(draw_storage);
	const DrawPass pass{3, PipelineHandle(7, 1), 0};
	BOOST_REQUIRE(Build_Decal_Draw_Data(visible_decals, gpu_scene, pass, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 2);
	BOOST_CHECK(draw_set.Records()[0].decal_index == gpu_scene.Decal_Index(first_handle));
	BOOST_CHECK(draw_set.Records()[1].decal_index == gpu_scene.Decal_Index(second_handle));
	BOOST_CHECK(draw_set.Records()[0].material_index == gpu_scene.Material_Index(material));
	BOOST_CHECK(draw_set.Records()[0].pipeline == pass.pipeline);

	std::array<DecalDrawData, 2> repeated_storage{};
	DecalDrawSet repeated_set(repeated_storage);
	BOOST_REQUIRE(Build_Decal_Draw_Data(visible_decals, gpu_scene, pass, repeated_set));
	BOOST_CHECK(repeated_set.Records()[0].decal_index == draw_set.Records()[0].decal_index);
	BOOST_CHECK(repeated_set.Records()[1].decal_index == draw_set.Records()[1].decal_index);
}

BOOST_AUTO_TEST_CASE(decal_draw_data_skips_stale_gpu_handles_and_rejects_missing_pipeline)
{
	MaterialPool materials;
	const MaterialHandle material = materials.Create();
	RenderScene scene;
	RenderDecal decal;
	decal.material = material;
	decal.bounds.center = {0.0f, 0.0f, -5.0f};
	const DecalHandle stale_handle = scene.Create_Decal(decal);
	const DecalHandle live_handle = scene.Create_Decal(decal);
	const View view{Matrix4x4::Identity(), Make_Perspective(1.0f, 100.0f), {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
	std::array<DecalHandle, 2> visible_storage{};
	VisibleDecalSet visible_decals(visible_storage);
	BOOST_REQUIRE(Build_Visible_Decals(scene, view, visible_decals));

	MeshPool meshes;
	TexturePool textures;
	SamplerPool samplers;
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	BOOST_REQUIRE(scene.Destroy_Decal(stale_handle));
	BOOST_REQUIRE(gpu_scene.Remove_Decal(stale_handle));

	std::array<DecalDrawData, 2> draw_storage{};
	DecalDrawSet draw_set(draw_storage);
	BOOST_REQUIRE(Build_Decal_Draw_Data(visible_decals, gpu_scene, {0, PipelineHandle(9, 1), 0}, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 1);
	BOOST_CHECK(draw_set.Records()[0].decal_index == gpu_scene.Decal_Index(live_handle));
	BOOST_CHECK(!Build_Decal_Draw_Data(visible_decals, gpu_scene, {0, {}, 0}, draw_set));
}
