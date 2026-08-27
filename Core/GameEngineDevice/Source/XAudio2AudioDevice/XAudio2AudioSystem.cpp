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
#include "XAudio2AudioSystem.h"
#include "XAudio2Voice.h"
#include "XAudio2Decoder.h"
#include "AudioFileProvider.h"

#include <xaudio2.h>
#include <x3daudio.h>
#include <objbase.h>
#include <cstring>

#pragma comment(lib, "xaudio2.lib")

static constexpr uint32_t NUM_XAUDIO2_2D_VOICES = 32;
static constexpr uint32_t NUM_XAUDIO2_3D_VOICES = 128;

// ─── Construction / Destruction ─────────────────────────────────

XAudio2AudioSystem::XAudio2AudioSystem()
	: m_xaudio(nullptr)
	, m_x3dInitialized(false)
	, m_comInitialized(false)
	, m_nextDirectHandle(0x80000000)
{
	std::memset(m_x3dInstance, 0, sizeof(m_x3dInstance));
	std::memset(m_listenerPos, 0, sizeof(m_listenerPos));
	m_listenerFwd[0] = 0.0f; m_listenerFwd[1] = 0.0f; m_listenerFwd[2] = 1.0f;
	m_listenerUp[0] = 0.0f; m_listenerUp[1] = 1.0f; m_listenerUp[2] = 0.0f;
	for (int i = 0; i < static_cast<int>(AudioBus::Count); ++i)
	{
		m_busVolume[i] = 1.0f;
		m_busEnabled[i] = true;
	}
}

XAudio2AudioSystem::~XAudio2AudioSystem()
{
	closeDevice();
}

// ─── Lifecycle ──────────────────────────────────────────────────

void XAudio2AudioSystem::openDevice()
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	m_comInitialized = SUCCEEDED(hr) || hr == S_FALSE;

	// Use 1024-sample quantum (~21ms at 48kHz) for lower latency
	hr = XAudio2Create(&m_xaudio, XAUDIO2_1024_QUANTUM, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr))
	{
		m_xaudio = nullptr;
		return;
	}

	if (!m_mastering.init(m_xaudio))
	{
		m_xaudio->Release();
		m_xaudio = nullptr;
		return;
	}


	// Initialize X3DAudio with the actual output channel mask
	DWORD channelMask = m_mastering.getChannelMask();
	if (channelMask == 0)
		channelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
	X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND,
		reinterpret_cast<X3DAUDIO_HANDLE &>(m_x3dInstance));
	m_x3dInitialized = true;
}

void XAudio2AudioSystem::closeDevice()
{
	// Clean up direct-play sounds
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.voice)
		{
			dpa.voice->stop();
			releaseDirectVoice(dpa.voice);
		}
	}
	m_directPlayingSounds.clear();

	// Destroy voice pools
	for (auto *v : m_voicePool2D)
		delete v;
	m_voicePool2D.clear();
	for (auto *v : m_voicePool3D)
		delete v;
	m_voicePool3D.clear();

	XAudio2Decoder::clearCache();

	m_mastering.shutdown();

	if (m_xaudio)
	{
		m_xaudio->Release();
		m_xaudio = nullptr;
	}

	if (m_comInitialized)
	{
		CoUninitialize();
		m_comInitialized = false;
	}
}

void *XAudio2AudioSystem::getDevice()
{
	return m_xaudio;
}

void XAudio2AudioSystem::init() {}
void XAudio2AudioSystem::postProcessLoad() {}
void XAudio2AudioSystem::reset() {}

void XAudio2AudioSystem::update()
{
	updateDirectPlayingSounds();
}

uint32_t XAudio2AudioSystem::getNum2DSamples() const { return NUM_XAUDIO2_2D_VOICES; }
uint32_t XAudio2AudioSystem::getNum3DSamples() const { return NUM_XAUDIO2_3D_VOICES; }
uint32_t XAudio2AudioSystem::getNumStreams() const
{
	return 0;
}

// ─── Game-Agnostic Direct-Play API (XAudio2) ────────────────────

