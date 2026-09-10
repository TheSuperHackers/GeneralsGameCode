module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

export module Graphics.Shaders.Pipeline;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.RHI;
export import Graphics.Shaders.ParameterLayout;

namespace Graphics
{

export struct PipelineDesc;

export struct PipelineKey final
{
	std::uint64_t value = 0;

	static constexpr PipelineKey From(const PipelineDesc &description) noexcept;

	friend constexpr bool operator==(PipelineKey left, PipelineKey right) noexcept
	{
		return left.value == right.value;
	}

	friend constexpr bool operator!=(PipelineKey left, PipelineKey right) noexcept
	{
		return !(left == right);
	}
};

export struct PipelineDesc final
{
	std::uint16_t vertex_shader = 0;
	std::uint16_t fragment_shader = 0;
	RHIPrimitiveTopology topology = RHIPrimitiveTopology::TriangleList;
	RHIVertexFormat vertex_format = RHIVertexFormat::Position3Color4UV2;
	bool depth_test = true;
	bool depth_write = true;
	RHIBlendMode blend_mode = RHIBlendMode::Disabled;
	RHICullMode cull_mode = RHICullMode::Back;
	RHIBlendOperation blend_operation = RHIBlendOperation::Add;
	ShaderLayoutKey parameter_layout_key = 0;

	void Set_Parameter_Layout(const ShaderInterfaceLayout &layout) noexcept
	{
		parameter_layout_key = layout.Key();
	}

	constexpr PipelineKey Key() const noexcept
	{
		return PipelineKey::From(*this);
	}
};

constexpr PipelineKey PipelineKey::From(const PipelineDesc &description) noexcept
{
	std::uint64_t key = 1469598103934665603ull;
	key ^= description.vertex_shader;
	key *= 1099511628211ull;
	key ^= description.fragment_shader;
	key *= 1099511628211ull;
	key ^= static_cast<std::uint8_t>(description.topology);
	key *= 1099511628211ull;
	key ^= static_cast<std::uint8_t>(description.vertex_format);
	key *= 1099511628211ull;
	key ^= description.depth_test ? 1u : 0u;
	key *= 1099511628211ull;
	key ^= description.depth_write ? 1u : 0u;
	key *= 1099511628211ull;
	key ^= static_cast<std::uint8_t>(description.blend_mode);
	key *= 1099511628211ull;
	key ^= static_cast<std::uint8_t>(description.cull_mode);
	key *= 1099511628211ull;
	key ^= static_cast<std::uint8_t>(description.blend_operation);
	key *= 1099511628211ull;
	key ^= description.parameter_layout_key;
	key *= 1099511628211ull;
	return {key};
}

export class PipelineCache final
{
public:
	bool Initialize(Device &device, const PipelineDesc &fallback, std::span<const PipelineDesc> known = {})
	{
		if (m_fallback.Is_Valid() || !m_entries.empty())
			return false;

		m_entries.reserve(known.size() + 1);
		m_fallback = Create(device, fallback);
		if (!m_fallback.Is_Valid())
			return false;

		m_entries.push_back({fallback.Key(), m_fallback});
		return Precreate(device, known);
	}

	bool Precreate(Device &device, std::span<const PipelineDesc> descriptions)
	{
		if (!m_fallback.Is_Valid())
			return false;

		m_entries.reserve(m_entries.size() + descriptions.size());
		bool complete = true;
		for (const PipelineDesc &description : descriptions) {
			const PipelineKey key = description.Key();
			if (Find(key).Is_Valid())
				continue;

			const PipelineHandle handle = Create(device, description);
			if (!handle.Is_Valid()) {
				complete = false;
				continue;
			}

			m_entries.push_back({key, handle});
		}

		return complete;
	}

	PipelineHandle Resolve(PipelineKey key) const noexcept
	{
		const PipelineHandle handle = Find(key);
		return handle.Is_Valid() ? handle : m_fallback;
	}

	PipelineHandle Resolve(const PipelineDesc &description) const noexcept
	{
		return Resolve(description.Key());
	}

	PipelineHandle Fallback() const noexcept
	{
		return m_fallback;
	}

	std::size_t Size() const noexcept
	{
		return m_entries.size();
	}

	bool Shutdown(Device &device) noexcept
	{
		bool complete = true;
		for (const Entry &entry : m_entries)
			complete = device.Destroy_Pipeline(entry.handle) && complete;

		m_entries.clear();
		m_fallback = {};
		return complete;
	}

private:
	struct Entry final
	{
		PipelineKey key{};
		PipelineHandle handle{};
	};

	static PipelineHandle Create(Device &device, const PipelineDesc &description)
	{
		const PipelineKey key = description.Key();
		const RHIPipeline rhi_description{
			key.value,
			description.depth_test,
			description.depth_write,
				description.topology,
				description.vertex_format,
				description.blend_mode,
				description.cull_mode,
				description.blend_operation
		};
		return device.Create_Pipeline(rhi_description);
	}

	PipelineHandle Find(PipelineKey key) const noexcept
	{
		for (const Entry &entry : m_entries) {
			if (entry.key == key)
				return entry.handle;
		}

		return {};
	}

	std::vector<Entry> m_entries;
	PipelineHandle m_fallback{};
};

}
