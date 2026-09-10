/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <array>
#include <ranges>
#include <limits>
#include <cstdlib>
#include <vector>
#include "W3DDevice/GameClient/W3DMeshGeometry.h"
#include "W3DDevice/GameClient/W3DMeshQueries.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWLib/chunkio.h"
#include "WWMath/aabox.h"
#include "WWMath/obbox.h"
#include "WWMath/sphere.h"
#include "WWMath/plane.h"
#include "WWDebug/wwdebug.h"
#include "WWDebug/wwmemlog.h"
#include "WWMath/vp.h"
import Assets.Adapters.W3D.Chunks;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.Models.GeometryMath;
import Graphics.Scene.Models.GeometryQueries;
import Assets.Adapters.W3D.Geometry;
import Assets.Adapters.W3D.MeshData;
import Assets.Math;
import Assets.MeshBoundsTree;


W3DMeshGeometry::W3DMeshGeometry() = default;

W3DMeshGeometry::W3DMeshGeometry(const W3DMeshGeometry& that)
    : Geometry(that.Geometry), Flags(that.Flags), SortLevel(that.SortLevel), W3dAttributes(that.W3dAttributes),
      CullTree(that.CullTree ? std::make_unique<Graphics::ModelBoundsTree>(*that.CullTree) : nullptr)
{
}

W3DMeshGeometry& W3DMeshGeometry::operator=(const W3DMeshGeometry& that)
{
    if (this != &that) {
        Geometry = that.Geometry;
        Flags = that.Flags;
        SortLevel = that.SortLevel;
        W3dAttributes = that.W3dAttributes;
        CullTree = that.CullTree ? std::make_unique<Graphics::ModelBoundsTree>(*that.CullTree) : nullptr;
    }
    return *this;
}

W3DMeshGeometry::~W3DMeshGeometry()
{
    Reset_Geometry(0, 0);
}

void W3DMeshGeometry::Reset_Geometry(int polycount, int vertcount)
{
    Geometry.Reset(polycount, vertcount);
    Flags = 0;
    SortLevel = Assets::W3D::W3DMeshSortLevelNone;
    CullTree.reset();
}

const char * W3DMeshGeometry::Get_Name() const
{
    return Geometry.name ? Geometry.name->c_str() : nullptr;
}


void W3DMeshGeometry::Set_Name(const char * newname)
{
    Geometry.name = newname ? std::make_shared<const std::string>(newname) : nullptr;
}


const char * W3DMeshGeometry::Get_User_Text()
{
    return Geometry.user_text ? Geometry.user_text->c_str() : nullptr;
}


void W3DMeshGeometry::Set_User_Text(char * usertext)
{
    Geometry.user_text = usertext ? std::make_shared<const std::string>(usertext) : nullptr;
}


void W3DMeshGeometry::Get_Bounding_Box(AABoxClass * set_box)
{
	WWASSERT(set_box != nullptr);
	set_box->Center = (Geometry.maximum + Geometry.minimum) * 0.5f;
	set_box->Extent = (Geometry.maximum - Geometry.minimum) * 0.5f;
}


void W3DMeshGeometry::Get_Bounding_Sphere(SphereClass * set_sphere)
{
	WWASSERT(set_sphere != nullptr);
	set_sphere->Center = Geometry.sphere_center;
	set_sphere->Radius = Geometry.sphere_radius;
}


void W3DMeshGeometry::Generate_Rigid_APT(const Vector3 & view_dir, SimpleDynVecClass<uint32> & apt)
{
    const MeshQueryAdapter::Triangles triangles(*this);
    Graphics::Collect_Model_Polygons(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return Vector3::Dot_Product(*triangle.N, view_dir) < 0.0f;
        }); }, [&](std::uint32_t polygon) { apt.Add(polygon); });
}


