module;

#define BOOST_TEST_MODULE GraphicsDrawGenerationTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.DrawGeneration.Tests;

import Graphics.Scene.DrawGeneration;

using namespace Graphics;

static constexpr PipelineHandle Test_Pipeline(8, 1);
static constexpr PipelineHandle Double_Sided_Pipeline(9, 1);

static Mesh Make_Mesh() noexcept
{
	Mesh mesh;
	mesh.vertex_count = 100;
	mesh.index_count = 300;
	mesh.vertex_stride = 32;
	mesh.index_format = MeshIndexFormat::UInt32;
	return mesh;
}

static RenderInstance Make_Instance(MeshHandle mesh, MaterialHandle material, float x) noexcept
{
	RenderInstance instance;
	instance.transform.matrix = {
		1.0f, 0.0f, 0.0f, x,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, -5.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	instance.bounds.radius = 1.0f;
	instance.mesh = mesh;
	instance.material = material;
	return instance;
}

static Matrix4x4 Make_Projection() noexcept
{
	constexpr float near_clip = 1.0f;
	constexpr float far_clip = 10.0f;
	constexpr float inverse_depth = 1.0f / (far_clip - near_clip);

	Matrix4x4 projection{};
	projection.values[0] = 1.0f;
	projection.values[5] = 1.0f;
	projection.values[10] = -(far_clip + near_clip) * inverse_depth;
	projection.values[11] = -2.0f * far_clip * near_clip * inverse_depth;
	projection.values[14] = -1.0f;
	return projection;
}

static View Make_View() noexcept
{
	return {Matrix4x4::Identity(), Make_Projection(), {}, {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}};
}

static bool Build_Inputs(RenderScene &scene, MeshPool &meshes, MaterialPool &materials, VisibleSet &visible_set, LODSet &lod_set, MeshHandle &mesh, MaterialHandle &material)
{
	mesh = meshes.Create(Make_Mesh());
	material = materials.Create();
	scene.Create(Make_Instance(mesh, material, -1.0f));
	scene.Create(Make_Instance(mesh, material, 1.0f));
	if (!Build_Visible_Set(scene, Make_View(), visible_set))
		return false;
	return Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set);
}

BOOST_AUTO_TEST_CASE(draw_generation_packs_submission_records)
{
	RenderScene scene;
	MeshPool meshes;
	MaterialPool materials;
	std::array<InstanceHandle, 2> visible_storage{};
	std::array<LODSelection, 2> lod_storage{};
	VisibleSet visible_set(visible_storage);
	LODSet lod_set(lod_storage);
	MeshHandle mesh;
	MaterialHandle material;
	BOOST_REQUIRE(Build_Inputs(scene, meshes, materials, visible_set, lod_set, mesh, material));

	GPUScene gpu_scene;
	TexturePool textures;
	SamplerPool samplers;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));

	std::array<DrawData, 2> draw_storage{};
	DrawSet draw_set(draw_storage);
	BOOST_REQUIRE(Build_Draw_Data(lod_set, gpu_scene, {4, Test_Pipeline, 0x0004000800000001ull}, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 2);

	const DrawData &draw = draw_set.Records()[0];
	BOOST_CHECK(draw.mesh_index == gpu_scene.Mesh_Index(mesh));
	BOOST_CHECK(draw.material_index == gpu_scene.Material_Index(material));
	BOOST_CHECK(draw.instance_index == 0);
	BOOST_CHECK(draw.instance_count == 1);
	BOOST_CHECK(draw.pipeline == Test_Pipeline);
	BOOST_CHECK(draw.sort_key == 0x0004000800000001ull);
}

BOOST_AUTO_TEST_CASE(draw_generation_preserves_deterministic_input_order)
{
	RenderScene scene;
	MeshPool meshes;
	MaterialPool materials;
	std::array<InstanceHandle, 2> visible_storage{};
	std::array<LODSelection, 2> lod_storage{};
	VisibleSet visible_set(visible_storage);
	LODSet lod_set(lod_storage);
	MeshHandle mesh;
	MaterialHandle material;
	BOOST_REQUIRE(Build_Inputs(scene, meshes, materials, visible_set, lod_set, mesh, material));

	GPUScene gpu_scene;
	TexturePool textures;
	SamplerPool samplers;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));

	std::array<DrawData, 2> draw_storage{};
	DrawSet draw_set(draw_storage);
	BOOST_REQUIRE(Build_Draw_Data(lod_set, gpu_scene, {0, Test_Pipeline, 0}, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 2);
	BOOST_CHECK(draw_set.Records()[0].instance_index == 0);
	BOOST_CHECK(draw_set.Records()[1].instance_index == 1);
}

