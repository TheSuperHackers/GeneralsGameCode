module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
export module Graphics.Scene.Models.GeometrySource;
import Graphics.Scene.Models.SourceRevision;

namespace Graphics {

// Application vector layouts remain usable at their boundary. Copies share
// authored channels; explicit position/normal detachment preserves the shared
// mutation domain because triangles and the other channels still alias.
export template<class Position, class Triangle, class Plane, class ShadeIndex = std::uint32_t>
class ModelGeometrySource final {
public:
    void Reset(int polygons, int vertices) {
        assert(polygons >= 0 && vertices >= 0);
        if (polygons != 0 && vertices != 0) revision.Reset();
        else revision.Invalidate();
        name.reset();
        user_text.reset();
        triangles.reset();
        surface_types.reset();
        positions.reset();
        normals.reset();
        shade_indices.reset();
        bone_indices.reset();
        plane_equations.clear();
        polygon_count = polygons;
        vertex_count = vertices;
        if (polygons != 0) {
            triangles = std::make_shared<std::vector<Triangle>>(polygons, Triangle{0, 0, 0});
            surface_types = std::make_shared<std::vector<std::uint8_t>>(polygons, 0);
        }
        if (vertices != 0) {
            positions = std::make_shared<std::vector<Position>>(vertices, Position{0, 0, 0});
            normals = std::make_shared<std::vector<Position>>(vertices, Position{0, 0, 0});
        }
    }

    void Detach_Positions_And_Normals() {
        assert(positions && normals);
        revision.Invalidate();
        positions = std::make_shared<std::vector<Position>>(*positions);
        normals = std::make_shared<std::vector<Position>>(*normals);
    }

    ShadeIndex* Shade_Indices(bool create) {
        if (create && !shade_indices) shade_indices = std::make_shared<std::vector<ShadeIndex>>(vertex_count);
        return shade_indices ? shade_indices->data() : nullptr;
    }
    std::uint16_t* Bone_Indices(bool create) {
        if (create && !bone_indices) bone_indices = std::make_shared<std::vector<std::uint16_t>>(vertex_count);
        return bone_indices ? bone_indices->data() : nullptr;
    }
    Plane* Planes() {
        plane_equations.resize(polygon_count);
        return plane_equations.data();
    }

    std::shared_ptr<const std::string> name;
    std::shared_ptr<const std::string> user_text;
    SourceRevision revision;
    int polygon_count = 0;
    int vertex_count = 0;
    std::shared_ptr<std::vector<Triangle>> triangles;
    std::shared_ptr<std::vector<Position>> positions;
    std::shared_ptr<std::vector<Position>> normals;
    std::shared_ptr<std::vector<ShadeIndex>> shade_indices;
    std::shared_ptr<std::vector<std::uint16_t>> bone_indices;
    std::shared_ptr<std::vector<std::uint8_t>> surface_types;
    // Plane results belong to the query's source, never shared global scratch.
    std::vector<Plane> plane_equations;
    Position minimum{0, 0, 0};
    Position maximum{1, 1, 1};
    Position sphere_center{0, 0, 0};
    float sphere_radius = 1;
};
}