void W3DMeshGeometry::Generate_Rigid_APT(const OBBoxClass & local_box, SimpleDynVecClass<uint32> & apt)
{
    if (CullTree) { MeshQueryAdapter::Collect(*CullTree, *this, local_box, apt); return; }
    const MeshQueryAdapter::Triangles triangles(*this);
    Graphics::Collect_Model_Polygons(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return CollisionMath::Intersection_Test(local_box, triangle);
        }); }, [&](std::uint32_t polygon) { apt.Add(polygon); });
}


void W3DMeshGeometry::Generate_Rigid_APT(const OBBoxClass & local_box,const Vector3 & viewdir,SimpleDynVecClass<uint32> & apt)
{
    if (CullTree) { MeshQueryAdapter::Collect(*CullTree, *this, local_box, apt, &viewdir); return; }
    const MeshQueryAdapter::Triangles triangles(*this);
    Graphics::Collect_Model_Polygons(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return Vector3::Dot_Product(*triangle.N, viewdir) < 0.0f && CollisionMath::Intersection_Test(local_box, triangle);
        }); }, [&](std::uint32_t polygon) { apt.Add(polygon); });
}

void W3DMeshGeometry::Generate_Skin_APT(const OBBoxClass & world_box, SimpleDynVecClass<uint32> & apt, const Vector3 *world_vertex_locs)
{
    WWASSERT(world_vertex_locs);
    const MeshQueryAdapter::Triangles triangles(*this, world_vertex_locs);
    Graphics::Collect_Model_Polygons(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return CollisionMath::Intersection_Test(world_box, triangle);
        }); }, [&](std::uint32_t polygon) { apt.Add(polygon); });
}


bool W3DMeshGeometry::Contains(const Vector3 &point)
{
    return Graphics::Model_Contains_Point([&](int axis) {
        unsigned char flags = TRI_RAYCAST_FLAG_NONE;
        const auto count = cast_semi_infinite_axis_aligned_ray(point, axis, flags);
        return Graphics::ModelAxisRayVote{static_cast<unsigned>(count),
            (flags & TRI_RAYCAST_FLAG_HIT_EDGE) != 0, (flags & TRI_RAYCAST_FLAG_START_IN_TRI) != 0};
    });
}


bool W3DMeshGeometry::Cast_Ray(W3DRayCastQuery & raytest)
{
	bool hit = false;

	if (CullTree) {
		hit = MeshQueryAdapter::Cast(*CullTree,*this,raytest);
	} else {
		hit = cast_ray_brute_force(raytest);
	}

	return hit;
}


bool W3DMeshGeometry::Cast_AABox(W3DBoxCastQuery & boxtest)
{
	bool hit = false;

	if (CullTree) {
		hit = MeshQueryAdapter::Cast(*CullTree,*this,boxtest);
	} else {
		hit = cast_aabox_brute_force(boxtest);
	}

	return hit;
}


bool W3DMeshGeometry::Cast_OBBox(W3DOrientedBoxCastQuery & boxtest)
{
	bool hit = false;

	if (CullTree) {
		hit = MeshQueryAdapter::Cast(*CullTree,*this,boxtest);
	} else {
		hit = cast_obbox_brute_force(boxtest);
	}

	return hit;
}


bool W3DMeshGeometry::Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest)
{
	bool hit = false;

	if (CullTree) {
		hit = MeshQueryAdapter::Intersects(*CullTree,*this,boxtest);
	} else {
		hit = intersect_obbox_brute_force(boxtest);
	}

	return hit;
}


