module;

#define BOOST_TEST_MODULE SceneTraversalTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <vector>

export module Graphics.Scene.Traversal.Tests;

import Graphics.Scene.Traversal;

using namespace Graphics;

namespace
{

struct Camera final
{
	unsigned view = 0;
};

struct Object final : SceneListMember
{
	explicit Object(unsigned id, std::vector<unsigned> &events)
		: id(id), events(events)
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

	unsigned id;
	std::vector<unsigned> &events;
	unsigned references = 1;
	bool force_visible = false;
	bool culled = false;
	bool visible = false;
	bool ignore_lod = false;
	unsigned references_at_release = 0;
};

struct Context final
{
	LocalLighting *light_environment = nullptr;
};

struct CallbackState final
{
	SceneTraversal<Object> *traversal = nullptr;
	std::vector<unsigned> *events = nullptr;
};

bool Is_Force_Visible(void *, const Object *object) noexcept
{
	return object->force_visible;
}

bool Is_Culled(void *, const Object *object, const Camera *camera) noexcept
{
	return object->culled || camera->view != object->id;
}

void Set_Visible(void *raw, Object *object, bool visible) noexcept
{
	const auto &state = *static_cast<const CallbackState *>(raw);
	object->visible = visible;
	state.events->push_back(100u + object->id);
}

bool Is_Really_Visible(void *, Object *object) noexcept
{
	return object->visible;
}

bool Is_Ignoring_LOD_Cost(void *, Object *object) noexcept
{
	return object->ignore_lod;
}

void Prepare_LOD(void *raw, Object *object, Camera *) noexcept
{
	const auto &state = *static_cast<const CallbackState *>(raw);
	state.events->push_back(200u + object->id);
}

void Update(void *raw, Object *object) noexcept
{
	const auto &state = *static_cast<const CallbackState *>(raw);
	state.events->push_back(300u + object->id);
}

SceneDrawResult Draw(void *raw, Object *object, Context *) noexcept
{
	const auto &state = *static_cast<const CallbackState *>(raw);
	state.events->push_back(400u + object->id);
	return SceneDrawResult::Submitted;
}

bool Describe_Light(void *raw, const Object *object, MaterialLightSource &source) noexcept
{
	const auto &state = *static_cast<const CallbackState *>(raw);
	state.events->push_back(500u + object->id);
	source.type = RenderLightType::Directional;
	source.direction = {0.0f, 0.0f, -1.0f};
	source.diffuse = object->id == 1 ? std::array<float, 3>{1.0f, 0.0f, 0.0f}
		: std::array<float, 3>{0.0f, 1.0f, 0.0f};
	return true;
}

void Detach(void *raw, Object *object) noexcept
{
	const auto &state = *static_cast<const CallbackState *>(raw);
	object->references_at_release = object->references;
	state.events->push_back(600u + object->id);
}

void Removed(void *raw, Object *object) noexcept
{
	const auto &state = *static_cast<const CallbackState *>(raw);
	state.events->push_back(900u + object->id);
}

}

BOOST_AUTO_TEST_CASE(registration_visibility_updates_lighting_and_render_order_are_retained)
{
	std::vector<unsigned> events;
	CallbackState state{nullptr, &events};
	SceneTraversal<Object> traversal;
	state.traversal = &traversal;
	Object first(1, events);
	Object second(2, events);
	first.force_visible = true;
	second.ignore_lod = true;

	BOOST_REQUIRE(traversal.Add_Render_Object(&first));
	BOOST_REQUIRE(traversal.Add_Render_Object(&second));
	BOOST_REQUIRE(traversal.Register(&first, SceneRegistration::On_Frame_Update));
	BOOST_REQUIRE(traversal.Register(&second, SceneRegistration::On_Frame_Update));
	BOOST_REQUIRE(traversal.Register(&first, SceneRegistration::Light));
	BOOST_REQUIRE(traversal.Register(&second, SceneRegistration::Light));

	SceneVisibilityCallbacks<Object, Camera> visibility;
	visibility.context = &state;
	visibility.is_force_visible = Is_Force_Visible;
	visibility.is_culled = Is_Culled;
	visibility.set_visible = Set_Visible;
	visibility.is_really_visible = Is_Really_Visible;
	visibility.is_ignoring_lod_cost = Is_Ignoring_LOD_Cost;
	visibility.prepare_lod = Prepare_LOD;
	SceneRenderCallbacks<Object, Context> render;
	render.context = &state;
	render.on_frame_update = Update;
	render.is_really_visible = Is_Really_Visible;
	render.draw = Draw;
	SceneLightingCallbacks<Object> lighting;
	lighting.context = &state;
	lighting.describe_light = Describe_Light;

	Camera first_view{1};
	Context context;
	BOOST_REQUIRE(traversal.Check_Visibility(first_view, visibility));
	BOOST_CHECK(traversal.Visibility_Checked());
	BOOST_REQUIRE(traversal.Render(first_view, context, visibility, render, lighting));
	BOOST_CHECK(!traversal.Visibility_Checked());
	BOOST_CHECK(first.visible);
	BOOST_CHECK(!second.visible);
	BOOST_CHECK(context.light_environment != nullptr);
	BOOST_CHECK_EQUAL(context.light_environment->count, 2u);

	// Add inserts at the head, updates do the same, and lights append. LOD
	// preparation is after visibility and skips the ignored object.
	const std::vector<unsigned> expected{
		102u, 101u, 201u, 302u, 301u, 501u, 502u, 401u
	};
	BOOST_CHECK(events == expected);

	// A second camera cannot reuse the first camera's visibility result.
	events.clear();
	first.force_visible = false;
	Camera second_view{2};
	BOOST_REQUIRE(traversal.Render(second_view, context, visibility, render, lighting));
	BOOST_CHECK(second.visible);
	BOOST_CHECK(!first.visible);
	BOOST_CHECK(events.front() == 102u);
	BOOST_CHECK(events[1] == 101u);

	BOOST_REQUIRE(traversal.Unregister(&first, SceneRegistration::On_Frame_Update));
	BOOST_REQUIRE(traversal.Unregister(&second, SceneRegistration::On_Frame_Update));
	BOOST_REQUIRE(traversal.Unregister(&first, SceneRegistration::Light));
	BOOST_REQUIRE(traversal.Unregister(&second, SceneRegistration::Light));
}

