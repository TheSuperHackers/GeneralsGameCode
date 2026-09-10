module;
#define BOOST_TEST_MODULE ProceduralPassTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstdint>
#include <memory>

export module Graphics.Materials.ProceduralPass.Tests;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.ProceduralPass;
import Graphics.Materials.State;

using namespace Graphics;

namespace
{

struct Bounds final
{
    int value = 0;
};

using Pass = ProceduralMaterialPass<std::shared_ptr<int>, Bounds>;

bool Prepare_Override(const Pass& pass, Pass::Description& description)
{
    description.shader = MaterialState::Additive();
    description.world_coordinates = true;
    description.color_write_mask = pass.cull_bounds == nullptr
        ? 8 : static_cast<std::uint8_t>(pass.cull_bounds->value);
    return true;
}

bool Prepare_Default(const Pass&, Pass::Description&)
{
    return true;
}

}

BOOST_AUTO_TEST_CASE(native_pass_describes_owned_resources_as_borrowed_inputs)
{
    Bounds bounds;
    std::weak_ptr<MeshMaterial> material_owner;
    std::weak_ptr<int> texture0_owner;
    std::weak_ptr<int> texture1_owner;
    std::weak_ptr<int> texture7_owner;
    {
        Pass pass;
        pass.shader = MaterialState::Opaque();
        auto material = std::make_shared<MeshMaterial>();
        material->parameters.opacity = .5f;
        material_owner = material;
        pass.material = material;
        auto texture0 = std::make_shared<int>(7);
        auto texture1 = std::make_shared<int>(11);
        auto texture7 = std::make_shared<int>(17);
        texture0_owner = texture0;
        texture1_owner = texture1;
        texture7_owner = texture7;
        pass.textures[0] = texture0;
        pass.textures[1] = texture1;
        pass.textures[7] = texture7;
        pass.cull_bounds = &bounds;
        material.reset(); texture0.reset(); texture1.reset(); texture7.reset();

        BOOST_CHECK(!material_owner.expired());
        BOOST_CHECK(!texture0_owner.expired());
        BOOST_CHECK(!texture1_owner.expired());
        BOOST_CHECK(!texture7_owner.expired());
        Pass::Description description;
        BOOST_REQUIRE(pass.Describe(description));
        BOOST_CHECK(description.shader == pass.shader);
        BOOST_CHECK(description.material == pass.material.get());
        BOOST_CHECK(description.textures[0] != nullptr);
        BOOST_CHECK(description.textures[1] != nullptr);
        bounds.value = 42;
        BOOST_CHECK_EQUAL(pass.cull_bounds->value, 42);
        BOOST_CHECK_EQUAL(pass.shader.Get_Bits(), MaterialState::Opaque().Get_Bits());
        pass.prepare = &Prepare_Override;
        Pass::Description live_description;
        BOOST_REQUIRE(pass.Describe(live_description));
        BOOST_CHECK_EQUAL(live_description.color_write_mask, 42);
    }
    BOOST_CHECK(material_owner.expired());
    BOOST_CHECK(texture0_owner.expired());
    BOOST_CHECK(texture1_owner.expired());
    BOOST_CHECK(texture7_owner.expired());
}

BOOST_AUTO_TEST_CASE(callback_descriptions_start_from_defaults_and_ignore_owner_fields)
{
    Pass pass;
    BOOST_CHECK_EQUAL(pass.shader.Get_Bits(), 0);
    pass.shader = MaterialState::Opaque();
    pass.material = std::make_shared<MeshMaterial>();
    pass.textures[0] = std::make_shared<int>(11);
    pass.prepare = &Prepare_Default;

    Pass::Description description;
    description.material = pass.material.get();
    description.textures[0] = pass.textures[0].get();
    BOOST_REQUIRE(pass.Describe(description));
    BOOST_CHECK_EQUAL(pass.shader.Get_Bits(), MaterialState::Opaque().Get_Bits());
    BOOST_CHECK_EQUAL(description.shader.Get_Bits(), 0x0008441b);
    BOOST_CHECK(description.material == nullptr);
    BOOST_CHECK(description.textures[0] == nullptr);
    BOOST_CHECK(!description.world_coordinates);
    BOOST_CHECK_EQUAL(description.color_write_mask, 15);
    constexpr std::array<float, 16> identity{1, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 1};
    BOOST_CHECK_EQUAL_COLLECTIONS(description.world_texture_transform.begin(),
        description.world_texture_transform.end(),
        identity.begin(), identity.end());
}

BOOST_AUTO_TEST_CASE(callback_can_override_description_values)
{
    Pass pass;
    pass.prepare = &Prepare_Override;
    Pass::Description description;
    BOOST_REQUIRE(pass.Describe(description));
    BOOST_CHECK(description.shader == MaterialState::Additive());
    BOOST_CHECK(description.material == nullptr);
    BOOST_CHECK(description.world_coordinates);
    BOOST_CHECK_EQUAL(description.color_write_mask, 8);
}