bool W3DMeshGeometry::Cast_World_Space_AABox(W3DBoxCastQuery & boxtest, const Matrix3D &transform)
{
	/*
	** Attempt to classify the transform:
	** NOTE: This code assumes that the matrix is orthogonal!  Much code in W3D assumes
	** orthogonal matrices, if that convention is broken this code is also broken.
	** Basically what I'm doing here is doing the minimum number of compares needed
	** to identify a transform which is a 90 degree rotation about the z axis.
	** TODO: cache some transform flags somewhere to reduce the number of times
	** that these compares are done
	*/
	bool hit = false;

	if ((transform[0][0] == 1.0f) && (transform[1][1] == 1.0f)) {

		hit = cast_aabox_identity(boxtest,-transform.Get_Translation());

	} else if ((transform[0][1] == -1.0f) && (transform[1][0] == 1.0f)) {

		// this mesh has been rotated 90 degrees about z
		hit = cast_aabox_z90(boxtest,-transform.Get_Translation());

	} else if ((transform[0][0] == -1.0f) && (transform[1][1] == -1.0f)) {

		// this mesh has been rotated 180
		hit = cast_aabox_z180(boxtest,-transform.Get_Translation());

	} else if ((transform[0][1] == 1.0f) && (transform[1][0] == -1.0f)) {

		// this mesh has been rotated 270
		hit = cast_aabox_z270(boxtest,-transform.Get_Translation());

	} else {

		/*
		** Ok, we fell through to here, that means there is a more general
		** transform on this mesh.  In this case, I create a new test which
		** is an oriented box test in the coordinate system of the mesh and cast it.
		*/
		Matrix3D world_to_obj;
		transform.Get_Orthogonal_Inverse(world_to_obj);
		W3DOrientedBoxCastQuery obbox(boxtest, world_to_obj);

		if (CullTree) {
			hit = MeshQueryAdapter::Cast(*CullTree,*this,obbox);
		} else {
			hit = cast_obbox_brute_force(obbox);
		}

		/*
		** now, we must transform the results of the test back to the original
		** coordinate system.
		*/
		if (hit) {
			Matrix3D::Rotate_Vector(transform, obbox.Result->Normal, &(obbox.Result->Normal));
			if (boxtest.Result->ComputeContactPoint) {
				Matrix3D::Transform_Vector(transform, obbox.Result->ContactPoint, &(obbox.Result->ContactPoint));
			}
		}
	}

	return hit;
}


int W3DMeshGeometry::cast_semi_infinite_axis_aligned_ray(const Vector3 & start_point, int axis_dir,
	unsigned char & flags)
{
    if (CullTree) return MeshQueryAdapter::Count_Axis_Ray(*CullTree, *this, start_point, axis_dir, flags);
    WWASSERT(axis_dir >= 0 && axis_dir < 6);
    const int axis = axis_dir / 2, first = (axis + 1) % 3, second = (axis + 2) % 3;
    const int direction = (axis_dir & 1) == 0;
    const auto* vertices = Peek_Vertex_Array();
    const auto* triangles = Get_Polygon_Array();
    const auto* planes = Get_Plane_Array();
    flags = TRI_RAYCAST_FLAG_NONE;
    return Graphics::Count_Model_Axis_Intersections(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())), [&](std::uint32_t polygon) {
        const auto& indices = triangles[polygon];
        return Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(vertices[indices[0]], vertices[indices[1]],
            vertices[indices[2]], planes[polygon], start_point, axis, first, second, direction, flags);
    });
}


bool W3DMeshGeometry::cast_aabox_identity(W3DBoxCastQuery & boxtest, const Vector3 & translation)
{
	// transform the test into the mesh's coordinate system
	W3DBoxCastQuery newbox(boxtest);
	newbox.Translate(translation);

	// cast the box against the mesh
	if (CullTree) {
		return MeshQueryAdapter::Cast(*CullTree,*this,newbox);
	} else {
		return cast_aabox_brute_force(newbox);
	}
}


