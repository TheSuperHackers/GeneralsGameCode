module;

#define BOOST_TEST_MODULE SceneTraversalDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

export module Graphics.Scene.TraversalDrawing.Tests;

import Graphics.Scene.RenderObjectDrawing;
import Graphics.Scene.Traversal;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;

using namespace Graphics;

namespace
{

struct Camera final
{
};

struct DrawContext final
{
	LocalLighting *light_environment = nullptr;
	PropRenderer *renderer = nullptr;
	CommandList *commands = nullptr;
};

struct Object;

struct Hook final
{
	std::vector<unsigned> *events = nullptr;
	bool veto = false;

	template<class ObjectType, class Context>
	bool Pre_Render(ObjectType *object, Context &) noexcept
	{
		events->push_back(10u + object->id);
		return !veto;
	}

	template<class ObjectType, class Context>
	void Post_Render(ObjectType *object, Context &) noexcept
	{
		events->push_back(20u + object->id);
	}
};

struct Object final : SceneListMember
{
	Object(unsigned id, std::vector<unsigned> &events)
		: id(id), events(events), hook{&events, false}
	{
	}

	void Add_Ref() noexcept
	{
		++references;
	}

	void Release_Ref() noexcept
	{
		--references;
	}

	Hook *Get_Render_Hook() noexcept
	{
		return &hook;
	}

	void Render(DrawContext &context) noexcept
	{
		events.push_back(30u + id);
		++render_calls;
		draw_succeeded = context.renderer->Draw(*context.commands, mesh, style, parameters, {});
	}

	unsigned id;
	std::vector<unsigned> &events;
	Hook hook;
	PropMeshHandle mesh{};
	PropStyle style{};
	PropParameters parameters{};
	unsigned references = 1;
	unsigned render_calls = 0;
	bool draw_succeeded = false;
	bool culled = false;
	bool visible = false;
};

bool Is_Force_Visible(void *, const Object *) noexcept
{
	return false;
}

bool Is_Culled(void *, const Object *object, const Camera *) noexcept
{
	return object->culled;
}

void Set_Visible(void *, Object *object, bool visible) noexcept
{
	object->visible = visible;
}

bool Is_Really_Visible(void *, Object *object) noexcept
{
	return object->visible;
}

SceneDrawResult Draw_Object(void *, Object *object, DrawContext *context) noexcept
{
	return Extract_Render_Object_Draw(*object, *context)
		? SceneDrawResult::Submitted
		: SceneDrawResult::Skipped;
}

PropMeshHandle Make_Quad(PropRenderer &renderer, float left, float right,
	std::array<float, 4> color)
{
	std::array<PropVertex, 4> vertices{};
	vertices[0].position = {left, -1.0f, 0.5f};
	vertices[1].position = {right, -1.0f, 0.5f};
	vertices[2].position = {right, 1.0f, 0.5f};
	vertices[3].position = {left, 1.0f, 0.5f};
	for (PropVertex &vertex : vertices)
		vertex.color = color;
	return renderer.Create_Mesh(vertices, std::array<std::uint32_t, 6>{0, 1, 2, 0, 2, 3});
}

}

