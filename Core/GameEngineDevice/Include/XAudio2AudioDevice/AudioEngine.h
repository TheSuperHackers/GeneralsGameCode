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

///
/// AudioEngine — the unified audio engine for all games.
///
/// This is the single authoritative audio system. It handles:
///   - Voice management (limits per bus, priority-based voice stealing)
///   - Bus volume hierarchy (System × Script × User per bus)
///   - Playing instance tracking
///   - Fade in / fade out
///   - 3D listener positioning (forwarded to backend)
///   - Completion callbacks
///   - Music track management
///
/// Game code calls AudioEngine directly. Game-specific adapters (e.g. the RTS
/// AudioManagerBridge) translate their types into AudioEngine calls.
///
/// AudioEngine owns the backend (AudioSystem) and delegates all device-level
/// work to it. The backend handles 3D spatialization, source pools, decoding.
/// AudioEngine handles scheduling decisions.
///
/// Zero game-specific dependencies. No RTS types, no Renegade types.
///

#include "core/AudioEvent.h"
#include "core/AudioEventDefinition.h"
#include "core/AudioHandle.h"
#include "core/AudioBus.h"
#include "core/AudioTypes.h"

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class AudioSystem;
class AudioFileProvider;
struct AudioBackendConfig;

class AudioEngine
{
public:
	AudioEngine();
	~AudioEngine();

	AudioEngine(const AudioEngine &) = delete;
	AudioEngine &operator=(const AudioEngine &) = delete;

	/// Singleton access. Created on first call.
	static AudioEngine &instance();
	static bool isInitialized();
	static void destroy();

	// ── Lifecycle ───────────────────────────────────────────────

	/// Initialize with a backend configuration.
	void init(const AudioBackendConfig &config);

	/// Per-frame update. Processes fading, cleans up finished voices,
	/// forwards listener to backend.
	void update();

	/// Reset all playing audio (e.g. on level change).
	void reset();

	/// Shut down the engine and release the backend.
	void shutdown();

	/// Post-load hook (backend-specific resource setup).
	void postProcessLoad();

	// ── Core playback API ───────────────────────────────────────

	/// Play an audio event. Returns a handle for subsequent control.
	/// Respects bus limits and priority-based voice stealing.
	AudioHandle play(const AudioEvent &event);

	/// Play a simple 2D sound by filename.
	AudioHandle play(const char *filename, AudioBus bus = AudioBus::Sound);

	/// Play a 3D-positioned sound by filename.
	AudioHandle play3D(const char *filename, float x, float y, float z,
		float minDist = 1.0f, float maxDist = 300.0f);

	// ── Event registry API ──────────────────────────────────────

	/// Register a named audio event definition. Overwrites any existing
	/// definition with the same name. Optionally pre-caches all files.
	void registerEvent(const AudioEventDefinition &def, bool precache = false);

	/// Unregister a named audio event definition.
	void unregisterEvent(const std::string &name);

	/// Clear all registered event definitions (e.g. on level change).
	void clearEvents();

	/// Play a registered event by name (2D).
	AudioHandle playEvent(const std::string &name);

	/// Play a registered event by name at a 3D position.
	AudioHandle playEvent(const std::string &name, float x, float y, float z);

	/// Pre-cache (decode) all files for a registered event.
	void precacheEvent(const std::string &name);

	/// Pre-cache all files for all registered events.
	void precacheAll();

	/// Look up a registered event definition by name (nullptr if not found).
	const AudioEventDefinition *findEvent(const std::string &name) const;

	/// Stop a playing sound. If the event has fadeOutSeconds > 0,
	/// it will fade out before stopping.
	void stop(AudioHandle handle);

	/// Immediately kill a sound with no fade.
	void kill(AudioHandle handle);

	/// Pause a playing sound.
	void pause(AudioHandle handle);

	/// Resume a paused sound.
	void resume(AudioHandle handle);

	/// Update the 3D position of a playing sound.
	void setPosition(AudioHandle handle, float x, float y, float z);

	/// Update the volume of a playing sound (0.0 – 1.0).
	void setVolume(AudioHandle handle, float volume);

	/// Update the pitch of a playing sound (1.0 = normal).
	void setPitch(AudioHandle handle, float pitch);

	/// Check if a sound is still actively playing.
	bool isPlaying(AudioHandle handle) const;

	/// Set a completion callback for a playing sound.
	void setCompletionCallback(AudioHandle handle,
		AudioCompletionCallback callback, void *userData);

	// ── Bus control ─────────────────────────────────────────────

	/// Set bus volume for a specific domain.
	void setBusVolume(AudioBus bus, AudioVolumeDomain domain, float volume);

	/// Convenience: set bus volume for the Final (user) domain.
	void setBusVolume(AudioBus bus, float volume)
	{
		setBusVolume(bus, AudioVolumeDomain::Final, volume);
	}

	/// Get effective bus volume (product of all domains).
	float getBusVolume(AudioBus bus) const;

	/// Get bus volume for a specific domain.
	float getBusVolumeDomain(AudioBus bus, AudioVolumeDomain domain) const;