bool W3DMeshGeometry::cast_aabox_z90(W3DBoxCastQuery & boxtest, const Vector3 & translation)
{
	// transform the test into the mesh's coordinate system
	W3DBoxCastQuery newbox(boxtest);
	newbox.Translate(translation);
	newbox.Rotate(W3DBoxCastQuery::ROTATE_Z270);

	// cast the box against the mesh, using culling if possible
	bool hit;
	if (CullTree) {
		hit = MeshQueryAdapter::Cast(*CullTree,*this,newbox);
	} else {
		hit = cast_aabox_brute_force(newbox);
	}

	// if we hit something, we need to rotate the normal back out of the mesh coordinate system
	if (hit) {
		// rotating the normal by 90 degrees about Z
		float tmp = boxtest.Result->Normal.X;
		boxtest.Result->Normal.X = -boxtest.Result->Normal.Y;
		boxtest.Result->Normal.Y = tmp;
	}

	return hit;
}


bool W3DMeshGeometry::cast_aabox_z180(W3DBoxCastQuery & boxtest, const Vector3 & translation)
{
	// transform the test into the meshes coordinate system
	W3DBoxCastQuery newbox(boxtest);
	newbox.Translate(translation);
	newbox.Rotate(W3DBoxCastQuery::ROTATE_Z180);

	// cast the box against the mesh, using culling if possible
	bool hit;
	if (CullTree) {
		hit = MeshQueryAdapter::Cast(*CullTree,*this,newbox);
	} else {
		hit = cast_aabox_brute_force(newbox);
	}

	// if we hit something, we need to rotate the normal back out of the mesh coordinate system
	if (hit) {
		// rotating the normal by 180 degrees about Z
		boxtest.Result->Normal.X = -boxtest.Result->Normal.X;
		boxtest.Result->Normal.Y = -boxtest.Result->Normal.Y;
	}

	return hit;
}


bool W3DMeshGeometry::cast_aabox_z270(W3DBoxCastQuery & boxtest, const Vector3 & translation)
{
	// transform the test into the mesh's coordinate system
	W3DBoxCastQuery newbox(boxtest);
	newbox.Translate(translation);
	newbox.Rotate(W3DBoxCastQuery::ROTATE_Z90);

	// cast the box against the mesh, using culling if possible
	bool hit;
	if (CullTree) {
		hit = MeshQueryAdapter::Cast(*CullTree,*this,newbox);
	} else {
		hit = cast_aabox_brute_force(newbox);
	}

	// if we hit something, we need to rotate the normal back out of the mesh coordinate system
	if (hit) {
		// rotating the normal by 270 degrees about Z
		float tmp = boxtest.Result->Normal.X;
		boxtest.Result->Normal.X = boxtest.Result->Normal.Y;
		boxtest.Result->Normal.Y = -tmp;
	}

	return hit;
}


bool W3DMeshGeometry::intersect_obbox_brute_force(W3DOrientedBoxIntersectionQuery & localtest)
{
    const MeshQueryAdapter::Triangles triangles(*this);
    return Graphics::Intersect_Model_Polygons(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return CollisionMath::Intersection_Test(localtest.Box, triangle);
        }); });
}


bool W3DMeshGeometry::cast_ray_brute_force(W3DRayCastQuery & raytest)
{
    const MeshQueryAdapter::Triangles triangles(*this);
    return Graphics::Cast_Model_Ray(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return MeshQueryAdapter::Collide(raytest, triangle);
        }); }, [&] { return raytest.Result->StartBad; },
        [&](std::uint32_t polygon) { raytest.Result->SurfaceType = Get_Poly_Surface_Type(polygon); });
}


bool W3DMeshGeometry::cast_aabox_brute_force(W3DBoxCastQuery & boxtest)
{
    const MeshQueryAdapter::Triangles triangles(*this);
    return Graphics::Cast_Model_Volume(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return MeshQueryAdapter::Collide(boxtest, triangle);
        }); }, [&] { return boxtest.Result->StartBad; },
        [&](std::uint32_t polygon) { boxtest.Result->SurfaceType = Get_Poly_Surface_Type(polygon); });
}


