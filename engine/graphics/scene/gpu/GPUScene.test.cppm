module;

#define BOOST_TEST_MODULE GraphicsGPUSceneTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <type_traits>

export module Graphics.Scene.GPUScene.Tests;

import Graphics.Scene.GPUScene;

using namespace Graphics;

static Mesh Make_Mesh(std::uint32_t vertex_count, std::uint32_t index_count) noexcept
{
	Mesh mesh;
	mesh.vertex_count = vertex_count;
	mesh.index_count = index_count;
	mesh.vertex_stride = 32;
	mesh.index_format = MeshIndexFormat::UInt32;
	return mesh;
}

static RenderInstance Make_Instance(MeshHandle mesh, MaterialHandle material) noexcept
{
	RenderInstance instance;
	instance.transform.matrix = {
		1.0f, 0.0f, 0.0f, 4.0f,
		0.0f, 1.0f, 0.0f, 5.0f,
		0.0f, 0.0f, 1.0f, 6.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	instance.bounds.center = {1.0f, 2.0f, 3.0f};
	instance.bounds.radius = 7.0f;
	instance.mesh = mesh;
	instance.material = material;
	instance.flags = RenderInstanceFlags::CastsShadow;
	instance.visibility_mask = Set_Submesh_Visible(All_Submeshes_Visible, 1, false);
	return instance;
}

static_assert(std::is_nothrow_move_constructible_v<GPUInstanceData>);
static_assert(std::is_nothrow_move_constructible_v<GPUMeshData>);
static_assert(std::is_nothrow_move_constructible_v<GPUMaterialData>);
static_assert(std::is_nothrow_move_constructible_v<GPUDecalData>);

BOOST_AUTO_TEST_CASE(gpu_scene_packs_tables_and_translates_handles)
{
	TexturePool textures;
	SamplerPool samplers;
	MaterialPool materials;
	MeshPool meshes;

	const TextureHandle texture = textures.Create();
	const SamplerHandle sampler = samplers.Create();
	const MeshHandle unused_mesh = meshes.Create();
	const MeshHandle base_mesh = meshes.Create(Make_Mesh(100, 300));
	const MeshHandle lod_mesh = meshes.Create(Make_Mesh(20, 60));
	const MaterialHandle material = materials.Create();
	BOOST_REQUIRE(meshes.Destroy(unused_mesh));

	Mesh *base = meshes.Resolve(base_mesh);
	base->lods[0] = {lod_mesh, 0.25f};
	base->lod_count = 1;
	Material *material_resource = materials.Resolve(material);
	material_resource->textures[0] = texture;
	material_resource->samplers[0] = sampler;
	material_resource->parameters.values[0] = 3.5f;
	material_resource->flags = MaterialFlags::Unlit;

	RenderScene scene;
	const InstanceHandle instance = scene.Create(Make_Instance(base_mesh, material));
	RenderLight light;
	light.type = RenderLightType::Spot;
	light.position = {7.0f, 8.0f, 9.0f};
	light.direction = {0.0f, 0.0f, -1.0f};
	light.color = {0.2f, 0.4f, 0.8f};
	light.intensity = 6.0f;
	light.range = 25.0f;
	light.inner_angle = 0.15f;
	light.outer_angle = 0.5f;
	const LightHandle light_handle = scene.Create_Light(light);
	GPUScene gpu_scene;

	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	BOOST_REQUIRE(gpu_scene.Instances().size() == 1);
	BOOST_REQUIRE(gpu_scene.Meshes().size() == 2);
	BOOST_REQUIRE(gpu_scene.Materials().size() == 1);
	BOOST_REQUIRE(gpu_scene.Lights().size() == 1);
	BOOST_CHECK(gpu_scene.Instance_Index(instance) == 0);
	BOOST_CHECK(gpu_scene.Mesh_Index(base_mesh) == 1);
	BOOST_CHECK(gpu_scene.Mesh_Index(lod_mesh) == 0);
	BOOST_CHECK(gpu_scene.Material_Index(material) == 0);
	BOOST_CHECK(gpu_scene.Light_Index(light_handle) == 0);

	const GPUInstanceData &gpu_instance = gpu_scene.Instances()[0];
	BOOST_CHECK(gpu_instance.transform[3] == 4.0f);
	BOOST_CHECK(gpu_instance.bounds[2] == 3.0f);
	BOOST_CHECK(gpu_instance.bounds[3] == 7.0f);
	BOOST_CHECK(gpu_instance.mesh_index == 1);
	BOOST_CHECK(gpu_instance.material_index == 0);
	BOOST_CHECK(gpu_instance.flags == static_cast<std::uint32_t>(RenderInstanceFlags::CastsShadow));
	BOOST_CHECK(gpu_instance.visibility_mask == Set_Submesh_Visible(All_Submeshes_Visible, 1, false));

	const GPUMeshData &gpu_mesh = gpu_scene.Meshes()[1];
	BOOST_CHECK(gpu_mesh.vertex_count == 100);
	BOOST_CHECK(gpu_mesh.index_count == 300);
	BOOST_CHECK(gpu_mesh.lod_count == 1);
	BOOST_CHECK(gpu_mesh.lod_indices[0] == 0);
	BOOST_CHECK(gpu_mesh.lod_max_screen_sizes[0] == 0.25f);
	BOOST_CHECK(gpu_mesh.part_count == 1);
	BOOST_CHECK(gpu_scene.Mesh_Parts()[gpu_mesh.part_offset].index_count == 300);

	const GPUMaterialData &gpu_material = gpu_scene.Materials()[0];
	BOOST_CHECK(gpu_material.parameters[0] == 3.5f);
	BOOST_CHECK(gpu_material.texture_indices[0] == texture.Get_Index());
	BOOST_CHECK(gpu_material.sampler_indices[0] == sampler.Get_Index());
	BOOST_CHECK(gpu_material.flags == static_cast<std::uint32_t>(MaterialFlags::Unlit));

	const GPULightData &gpu_light = gpu_scene.Lights()[0];
	BOOST_CHECK(gpu_light.position_range[0] == 7.0f);
	BOOST_CHECK(gpu_light.position_range[3] == 25.0f);
	BOOST_CHECK(gpu_light.direction_intensity[3] == 6.0f);
	BOOST_CHECK(gpu_light.color_inner_angle[2] == 0.8f);
	BOOST_CHECK(gpu_light.color_inner_angle[3] == 0.15f);
	BOOST_CHECK(gpu_light.outer_angle == 0.5f);
	BOOST_CHECK(gpu_light.type == static_cast<std::uint32_t>(RenderLightType::Spot));
}

BOOST_AUTO_TEST_CASE(gpu_scene_syncs_only_changed_rows)
{
	TexturePool textures;
	SamplerPool samplers;
	MaterialPool materials;
	MeshPool meshes;
	const MeshHandle mesh = meshes.Create();
	const MaterialHandle material = materials.Create();

	RenderScene scene;
	const InstanceHandle first = scene.Create(Make_Instance(mesh, material));
	const InstanceHandle second = scene.Create(Make_Instance(mesh, material));
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	BOOST_REQUIRE(gpu_scene.Instance_Index(first) == 0);
	BOOST_REQUIRE(gpu_scene.Instance_Index(second) == 1);
	gpu_scene.Clear_Dirty();

	RenderInstance updated = Make_Instance(mesh, material);
	updated.transform.matrix[3] = 99.0f;
	BOOST_REQUIRE(scene.Update(second, updated));
	BOOST_REQUIRE(gpu_scene.Sync_Instance(second, scene));

	BOOST_REQUIRE(gpu_scene.Instances().size() == 2);
	BOOST_CHECK(gpu_scene.Instances()[0].transform[3] == 4.0f);
	BOOST_CHECK(gpu_scene.Instances()[1].transform[3] == 99.0f);
	BOOST_REQUIRE(gpu_scene.Dirty_Ranges().size() == 1);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].table == GPUSceneTable::Instances);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].first == 1);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].count == 1);
}

