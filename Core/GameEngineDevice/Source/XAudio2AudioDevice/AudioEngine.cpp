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

#include "AudioEngine.h"
#include "core/AudioSystem.h"
#include "core/AudioFactory.h"
#include "core/AudioVideoStream.h"

#include <algorithm>
#include <cstring>
#include <random>

AudioEngine *AudioEngine::s_instance = nullptr;

// ── Voice helpers ───────────────────────────────────────────────

float AudioEngine::Voice::getFadeMultiplier() const
{
	if (fadeState == FadeState::None || fadeDuration <= 0.0f)
		return 1.0f;

	float t = fadeElapsed / fadeDuration;
	if (t > 1.0f) t = 1.0f;

	if (fadeState == FadeState::FadingIn)
		return t;
	else // FadingOut
		return 1.0f - t;
}

// ── Singleton ───────────────────────────────────────────────────

AudioEngine::AudioEngine()
{
	// Default bus limits
	m_buses[static_cast<int>(AudioBus::Sound)].maxVoices = 24;
	m_buses[static_cast<int>(AudioBus::Sound3D)].maxVoices = 48;
	m_buses[static_cast<int>(AudioBus::Music)].maxVoices = 2;
	m_buses[static_cast<int>(AudioBus::Speech)].maxVoices = 4;
}

AudioEngine::~AudioEngine()
{
	if (m_initialized)
		shutdown();
}

AudioEngine &AudioEngine::instance()
{
	if (!s_instance)
		s_instance = new AudioEngine();
	return *s_instance;
}

bool AudioEngine::isInitialized()
{
	return s_instance != nullptr && s_instance->m_initialized;
}

void AudioEngine::destroy()
{
	delete s_instance;
	s_instance = nullptr;
}

// ── Lifecycle ───────────────────────────────────────────────────

void AudioEngine::init(const AudioBackendConfig &config)
{
	if (m_initialized)
		return;

	m_backend = createAudioSystem(config);
	if (m_backend)
		m_backend->init();

	m_initialized = true;
}

void AudioEngine::update()
{
	if (!m_backend)
		return;

	// Use a fixed time step for frame-based updates (33ms ≈ 30fps)
	const float dt = 1.0f / 30.0f;

	processFading(dt);
	processCompletions();

	m_backend->update();
}

void AudioEngine::reset()
{
	// Stop all voices
	for (auto &voice : m_voices)
	{
		if (m_backend)
			m_backend->stopAudioEvent(voice.backendHandle);
	}
	m_voices.clear();

	m_currentMusicHandle = AUDIO_HANDLE_INVALID;
	m_currentMusicFilename.clear();

	if (m_backend)
		m_backend->reset();
}

void AudioEngine::shutdown()
{
	reset();

	if (m_backend)
	{
		m_backend->closeDevice();
		m_backend.reset();
	}

	m_initialized = false;
}

void AudioEngine::postProcessLoad()
{
	if (m_backend)
		m_backend->postProcessLoad();
}

// ── Core playback ───────────────────────────────────────────────

AudioHandle AudioEngine::play(const char *filename, AudioBus bus)
{
	AudioEvent event;
	event.filename = filename;
	event.bus = bus;
	return play(event);
}

AudioHandle AudioEngine::play3D(const char *filename, float x, float y, float z,
	float minDist, float maxDist)
{
	AudioEvent event;
	event.filename = filename;
	event.bus = AudioBus::Sound3D;
	event.is3D = true;
	event.posX = x;
	event.posY = y;
	event.posZ = z;
	event.minDistance = minDist;
	event.maxDistance = maxDist;
	return play(event);
}

