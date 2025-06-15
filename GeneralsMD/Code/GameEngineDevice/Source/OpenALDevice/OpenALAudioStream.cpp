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

// FILE: OpenALAudioStream.cpp //////////////////////////////////////////////////////////////////////////
// OpenALAudioStream implementation
// Author: Stephan Vedder, May 2025
#include "OpenALDevice/OpenALAudioStream.h"
#include "OpenALDevice/OpenALAudioManager.h"

OpenALAudioStream::OpenALAudioStream()
{ 
		alGenSources(1, &m_source);
		alGenBuffers(AL_STREAM_BUFFER_COUNT, m_buffers);

		// Make stream ignore positioning
		alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);

		DEBUG_LOG(("OpenALAudioStream created: %i\n", m_source));
}

OpenALAudioStream::~OpenALAudioStream()
{
		DEBUG_LOG(("OpenALAudioStream freed: %i\n", m_source));
		// Unbind the buffers first
		alSourceStop(m_source);
		alSourcei(m_source, AL_BUFFER, 0);
		alDeleteSources(1, &m_source);
		// Now delete the buffers
		alDeleteBuffers(AL_STREAM_BUFFER_COUNT, m_buffers);
}

bool OpenALAudioStream::bufferData(uint8_t *data, size_t data_size, ALenum format, int samplerate)
{
		DEBUG_LOG(("Buffering %zu bytes of data (samplerate: %i, format: %i)\n", data_size, samplerate, format));
		ALint numQueued;
		alGetSourcei(m_source, AL_BUFFERS_QUEUED, &numQueued);
		if (numQueued >= AL_STREAM_BUFFER_COUNT) {
				DEBUG_LOG(("Having too many buffers already queued: %i", numQueued));
				return false;
		}

		ALuint &currentBuffer = m_buffers[m_currentBufferIndex];
		alBufferData(currentBuffer, format, data, data_size, samplerate);
		alSourceQueueBuffers(m_source, 1, &currentBuffer);
		m_currentBufferIndex++;

		if (m_currentBufferIndex >= AL_STREAM_BUFFER_COUNT)
				m_currentBufferIndex = 0;

		return true;
}

void OpenALAudioStream::update()
{
		ALint sourceState;
		alGetSourcei(m_source, AL_SOURCE_STATE, &sourceState);

		ALint processed;
		alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);
		DEBUG_LOG(("%i buffers have been processed\n", processed));
		while (processed > 0) {
				ALuint buffer;
				alSourceUnqueueBuffers(m_source, 1, &buffer);
				processed--;
		}

		ALint numQueued;
		alGetSourcei(m_source, AL_BUFFERS_QUEUED, &numQueued);
		DEBUG_LOG(("Having %i buffers queued\n", numQueued));
		if (numQueued < AL_STREAM_BUFFER_COUNT && m_requireDataCallback) {
				// Ask for more data to be buffered
				while (numQueued < AL_STREAM_BUFFER_COUNT) {
						m_requireDataCallback();
					numQueued++;
				}
		}

		if (sourceState == AL_STOPPED) {
			 play();
		}
}

void OpenALAudioStream::reset()
{
		DEBUG_LOG(("Resetting stream\n"));
		alSourceRewind(m_source);
		alSourcei(m_source, AL_BUFFER, 0);
		m_currentBufferIndex = 0;
}

bool OpenALAudioStream::isPlaying()
{
		ALint state;
		alGetSourcei(m_source, AL_SOURCE_STATE, &state);
		return state == AL_PLAYING;
}