bool W3DMeshGeometry::cast_obbox_brute_force(W3DOrientedBoxCastQuery & boxtest)
{
    const MeshQueryAdapter::Triangles triangles(*this);
    return Graphics::Cast_Model_Volume(std::views::iota(std::uint32_t{0}, static_cast<std::uint32_t>(Get_Polygon_Count())),
        [&](std::uint32_t polygon) { return triangles.Test(polygon, [&](const TriClass& triangle) {
            return MeshQueryAdapter::Collide(boxtest, triangle);
        }); }, [&] { return boxtest.Result->StartBad; },
        [&](std::uint32_t polygon) { boxtest.Result->SurfaceType = Get_Poly_Surface_Type(polygon); });
}


void W3DMeshGeometry::Compute_Plane_Equations(Vector4 * peq)
{
    Graphics::Compute_Model_Planes(std::span<const Vector3>(*Geometry.positions),
        std::span<const TriIndex>(*Geometry.triangles), std::span(peq, static_cast<std::size_t>(Geometry.polygon_count)));
    Set_Flag(DIRTY_PLANES, false);
}


void W3DMeshGeometry::Compute_Vertex_Normals(Vector3 * vnorm)
{
    Geometry.revision.Invalidate();
    WWASSERT(vnorm);
    if (Geometry.polygon_count == 0 || Geometry.vertex_count == 0) return;
    const auto* planes = Get_Plane_Array();
    const auto* shade_indices = Get_Vertex_Shade_Index_Array(false);
    Graphics::Compute_Model_Normals(std::span<const TriIndex>(*Geometry.triangles),
        std::span(planes, static_cast<std::size_t>(Geometry.polygon_count)),
        std::span(shade_indices, shade_indices ? static_cast<std::size_t>(Geometry.vertex_count) : 0),
        std::span(vnorm, static_cast<std::size_t>(Geometry.vertex_count)));
    Set_Flag(DIRTY_VNORMALS, false);
}


void W3DMeshGeometry::Compute_Bounds(Vector3 * verts)
{
    if (!verts && Geometry.positions) verts = Geometry.positions->data();
    if (Graphics::Compute_Model_Bounds(std::span<const Vector3>(verts, static_cast<std::size_t>(Geometry.vertex_count)),
        Geometry.minimum, Geometry.maximum, Geometry.sphere_center, Geometry.sphere_radius))
        Set_Flag(DIRTY_BOUNDS, false);
}


Vector3 * W3DMeshGeometry::get_vert_normals()
{
    WWASSERT(Geometry.normals);
    return Geometry.normals->data();
}


const Vector3 * W3DMeshGeometry::Get_Vertex_Normal_Array()
{
    if (Get_Flag(DIRTY_VNORMALS)) Compute_Vertex_Normals(get_vert_normals());
    return get_vert_normals();
}


Vector4 * W3DMeshGeometry::get_planes(bool create)
{
    return Geometry.Planes();
}


const Vector4 * W3DMeshGeometry::Get_Plane_Array(bool create)
{
    auto* planes = get_planes(create);
    Compute_Plane_Equations(planes);
    return planes;
}

void W3DMeshGeometry::Compute_Plane(int pidx,PlaneClass * set_plane) const
{
	WWASSERT(pidx >= 0);
	WWASSERT(pidx < Geometry.polygon_count);
	TriIndex & poly = Geometry.triangles->data()[pidx];
	Vector3 * verts = Geometry.positions->data();

	set_plane->Set(verts[poly.I],verts[poly.J],verts[poly.K]);
}