BOOST_AUTO_TEST_CASE(gpu_scene_removes_and_rejects_stale_handles)
{
	TexturePool textures;
	SamplerPool samplers;
	MaterialPool materials;
	MeshPool meshes;
	const MeshHandle mesh = meshes.Create();
	const MaterialHandle material = materials.Create();

	RenderScene scene;
	const InstanceHandle first = scene.Create(Make_Instance(mesh, material));
	const InstanceHandle second = scene.Create(Make_Instance(mesh, material));
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	gpu_scene.Clear_Dirty();

	BOOST_REQUIRE(scene.Destroy(first));
	BOOST_REQUIRE(gpu_scene.Remove_Instance(first));
	BOOST_CHECK(gpu_scene.Instance_Index(first) == Invalid_GPU_Index);
	BOOST_CHECK(gpu_scene.Instance_Index(second) == 0);
	BOOST_CHECK(gpu_scene.Instances().size() == 1);
	BOOST_CHECK(!gpu_scene.Remove_Instance(first));
	BOOST_REQUIRE(gpu_scene.Dirty_Ranges().size() == 1);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].first == 0);
}

BOOST_AUTO_TEST_CASE(gpu_scene_syncs_only_changed_light_rows)
{
	RenderScene scene;
	const LightHandle first = scene.Create_Light();
	const LightHandle second = scene.Create_Light();
	GPUScene gpu_scene;
	MeshPool meshes;
	TexturePool textures;
	SamplerPool samplers;
	MaterialPool materials;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	BOOST_REQUIRE(gpu_scene.Light_Index(first) == 0);
	BOOST_REQUIRE(gpu_scene.Light_Index(second) == 1);
	gpu_scene.Clear_Dirty();

	RenderLight updated;
	updated.type = RenderLightType::Directional;
	updated.direction = {1.0f, 0.0f, 0.0f};
	updated.intensity = 9.0f;
	BOOST_REQUIRE(scene.Update_Light(second, updated));
	BOOST_REQUIRE(gpu_scene.Sync_Light(second, scene));

	BOOST_REQUIRE(gpu_scene.Lights().size() == 2);
	BOOST_CHECK(gpu_scene.Lights()[0].type == static_cast<std::uint32_t>(RenderLightType::Point));
	BOOST_CHECK(gpu_scene.Lights()[1].type == static_cast<std::uint32_t>(RenderLightType::Directional));
	BOOST_CHECK(gpu_scene.Lights()[1].direction_intensity[0] == 1.0f);
	BOOST_CHECK(gpu_scene.Lights()[1].direction_intensity[3] == 9.0f);
	BOOST_REQUIRE(gpu_scene.Dirty_Ranges().size() == 1);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].table == GPUSceneTable::Lights);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].first == 1);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].count == 1);

	BOOST_REQUIRE(scene.Destroy_Light(first));
	BOOST_REQUIRE(gpu_scene.Remove_Light(first));
	BOOST_CHECK(gpu_scene.Light_Index(first) == Invalid_GPU_Index);
	BOOST_CHECK(gpu_scene.Light_Index(second) == 0);
	BOOST_CHECK(!gpu_scene.Remove_Light(first));
}

