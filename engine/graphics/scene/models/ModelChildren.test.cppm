module;
#define BOOST_TEST_MODULE ModelChildrenTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <utility>
#include <vector>
export module Graphics.Scene.Models.Children.Tests;
import Graphics.Scene.Models.Children;
import Graphics.Scene.Models.DetailLevels;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(ordered_growth_bone_lookup_queries_and_first_duplicate_extraction) {
    ModelChildren<std::shared_ptr<int>> children;
    children.Initialize(3);
    for (int i = 0; i < 513; ++i) children.Add(0, std::make_shared<int>(i), i % 2);
    auto duplicate = std::make_shared<int>(900);
    children.Add(1, duplicate, 7);
    children.Add(2, duplicate, 8);
    children.Add(2, std::make_shared<int>(901), 7);
    children.Add_Additional(duplicate, 7);
    children.Add_Additional(std::make_shared<int>(902), 8);
    BOOST_CHECK_EQUAL(children.Count(), 518u);
    BOOST_CHECK_EQUAL(children.Count_On_Bone(7), 3u);
    BOOST_REQUIRE(children.On_Bone(1, 7));
    BOOST_CHECK_EQUAL(*children.On_Bone(1, 7)->model, 901);
    BOOST_CHECK(!children.On_Bone(3, 7));
    BOOST_CHECK_EQUAL(*children.At(512)->model, 512);
    BOOST_CHECK_EQUAL(*children.At(513)->model, 900);
    BOOST_CHECK_EQUAL(*children.At(517)->model, 902);
    BOOST_CHECK(!children.At(518));
    std::vector<int> queried;
    BOOST_CHECK(children.Query_Level(2, [&](const auto& child) {
        queried.push_back(*child.model);
        return *child.model == 900;
    }));
    const std::vector<int> expected{900, 901, 900, 902};
    BOOST_CHECK(queried == expected);
    const std::weak_ptr<int> lifetime = duplicate;
    auto removed = children.Extract_First([&](const auto& child) { return child.model == duplicate; });
    BOOST_REQUIRE(removed);
    BOOST_CHECK_EQUAL(removed->level, 1);
    BOOST_CHECK(children.Level(1).empty());
    BOOST_CHECK_EQUAL(children.Count_On_Bone(7), 2u);
    duplicate.reset();
    children.Clear([](const auto&) {});
    BOOST_CHECK(!lifetime.expired());
    removed.reset();
    BOOST_CHECK(lifetime.expired());
}

BOOST_AUTO_TEST_CASE(selection_callbacks_observe_old_then_new_level_and_clones_own_their_children) {
    ModelChildren<std::shared_ptr<int>> children;
    children.Initialize(2);
    children.Add(0, std::make_shared<int>(10), 1);
    children.Add(0, std::make_shared<int>(11), 2);
    children.Add(1, std::make_shared<int>(20), 3);
    children.Add_Additional(std::make_shared<int>(30), 4);
    std::vector<std::pair<int, int>> notifications;
    const auto removed = [&](const auto& child) { notifications.emplace_back(-*child.model, children.Current_Level()); };
    const auto added = [&](const auto& child) { notifications.emplace_back(*child.model, children.Current_Level()); };
    children.Select_Level(8, removed, added);
    children.Select_Level(1, removed, added);
    children.Select_Level(-3, removed, added);
    const std::vector<std::pair<int, int>> expected{{-10,0},{-11,0},{20,1},{-20,1},{10,0},{11,0}};
    BOOST_CHECK(notifications == expected);
    children.Select_Level(1, removed, added);
    ModelChildren<std::shared_ptr<int>> clone;
    std::vector<int> cloned;
    clone.Clone_From(children, [&](const auto& source) {
        cloned.push_back(*source);
        return std::make_shared<int>(*source);
    });
    BOOST_CHECK(cloned == std::vector<int>({10,11,20,30}));
    BOOST_CHECK_EQUAL(clone.Current_Level(), 0);
    *children.At(0)->model = 99;
    BOOST_CHECK_EQUAL(*clone.At(0)->model, 10);
    std::vector<int> detached;
    std::weak_ptr<int> previous;
    children.Clear([&](const auto& source) {
        BOOST_CHECK(previous.expired());
        previous = source;
        detached.push_back(*source);
    });
    BOOST_CHECK(previous.expired());
    BOOST_CHECK(detached == std::vector<int>({99,11,20,30}));
    BOOST_CHECK_EQUAL(clone.Count(), 4u);
}

