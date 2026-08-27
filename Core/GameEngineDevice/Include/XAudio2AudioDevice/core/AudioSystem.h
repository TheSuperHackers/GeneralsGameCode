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

#include "AudioEvent.h"
#include "AudioHandle.h"
#include "AudioTypes.h"

#include <cstdint>

class AudioFileProvider;

///
/// AudioSystem — game-agnostic backend interface for audio device operations.
///
/// This is the pure device-level interface. It knows nothing about RTS event
/// pipelines, request queues, or game-specific audio types. All RTS pipeline
/// logic lives in AudioManagerBridge (the RTS adapter layer).
///
/// The interface provides:
///   - Device lifecycle (open/close/init/reset/update)
///   - Handle-based direct-play API (play/stop/pause/resume/position/volume/pitch)
///   - Bus volume control
///   - 3D listener positioning
///   - File duration queries
///   - Channel count queries
///
/// Implementations: XAudio2AudioSystem, NullAudioSystem.
///
class AudioSystem
{
public:
	virtual ~AudioSystem() = default;

	// ── File provider (injected dependency) ──────────────────
	void setFileProvider(AudioFileProvider *provider) { m_fileProvider = provider; }
	AudioFileProvider *getFileProvider() const { return m_fileProvider; }

	// ── Lifecycle ──────────────────────────────────────────────
	virtual void openDevice() = 0;
	virtual void closeDevice() = 0;
	virtual void *getDevice() = 0;

	virtual void init() = 0;
	virtual void postProcessLoad() = 0;
	virtual void reset() = 0;
	virtual void update() = 0;

	// ── Backend identity ──────────────────────────────────────
	virtual AudioBackendType getBackendType() const = 0;

	// ── Channel counts / limits ─────────────────────────────
	virtual uint32_t getNum2DSamples() const = 0;
	virtual uint32_t getNum3DSamples() const = 0;
	virtual uint32_t getNumStreams() const = 0;

	// ── Handle-based direct-play API ────────────────────────

	/// Play an audio event. Returns a handle for subsequent control.
	virtual AudioHandle playAudioEvent(const AudioEvent &event) = 0;

	/// Stop a playing audio event by handle.
	virtual void stopAudioEvent(AudioHandle handle) = 0;

	/// Pause a playing audio event.
	virtual void pauseAudioEvent(AudioHandle handle) = 0;

	/// Resume a paused audio event.
	virtual void resumeAudioEvent(AudioHandle handle) = 0;

	/// Update the 3D position of a playing audio event.
	virtual void setAudioPosition(AudioHandle handle, float x, float y, float z) = 0;

	/// Update the volume of a playing audio event (0.0 – 1.0).
	virtual void setAudioVolume(AudioHandle handle, float volume) = 0;

	/// Update the pitch of a playing audio event (1.0 = normal).
	virtual void setAudioPitch(AudioHandle handle, float pitch) = 0;

	/// Check if an audio event is still playing.
	virtual bool isAudioPlaying(AudioHandle handle) const = 0;

	/// Register a completion callback for a playing audio event.
	virtual void setCompletionCallback(AudioHandle handle,
		AudioCompletionCallback callback, void *userData) = 0;

	// ── Bus volume ──────────────────────────────────────────

	/// Set the master volume for an audio bus (0.0 – 1.0).
	virtual void setBusVolume(AudioBus bus, float volume) = 0;

	/// Get the master volume for an audio bus.
	virtual float getBusVolume(AudioBus bus) const = 0;

	/// Enable or disable an audio bus.
	virtual void setBusEnabled(AudioBus bus, bool enabled) = 0;

	/// Check if an audio bus is enabled.
	virtual bool isBusEnabled(AudioBus bus) const = 0;

	// ── Video stream ────────────────────────────────────────

	/// Create a streaming audio channel for video playback.
	/// Returns an AudioVideoStream pointer (caller must release via releaseVideoStream).
	virtual class AudioVideoStream *createVideoStream() { return nullptr; }

	/// Release a previously created video stream.
	virtual void releaseVideoStream(class AudioVideoStream *stream) { (void)stream; }

	// ── Utility ─────────────────────────────────────────────

	/// Get the duration of an audio file in milliseconds (0 if unknown).
	virtual float getFileDurationMs(const char *filename) const = 0;

	/// Pre-decode an audio file into the backend's cache without playing it.
	/// Called during level load to ensure all event files are ready for instant playback.
	virtual void precacheFile(const char *filename) { (void)filename; }

	// ── Listener ─────────────────────────────────────────────

	/// Set the 3D listener position and orientation.
	virtual void setListenerPosition(float posX, float posY, float posZ,
		float fwdX, float fwdY, float fwdZ,
		float upX, float upY, float upZ) = 0;

protected:
	AudioFileProvider *m_fileProvider = nullptr;
};

