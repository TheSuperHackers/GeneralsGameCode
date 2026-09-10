module;

#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>

export module Video.Playback;

export import Video.Decoder;

namespace Engine::Video
{

export enum class PlaybackState : std::uint8_t
{
	Closed,
	Playing,
	Paused,
	Finished,
	Error
};

export enum class PlaybackMode : std::uint8_t
{
	Once,
	Loop,
	Hold_Last_Frame
};

export class Playback final
{
public:
	explicit Playback(Decoder &decoder) noexcept
		: m_decoder(&decoder)
	{
	}

	Playback(const Playback &) = delete;
	Playback &operator=(const Playback &) = delete;

	void Set_Audio_Sink(AudioSink *sink) noexcept
	{
		if (m_audio_sink != nullptr && m_audio_sink != sink && m_state != PlaybackState::Closed)
			m_audio_sink->Stop();
		m_audio_sink = sink;
		if (m_decoder != nullptr)
			m_decoder->Set_Audio_Sink(sink);
	}

	~Playback() noexcept
	{
		Close();
	}

	bool Open(std::string_view source)
	{
		Close();
		if (m_decoder != nullptr)
			m_decoder->Set_Audio_Sink(m_audio_sink);
		if (m_decoder == nullptr || !m_decoder->Open(source)) {
			if (m_decoder != nullptr)
				m_decoder->Close();
			m_state = PlaybackState::Error;
			return false;
		}

		return Initialize_Open();
	}

	bool Open(Source &source)
	{
		Close();
		if (m_decoder != nullptr)
			m_decoder->Set_Audio_Sink(m_audio_sink);
		if (m_decoder == nullptr || !m_decoder->Open(source)) {
			if (m_decoder != nullptr)
				m_decoder->Close();
			m_state = PlaybackState::Error;
			return false;
		}

		return Initialize_Open();
	}

	void Set_Mode(PlaybackMode mode) noexcept
	{
		m_mode = mode;
	}

private:
	bool Initialize_Open()
	{
		m_info = m_decoder->Info();
		if (m_info.width == 0 || m_info.height == 0 || !std::isfinite(m_info.frame_duration_seconds)
			|| m_info.frame_duration_seconds <= 0.0) {
			m_decoder->Close();
			m_state = PlaybackState::Error;
			return false;
		}

		m_state = PlaybackState::Playing;
		m_frame = {};
		m_has_frame = false;
		m_new_frame = false;
		m_frame_index = 0;
		m_frame_sequence = 0;
		m_elapsed_seconds = 0.0;
		if (m_audio_sink != nullptr) {
			m_audio_sink->Reset();
			m_audio_sink->Start();
		}
		return true;
	}

public:
	void Close() noexcept
	{
		const bool was_open = m_state != PlaybackState::Closed;
		if (m_decoder != nullptr && m_state != PlaybackState::Closed)
			m_decoder->Close();
		if (was_open && m_audio_sink != nullptr)
			m_audio_sink->Stop();

		m_state = PlaybackState::Closed;
		m_info = {};
		m_frame = {};
		m_has_frame = false;
		m_new_frame = false;
		m_frame_index = 0;
		m_frame_sequence = 0;
		m_elapsed_seconds = 0.0;
	}

	bool Play() noexcept
	{
		if (m_state != PlaybackState::Paused)
			return false;

		m_state = PlaybackState::Playing;
		if (m_audio_sink != nullptr)
			m_audio_sink->Resume();
		return true;
	}

	bool Pause() noexcept
	{
		if (m_state != PlaybackState::Playing)
			return false;

		m_state = PlaybackState::Paused;
		if (m_audio_sink != nullptr)
			m_audio_sink->Pause();
		return true;
	}