static XAudio2Mastering::Bus audioBusToMasteringBus(AudioBus bus, bool is3D)
{
	switch (bus)
	{
	case AudioBus::Music:  return XAudio2Mastering::BUS_MUSIC;
	case AudioBus::Speech: return XAudio2Mastering::BUS_SPEECH;
	default:               return is3D ? XAudio2Mastering::BUS_SOUND3D : XAudio2Mastering::BUS_SOUND;
	}
}

XAudio2Voice *XAudio2AudioSystem::acquireDirectVoice(AudioBus bus, bool is3D, uint32_t channels, uint32_t sampleRate)
{
	if (!m_xaudio)
		return nullptr;

	auto vt = is3D ? XAudio2Voice::VOICE_3D : XAudio2Voice::VOICE_2D;
	auto masterBus = audioBusToMasteringBus(bus, is3D);

	auto *voice = new XAudio2Voice();
	if (!voice->create(m_xaudio, channels, sampleRate, m_mastering.getSubmixVoice(masterBus), vt))
	{
		delete voice;
		return nullptr;
	}
	return voice;
}

void XAudio2AudioSystem::releaseDirectVoice(XAudio2Voice *voice)
{
	if (voice)
	{
		voice->destroy();
		delete voice;
	}
}

AudioHandle XAudio2AudioSystem::playAudioEvent(const AudioEvent &event)
{
	if (!m_xaudio || !event.filename || event.filename[0] == '\0')
		return AUDIO_HANDLE_INVALID;

	if (!m_busEnabled[static_cast<int>(event.bus)])
		return AUDIO_HANDLE_INVALID;

	// Fast path: file already decoded — play immediately
	if (XAudio2Decoder::isCached(event.filename))
		return finalizePlay(event);

	// Slow path: pre-read file then decode synchronously from memory
	if (m_fileProvider)
	{
	AudioFileProvider::FileHandle fh = m_fileProvider->open(event.filename);
		if (!fh.isValid())
		{
			return AUDIO_HANDLE_INVALID;
		}

		int64_t sz = m_fileProvider->size(fh);
		if (sz > 0)
		{
			std::vector<uint8_t> fileData(static_cast<size_t>(sz));
			m_fileProvider->read(fh, fileData.data(), static_cast<int>(sz));
			m_fileProvider->close(fh);

			XAudio2Decoder::DecodedBuffer decoded =
				XAudio2Decoder::decodeFromMemory(event.filename, fileData);
			if (!decoded.data)
			{
				return AUDIO_HANDLE_INVALID;
			}
			return finalizePlay(event);
		}
		m_fileProvider->close(fh);
	}

	return AUDIO_HANDLE_INVALID;
}

void XAudio2AudioSystem::precacheFile(const char *filename)
{
	if (!filename || filename[0] == '\0')
		return;

	if (XAudio2Decoder::isCached(filename))
		return;

	if (!m_fileProvider)
		return;

	AudioFileProvider::FileHandle fh = m_fileProvider->open(filename);
	if (!fh.isValid())
		return;

	int64_t sz = m_fileProvider->size(fh);
	if (sz > 0)
	{
		std::vector<uint8_t> fileData(static_cast<size_t>(sz));
		m_fileProvider->read(fh, fileData.data(), static_cast<int>(sz));
		m_fileProvider->close(fh);
		XAudio2Decoder::decodeFromMemory(filename, fileData);
	}
	else
	{
		m_fileProvider->close(fh);
	}
}

