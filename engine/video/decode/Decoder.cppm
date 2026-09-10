module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

export module Video.Decoder;

export import Video.Frame;

namespace Engine::Video
{

export struct StreamInfo final
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint64_t frame_count = 0;
	double frame_duration_seconds = 1.0 / 30.0;
};

export enum class SourceSeekOrigin : std::uint8_t
{
	Begin,
	Current,
	End
};

// A source keeps the decoder independent from the game filesystem. Runtime
// integrations can provide archive-backed or ordinary file I/O without
// moving that platform-specific detail into the graphics layer.
export class Source
{
public:
	virtual ~Source() noexcept = default;
	virtual std::size_t Read(std::span<std::byte> destination) noexcept = 0;
	virtual bool Seek(std::int64_t offset, SourceSeekOrigin origin) noexcept = 0;
	virtual std::int64_t Tell() const noexcept = 0;
	virtual std::int64_t Size() const noexcept = 0;
};

export enum class DecodeResult : std::uint8_t
{
	Frame,
	NotReady,
	EndOfStream,
	Error
};

export enum class AudioSampleFormat : std::uint8_t
{
	S16
};

export struct DecodedAudioChunk final
{
	std::uint32_t sample_rate = 0;
	std::uint16_t channels = 0;
	AudioSampleFormat format = AudioSampleFormat::S16;
	std::uint64_t presentation_time_us = 0;
	std::span<const std::byte> samples{};

	bool Is_Valid() const noexcept
	{
		return sample_rate != 0 && channels != 0 && format == AudioSampleFormat::S16
			&& !samples.empty() && samples.size() % (static_cast<std::size_t>(channels) * 2u) == 0;
	}
};

// The sink must copy the samples before Submit returns. Decoders reuse their
// conversion buffer for the next decoded audio frame.
export class AudioSink
{
public:
	virtual ~AudioSink() noexcept = default;
	virtual bool Submit(const DecodedAudioChunk &chunk) = 0;
	virtual void Start() noexcept = 0;
	virtual void Pause() noexcept = 0;
	virtual void Resume() noexcept = 0;
	virtual void Stop() noexcept = 0;
	virtual void Reset() noexcept = 0;
};

// A decoder owns the format-specific decode state. VideoPlayback owns the
// stream state and timing, and only observes decoded frame views from here.
export class Decoder
{
public:
	virtual ~Decoder() noexcept = default;

	virtual bool Open(std::string_view source) = 0;
	virtual bool Open(Source &source) = 0;
	virtual void Close() noexcept = 0;
	virtual StreamInfo Info() const noexcept = 0;
	virtual DecodeResult Decode_Next(DecodedVideoFrame &frame) = 0;
	virtual bool Seek(std::uint64_t frame_index) = 0;
	virtual void Set_Audio_Sink(AudioSink *sink) noexcept { (void)sink; }
};

}
