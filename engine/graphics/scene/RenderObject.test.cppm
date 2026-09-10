module;

#define BOOST_TEST_MODULE RenderObjectNativeTests
#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

export module Graphics.Scene.RenderObject.Tests;

import Graphics.Scene.RenderObjectBounds;
import Graphics.Scene.RenderObjectDrawing;
import Graphics.Scene.RenderObjectHierarchy;
import Graphics.Scene.RenderObjectLOD;
import Graphics.Scene.RenderObjectState;
#if defined(_WIN32)
import Graphics.Renderer2D;
import Graphics.Tests.Device;
#endif

using namespace Graphics;

#if defined(_WIN32)
namespace
{

struct NativeRenderObject;

struct NativeRenderContext final
{
	Renderer2D &renderer;
	std::vector<int> &events;
};

struct NativeRenderHook final
{
	bool Pre_Render(NativeRenderObject *object, NativeRenderContext &context);
	void Post_Render(NativeRenderObject *object, NativeRenderContext &context);
};

struct NativeRenderObject final
{
	RenderObjectState state;
	NativeRenderHook *hook = nullptr;
	bool draw_succeeded = false;

	NativeRenderHook *Get_Render_Hook() noexcept { return hook; }

	void Render(NativeRenderContext &context)
	{
		context.events.push_back(2);
		const auto position = state.Position();
		draw_succeeded = context.renderer.Add_Rect(
			{position[0], position[1], position[0] + 4.0f, position[1] + 4.0f},
			{1.0f, 0.0f, 0.0f, 1.0f}, Renderer2DBlendMode::Solid);
	}
};

bool NativeRenderHook::Pre_Render(NativeRenderObject *object, NativeRenderContext &context)
{
	context.events.push_back(1);
	return object != nullptr && object->state.Has_Flag(RenderObjectFlags::Visible);
}

void NativeRenderHook::Post_Render(NativeRenderObject *object, NativeRenderContext &context)
{
	(void)object;
	context.events.push_back(3);
}

}
#endif

BOOST_AUTO_TEST_CASE(state_keeps_bitwise_affine_identity_and_invalidates_on_position_change)
{
	RenderObjectState state;
	BOOST_CHECK(!state.Is_Transform_Identity());
	state.Set_Transform(Affine_Identity());
	BOOST_CHECK(state.Is_Transform_Identity());

	auto transform = Affine_Identity();
	transform.matrix[3] = -0.0f;
	state.Set_Transform(transform);
	BOOST_CHECK(!state.Is_Transform_Identity());

	state.Set_Position({0.0f, 0.0f, 0.0f});
	BOOST_CHECK(state.Is_Transform_Identity());
	BOOST_CHECK_EQUAL(state.Position()[0], 0.0f);
	BOOST_CHECK_EQUAL(state.Position()[1], 0.0f);
	BOOST_CHECK_EQUAL(state.Position()[2], 0.0f);
}

BOOST_AUTO_TEST_CASE(bounds_cache_transforms_centers_boxes_and_object_scaled_spheres)
{
	RenderObjectBounds local;
	local.sphere = {{1.0f, 2.0f, 3.0f}, 2.0f};
	local.box = {{1.0f, 2.0f, 3.0f}, {1.0f, 2.0f, 3.0f}};
	auto transform = Affine_Identity();
	transform.matrix[0] = 0.0f;
	transform.matrix[1] = -2.0f;
	transform.matrix[4] = 3.0f;
	transform.matrix[5] = 0.0f;
	transform.matrix[3] = 10.0f;
	transform.matrix[7] = 20.0f;

	RenderObjectBoundsCache cache;
	cache.Update(transform, local, 1.5f);
	BOOST_REQUIRE(cache.Is_Valid());
	// The first row is 0*1 + -2*2 + 10, so the transformed x center is 6.
	BOOST_CHECK_EQUAL(cache.World_Sphere().center[0], 6.0f);
	BOOST_CHECK_EQUAL(cache.World_Sphere().center[1], 23.0f);
	BOOST_CHECK_EQUAL(cache.World_Sphere().center[2], 3.0f);
	BOOST_CHECK_EQUAL(cache.World_Sphere().radius, 3.0f);
	BOOST_CHECK_EQUAL(cache.World_Box().center[0], 6.0f);
	BOOST_CHECK_EQUAL(cache.World_Box().center[1], 23.0f);
	BOOST_CHECK_EQUAL(cache.World_Box().extent[0], 4.0f);
	BOOST_CHECK_EQUAL(cache.World_Box().extent[1], 3.0f);
	BOOST_CHECK_EQUAL(cache.World_Box().extent[2], 3.0f);

	cache.Invalidate();
	BOOST_CHECK(!cache.Is_Valid());
}