BOOST_AUTO_TEST_CASE(detail_thresholds_costs_bias_and_terminal_values_keep_authored_boundaries) {
    ModelDetailLevels levels;
    levels.Initialize(3);
    levels.Set_Maximum_Area(0, .25f);
    levels.Set_Maximum_Area(1, .5f);
    levels.Set_Maximum_Area(2, 1);
    levels.Set_Polygon_Count(0, 1);
    levels.Set_Polygon_Count(1, 2);
    levels.Set_Polygon_Count(2, 4);
    levels.Set_Bias(2);
    BOOST_CHECK_EQUAL(levels.Update(.5f), 1);
    BOOST_CHECK_EQUAL(levels.Cost(0), 1);
    BOOST_CHECK_EQUAL(levels.Cost(1), 2);
    BOOST_CHECK_EQUAL(levels.Cost(2), 4);
    BOOST_CHECK_EQUAL(levels.Value(0), (std::numeric_limits<float>::max)());
    BOOST_CHECK_EQUAL(levels.Value(1), (std::numeric_limits<float>::max)());
    BOOST_CHECK_EQUAL(levels.Value(2), .2421875f);
    BOOST_CHECK_EQUAL(levels.Value(3), -1);
    BOOST_CHECK_EQUAL(levels.Update(.25f), 0);
    BOOST_CHECK_EQUAL(levels.Value(1), .21875f);
    BOOST_CHECK_EQUAL(levels.Update(2), 2);
    BOOST_CHECK_EQUAL(levels.Value(2), (std::numeric_limits<float>::max)());
    levels.Set_Polygon_Count(2, 0);
    BOOST_CHECK_EQUAL(levels.Update(.5f), 1);
    BOOST_CHECK_EQUAL(levels.Cost(2), .000001f);
    BOOST_CHECK_EQUAL(levels.Value(2), 0);
}

BOOST_AUTO_TEST_CASE(selected_children_and_additional_attachments_draw_after_source_release_and_resize) {
    struct Geometry {
        std::array<float, 4> color;
        float center;
    };
    using Children = ModelChildren<std::shared_ptr<Geometry>>;
    Children source;
    source.Initialize(2);
    source.Add(0, std::make_shared<Geometry>(Geometry{{1,0,0,.5f}, -.5f}), 1);
    source.Add(1, std::make_shared<Geometry>(Geometry{{0,1,0,.75f}, .5f}), 2);
    source.Add_Additional(std::make_shared<Geometry>(Geometry{{0,0,1,1}, 0}), 3);
    Children children;
    children.Clone_From(source, [](const auto& geometry) { return std::make_shared<Geometry>(*geometry); });
    source.Clear([](const auto&) {});
    for (bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        auto& commands = device.Immediate_Command_List();
        PropStyle style;
        style.depth_test = style.depth_write = false;
        PropParameters parameters;
        parameters.textured = 0;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        for (unsigned width : {32u,64u,32u}) {
            const auto target = device.Create_Texture({width,32,1,RHITextureFormat::RGBA8_UNorm,
                static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth = device.Create_Texture({width,32,1,RHITextureFormat::D32_Float,
                static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
            BOOST_REQUIRE(commands.Set_Viewport({0,0,width,32}));
            for (int level : {1,0,1,0}) {
                children.Select_Level(level, [](const auto&) {}, [](const auto&) {});
                BOOST_REQUIRE(commands.Clear({0,0,0,0}, 1));
                children.Visit_Level(children.Current_Level(), [&](const auto& child, int) {
                    std::array<PropVertex,4> vertices{};
                    const auto& geometry = *child.model;
                    vertices[0].position = {geometry.center-.18f,-.6f,.5f};
                    vertices[1].position = {geometry.center+.18f,-.6f,.5f};
                    vertices[2].position = {geometry.center+.18f,.6f,.5f};
                    vertices[3].position = {geometry.center-.18f,.6f,.5f};
                    for (auto& vertex : vertices) vertex.color = geometry.color;
                    const auto mesh = renderer.Create_Mesh(vertices, std::array<std::uint32_t,6>{0,1,2,0,2,3});
                    BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters, {}));
                    renderer.Destroy_Mesh(mesh);
                });
                std::vector<std::byte> pixels(width*32*4);
                BOOST_REQUIRE(device.Readback_Texture(target, pixels, width*4));
                for (unsigned row : {10u,22u}) {
                    for (unsigned slot = 0; slot < 3; ++slot) {
                        const unsigned column = width*(slot+1)/4;
                        std::array<unsigned,4> expected{};
                        if (slot == 1) expected = {0,0,255,255};
                        else if (slot == 0 && level == 0) expected = {255,0,0,128};
                        else if (slot == 2 && level == 1) expected = {0,255,0,191};
                        BOOST_TEST_CONTEXT("warp=" << warp << " width=" << width << " level=" << level << " row=" << row << " slot=" << slot) {
                            for (unsigned channel = 0; channel < 4; ++channel) {
                                const unsigned actual = std::to_integer<unsigned>(pixels[(row*width+column)*4+channel]);
                                // Hardware interpolation may land on either side
                                // of the half-byte alpha quantization boundary.
                                if (channel == 3 && expected[channel] == 128) {
                                    BOOST_CHECK_GE(actual, 127u);
                                    BOOST_CHECK_LE(actual, 128u);
                                } else BOOST_CHECK_EQUAL(actual, expected[channel]);
                            }
                        }
                    }
                }
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]), 0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[3]), 0u);
            }
            device.Destroy_Texture(target);
            device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
