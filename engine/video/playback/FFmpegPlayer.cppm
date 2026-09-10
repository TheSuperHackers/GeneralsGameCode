module;

#include <cstddef>
#include <string_view>

export module Video.FFmpeg.Player;

export import Video.FFmpeg.Decoder;
export import Video.Playback;

namespace Engine::Video
{

export class FFmpegPlayer final
{
public:
	FFmpegPlayer() noexcept
		: m_playback(m_decoder)
	{
	}

	bool Open(std::string_view source)
	{
		return m_playback.Open(source);
	}

	bool Open(Source &source)
	{
		return m_playback.Open(source);
	}

	void Close() noexcept
	{
		m_playback.Close();
	}

	void Set_Mode(PlaybackMode mode) noexcept
	{
		m_playback.Set_Mode(mode);
	}

	void Set_Audio_Sink(AudioSink *sink) noexcept
	{
		m_playback.Set_Audio_Sink(sink);
	}

	bool Update(double delta_seconds)
	{
		return m_playback.Update(delta_seconds);
	}

	bool Pause() noexcept
	{
		return m_playback.Pause();
	}

	bool Play() noexcept
	{
		return m_playback.Play();
	}

	bool Seek(std::uint64_t frame_index)
	{
		return m_playback.Seek(frame_index);
	}

	PlaybackState State() const noexcept
	{
		return m_playback.State();
	}

	StreamInfo Info() const noexcept
	{
		return m_playback.Info();
	}

	const DecodedVideoFrame *Current_Frame() const noexcept
	{
		return m_playback.Current_Frame();
	}

	bool Is_Frame_Ready() const noexcept
	{
		return m_playback.Is_Frame_Ready();
	}

	void Clear_Frame_Ready() noexcept
	{
		m_playback.Clear_Frame_Ready();
	}

private:
	FFmpegDecoder m_decoder;
	Playback m_playback;
};

}
