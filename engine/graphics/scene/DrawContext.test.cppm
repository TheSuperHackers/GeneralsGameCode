module;

#define BOOST_TEST_MODULE SceneDrawContextTests
#include <boost/test/included/unit_test.hpp>
#include <memory>

export module Graphics.Scene.DrawContext.Tests;
import Graphics.Scene.DrawContext;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(context_passes_retain_order_resources_and_balance_rejected_pushes)
{
    SceneDrawContext<int> context;
    std::weak_ptr<int> first, last, rejected;
    for (unsigned index = 0; index < 33; ++index) {
        auto pass = std::make_shared<int>(index);
        if (index == 0) first = pass;
        if (index == 30) last = pass;
        if (index == 31) rejected = pass;
        BOOST_CHECK_EQUAL(context.Push_Material_Pass(pass), index < 31);
    }
    BOOST_CHECK(!first.expired());
    BOOST_CHECK(!last.expired());
    BOOST_CHECK(rejected.expired());
    BOOST_REQUIRE_EQUAL(context.Additional_Pass_Count(), 31);
    for (int index = 0; index < context.Additional_Pass_Count(); ++index)
        BOOST_CHECK_EQUAL(*context.Peek_Additional_Pass(index), index);
    BOOST_CHECK(!context.Pop_Material_Pass());
    BOOST_CHECK(!context.Pop_Material_Pass());
    BOOST_CHECK_EQUAL(context.Additional_Pass_Count(), 31);
    BOOST_CHECK(context.Pop_Material_Pass());
    BOOST_CHECK(last.expired());
    for (unsigned index = 0; index < 30; ++index)
        BOOST_CHECK(context.Pop_Material_Pass());
    BOOST_CHECK(first.expired());
    BOOST_CHECK(!context.Pop_Material_Pass());
}

BOOST_AUTO_TEST_CASE(nested_draw_overrides_replace_then_restore_without_leaking_between_contexts)
{
    SceneDrawContext<int> context;
    BOOST_CHECK(context.Current_Override_Flags() == DrawOverride::Default);
    BOOST_REQUIRE(context.Push_Override_Flags(DrawOverride::ForceTwoSided | DrawOverride::ForceSorting));
    BOOST_CHECK(context.Has_Override(DrawOverride::ForceTwoSided));
    BOOST_CHECK(context.Has_Override(DrawOverride::ForceSorting));
    BOOST_REQUIRE(context.Push_Override_Flags(DrawOverride::AdditionalPassesOnly));
    BOOST_CHECK(context.Has_Override(DrawOverride::AdditionalPassesOnly));
    BOOST_CHECK(!context.Has_Override(DrawOverride::ForceTwoSided));
    BOOST_REQUIRE(context.Pop_Override_Flags());
    BOOST_CHECK(context.Has_Override(DrawOverride::ForceTwoSided));
    BOOST_REQUIRE(context.Pop_Override_Flags());
    BOOST_CHECK(context.Current_Override_Flags() == DrawOverride::Default);
    BOOST_CHECK(!context.Pop_Override_Flags());
    const SceneDrawContext<int> other;
    BOOST_CHECK(other.Current_Override_Flags() == DrawOverride::Default);
    BOOST_CHECK_EQUAL(other.alpha_override, 1);
    BOOST_CHECK_EQUAL(other.pass_alpha_override, 1);
    BOOST_CHECK_EQUAL(other.pass_emissive_override, 1);
    BOOST_CHECK_EQUAL(other.fog_scale, 0);
    BOOST_CHECK(other.light_environment == nullptr);
}

BOOST_AUTO_TEST_CASE(override_stack_rejects_bounded_overflow_and_recovers_rejected_pushes)
{
    constexpr unsigned expected_accepted_levels = 31;
    constexpr unsigned expected_rejected_pushes = 2;
    SceneDrawContext<int> context;

    for (unsigned level = 0; level < expected_accepted_levels; ++level) {
        const auto expected_flags = level + 1 == expected_accepted_levels
            ? DrawOverride::ShadowRendering : DrawOverride::ForceSorting;
        BOOST_REQUIRE(context.Push_Override_Flags(expected_flags));
        BOOST_CHECK(context.Current_Override_Flags() == expected_flags);
    }

    constexpr DrawOverride expected_top = DrawOverride::ShadowRendering;
    for (unsigned push = 0; push < expected_rejected_pushes; ++push) {
        BOOST_CHECK(!context.Push_Override_Flags(DrawOverride::AdditionalPassesOnly));
        BOOST_CHECK(context.Current_Override_Flags() == expected_top);
    }
    for (unsigned pop = 0; pop < expected_rejected_pushes; ++pop) {
        BOOST_CHECK(!context.Pop_Override_Flags());
        BOOST_CHECK(context.Current_Override_Flags() == expected_top);
    }

    BOOST_REQUIRE(context.Pop_Override_Flags());
    BOOST_CHECK(context.Current_Override_Flags() == DrawOverride::ForceSorting);
    BOOST_REQUIRE(context.Push_Override_Flags(DrawOverride::AdditionalPassesOnly));
    BOOST_CHECK(context.Current_Override_Flags() == DrawOverride::AdditionalPassesOnly);
    BOOST_REQUIRE(context.Pop_Override_Flags());

    for (unsigned level = 1; level < expected_accepted_levels; ++level)
        BOOST_REQUIRE(context.Pop_Override_Flags());
    BOOST_CHECK(context.Current_Override_Flags() == DrawOverride::Default);
    BOOST_CHECK(!context.Pop_Override_Flags());
}

BOOST_AUTO_TEST_CASE(context_copy_has_independent_stack_state_and_retains_its_own_pass_references)
{
    std::weak_ptr<int> resource;
    {
        SceneDrawContext<int> source;
        auto pass = std::make_shared<int>(7);
        resource = pass;
        source.Push_Material_Pass(pass);
        source.Push_Override_Flags(DrawOverride::ShadowRendering);
        SceneDrawContext<int> copy(source);
        pass.reset();
        source.Pop_Material_Pass();
        source.Pop_Override_Flags();
        BOOST_CHECK(!resource.expired());
        BOOST_CHECK_EQUAL(*copy.Peek_Additional_Pass(0), 7);
        BOOST_CHECK(copy.Has_Override(DrawOverride::ShadowRendering));
        BOOST_CHECK(!source.Has_Override(DrawOverride::ShadowRendering));
    }
    BOOST_CHECK(resource.expired());
}
