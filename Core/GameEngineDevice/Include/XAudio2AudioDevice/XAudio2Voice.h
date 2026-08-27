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

#include <cstdint>

struct IXAudio2;
struct IXAudio2SourceVoice;
struct IXAudio2SubmixVoice;
struct X3DAUDIO_EMITTER;
struct X3DAUDIO_LISTENER;
struct X3DAUDIO_DSP_SETTINGS;

/// Wraps a single XAudio2 source voice for 2D, 3D, or streaming playback.
class XAudio2Voice
{
public:
	enum VoiceType
	{
		VOICE_2D,
		VOICE_3D,
		VOICE_STREAM
	};

	XAudio2Voice();
	~XAudio2Voice();

	bool create(IXAudio2 *xaudio, uint32_t channels, uint32_t sampleRate,
		IXAudio2SubmixVoice *submix, VoiceType type);
	void destroy();

	void play();
	void stop();
	void pause();
	void resume();

	void setVolume(float volume);
	void setPitch(float pitch);
	void setLowPass(float frequency);
	void setLooping(bool loop);
	void setPosition(float x, float y, float z);

	// Submit PCM data to the source voice
	bool submitBuffer(const uint8_t *data, uint32_t sizeBytes, bool endOfStream = false);

	// Apply 3D calculations (minDistance = full volume range, maxDistance = hard cutoff)
	void apply3D(const void *x3dInstance, const void *listener, const void *emitter, uint32_t dstChannels, float minDistance = 1.0f);

	bool isPlaying() const;
	bool isStopped() const;

	VoiceType getType() const { return m_type; }
	uint32_t getChannels() const { return m_channels; }
	IXAudio2SourceVoice *getSourceVoice() const { return m_sourceVoice; }

private:
	IXAudio2SourceVoice *m_sourceVoice;
	IXAudio2SubmixVoice *m_submixVoice;
	VoiceType m_type;
	uint32_t m_channels;
	bool m_playing;
	bool m_looping;
	float m_position[3];
	float m_prevPosition[3];
	float m_velocity[3];
	bool m_hasLastPosition;
};
