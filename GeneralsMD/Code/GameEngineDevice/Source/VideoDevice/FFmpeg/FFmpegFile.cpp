/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//////// FFmpegFile.cpp ///////////////////////////
// Stephan Vedder, April 2025
/////////////////////////////////////////////////

#include "VideoDevice/FFmpeg/FFmpegFile.h"
#include "Common/File.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


FFmpegFile::FFmpegFile() {}

FFmpegFile::FFmpegFile(File *file)
{
	open(file);
}

FFmpegFile::~FFmpegFile()
{
	close();
}

Bool FFmpegFile::open(File *file)
{
	DEBUG_ASSERTCRASH(m_file == nullptr, ("already open"));
	DEBUG_ASSERTCRASH(file != nullptr, ("null file pointer"));
#if LOGGING_LEVEL != LOGLEVEL_NONE
	av_log_set_level(AV_LOG_INFO);
#endif

// This is required for FFmpeg older than 4.0 -> deprecated afterwards though
#if LIBAVFORMAT_VERSION_MAJOR < 58
	av_register_all();
#endif

	m_file = file;

	// FFmpeg setup
	m_fmtCtx = avformat_alloc_context();
	if (!m_fmtCtx) {
		DEBUG_LOG(("Failed to alloc AVFormatContext"));
		close();
		return false;
	}

	constexpr size_t avio_ctx_buffer_size = 0x10000;
	uint8_t *buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
	if (buffer == nullptr) {
		DEBUG_LOG(("Failed to alloc AVIOContextBuffer"));
		close();
		return false;
	}

	m_avioCtx = avio_alloc_context(buffer, avio_ctx_buffer_size, 0, file, &readPacket, nullptr, nullptr);
	if (m_avioCtx == nullptr) {
		DEBUG_LOG(("Failed to alloc AVIOContext"));
		close();
		return false;
	}

	m_fmtCtx->pb = m_avioCtx;
	m_fmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

	int result = avformat_open_input(&m_fmtCtx, nullptr, nullptr, nullptr);
	if (result < 0) {
		char error_buffer[1024];
		av_strerror(result, error_buffer, sizeof(error_buffer));
		DEBUG_LOG(("Failed 'avformat_open_input': %s", error_buffer));
		close();
		return false;
	}

	result = avformat_find_stream_info(m_fmtCtx, nullptr);
	if (result < 0) {
		char error_buffer[1024];
		av_strerror(result, error_buffer, sizeof(error_buffer));
		DEBUG_LOG(("Failed 'avformat_find_stream_info': %s", error_buffer));
		close();
		return false;
	}

	m_streams.resize(m_fmtCtx->nb_streams);
	for (unsigned int stream_idx = 0; stream_idx < m_fmtCtx->nb_streams; stream_idx++) {
		AVStream *av_stream = m_fmtCtx->streams[stream_idx];
		const AVCodec *input_codec = avcodec_find_decoder(av_stream->codecpar->codec_id);
		if (input_codec == nullptr) {
			DEBUG_LOG(("Codec not supported: '%s'", avcodec_get_name(av_stream->codecpar->codec_id)));
			close();
			return false;
		}

		AVCodecContext *codec_ctx = avcodec_alloc_context3(input_codec);
		if (codec_ctx == nullptr) {
			DEBUG_LOG(("Could not allocate codec context"));
			close();
			return false;
		}

		result = avcodec_parameters_to_context(codec_ctx, av_stream->codecpar);
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			DEBUG_LOG(("Failed 'avcodec_parameters_to_context': %s", error_buffer));
			close();
			return false;
		}

		result = avcodec_open2(codec_ctx, input_codec, nullptr);
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			DEBUG_LOG(("Failed 'avcodec_open2': %s", error_buffer));
			close();
			return false;
		}

		FFmpegStream &output_stream = m_streams[stream_idx];
		output_stream.codec_ctx = codec_ctx;
		output_stream.codec = input_codec;
		output_stream.stream_type = input_codec->type;
		output_stream.stream_idx = stream_idx;
		output_stream.frame = av_frame_alloc();
	}

	m_packet = av_packet_alloc();

	return true;
}

/**
 * Read an FFmpeg packet from file
 */
int FFmpegFile::readPacket(void *opaque, uint8_t *buf, int buf_size)
{
	File *file = static_cast<File *>(opaque);
	const int read = file->read(buf, buf_size);

	// Streaming protocol requires us to return real errors - when we read less equal 0 we're at EOF
	if (read <= 0)
		return AVERROR_EOF;

	return read;
}

/**
 * close all the open FFmpeg handles for an open file.
 */
