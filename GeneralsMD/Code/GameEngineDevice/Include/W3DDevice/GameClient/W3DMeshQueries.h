#pragma once
#include "W3DDevice/GameClient/W3DMeshGeometry.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWMath/colmathinlines.h"
#include "WWMath/tri.h"
import Assets.Math;
import Graphics.Scene.Models.GeometryQueries;
import Graphics.Scene.Models.BoundsTree;

// Borrow mesh geometry and translate query types; graphics owns traversal and
// leaf-result policy. No mesh or query lifetime is retained here.
namespace MeshQueryAdapter {
class Triangles final {
public:
    explicit Triangles(W3DMeshGeometry& mesh, const Vector3* positions = nullptr)
        : m_positions(positions ? positions : mesh.Peek_Vertex_Array()),
          m_triangles(mesh.Get_Polygon_Array()), m_planes(positions ? nullptr : mesh.Get_Plane_Array()) {}
    template<class Operation>
    bool Test(std::uint32_t polygon, Operation&& operation) const {
        const auto& indices = m_triangles[polygon];
        TriClass triangle;
        for (unsigned corner = 0; corner < 3; ++corner) triangle.V[corner] = &m_positions[indices[corner]];
        const Vector3 normal = m_planes ? Vector3(m_planes[polygon][0], m_planes[polygon][1], m_planes[polygon][2])
            : Vector3(0, 0, 1);
        triangle.N = &normal;
        return operation(triangle);
    }
private:
    const Vector3* m_positions;
    const TriIndex* m_triangles;
    const Vector4* m_planes;
};

template<class Test>
bool Cull(const Assets::Bounds3f& bounds,Test& test)
{
    return test.Cull(Vector3(bounds.minimum.x,bounds.minimum.y,bounds.minimum.z),
        Vector3(bounds.maximum.x,bounds.maximum.y,bounds.maximum.z));
}

inline bool Collide(W3DRayCastQuery& test,const TriClass& triangle)
{ return CollisionMath::Collide(test.Ray,triangle,test.Result); }
inline bool Collide(W3DBoxCastQuery& test,const TriClass& triangle)
{ return CollisionMath::Collide(test.Box,test.Move,triangle,test.Result); }
inline bool Collide(W3DOrientedBoxCastQuery& test,const TriClass& triangle)
{ return CollisionMath::Collide(test.Box,test.Move,triangle,Vector3(0,0,0),test.Result); }

template<class Test>
bool Cast(const Graphics::ModelBoundsTree& tree,W3DMeshGeometry& mesh,Test& test)
{
    const Triangles triangles(mesh);
    return tree.Cast([&](const Assets::Bounds3f& bounds) { return Cull(bounds,test); },
        [&](std::uint32_t polygon) { return triangles.Test(polygon,[&](const TriClass& triangle) { return Collide(test,triangle); }); },
        [&] { return test.Result->StartBad; },
        [&](std::uint32_t polygon) { test.Result->SurfaceType=mesh.Get_Poly_Surface_Type(polygon); });
}

inline bool Intersects(const Graphics::ModelBoundsTree& tree,W3DMeshGeometry& mesh,W3DOrientedBoxIntersectionQuery& test)
{
    const Triangles triangles(mesh);
    return tree.Intersects([&](const Assets::Bounds3f& bounds) { return Cull(bounds,test); },
        [&](std::uint32_t polygon) { return triangles.Test(polygon,[&](const TriClass& triangle) {
            return CollisionMath::Intersection_Test(test.Box,triangle);
        }); });
}

inline void Collect(const Graphics::ModelBoundsTree& tree,W3DMeshGeometry& mesh,const OBBoxClass& box,
    SimpleDynVecClass<uint32>& output,const Vector3* view=nullptr)
{
    const Triangles triangles(mesh);
    tree.Visit([&](const Assets::Bounds3f& bounds) {
        AABoxClass node;
        node.Init_Min_Max(Vector3(bounds.minimum.x,bounds.minimum.y,bounds.minimum.z),
            Vector3(bounds.maximum.x,bounds.maximum.y,bounds.maximum.z));
        return !CollisionMath::Intersection_Test(box,node);
    },[&](std::span<const std::uint32_t> polygons) {
        Graphics::Collect_Model_Polygons(polygons, [&](std::uint32_t polygon) {
            return triangles.Test(polygon,[&](const TriClass& triangle) {
                return (!view || Vector3::Dot_Product(*triangle.N,*view)<0.0f) && CollisionMath::Intersection_Test(box,triangle);
            });
        }, [&](std::uint32_t polygon) { output.Add(polygon); });
    });
}

inline int Count_Axis_Ray(const Graphics::ModelBoundsTree& tree,W3DMeshGeometry& mesh,
    const Vector3& point,int axis_dir,unsigned char& flags)
{
    static const int axes[6]={0,0,1,1,2,2},first_axes[6]={1,1,2,2,0,0},second_axes[6]={2,2,0,0,1,1};
    static const int directions[6]={1,0,1,0,1,0};
    WWASSERT(axis_dir>=0 && axis_dir<6);
    const int axis=axes[axis_dir],first=first_axes[axis_dir],second=second_axes[axis_dir],direction=directions[axis_dir];
    flags=TRI_RAYCAST_FLAG_NONE;
    int count=0;
    const auto* vertices=mesh.Peek_Vertex_Array();
    const auto* triangles=mesh.Get_Polygon_Array();
    const auto* planes=mesh.Get_Plane_Array();
    tree.Visit([&](const Assets::Bounds3f& bounds) {
        const Vector3 minimum(bounds.minimum.x,bounds.minimum.y,bounds.minimum.z),maximum(bounds.maximum.x,bounds.maximum.y,bounds.maximum.z);
        const float ends[2]={-minimum[axis],maximum[axis]},start[2]={-point[axis],point[axis]};
        return point[first]<minimum[first] || point[second]<minimum[second] ||
            point[first]>maximum[first] || point[second]>maximum[second] || start[direction]>ends[direction];
    },[&](std::span<const std::uint32_t> polygons) {
        count += Graphics::Count_Model_Axis_Intersections(polygons, [&](std::uint32_t polygon) {
            const auto& indices=triangles[polygon];
            return Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(vertices[indices[0]],
                vertices[indices[1]],vertices[indices[2]],planes[polygon],point,axis,first,second,direction,flags);
        });
    });
    return count;
}
}