	/// Enable or disable a bus. Disabled buses reject new play requests
	/// and stop all currently playing voices on that bus.
	void setBusEnabled(AudioBus bus, bool enabled);

	/// Check if a bus is enabled.
	bool isBusEnabled(AudioBus bus) const;

	/// Set the maximum number of concurrent voices on a bus.
	/// 0 = unlimited. Default per bus: Sound=24, Sound3D=48, Music=2, Speech=4.
	void setBusMaxVoices(AudioBus bus, int maxVoices);

	/// Get current count of active voices on a bus.
	int getBusActiveVoiceCount(AudioBus bus) const;

	// ── Bus-wide operations ─────────────────────────────────────

	/// Stop all voices on a bus (with optional fade).
	void stopBus(AudioBus bus, bool fade = false);

	/// Pause all voices on a bus.
	void pauseBus(AudioBus bus);

	/// Resume all paused voices on a bus.
	void resumeBus(AudioBus bus);

	// ── Listener ────────────────────────────────────────────────

	/// Set the 3D listener position and orientation.
	void setListenerPosition(float posX, float posY, float posZ,
		float fwdX, float fwdY, float fwdZ,
		float upX, float upY, float upZ);

	// ── Music convenience ───────────────────────────────────────

	/// Play a music track (stops current music first).
	AudioHandle playMusic(const char *filename, float fadeInSeconds = 0.0f);

	/// Stop current music.
	void stopMusic(float fadeOutSeconds = 0.0f);

	/// Check if music is playing.
	bool isMusicPlaying() const;

	/// Get name of the currently playing music track.
	const char *getMusicTrackName() const;

	// ── Query ───────────────────────────────────────────────────

	/// Get the duration of an audio file in milliseconds.
	float getFileDurationMs(const char *filename) const;

	/// Count playing voices that match a filename.
	int countPlayingByFilename(const char *filename) const;

	/// Check if any voice with the given filename is playing.
	bool isFilePlaying(const char *filename) const;

	/// Stop all voices playing a specific filename.
	void stopByFilename(const char *filename, bool fade = false);

	// ── Backend access ──────────────────────────────────────────

	/// Get the underlying backend (for game-specific adapters that need
	/// device-level info like channel counts).
	AudioSystem *getBackend() const { return m_backend.get(); }

	/// Get backend type.
	AudioBackendType getBackendType() const;

	/// Channel counts (delegated to backend).
	uint32_t getNum2DSamples() const;
	uint32_t getNum3DSamples() const;
	uint32_t getNumStreams() const;

	/// Create a video audio stream (delegated to backend).
	class AudioVideoStream *createVideoStream();

	/// Release a video audio stream (delegated to backend).
	void releaseVideoStream(class AudioVideoStream *stream);

private:
	static AudioEngine *s_instance;

	/// A voice currently managed by the engine.
	struct Voice
	{
		AudioHandle handle = AUDIO_HANDLE_INVALID;
		AudioHandle backendHandle = AUDIO_HANDLE_INVALID;
		AudioBus bus = AudioBus::Sound;
		float priority = 0.5f;
		float eventVolume = 1.0f;
		std::string filename;

		// Fade state
		enum class FadeState { None, FadingIn, FadingOut };
		FadeState fadeState = FadeState::None;
		float fadeDuration = 0.0f;
		float fadeElapsed = 0.0f;

		// Status
		enum class Status { Playing, Paused, Stopping };
		Status status = Status::Playing;

		// Completion callback
		AudioCompletionCallback completionCallback = nullptr;
		void *completionUserData = nullptr;

		/// Current fade multiplier (1.0 if not fading).
		float getFadeMultiplier() const;
	};

	/// Per-bus state.
	struct BusState
	{
		float volume[3] = {1.0f, 1.0f, 1.0f}; // System, Script, User/Final
		bool enabled = true;
		int maxVoices = 0; // 0 = unlimited

		float getEffectiveVolume() const
		{
			return volume[0] * volume[1] * volume[2];
		}
	};

	AudioHandle allocateHandle();
	Voice *findVoice(AudioHandle handle);
	const Voice *findVoice(AudioHandle handle) const;

	/// Enforce voice limits on a bus, stealing lowest-priority voices.
	void enforceVoiceLimits(AudioBus bus, float newPriority);

	/// Update fading voices and clean up finished ones.
	void processFading(float dt);
	void processCompletions();

	/// Compute the final volume for a voice.
	float computeVoiceVolume(const Voice &voice) const;

	std::unique_ptr<AudioSystem> m_backend;

	std::list<Voice> m_voices;

	BusState m_buses[static_cast<int>(AudioBus::Count)];

	AudioHandle m_nextHandle = 1;

	// Music tracking
	AudioHandle m_currentMusicHandle = AUDIO_HANDLE_INVALID;
	std::string m_currentMusicFilename;

	// Event registry
	std::unordered_map<std::string, AudioEventDefinition> m_eventRegistry;

	// Update timing
	float m_lastUpdateTime = 0.0f;
	bool m_initialized = false;
};

/// Global convenience accessor.
inline AudioEngine &theAudio() { return AudioEngine::instance(); }


