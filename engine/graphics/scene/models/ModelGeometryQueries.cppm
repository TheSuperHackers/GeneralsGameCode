module;
#include <cstddef>
#include <cstdint>
#include <optional>
export module Graphics.Scene.Models.GeometryQueries;
namespace Graphics {
// Polygon order and result publication differ between unpartitioned ray and
// volume queries. Callers supply primitive intersection math and result access.
export template<class Range, class Intersect, class StartBad, class Surface>
bool Cast_Model_Ray(const Range& polygons, Intersect&& intersect, StartBad&& start_bad, Surface&& surface) {
    bool hit = false;
    for (const auto polygon : polygons) {
        if (intersect(polygon)) { hit = true; surface(polygon); }
        if (start_bad()) return true;
    }
    return hit;
}
export template<class Range, class Intersect, class StartBad, class Surface>
bool Cast_Model_Volume(const Range& polygons, Intersect&& intersect, StartBad&& start_bad, Surface&& surface) {
    std::optional<std::uint32_t> last_hit;
    for (const auto polygon : polygons) {
        if (intersect(polygon)) last_hit = polygon;
        if (start_bad()) return true;
    }
    if (!last_hit) return false;
    surface(*last_hit);
    return true;
}
export template<class Range, class Intersect>
bool Intersect_Model_Polygons(const Range& polygons, Intersect&& intersect) {
    for (const auto polygon : polygons) if (intersect(polygon)) return true;
    return false;
}
export template<class Range, class Intersect, class Append>
void Collect_Model_Polygons(const Range& polygons, Intersect&& intersect, Append&& append) {
    for (const auto polygon : polygons) if (intersect(polygon)) append(polygon);
}
export struct ModelAxisRayVote final {
    unsigned intersections = 0;
    bool hit_edge = false;
    bool starts_in_triangle = false;
};
export template<class CastAxis>
bool Model_Contains_Point(CastAxis&& cast_axis) {
    float yes = 0;
    float no = 0;
    for (int axis = 0; axis < 6; ++axis) {
        const auto result = cast_axis(axis);
        if (result.starts_in_triangle) return true;
        const float weight = result.hit_edge ? .1f : 1.f;
        if (result.intersections & 1u) yes += weight;
        else no += weight;
    }
    return yes > no;
}
export template<class Range, class CastTriangle>
unsigned Count_Model_Axis_Intersections(const Range& polygons, CastTriangle&& cast_triangle) {
    unsigned count = 0;
    for (const auto polygon : polygons) count += static_cast<unsigned>(cast_triangle(polygon));
    return count;
}
}