void FFmpegFile::close()
{
	if (m_fmtCtx != nullptr) {
		avformat_close_input(&m_fmtCtx);
	}

	for (auto &stream : m_streams) {
		if (stream.codec_ctx != nullptr) {
			avcodec_free_context(&stream.codec_ctx);
			av_frame_free(&stream.frame);
		}
	}
	m_streams.clear();

	if (m_avioCtx != nullptr) {
		av_freep(&m_avioCtx->buffer);
		avio_context_free(&m_avioCtx);
	}

	if (m_packet != nullptr) {
		av_packet_free(&m_packet);
	}

	if (m_file != nullptr) {
		m_file->close();
		m_file = nullptr;
	}
}

Bool FFmpegFile::decodePacket()
{
	DEBUG_ASSERTCRASH(m_fmtCtx != nullptr, ("null format context"));
	DEBUG_ASSERTCRASH(m_packet != nullptr, ("null packet pointer"));

	int result = av_read_frame(m_fmtCtx, m_packet);
	if (result == AVERROR_EOF)
		return false;

	const int stream_idx = m_packet->stream_index;
	DEBUG_ASSERTCRASH(m_streams.size() > stream_idx, ("stream index out of bounds"));

	auto &stream = m_streams[stream_idx];
	AVCodecContext *codec_ctx = stream.codec_ctx;
	result = avcodec_send_packet(codec_ctx, m_packet);
	// Check if we need more data
	if (result == AVERROR(EAGAIN))
		return true;

	// Handle any other errors
	if (result < 0) {
		char error_buffer[1024];
		av_strerror(result, error_buffer, sizeof(error_buffer));
		DEBUG_LOG(("Failed 'avcodec_send_packet': %s", error_buffer));
		return false;
	}
	av_packet_unref(m_packet);

	// Get all frames in this packet
	while (result >= 0) {
		result = avcodec_receive_frame(codec_ctx, stream.frame);

		// Check if we need more data
		if (result == AVERROR(EAGAIN))
			return true;

		// Handle any other errors
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			DEBUG_LOG(("Failed 'avcodec_receive_frame': %s", error_buffer));
			return false;
		}

		if (m_frameCallback != nullptr) {
			m_frameCallback(stream.frame, stream_idx, stream.stream_type, m_userData);
		}
	}

	return true;
}

void FFmpegFile::seekFrame(int frame_idx)
{
	// Note: not tested, since not used ingame
	for (const auto &stream : m_streams) {
		Int64 timestamp = av_q2d(m_fmtCtx->streams[stream.stream_idx]->time_base) * frame_idx
			* av_q2d(m_fmtCtx->streams[stream.stream_idx]->avg_frame_rate);
		int result = av_seek_frame(m_fmtCtx, stream.stream_idx, timestamp, AVSEEK_FLAG_ANY);
		if (result < 0) {
			char error_buffer[1024];
			av_strerror(result, error_buffer, sizeof(error_buffer));
			DEBUG_LOG(("Failed 'av_seek_frame': %s", error_buffer));
		}
	}
}

Bool FFmpegFile::hasAudio() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_AUDIO);
	return stream != nullptr;
}

const FFmpegFile::FFmpegStream *FFmpegFile::findMatch(Int type) const
{
	for (auto &stream : m_streams) {
		if (stream.stream_type == type)
			return &stream;
	}

	return nullptr;
}

Int FFmpegFile::getNumChannels() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr)
		return 0;

	return stream->codec_ctx->ch_layout.nb_channels;
}

Int FFmpegFile::getSampleRate() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr)
		return 0;

	return stream->codec_ctx->sample_rate;
}

Int FFmpegFile::getBytesPerSample() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr)
		return 0;

	return av_get_bytes_per_sample(stream->codec_ctx->sample_fmt);
}

Int FFmpegFile::getSizeForSamples(Int numSamples) const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_AUDIO);
	if (stream == nullptr)
		return 0;

	return av_samples_get_buffer_size(nullptr, stream->codec_ctx->ch_layout.nb_channels, numSamples, stream->codec_ctx->sample_fmt, 1);
}

Int FFmpegFile::getHeight() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr)
		return 0;

	return stream->codec_ctx->height;
}

Int FFmpegFile::getWidth() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr)
		return 0;

	return stream->codec_ctx->width;
}

Int FFmpegFile::getNumFrames() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_VIDEO);
	if (m_fmtCtx == nullptr || stream == nullptr || m_fmtCtx->streams[stream->stream_idx] == nullptr)
		return 0;

	return (m_fmtCtx->duration / (double)AV_TIME_BASE) * av_q2d(m_fmtCtx->streams[stream->stream_idx]->avg_frame_rate);
}

Int FFmpegFile::getCurrentFrame() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr)
		return 0;
	return stream->codec_ctx->frame_num;
}

Int FFmpegFile::getPixelFormat() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr)
		return AV_PIX_FMT_NONE;

	return stream->codec_ctx->pix_fmt;
}

UnsignedInt FFmpegFile::getFrameTime() const
{
	const FFmpegStream *stream = findMatch(AVMEDIA_TYPE_VIDEO);
	if (stream == nullptr)
		return 0u;
	return 1000u / av_q2d(m_fmtCtx->streams[stream->stream_idx]->avg_frame_rate);
}