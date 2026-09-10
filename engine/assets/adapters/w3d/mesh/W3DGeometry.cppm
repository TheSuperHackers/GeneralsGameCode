module;
#include <array>
#include <bit>
#include <string>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include <limits>
export module Assets.Adapters.W3D.Geometry;
import Assets.Adapters.W3D.Chunks;
import Assets.Math;
import Assets.MeshBoundsTree;

namespace Assets::W3D {

export inline constexpr std::uint32_t W3DMeshVersion4_1 = 0x00040001u;
export inline constexpr std::uint32_t W3DMeshAttributeCollisionTypeMask = 0x00000FF0u;
export inline constexpr std::uint32_t W3DMeshAttributeCollisionTypeShift = 4u;
export inline constexpr std::uint32_t W3DMeshAttributeHidden = 0x00001000u;
export inline constexpr std::uint32_t W3DMeshAttributeTwoSided = 0x00002000u;
export inline constexpr std::uint32_t W3DMeshAttributeObsoleteLightmapped = 0x00004000u;
export inline constexpr std::uint32_t W3DMeshAttributeCastShadow = 0x00008000u;
export inline constexpr std::uint32_t W3DMeshAttributeGeometryTypeMask = 0x00FF0000u;
export inline constexpr std::uint32_t W3DMeshAttributeGeometryTypeCameraAligned = 0x00010000u;
export inline constexpr std::uint32_t W3DMeshAttributeGeometryTypeSkin = 0x00020000u;
export inline constexpr std::uint32_t W3DMeshAttributeGeometryTypeCameraOriented = 0x00060000u;
export inline constexpr std::uint32_t W3DMeshAttributePrelitMask = 0x0F000000u;
export inline constexpr std::uint32_t W3DMeshAttributeNPatchable = 0x20000000u;
export inline constexpr int W3DMeshSortLevelNone = 0;

export bool W3DRead_Mesh_Bounds_Tree(W3DByteSpan bytes, std::uint32_t mesh_triangle_count,
    MeshBoundsTree& result)
{
    W3DByteSpan header, polygons, nodes;
    bool has_header=false, has_polygons=false, has_nodes=false;
    if(!W3DVisit_Chunks(bytes,[&](const W3DChunkView& chunk) {
        switch(chunk.id) {
            case 0x91: header=chunk.payload;has_header=true;break;
            case 0x92: polygons=chunk.payload;has_polygons=true;break;
            case 0x93: nodes=chunk.payload;has_nodes=true;break;
        }
        return true;
    }) || !has_header || !has_polygons || !has_nodes || header.size()<32) return false;
    std::uint32_t node_count=0, polygon_count=0;
    if(!W3DRead_U32(header,0,node_count) || !W3DRead_U32(header,4,polygon_count) || !node_count ||
        node_count>static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()) ||
        polygon_count>static_cast<std::uint32_t>((std::numeric_limits<std::int32_t>::max)()) ||
        node_count>nodes.size()/32 || nodes.size()!=std::size_t(node_count)*32 ||
        polygon_count>polygons.size()/4 || polygons.size()!=std::size_t(polygon_count)*4) return false;
    MeshBoundsTree tree;
    tree.nodes.resize(node_count);tree.polygon_indices.resize(polygon_count);
    for(std::size_t i=0;i<polygon_count;++i)
        if(!W3DRead_U32(polygons,i*4,tree.polygon_indices[i]) || tree.polygon_indices[i]>=mesh_triangle_count)
            return false;
    for(std::size_t i=0;i<node_count;++i) {
        auto& node=tree.nodes[i];const auto offset=i*32;
        std::uint32_t first=0;
        if(!W3DRead_Vector3(nodes,offset,node.bounds.minimum) ||
            !W3DRead_Vector3(nodes,offset+12,node.bounds.maximum) || !node.bounds.Is_Valid() ||
            !W3DRead_U32(nodes,offset+24,first) || !W3DRead_U32(nodes,offset+28,node.second)) return false;
        node.leaf=(first&0x80000000u)!=0;
        node.first=first&0x7fffffffu;
        if(node.leaf) {
            if(node.first>polygon_count || node.second>polygon_count-node.first) return false;
        } else if(node.first>=node_count || node.second>=node_count) return false;
    }
    // Validate the reachable hierarchy without recursing through untrusted data.
    std::vector<bool> visited(node_count);
    std::vector<std::uint32_t> pending{0};
    while(!pending.empty()) {
        const auto index=pending.back();pending.pop_back();
        if(visited[index]) return false;
        visited[index]=true;
        const auto& node=tree.nodes[index];
        if(!node.leaf) { pending.push_back(node.second);pending.push_back(node.first); }
    }
    result=std::move(tree);
    return true;
}

