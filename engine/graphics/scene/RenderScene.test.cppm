module;

#define BOOST_TEST_MODULE GraphicsRenderSceneTests

#include <boost/test/included/unit_test.hpp>

#include <cstdint>
#include <type_traits>

export module Graphics.Scene.RenderScene.Tests;

import Graphics.Scene.RenderScene;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<RenderInstance>);
static_assert(std::is_nothrow_move_assignable_v<RenderInstance>);

BOOST_AUTO_TEST_CASE(render_scene_exposes_dense_soa_data_and_proxy_views)
{
	RenderScene scene;
	RenderInstance instance;
	instance.transform.matrix[0] = 1.0f;
	instance.bounds.center = {1.0f, 2.0f, 3.0f};
	instance.bounds.radius = 4.0f;
	instance.mesh = MeshHandle(2, 1);
	instance.material = MaterialHandle(3, 1);
	instance.flags = RenderInstanceFlags::CastsShadow;

	const InstanceHandle handle = scene.Create(instance);
	const RenderSceneData data = scene.Data();

	BOOST_REQUIRE(data.Size() == 1);
	BOOST_CHECK(data.transforms[0].matrix[0] == 1.0f);
	BOOST_CHECK(data.bounds[0].center[1] == 2.0f);
	BOOST_CHECK(data.bounds[0].radius == 4.0f);
	BOOST_CHECK(data.world_bounds[0].center_x == 1.0f);
	BOOST_CHECK(data.world_bounds[0].center_y == 0.0f);
	BOOST_CHECK(data.world_bounds[0].center_z == 0.0f);
	BOOST_CHECK(data.world_bounds[0].radius == 4.0f);
	BOOST_CHECK(data.meshes[0] == instance.mesh);
	BOOST_CHECK(data.materials[0] == instance.material);
	BOOST_CHECK(data.flags[0] == RenderInstanceFlags::CastsShadow);
	BOOST_CHECK(data.visibility_masks[0] == instance.visibility_mask);
	BOOST_CHECK(data.handles[0] == handle);
	BOOST_CHECK((reinterpret_cast<std::uintptr_t>(data.transforms.elements[0].data()) % 16u) == 0u);
	BOOST_CHECK((reinterpret_cast<std::uintptr_t>(data.bounds.center[0].data()) % 16u) == 0u);
	BOOST_CHECK((reinterpret_cast<std::uintptr_t>(data.world_bounds.center_x.data()) % 16u) == 0u);
	BOOST_CHECK(data.transforms.elements[0].data() + 0 == &data.transforms.elements[0][0]);
	BOOST_CHECK(data.bounds.center[0].data() + 0 == &data.bounds.center[0][0]);
	BOOST_CHECK(data.world_bounds.center_x.data() + 0 == &data.world_bounds.center_x[0]);

	bool visited = false;
	BOOST_REQUIRE(scene.Visit(handle, [&](InstanceHandle visited_handle, const RenderInstanceView &view) noexcept {
		visited = visited_handle == handle
			&& view.transform.matrix[0] == 1.0f
			&& view.bounds.center[1] == 2.0f
			&& view.mesh == instance.mesh
			&& view.material == instance.material
			&& view.flags == RenderInstanceFlags::CastsShadow
			&& view.visibility_mask == instance.visibility_mask;
	}));
	BOOST_CHECK(visited);
}

BOOST_AUTO_TEST_CASE(render_scene_destroys_instances)
{
	RenderScene scene;
	const InstanceHandle handle = scene.Create();

	BOOST_REQUIRE(scene.Destroy(handle));
	BOOST_CHECK(scene.Dense_Index(handle) == Invalid_Render_Scene_Index);
	BOOST_CHECK(scene.Size() == 0);
	BOOST_CHECK(!scene.Visit(handle, [](InstanceHandle, const RenderInstanceView &) noexcept {}));
}

BOOST_AUTO_TEST_CASE(render_scene_rejects_stale_handles_and_reuses_slots)
{
	RenderScene scene;
	const InstanceHandle old_handle = scene.Create();

	BOOST_REQUIRE(scene.Destroy(old_handle));
	const InstanceHandle new_handle = scene.Create();

	BOOST_CHECK(scene.Dense_Index(old_handle) == Invalid_Render_Scene_Index);
	BOOST_CHECK(!scene.Destroy(old_handle));
	BOOST_CHECK(scene.Dense_Index(new_handle) == 0);
}

