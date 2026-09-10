module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

export module Graphics.RenderGraph;

import Graphics.Resources.Handles.ResourceHandle;
import Graphics.Resources.Pools.ResourcePool;

namespace Graphics
{

export struct GraphResourceHandleTag
{
};

export using GraphResourceHandle = ResourceHandle<GraphResourceHandleTag>;

export struct GraphPassHandleTag
{
};

export using GraphPassHandle = ResourceHandle<GraphPassHandleTag>;

export enum class GraphResourceKind : std::uint8_t
{
	Buffer,
	Texture
};

export struct GraphResource final
{
	GraphResourceKind kind = GraphResourceKind::Buffer;
};

export enum class GraphResourceAccess : std::uint8_t
{
	Read,
	Write
};

export struct GraphResourceUse final
{
	GraphResourceHandle resource{};
	GraphResourceAccess access = GraphResourceAccess::Read;

	static constexpr GraphResourceUse Read(GraphResourceHandle resource) noexcept
	{
		return {resource, GraphResourceAccess::Read};
	}

	static constexpr GraphResourceUse Write(GraphResourceHandle resource) noexcept
	{
		return {resource, GraphResourceAccess::Write};
	}
};

export struct RenderPass final
{
	std::uint32_t pass_key = 0;
};

export class RenderGraph final
{
public:
	void Reserve(std::size_t resource_capacity, std::size_t pass_capacity, std::size_t access_capacity, std::size_t dependency_capacity = 0)
	{
		m_resources.Reserve(resource_capacity);
		m_passes.Reserve(pass_capacity);
		m_accesses.reserve(access_capacity);
		m_dependencies.reserve(dependency_capacity);
		m_pass_handles.reserve(pass_capacity);
		m_execution_order.reserve(pass_capacity);
		m_ready.reserve(pass_capacity);
		m_pass_slot_to_node.reserve(pass_capacity);
		m_indegree.reserve(pass_capacity);
		const std::size_t hazard_capacity = pass_capacity < 2
			? 0
			: pass_capacity > std::numeric_limits<std::size_t>::max() / (pass_capacity - 1)
				? std::numeric_limits<std::size_t>::max()
				: pass_capacity * (pass_capacity - 1) / 2;
		const std::size_t edge_capacity = dependency_capacity > std::numeric_limits<std::size_t>::max() - hazard_capacity
			? hazard_capacity
			: hazard_capacity + dependency_capacity;
		m_edges.reserve(edge_capacity);
	}

	GraphResourceHandle Create_Resource(GraphResource resource)
	{
		m_compiled = false;
		return m_resources.Create(std::move(resource));
	}

	GraphPassHandle Add_Pass(RenderPass pass, std::span<const GraphResourceUse> uses)
	{
		if (m_accesses.size() > std::numeric_limits<std::uint32_t>::max()
			|| uses.size() > std::numeric_limits<std::uint32_t>::max() - m_accesses.size())
			return {};

		for (const GraphResourceUse use : uses) {
			if (m_resources.Resolve(use.resource) == nullptr)
				return {};
		}

		m_accesses.reserve(m_accesses.size() + uses.size());
		m_passes.Reserve(m_passes.Size() + 1);

		const std::uint32_t access_offset = static_cast<std::uint32_t>(m_accesses.size());
		m_accesses.insert(m_accesses.end(), uses.begin(), uses.end());
		m_compiled = false;
		return m_passes.Create(PassDeclaration{pass, access_offset, static_cast<std::uint32_t>(uses.size())});
	}

	bool Is_Resource_Valid(GraphResourceHandle resource) const noexcept
	{
		return m_resources.Resolve(resource) != nullptr;
	}

	GraphResourceKind Resource_Kind(GraphResourceHandle resource) const noexcept
	{
		const GraphResource *description = m_resources.Resolve(resource);
		return description == nullptr ? GraphResourceKind::Buffer : description->kind;
	}

	bool Is_Pass_Valid(GraphPassHandle pass) const noexcept
	{
		return m_passes.Resolve(pass) != nullptr;
	}

	std::span<const GraphResourceUse> Pass_Resources(GraphPassHandle pass) const noexcept
	{
		const PassDeclaration *declaration = m_passes.Resolve(pass);
		if (declaration == nullptr || declaration->access_count == 0)
			return {};

		return {m_accesses.data() + declaration->access_offset, declaration->access_count};
	}

	bool Add_Dependency(GraphPassHandle dependent, GraphPassHandle prerequisite)
	{
		if (m_passes.Resolve(dependent) == nullptr || m_passes.Resolve(prerequisite) == nullptr)
			return false;

		m_dependencies.reserve(m_dependencies.size() + 1);
		m_dependencies.push_back({dependent, prerequisite});
		m_compiled = false;
		return true;
	}

