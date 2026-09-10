module;

#define BOOST_TEST_MODULE GraphicsShadowsTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cmath>

export module Graphics.Scene.Shadows.Tests;

import Graphics.Scene.Shadows;

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

View Make_View() noexcept
{
	return {Matrix4x4::Identity(), Make_Perspective(1.0f, 100.0f), {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
}
}

BOOST_AUTO_TEST_CASE(cascade_receivers_and_upstream_casters_fit_gpu_depth_range)
{
    const View view = Make_View();
    RenderLight light;
    light.type = RenderLightType::Directional;
    light.flags = RenderLightFlags::Enabled;
    light.direction = {0,0,-1};
    const ShadowSettings settings{4,1,100,0.5f,10,1024};
    ShadowCascades cascades;
    BOOST_REQUIRE(Build_Shadow_Cascades(view,LightHandle(0,1),light,settings,cascades));
    for (std::uint32_t cascade=0;cascade<cascades.count;++cascade) {
        const auto& shadow = cascades.views[cascade];
        for (const float depth : {shadow.split_near,shadow.split_far}) {
            for (const float x : {-depth,depth}) {
                for (const float y : {-depth,depth}) {
                    const std::array<float,4> world{x,y,-depth,1};
                    std::array<float,4> clip{};
                    for (std::size_t row=0;row<4;++row)
                        for (std::size_t column=0;column<4;++column)
                            clip[row] += shadow.view_projection(row,column)*world[column];
                    BOOST_REQUIRE(clip[3] > 0);
                    BOOST_CHECK_LE(std::abs(clip[0]/clip[3]),1.0001f);
                    BOOST_CHECK_LE(std::abs(clip[1]/clip[3]),1.0001f);
                    BOOST_CHECK_GE(clip[2]/clip[3],-0.0001f);
                    BOOST_CHECK_LE(clip[2]/clip[3],1.0001f);
                }
            }
        }
        // A caster outside the camera slice can cast into the slice. Padding
        // must enlarge the depth interval, not just move the light camera.
        const float caster_z = -shadow.split_near + settings.depth_padding*0.5f;
        const float caster_depth = shadow.view_projection(2,2)*caster_z + shadow.view_projection(2,3);
        BOOST_CHECK_GE(caster_depth,0);
        BOOST_CHECK_LE(caster_depth,1);
    }
}

BOOST_AUTO_TEST_CASE(sub_texel_camera_translation_keeps_shadow_xy_projection_stable)
{
    View view = Make_View();
    RenderLight light;
    light.type = RenderLightType::Directional;
    light.flags = RenderLightFlags::Enabled;
    light.direction = {0,0,-1};
    const ShadowSettings settings{4,1,100,0.5f,10,1024};
    ShadowCascades before, after;
    BOOST_REQUIRE(Build_Shadow_Cascades(view,LightHandle(0,1),light,settings,before));
    view.view_matrix.values[3] = -0.001f;
    BOOST_REQUIRE(Build_Shadow_Cascades(view,LightHandle(0,1),light,settings,after));
    for (unsigned cascade=0;cascade<before.count;++cascade) {
        for (unsigned element : {0u,1u,2u,3u,4u,5u,6u,7u})
            BOOST_CHECK_SMALL(before.views[cascade].view_projection.values[element]
                - after.views[cascade].view_projection.values[element],0.000001f);
    }
}

BOOST_AUTO_TEST_CASE(cascade_splits_are_deterministic_and_monotonic)
{
	std::array<float, 4> splits{};
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, 0.5f, splits));
	BOOST_CHECK(splits[0] > 1.0f);
	BOOST_CHECK(splits[0] < splits[1]);
	BOOST_CHECK(splits[1] < splits[2]);
	BOOST_CHECK(splits[2] < splits[3]);
	BOOST_CHECK_CLOSE_FRACTION(splits[3], 100.0f, 0.00001f);

	std::array<float, 4> repeated{};
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, 0.5f, repeated));
	BOOST_CHECK(splits == repeated);

	std::array<float, 4> linear{};
	std::array<float, 4> logarithmic{};
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, 0.0f, linear));
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, 1.0f, logarithmic));
	BOOST_CHECK_CLOSE_FRACTION(linear[0], 25.75f, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(logarithmic[0], std::sqrt(std::sqrt(100.0f)), 0.00001f);
}

BOOST_AUTO_TEST_CASE(cascade_split_inputs_are_rejected_or_clamped)
{
	std::array<float, 4> splits{};
	BOOST_CHECK(!Calculate_Cascade_Splits(0.0f, 100.0f, 0.5f, splits));
	BOOST_CHECK(!Calculate_Cascade_Splits(1.0f, 1.0f, 0.5f, splits));
	BOOST_CHECK(!Calculate_Cascade_Splits(1.0f, 100.0f, 0.5f, {}));

	std::array<float, 4> lower_lambda{};
	std::array<float, 4> linear{};
	std::array<float, 4> upper_lambda{};
	std::array<float, 4> logarithmic{};
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, -1.0f, lower_lambda));
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, 0.0f, linear));
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, 2.0f, upper_lambda));
	BOOST_REQUIRE(Calculate_Cascade_Splits(1.0f, 100.0f, 1.0f, logarithmic));
	BOOST_CHECK(lower_lambda == linear);
	BOOST_CHECK(upper_lambda == logarithmic);
}

BOOST_AUTO_TEST_CASE(directional_shadow_views_are_generated_deterministically)
{
	const View view = Make_View();
	const LightHandle light_handle(4, 2);
	const RenderLight directional{
		RenderLightType::Directional,
		RenderLightFlags::Enabled,
		{},
		{0.0f, 0.0f, -1.0f},
		{1.0f, 0.9f, 0.8f},
		2.0f,
		0.0f,
		0.0f,
		0.0f
	};
	const ShadowSettings settings{4, 1.0f, 100.0f, 0.5f, 10.0f, 512};

	ShadowCascades first;
	ShadowCascades second;
	BOOST_REQUIRE(Build_Shadow_Cascades(view, light_handle, directional, settings, first));
	BOOST_REQUIRE(Build_Shadow_Cascades(view, light_handle, directional, settings, second));
	BOOST_CHECK(first.light == light_handle);
	BOOST_CHECK(first.count == 4);
	for (std::size_t index = 0; index < first.count; ++index) {
		BOOST_CHECK(first.views[index].split_near < first.views[index].split_far);
		BOOST_CHECK(first.views[index].viewport.width == 512.0f);
		BOOST_CHECK(first.views[index].viewport.height == 512.0f);
		BOOST_CHECK(std::isfinite(first.views[index].view_projection.values[0]));
		BOOST_CHECK(first.views[index].view_projection.values == second.views[index].view_projection.values);
	}
}

BOOST_AUTO_TEST_CASE(shadow_cascade_generation_requires_directional_enabled_lights)
{
	const View view = Make_View();
	const ShadowSettings settings{};
	ShadowCascades cascades;
	BOOST_CHECK(!Build_Shadow_Cascades(view, LightHandle(1, 1), RenderLight{}, settings, cascades));

	RenderLight point;
	point.type = RenderLightType::Point;
	BOOST_CHECK(!Build_Shadow_Cascades(view, LightHandle(1, 1), point, settings, cascades));

	RenderLight invalid_direction;
	invalid_direction.type = RenderLightType::Directional;
	invalid_direction.direction = {};
	BOOST_CHECK(!Build_Shadow_Cascades(view, LightHandle(1, 1), invalid_direction, settings, cascades));
}