BOOST_AUTO_TEST_CASE(render_scene_updates_soa_columns)
{
	RenderScene scene;
	const InstanceHandle handle = scene.Create();
	RenderInstance updated_instance;
	updated_instance.transform.matrix[5] = 2.0f;
	updated_instance.bounds.radius = 8.0f;
	updated_instance.mesh = MeshHandle(4, 1);
	updated_instance.material = MaterialHandle(5, 1);
	updated_instance.flags = RenderInstanceFlags::ReceivesShadow;

	BOOST_CHECK(scene.Update(handle, updated_instance));

	const RenderSceneData data = scene.Data();
	BOOST_REQUIRE(data.Size() == 1);
	BOOST_CHECK(data.transforms[0].matrix[5] == 2.0f);
	BOOST_CHECK(data.bounds[0].radius == 8.0f);
	BOOST_CHECK(data.world_bounds[0].radius == 16.0f);
	BOOST_CHECK(data.meshes[0] == updated_instance.mesh);
	BOOST_CHECK(data.materials[0] == updated_instance.material);
	BOOST_CHECK(data.flags[0] == RenderInstanceFlags::ReceivesShadow);
}

BOOST_AUTO_TEST_CASE(render_scene_updates_submesh_visibility_without_replacing_instance_state)
{
	RenderScene scene;
	RenderInstance instance;
	instance.transform.matrix[3] = 7.0f;
	instance.bounds.radius = 3.0f;
	instance.mesh = MeshHandle(4, 1);
	instance.material = MaterialHandle(5, 1);
	instance.flags = RenderInstanceFlags::ReceivesShadow;
	const InstanceHandle handle = scene.Create(instance);

	const SubmeshVisibilityMask hidden_part_mask = Set_Submesh_Visible(All_Submeshes_Visible, 2, false);
	BOOST_REQUIRE(scene.Update_Visibility(handle, hidden_part_mask));

	const RenderSceneData data = scene.Data();
	BOOST_REQUIRE(data.Size() == 1);
	BOOST_CHECK(data.transforms[0].matrix[3] == 7.0f);
	BOOST_CHECK(data.bounds[0].radius == 3.0f);
	BOOST_CHECK(data.meshes[0] == instance.mesh);
	BOOST_CHECK(data.materials[0] == instance.material);
	BOOST_CHECK(data.flags[0] == instance.flags);
	BOOST_CHECK(data.visibility_masks[0] == hidden_part_mask);
	BOOST_CHECK(!scene.Update_Visibility(InstanceHandle(handle.Get_Index(), handle.Get_Generation() + 1), All_Submeshes_Visible));
}

BOOST_AUTO_TEST_CASE(render_scene_updates_shadow_casting_without_replacing_instance_state)
{
	RenderScene scene;
	RenderInstance instance;
	instance.mesh = MeshHandle(4, 1);
	instance.material = MaterialHandle(5, 1);
	instance.flags = RenderInstanceFlags::CastsShadow | RenderInstanceFlags::ReceivesShadow;
	instance.visibility_mask = Set_Submesh_Visible(All_Submeshes_Visible, 3, false);
	const InstanceHandle handle = scene.Create(instance);

	BOOST_REQUIRE(scene.Update_Shadow_Casting(handle, false));
	const RenderSceneData data = scene.Data();
	BOOST_REQUIRE(data.Size() == 1);
	BOOST_CHECK(!Render_Instance_Casts_Shadow(data.flags[0]));
	BOOST_CHECK(data.meshes[0] == instance.mesh);
	BOOST_CHECK(data.materials[0] == instance.material);
	BOOST_CHECK(data.visibility_masks[0] == instance.visibility_mask);

	BOOST_REQUIRE(scene.Update_Shadow_Casting(handle, true));
	BOOST_CHECK(Render_Instance_Casts_Shadow(scene.Data().flags[0]));
	BOOST_CHECK(!scene.Update_Shadow_Casting(InstanceHandle(handle.Get_Index(), handle.Get_Generation() + 1), false));
}

