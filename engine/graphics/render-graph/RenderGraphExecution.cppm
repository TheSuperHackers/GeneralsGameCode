module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

export module Graphics.RenderGraph.Execution;

export import Graphics.RenderGraph;
export import Graphics.RHI;

namespace Graphics
{

export struct GraphResourceBinding final
{
	GraphResourceHandle resource{};
	GraphResourceKind kind = GraphResourceKind::Buffer;
	RHIBufferHandle buffer{};
	RHITextureHandle texture{};

	static constexpr GraphResourceBinding Buffer(GraphResourceHandle resource, RHIBufferHandle buffer) noexcept
	{
		return {resource, GraphResourceKind::Buffer, buffer, {}};
	}

	static constexpr GraphResourceBinding Texture(GraphResourceHandle resource, RHITextureHandle texture) noexcept
	{
		return {resource, GraphResourceKind::Texture, {}, texture};
	}
};

export class PassResources final
{
public:
	std::span<const GraphResourceUse> Declarations() const noexcept
	{
		return m_declarations;
	}

	bool Is_Declared(GraphResourceHandle resource) const noexcept
	{
		for (const GraphResourceUse declaration : m_declarations) {
			if (declaration.resource == resource)
				return true;
		}
		return false;
	}

	RHIBufferHandle Buffer(GraphResourceHandle resource) const noexcept
	{
		if (!Is_Declared(resource))
			return {};

		for (const GraphResourceBinding binding : m_bindings) {
			if (binding.resource == resource && binding.kind == GraphResourceKind::Buffer)
				return binding.buffer;
		}
		return {};
	}

	RHITextureHandle Texture(GraphResourceHandle resource) const noexcept
	{
		if (!Is_Declared(resource))
			return {};

		for (const GraphResourceBinding binding : m_bindings) {
			if (binding.resource == resource && binding.kind == GraphResourceKind::Texture)
				return binding.texture;
		}
		return {};
	}

private:
	friend class ExecutionPlan;

	PassResources(std::span<const GraphResourceUse> declarations, std::span<const GraphResourceBinding> bindings) noexcept
		: m_declarations(declarations),
		  m_bindings(bindings)
	{
	}

	std::span<const GraphResourceUse> m_declarations;
	std::span<const GraphResourceBinding> m_bindings;
};

export using PassExecuteFunction = bool (*)(CommandList &, const PassResources &) noexcept;

export struct PassExecution final
{
	GraphPassHandle pass{};
	PassExecuteFunction execute = nullptr;
};

export class ExecutionPlan final
{
public:
	bool Compile(RenderGraph &graph, std::span<const GraphResourceBinding> resources, std::span<const PassExecution> executions)
	{
		if (!Prepare(graph, resources) || !Validate_Executions(graph, m_order, executions)) {
			Invalidate();
			return false;
		}

		m_executions = executions;
		return true;
	}

	bool Compile(RenderGraph &graph, std::span<const GraphResourceBinding> resources)
	{
		if (!Prepare(graph, resources)) {
			Invalidate();
			return false;
		}

		m_executions = {};
		return true;
	}

	bool Execute(const RenderGraph &graph, CommandList &command_list) const noexcept
	{
		if (!Is_Current_Order(graph))
			return false;

		for (const GraphPassHandle pass : m_order) {
			if (!graph.Is_Pass_Valid(pass))
				return false;

			const PassExecution *execution = Find_Execution(pass);
			if (execution == nullptr)
				return false;

			const PassResources resources(graph.Pass_Resources(pass), m_resources);
			if (!execution->execute(command_list, resources))
				return false;
		}

		return true;
	}

	template <typename ExecuteFunction>
	bool Execute(const RenderGraph &graph, CommandList &command_list, ExecuteFunction &&execute) const noexcept
	{
		if (!Is_Current_Order(graph))
			return false;

		for (const GraphPassHandle pass : m_order) {
			if (!graph.Is_Pass_Valid(pass))
				return false;

			const PassResources resources(graph.Pass_Resources(pass), m_resources);
			if (!std::forward<ExecuteFunction>(execute)(pass, command_list, resources))
				return false;
		}

		return true;
	}