BOOST_AUTO_TEST_CASE(bounds_match_center_extent_transform_order_and_negative_extent_handling)
{
	RenderObjectBounds local;
	local.sphere = {{-2.0f, 1.0f, 0.5f}, 1.0f};
	local.box = {{-2.0f, 1.0f, 0.5f}, {-3.0f, 4.0f, -5.0f}};
	auto transform = Affine_Identity();
	transform.matrix[0] = -2.0f;
	transform.matrix[1] = 3.0f;
	transform.matrix[2] = 4.0f;
	transform.matrix[4] = 5.0f;
	transform.matrix[5] = -6.0f;
	transform.matrix[6] = 7.0f;
	transform.matrix[8] = -8.0f;
	transform.matrix[9] = 9.0f;
	transform.matrix[10] = -10.0f;
	transform.matrix[3] = 11.0f;
	transform.matrix[7] = -12.0f;
	transform.matrix[11] = 13.0f;

	const RenderObjectBox world = Transform_Render_Object_Box(transform, local.box);
	// Matrix3D::Transform_Center_Extent_AABox starts with the translation and
	// adds each row in order; fabs applies to every matrix*extent product.
	BOOST_CHECK_EQUAL(world.center[0], 20.0f);
	BOOST_CHECK_EQUAL(world.center[1], -24.5f);
	BOOST_CHECK_EQUAL(world.center[2], 33.0f);
	BOOST_CHECK_EQUAL(world.extent[0], 38.0f);
	BOOST_CHECK_EQUAL(world.extent[1], 74.0f);
	BOOST_CHECK_EQUAL(world.extent[2], 110.0f);
}

BOOST_AUTO_TEST_CASE(child_lookup_checks_full_names_before_suffixes_and_releases_nonmatches)
{
	struct Child final {
		std::string name;
		RenderObjectFlags flags = RenderObjectFlags::None;
		int references = 0;
	};
	std::array<Child, 3> children{
		Child{"vehicle.Turret", RenderObjectFlags::Alpha, 0},
		Child{"Turret", RenderObjectFlags::Additive, 0},
		Child{"wheel", RenderObjectFlags::Translucent, 0}
	};

	int index = -1;
	Child *found = Find_Render_Object_Child_By_Name(
		children.size(), "TURRET",
		[&](std::size_t child_index) {
			++children[child_index].references;
			return &children[child_index];
		},
		[](Child *child) { --child->references; },
		[](const Child &child) { return std::string_view(child.name); },
		&index);
	BOOST_REQUIRE(found != nullptr);
	BOOST_CHECK_EQUAL(index, 1);
	BOOST_CHECK_EQUAL(found->name, "Turret");
	BOOST_CHECK_EQUAL(children[0].references, 0);
	BOOST_CHECK_EQUAL(children[1].references, 1);
	--found->references;
	BOOST_CHECK_EQUAL(children[1].references, 0);

	// With no full-name match, the suffix pass finds vehicle.Turret.
	children[1].name = "vehicle.Gun";
	index = -1;
	found = Find_Render_Object_Child_By_Name(
		children.size(), "TURRET",
		[&](std::size_t child_index) {
			++children[child_index].references;
			return &children[child_index];
		},
		[](Child *child) { --child->references; },
		[](const Child &child) { return std::string_view(child.name); },
		&index);
	BOOST_REQUIRE(found != nullptr);
	BOOST_CHECK_EQUAL(index, 0);
	BOOST_CHECK_EQUAL(found->name, "vehicle.Turret");
	BOOST_CHECK_EQUAL(children[0].references, 1);
	BOOST_CHECK_EQUAL(children[1].references, 0);
	BOOST_CHECK_EQUAL(children[2].references, 0);
	--found->references;
	BOOST_CHECK_EQUAL(children[0].references, 0);

	found = Find_Render_Object_Child_By_Name(
		children.size(), "WHEEL",
		[&](std::size_t child_index) {
			++children[child_index].references;
			return &children[child_index];
		},
		[](Child *child) { --child->references; },
		[](const Child &child) { return std::string_view(child.name); });
	BOOST_REQUIRE(found != nullptr);
	BOOST_CHECK_EQUAL(found->name, "wheel");
	BOOST_CHECK_EQUAL(children[2].references, 1);
	--found->references;
	for (const Child &child : children)
		BOOST_CHECK_EQUAL(child.references, 0);
}

