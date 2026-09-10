module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <utility>
#include <vector>
export module Assets.MeshBoundsTree;
import Assets.Math;

namespace Assets {
export struct MeshBoundsNode final
{
	Bounds3f bounds;
	std::uint32_t first = 0; // Front child or first polygon index.
	std::uint32_t second = 0; // Back child or polygon count.
	bool leaf = true;
};

export struct MeshBoundsTree final
{
	std::vector<MeshBoundsNode> nodes;
	std::vector<std::uint32_t> polygon_indices;
};

namespace MeshBoundsDetail {
inline Bounds3f Empty_Bounds(float limit)
{
	return {{limit,limit,limit},{-limit,-limit,-limit}};
}

inline void Include(Bounds3f& bounds, Vector3f point)
{
	if (point.x < bounds.minimum.x) bounds.minimum.x = point.x;
	if (point.y < bounds.minimum.y) bounds.minimum.y = point.y;
	if (point.z < bounds.minimum.z) bounds.minimum.z = point.z;
	if (point.x > bounds.maximum.x) bounds.maximum.x = point.x;
	if (point.y > bounds.maximum.y) bounds.maximum.y = point.y;
	if (point.z > bounds.maximum.z) bounds.maximum.z = point.z;
}

inline float Component(Vector3f value, unsigned axis)
{
	return axis == 0 ? value.x : axis == 1 ? value.y : value.z;
}
}

// The caller supplies samples so building does not introduce or reseed a global
// RNG. Samples are consumed in front-first depth-first order, including rejected
// split candidates. Source arrays are borrowed only for this call.
export template<class Sample>
bool Build_Mesh_Bounds_Tree(std::span<const Vector3f> vertices,
	std::span<const std::array<std::uint32_t,3>> triangles, Sample&& sample,
	MeshBoundsTree& result)
{
	if (triangles.empty() || vertices.empty() ||
		triangles.size() > (std::numeric_limits<std::int32_t>::max)() / 2)
		return false;
	for (const auto& triangle : triangles) {
		for (const auto index : triangle) {
			if (index >= vertices.size()) return false;
			const auto point = vertices[index];
			if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) return false;
		}
	}

	using namespace MeshBoundsDetail;
	const auto include_polygon = [&](Bounds3f& bounds, std::uint32_t polygon) {
		for (const auto index : triangles[polygon]) Include(bounds, vertices[index]);
	};
	const auto is_back = [&](std::uint32_t polygon, unsigned axis, float distance) {
		bool negative = false;
		for (const auto index : triangles[polygon]) {
			const float delta = Component(vertices[index], axis) - distance;
			if (delta > 0.001f) return false;
			if (delta < -0.001f) negative = true;
		}
		// Coplanar and spanning polygons go to the front, preserving input order.
		return negative;
	};
	struct Pending {
		std::vector<std::uint32_t> polygons;
		std::uint32_t parent = 0;
		bool back = false;
	};
	Pending root;
	root.polygons.resize(triangles.size());
	std::iota(root.polygons.begin(), root.polygons.end(), 0u);
	std::vector<Pending> pending;
	pending.push_back(std::move(root));
	MeshBoundsTree tree;
	tree.polygon_indices.reserve(triangles.size());
	while (!pending.empty()) {
		auto task = std::move(pending.back());
		pending.pop_back();
		const auto node_index = static_cast<std::uint32_t>(tree.nodes.size());
		tree.nodes.emplace_back();
		if (node_index != 0) {
			auto& parent = tree.nodes[task.parent];
			(task.back ? parent.second : parent.first) = node_index;
		}
		float best_cost = (std::numeric_limits<float>::max)();
		unsigned best_axis = 0;
		float best_distance = 0;
		bool split = false;
		const auto count = static_cast<std::uint32_t>(task.polygons.size());
		if (count > 4) {
			for (unsigned attempt = 0; attempt < std::min(50u, count); ++attempt) {
				const auto polygon = task.polygons[sample() % count];
				const auto corner = sample() % 3;
				const auto axis = static_cast<unsigned>(sample() % 3);
				const float distance = Component(vertices[triangles[polygon][corner]], axis);
				auto front_bounds = Empty_Bounds(100000.0f);
				auto back_bounds = Empty_Bounds(100000.0f);
				std::uint32_t back_count = 0;
				for (const auto index : task.polygons) {
					if (is_back(index, axis, distance)) {
						++back_count;
						include_polygon(back_bounds, index);
					} else include_polygon(front_bounds, index);
				}
				const auto front_count = count - back_count;
				back_bounds.minimum.x -= 0.0001f;
				back_bounds.minimum.y -= 0.0001f;
				back_bounds.minimum.z -= 0.0001f;
				back_bounds.maximum.x += 0.0001f;
				back_bounds.maximum.y += 0.0001f;
				back_bounds.maximum.z += 0.0001f;
				const auto cost = [](Bounds3f bounds, std::uint32_t size) {
					return (bounds.maximum.x - bounds.minimum.x) *
						(bounds.maximum.y - bounds.minimum.y) *
						(bounds.maximum.z - bounds.minimum.z) * size;
				};
				const float back_cost = cost(back_bounds, back_count);
				const float front_cost = cost(front_bounds, front_count);
				const float candidate = front_cost + back_cost;
				if (front_count && back_count && candidate < best_cost) {
					best_cost = candidate;
					best_axis = axis;
					best_distance = distance;
					split = true;
				}
			}
		}
		if (split) {
			Pending front{{},node_index,false}, back{{},node_index,true};
			for (const auto index : task.polygons)
				(is_back(index,best_axis,best_distance) ? back.polygons : front.polygons).push_back(index);
			tree.nodes[node_index].leaf = false;
			pending.push_back(std::move(back));
			pending.push_back(std::move(front));
		} else {
			auto& node = tree.nodes[node_index];
			node.first = static_cast<std::uint32_t>(tree.polygon_indices.size());
			node.second = count;
			node.bounds = Empty_Bounds((std::numeric_limits<float>::max)());
			for (const auto index : task.polygons) {
				include_polygon(node.bounds, index);
				tree.polygon_indices.push_back(index);
			}
		}
	}
	for (auto index = tree.nodes.size(); index-- > 0;) {
		auto& node = tree.nodes[index];
		if (node.leaf) continue;
		node.bounds = Empty_Bounds((std::numeric_limits<float>::max)());
		for (const auto child : {node.first,node.second}) {
			const auto& bounds = tree.nodes[child].bounds;
			if (bounds.minimum.x < node.bounds.minimum.x) node.bounds.minimum.x = bounds.minimum.x;
			if (bounds.minimum.y < node.bounds.minimum.y) node.bounds.minimum.y = bounds.minimum.y;
			if (bounds.minimum.z < node.bounds.minimum.z) node.bounds.minimum.z = bounds.minimum.z;
			if (bounds.maximum.x > node.bounds.maximum.x) node.bounds.maximum.x = bounds.maximum.x;
			if (bounds.maximum.y > node.bounds.maximum.y) node.bounds.maximum.y = bounds.maximum.y;
			if (bounds.maximum.z > node.bounds.maximum.z) node.bounds.maximum.z = bounds.maximum.z;
		}
	}
	result = std::move(tree);
	return true;
}
}
