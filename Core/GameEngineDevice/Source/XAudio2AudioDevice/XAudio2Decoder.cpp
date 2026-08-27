/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

#if defined(_WIN32) && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0602)

#include "Lib/BaseType.h"
#include "XAudio2Decoder.h"
#include "AudioFileProvider.h"

#include <unordered_map>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <mutex>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

static std::unordered_map<std::string, XAudio2Decoder::DecodedBuffer> s_pcmCache;
static std::mutex s_cacheMutex;

/// Pairs a provider + open handle for FFmpeg custom I/O callbacks.
struct XAudio2Decoder::ProviderIO
{
	AudioFileProvider *provider;
	AudioFileProvider::FileHandle handle;
};

static int xaDecoderReadPacket(void *opaque, uint8_t *buf, int buf_size)
{
	auto *io = static_cast<XAudio2Decoder::ProviderIO *>(opaque);
	int bytesRead = io->provider->read(io->handle, buf, buf_size);
	if (bytesRead <= 0)
		return AVERROR_EOF;
	return bytesRead;
}

static int64_t xaDecoderSeek(void *opaque, int64_t offset, int whence)
{
	auto *io = static_cast<XAudio2Decoder::ProviderIO *>(opaque);
	if (whence == AVSEEK_SIZE)
		return io->provider->size(io->handle);
	return io->provider->seek(io->handle, offset, whence);
}

/// Internal decode from an AVFormatContext (shared by both code paths).
/// Always outputs float32 PCM at 48 kHz for maximum quality.
static XAudio2Decoder::DecodedBuffer decodeFromFormat(AVFormatContext *fmtCtx)
{
	XAudio2Decoder::DecodedBuffer result = {nullptr, 0, 0, 0, XAudio2Decoder::OUTPUT_BITS_PER_SAMPLE};

	if (avformat_find_stream_info(fmtCtx, nullptr) < 0)
		return result;

	int audioStreamIdx = -1;
	for (unsigned i = 0; i < fmtCtx->nb_streams; ++i)
	{
		if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStreamIdx = (int)i;
			break;
		}
	}
	if (audioStreamIdx < 0)
		return result;

	AVCodecParameters *codecPar = fmtCtx->streams[audioStreamIdx]->codecpar;
	const AVCodec *codec = avcodec_find_decoder(codecPar->codec_id);
	if (!codec)
		return result;

	AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecCtx, codecPar);
	if (avcodec_open2(codecCtx, codec, nullptr) < 0)
	{
		avcodec_free_context(&codecCtx);
		return result;
	}

	SwrContext *swr = swr_alloc();
	AVChannelLayout outLayout;
	if (codecCtx->ch_layout.nb_channels >= 2)
		outLayout = AV_CHANNEL_LAYOUT_STEREO;
	else
		outLayout = AV_CHANNEL_LAYOUT_MONO;

	constexpr int outRate = XAudio2Decoder::OUTPUT_SAMPLE_RATE;
	constexpr int bytesPerSample = sizeof(float); // 4 bytes for float32

	swr_alloc_set_opts2(&swr,
		&outLayout, AV_SAMPLE_FMT_FLT, outRate,
		&codecCtx->ch_layout, codecCtx->sample_fmt, codecCtx->sample_rate,
		0, nullptr);

	// High-quality Kaiser-windowed sinc resampler
	av_opt_set_int(swr, "filter_size", 64, 0);
	swr_init(swr);

	std::vector<uint8_t> pcmData;
	AVPacket *pkt = av_packet_alloc();
	AVFrame *frame = av_frame_alloc();

	while (av_read_frame(fmtCtx, pkt) >= 0)
	{
		if (pkt->stream_index == audioStreamIdx)
		{
			if (avcodec_send_packet(codecCtx, pkt) == 0)
			{
				while (avcodec_receive_frame(codecCtx, frame) == 0)
				{
					int outSamples = swr_get_out_samples(swr, frame->nb_samples);
					int outBytes = outSamples * outLayout.nb_channels * bytesPerSample;
					size_t offset = pcmData.size();
					pcmData.resize(offset + outBytes);

					uint8_t *outBuf = pcmData.data() + offset;
					int converted = swr_convert(swr, &outBuf, outSamples,
						(const uint8_t **)frame->extended_data, frame->nb_samples);
					if (converted > 0)
						pcmData.resize(offset + converted * outLayout.nb_channels * bytesPerSample);
					else
						pcmData.resize(offset);
				}
			}
		}
		av_packet_unref(pkt);
	}

	// Flush
	avcodec_send_packet(codecCtx, nullptr);
	while (avcodec_receive_frame(codecCtx, frame) == 0)
	{
		int outSamples = swr_get_out_samples(swr, frame->nb_samples);
		int outBytes = outSamples * outLayout.nb_channels * bytesPerSample;
		size_t offset = pcmData.size();
		pcmData.resize(offset + outBytes);
		uint8_t *outBuf = pcmData.data() + offset;
		int converted = swr_convert(swr, &outBuf, outSamples,
			(const uint8_t **)frame->extended_data, frame->nb_samples);
		if (converted > 0)
			pcmData.resize(offset + converted * outLayout.nb_channels * bytesPerSample);
		else
			pcmData.resize(offset);
	}

	result.channels = outLayout.nb_channels;
	result.sampleRate = outRate;
	result.bitsPerSample = XAudio2Decoder::OUTPUT_BITS_PER_SAMPLE;
	result.sizeBytes = static_cast<uint32_t>(pcmData.size());
	if (!pcmData.empty())
	{
		result.data = static_cast<uint8_t *>(malloc(pcmData.size()));
		std::memcpy(result.data, pcmData.data(), pcmData.size());
	}

	av_frame_free(&frame);
	av_packet_free(&pkt);
	swr_free(&swr);
	avcodec_free_context(&codecCtx);

	return result;
}