BOOST_AUTO_TEST_CASE(render_scene_keeps_instance_handle_when_static_variant_changes)
{
	RenderScene scene;
	RenderInstance first;
	first.mesh = MeshHandle(1, 1);
	first.material = MaterialHandle(2, 1);
	first.flags = RenderInstanceFlags::CastsShadow;
	const InstanceHandle handle = scene.Create(first);

	RenderInstance variant = first;
	variant.mesh = MeshHandle(4, 1);
	variant.material = MaterialHandle(5, 1);
	variant.flags = RenderInstanceFlags::ReceivesShadow;

	BOOST_REQUIRE(scene.Update(handle, variant));
	BOOST_CHECK(scene.Dense_Index(handle) == 0);
	BOOST_REQUIRE(scene.Visit(handle, [&](InstanceHandle visited_handle, const RenderInstanceView &view) noexcept {
		BOOST_CHECK(visited_handle == handle);
		BOOST_CHECK(view.mesh == variant.mesh);
		BOOST_CHECK(view.material == variant.material);
		BOOST_CHECK(view.flags == variant.flags);
	}));
}

BOOST_AUTO_TEST_CASE(render_scene_stores_lights_in_soa_columns)
{
	RenderScene scene;
	RenderLight light;
	light.type = RenderLightType::Spot;
	light.position = {10.0f, 20.0f, 30.0f};
	light.direction = {0.0f, 1.0f, -1.0f};
	light.color = {0.25f, 0.5f, 0.75f};
	light.intensity = 12.0f;
	light.range = 40.0f;
	light.inner_angle = 0.2f;
	light.outer_angle = 0.6f;
	const LightHandle handle = scene.Create_Light(light);
	const RenderLightData data = scene.Lights();

	BOOST_REQUIRE(data.Size() == 1);
	BOOST_CHECK(data.types[0] == RenderLightType::Spot);
	BOOST_CHECK(data.position_x[0] == 10.0f);
	BOOST_CHECK(data.position_y[0] == 20.0f);
	BOOST_CHECK(data.position_z[0] == 30.0f);
	BOOST_CHECK(data.direction_y[0] == 1.0f);
	BOOST_CHECK(data.color_b[0] == 0.75f);
	BOOST_CHECK(data.intensities[0] == 12.0f);
	BOOST_CHECK(data.ranges[0] == 40.0f);
	BOOST_CHECK(data.inner_angles[0] == 0.2f);
	BOOST_CHECK(data.outer_angles[0] == 0.6f);
	BOOST_CHECK(data.handles[0] == handle);

	bool visited = false;
	BOOST_REQUIRE(scene.Visit_Light(handle, [&](LightHandle visited_handle, const RenderLightView &view) noexcept {
		visited = visited_handle == handle
			&& view.type == RenderLightType::Spot
			&& view.position.x == 10.0f
			&& view.direction.z == -1.0f
			&& view.color.y == 0.5f
			&& view.intensity == 12.0f;
	}));
	BOOST_CHECK(visited);
}

BOOST_AUTO_TEST_CASE(render_scene_updates_destroys_and_rejects_stale_lights)
{
	RenderScene scene;
	const LightHandle old_handle = scene.Create_Light();
	RenderLight updated;
	updated.type = RenderLightType::Directional;
	updated.direction = {1.0f, 0.0f, 0.0f};
	updated.intensity = 4.0f;

	BOOST_REQUIRE(scene.Update_Light(old_handle, updated));
	BOOST_CHECK(scene.Lights().types[0] == RenderLightType::Directional);
	BOOST_CHECK(scene.Lights().direction_x[0] == 1.0f);
	BOOST_CHECK(scene.Lights().intensities[0] == 4.0f);
	BOOST_REQUIRE(scene.Destroy_Light(old_handle));
	BOOST_CHECK(scene.Dense_Light_Index(old_handle) == Invalid_Render_Scene_Index);
	BOOST_CHECK(!scene.Update_Light(old_handle, updated));
	BOOST_CHECK(!scene.Destroy_Light(old_handle));

	const LightHandle new_handle = scene.Create_Light();
	BOOST_CHECK(new_handle.Get_Index() == old_handle.Get_Index());
	BOOST_CHECK(new_handle != old_handle);
	BOOST_CHECK(scene.Dense_Light_Index(new_handle) == 0);
}