AudioHandle AudioEngine::play(const AudioEvent &event)
{
	if (!m_backend || !event.filename)
		return AUDIO_HANDLE_INVALID;

	int busIdx = static_cast<int>(event.bus);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return AUDIO_HANDLE_INVALID;

	// Check bus enabled
	if (!m_buses[busIdx].enabled)
		return AUDIO_HANDLE_INVALID;

	// Music may overlap a fading track while the replacement starts.
	if (event.bus != AudioBus::Music)
		enforceVoiceLimits(event.bus, event.priority);

	// Check again after stealing — if still at limit and nothing was stolen, reject
	if (event.bus != AudioBus::Music && m_buses[busIdx].maxVoices > 0)
	{
		int count = getBusActiveVoiceCount(event.bus);
		if (count >= m_buses[busIdx].maxVoices)
			return AUDIO_HANDLE_INVALID;
	}

	// Compute initial volume: event volume × bus volume × fade
	float busVol = m_buses[busIdx].getEffectiveVolume();
	float initialVolume = event.volume * busVol;

	// Create a modified event with the computed volume for the backend
	AudioEvent backendEvent = event;
	backendEvent.volume = (event.fadeInSeconds > 0.0f) ? 0.0f : initialVolume;
	// Clamp pitch to valid range (XAudio2 requires > 0)
	if (backendEvent.pitchShift < 0.01f)
		backendEvent.pitchShift = 1.0f;
	// AudioEngine handles all fading — don't let the backend do its own fade.
	backendEvent.fadeInSeconds = 0.0f;
	backendEvent.fadeOutSeconds = 0.0f;

	AudioHandle backendHandle = m_backend->playAudioEvent(backendEvent);
	if (backendHandle == AUDIO_HANDLE_INVALID)
		return AUDIO_HANDLE_INVALID;

	Voice voice;
	voice.handle = allocateHandle();
	voice.backendHandle = backendHandle;
	voice.bus = event.bus;
	voice.priority = event.priority;
	voice.eventVolume = event.volume;
	voice.filename = event.filename ? event.filename : "";
	voice.status = Voice::Status::Playing;

	if (event.fadeInSeconds > 0.0f)
	{
		voice.fadeState = Voice::FadeState::FadingIn;
		voice.fadeDuration = event.fadeInSeconds;
		voice.fadeElapsed = 0.0f;
	}

	m_voices.push_back(voice);

	// Auto-track music handle for isMusicPlaying()/getMusicTrackName()
	if (event.bus == AudioBus::Music)
	{
		m_currentMusicHandle = voice.handle;
		m_currentMusicFilename = voice.filename;
	}

	return voice.handle;
}

void AudioEngine::stop(AudioHandle handle)
{
	Voice *voice = findVoice(handle);
	if (!voice)
		return;

	// If the voice has a fade-out configured, start fading
	// Otherwise just kill it immediately
	// Check if there's a pending fade-out (from the original event)
	// For explicit stop with fade, use stopBus or call with fade param
	kill(handle);
}

void AudioEngine::kill(AudioHandle handle)
{
	for (auto it = m_voices.begin(); it != m_voices.end(); ++it)
	{
		if (it->handle == handle)
		{
			if (m_backend)
				m_backend->stopAudioEvent(it->backendHandle);

			if (it->completionCallback)
				it->completionCallback(it->handle, it->completionUserData);

			if (handle == m_currentMusicHandle)
			{
				m_currentMusicHandle = AUDIO_HANDLE_INVALID;
				m_currentMusicFilename.clear();
			}

			m_voices.erase(it);
			return;
		}
	}
}

void AudioEngine::pause(AudioHandle handle)
{
	Voice *voice = findVoice(handle);
	if (!voice || voice->status != Voice::Status::Playing)
		return;

	if (m_backend)
		m_backend->pauseAudioEvent(voice->backendHandle);
	voice->status = Voice::Status::Paused;
}

void AudioEngine::resume(AudioHandle handle)
{
	Voice *voice = findVoice(handle);
	if (!voice || voice->status != Voice::Status::Paused)
		return;

	if (m_backend)
		m_backend->resumeAudioEvent(voice->backendHandle);
	voice->status = Voice::Status::Playing;
}

void AudioEngine::setPosition(AudioHandle handle, float x, float y, float z)
{
	Voice *voice = findVoice(handle);
	if (voice && m_backend)
		m_backend->setAudioPosition(voice->backendHandle, x, y, z);
}

void AudioEngine::setVolume(AudioHandle handle, float volume)
{
	Voice *voice = findVoice(handle);
	if (!voice)
		return;

	voice->eventVolume = volume;

	if (m_backend)
		m_backend->setAudioVolume(voice->backendHandle, computeVoiceVolume(*voice));
}

void AudioEngine::setPitch(AudioHandle handle, float pitch)
{
	Voice *voice = findVoice(handle);
	if (voice && m_backend)
		m_backend->setAudioPitch(voice->backendHandle, pitch);
}