BOOST_AUTO_TEST_CASE(child_visual_flags_aggregate_without_game_query_masks)
{
	struct Child final {
		RenderObjectFlags flags;
	};
	const std::array<Child, 3> children{{
		{RenderObjectFlags::Alpha},
		{RenderObjectFlags::Additive | RenderObjectFlags::ForceVisible},
		{RenderObjectFlags::Translucent}
	}};
	const RenderObjectFlags result = Aggregate_Render_Object_Child_Flags(
		children.size(),
		[&](std::size_t index) { return &children[index]; },
		[](const Child *) {},
		[](const Child &child) { return child.flags; });
	BOOST_CHECK(Has_Render_Object_Flag(result, RenderObjectFlags::Alpha));
	BOOST_CHECK(Has_Render_Object_Flag(result, RenderObjectFlags::Additive));
	BOOST_CHECK(Has_Render_Object_Flag(result, RenderObjectFlags::Translucent));
	BOOST_CHECK(!Has_Render_Object_Flag(result, RenderObjectFlags::ForceVisible));
}

BOOST_AUTO_TEST_CASE(transform_validation_climbs_to_one_root_update)
{
	struct Node final {
		Node *parent = nullptr;
		bool dirty = false;
		int updates = 0;
		int id = 0;
	};
	Node root{nullptr, false, 0, 0};
	Node middle{&root, true, 0, 1};
	Node child_parent{&middle, false, 0, 2};
	std::vector<int> inspected;

	bool refreshed = false;
	const bool dirty = Validate_Render_Object_Transform(
		&middle,
		[&](Node *node) {
			inspected.push_back(node->id);
			return node->dirty;
		},
		[](Node *node) { return node->parent; },
		[](Node *node) { ++node->updates; },
		[&]() { refreshed = true; });

	BOOST_CHECK(dirty);
	BOOST_CHECK(refreshed);
	BOOST_CHECK_EQUAL(root.updates, 1);
	BOOST_CHECK(inspected == std::vector<int>({1, 1}));
}

BOOST_AUTO_TEST_CASE(draw_extraction_preserves_hook_order_and_vetoes_render)
{
	struct Context final {
		std::vector<int> events;
	};
	struct Object;
	struct Hook final {
		bool allow = true;
		bool Pre_Render(Object *, Context &context)
		{
			context.events.push_back(1);
			return allow;
		}
		void Post_Render(Object *, Context &context)
		{
			context.events.push_back(3);
		}
	};
	struct Object final {
		Hook *hook = nullptr;
		bool rendered = false;
		Hook *Get_Render_Hook() { return hook; }
		void Render(Context &context)
		{
			context.events.push_back(2);
			rendered = true;
		}
	};

	Context context;
	Hook hook;
	Object object{&hook, false};
	BOOST_CHECK(Extract_Render_Object_Draw(object, context));
	BOOST_CHECK(object.rendered);
	BOOST_CHECK(context.events == std::vector<int>({1, 2, 3}));

	context.events.clear();
	object.rendered = false;
	hook.allow = false;
	BOOST_CHECK(!Extract_Render_Object_Draw(object, context));
	BOOST_CHECK(!object.rendered);
	BOOST_CHECK(context.events == std::vector<int>({1, 3}));
}

