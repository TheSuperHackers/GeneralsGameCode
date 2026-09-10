module;

#define BOOST_TEST_MODULE SceneAdapterTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <vector>

export module Graphics.Scene.Adapter.Tests;

import Graphics.Scene.Traversal;

using namespace Graphics;

namespace
{

struct Camera final
{
	bool accepts_object = true;
};

struct Context final
{
	LocalLighting *light_environment = nullptr;
	std::vector<unsigned> *events = nullptr;
};

struct Object final : SceneListMember
{
	Object(unsigned id, std::vector<unsigned> &events)
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
	bool visible = false;
};

struct Lifecycle final
{
	std::vector<unsigned> *events = nullptr;
};

void Added(void *raw, Object *object) noexcept
{
	auto &lifecycle = *static_cast<Lifecycle *>(raw);
	lifecycle.events->push_back(10u + object->id);
}

void Removed(void *raw, Object *object) noexcept
{
	auto &lifecycle = *static_cast<Lifecycle *>(raw);
	lifecycle.events->push_back(20u + object->id);
}

bool Is_Force_Visible(void *, const Object *) noexcept
{
	return false;
}

bool Is_Culled(void *, const Object *, const Camera *camera) noexcept
{
	return !camera->accepts_object;
}

void Set_Visible(void *, Object *object, bool visible) noexcept
{
	object->visible = visible;
}

bool Is_Really_Visible(void *, Object *object) noexcept
{
	return object->visible;
}

void Update(void *raw, Object *object) noexcept
{
	static_cast<Context *>(raw)->events->push_back(30u + object->id);
}

SceneDrawResult Draw(void *raw, Object *object, Context *) noexcept
{
	static_cast<Context *>(raw)->events->push_back(40u + object->id);
	return SceneDrawResult::Submitted;
}

}

BOOST_AUTO_TEST_CASE(adapter_contract_keeps_membership_and_uses_published_view)
{
	std::vector<unsigned> events;
	Lifecycle lifecycle{&events};
	SceneTraversal<Object> traversal({&lifecycle, &Added, &Removed});
	Object object(1, events);

	BOOST_REQUIRE(traversal.Add_Render_Object(&object));
	BOOST_CHECK_EQUAL(object.references, 2u);
	BOOST_CHECK(traversal.Render_Objects().Contains(&object));
	BOOST_REQUIRE(traversal.Register(&object, SceneRegistration::On_Frame_Update));

	SceneVisibilityCallbacks<Object, Camera> visibility;
	visibility.is_force_visible = &Is_Force_Visible;
	visibility.is_culled = &Is_Culled;
	visibility.set_visible = &Set_Visible;
	visibility.is_really_visible = &Is_Really_Visible;
	SceneRenderCallbacks<Object, Context> render;
	Context context{nullptr, &events};
	render.context = &context;
	render.on_frame_update = &Update;
	render.is_really_visible = &Is_Really_Visible;
	render.draw = &Draw;

	// Game-specific code can publish the result it produced without asking
	// the generic traversal to recalculate visibility for this view.
	events.clear();
	Camera main_view{true};
	object.visible = true;
	traversal.Adopt_Visibility(main_view);
	BOOST_REQUIRE(traversal.Render(main_view, context, visibility, render));
	const std::vector<unsigned> expected_render{31u, 41u};
	BOOST_CHECK(events == expected_render);

	events.clear();
	Camera other_view{false};
	BOOST_REQUIRE(traversal.Render(other_view, context, visibility, render));
	const std::vector<unsigned> expected_other_view{31u};
	BOOST_CHECK(events == expected_other_view);

	events.clear();
	BOOST_REQUIRE(traversal.Unregister(&object, SceneRegistration::On_Frame_Update));
	BOOST_REQUIRE(traversal.Remove_Render_Object(&object));
	BOOST_CHECK_EQUAL(object.references, 1u);
	const std::vector<unsigned> expected_lifecycle{21u};
	BOOST_CHECK(events == expected_lifecycle);
}
