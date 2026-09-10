module;
#define BOOST_TEST_MODULE ModelFactoryTests
#include <boost/test/included/unit_test.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>
export module Graphics.Scene.Models.Factory.Tests;
import Graphics.Scene.Models.Factory;
import Graphics.Scene.Models.FactoryStore;
using namespace Graphics;

struct ModelSource { std::vector<int> attachments; };
struct ModelInstance { std::shared_ptr<const ModelSource> source; int color=0; };

BOOST_AUTO_TEST_CASE(instances_keep_assets_after_factory_eviction_and_have_independent_state)
{
    auto source=std::make_shared<ModelSource>();
    source->attachments={3,7,11};
    std::weak_ptr<const ModelSource> lifetime=source;
    ModelFactoryStore<ModelFactory<ModelInstance>> factories;
    factories.Insert("Tank",std::make_unique<ModelFactory<ModelInstance>>("Tank",23,
        [source] { return new ModelInstance{source,5}; }));
    source.reset();
    const auto* factory=factories.Find("TANK");BOOST_REQUIRE(factory);
    BOOST_CHECK_EQUAL(factory->name,"Tank");BOOST_CHECK_EQUAL(factory->class_id,23);
    std::unique_ptr<ModelInstance> first(factory->Instantiate()),second(factory->Instantiate());
    BOOST_REQUIRE(first);BOOST_REQUIRE(second);BOOST_CHECK(first.get()!=second.get());
    first->color=9;BOOST_CHECK_EQUAL(second->color,5);
    factories.Clear();
    BOOST_CHECK(!lifetime.expired());
    BOOST_CHECK_EQUAL(first->source->attachments[2],11);
    first.reset();BOOST_CHECK(!lifetime.expired());
    second.reset();BOOST_CHECK(lifetime.expired());
}

BOOST_AUTO_TEST_CASE(failed_creation_does_not_consume_factory_or_its_source)
{
    auto dependency=std::make_shared<std::shared_ptr<const ModelSource>>();
    ModelFactory<ModelInstance> factory("Assembly",25,[dependency]() -> ModelInstance* {
        if(!*dependency)return nullptr;
        return new ModelInstance{*dependency};
    });
    BOOST_CHECK(factory.Instantiate()==nullptr);
    *dependency=std::make_shared<ModelSource>(ModelSource{{1,2}});
    std::unique_ptr<ModelInstance> instance(factory.Instantiate());BOOST_REQUIRE(instance);
    dependency->reset();
    BOOST_CHECK(factory.Instantiate()==nullptr);
    BOOST_CHECK_EQUAL(instance->source->attachments.size(),2);
}

BOOST_AUTO_TEST_CASE(variant_identity_is_independent_of_shared_clone_source)
{
    auto source=std::make_shared<ModelInstance>();source->color=17;
    std::weak_ptr<ModelInstance> lifetime=source;
    ModelFactoryStore<ModelFactory<ModelInstance>> factories;
    for(const auto* name:{"Tank.Red","Tank.Blue"})
        factories.Insert(name,std::make_unique<ModelFactory<ModelInstance>>(name,23,
            [source] { return new ModelInstance(*source); }));
    source.reset();
    factories.Erase_If([](const auto& factory) { return factory.name=="Tank.Red"; });
    BOOST_CHECK(!lifetime.expired());
    const auto* blue=factories.Find("tank.blue");BOOST_REQUIRE(blue);
    std::unique_ptr<ModelInstance> instance(blue->Instantiate());BOOST_REQUIRE(instance);
    factories.Clear();BOOST_CHECK(lifetime.expired());
    BOOST_CHECK_EQUAL(instance->color,17);
}