void W3DMeshGeometry::Generate_Culling_Tree()
{
 WWMEMLOG(MEM_CULLINGDATA);
 std::vector<Assets::Vector3f> vertices(Geometry.vertex_count);
 for (int i = 0; i < Geometry.vertex_count; ++i) {
  const auto& source = Geometry.positions->data()[i];
  vertices[i] = {source.X, source.Y, source.Z};
 }
 std::vector<std::array<std::uint32_t,3>> triangles(Geometry.polygon_count);
 for (int i = 0; i < Geometry.polygon_count; ++i) {
  const auto& source = Geometry.triangles->data()[i];
  triangles[i] = {source[0], source[1], source[2]};
 }
 Assets::MeshBoundsTree tree;
 const bool built = Assets::Build_Mesh_Bounds_Tree(vertices, triangles,
  [] { return static_cast<unsigned>(rand()); }, tree);
 WWASSERT(built);
 if (!built) return;
 DEBUG_ASSERTCRASH(CullTree == nullptr, ("W3DMeshGeometry::Generate_Culling_Tree: Leaking CullTree"));
 CullTree = std::make_unique<Graphics::ModelBoundsTree>(std::move(tree));
}


bool W3DMeshGeometry::Can_Install_Geometry(const Assets::W3D::W3DMeshData& mesh) {
    if (mesh.header.vertex_count > static_cast<unsigned>((std::numeric_limits<int>::max)())
        || mesh.header.triangle_count > static_cast<unsigned>((std::numeric_limits<int>::max)() / 3)) return false;
    for (const auto& triangle : mesh.triangles)
        for (const auto index : triangle.indices)
            if (index > (std::numeric_limits<unsigned short>::max)()) return false;
    return true;
}

void W3DMeshGeometry::Install_Geometry(const Assets::W3D::W3DMeshData& mesh) {
    const auto& header = mesh.header;
    W3dAttributes = header.attributes;
    SortLevel = static_cast<char>(header.sort_level);
    const auto name = header.container_name.empty() ? header.name : header.container_name + "." + header.name;
    Set_Name(name.c_str());
    Geometry.minimum = {header.bounds.minimum.x, header.bounds.minimum.y, header.bounds.minimum.z};
    Geometry.maximum = {header.bounds.maximum.x, header.bounds.maximum.y, header.bounds.maximum.z};
    Geometry.sphere_center = {header.sphere_center.x, header.sphere_center.y, header.sphere_center.z};
    Geometry.sphere_radius = header.sphere_radius;
    for (std::size_t i = 0; i < mesh.positions.size(); ++i) {
        const auto& value = mesh.positions[i];
        (*Geometry.positions)[i] = {value.x, value.y, value.z};
    }
    for (std::size_t i = 0; i < mesh.normals.size(); ++i) {
        const auto& value = mesh.normals[i];
        (*Geometry.normals)[i] = {value.x, value.y, value.z};
    }
    auto* planes = Geometry.Planes();
    for (std::size_t i = 0; i < mesh.triangles.size(); ++i) {
        const auto& value = mesh.triangles[i];
        (*Geometry.triangles)[i] = {static_cast<unsigned short>(value.indices[0]),
            static_cast<unsigned short>(value.indices[1]), static_cast<unsigned short>(value.indices[2])};
        (*Geometry.surface_types)[i] = static_cast<std::uint8_t>(value.surface_type);
        planes[i] = {value.normal.x, value.normal.y, value.normal.z, -value.distance};
    }
    if (!mesh.shade_indices.empty()) {
        auto* shades = Geometry.Shade_Indices(true);
        for (std::size_t i = 0; i < mesh.shade_indices.size(); ++i) shades[i] = mesh.shade_indices[i];
    }
    if (mesh.bone_indices) {
        Geometry.bone_indices = std::make_shared<std::vector<std::uint16_t>>(*mesh.bone_indices);
        Set_Flag(SKIN, true);
    }
    if (mesh.user_text) Geometry.user_text = std::make_shared<const std::string>(*mesh.user_text);
    if (mesh.bounds_tree) CullTree = std::make_unique<Graphics::ModelBoundsTree>(*mesh.bounds_tree);
    if (header.version >= Assets::W3D::W3DMeshVersion4_1) {
        switch (header.attributes & Assets::W3D::W3DMeshAttributeGeometryTypeMask) {
        case Assets::W3D::W3DMeshAttributeGeometryTypeCameraAligned: Set_Flag(ALIGNED, true); break;
        case Assets::W3D::W3DMeshAttributeGeometryTypeCameraOriented: Set_Flag(ORIENTED, true); break;
        case Assets::W3D::W3DMeshAttributeGeometryTypeSkin: Set_Flag(SKIN, true); break;
        default: break;
        }
    }
    if (header.attributes & Assets::W3D::W3DMeshAttributeTwoSided) Set_Flag(TWO_SIDED, true);
    if (header.attributes & Assets::W3D::W3DMeshAttributeCastShadow) Set_Flag(CAST_SHADOW, true);
    Geometry.revision.Invalidate();
}

