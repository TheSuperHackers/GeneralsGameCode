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

#pragma once

#include "AudioVideoStream.h"

#include <cstdint>
#include <queue>
#include <vector>

struct IXAudio2;
struct IXAudio2SourceVoice;
struct IXAudio2SubmixVoice;

/// XAudio2 implementation of AudioVideoStream.
/// Uses a streaming source voice with ring-buffer queuing.
class XAudio2VideoStream : public AudioVideoStream
{
public:
	explicit XAudio2VideoStream(IXAudio2 *xaudio, IXAudio2SubmixVoice *speechSubmix);
	~XAudio2VideoStream() override;

	bool init() override;
	void reset() override;
	void queueBuffer(const void *data, int dataSize, int sampleRate, int channels, int bitsPerSample) override;
	void play() override;
	void stop() override;
	void update() override;
	bool isPlaying() const override;

private:
	static constexpr int NUM_BUFFERS = 8;

	bool ensureVoice(int sampleRate, int channels, int bitsPerSample);
	void destroyVoice();

	IXAudio2 *m_xaudio;
	IXAudio2SubmixVoice *m_speechSubmix;
	IXAudio2SourceVoice *m_sourceVoice;

	// Ring buffer management — we keep copies of queued PCM data
	struct BufferEntry {
		std::vector<uint8_t> data;
	};
	std::queue<BufferEntry> m_pendingBuffers;

	int m_currentSampleRate;
	int m_currentChannels;
	int m_currentBitsPerSample;
	bool m_playing;
	bool m_initialized;
	bool m_stopped;
};