BOOST_AUTO_TEST_CASE(base_lod_uses_small_nonzero_cost_and_terminal_values)
{
	std::array<float, 2> values{};
	std::array<float, 1> costs{};
	BOOST_CHECK_EQUAL(Calculate_Base_Render_Object_LOD(0.0f, values, costs), 0);
	BOOST_CHECK_EQUAL(costs[0], 0.000001f);
	BOOST_CHECK_EQUAL(values[0], Render_Object_Min_LOD_Value);
	BOOST_CHECK_EQUAL(values[1], Render_Object_Max_LOD_Value);

	BOOST_CHECK_EQUAL(Calculate_Base_Render_Object_LOD(12.0f, values, costs), 0);
	BOOST_CHECK_EQUAL(costs[0], 12.0f);
}

#if defined(_WIN32)
BOOST_AUTO_TEST_CASE(native_state_and_hook_submit_a_gpu_draw)
{
	GraphicsTestDeviceOptions options;
	options.use_warp = true;
	options.shader_directory = GRAPHICS_TERRAIN_SHADER_DIRECTORY;
	GraphicsTestDevice device(options);
	BOOST_REQUIRE(device.Is_Valid());

	Renderer2D renderer;
	BOOST_REQUIRE(renderer.Initialize(
		device, Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
	const auto target = device.Create_Texture({
		8, 8, 1, RHITextureFormat::RGBA8_UNorm,
		static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
	const auto depth = device.Create_Texture({
		8, 8, 1, RHITextureFormat::D32_Float,
		static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
	BOOST_REQUIRE(target.Is_Valid());
	BOOST_REQUIRE(depth.Is_Valid());

	auto &commands = device.Immediate_Command_List();
	BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
	BOOST_REQUIRE(commands.Set_Viewport({0, 0, 8, 8}));
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 1}, 1));

	NativeRenderHook hook;
	NativeRenderObject object;
	object.hook = &hook;
	object.state.Set_Flag(RenderObjectFlags::Visible, true);
	object.state.Set_Position({2.0f, 2.0f, 0.0f});
	std::vector<int> events;
	NativeRenderContext context{renderer, events};
	renderer.Begin(8, 8);
	BOOST_REQUIRE(Extract_Render_Object_Draw(object, context));
	BOOST_REQUIRE(object.draw_succeeded);
	BOOST_CHECK(events == std::vector<int>({1, 2, 3}));
	BOOST_REQUIRE(renderer.Execute(device, commands, target, depth, {0, 0, 8, 8}));

	std::array<std::byte, 8 * 8 * 4> pixels{};
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 8 * 4));
	const std::size_t center = (4 * 8 + 4) * 4;
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center]), 255u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center + 1]), 0u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center + 2]), 0u);

	// A veto still runs Post_Render but must leave the target at its cleared
	// color. This exercises the native hook adapter through a real GPU pass.
	events.clear();
	object.draw_succeeded = false;
	object.state.Set_Flag(RenderObjectFlags::Visible, false);
	BOOST_REQUIRE(commands.Clear({0, 0, 0, 1}, 1));
	renderer.Begin(8, 8);
	BOOST_CHECK(!Extract_Render_Object_Draw(object, context));
	BOOST_CHECK(!object.draw_succeeded);
	BOOST_CHECK(events == std::vector<int>({1, 3}));
	BOOST_REQUIRE(renderer.Execute(device, commands, target, depth, {0, 0, 8, 8}));

	pixels.fill(std::byte{0xff});
	BOOST_REQUIRE(device.Readback_Texture(target, pixels, 8 * 4));
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center]), 0u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center + 1]), 0u);
	BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[center + 2]), 0u);

	renderer.Shutdown();
	device.Destroy_Texture(target);
	device.Destroy_Texture(depth);
}
#endif
