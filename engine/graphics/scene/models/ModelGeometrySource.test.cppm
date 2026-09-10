module;
#define BOOST_TEST_MODULE ModelGeometrySourceTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>
export module Graphics.Scene.Models.GeometrySource.Tests;
import Graphics.Scene.Models.GeometrySource;
import Graphics.Scene.Models.GeometryMath;
import Graphics.Scene.Models.Deformation;
import Graphics.Scene.Models.GeometryQueries;
import Graphics.Scene.Models.SourceRevision;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.AffineTransform;
import Assets.ModelRig;
namespace {
using Position = std::array<float, 3>;
using Triangle = std::array<std::uint32_t, 3>;
using Plane = std::array<float, 4>;
using Source = Graphics::ModelGeometrySource<Position, Triangle, Plane>;
}
BOOST_AUTO_TEST_CASE(deformation_reuses_only_matching_geometry_bone_links_and_evaluated_pose) {
    Assets::ModelRigDesc rig; rig.skeleton_name = "CACHE";
    rig.bones = {{"ROOT"}, {"CHILD", 0, {2, 0, 0}}};
    Graphics::ModelHierarchy hierarchy(rig);
    auto root = Graphics::Affine_Identity();
    hierarchy.Evaluate_Rest(root);
    std::array<Position, 1> positions{{{1, 2, 3}}}, normals{{{0, 1, 0}}};
    std::array<std::uint16_t, 1> bones{1};
    Graphics::SourceRevision source;
    Graphics::ModelDeformation<Position> cache, other;
    const auto update = [&](auto& target) {
        target.Update(positions, normals, bones, hierarchy, source.Token());
    };
    update(cache);
    auto revision = cache.Revision();
    const auto* storage = cache.Positions().data();
    BOOST_CHECK((cache.Positions()[0] == Position{3, 2, 3}));
    update(cache);
    BOOST_CHECK_EQUAL(cache.Revision(), revision);
    BOOST_CHECK(cache.Positions().data() == storage);
    root.matrix[3] = 5;
    hierarchy.Evaluate_Rest(root); update(cache);
    BOOST_CHECK_NE(cache.Revision(), revision);
    BOOST_CHECK_EQUAL(cache.Positions()[0][0], 8.f);
    hierarchy.Evaluate(root, [](int) {
        Graphics::BoneMotion motion; motion.translate = true; motion.translation = {4, 0, 0}; return motion;
    });
    update(cache); BOOST_CHECK_EQUAL(cache.Positions()[0][0], 12.f);
    hierarchy.Capture(1);
    auto control = Graphics::Affine_Identity(); control.matrix[3] = 3;
    hierarchy.Control(1, control); hierarchy.Evaluate_Rest(root);
    update(cache); BOOST_CHECK_EQUAL(cache.Positions()[0][0], 11.f);
    update(other);
    BOOST_CHECK(other.Positions().data() != cache.Positions().data());
    bones[0] = 0; update(cache);
    BOOST_CHECK_EQUAL(cache.Positions()[0][0], 6.f);
    BOOST_CHECK_EQUAL(other.Positions()[0][0], 11.f);
    positions[0][0] = 2; source.Invalidate(); update(cache);
    BOOST_CHECK_EQUAL(cache.Positions()[0][0], 7.f);
    source.Expose_Writable(); update(cache);
    revision = cache.Revision(); update(cache);
    BOOST_CHECK_EQUAL(cache.Revision(), revision);
    positions[0][0] = 9; normals[0] = {0, 0, 2}; update(cache);
    BOOST_CHECK_NE(cache.Revision(), revision);
    BOOST_CHECK_EQUAL(cache.Positions()[0][0], 14.f);
    BOOST_CHECK((cache.Normals()[0] == Position{0, 0, 2}));
    cache.Update(positions, {}, bones, hierarchy, 0);
    BOOST_CHECK(cache.Normals().empty());
}
BOOST_AUTO_TEST_CASE(planes_grow_past_the_retired_capacity_and_copies_retain_shared_mutation) {
    Source source; source.Reset(1100, 3300);
    source.name = std::make_shared<const std::string>("SOURCE");
    for (unsigned i = 0; i < 1100; ++i) {
        const unsigned v = i * 3;
        (*source.positions)[v] = {0, 0, static_cast<float>(i)};
        (*source.positions)[v + 1] = {2, 0, static_cast<float>(i)};
        (*source.positions)[v + 2] = {0, 3, static_cast<float>(i)};
        (*source.triangles)[i] = {v, v + 1, v + 2};
    }
    Graphics::Compute_Model_Planes(std::span<const Position>(*source.positions),
        std::span<const Triangle>(*source.triangles), std::span(source.Planes(), 1100));
    BOOST_CHECK_EQUAL(source.plane_equations[1099][2], 1.f);
    BOOST_CHECK_EQUAL(source.plane_equations[1099][3], -1099.f);
    auto copy = source; const auto revision = source.revision.Token();
    copy.Detach_Positions_And_Normals();
    BOOST_CHECK_NE(source.revision.Token(), revision);
    BOOST_CHECK_EQUAL(source.revision.Token(), copy.revision.Token());
    (*copy.positions)[0][0] = 19;
    BOOST_CHECK_EQUAL((*source.positions)[0][0], 0.f);
    (*copy.triangles)[0][0] = 2;
    BOOST_CHECK_EQUAL((*source.triangles)[0][0], 2);
    source.Reset(0, 0);
    BOOST_CHECK_EQUAL(*copy.name, "SOURCE");
    BOOST_CHECK_EQUAL(copy.plane_equations[1099][3], -1099.f);
    copy.revision.Expose_Writable(); BOOST_CHECK_EQUAL(copy.revision.Token(), 0u);
}
BOOST_AUTO_TEST_CASE(smoothing_bounds_and_signed_scale_preserve_authored_math) {
    const std::array<Triangle, 2> triangles{{{0, 1, 2}, {3, 4, 5}}};
    const std::array<Plane, 2> planes{{{0, 0, 1, 0}, {0, 1, 0, 0}}};
    const std::array<std::uint32_t, 6> shades{0, 1, 2, 0, 1, 2};
    std::array<Position, 6> normals;
    Graphics::Compute_Model_Normals(std::span<const Triangle>(triangles), std::span<const Plane>(planes),
        std::span<const std::uint32_t>(shades), std::span<Position>(normals));
    for (const auto& normal : normals) {
        BOOST_CHECK_SMALL(normal[0], 1e-6f);
        BOOST_CHECK_SMALL(normal[1] - 0.70710677f, 1e-6f);
        BOOST_CHECK_SMALL(normal[2] - 0.70710677f, 1e-6f);
    }
    std::array<Position, 2> positions{{{-2, -4, 0}, {2, 4, 6}}};
    Position minimum, maximum, center; float radius;
    BOOST_REQUIRE(Graphics::Compute_Model_Bounds(std::span<const Position>(positions), minimum, maximum, center, radius));
    BOOST_CHECK_EQUAL(center[2], 3.f); BOOST_CHECK_SMALL(radius - 5.38521866f, 1e-5f);
    BOOST_CHECK(Graphics::Scale_Model_Geometry(std::span<Position>(positions), minimum, maximum, center, radius, Position{-2, -3, -4}));
    BOOST_CHECK_EQUAL(minimum[0], 4.f); BOOST_CHECK_EQUAL(maximum[0], -4.f);
    BOOST_CHECK_SMALL(radius + 10.7704373f, 2e-5f);
}
BOOST_AUTO_TEST_CASE(deformation_groups_bones_without_normalizing_scaled_normals) {
    Assets::ModelRigDesc rig; rig.skeleton_name = "DEFORM"; rig.bones = {{"ROOT"}, {"CHILD", 0, {2, 3, 4}}};
    Graphics::ModelHierarchy hierarchy; std::string error;
    BOOST_REQUIRE_MESSAGE(hierarchy.Initialize(rig, error), error);
    auto transform = Graphics::Affine_Identity();
    transform.matrix = {0, -3, 0, 10, 2, 0, 0, 20, 0, 0, 4, 30, 0, 0, 0, 1};
    hierarchy.Evaluate_Rest(transform);
    const std::array<Position, 3> positions{{{1, 2, 3}, {1, 2, 3}, {1, 2, 3}}};
    const std::array<Position, 3> normals{{{1, 0, 1}, {1, 0, 1}, {1, 0, 1}}};
    const std::array<std::uint16_t, 3> bones{0, 0, 1};
    std::array<Position, 3> output, normal_output;
    Graphics::Deform_Model_Geometry(std::span<const Position>(positions), std::span<const Position>(normals),
        std::span<const std::uint16_t>(bones), hierarchy, std::span<Position>(output), std::span<Position>(normal_output));
    BOOST_CHECK((output[0] == Position{4, 22, 42}));
    BOOST_CHECK((output[1] == Position{4, 22, 42}));
    BOOST_CHECK((output[2] == Position{-5, 26, 58}));
    BOOST_CHECK((normal_output[2] == Position{0, 2, 4}));
}