BOOST_AUTO_TEST_CASE(release_processing_detaches_every_object_before_releasing_list_references)
{
	std::vector<unsigned> events;
	CallbackState state{nullptr, &events};
	SceneTraversal<Object> traversal;
	Object first(1, events);
	Object second(2, events);
	BOOST_REQUIRE(traversal.Register(&first, SceneRegistration::Release));
	BOOST_REQUIRE(traversal.Register(&second, SceneRegistration::Release));

	SceneReleaseCallbacks<Object> release;
	release.context = &state;
	release.detach = Detach;
	traversal.Process_Releases(release);

	const std::vector<unsigned> expected_events{602u, 601u};
	BOOST_CHECK(events == expected_events);
	BOOST_CHECK_EQUAL(first.references_at_release, 2u);
	BOOST_CHECK_EQUAL(second.references_at_release, 2u);
	BOOST_CHECK(traversal.Release_Objects().Is_Empty());
}

BOOST_AUTO_TEST_CASE(explicit_visibility_survives_membership_changes_and_removed_notifies_absent_objects)
{
	std::vector<unsigned> events;
	CallbackState state{nullptr, &events};
	SceneLifecycleCallbacks<Object> lifecycle;
	lifecycle.context = &state;
	lifecycle.removed = Removed;
	SceneTraversal<Object> traversal(lifecycle);
	Object first(1, events);
	Object second(2, events);
	Object absent(9, events);
	BOOST_REQUIRE(traversal.Add_Render_Object(&first));

	SceneVisibilityCallbacks<Object, Camera> visibility;
	visibility.context = &state;
	visibility.is_force_visible = Is_Force_Visible;
	visibility.is_culled = Is_Culled;
	visibility.set_visible = Set_Visible;
	visibility.is_really_visible = Is_Really_Visible;
	SceneRenderCallbacks<Object, Context> render;
	render.context = &state;
	render.is_really_visible = Is_Really_Visible;
	render.draw = Draw;
	Camera camera{1};
	Context context;
	BOOST_REQUIRE(traversal.Check_Visibility(camera, visibility));
	BOOST_REQUIRE(traversal.Add_Render_Object(&second));
	BOOST_REQUIRE(traversal.Render(camera, context, visibility, render));
	const std::vector<unsigned> expected_events{101u, 401u};
	BOOST_CHECK(events == expected_events);

	BOOST_CHECK(!traversal.Remove_Render_Object(&absent));
	BOOST_CHECK_EQUAL(events.back(), 909u);
}

BOOST_AUTO_TEST_CASE(game_visibility_can_publish_a_camera_tagged_result)
{
	std::vector<unsigned> events;
	CallbackState state{nullptr, &events};
	SceneTraversal<Object> traversal;
	Object object(1, events);
	object.visible = true;
	BOOST_REQUIRE(traversal.Add_Render_Object(&object));

	SceneVisibilityCallbacks<Object, Camera> visibility;
	visibility.context = &state;
	visibility.is_force_visible = Is_Force_Visible;
	visibility.is_culled = Is_Culled;
	visibility.set_visible = Set_Visible;
	visibility.is_really_visible = Is_Really_Visible;
	SceneRenderCallbacks<Object, Context> render;
	render.context = &state;
	render.is_really_visible = Is_Really_Visible;
	render.draw = Draw;

	Camera main_view{1};
	traversal.Adopt_Visibility(main_view);
	Context context;
	BOOST_REQUIRE(traversal.Render(main_view, context, visibility, render));
	// Adoption trusts the game-specific result and therefore does not invoke
	// the generic visibility callbacks for the tagged camera.
	const std::vector<unsigned> expected_draw{401u};
	BOOST_CHECK(events == expected_draw);

	events.clear();
	Camera reflection_view{2};
	BOOST_REQUIRE(traversal.Render(reflection_view, context, visibility, render));
	const std::vector<unsigned> expected_reflection_visibility{101u};
	BOOST_CHECK(events == expected_reflection_visibility);
}