BOOST_AUTO_TEST_CASE(scene_traversal_preserves_visibility_hook_order_and_draws_only_accepted_objects)
{
	for (const bool use_warp : {true, false}) {
		GraphicsTestDevice device({use_warp});
		if (!device.Is_Valid())
			continue;

		PropRenderer renderer;
		BOOST_REQUIRE(renderer.Initialize(device,
			Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
		constexpr std::uint32_t size = 16;
		const auto target = device.Create_Texture({size, size, 1, RHITextureFormat::RGBA8_UNorm,
			static_cast<unsigned>(RHITextureUsage::RenderTarget)});
		const auto depth = device.Create_Texture({size, size, 1, RHITextureFormat::D24_UNorm_S8,
			static_cast<unsigned>(RHITextureUsage::DepthStencil)});
		BOOST_REQUIRE(target.Is_Valid());
		BOOST_REQUIRE(depth.Is_Valid());
		auto &commands = device.Immediate_Command_List();
		BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
		BOOST_REQUIRE(commands.Set_Viewport({0, 0, size, size}));
		BOOST_REQUIRE(commands.Clear({0, 0, 0, 0}, 1));

		std::vector<unsigned> events;
		Object accepted(1, events);
		Object vetoed(2, events);
		Object culled(3, events);
		accepted.mesh = Make_Quad(renderer, -1.0f, 0.0f, {1, 0, 0, 1});
		vetoed.mesh = Make_Quad(renderer, 0.0f, 1.0f, {0, 1, 0, 1});
		culled.mesh = Make_Quad(renderer, -1.0f, 1.0f, {0, 0, 1, 1});
		BOOST_REQUIRE(accepted.mesh.Is_Valid());
		BOOST_REQUIRE(vetoed.mesh.Is_Valid());
		BOOST_REQUIRE(culled.mesh.Is_Valid());
		for (Object *object : {&accepted, &vetoed, &culled}) {
			object->style.blend = RHIBlendMode::Disabled;
			object->style.depth_test = false;
			object->style.depth_write = false;
			object->style.cull = RHICullMode::None;
			object->parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
				0, 0, 1, 0, 0, 0, 0, 1};
			object->parameters.textured = 0;
		}
		vetoed.hook.veto = true;
		culled.culled = true;

		SceneTraversal<Object> traversal;
		BOOST_REQUIRE(traversal.Add_Render_Object(&accepted));
		BOOST_REQUIRE(traversal.Add_Render_Object(&vetoed));
		BOOST_REQUIRE(traversal.Add_Render_Object(&culled));

		SceneVisibilityCallbacks<Object, Camera> visibility;
		visibility.is_force_visible = Is_Force_Visible;
		visibility.is_culled = Is_Culled;
		visibility.set_visible = Set_Visible;
		visibility.is_really_visible = Is_Really_Visible;
		SceneRenderCallbacks<Object, DrawContext> render;
		render.is_really_visible = Is_Really_Visible;
		render.draw = Draw_Object;

		DrawContext context;
		context.renderer = &renderer;
		context.commands = &commands;
		Camera camera;
		BOOST_REQUIRE(traversal.Render(camera, context, visibility, render));
		BOOST_CHECK(context.light_environment != nullptr);
		BOOST_CHECK(accepted.visible);
		BOOST_CHECK(vetoed.visible);
		BOOST_CHECK_EQUAL(accepted.render_calls, 1u);
		BOOST_CHECK_EQUAL(vetoed.render_calls, 0u);
		BOOST_CHECK(accepted.draw_succeeded);
		BOOST_CHECK(!culled.visible);
		BOOST_CHECK_EQUAL(culled.render_calls, 0u);
		BOOST_CHECK(!culled.draw_succeeded);

		const std::vector<unsigned> expected{12u, 22u, 11u, 31u, 21u};
		BOOST_CHECK(events == expected);
		std::array<std::byte, size * size * 4> pixels{};
		BOOST_REQUIRE(device.Readback_Texture(target, pixels, size * 4));
		for (unsigned channel = 0; channel < 4; ++channel) {
			BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8 * size + 4) * 4 + channel]),
				channel == 0 || channel == 3 ? 255u : 0u);
			BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[(8 * size + 12) * 4 + channel]), 0u);
		}

		BOOST_REQUIRE(renderer.Destroy_Mesh(accepted.mesh));
		BOOST_REQUIRE(renderer.Destroy_Mesh(vetoed.mesh));
		BOOST_REQUIRE(renderer.Destroy_Mesh(culled.mesh));
		renderer.Shutdown();
		device.Destroy_Texture(target);
		device.Destroy_Texture(depth);
	}
}
