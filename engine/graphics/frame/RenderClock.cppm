module;

#include <cstdint>

export module Graphics.Frame.RenderClock;

namespace Graphics
{

// The render clock keeps the simulation-facing logic step separate from the
// integral synchronized time used by authored animation. Fractional time is
// retained until a caller elects to advance a logic frame.
export class RenderClock final
{
public:
	static constexpr float Default_Logic_Frame_Time_Milliseconds() noexcept
	{
		return 1000.0f / 30.0f;
	}

	RenderClock() noexcept
		: m_logic_frame_time_ms(Default_Logic_Frame_Time_Milliseconds())
	{
	}

	void Update_Logic_Frame_Time(float milliseconds) noexcept
	{
		m_logic_frame_time_ms = milliseconds;
		m_fractional_sync_ms += milliseconds;
	}

	void Sync(bool step) noexcept
	{
		// Previous time is updated on every sync call, including a paused
		// call. This makes the reported delta zero when no logic frame steps.
		m_previous_sync_time = m_sync_time;

		if (!step)
			return;

		const std::uint32_t integral_sync_ms =
			static_cast<std::uint32_t>(m_fractional_sync_ms);
		m_fractional_sync_ms -= static_cast<float>(integral_sync_ms);
		m_sync_time += integral_sync_ms;
	}

	std::uint32_t Sync_Time() const noexcept
	{
		return m_sync_time;
	}

	std::uint32_t Sync_Delta() const noexcept
	{
		return m_sync_time - m_previous_sync_time;
	}

	std::uint32_t Fractional_Sync_Milliseconds() const noexcept
	{
		return static_cast<std::uint32_t>(m_fractional_sync_ms);
	}

	std::uint32_t Logic_Time_Milliseconds() const noexcept
	{
		// Keep the conversion order used by the authored runtime. Converting
		// the complete sum to float first changes results near uint32 limits.
		return m_sync_time + static_cast<std::uint32_t>(m_fractional_sync_ms);
	}

	float Logic_Frame_Time_Milliseconds() const noexcept
	{
		return m_logic_frame_time_ms;
	}

	float Logic_Frame_Time_Seconds() const noexcept
	{
		return m_logic_frame_time_ms * 0.001f;
	}

private:
	float m_logic_frame_time_ms;
	float m_fractional_sync_ms = 0.0f;
	std::uint32_t m_sync_time = 0;
	std::uint32_t m_previous_sync_time = 0;
};

export RenderClock &Get_Render_Clock() noexcept
{
	static RenderClock clock;
	return clock;
}

}
