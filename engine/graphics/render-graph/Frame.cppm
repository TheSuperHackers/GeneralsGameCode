module;

#include <cstdint>
#include <cstddef>
#include <span>

export module Graphics.RenderGraph.Frame;

export import Graphics.RenderGraph.Execution;
export import Graphics.RHI;

namespace Graphics
{

export class Frame final
{
public:
	bool Begin(RenderGraph &graph, Device &device, GraphResourceHandle color_target, GraphResourceHandle depth_target, std::span<GraphResourceBinding> bindings) noexcept
	{
		if (m_active || bindings.size() < 2 || color_target == depth_target || !graph.Is_Resource_Valid(color_target) || !graph.Is_Resource_Valid(depth_target) || graph.Resource_Kind(color_target) != GraphResourceKind::Texture || graph.Resource_Kind(depth_target) != GraphResourceKind::Texture)
			return false;

		const RHIBackbuffer backbuffer = device.Get_Swap_Chain().Backbuffer();
		const RHIDepthTarget depth = device.Get_Swap_Chain().Depth_Target();
		if (!device.Get_Swap_Chain().Is_Valid() || !backbuffer.texture.Is_Valid() || !depth.texture.Is_Valid())
			return false;

		bindings[0] = GraphResourceBinding::Texture(color_target, backbuffer.texture);
		bindings[1] = GraphResourceBinding::Texture(depth_target, depth.texture);
		if (!device.Begin_Frame())
			return false;

		m_active = true;
		m_ready_to_present = false;
		return true;
	}

	bool End(Device &device) noexcept
	{
		if (!m_active || !device.End_Frame())
			return false;

		m_active = false;
		m_ready_to_present = true;
		return true;
	}

	bool Present(Device &device) noexcept
	{
		if (m_active || !m_ready_to_present)
			return false;

		if (!device.Get_Swap_Chain().Present())
			return false;

		m_ready_to_present = false;
		return true;
	}

	bool Is_Active() const noexcept
	{
		return m_active;
	}

private:
	bool m_active = false;
	bool m_ready_to_present = false;
};

}
