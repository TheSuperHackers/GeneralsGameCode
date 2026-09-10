module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

export module Video.FFmpeg.Decoder;

export import Video.Decoder;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 5244)
#endif
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Engine::Video
{

export class FFmpegDecoder final : public Decoder
{
public:
	FFmpegDecoder() noexcept = default;
	~FFmpegDecoder() noexcept override
	{
		Close();
	}

	FFmpegDecoder(const FFmpegDecoder &) = delete;
	FFmpegDecoder &operator=(const FFmpegDecoder &) = delete;

	bool Open(std::string_view source) override
	{
		Close();
		if (source.empty())
			return false;

		const std::string path(source);
		if (avformat_open_input(&m_format, path.c_str(), nullptr, nullptr) < 0
			|| avformat_find_stream_info(m_format, nullptr) < 0) {
			Close();
			return false;
		}
		return Initialize_Stream();
	}

	bool Open(Source &source) override
	{
		Close();
		m_source = &source;
		m_format = avformat_alloc_context();
		if (m_format == nullptr) {
			Close();
			return false;
		}

		constexpr std::size_t io_buffer_size = 0x10000;
		m_io_buffer = static_cast<unsigned char *>(av_malloc(io_buffer_size));
		if (m_io_buffer == nullptr) {
			Close();
			return false;
		}
		m_io = avio_alloc_context(
			m_io_buffer,
			static_cast<int>(io_buffer_size),
			0,
			m_source,
			&Read_Source,
			nullptr,
			&Seek_Source);
		if (m_io == nullptr) {
			Close();
			return false;
		}

		m_format->pb = m_io;
		m_format->flags |= AVFMT_FLAG_CUSTOM_IO;
		if (avformat_open_input(&m_format, nullptr, nullptr, nullptr) < 0
			|| avformat_find_stream_info(m_format, nullptr) < 0) {
			Close();
			return false;
		}
		return Initialize_Stream();
	}

	private:
	bool Initialize_Stream()
	{
		for (unsigned int index = 0; index < m_format->nb_streams; ++index) {
			const AVMediaType type = m_format->streams[index]->codecpar->codec_type;
			if (type == AVMEDIA_TYPE_VIDEO && m_stream_index < 0) {
				m_stream_index = static_cast<int>(index);
			}
			if (type == AVMEDIA_TYPE_AUDIO && m_audio_stream_index < 0)
				m_audio_stream_index = static_cast<int>(index);
		}
		if (m_stream_index < 0) {
			Close();
			return false;
		}

		AVStream *stream = m_format->streams[m_stream_index];
		const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
		if (codec == nullptr) {
			Close();
			return false;
		}
		m_codec = avcodec_alloc_context3(codec);
		if (m_codec == nullptr
			|| avcodec_parameters_to_context(m_codec, stream->codecpar) < 0
			|| avcodec_open2(m_codec, codec, nullptr) < 0) {
			Close();
			return false;
		}

		if (m_audio_stream_index >= 0) {
			AVStream *audio_stream = m_format->streams[m_audio_stream_index];
			const AVCodec *audio_codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
			if (audio_codec != nullptr) {
				m_audio_codec = avcodec_alloc_context3(audio_codec);
				if (m_audio_codec == nullptr
					|| avcodec_parameters_to_context(m_audio_codec, audio_stream->codecpar) < 0
					|| avcodec_open2(m_audio_codec, audio_codec, nullptr) < 0) {
					if (m_audio_codec != nullptr)
						avcodec_free_context(&m_audio_codec);
					m_audio_stream_index = -1;
				}
			}
			else {
				m_audio_stream_index = -1;
			}
		}

		m_packet = av_packet_alloc();
		m_frame = av_frame_alloc();
		if (m_audio_codec != nullptr)
			m_audio_frame = av_frame_alloc();
		if (m_packet == nullptr || m_frame == nullptr
			|| (m_audio_codec != nullptr && m_audio_frame == nullptr)
			|| m_codec->width <= 0 || m_codec->height <= 0) {
			Close();
			return false;
		}

		const AVRational frame_rate = av_guess_frame_rate(m_format, stream, nullptr);
		if (frame_rate.num > 0 && frame_rate.den > 0) {
			m_info.frame_duration_seconds = static_cast<double>(frame_rate.den) / frame_rate.num;
			m_frame_rate = frame_rate;
		}
		m_info.width = static_cast<std::uint32_t>(m_codec->width);
		m_info.height = static_cast<std::uint32_t>(m_codec->height);
		m_info.frame_count = stream->nb_frames > 0 ? static_cast<std::uint64_t>(stream->nb_frames) : 0;
		if (m_info.frame_count == 0 && m_format->duration > 0 && frame_rate.num > 0 && frame_rate.den > 0) {
			const long double seconds = static_cast<long double>(m_format->duration) / AV_TIME_BASE;
			const long double count = seconds * frame_rate.num / frame_rate.den;
			if (count > 0.0L && count < std::numeric_limits<std::uint64_t>::max())
				m_info.frame_count = static_cast<std::uint64_t>(count + 0.5L);
		}

		const std::uint64_t row_pitch = static_cast<std::uint64_t>(m_info.width) * 4u;
		if (row_pitch == 0 || m_info.height > std::numeric_limits<std::uint64_t>::max() / row_pitch) {
			Close();
			return false;
		}
		const std::uint64_t pixel_size = row_pitch * m_info.height;
		if (row_pitch > std::numeric_limits<std::uint32_t>::max()
			|| pixel_size > std::numeric_limits<std::size_t>::max()) {
			Close();
			return false;
		}
		m_pixels.resize(static_cast<std::size_t>(pixel_size));
		m_next_frame = 0;
		m_input_eof = false;
		m_video_flush_sent = false;
		m_audio_flush_sent = m_audio_codec == nullptr;
		m_video_drained = false;
		m_audio_drained = m_audio_codec == nullptr;
		m_audio_sample_cursor = 0;
		return true;
	}

	static int Read_Source(void *opaque, unsigned char *destination, int destination_size)
	{
		Source *source = static_cast<Source *>(opaque);
		if (source == nullptr || destination == nullptr || destination_size <= 0)
			return AVERROR_EOF;

		const std::size_t bytes_read = source->Read({reinterpret_cast<std::byte *>(destination),
			static_cast<std::size_t>(destination_size)});
		if (bytes_read == 0)
			return AVERROR_EOF;
		return static_cast<int>(std::min<std::size_t>(bytes_read, static_cast<std::size_t>(std::numeric_limits<int>::max())));
	}

	static std::int64_t Seek_Source(void *opaque, std::int64_t offset, int whence)
	{
		Source *source = static_cast<Source *>(opaque);
		if (source == nullptr)
			return -1;
		if ((whence & ~AVSEEK_FORCE) == AVSEEK_SIZE)
			return source->Size();

		SourceSeekOrigin origin = SourceSeekOrigin::Begin;
		switch (whence & ~AVSEEK_FORCE) {
		case SEEK_SET:
			origin = SourceSeekOrigin::Begin;
			break;
		case SEEK_CUR:
			origin = SourceSeekOrigin::Current;
			break;
		case SEEK_END:
			origin = SourceSeekOrigin::End;
			break;
		default:
			return -1;
		}

		return source->Seek(offset, origin) ? source->Tell() : -1;
	}

	public:

	void Close() noexcept override
	{
		if (m_frame != nullptr)
			av_frame_free(&m_frame);
		if (m_audio_frame != nullptr)
			av_frame_free(&m_audio_frame);
		if (m_packet != nullptr)
			av_packet_free(&m_packet);
		if (m_codec != nullptr)
			avcodec_free_context(&m_codec);
		if (m_audio_codec != nullptr)
			avcodec_free_context(&m_audio_codec);
		// AVFMT_FLAG_CUSTOM_IO leaves ownership of the AVIOContext and its
		// buffer with this decoder. Detach it before closing the format so
		// FFmpeg cannot release the same objects a second time.
		if (m_format != nullptr && m_format->pb == m_io)
			m_format->pb = nullptr;
		if (m_format != nullptr)
			avformat_close_input(&m_format);
		if (m_io != nullptr) {
			// Libavformat may replace the initial buffer while probing. Free
			// the buffer currently owned by the AVIOContext, not the original
			// allocation kept in m_io_buffer.
			av_freep(&m_io->buffer);
			avio_context_free(&m_io);
		} else if (m_io_buffer != nullptr) {
			av_free(m_io_buffer);
		}
		m_io_buffer = nullptr;
		m_source = nullptr;
		if (m_scale != nullptr) {
			sws_freeContext(m_scale);
			m_scale = nullptr;
		}
		if (m_audio_scale != nullptr)
			swr_free(&m_audio_scale);
		m_pixels.clear();
		m_audio_pixels.clear();
		m_info = {};
		m_stream_index = -1;
		m_audio_stream_index = -1;
		m_frame_rate = {};
		m_next_frame = 0;
		m_audio_sample_rate = 0;
		m_audio_channels = 0;
		m_audio_input_format = AV_SAMPLE_FMT_NONE;
		m_audio_sample_cursor = 0;
		m_input_eof = false;
		m_video_flush_sent = false;
		m_audio_flush_sent = false;
		m_video_drained = false;
		m_audio_drained = false;
	}

	StreamInfo Info() const noexcept override
	{
		return m_info;
	}

	void Set_Audio_Sink(AudioSink *sink) noexcept override
	{
		m_audio_sink = sink;
	}

	DecodeResult Decode_Next(DecodedVideoFrame &output) override
	{
		output = {};
		if (m_codec == nullptr || m_packet == nullptr || m_frame == nullptr || m_format == nullptr)
			return DecodeResult::Error;

		for (;;) {
			if (!m_video_drained) {
				const int result = avcodec_receive_frame(m_codec, m_frame);
				if (result == 0) {
					if (!Convert_Frame(*m_frame))
						return DecodeResult::Error;

					std::uint64_t presentation_time = m_next_frame * Duration_Microseconds();
					const AVStream *stream = m_format->streams[m_stream_index];
					if (m_frame->best_effort_timestamp != AV_NOPTS_VALUE) {
						const std::int64_t timestamp = av_rescale_q(
							m_frame->best_effort_timestamp, stream->time_base, AVRational{1, 1000000});
						presentation_time = timestamp < 0 ? 0 : static_cast<std::uint64_t>(timestamp);
					}
					output = {
						m_info.width,
						m_info.height,
						m_info.width * 4u,
						PixelFormat::RGBA8,
						m_next_frame++,
						presentation_time,
						{m_pixels.data(), m_pixels.size()}
					};
					return DecodeResult::Frame;
				}
				if (result != AVERROR(EAGAIN) && result != AVERROR_EOF)
					return DecodeResult::Error;
				if (result == AVERROR_EOF)
					m_video_drained = true;
			}

			if (!m_audio_drained) {
				const int result = avcodec_receive_frame(m_audio_codec, m_audio_frame);
				if (result == 0) {
					if (!Convert_Audio_Frame(*m_audio_frame))
						return DecodeResult::Error;
					continue;
				}
				if (result != AVERROR(EAGAIN) && result != AVERROR_EOF)
					return DecodeResult::Error;
				if (result == AVERROR_EOF)
					m_audio_drained = true;
			}

			if (m_input_eof) {
				if (!m_video_flush_sent) {
					const int result = avcodec_send_packet(m_codec, nullptr);
					if (result < 0 && result != AVERROR_EOF && result != AVERROR(EAGAIN))
						return DecodeResult::Error;
					m_video_flush_sent = true;
					if (result == AVERROR_EOF)
						m_video_drained = true;
				}
				if (!m_audio_flush_sent) {
					const int result = avcodec_send_packet(m_audio_codec, nullptr);
					if (result < 0 && result != AVERROR_EOF && result != AVERROR(EAGAIN))
						return DecodeResult::Error;
					m_audio_flush_sent = true;
					if (result == AVERROR_EOF)
						m_audio_drained = true;
				}
				if (m_video_drained && m_audio_drained)
					return DecodeResult::EndOfStream;
				continue;
			}

			int result = av_read_frame(m_format, m_packet);
			if (result == AVERROR_EOF) {
				m_input_eof = true;
				continue;
			}
			if (result < 0)
				return DecodeResult::Error;

			if (m_packet->stream_index == m_stream_index) {
				result = avcodec_send_packet(m_codec, m_packet);
				av_packet_unref(m_packet);
				if (result < 0 && result != AVERROR(EAGAIN))
					return DecodeResult::Error;
			}
			else if (m_packet->stream_index == m_audio_stream_index && m_audio_codec != nullptr) {
				result = avcodec_send_packet(m_audio_codec, m_packet);
				av_packet_unref(m_packet);
				if (result < 0 && result != AVERROR(EAGAIN))
					return DecodeResult::Error;
			}
			else {
				av_packet_unref(m_packet);
			}
		}
	}

	bool Seek(std::uint64_t frame_index) override
	{
		if (m_format == nullptr || m_codec == nullptr || m_stream_index < 0
			|| frame_index > std::numeric_limits<std::int64_t>::max())
			return false;

		const AVStream *stream = m_format->streams[m_stream_index];
		if (m_frame_rate.num <= 0 || m_frame_rate.den <= 0
			|| av_seek_frame(m_format, m_stream_index,
				av_rescale_q(static_cast<std::int64_t>(frame_index),
					AVRational{m_frame_rate.den, m_frame_rate.num}, stream->time_base),
				AVSEEK_FLAG_BACKWARD) < 0)
			return false;

		avcodec_flush_buffers(m_codec);
		if (m_audio_codec != nullptr)
			avcodec_flush_buffers(m_audio_codec);
		av_packet_unref(m_packet);
		if (m_audio_scale != nullptr)
			swr_free(&m_audio_scale);
		m_audio_pixels.clear();
		m_audio_sample_rate = 0;
		m_audio_channels = 0;
		m_audio_input_format = AV_SAMPLE_FMT_NONE;
		m_audio_sample_cursor = 0;
		m_next_frame = frame_index;
		m_input_eof = false;
		m_video_flush_sent = false;
		m_audio_flush_sent = m_audio_codec == nullptr;
		m_video_drained = false;
		m_audio_drained = m_audio_codec == nullptr;
		return true;
	}

private:
	std::uint64_t Duration_Microseconds() const noexcept
	{
		if (!std::isfinite(m_info.frame_duration_seconds) || m_info.frame_duration_seconds <= 0.0)
			return 0;
		return static_cast<std::uint64_t>(m_info.frame_duration_seconds * 1000000.0 + 0.5);
	}

	bool Convert_Frame(const AVFrame &frame)
	{
		m_scale = sws_getCachedContext(
			m_scale,
			frame.width,
			frame.height,
			static_cast<AVPixelFormat>(frame.format),
			static_cast<int>(m_info.width),
			static_cast<int>(m_info.height),
			AV_PIX_FMT_RGBA,
			SWS_BICUBIC,
			nullptr,
			nullptr,
			nullptr);
		if (m_scale == nullptr)
			return false;

		uint8_t *destination[] = {reinterpret_cast<uint8_t *>(m_pixels.data())};
		const int stride[] = {static_cast<int>(m_info.width * 4u)};
		return sws_scale(m_scale, frame.data, frame.linesize, 0, frame.height, destination, stride) > 0;
	}

	bool Convert_Audio_Frame(const AVFrame &frame)
	{
		if (m_audio_sink == nullptr || frame.sample_rate <= 0 || frame.nb_samples <= 0)
			return true;

		const int input_channels = frame.ch_layout.nb_channels > 0
			? frame.ch_layout.nb_channels
			: (m_audio_codec != nullptr && m_audio_codec->ch_layout.nb_channels > 0
				? m_audio_codec->ch_layout.nb_channels : 2);
		if (input_channels <= 0 || input_channels > std::numeric_limits<std::uint16_t>::max())
			return false;

		const std::uint16_t output_channels = input_channels == 1 ? 1 : 2;
		const AVSampleFormat input_format = static_cast<AVSampleFormat>(frame.format);
		if (m_audio_scale == nullptr || m_audio_sample_rate != frame.sample_rate
			|| m_audio_channels != output_channels || m_audio_input_format != input_format) {
			if (m_audio_scale != nullptr)
				swr_free(&m_audio_scale);

			AVChannelLayout input_layout = frame.ch_layout;
			bool owns_input_layout = false;
			if (input_layout.nb_channels == 0) {
				av_channel_layout_default(&input_layout, input_channels);
				owns_input_layout = true;
			}
			AVChannelLayout output_layout{};
			av_channel_layout_default(&output_layout, output_channels);
			const int alloc_result = swr_alloc_set_opts2(
				&m_audio_scale,
				&output_layout,
				AV_SAMPLE_FMT_S16,
				frame.sample_rate,
				&input_layout,
				input_format,
				frame.sample_rate,
				0,
				nullptr);
			const int init_result = alloc_result >= 0 && m_audio_scale != nullptr
				? swr_init(m_audio_scale) : -1;
			av_channel_layout_uninit(&output_layout);
			if (owns_input_layout)
				av_channel_layout_uninit(&input_layout);
			if (alloc_result < 0 || init_result < 0) {
				if (m_audio_scale != nullptr)
					swr_free(&m_audio_scale);
				return false;
			}
			m_audio_sample_rate = static_cast<std::uint32_t>(frame.sample_rate);
			m_audio_channels = output_channels;
			m_audio_input_format = input_format;
		}

		const int output_capacity = swr_get_out_samples(m_audio_scale, frame.nb_samples);
		if (output_capacity <= 0
			|| static_cast<std::uint64_t>(output_capacity) * output_channels * sizeof(std::int16_t)
				> std::numeric_limits<std::size_t>::max())
			return false;
		const std::size_t output_size = static_cast<std::size_t>(output_capacity)
			* output_channels * sizeof(std::int16_t);
		if (m_audio_pixels.size() < output_size)
			m_audio_pixels.resize(output_size);

		uint8_t *output_data[] = {reinterpret_cast<uint8_t *>(m_audio_pixels.data())};
		const int converted = swr_convert(
			m_audio_scale,
			output_data,
			output_capacity,
			const_cast<const uint8_t **>(frame.extended_data),
			frame.nb_samples);
		if (converted < 0)
			return false;
		const int data_size = av_samples_get_buffer_size(
			nullptr, output_channels, converted, AV_SAMPLE_FMT_S16, 1);
		if (data_size <= 0)
			return true;

		std::uint64_t presentation_time = m_audio_sample_rate != 0
			? (m_audio_sample_cursor * 1000000u) / m_audio_sample_rate : 0;
		const AVStream *stream = m_format->streams[m_audio_stream_index];
		if (frame.best_effort_timestamp != AV_NOPTS_VALUE) {
			const std::int64_t timestamp = av_rescale_q(
				frame.best_effort_timestamp, stream->time_base, AVRational{1, 1000000});
			presentation_time = timestamp < 0 ? 0 : static_cast<std::uint64_t>(timestamp);
		}
		const DecodedAudioChunk chunk{
			m_audio_sample_rate,
			m_audio_channels,
			AudioSampleFormat::S16,
			presentation_time,
			{m_audio_pixels.data(), static_cast<std::size_t>(data_size)}};
		m_audio_sample_cursor += static_cast<std::uint64_t>(converted);
		(void)m_audio_sink->Submit(chunk);
		return true;
	}

	AVFormatContext *m_format = nullptr;
	AVIOContext *m_io = nullptr;
	unsigned char *m_io_buffer = nullptr;
	Source *m_source = nullptr;
	AVCodecContext *m_codec = nullptr;
	AVCodecContext *m_audio_codec = nullptr;
	AVPacket *m_packet = nullptr;
	AVFrame *m_frame = nullptr;
	AVFrame *m_audio_frame = nullptr;
	SwsContext *m_scale = nullptr;
	SwrContext *m_audio_scale = nullptr;
	std::vector<std::byte> m_pixels;
	std::vector<std::byte> m_audio_pixels;
	AudioSink *m_audio_sink = nullptr;
	StreamInfo m_info{};
	AVRational m_frame_rate{};
	int m_stream_index = -1;
	int m_audio_stream_index = -1;
	std::uint64_t m_next_frame = 0;
	std::uint32_t m_audio_sample_rate = 0;
	std::uint16_t m_audio_channels = 0;
	AVSampleFormat m_audio_input_format = AV_SAMPLE_FMT_NONE;
	std::uint64_t m_audio_sample_cursor = 0;
	bool m_input_eof = false;
	bool m_video_flush_sent = false;
	bool m_audio_flush_sent = false;
	bool m_video_drained = false;
	bool m_audio_drained = false;
};

}