bool AudioEngine::isPlaying(AudioHandle handle) const
{
	const Voice *voice = findVoice(handle);
	if (!voice)
		return false;
	if (voice->status == Voice::Status::Stopping)
		return false;
	if (m_backend)
		return m_backend->isAudioPlaying(voice->backendHandle);
	return false;
}

void AudioEngine::setCompletionCallback(AudioHandle handle,
	AudioCompletionCallback callback, void *userData)
{
	Voice *voice = findVoice(handle);
	if (voice)
	{
		voice->completionCallback = callback;
		voice->completionUserData = userData;
	}
}

// ── Bus control ─────────────────────────────────────────────────

void AudioEngine::setBusVolume(AudioBus bus, AudioVolumeDomain domain, float volume)
{
	int busIdx = static_cast<int>(bus);
	int domIdx = static_cast<int>(domain);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return;
	if (domIdx < 0 || domIdx > 2)
		return;

	m_buses[busIdx].volume[domIdx] = volume;

	// Update all playing voices on this bus
	for (auto &voice : m_voices)
	{
		if (voice.bus == bus && m_backend)
			m_backend->setAudioVolume(voice.backendHandle, computeVoiceVolume(voice));
	}
}

float AudioEngine::getBusVolume(AudioBus bus) const
{
	int busIdx = static_cast<int>(bus);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return 0.0f;
	return m_buses[busIdx].getEffectiveVolume();
}

float AudioEngine::getBusVolumeDomain(AudioBus bus, AudioVolumeDomain domain) const
{
	int busIdx = static_cast<int>(bus);
	int domIdx = static_cast<int>(domain);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return 0.0f;
	if (domIdx < 0 || domIdx > 2)
		return 0.0f;
	return m_buses[busIdx].volume[domIdx];
}

void AudioEngine::setBusEnabled(AudioBus bus, bool enabled)
{
	int busIdx = static_cast<int>(bus);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return;

	m_buses[busIdx].enabled = enabled;

	// If disabling, stop all voices on this bus
	if (!enabled)
		stopBus(bus);
}

bool AudioEngine::isBusEnabled(AudioBus bus) const
{
	int busIdx = static_cast<int>(bus);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return false;
	return m_buses[busIdx].enabled;
}

void AudioEngine::setBusMaxVoices(AudioBus bus, int maxVoices)
{
	int busIdx = static_cast<int>(bus);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return;
	m_buses[busIdx].maxVoices = maxVoices;
}

int AudioEngine::getBusActiveVoiceCount(AudioBus bus) const
{
	int count = 0;
	for (const auto &voice : m_voices)
	{
		if (voice.bus == bus && voice.status != Voice::Status::Stopping)
			count++;
	}
	return count;
}

// ── Bus-wide operations ─────────────────────────────────────────

void AudioEngine::stopBus(AudioBus bus, bool fade)
{
	for (auto it = m_voices.begin(); it != m_voices.end(); )
	{
		if (it->bus != bus)
		{
			++it;
			continue;
		}

		if (fade && it->fadeState != Voice::FadeState::FadingOut)
		{
			it->fadeState = Voice::FadeState::FadingOut;
			it->fadeDuration = 0.5f; // Default fade-out duration
			it->fadeElapsed = 0.0f;
			it->status = Voice::Status::Stopping;
			++it;
		}
		else
		{
			if (m_backend)
				m_backend->stopAudioEvent(it->backendHandle);

			if (it->handle == m_currentMusicHandle)
			{
				m_currentMusicHandle = AUDIO_HANDLE_INVALID;
				m_currentMusicFilename.clear();
			}

			it = m_voices.erase(it);
		}
	}
}

void AudioEngine::pauseBus(AudioBus bus)
{
	for (auto &voice : m_voices)
	{
		if (voice.bus == bus && voice.status == Voice::Status::Playing)
		{
			if (m_backend)
				m_backend->pauseAudioEvent(voice.backendHandle);
			voice.status = Voice::Status::Paused;
		}
	}
}

void AudioEngine::resumeBus(AudioBus bus)
{
	for (auto &voice : m_voices)
	{
		if (voice.bus == bus && voice.status == Voice::Status::Paused)
		{
			if (m_backend)
				m_backend->resumeAudioEvent(voice.backendHandle);
			voice.status = Voice::Status::Playing;
		}
	}
}

// ── Listener ────────────────────────────────────────────────────