export struct W3DMeshHeader final
{
	std::uint32_t version = 0;
	std::uint32_t attributes = 0;
	std::string name;
	std::string container_name;
	std::uint32_t triangle_count = 0;
	std::uint32_t vertex_count = 0;
	std::uint32_t material_count = 0;
	std::int32_t sort_level = 0;
	std::uint32_t vertex_channels = 0;
	std::uint32_t face_channels = 0;
	Bounds3f bounds{};
	Vector3f sphere_center{};
	float sphere_radius = 0.0f;
};

export bool W3DRead_Mesh_Header(W3DByteSpan bytes, W3DMeshHeader &result)
{
	// GeneralsMD writes the 116-byte W3dMeshHeader3Struct. Reading by offset
	// avoids compiler packing and keeps WW3D2 types outside this boundary.
	if (bytes.size() < 116)
		return false;

	W3DMeshHeader header;
	std::uint32_t sort_level_bits = 0;
	if (!W3DRead_U32(bytes, 0, header.version) ||
		!W3DRead_U32(bytes, 4, header.attributes) ||
		!W3DRead_U32(bytes, 40, header.triangle_count) ||
		!W3DRead_U32(bytes, 44, header.vertex_count) ||
		!W3DRead_U32(bytes, 48, header.material_count) ||
		!W3DRead_U32(bytes, 56, sort_level_bits) ||
		!W3DRead_U32(bytes, 68, header.vertex_channels) ||
		!W3DRead_U32(bytes, 72, header.face_channels) ||
		!W3DRead_Vector3(bytes, 76, header.bounds.minimum) ||
		!W3DRead_Vector3(bytes, 88, header.bounds.maximum) ||
		!W3DRead_Vector3(bytes, 100, header.sphere_center) ||
		!W3DRead_F32(bytes, 112, header.sphere_radius))
		return false;

	header.sort_level = std::bit_cast<std::int32_t>(sort_level_bits);
	if (!std::isfinite(header.sphere_center.x) || !std::isfinite(header.sphere_center.y) || !std::isfinite(header.sphere_center.z) ||
		!std::isfinite(header.sphere_radius) || header.sphere_radius < 0.0f)
		return false;

	header.name = W3DRead_Fixed_String(bytes, 8, 16);
	header.container_name = W3DRead_Fixed_String(bytes, 24, 16);
	if(!header.bounds.Is_Valid())return false;
	result=std::move(header);
	return true;
}

export struct W3DTriangleRecord {
    std::array<std::uint32_t,3> indices{};
    std::uint32_t surface_type=0;
    Vector3f normal{};
    float distance=0;
};

export bool W3DRead_Geometry_Vectors(W3DByteSpan bytes,std::size_t count,std::vector<Vector3f>& result)
{
    if(count>bytes.size()/12 || bytes.size()!=count*12)return false;
    std::vector<Vector3f> values(count);
    for(std::size_t i=0;i<count;++i) {
        auto& value=values[i];
        if(!W3DRead_Vector3(bytes,i*12,value) || !std::isfinite(value.x)
            || !std::isfinite(value.y) || !std::isfinite(value.z))return false;
    }
    result=std::move(values);return true;
}

export bool W3DRead_Geometry_Triangles(W3DByteSpan bytes,std::size_t count,std::vector<W3DTriangleRecord>& result)
{
    if(count>bytes.size()/32 || bytes.size()!=count*32)return false;
    std::vector<W3DTriangleRecord> values(count);
    for(std::size_t i=0;i<count;++i) {
        auto& value=values[i];const auto offset=i*32;
        for(std::size_t corner=0;corner<3;++corner)
            if(!W3DRead_U32(bytes,offset+corner*4,value.indices[corner]))return false;
        if(!W3DRead_U32(bytes,offset+12,value.surface_type)
            || !W3DRead_Vector3(bytes,offset+16,value.normal)
            || !W3DRead_F32(bytes,offset+28,value.distance))return false;
    }
    result=std::move(values);return true;
}

export bool W3DRead_Geometry_Bone_Links(W3DByteSpan bytes,std::size_t count,std::vector<std::uint16_t>& result)
{
    if(count>bytes.size()/8 || bytes.size()!=count*8)return false;
    std::vector<std::uint16_t> values(count);
    for(std::size_t i=0;i<count;++i)
        values[i]=static_cast<std::uint16_t>(std::to_integer<unsigned>(bytes[i*8])
            | (std::to_integer<unsigned>(bytes[i*8+1])<<8));
    // The remaining six bytes are authored padding, not extra influences.
    result=std::move(values);return true;
}

export bool W3DRead_Geometry_Shade_Indices(W3DByteSpan bytes,std::size_t count,std::vector<std::uint32_t>& result)
{
    if(count>bytes.size()/4 || bytes.size()!=count*4)return false;
    std::vector<std::uint32_t> values(count);
    for(std::size_t i=0;i<count;++i)if(!W3DRead_U32(bytes,i*4,values[i]))return false;
    result=std::move(values);return true;
}
}
