module;

#define BOOST_TEST_MODULE GraphicsMaterialSlotsTests

#include <boost/test/included/unit_test.hpp>

#include <cstddef>
#include <memory>

export module Graphics.Scene.Models.MaterialSlots.Tests;

import Graphics.Scene.Models.MaterialSlots;

namespace {

struct Resource final
{
    explicit Resource(int id) : value(id) {}

    int value;
};

}

BOOST_AUTO_TEST_CASE(material_slots_preserve_absent_and_allocated_empty_states)
{
    using Slots = Graphics::MaterialSlots<std::shared_ptr<Resource>>;

    Slots absent;
    BOOST_CHECK(!absent.Is_Allocated());
    BOOST_CHECK_EQUAL(absent.Count(), 0u);
    BOOST_CHECK(!absent.Peek(0));
    BOOST_CHECK(!absent.Get(0));

    absent.Allocate(0);
    BOOST_CHECK(absent.Is_Allocated());
    BOOST_CHECK_EQUAL(absent.Count(), 0u);
    BOOST_CHECK(!absent.Peek(0));

    const Slots copy = absent;
    BOOST_CHECK(copy.Is_Allocated());
    BOOST_CHECK_EQUAL(copy.Count(), 0u);

    const Slots clone = absent.Clone();
    BOOST_CHECK(clone.Is_Allocated());
    BOOST_CHECK_EQUAL(clone.Count(), 0u);

    absent.Reset();
    BOOST_CHECK(!absent.Is_Allocated());
    BOOST_CHECK(copy.Is_Allocated());
    BOOST_CHECK(clone.Is_Allocated());
}

BOOST_AUTO_TEST_CASE(material_slots_share_edits_and_clone_independently)
{
    using Slots = Graphics::MaterialSlots<std::shared_ptr<Resource>>;

    auto first = std::make_shared<Resource>(1);
    auto second = std::make_shared<Resource>(2);
    Slots source;
    source.Allocate(3);
    source.Set(0, first);
    source.Set(1, second);

    Slots shared = source;
    BOOST_CHECK(shared.Peek(0)->get() == first.get());
    BOOST_CHECK(shared.Peek(1)->get() == second.get());

    auto replacement = std::make_shared<Resource>(3);
    shared.Set(0, replacement);
    BOOST_CHECK(source.Peek(0)->get() == replacement.get());
    BOOST_CHECK(source.Peek(1)->get() == second.get());

    Slots independent = source.Clone();
    BOOST_CHECK(independent.Is_Allocated());
    BOOST_CHECK_EQUAL(independent.Count(), source.Count());
    BOOST_CHECK(independent.Peek(0)->get() == replacement.get());
    BOOST_CHECK(independent.Peek(1)->get() == second.get());

    auto clone_replacement = std::make_shared<Resource>(4);
    independent.Set(0, clone_replacement);
    BOOST_CHECK(independent.Peek(0)->get() == clone_replacement.get());
    BOOST_CHECK(source.Peek(0)->get() == replacement.get());
}

BOOST_AUTO_TEST_CASE(material_slots_get_retains_resources_and_replacements_release_them)
{
    using Slots = Graphics::MaterialSlots<std::shared_ptr<Resource>>;

    std::weak_ptr<Resource> first;
    std::weak_ptr<Resource> second;
    Slots slots;
    {
        auto first_owner = std::make_shared<Resource>(10);
        auto second_owner = std::make_shared<Resource>(20);
        first = first_owner;
        second = second_owner;
        slots.Allocate(1);
        slots.Set(0, first_owner);
        BOOST_CHECK_EQUAL(first.use_count(), 2u);

        auto retained = slots.Get(0);
        BOOST_REQUIRE(retained);
        BOOST_CHECK_EQUAL(retained->value, 10);
        first_owner.reset();
        BOOST_CHECK(!first.expired());

        slots.Set(0, second_owner);
        BOOST_CHECK(!first.expired());
        retained.reset();
        BOOST_CHECK(first.expired());
        BOOST_CHECK(!second.expired());

        slots.Set(0, std::shared_ptr<Resource>{});
        BOOST_CHECK(second.expired() == false);
    }
    BOOST_CHECK(second.expired());

    slots.Reset();
    BOOST_CHECK(first.expired());
}

BOOST_AUTO_TEST_CASE(material_slots_self_assignment_keeps_all_owners)
{
    using Slots = Graphics::MaterialSlots<std::shared_ptr<Resource>>;

    auto resource = std::make_shared<Resource>(99);
    std::weak_ptr<Resource> weak = resource;
    Slots slots;
    slots.Allocate(2);
    slots.Set(0, resource);
    slots.Set(1, resource);
    resource.reset();

    slots = slots;
    BOOST_CHECK(!weak.expired());
    BOOST_CHECK(slots.Peek(0)->get() == slots.Peek(1)->get());
    BOOST_CHECK_EQUAL(slots.Get(0)->value, 99);
    slots.Set(0, std::shared_ptr<Resource>{});
    BOOST_CHECK(!weak.expired());
    slots.Set(1, std::shared_ptr<Resource>{});
    BOOST_CHECK(weak.expired());
}