void AudioEngine::setListenerPosition(float posX, float posY, float posZ,
	float fwdX, float fwdY, float fwdZ,
	float upX, float upY, float upZ)
{
	if (m_backend)
		m_backend->setListenerPosition(posX, posY, posZ, fwdX, fwdY, fwdZ, upX, upY, upZ);
}

// ── Music convenience ───────────────────────────────────────────

AudioHandle AudioEngine::playMusic(const char *filename, float fadeInSeconds)
{
	// Stop current music first
	if (m_currentMusicHandle != AUDIO_HANDLE_INVALID)
		stopMusic(0.0f);

	AudioEvent event;
	event.filename = filename;
	event.bus = AudioBus::Music;
	event.loopCount = 0; // Loop forever
	event.fadeInSeconds = fadeInSeconds;
	event.priority = 1.0f; // Music is high priority

	m_currentMusicHandle = play(event);
	m_currentMusicFilename = filename ? filename : "";
	return m_currentMusicHandle;
}

void AudioEngine::stopMusic(float fadeOutSeconds)
{
	if (m_currentMusicHandle == AUDIO_HANDLE_INVALID)
		return;

	if (fadeOutSeconds > 0.0f)
	{
		Voice *voice = findVoice(m_currentMusicHandle);
		if (voice)
		{
			voice->fadeState = Voice::FadeState::FadingOut;
			voice->fadeDuration = fadeOutSeconds;
			voice->fadeElapsed = 0.0f;
			voice->status = Voice::Status::Stopping;
		}
	}
	else
	{
		kill(m_currentMusicHandle);
	}

	// Don't clear tracking here — let processCompletions handle it
	// (fade-out voices still need to be tracked)
	if (fadeOutSeconds <= 0.0f)
	{
		m_currentMusicHandle = AUDIO_HANDLE_INVALID;
		m_currentMusicFilename.clear();
	}
}

bool AudioEngine::isMusicPlaying() const
{
	if (m_currentMusicHandle == AUDIO_HANDLE_INVALID)
		return false;
	return isPlaying(m_currentMusicHandle);
}

const char *AudioEngine::getMusicTrackName() const
{
	if (m_currentMusicHandle == AUDIO_HANDLE_INVALID)
		return "";
	return m_currentMusicFilename.c_str();
}

// ── Query ───────────────────────────────────────────────────────

float AudioEngine::getFileDurationMs(const char *filename) const
{
	if (m_backend)
		return m_backend->getFileDurationMs(filename);
	return 0.0f;
}

int AudioEngine::countPlayingByFilename(const char *filename) const
{
	if (!filename)
		return 0;
	int count = 0;
	for (const auto &voice : m_voices)
	{
		if (voice.status != Voice::Status::Stopping && voice.filename == filename)
			count++;
	}
	return count;
}

bool AudioEngine::isFilePlaying(const char *filename) const
{
	return countPlayingByFilename(filename) > 0;
}

void AudioEngine::stopByFilename(const char *filename, bool fade)
{
	if (!filename)
		return;

	for (auto it = m_voices.begin(); it != m_voices.end(); )
	{
		if (it->filename != filename)
		{
			++it;
			continue;
		}

		if (fade)
		{
			it->fadeState = Voice::FadeState::FadingOut;
			it->fadeDuration = 0.5f;
			it->fadeElapsed = 0.0f;
			it->status = Voice::Status::Stopping;
			++it;
		}
		else
		{
			if (m_backend)
				m_backend->stopAudioEvent(it->backendHandle);
			it = m_voices.erase(it);
		}
	}
}

// ── Backend access ──────────────────────────────────────────────

AudioBackendType AudioEngine::getBackendType() const
{
	if (m_backend)
		return m_backend->getBackendType();
	return AudioBackendType::Null;
}

uint32_t AudioEngine::getNum2DSamples() const
{
	return m_backend ? m_backend->getNum2DSamples() : 0;
}

uint32_t AudioEngine::getNum3DSamples() const
{
	return m_backend ? m_backend->getNum3DSamples() : 0;
}

uint32_t AudioEngine::getNumStreams() const
{
	return m_backend ? m_backend->getNumStreams() : 0;
}

AudioVideoStream *AudioEngine::createVideoStream()
{
	return m_backend ? m_backend->createVideoStream() : nullptr;
}

void AudioEngine::releaseVideoStream(AudioVideoStream *stream)
{
	if (m_backend)
		m_backend->releaseVideoStream(stream);
	else
		delete stream;
}

