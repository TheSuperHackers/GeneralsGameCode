module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <utility>

export module Video.Runtime;

export import Video.FFmpeg.Player;
export import Video.Presentation;

namespace Engine::Video
{

export enum class PlaybackSlot : std::uint8_t
{
	Fullscreen,
	Cameo
};

export using SessionId = PresentationId;

export class Runtime final
{
public:
	static constexpr std::size_t Max_Sessions = 32;

	~Runtime() noexcept
	{
		Close_All();
	}

	bool Open(PlaybackSlot slot, std::string_view source, PlaybackMode mode = PlaybackMode::Once)
	{
		return Open_Internal(Session_Id(slot), source, Slot_Target(slot), mode);
	}

	bool Open(PlaybackSlot slot, std::unique_ptr<Source> source, PlaybackMode mode = PlaybackMode::Once)
	{
		return Open_Internal(Session_Id(slot), std::move(source), Slot_Target(slot), mode);
	}

	bool Open(SessionId id, std::string_view source, PlaybackMode mode = PlaybackMode::Once)
	{
		return Open_Internal(id, source, {}, mode);
	}

	bool Open(SessionId id, std::unique_ptr<Source> source, PlaybackMode mode = PlaybackMode::Once)
	{
		return Open_Internal(id, std::move(source), {}, mode);
	}

	bool Open(
		SessionId id,
		std::unique_ptr<Source> source,
		PresentationTarget target,
		PlaybackMode mode = PlaybackMode::Once)
	{
		return Open_Internal(id, std::move(source), target, mode);
	}

	SessionId Open(
		std::unique_ptr<Source> source,
		PresentationTarget target = {},
		PlaybackMode mode = PlaybackMode::Once)
	{
		if (source == nullptr)
			return Invalid_Presentation;

		const SessionId id = Allocate_Session_Id();
		if (id == Invalid_Presentation)
			return Invalid_Presentation;
		return Open_Internal(id, std::move(source), target, mode) ? id : Invalid_Presentation;
	}

	void Set_Audio_Sink(AudioSink *sink) noexcept
	{
		m_audio_sink = sink;
		for (std::size_t index = 0; index < Max_Sessions; ++index) {
			if (m_used[index])
				m_players[index].Set_Audio_Sink(sink);
		}
	}

	bool Configure(PlaybackSlot slot, PresentationTarget target)
	{
		return Configure(Session_Id(slot), target);
	}

	bool Configure(SessionId id, PresentationTarget target)
	{
		const std::size_t index = Find(id);
		if (index == Max_Sessions)
			return false;
		m_targets[index] = target;
		return true;
	}

	bool Set_Visible(SessionId id, bool visible) noexcept
	{
		const std::size_t index = Find(id);
		if (index == Max_Sessions)
			return false;
		m_targets[index].visible = visible;
		return true;
	}

	void Close(PlaybackSlot slot) noexcept
	{
		Close(Session_Id(slot));
	}

	void Close(SessionId id) noexcept
	{
		const std::size_t index = Find(id);
		if (index == Max_Sessions)
			return;
		m_players[index].Close();
		m_sources[index].reset();
		Release(index);
	}

	void Close_All() noexcept
	{
		for (std::size_t index = 0; index < Max_Sessions; ++index) {
			m_players[index].Close();
			m_sources[index].reset();
			Release(index);
		}
	}

	bool Update(double delta_seconds)
	{
		bool delivered = false;
		for (std::size_t index = 0; index < Max_Sessions; ++index) {
			if (m_used[index])
				delivered = m_players[index].Update(delta_seconds) || delivered;
		}
		return delivered;
	}

	PlaybackState State(PlaybackSlot slot) const noexcept
	{
		return State(Session_Id(slot));
	}

	PlaybackState State(SessionId id) const noexcept
	{
		const std::size_t index = Find(id);
		return index == Max_Sessions ? PlaybackState::Closed : m_players[index].State();
	}

	StreamInfo Info(PlaybackSlot slot) const noexcept
	{
		return Info(Session_Id(slot));
	}

	StreamInfo Info(SessionId id) const noexcept
	{
		const std::size_t index = Find(id);
		return index == Max_Sessions ? StreamInfo{} : m_players[index].Info();
	}

	const DecodedVideoFrame *Current_Frame(PlaybackSlot slot) const noexcept
	{
		return Current_Frame(Session_Id(slot));
	}

	const DecodedVideoFrame *Current_Frame(SessionId id) const noexcept
	{
		const std::size_t index = Find(id);
		return index == Max_Sessions ? nullptr : m_players[index].Current_Frame();
	}

	bool Pause(SessionId id) noexcept
	{
		const std::size_t index = Find(id);
		if (index == Max_Sessions)
			return false;
		return m_players[index].Pause();
	}

	bool Play(SessionId id) noexcept
	{
		const std::size_t index = Find(id);
		if (index == Max_Sessions)
			return false;
		return m_players[index].Play();
	}

	bool Seek(SessionId id, std::uint64_t frame_index)
	{
		const std::size_t index = Find(id);
		if (index == Max_Sessions)
			return false;
		return m_players[index].Seek(frame_index);
	}

	bool Present(FramePresenter &presenter, PresentationExtent output)
	{
		if (output.width == 0 || output.height == 0
			|| output.width > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())
			|| output.height > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()))
			return false;