BOOST_AUTO_TEST_CASE(gpu_scene_links_shadow_data_to_gpu_lights)
{
	RenderScene scene;
	RenderLight directional;
	directional.type = RenderLightType::Directional;
	directional.direction = {0.0f, 0.0f, -1.0f};
	const LightHandle light = scene.Create_Light(directional);

	MeshPool meshes;
	TexturePool textures;
	SamplerPool samplers;
	MaterialPool materials;
	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	gpu_scene.Clear_Dirty();

	ShadowCascades cascades;
	cascades.light = light;
	cascades.count = 1;
	cascades.views[0].view_projection = Matrix4x4::Identity();
	cascades.views[0].split_far = 40.0f;
	BOOST_REQUIRE(cascades.Set_Shadow_Map_Index(0, ResourceIndex(8, 1)));
	BOOST_REQUIRE(gpu_scene.Sync_Shadow_Cascades(cascades));

	BOOST_REQUIRE(gpu_scene.Shadows().size() == 1);
	BOOST_CHECK(gpu_scene.Lights()[0].shadow_data_index == 0);
	BOOST_CHECK(gpu_scene.Shadows()[0].cascade_count == 1);
	BOOST_CHECK(gpu_scene.Shadows()[0].light_index == 0);
	BOOST_CHECK(gpu_scene.Shadows()[0].shadow_map_indices[0] == 8);
	BOOST_REQUIRE(gpu_scene.Dirty_Ranges().size() == 2);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].table == GPUSceneTable::Shadows);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[1].table == GPUSceneTable::Lights);

	BOOST_REQUIRE(gpu_scene.Remove_Shadow_Cascades(light));
	BOOST_CHECK(gpu_scene.Shadows().empty());
	BOOST_CHECK(gpu_scene.Lights()[0].shadow_data_index == Invalid_Shadow_Data_Index);
}

BOOST_AUTO_TEST_CASE(gpu_scene_packs_and_syncs_decal_rows)
{
	MaterialPool materials;
	MeshPool meshes;
	TexturePool textures;
	SamplerPool samplers;
	const MaterialHandle material = materials.Create();

	RenderScene scene;
	RenderDecal decal;
	decal.transform.matrix.values[3] = 14.0f;
	decal.bounds.center = {1.0f, 2.0f, 3.0f};
	decal.bounds.radius = 6.0f;
	decal.material = material;
	const DecalHandle handle = scene.Create_Decal(decal);

	GPUScene gpu_scene;
	BOOST_REQUIRE(gpu_scene.Build(scene, meshes, textures, samplers, materials));
	BOOST_REQUIRE(gpu_scene.Decals().size() == 1);
	BOOST_CHECK(gpu_scene.Decal_Index(handle) == 0);
	BOOST_CHECK(gpu_scene.Decals()[0].transform[3] == 14.0f);
	BOOST_CHECK(gpu_scene.Decals()[0].bounds[2] == 3.0f);
	BOOST_CHECK(gpu_scene.Decals()[0].bounds[3] == 6.0f);
	BOOST_CHECK(gpu_scene.Decals()[0].material_index == 0);
	BOOST_CHECK(gpu_scene.Decals()[0].flags == static_cast<std::uint32_t>(RenderDecalFlags::Enabled));

	gpu_scene.Clear_Dirty();
	decal.bounds.radius = 12.0f;
	BOOST_REQUIRE(scene.Update_Decal(handle, decal));
	BOOST_REQUIRE(gpu_scene.Sync_Decal(handle, scene));
	BOOST_CHECK(gpu_scene.Decals()[0].bounds[3] == 12.0f);
	BOOST_REQUIRE(gpu_scene.Dirty_Ranges().size() == 1);
	BOOST_CHECK(gpu_scene.Dirty_Ranges()[0].table == GPUSceneTable::Decals);

	BOOST_REQUIRE(scene.Destroy_Decal(handle));
	BOOST_REQUIRE(gpu_scene.Remove_Decal(handle));
	BOOST_CHECK(gpu_scene.Decal_Index(handle) == Invalid_GPU_Index);
	BOOST_CHECK(!gpu_scene.Remove_Decal(handle));
}
