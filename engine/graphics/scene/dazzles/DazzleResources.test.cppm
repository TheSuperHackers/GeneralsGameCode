module;
#define BOOST_TEST_MODULE DazzleResourcesTests
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>
export module Graphics.Scene.Dazzles.Resources.Tests;
import Assets.Dazzles;
import Graphics.Scene.Dazzles.Resources;
BOOST_AUTO_TEST_CASE(catalog_owns_definitions_resolves_exact_names_and_shares_lazy_flare_resources) {
    Assets::DazzleDefinitions definitions;
    definitions.lens_flares.push_back({"Flare","flare.tga",{{-.5f,.25f}}});
    for(const auto* name:{"Lamp","Other"}) {
        auto& dazzle=definitions.dazzles.emplace_back();
        dazzle.name=name; dazzle.lens_flare="Flare"; dazzle.primary_texture="glare.tga"; dazzle.halo_texture="halo.tga";
    }
    Graphics::DazzleResources<std::shared_ptr<std::string>> resources;
    resources.Initialize(definitions); definitions={};
    BOOST_REQUIRE_EQUAL(resources.Size(),2);
    BOOST_CHECK_EQUAL(resources.Find("Lamp"),0); BOOST_CHECK_EQUAL(resources.Find("lamp"),Graphics::Invalid_Dazzle_Type);
    BOOST_CHECK_EQUAL(resources.Sprites(1).front().location,-.5f);
    std::vector<std::string> requested; std::vector<std::weak_ptr<std::string>> owners;
    const auto acquire=[&](const std::string& name) {
        requested.push_back(name); auto owner=std::make_shared<std::string>(name); owners.push_back(owner); return owner;
    };
    const auto* flare=resources.Texture(0,Graphics::DazzleImage::LensFlare,acquire);
    BOOST_CHECK_EQUAL(flare,resources.Texture(1,Graphics::DazzleImage::LensFlare,acquire));
    const auto* glare=resources.Texture(0,Graphics::DazzleImage::Glare,acquire);
    BOOST_CHECK_EQUAL(glare,resources.Texture(0,Graphics::DazzleImage::Glare,acquire));
    BOOST_CHECK_EQUAL(*resources.Texture(0,Graphics::DazzleImage::Halo,acquire),"halo.tga");
    BOOST_CHECK((requested==std::vector<std::string>{"flare.tga","glare.tga","halo.tga"}));
    for(const auto& owner:owners) BOOST_CHECK(!owner.expired());
    resources.Clear();
    for(const auto& owner:owners) BOOST_CHECK(owner.expired());
    BOOST_CHECK_EQUAL(resources.Size(),0);
}
