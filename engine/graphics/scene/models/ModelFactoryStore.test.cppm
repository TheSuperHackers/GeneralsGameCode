module;
#define BOOST_TEST_MODULE ModelFactoryStoreTests
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
export module Graphics.Scene.Models.FactoryStore.Tests;
import Graphics.Scene.Models.FactoryStore;
using namespace Graphics;
struct Factory { int id; };
BOOST_AUTO_TEST_CASE(factory_identity_order_and_pointers_survive_growth)
{
    ModelFactoryStore<Factory> store;
    auto first=std::make_unique<Factory>(Factory{0});const auto* original=first.get();
    store.Insert("Root",std::move(first));
    for(int i=1;i<=17000;++i)store.Insert("Model"+std::to_string(i),std::make_unique<Factory>(Factory{i}));
    BOOST_REQUIRE_EQUAL(store.Size(),17001);
    BOOST_CHECK(store.Find("ROOT")==original);
    for(int i=1;i<=17000;++i) {
        BOOST_REQUIRE(store.At(i));BOOST_CHECK_EQUAL(store.At(i)->id,i);
        BOOST_CHECK(store.Find("MODEL"+std::to_string(i))==store.At(i));
    }
    store.Erase_If([](const Factory& factory) { return factory.id%2!=0; });
    BOOST_CHECK_EQUAL(store.Size(),8501);
    BOOST_CHECK(store.Find("root")==original);
    BOOST_CHECK(store.Find("MODEL1")==nullptr);
    BOOST_CHECK_EQUAL(store.At(1)->id,2);
}
BOOST_AUTO_TEST_CASE(duplicate_name_precedence_and_exact_release_preserve_other_factories)
{
    ModelFactoryStore<Factory> store;
    auto first=std::make_unique<Factory>(Factory{1});auto* first_pointer=first.get();
    store.Insert("Model",std::move(first));
    store.Insert("MODEL",std::make_unique<Factory>(Factory{2}));
    BOOST_CHECK_EQUAL(store.Find("model")->id,2);
    auto released=store.Release(first_pointer);
    BOOST_CHECK(released.get()==first_pointer);
    BOOST_CHECK_EQUAL(store.Find("model")->id,2);
    store.Insert("model",std::make_unique<Factory>(Factory{3}));
    store.Erase_If([](const Factory& factory) { return factory.id==3; });
    BOOST_CHECK_EQUAL(store.Find("model")->id,2);
    store.Insert("A/B",std::make_unique<Factory>(Factory{4}));
    BOOST_CHECK(store.Find("A\\B")==nullptr);
    store.Clear();
    BOOST_CHECK_EQUAL(released->id,1);
    BOOST_CHECK(store.Find("model")==nullptr);
}
struct TrackedDelete {
    std::vector<int>* deleted=nullptr;
    void operator()(Factory* factory) const { deleted->push_back(factory->id);delete factory; }
};
BOOST_AUTO_TEST_CASE(selective_destruction_and_clear_have_explicit_order)
{
    std::vector<int> deleted;
    ModelFactoryStore<Factory,TrackedDelete> store;
    using Owner=decltype(store)::Owner;
    for(int i=0;i<5;++i)store.Insert(std::to_string(i),Owner(new Factory{i},TrackedDelete{&deleted}));
    BOOST_CHECK_THROW(store.Erase_If([](const Factory& factory) {
        if(factory.id==3)throw std::runtime_error("selection failed");
        return true;
    }),std::runtime_error);
    BOOST_CHECK_EQUAL(store.Size(),5);
    BOOST_CHECK(deleted.empty());
    BOOST_CHECK_EQUAL(store.Find("2")->id,2);
    store.Erase_If([](const Factory& factory) { return factory.id%2==0; });
    const std::vector<int> evicted{0,2,4};
    BOOST_CHECK_EQUAL_COLLECTIONS(deleted.begin(),deleted.end(),evicted.begin(),evicted.end());
    BOOST_REQUIRE_EQUAL(store.Size(),2);
    BOOST_CHECK_EQUAL(store.At(0)->id,1);BOOST_CHECK_EQUAL(store.At(1)->id,3);
    store.Clear();
    const std::vector<int> all{0,2,4,3,1};
    BOOST_CHECK_EQUAL_COLLECTIONS(deleted.begin(),deleted.end(),all.begin(),all.end());
}