	bool Compile()
	{
		m_execution_order.clear();
		m_edges.clear();
		m_indegree.clear();
		m_ready.clear();
		m_pass_handles.clear();
		m_pass_slot_to_node.clear();

		m_pass_handles.reserve(m_passes.Size());
		m_passes.For_Each([&](GraphPassHandle handle, const PassDeclaration &) noexcept {
			m_pass_handles.push_back(handle);
		});

		const std::size_t pass_count = m_pass_handles.size();
		m_indegree.assign(pass_count, 0);
		m_execution_order.reserve(pass_count);
		m_ready.reserve(pass_count);

		std::uint32_t highest_slot = 0;
		for (const GraphPassHandle handle : m_pass_handles) {
			if (handle.Get_Index() > highest_slot)
				highest_slot = handle.Get_Index();
		}
		if (!m_pass_handles.empty())
			m_pass_slot_to_node.assign(static_cast<std::size_t>(highest_slot) + 1, Invalid_Node);

		for (std::size_t node = 0; node < pass_count; ++node)
			m_pass_slot_to_node[m_pass_handles[node].Get_Index()] = static_cast<std::uint32_t>(node);

		for (std::size_t left = 0; left < pass_count; ++left) {
			const PassDeclaration *left_pass = m_passes.Resolve(m_pass_handles[left]);
			for (std::size_t right = left + 1; right < pass_count; ++right) {
				const PassDeclaration *right_pass = m_passes.Resolve(m_pass_handles[right]);
				if (Has_Hazard(*left_pass, *right_pass))
					Add_Edge(static_cast<std::uint32_t>(left), static_cast<std::uint32_t>(right));
			}
		}

		for (const Dependency dependency : m_dependencies) {
			const std::uint32_t dependent = Node_Of(dependency.dependent);
			const std::uint32_t prerequisite = Node_Of(dependency.prerequisite);
			if (dependent == Invalid_Node || prerequisite == Invalid_Node)
				return Finish_Compile(false);

			Add_Edge(prerequisite, dependent);
		}

		for (std::uint32_t node = 0; node < pass_count; ++node) {
			if (m_indegree[node] == 0)
				m_ready.push_back(node);
		}

		while (!m_ready.empty()) {
			std::size_t ready_index = 0;
			for (std::size_t index = 1; index < m_ready.size(); ++index) {
				if (m_ready[index] < m_ready[ready_index])
					ready_index = index;
			}

			const std::uint32_t node = m_ready[ready_index];
			m_ready[ready_index] = m_ready.back();
			m_ready.pop_back();
			m_execution_order.push_back(m_pass_handles[node]);

			for (const Edge edge : m_edges) {
				if (edge.from != node)
					continue;

				--m_indegree[edge.to];
				if (m_indegree[edge.to] == 0)
					m_ready.push_back(edge.to);
			}
		}

		return Finish_Compile(m_execution_order.size() == pass_count);
	}

	bool Is_Compiled() const noexcept
	{
		return m_compiled;
	}

	std::span<const GraphPassHandle> Execution_Order() const noexcept
	{
		return m_execution_order;
	}

	std::size_t Resource_Count() const noexcept
	{
		return m_resources.Size();
	}

	std::size_t Pass_Count() const noexcept
	{
		return m_passes.Size();
	}

private:
	struct PassDeclaration final
	{
		RenderPass pass{};
		std::uint32_t access_offset = 0;
		std::uint32_t access_count = 0;
	};

	struct Dependency final
	{
		GraphPassHandle dependent{};
		GraphPassHandle prerequisite{};
	};

	struct Edge final
	{
		std::uint32_t from = 0;
		std::uint32_t to = 0;
	};

	static constexpr std::uint32_t Invalid_Node = std::numeric_limits<std::uint32_t>::max();

	static bool Has_Hazard(const PassDeclaration &left, const PassDeclaration &right, const std::vector<GraphResourceUse> &accesses) noexcept
	{
		const std::span<const GraphResourceUse> left_uses(accesses.data() + left.access_offset, left.access_count);
		const std::span<const GraphResourceUse> right_uses(accesses.data() + right.access_offset, right.access_count);
		for (const GraphResourceUse left_use : left_uses) {
			for (const GraphResourceUse right_use : right_uses) {
				if (left_use.resource != right_use.resource)
					continue;

				if (left_use.access == GraphResourceAccess::Write || right_use.access == GraphResourceAccess::Write)
					return true;
			}
		}
		return false;
	}

	bool Has_Hazard(const PassDeclaration &left, const PassDeclaration &right) const noexcept
	{
		if (left.access_count == 0 || right.access_count == 0)
			return false;

		return Has_Hazard(left, right, m_accesses);
	}

	void Add_Edge(std::uint32_t from, std::uint32_t to)
	{
		m_edges.push_back({from, to});
		++m_indegree[to];
	}

	std::uint32_t Node_Of(GraphPassHandle handle) const noexcept
	{
		if (!handle.Is_Valid() || handle.Get_Index() >= m_pass_slot_to_node.size())
			return Invalid_Node;

		const std::uint32_t node = m_pass_slot_to_node[handle.Get_Index()];
		if (node == Invalid_Node || node >= m_pass_handles.size() || m_pass_handles[node] != handle)
			return Invalid_Node;

		return node;
	}

	bool Finish_Compile(bool success) noexcept
	{
		m_compiled = success;
		if (!success)
			m_execution_order.clear();
		return success;
	}

	ResourcePool<GraphResource, GraphResourceHandle> m_resources;
	ResourcePool<PassDeclaration, GraphPassHandle> m_passes;
	std::vector<GraphResourceUse> m_accesses;
	std::vector<Dependency> m_dependencies;
	std::vector<GraphPassHandle> m_pass_handles;
	std::vector<std::uint32_t> m_pass_slot_to_node;
	std::vector<Edge> m_edges;
	std::vector<std::uint32_t> m_indegree;
	std::vector<std::uint32_t> m_ready;
	std::vector<GraphPassHandle> m_execution_order;
	bool m_compiled = false;
};

}