AudioHandle XAudio2AudioSystem::finalizePlay(const AudioEvent &event, AudioHandle preAssignedHandle)
{
	XAudio2Decoder::DecodedBuffer pcm = XAudio2Decoder::tryGetCached(event.filename);
	if (!pcm.data)
		return AUDIO_HANDLE_INVALID;

	// Downmix stereo to mono for 3D sounds (float32)
	std::vector<uint8_t> monoData;
	if (event.is3D && pcm.channels == 2)
	{
		uint32_t sampleCount = pcm.sizeBytes / (2 * sizeof(float));
		monoData.resize(sampleCount * sizeof(float));
		const float *src = reinterpret_cast<const float *>(pcm.data);
		float *dst = reinterpret_cast<float *>(monoData.data());
		for (uint32_t i = 0; i < sampleCount; ++i)
			dst[i] = (src[i * 2] + src[i * 2 + 1]) * 0.5f;
		pcm.data = monoData.data();
		pcm.sizeBytes = sampleCount * sizeof(float);
		pcm.channels = 1;
	}

	XAudio2Voice *voice = acquireDirectVoice(event.bus, event.is3D, pcm.channels, pcm.sampleRate);
	if (!voice)
		return AUDIO_HANDLE_INVALID;

	float startVol = (event.fadeInSeconds > 0.0f) ? 0.0f : event.volume;
	voice->setVolume(startVol);
	voice->setPitch(event.pitchShift);
	voice->setLowPass(event.lowPassFreq);

	bool infiniteLoop = (event.loopCount == 0);
	voice->setLooping(infiniteLoop);

	if (event.is3D)
	{
		voice->setPosition(event.posX, event.posY, event.posZ);

		if (m_x3dInitialized)
		{
			X3DAUDIO_LISTENER listener = {};
			listener.Position = {m_listenerPos[0], m_listenerPos[1], m_listenerPos[2]};
			listener.OrientFront = {m_listenerFwd[0], m_listenerFwd[1], m_listenerFwd[2]};
			listener.OrientTop = {m_listenerUp[0], m_listenerUp[1], m_listenerUp[2]};

			X3DAUDIO_EMITTER emitter = {};
			emitter.ChannelCount = 1;
			float maxDist = (event.maxDistance > 0.0f) ? event.maxDistance : 300.0f;
			float minDist = (event.minDistance > 0.0f) ? event.minDistance : 1.0f;
			emitter.CurveDistanceScaler = maxDist;
			voice->apply3D(m_x3dInstance, &listener, &emitter, m_mastering.getChannelCount(), minDist);
		}
	}

	if (!voice->submitBuffer(pcm.data, pcm.sizeBytes, true))
	{
		releaseDirectVoice(voice);
		return AUDIO_HANDLE_INVALID;
	}
	voice->play();

	AudioHandle handle = preAssignedHandle;
	if (handle == AUDIO_HANDLE_INVALID)
	{
		handle = m_nextDirectHandle++;
		if (m_nextDirectHandle == 0)
			m_nextDirectHandle = 0x80000000;
	}

	DirectPlayingAudio dpa{};
	dpa.handle = handle;
	dpa.voice = voice;
	dpa.monoDownmix = std::move(monoData);
	dpa.bus = event.bus;
	dpa.volume = event.volume;
	dpa.loopsRemaining = infiniteLoop ? -1 : event.loopCount - 1;
	dpa.fadeInTime = event.fadeInSeconds;
	dpa.fadeOutTime = event.fadeOutSeconds;
	dpa.fadeElapsed = 0.0f;
	dpa.maxDistance = (event.maxDistance > 0.0f) ? event.maxDistance : 300.0f;
	dpa.minDistance = (event.minDistance > 0.0f) ? event.minDistance : 1.0f;
	dpa.fadingOut = false;
	dpa.paused = false;
	dpa.completionCb = nullptr;
	dpa.completionUserData = nullptr;
	dpa.pcmData = dpa.monoDownmix.empty() ? pcm.data : dpa.monoDownmix.data();
	dpa.pcmSize = pcm.sizeBytes;
	m_directPlayingSounds.push_back(std::move(dpa));

	return handle;
}

void XAudio2AudioSystem::stopAudioEvent(AudioHandle handle)
{
	for (auto it = m_directPlayingSounds.begin(); it != m_directPlayingSounds.end(); ++it)
	{
		if (it->handle == handle)
		{
			if (it->fadeOutTime > 0.0f && !it->fadingOut)
			{
				it->fadingOut = true;
				it->fadeElapsed = 0.0f;
				return;
			}
			it->voice->stop();
			releaseDirectVoice(it->voice);
			m_directPlayingSounds.erase(it);
			return;
		}
	}
}