XAudio2Decoder::DecodedBuffer XAudio2Decoder::decode(const char *filename, AudioFileProvider *provider)
{
	if (!filename || !filename[0])
		return {nullptr, 0, 0, 0, 0};

	{
		std::lock_guard<std::mutex> lock(s_cacheMutex);
		auto it = s_pcmCache.find(filename);
		if (it != s_pcmCache.end())
			return it->second;
	}

	if (!provider)
	{
		DEBUG_LOG(("XAudio2Decoder: No file provider set\n"));
		return {nullptr, 0, 0, 0, 0};
	}

	AudioFileProvider::FileHandle fh = provider->open(filename);
	if (!fh.isValid())
	{
		DEBUG_LOG(("Missing Audio File: '%s'\n", filename));
		return {nullptr, 0, 0, 0, 0};
	}

	ProviderIO io{provider, fh};

	AVFormatContext *fmtCtx = avformat_alloc_context();
	if (!fmtCtx)
	{
		provider->close(fh);
		return {nullptr, 0, 0, 0, 0};
	}

	constexpr size_t avioBufferSize = 0x8000;
	uint8_t *avioBuffer = static_cast<uint8_t *>(av_malloc(avioBufferSize));
	AVIOContext *avioCtx = avio_alloc_context(avioBuffer, avioBufferSize, 0, &io,
		&xaDecoderReadPacket, nullptr, &xaDecoderSeek);
	if (!avioCtx)
	{
		av_free(avioBuffer);
		avformat_free_context(fmtCtx);
		provider->close(fh);
		return {nullptr, 0, 0, 0, 0};
	}

	fmtCtx->pb = avioCtx;
	fmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

	if (avformat_open_input(&fmtCtx, nullptr, nullptr, nullptr) < 0)
	{
		av_freep(&avioCtx->buffer);
		avio_context_free(&avioCtx);
		provider->close(fh);
		return {nullptr, 0, 0, 0, 0};
	}

	DecodedBuffer result = decodeFromFormat(fmtCtx);

	avformat_close_input(&fmtCtx);
	av_freep(&avioCtx->buffer);
	avio_context_free(&avioCtx);
	provider->close(fh);

	if (result.data)
	{
		std::lock_guard<std::mutex> lock(s_cacheMutex);
		s_pcmCache[filename] = result;
	}

	return result;
}

/// In-memory I/O context for FFmpeg
struct MemoryIO
{
	const uint8_t *data;
	int64_t size;
	int64_t pos;
};