		bool submitted = false;
		for (std::size_t index = 0; index < Max_Sessions; ++index) {
			if (!m_used[index] || !m_targets[index].visible)
				continue;

			const DecodedVideoFrame *frame = m_players[index].Current_Frame();
			if (frame == nullptr || !frame->Is_Valid())
				continue;

			PresentationRect destination = m_targets[index].rect;
			if (m_targets[index].layout == PresentationLayout::Fit_Output) {
				if (!Fit_To_Output(*frame, output, destination))
					continue;
			} else if (m_targets[index].rect_provider != nullptr
				&& !m_targets[index].rect_provider(m_targets[index].context, destination)) {
				continue;
			}

			if (destination.width == 0 || destination.height == 0)
				continue;
			submitted = presenter.Submit(m_ids[index], *frame, destination) || submitted;
		}
		return submitted;
	}

	bool Is_Active(PlaybackSlot slot) const noexcept
	{
		return Is_Active(Session_Id(slot));
	}

	bool Is_Active(SessionId id) const noexcept
	{
		const PlaybackState state = State(id);
		return state == PlaybackState::Playing || state == PlaybackState::Paused;
	}

	static constexpr SessionId Session_Id(PlaybackSlot slot) noexcept
	{
		return static_cast<SessionId>(slot) + 1;
	}

private:
	static PresentationTarget Slot_Target(PlaybackSlot slot) noexcept
	{
		PresentationTarget target{};
		if (slot == PlaybackSlot::Fullscreen)
			target.layout = PresentationLayout::Fit_Output;
		return target;
	}

	static bool Fit_To_Output(
		const DecodedVideoFrame &frame,
		PresentationExtent output,
		PresentationRect &destination) noexcept
	{
		if (frame.width == 0 || frame.height == 0)
			return false;

		const double video_aspect = static_cast<double>(frame.width) / static_cast<double>(frame.height);
		const double output_aspect = static_cast<double>(output.width) / static_cast<double>(output.height);
		if (!std::isfinite(video_aspect) || !std::isfinite(output_aspect))
			return false;

		destination = {0, 0, output.width, output.height};
		if (output_aspect >= video_aspect) {
			const std::uint32_t width = static_cast<std::uint32_t>(std::min<double>(
				output.width, static_cast<double>(output.height) * video_aspect + 0.5));
			destination.x = static_cast<std::int32_t>((output.width - width) / 2);
			destination.width = width;
		} else {
			const std::uint32_t height = static_cast<std::uint32_t>(std::min<double>(
				output.height, static_cast<double>(output.width) / video_aspect + 0.5));
			destination.y = static_cast<std::int32_t>((output.height - height) / 2);
			destination.height = height;
		}
		return destination.width != 0 && destination.height != 0;
	}

	bool Open_Internal(SessionId id, std::string_view source, PresentationTarget target, PlaybackMode mode)
	{
		if (id == Invalid_Presentation)
			return false;

		const std::size_t index = Find_Or_Allocate(id);
		if (index == Max_Sessions)
			return false;

		m_players[index].Close();
		m_sources[index].reset();
		m_targets[index] = target;
		m_players[index].Set_Audio_Sink(m_audio_sink);
		m_players[index].Set_Mode(mode);
		if (!m_players[index].Open(source)) {
			m_players[index].Close();
			Release(index);
			return false;
		}
		return true;
	}

	bool Open_Internal(
		SessionId id,
		std::unique_ptr<Source> source,
		PresentationTarget target,
		PlaybackMode mode)
	{
		if (id == Invalid_Presentation || source == nullptr)
			return false;

		const std::size_t index = Find_Or_Allocate(id);
		if (index == Max_Sessions)
			return false;

		m_players[index].Close();
		m_sources[index].reset();
		m_targets[index] = target;
		m_players[index].Set_Audio_Sink(m_audio_sink);
		m_players[index].Set_Mode(mode);
		if (!m_players[index].Open(*source)) {
			m_players[index].Close();
			Release(index);
			return false;
		}
		m_sources[index] = std::move(source);
		return true;
	}

	SessionId Allocate_Session_Id() noexcept
	{
		for (std::size_t attempt = 0; attempt < Max_Sessions * 2; ++attempt) {
			const SessionId id = m_next_session_id++;
			if (id == Invalid_Presentation || id == Session_Id(PlaybackSlot::Fullscreen)
				|| id == Session_Id(PlaybackSlot::Cameo) || Find(id) != Max_Sessions)
				continue;
			return id;
		}
		return Invalid_Presentation;
	}

	std::size_t Find(SessionId id) const noexcept
	{
		for (std::size_t index = 0; index < Max_Sessions; ++index) {
			if (m_used[index] && m_ids[index] == id)
				return index;
		}
		return Max_Sessions;
	}

	std::size_t Find_Or_Allocate(SessionId id) noexcept
	{
		const std::size_t existing = Find(id);
		if (existing != Max_Sessions)
			return existing;

		for (std::size_t index = 0; index < Max_Sessions; ++index) {
			if (!m_used[index]) {
				m_used[index] = true;
				m_ids[index] = id;
				return index;
			}
		}
		return Max_Sessions;
	}

	void Release(std::size_t index) noexcept
	{
		m_used[index] = false;
		m_ids[index] = 0;
		m_targets[index] = {};
	}

	std::array<FFmpegPlayer, Max_Sessions> m_players{};
	std::array<std::unique_ptr<Source>, Max_Sessions> m_sources{};
	std::array<SessionId, Max_Sessions> m_ids{};
	std::array<bool, Max_Sessions> m_used{};
	std::array<PresentationTarget, Max_Sessions> m_targets{};
	AudioSink *m_audio_sink = nullptr;
	SessionId m_next_session_id = 100;
};

export Runtime &Get_Runtime() noexcept
{
	static Runtime runtime;
	return runtime;
}

}