void XAudio2AudioSystem::pauseAudioEvent(AudioHandle handle)
{
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.handle == handle)
		{
			dpa.voice->pause();
			dpa.paused = true;
			return;
		}
	}
}

void XAudio2AudioSystem::resumeAudioEvent(AudioHandle handle)
{
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.handle == handle)
		{
			dpa.voice->resume();
			dpa.paused = false;
			return;
		}
	}
}

void XAudio2AudioSystem::setAudioPosition(AudioHandle handle, float x, float y, float z)
{
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.handle == handle)
		{
			if (dpa.voice)
			{
				dpa.voice->setPosition(x, y, z);
				if (m_x3dInitialized && dpa.voice->getType() == XAudio2Voice::VOICE_3D)
				{
					X3DAUDIO_LISTENER listener = {};
					listener.Position = {m_listenerPos[0], m_listenerPos[1], m_listenerPos[2]};
					listener.OrientFront = {m_listenerFwd[0], m_listenerFwd[1], m_listenerFwd[2]};
					listener.OrientTop = {m_listenerUp[0], m_listenerUp[1], m_listenerUp[2]};
					X3DAUDIO_EMITTER emitter = {};
					emitter.ChannelCount = 1;
					emitter.CurveDistanceScaler = dpa.maxDistance;
					dpa.voice->apply3D(m_x3dInstance, &listener, &emitter,
						m_mastering.getChannelCount(), dpa.minDistance);
				}
			}
			return;
		}
	}
}

void XAudio2AudioSystem::setAudioVolume(AudioHandle handle, float volume)
{
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.handle == handle)
		{
			dpa.volume = volume;
			float busVol = m_busVolume[static_cast<int>(dpa.bus)];
			if (dpa.voice)
				dpa.voice->setVolume(volume * busVol);
			return;
		}
	}
}

void XAudio2AudioSystem::setAudioPitch(AudioHandle handle, float pitch)
{
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.handle == handle)
		{
			if (dpa.voice)
				dpa.voice->setPitch(pitch);
			return;
		}
	}
}

bool XAudio2AudioSystem::isAudioPlaying(AudioHandle handle) const
{
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.handle == handle)
			return dpa.voice && (dpa.voice->isPlaying() || dpa.paused);
	}
	return false;
}

void XAudio2AudioSystem::setCompletionCallback(AudioHandle handle,
	AudioCompletionCallback callback, void *userData)
{
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.handle == handle)
		{
			dpa.completionCb = callback;
			dpa.completionUserData = userData;
			return;
		}
	}
}

float XAudio2AudioSystem::getFileDurationMs(const char *filename) const
{
	if (!filename || filename[0] == '\0')
		return 0.0f;
	return XAudio2Decoder::getDurationMs(filename, getFileProvider());
}

void XAudio2AudioSystem::setBusVolume(AudioBus bus, float volume)
{
	m_busVolume[static_cast<int>(bus)] = volume;
	// Update gain on all playing sounds on this bus
	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.bus == bus && dpa.voice)
			dpa.voice->setVolume(dpa.volume * volume);
	}
}

float XAudio2AudioSystem::getBusVolume(AudioBus bus) const
{
	return m_busVolume[static_cast<int>(bus)];
}

void XAudio2AudioSystem::setBusEnabled(AudioBus bus, bool enabled)
{
	m_busEnabled[static_cast<int>(bus)] = enabled;
}

bool XAudio2AudioSystem::isBusEnabled(AudioBus bus) const
{
	return m_busEnabled[static_cast<int>(bus)];
}