// ── Internal helpers ────────────────────────────────────────────

AudioHandle AudioEngine::allocateHandle()
{
	// Handles start at 1, wrap at 0x7FFFFFFF to avoid collision with
	// any game-specific handle schemes.
	if (m_nextHandle >= 0x7FFFFFFF)
		m_nextHandle = 1;
	return m_nextHandle++;
}

AudioEngine::Voice *AudioEngine::findVoice(AudioHandle handle)
{
	for (auto &voice : m_voices)
		if (voice.handle == handle)
			return &voice;
	return nullptr;
}

const AudioEngine::Voice *AudioEngine::findVoice(AudioHandle handle) const
{
	for (const auto &voice : m_voices)
		if (voice.handle == handle)
			return &voice;
	return nullptr;
}

void AudioEngine::enforceVoiceLimits(AudioBus bus, float newPriority)
{
	int busIdx = static_cast<int>(bus);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return;

	int maxVoices = m_buses[busIdx].maxVoices;
	if (maxVoices <= 0)
		return; // Unlimited

	int count = getBusActiveVoiceCount(bus);
	if (count < maxVoices)
		return; // Under limit

	// Find and kill the lowest-priority voice on this bus.
	// At equal priority, steal the oldest (first found) voice.
	auto lowestIt = m_voices.end();
	float lowestPri = newPriority + 0.001f; // Allow stealing at equal priority

	for (auto it = m_voices.begin(); it != m_voices.end(); ++it)
	{
		if (it->bus != bus || it->status == Voice::Status::Stopping)
			continue;
		if (it->priority < lowestPri)
		{
			lowestPri = it->priority;
			lowestIt = it;
		}
	}

	if (lowestIt != m_voices.end())
	{
		if (m_backend)
			m_backend->stopAudioEvent(lowestIt->backendHandle);

		if (lowestIt->handle == m_currentMusicHandle)
		{
			m_currentMusicHandle = AUDIO_HANDLE_INVALID;
			m_currentMusicFilename.clear();
		}

		m_voices.erase(lowestIt);
	}
}

void AudioEngine::processFading(float dt)
{
	for (auto &voice : m_voices)
	{
		if (voice.fadeState == Voice::FadeState::None)
			continue;

		voice.fadeElapsed += dt;

		if (voice.fadeState == Voice::FadeState::FadingIn)
		{
			if (voice.fadeElapsed >= voice.fadeDuration)
			{
				voice.fadeState = Voice::FadeState::None;
				voice.fadeElapsed = 0.0f;
			}
		}
		else if (voice.fadeState == Voice::FadeState::FadingOut)
		{
			if (voice.fadeElapsed >= voice.fadeDuration)
			{
				// Fade complete — mark for removal
				if (m_backend)
					m_backend->stopAudioEvent(voice.backendHandle);
				voice.status = Voice::Status::Stopping;
				voice.fadeState = Voice::FadeState::None;
				continue;
			}
		}

		// Apply faded volume to backend
		if (m_backend)
			m_backend->setAudioVolume(voice.backendHandle, computeVoiceVolume(voice));
	}
}

void AudioEngine::processCompletions()
{
	for (auto it = m_voices.begin(); it != m_voices.end(); )
	{
		bool finished = false;

		if (it->status == Voice::Status::Stopping &&
			it->fadeState == Voice::FadeState::None)
		{
			finished = true;
		}
		else if (it->status != Voice::Status::Paused &&
			m_backend && !m_backend->isAudioPlaying(it->backendHandle))
		{
			finished = true;
		}

		if (finished)
		{
			if (it->completionCallback)
				it->completionCallback(it->handle, it->completionUserData);

			if (it->handle == m_currentMusicHandle)
			{
				m_currentMusicHandle = AUDIO_HANDLE_INVALID;
				m_currentMusicFilename.clear();
			}

			it = m_voices.erase(it);
		}
		else
		{
			++it;
		}
	}
}

float AudioEngine::computeVoiceVolume(const Voice &voice) const
{
	int busIdx = static_cast<int>(voice.bus);
	if (busIdx < 0 || busIdx >= static_cast<int>(AudioBus::Count))
		return voice.eventVolume;

	float busVol = m_buses[busIdx].getEffectiveVolume();
	float fadeVol = voice.getFadeMultiplier();

	return voice.eventVolume * busVol * fadeVol;
}