bool W3DMeshGeometry::Load_W3D(ChunkLoadClass& cload) {
    std::vector<std::byte> bytes(cload.Cur_Chunk_Length());
    if (cload.Read(bytes.data(), static_cast<unsigned>(bytes.size())) != bytes.size()) return false;
    Assets::W3D::W3DMeshData mesh;
    std::string error;
    if (!Assets::W3D::W3DRead_Mesh_Data(bytes, Assets::W3D::W3DMeshPrelighting::Unlit, mesh, error)
        || !Can_Install_Geometry(mesh)) return false;
    Reset_Geometry(static_cast<int>(mesh.header.triangle_count), static_cast<int>(mesh.header.vertex_count));
    Install_Geometry(mesh);
    if ((W3dAttributes & Assets::W3D::W3DMeshAttributeCollisionTypeMask) && !CullTree)
        Generate_Culling_Tree();
    return true;
}

void W3DMeshGeometry::Scale(const Vector3 &sc)
{
    Geometry.revision.Invalidate();
    WWASSERT(Geometry.positions);
    if (Graphics::Scale_Model_Geometry(std::span<Vector3>(*Geometry.positions), Geometry.minimum, Geometry.maximum,
        Geometry.sphere_center, Geometry.sphere_radius, sc)) Set_Flag(DIRTY_VNORMALS, true);
    Set_Flag(DIRTY_PLANES, true);
	// the cull tree is invalid, release it and make a new one
	if (CullTree) {
		// If the scale is uniform, we can scale the cull tree, which is a lot faster than creating a new one
		if (fabs(sc[0]-sc[1])<WWMATH_EPSILON && fabs(sc[0]-sc[2])<WWMATH_EPSILON) {
			// create a copy of the old culltree
			CullTree = std::make_unique<Graphics::ModelBoundsTree>(*CullTree);
			CullTree->Scale(sc[0]);
		}
		else {
			CullTree.reset();
			Generate_Culling_Tree();
		}
	}
}


// Destination pointers MUST point to arrays large enough to hold all vertices
void W3DMeshGeometry::get_deformed_vertices(Vector3 *dst_vert,const Graphics::ModelHierarchy * htree)
{
    WWASSERT(htree && Geometry.positions && Geometry.bone_indices);
    Graphics::Deform_Model_Geometry(std::span<const Vector3>(*Geometry.positions), std::span<const Vector3>{},
        std::span<const std::uint16_t>(*Geometry.bone_indices), *htree,
        std::span(dst_vert, static_cast<std::size_t>(Geometry.vertex_count)));
}


// Destination pointers MUST point to arrays large enough to hold all vertices
void W3DMeshGeometry::get_deformed_vertices(Vector3 *dst_vert, Vector3 *dst_norm,const Graphics::ModelHierarchy * htree)
{
    WWASSERT(htree && Geometry.positions && Geometry.normals && Geometry.bone_indices);
    Graphics::Deform_Model_Geometry(std::span<const Vector3>(*Geometry.positions), std::span<const Vector3>(*Geometry.normals),
        std::span<const std::uint16_t>(*Geometry.bone_indices), *htree,
        std::span(dst_vert, static_cast<std::size_t>(Geometry.vertex_count)),
        std::span(dst_norm, static_cast<std::size_t>(Geometry.vertex_count)));
}