BOOST_AUTO_TEST_CASE(draw_generation_skips_invalid_inputs_and_rejects_capacity_overflow)
{
	RenderScene scene;
	MeshPool meshes;
	MaterialPool materials;
	std::array<InstanceHandle, 2> visible_storage{};
	std::array<LODSelection, 2> lod_storage{};
	VisibleSet visible_set(visible_storage);
	LODSet lod_set(lod_storage);
	MeshHandle mesh;
	MaterialHandle material;
	BOOST_REQUIRE(Build_Inputs(scene, meshes, materials, visible_set, lod_set, mesh, material));

	GPUScene gpu_scene;
	TexturePool textures;
	SamplerPool samplers;
	MaterialPool empty_materials;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, empty_materials));

	std::array<DrawData, 2> invalid_storage{};
	DrawSet invalid_draw_set(invalid_storage);
	BOOST_REQUIRE(Build_Draw_Data(lod_set, gpu_scene, {0, Test_Pipeline, 0}, invalid_draw_set));
	BOOST_CHECK(invalid_draw_set.Size() == 0);

	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	std::array<DrawData, 1> small_storage{};
	DrawSet small_draw_set(small_storage);
	BOOST_CHECK(!Build_Draw_Data(lod_set, gpu_scene, {0, Test_Pipeline, 0}, small_draw_set));
	BOOST_CHECK(small_draw_set.Size() == 0);
}

BOOST_AUTO_TEST_CASE(draw_generation_filters_hidden_submeshes)
{
	const std::array<MeshPart, 2> parts = {{{0, 3, 0}, {3, 3, 0}}};
	Mesh mesh = Make_Mesh();
	mesh.index_count = 6;
	mesh.parts = parts;

	MeshPool meshes;
	MaterialPool materials;
	const MeshHandle mesh_handle = meshes.Create(mesh);
	const MaterialHandle material_handle = materials.Create();
	RenderInstance instance = Make_Instance(mesh_handle, material_handle, 0.0f);
	instance.visibility_mask = Set_Submesh_Visible(All_Submeshes_Visible, 1, false);
	RenderScene scene;
	const InstanceHandle instance_handle = scene.Create(instance);

	std::array<InstanceHandle, 1> visible_storage{};
	std::array<LODSelection, 1> lod_storage{};
	VisibleSet visible_set(visible_storage);
	LODSet lod_set(lod_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));
	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set));

	TexturePool textures;
	SamplerPool samplers;
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));

	std::array<DrawData, 2> draw_storage{};
	DrawSet draw_set(draw_storage);
	BOOST_REQUIRE(Build_Draw_Data(lod_set, gpu_scene, {0, Test_Pipeline, 0}, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 1);
	BOOST_CHECK(draw_set.Records()[0].instance_index == gpu_scene.Instance_Index(instance_handle));
	BOOST_CHECK(draw_set.Records()[0].submesh_index == 0);
}

BOOST_AUTO_TEST_CASE(draw_generation_selects_double_sided_pipeline_from_instance_flags)
{
	MeshPool meshes;
	MaterialPool materials;
	const MeshHandle mesh = meshes.Create(Make_Mesh());
	const MaterialHandle material = materials.Create();
	RenderInstance instance = Make_Instance(mesh, material, 0.0f);
	instance.flags = RenderInstanceFlags::DoubleSided;
	RenderScene scene;
	BOOST_REQUIRE(scene.Create(instance).Is_Valid());

	std::array<InstanceHandle, 1> visible_storage{};
	std::array<LODSelection, 1> lod_storage{};
	VisibleSet visible_set(visible_storage);
	LODSet lod_set(lod_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));
	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set));

	TexturePool textures;
	SamplerPool samplers;
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));

	std::array<DrawData, 1> draw_storage{};
	DrawSet draw_set(draw_storage);
	BOOST_REQUIRE(Build_Draw_Data(lod_set, gpu_scene,
		{0, Test_Pipeline, 0, {}, Double_Sided_Pipeline}, draw_set));
	BOOST_REQUIRE(draw_set.Size() == 1);
	BOOST_CHECK(draw_set.Records()[0].pipeline == Double_Sided_Pipeline);
}