	bool Update(double delta_seconds)
	{
		m_new_frame = false;
		if (m_state != PlaybackState::Playing || m_decoder == nullptr)
			return false;

		if (!m_has_frame && !Decode_One())
			return false;

		if (!std::isfinite(delta_seconds) || delta_seconds < 0.0)
			delta_seconds = 0.0;
		m_elapsed_seconds += delta_seconds;

		bool delivered = m_new_frame;
		const double frame_duration = m_info.frame_duration_seconds;
		while (m_elapsed_seconds >= frame_duration && m_state == PlaybackState::Playing) {
			if (m_info.frame_count != 0 && m_frame_index + 1 >= m_info.frame_count) {
				if (m_mode != PlaybackMode::Loop || !Restart_From_Beginning()) {
					m_state = PlaybackState::Finished;
					break;
				}
				m_frame_index = 0;
			}

			m_elapsed_seconds -= frame_duration;
			if (!Decode_One(false))
				break;
			delivered = true;
		}

		return delivered;
	}

	bool Seek(std::uint64_t frame_index)
	{
		if (m_decoder == nullptr || m_state == PlaybackState::Closed || !m_decoder->Seek(frame_index))
			return false;
		if (m_audio_sink != nullptr) {
			m_audio_sink->Reset();
			m_audio_sink->Start();
		}

		m_frame = {};
		m_has_frame = false;
		m_new_frame = false;
		m_frame_index = frame_index;
		m_frame_sequence = 0;
		m_elapsed_seconds = 0.0;
		m_state = PlaybackState::Playing;
		return true;
	}

	PlaybackState State() const noexcept
	{
		return m_state;
	}

	StreamInfo Info() const noexcept
	{
		return m_info;
	}

	bool Is_Frame_Ready() const noexcept
	{
		return m_new_frame;
	}

	void Clear_Frame_Ready() noexcept
	{
		m_new_frame = false;
	}

	const DecodedVideoFrame *Current_Frame() const noexcept
	{
		return m_has_frame ? &m_frame : nullptr;
	}

	std::uint64_t Frame_Index() const noexcept
	{
		return m_frame_index;
	}

	std::uint64_t Frame_Sequence() const noexcept
	{
		return m_frame_sequence;
	}

	double Elapsed_Seconds() const noexcept
	{
		return m_elapsed_seconds;
	}

	private:
	bool Restart_From_Beginning()
	{
		if (m_decoder == nullptr || !m_decoder->Seek(0))
			return false;
		if (m_audio_sink != nullptr) {
			m_audio_sink->Reset();
			m_audio_sink->Start();
		}
		m_frame_index = 0;
		return true;
	}

	bool Decode_One(bool allow_loop_restart = true)
	{
		DecodedVideoFrame frame{};
		switch (m_decoder->Decode_Next(frame)) {
		case DecodeResult::Frame:
			if (!frame.Is_Valid()) {
				m_state = PlaybackState::Error;
				return false;
			}
			m_frame = frame;
			m_frame_index = frame.frame_index;
			m_has_frame = true;
			m_new_frame = true;
			++m_frame_sequence;
			return true;
		case DecodeResult::NotReady:
			return false;
		case DecodeResult::EndOfStream:
			if (allow_loop_restart && m_mode == PlaybackMode::Loop && Restart_From_Beginning()) {
				return Decode_One(false);
			}
			m_state = PlaybackState::Finished;
			return false;
		case DecodeResult::Error:
			m_state = PlaybackState::Error;
			return false;
		}

		m_state = PlaybackState::Error;
		return false;
	}

	Decoder *m_decoder = nullptr;
	AudioSink *m_audio_sink = nullptr;
	PlaybackMode m_mode = PlaybackMode::Once;
	StreamInfo m_info{};
	DecodedVideoFrame m_frame{};
	PlaybackState m_state = PlaybackState::Closed;
	std::uint64_t m_frame_index = 0;
	std::uint64_t m_frame_sequence = 0;
	double m_elapsed_seconds = 0.0;
	bool m_has_frame = false;
	bool m_new_frame = false;
};

}
