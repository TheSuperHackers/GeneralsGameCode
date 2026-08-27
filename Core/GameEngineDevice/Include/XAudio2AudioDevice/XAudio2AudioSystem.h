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

#include "AudioSystem.h"
#include "XAudio2Mastering.h"

#include <cstdint>
#include <list>
#include <vector>
#include <string>

struct IXAudio2;
class XAudio2Voice;

/// XAudio2 implementation of AudioSystem (Windows only).
/// Uses XAudio2 for mixing/playback and X3DAudio for 3D spatialization.
/// Audio decoding runs on a dedicated background thread to avoid main-thread stalls.
class XAudio2AudioSystem : public AudioSystem
{
public:
	XAudio2AudioSystem();
	~XAudio2AudioSystem() override;

	void openDevice() override;
	void closeDevice() override;
	void *getDevice() override;

	void init() override;
	void postProcessLoad() override;
	void reset() override;
	void update() override;

	uint32_t getNum2DSamples() const override;
	uint32_t getNum3DSamples() const override;
	uint32_t getNumStreams() const override;

	AudioBackendType getBackendType() const override { return AudioBackendType::XAudio2; }

	// ── Game-agnostic direct-play API ────────────────────────
	AudioHandle playAudioEvent(const AudioEvent &event) override;
	void stopAudioEvent(AudioHandle handle) override;
	void pauseAudioEvent(AudioHandle handle) override;
	void resumeAudioEvent(AudioHandle handle) override;
	void setAudioPosition(AudioHandle handle, float x, float y, float z) override;
	void setAudioVolume(AudioHandle handle, float volume) override;
	void setAudioPitch(AudioHandle handle, float pitch) override;
	bool isAudioPlaying(AudioHandle handle) const override;
	void setCompletionCallback(AudioHandle handle,
		AudioCompletionCallback callback, void *userData) override;
	void setBusVolume(AudioBus bus, float volume) override;
	float getBusVolume(AudioBus bus) const override;
	void setBusEnabled(AudioBus bus, bool enabled) override;
	bool isBusEnabled(AudioBus bus) const override;
	float getFileDurationMs(const char *filename) const override;
	void setListenerPosition(float posX, float posY, float posZ,
		float fwdX, float fwdY, float fwdZ,
		float upX, float upY, float upZ) override;

	AudioVideoStream *createVideoStream() override;
	void releaseVideoStream(AudioVideoStream *stream) override;

	void precacheFile(const char *filename) override;

private:
	IXAudio2 *m_xaudio;
	XAudio2Mastering m_mastering;

	uint8_t m_x3dInstance[20]; // X3DAUDIO_HANDLE is a byte array
	bool m_x3dInitialized;
	bool m_comInitialized;

	std::vector<XAudio2Voice *> m_voicePool2D;
	std::vector<XAudio2Voice *> m_voicePool3D;

	// ── Direct-play API state ────────────────────────────────
	struct DirectPlayingAudio
	{
		AudioHandle handle;
		XAudio2Voice *voice;
		const uint8_t *pcmData;
		uint32_t pcmSize;
		AudioBus bus;
		float volume;
		int loopsRemaining;      // <0 = infinite, 0 = done after current play
		float fadeInTime;
		float fadeOutTime;
		float fadeElapsed;
		float maxDistance;        // For 3D distance curve scaling
		float minDistance;        // Distance at which volume is 100%
		bool fadingOut;
		bool paused;
		AudioCompletionCallback completionCb;
		void *completionUserData;
		std::vector<uint8_t> monoDownmix; // Owned mono PCM when stereo was downmixed for 3D
	};

	XAudio2Voice *acquireDirectVoice(AudioBus bus, bool is3D, uint32_t channels, uint32_t sampleRate);
	void releaseDirectVoice(XAudio2Voice *voice);
	void updateDirectPlayingSounds();

	/// Finalize a decoded buffer into an active playing sound.
	/// Shared by immediate play and deferred pending-play completion.
	AudioHandle finalizePlay(const AudioEvent &event, AudioHandle preAssignedHandle = AUDIO_HANDLE_INVALID);

	uint32_t m_nextDirectHandle;
	std::list<DirectPlayingAudio> m_directPlayingSounds;
	float m_busVolume[static_cast<int>(AudioBus::Count)];
	bool m_busEnabled[static_cast<int>(AudioBus::Count)];

	// X3DAudio listener state for direct-play 3D
	float m_listenerPos[3];
	float m_listenerFwd[3];
	float m_listenerUp[3];
};