// ── Event Registry ─────────────────────────────────────────────

void AudioEngine::registerEvent(const AudioEventDefinition &def, bool precache)
{
	m_eventRegistry[def.name] = def;

	if (precache)
		precacheEvent(def.name);
}

void AudioEngine::unregisterEvent(const std::string &name)
{
	m_eventRegistry.erase(name);
}

void AudioEngine::clearEvents()
{
	m_eventRegistry.clear();
}

const AudioEventDefinition *AudioEngine::findEvent(const std::string &name) const
{
	auto it = m_eventRegistry.find(name);
	return (it != m_eventRegistry.end()) ? &it->second : nullptr;
}

void AudioEngine::precacheEvent(const std::string &name)
{
	if (!m_backend)
		return;

	const AudioEventDefinition *def = findEvent(name);
	if (!def)
		return;

	for (const auto &filename : def->filenames)
		m_backend->precacheFile(filename.c_str());
}

void AudioEngine::precacheAll()
{
	if (!m_backend)
		return;

	for (const auto &pair : m_eventRegistry)
		for (const auto &filename : pair.second.filenames)
			m_backend->precacheFile(filename.c_str());
}

AudioHandle AudioEngine::playEvent(const std::string &name)
{
	const AudioEventDefinition *def = findEvent(name);
	if (!def || def->filenames.empty())
		return AUDIO_HANDLE_INVALID;

	AudioEvent ev;
	ev.bus = def->bus;
	ev.is3D = false;
	ev.volume = def->volume;
	ev.priority = def->priorityAsFloat();
	ev.loopCount = def->isLooping() ? 0 : def->loopCount;
	ev.minDistance = def->minDistance;
	ev.maxDistance = def->maxDistance;

	// Pitch: randomize within range
	if (def->pitchMin < def->pitchMax)
	{
		thread_local std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(def->pitchMin, def->pitchMax);
		ev.pitchShift = dist(rng);
	}
	else
	{
		ev.pitchShift = def->pitchMin;
	}

	// Volume shift: apply random ± shift
	if (def->volumeShift > 0.0f)
	{
		thread_local std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(-def->volumeShift, def->volumeShift);
		ev.volume = (std::max)(def->minVolume, ev.volume + dist(rng));
	}

	// File selection
	if (def->filenames.size() == 1)
	{
		ev.filename = def->filenames[0].c_str();
	}
	else if (def->isRandom())
	{
		thread_local std::mt19937 rng(std::random_device{}());
		std::uniform_int_distribution<size_t> dist(0, def->filenames.size() - 1);
		ev.filename = def->filenames[dist(rng)].c_str();
	}
	else
	{
		ev.filename = def->filenames[0].c_str();
	}

	return play(ev);
}

AudioHandle AudioEngine::playEvent(const std::string &name, float x, float y, float z)
{
	const AudioEventDefinition *def = findEvent(name);
	if (!def || def->filenames.empty())
		return AUDIO_HANDLE_INVALID;

	AudioEvent ev;
	ev.bus = def->bus;
	ev.is3D = def->is3D;
	ev.posX = x;
	ev.posY = y;
	ev.posZ = z;
	ev.volume = def->volume;
	ev.priority = def->priorityAsFloat();
	ev.loopCount = def->isLooping() ? 0 : def->loopCount;
	ev.minDistance = def->minDistance;
	ev.maxDistance = def->maxDistance;

	// Pitch: randomize within range
	if (def->pitchMin < def->pitchMax)
	{
		thread_local std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(def->pitchMin, def->pitchMax);
		ev.pitchShift = dist(rng);
	}
	else
	{
		ev.pitchShift = def->pitchMin;
	}

	// Volume shift
	if (def->volumeShift > 0.0f)
	{
		thread_local std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(-def->volumeShift, def->volumeShift);
		ev.volume = (std::max)(def->minVolume, ev.volume + dist(rng));
	}

	// File selection
	if (def->filenames.size() == 1)
	{
		ev.filename = def->filenames[0].c_str();
	}
	else if (def->isRandom())
	{
		thread_local std::mt19937 rng(std::random_device{}());
		std::uniform_int_distribution<size_t> dist(0, def->filenames.size() - 1);
		ev.filename = def->filenames[dist(rng)].c_str();
	}
	else
	{
		ev.filename = def->filenames[0].c_str();
	}

	return play(ev);
}