static int memReadPacket(void *opaque, uint8_t *buf, int buf_size)
{
	auto *mem = static_cast<MemoryIO *>(opaque);
	int64_t remaining = mem->size - mem->pos;
	if (remaining <= 0)
		return AVERROR_EOF;
	int toRead = static_cast<int>((buf_size < remaining) ? buf_size : remaining);
	std::memcpy(buf, mem->data + mem->pos, toRead);
	mem->pos += toRead;
	return toRead;
}

static int64_t memSeek(void *opaque, int64_t offset, int whence)
{
	auto *mem = static_cast<MemoryIO *>(opaque);
	if (whence == AVSEEK_SIZE)
		return mem->size;
	if (whence == SEEK_SET)
		mem->pos = offset;
	else if (whence == SEEK_CUR)
		mem->pos += offset;
	else if (whence == SEEK_END)
		mem->pos = mem->size + offset;
	return mem->pos;
}

XAudio2Decoder::DecodedBuffer XAudio2Decoder::decodeFromMemory(const char *filename, const std::vector<uint8_t> &fileData)
{
	if (!filename || !filename[0] || fileData.empty())
		return {nullptr, 0, 0, 0, 0};

	{
		std::lock_guard<std::mutex> lock(s_cacheMutex);
		auto it = s_pcmCache.find(filename);
		if (it != s_pcmCache.end())
			return it->second;
	}

	MemoryIO mem{fileData.data(), static_cast<int64_t>(fileData.size()), 0};

	AVFormatContext *fmtCtx = avformat_alloc_context();
	if (!fmtCtx)
		return {nullptr, 0, 0, 0, 0};

	constexpr size_t avioBufferSize = 0x8000;
	uint8_t *avioBuffer = static_cast<uint8_t *>(av_malloc(avioBufferSize));
	AVIOContext *avioCtx = avio_alloc_context(avioBuffer, avioBufferSize, 0, &mem,
		&memReadPacket, nullptr, &memSeek);
	if (!avioCtx)
	{
		av_free(avioBuffer);
		avformat_free_context(fmtCtx);
		return {nullptr, 0, 0, 0, 0};
	}

	fmtCtx->pb = avioCtx;
	fmtCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

	if (avformat_open_input(&fmtCtx, nullptr, nullptr, nullptr) < 0)
	{
		av_freep(&avioCtx->buffer);
		avio_context_free(&avioCtx);
		return {nullptr, 0, 0, 0, 0};
	}

	DecodedBuffer result = decodeFromFormat(fmtCtx);

	avformat_close_input(&fmtCtx);
	av_freep(&avioCtx->buffer);
	avio_context_free(&avioCtx);

	if (result.data)
	{
		std::lock_guard<std::mutex> lock(s_cacheMutex);
		s_pcmCache[filename] = result;
	}

	return result;
}

bool XAudio2Decoder::isCached(const char *filename)
{
	if (!filename || !filename[0])
		return false;
	std::lock_guard<std::mutex> lock(s_cacheMutex);
	return s_pcmCache.find(filename) != s_pcmCache.end();
}

XAudio2Decoder::DecodedBuffer XAudio2Decoder::tryGetCached(const char *filename)
{
	if (!filename || !filename[0])
		return {nullptr, 0, 0, 0, 0};
	std::lock_guard<std::mutex> lock(s_cacheMutex);
	auto it = s_pcmCache.find(filename);
	if (it != s_pcmCache.end())
		return it->second;
	return {nullptr, 0, 0, 0, 0};
}

float XAudio2Decoder::getDurationMs(const char *filename, AudioFileProvider *provider)
{
	DecodedBuffer buf = decode(filename, provider);
	if (!buf.data || buf.sampleRate == 0 || buf.channels == 0 || buf.bitsPerSample == 0)
		return 0.0f;

	float bytesPerSample = (buf.bitsPerSample / 8.0f) * buf.channels;
	float lengthSec = (buf.sizeBytes / bytesPerSample) / buf.sampleRate;
	return lengthSec * 1000.0f;
}

void XAudio2Decoder::clearCache()
{
	std::lock_guard<std::mutex> lock(s_cacheMutex);
	for (auto &pair : s_pcmCache)
		free(pair.second.data);
	s_pcmCache.clear();
}

#endif // _WIN32