BOOST_AUTO_TEST_CASE(ray_and_volume_surface_publication_keep_distinct_start_inside_ordering) {
    const std::array<std::uint32_t, 3> polygons{4, 2, 7};
    for (bool ray : {false, true}) {
        std::vector<unsigned> visited, surfaces; bool start_bad = false;
        const auto intersect = [&](unsigned polygon) {
            visited.push_back(polygon); start_bad = polygon == 2; return true;
        };
        const auto bad = [&] { return start_bad; };
        const auto surface = [&](unsigned polygon) { surfaces.push_back(polygon); };
        const bool hit = ray ? Graphics::Cast_Model_Ray(polygons,intersect,bad,surface)
            : Graphics::Cast_Model_Volume(polygons,intersect,bad,surface);
        BOOST_CHECK(hit); BOOST_CHECK((visited == std::vector<unsigned>{4,2}));
        if (ray) BOOST_CHECK((surfaces == std::vector<unsigned>{4,2}));
        else BOOST_CHECK(surfaces.empty());
    }
    std::vector<unsigned> surfaces;
    BOOST_CHECK(Graphics::Cast_Model_Volume(polygons,[](unsigned polygon) { return polygon != 2; },
        [] { return false; },[&](unsigned polygon) { surfaces.push_back(polygon); }));
    BOOST_CHECK((surfaces == std::vector<unsigned>{7}));
    std::vector<unsigned> selected;
    Graphics::Collect_Model_Polygons(polygons,[](unsigned polygon) { return polygon != 2; },
        [&](unsigned polygon) { selected.push_back(polygon); });
    BOOST_CHECK((selected == std::vector<unsigned>{4,7}));
}
BOOST_AUTO_TEST_CASE(containment_uses_weighted_six_axis_votes_and_stops_only_when_embedded) {
    unsigned count = 0;
    BOOST_CHECK(!Graphics::Model_Contains_Point([&](int axis) {
        ++count; return Graphics::ModelAxisRayVote{static_cast<unsigned>(axis < 3), false, false};
    }));
    BOOST_CHECK_EQUAL(count, 6);
    BOOST_CHECK(!Graphics::Model_Contains_Point([](int axis) {
        return Graphics::ModelAxisRayVote{static_cast<unsigned>(axis < 4), axis < 4, false};
    }));
    count = 0;
    BOOST_CHECK(Graphics::Model_Contains_Point([&](int axis) {
        ++count; return Graphics::ModelAxisRayVote{0, false, axis == 2};
    }));
    BOOST_CHECK_EQUAL(count, 3);
}
