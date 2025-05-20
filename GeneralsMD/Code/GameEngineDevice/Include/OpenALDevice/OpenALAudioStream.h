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

// FILE: OpenALAudioStream.h //////////////////////////////////////////////////////////////////////////
// OpenALAudioStream implementation
// Author: Stephan Vedder, May 2025
#pragma once

#include "always.h"
#include <AL/al.h>
#include <stdint.h>
#include <functional>

#define AL_STREAM_BUFFER_COUNT 32

class OpenALAudioStream
{
public:
		OpenALAudioStream();
		virtual ~OpenALAudioStream();

		void setRequireDataCallback(std::function<void()> callback) { m_requireDataCallback = callback; }
		ALuint getSource() const { return m_source; }

		bool bufferData(uint8_t *data, size_t data_size, ALenum format, int samplerate);
		bool isPlaying();
		void update();
		void reset();

		void play() { alSourcePlay(m_source); }
		void pause() { alSourcePause(m_source); }
		void stop() { alSourceStop(m_source); }

		void setVolume(float vol) { alSourcef(m_source, AL_GAIN, vol); }

protected:
		std::function<void()> m_requireDataCallback = nullptr;
		ALuint m_source = 0;
		ALuint m_buffers[AL_STREAM_BUFFER_COUNT] = {};
		unsigned int m_currentBufferIndex = 0;
};