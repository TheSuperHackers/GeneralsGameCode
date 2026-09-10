module;

#include <cstdint>

export module Graphics.FrameOwner;

export import Graphics.FrameTargets;

namespace Graphics
{

export using FrameRendererInitializer = bool (*)(Device &);
export using FrameDrawExecutor = bool (*)(Device &, CommandList &, const FrameTargets &) noexcept;

export enum class FrameOwnerPhase : std::uint8_t
{
	Idle,
	Drawing,
	Submitted,
	Failed,
	ReadyToPresent
};

export class FrameOwner final
{
public:
	bool Set_Draw_Executor(FrameDrawExecutor executor) noexcept
	{
		if (m_phase != FrameOwnerPhase::Idle)
			return Reject();

		m_draw_executor = executor;
		return true;
	}

	bool Begin_Frame(Device &device) noexcept
	{
		if (m_phase != FrameOwnerPhase::Idle || !device.Is_Valid())
			return Reject();

		SwapChain &swap_chain = device.Get_Swap_Chain();
		if (!swap_chain.Is_Valid())
			return Reject();

		const RHIBackbuffer backbuffer = swap_chain.Backbuffer();
		const RHIDepthTarget depth = swap_chain.Depth_Target();
		if (!backbuffer.texture.Is_Valid() || !depth.texture.Is_Valid())
			return Reject();

		if (!device.Begin_Frame())
			return Reject();

		m_device = &device;
		m_targets = {backbuffer, depth};
		m_phase = FrameOwnerPhase::Drawing;
		return true;
	}

	bool Execute_Queued_Draws(Device &device) noexcept
	{
		if (m_phase != FrameOwnerPhase::Drawing || m_device != &device)
			return Reject();

		CommandList &command_list = device.Immediate_Command_List();
		if (!command_list.Reset_State() || !command_list.Set_Render_Targets(m_targets.backbuffer.texture, m_targets.depth.texture))
			return Fail_Draws();

		if (m_draw_executor != nullptr && !m_draw_executor(device, command_list, m_targets))
			return Fail_Draws();

		m_phase = FrameOwnerPhase::Submitted;
		return true;
	}

	bool End_Frame(Device &device) noexcept
	{
		if ((m_phase != FrameOwnerPhase::Drawing && m_phase != FrameOwnerPhase::Submitted) || m_device != &device || !device.End_Frame())
			return Reject();

		m_phase = FrameOwnerPhase::ReadyToPresent;
		return true;
	}

	bool Present(Device &device) noexcept
	{
		if (m_phase != FrameOwnerPhase::ReadyToPresent || m_device != &device || !device.Get_Swap_Chain().Present())
			return Reject();

		m_phase = FrameOwnerPhase::Idle;
		m_device = nullptr;
		m_targets = {};
		return true;
	}

	void Abort(Device &device) noexcept
	{
		if (m_device != nullptr && m_device != &device)
		{
			Reject();
			return;
		}

		if (m_phase == FrameOwnerPhase::Drawing || m_phase == FrameOwnerPhase::Submitted
			|| m_phase == FrameOwnerPhase::Failed)
			device.End_Frame();

		m_phase = FrameOwnerPhase::Idle;
		m_device = nullptr;
		m_targets = {};
	}

	FrameOwnerPhase Phase() const noexcept
	{
		return m_phase;
	}

	FrameTargets Targets() const noexcept
	{
		return m_targets;
	}

	std::uint32_t Invalid_Operation_Count() const noexcept
	{
		return m_invalid_operation_count;
	}

private:
	bool Fail_Draws() noexcept
	{
		m_phase = FrameOwnerPhase::Failed;
		return Reject();
	}

	bool Reject() noexcept
	{
		++m_invalid_operation_count;
		return false;
	}

	FrameOwnerPhase m_phase = FrameOwnerPhase::Idle;
	Device *m_device = nullptr;
	FrameTargets m_targets{};
	FrameDrawExecutor m_draw_executor = nullptr;
	std::uint32_t m_invalid_operation_count = 0;
};


}