	bool Is_Valid() const noexcept
	{
		return m_valid;
	}

	std::span<const GraphPassHandle> Passes() const noexcept
	{
		return m_order;
	}

private:
	bool Prepare(RenderGraph &graph, std::span<const GraphResourceBinding> resources)
	{
		Invalidate();
		if (!graph.Compile())
			return false;

		const std::span<const GraphPassHandle> order = graph.Execution_Order();
		if (order.size() != graph.Pass_Count() || !Validate_Resources(graph, order, resources))
			return false;

		m_order = order;
		m_resources = resources;
		m_valid = true;
		return true;
	}

	void Invalidate() noexcept
	{
		m_valid = false;
		m_order = {};
		m_resources = {};
		m_executions = {};
	}

	bool Is_Current_Order(const RenderGraph &graph) const noexcept
	{
		if (!m_valid || !graph.Is_Compiled())
			return false;

		const std::span<const GraphPassHandle> current_order = graph.Execution_Order();
		if (current_order.size() != m_order.size())
			return false;

		for (std::size_t index = 0; index < m_order.size(); ++index) {
			if (current_order[index] != m_order[index])
				return false;
		}

		return true;
	}

	static bool Has_Binding(std::span<const GraphResourceBinding> resources, GraphResourceHandle resource) noexcept
	{
		for (const GraphResourceBinding binding : resources) {
			if (binding.resource == resource)
				return true;
		}
		return false;
	}

	static bool Validate_Resources(const RenderGraph &graph, std::span<const GraphPassHandle> order, std::span<const GraphResourceBinding> resources) noexcept
	{
		for (std::size_t index = 0; index < resources.size(); ++index) {
			const GraphResourceBinding binding = resources[index];
			if (!graph.Is_Resource_Valid(binding.resource) || graph.Resource_Kind(binding.resource) != binding.kind)
				return false;

			if (binding.kind == GraphResourceKind::Buffer) {
				if (!binding.buffer.Is_Valid() || binding.texture.Is_Valid())
					return false;
			} else {
				if (!binding.texture.Is_Valid() || binding.buffer.Is_Valid())
					return false;
			}

			for (std::size_t previous = 0; previous < index; ++previous) {
				if (resources[previous].resource == binding.resource)
					return false;
			}
		}

		for (const GraphPassHandle pass : order) {
			if (!graph.Is_Pass_Valid(pass))
				return false;

			for (const GraphResourceUse declaration : graph.Pass_Resources(pass)) {
				if (!Has_Binding(resources, declaration.resource))
					return false;
			}
		}

		return true;
	}

	static bool Validate_Executions(const RenderGraph &graph, std::span<const GraphPassHandle> order, std::span<const PassExecution> executions) noexcept
	{
		if (order.size() != executions.size())
			return false;

		for (std::size_t index = 0; index < executions.size(); ++index) {
			const PassExecution execution = executions[index];
			if (!graph.Is_Pass_Valid(execution.pass) || execution.execute == nullptr)
				return false;

			for (std::size_t previous = 0; previous < index; ++previous) {
				if (executions[previous].pass == execution.pass)
					return false;
			}
		}

		for (const GraphPassHandle pass : order) {
			bool found = false;
			for (const PassExecution execution : executions) {
				if (execution.pass != pass)
					continue;
				if (found)
					return false;
				found = true;
			}
			if (!found)
				return false;
		}

		return true;
	}

	const PassExecution *Find_Execution(GraphPassHandle pass) const noexcept
	{
		for (const PassExecution &execution : m_executions) {
			if (execution.pass == pass)
				return &execution;
		}
		return nullptr;
	}

	std::span<const GraphPassHandle> m_order;
	std::span<const GraphResourceBinding> m_resources;
	std::span<const PassExecution> m_executions;
	bool m_valid = false;
};

}