void XAudio2AudioSystem::setListenerPosition(float posX, float posY, float posZ,
	float fwdX, float fwdY, float fwdZ,
	float upX, float upY, float upZ)
{
	m_listenerPos[0] = posX; m_listenerPos[1] = posY; m_listenerPos[2] = posZ;
	m_listenerFwd[0] = fwdX; m_listenerFwd[1] = fwdY; m_listenerFwd[2] = fwdZ;
	m_listenerUp[0]  = upX;  m_listenerUp[1]  = upY;  m_listenerUp[2]  = upZ;

	if (!m_x3dInitialized)
		return;

	// Apply 3D to all currently playing 3D direct-play sounds
	X3DAUDIO_LISTENER listener = {};
	listener.Position = {posX, posY, posZ};
	listener.OrientFront = {fwdX, fwdY, fwdZ};
	listener.OrientTop = {upX, upY, upZ};

	X3DAUDIO_EMITTER emitter = {};
	emitter.ChannelCount = 1;

	for (auto &dpa : m_directPlayingSounds)
	{
		if (dpa.voice && dpa.voice->getType() == XAudio2Voice::VOICE_3D)
		{
			emitter.CurveDistanceScaler = dpa.maxDistance;
			dpa.voice->apply3D(m_x3dInstance, &listener, &emitter, m_mastering.getChannelCount(), dpa.minDistance);
		}
	}
}

// ─── Direct-play update (cleanup, loops, fading, callbacks) ─────

void XAudio2AudioSystem::updateDirectPlayingSounds()
{
	constexpr float dt = 1.0f / 30.0f;

	for (auto it = m_directPlayingSounds.begin(); it != m_directPlayingSounds.end(); )
	{
		if (!it->voice)
		{
			it = m_directPlayingSounds.erase(it);
			continue;
		}

		bool playing = it->voice->isPlaying();

		// Handle fade-in
		if (it->fadeInTime > 0.0f && !it->fadingOut && playing)
		{
			it->fadeElapsed += dt;
			float t = it->fadeElapsed / it->fadeInTime;
			if (t >= 1.0f)
			{
				t = 1.0f;
				it->fadeInTime = 0.0f;
				it->fadeElapsed = 0.0f;
			}
			float busVol = m_busVolume[static_cast<int>(it->bus)];
			it->voice->setVolume(it->volume * busVol * t);
		}

		// Handle fade-out
		if (it->fadingOut)
		{
			it->fadeElapsed += dt;
			float t = 1.0f - (it->fadeElapsed / it->fadeOutTime);
			if (t <= 0.0f)
			{
				it->voice->stop();
				releaseDirectVoice(it->voice);
				if (it->completionCb)
					it->completionCb(it->handle, it->completionUserData);
				it = m_directPlayingSounds.erase(it);
				continue;
			}
			float busVol = m_busVolume[static_cast<int>(it->bus)];
			it->voice->setVolume(it->volume * busVol * t);
		}

		// Source finished naturally
		if (!playing && !it->paused)
		{
			if (it->loopsRemaining > 0)
			{
				it->loopsRemaining--;
				it->voice->stop();
				it->voice->setLooping(false);
				it->voice->submitBuffer(it->pcmData, it->pcmSize, true);
				it->voice->play();
			}
			else if (it->loopsRemaining == 0)
			{
				releaseDirectVoice(it->voice);
				if (it->completionCb)
					it->completionCb(it->handle, it->completionUserData);
				it = m_directPlayingSounds.erase(it);
				continue;
			}
			// loopsRemaining < 0 means infinite — handled by XAudio2 looping
		}

		++it;
	}
}

// ─── Video stream ───────────────────────────────────────────────

#include "XAudio2VideoStream.h"

AudioVideoStream *XAudio2AudioSystem::createVideoStream()
{
	if (!m_xaudio)
		return nullptr;

	// Route video audio through the Speech submix voice
	IXAudio2SubmixVoice *speechSubmix = m_mastering.getSubmixVoice(XAudio2Mastering::BUS_SPEECH);
	auto *stream = new XAudio2VideoStream(m_xaudio, speechSubmix);
	if (!stream->init())
	{
		delete stream;
		return nullptr;
	}
	return stream;
}

void XAudio2AudioSystem::releaseVideoStream(AudioVideoStream *stream)
{
	delete stream;
}

#endif // _WIN32
