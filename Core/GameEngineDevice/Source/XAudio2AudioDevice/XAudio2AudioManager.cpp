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

#include "XAudio2AudioDevice/XAudio2AudioManager.h"
#include "AudioEngine.h"
#include "AudioFactory.h"
#include "core/AudioSystem.h"
#include "core/AudioEventDefinition.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioEventInfo.h"
#include "Common/AudioRequest.h"
#include "Common/AudioAffect.h"
#include "Common/AudioHandleSpecialValues.h"
#include "Common/AudioSettings.h"
#include "Common/Thing.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameClient/GameClient.h"
#include "GameClient/Drawable.h"

XAudio2AudioManager::XAudio2AudioManager()
{
}

XAudio2AudioManager::~XAudio2AudioManager()
{
}

// ── SubsystemInterface ──────────────────────────────────────────

void XAudio2AudioManager::init()
{
	AudioManager::init();
	AudioBackendConfig config;
	config.fileProvider = &m_fileProvider;
	theAudio().init(config);
	syncBusState();
}

void XAudio2AudioManager::postProcessLoad()
{
	AudioManager::postProcessLoad();
	theAudio().postProcessLoad();
}

void XAudio2AudioManager::reset()
{
	releaseHandleForVideo();
	removeAllAudioRequests();
	theAudio().reset();
	m_playingEvents.clear();
	m_playingPositional.clear();
	AudioManager::reset();
}

void XAudio2AudioManager::update()
{
	AudioManager::update();
	syncBusState();
	processRequestList();
	prunePlayingEvents();
	updatePlayingPositions();
	setDeviceListenerPosition();
	theAudio().update();
}

#if defined(RTS_DEBUG)
void XAudio2AudioManager::audioDebugDisplay(DebugDisplayInterface *, void *, FILE *) {}
#endif

// ── Request processing ──────────────────────────────────────────

void XAudio2AudioManager::processRequestList()
{
	std::list<AudioRequest *> &requests = m_audioRequests;

	for (auto it = requests.begin(); it != requests.end(); )
	{
		AudioRequest *req = *it;

		// Check delay
		if (req->m_pendingEvent)
		{
			Real delay = req->m_pendingEvent->getDelay();
			if (delay > MSEC_PER_LOGICFRAME_REAL)
			{
				req->m_pendingEvent->decrementDelay(MSEC_PER_LOGICFRAME_REAL);
				++it;
				continue;
			}
		}

		switch (req->m_request)
		{
		case AR_Play:
			if (req->m_pendingEvent)
			{
				AudioEventRTS *event = req->m_pendingEvent.Peek();
				getInfoForAudioEvent(event);
				const AudioEventInfo *info = event->getAudioEventInfo();
				if (info)
				{
					// Check audio type enabled
					bool enabled = true;
					switch (info->m_soundType)
					{
					case AT_Music:
						enabled = isOn(AudioAffect_Music);
						break;
					case AT_SoundEffect:
						enabled = event->isPositionalAudio()
							? isOn(AudioAffect_Sound3D) : isOn(AudioAffect_Sound);
						break;
					case AT_Streaming:
						enabled = isOn(AudioAffect_Speech);
						break;
					default: break;
					}

					if (enabled && info->m_soundType == AT_SoundEffect &&
						!(info->m_control & AC_LOOP) && isPlayingAlready(event))
						enabled = false;

					if (enabled)
					{
						// Check per-event limits (matching original MilesAudioManager)
						if (doesViolateLimit(event))
						{
							if (info->m_control & AC_INTERRUPT)
							{
								// Kill the oldest instance to make room
								AudioHandle handleToKill = event->getHandleToKill();
								if (handleToKill)
								{
									theAudio().kill(handleToKill);
									// Remove from tracking lists
									m_playingEvents.remove_if([handleToKill](const PlayingEventInfo &e) { return e.handle == handleToKill; });
									m_playingPositional.remove_if([handleToKill](const PlayingPositional &p) { return p.handle == handleToKill; });
								}
								else
								{
									enabled = false;
								}
							}
							else
							{
								enabled = false;
							}
						}
					}

					if (enabled)
					{
						event->generateFilename();
						event->generatePlayInfo();
						// getFilename() returns an AsciiString by value. Keep a local
						// copy alive while the backend synchronously opens/decodes it;
						// AudioEvent only contains a borrowed const char pointer.
						AsciiString generatedFilename = event->getFilename();
						AudioEvent ae = makeAudioEvent(event);
						ae.filename = generatedFilename.str();

						// A new music stream replaces the current stream immediately,
						// matching the working Miles/AudioManagerBridge behavior.
						if (info->m_soundType == AT_Music)
							theAudio().stopMusic(0.0f);

						// Submit music through the same path as every other RTS audio
						// event.  The playMusic() convenience function only accepts a
						// filename and consequently discards the event's volume, pitch,
						// priority and fade state.  That is particularly important for
						// the credits transition, which queues a fade-stop immediately
						// before the Credits event.
						AudioHandle handle = theAudio().play(ae);
						if (handle != AUDIO_HANDLE_INVALID)
						{
							event->setPlayingHandle(handle);

							// Track event info for limit checks and voice queries
							PlayingEventInfo pei;
							pei.handle = handle;
							pei.eventName = event->getEventName();
							pei.objectID = event->getObjectID();
							pei.soundType = info->m_type;
							m_playingEvents.push_back(pei);

							// Track positional sounds attached to objects/drawables
							// so we can update their position each frame
							if (event->isPositionalAudio() &&
								event->getOwnerType() != OT_Positional &&
								event->getOwnerType() != OT_INVALID)
							{
								PlayingPositional pp;
								pp.handle = handle;
								pp.objectID = (event->getOwnerType() == OT_Object) ? event->getObjectID() : INVALID_ID;
								pp.drawableID = (event->getOwnerType() == OT_Drawable) ? event->getDrawableID() : INVALID_DRAWABLE_ID;
								pp.ownerType = event->getOwnerType();
								m_playingPositional.push_back(pp);
							}
						}
						else
						{
						}
					}
				}
			}
			break;

		case AR_Pause:
			theAudio().pause(req->m_handleToInteractOn);
			break;

		case AR_Stop:
		{
			AudioHandle h = req->m_handleToInteractOn;
			// Handle music stop sentinels
			if (h == AHSV_StopTheMusic)
				theAudio().stopMusic(0.0f);
			else if (h == AHSV_StopTheMusicFade)
				theAudio().stopMusic(2.0f);
			else
			{
				theAudio().stop(h);
				m_playingPositional.remove_if([h](const PlayingPositional &pp) { return pp.handle == h; });
			}
			break;
		}
		}

		it = requests.erase(it);
		releaseAudioRequest(req);
	}
}

// ── Stop / Pause / Resume by AudioAffect ────────────────────────

void XAudio2AudioManager::stopAudio(AudioAffect which)
{
	if (which & AudioAffect_Sound)
		theAudio().stopBus(AudioBus::Sound);
	if (which & AudioAffect_Sound3D)
	{
		theAudio().stopBus(AudioBus::Sound3D);
		m_playingPositional.clear();
	}
	if (which & AudioAffect_Music)
		theAudio().stopBus(AudioBus::Music);
	if (which & AudioAffect_Speech)
		theAudio().stopBus(AudioBus::Speech);
}

void XAudio2AudioManager::pauseAudio(AudioAffect which)
{
	if (which & AudioAffect_Sound)
		theAudio().pauseBus(AudioBus::Sound);
	if (which & AudioAffect_Sound3D)
		theAudio().pauseBus(AudioBus::Sound3D);
	if (which & AudioAffect_Music)
		theAudio().pauseBus(AudioBus::Music);
	if (which & AudioAffect_Speech)
		theAudio().pauseBus(AudioBus::Speech);
}

void XAudio2AudioManager::resumeAudio(AudioAffect which)
{
	if (which & AudioAffect_Sound)
		theAudio().resumeBus(AudioBus::Sound);
	if (which & AudioAffect_Sound3D)
		theAudio().resumeBus(AudioBus::Sound3D);
	if (which & AudioAffect_Music)
		theAudio().resumeBus(AudioBus::Music);
	if (which & AudioAffect_Speech)
		theAudio().resumeBus(AudioBus::Speech);
}

void XAudio2AudioManager::pauseAmbient(Bool /*shouldPause*/)
{
}

// ── Handle-based operations ─────────────────────────────────────

void XAudio2AudioManager::killAudioEventImmediately(AudioHandle handle)
{
	theAudio().kill(handle);
}

Bool XAudio2AudioManager::isCurrentlyPlaying(AudioHandle handle)
{
	return theAudio().isPlaying(handle) ? TRUE : FALSE;
}

// ── Music ───────────────────────────────────────────────────────

AsciiString XAudio2AudioManager::nextMusicTrack()
{
	return AudioManager::nextTrackName(AsciiString::TheEmptyString);
}

AsciiString XAudio2AudioManager::prevMusicTrack()
{
	return AudioManager::prevTrackName(AsciiString::TheEmptyString);
}

Bool XAudio2AudioManager::isMusicPlaying() const
{
	return theAudio().isMusicPlaying() ? TRUE : FALSE;
}

Bool XAudio2AudioManager::hasMusicTrackCompleted(const AsciiString & /*trackName*/, Int /*numberOfTimes*/) const
{
	return !theAudio().isMusicPlaying() ? TRUE : FALSE;
}

// ── Device ──────────────────────────────────────────────────────

void XAudio2AudioManager::openDevice()
{
}

void XAudio2AudioManager::closeDevice()
{
}

void *XAudio2AudioManager::getDevice()
{
	auto *backend = theAudio().getBackend();
	return backend ? backend->getDevice() : nullptr;
}

// ── Completion ──────────────────────────────────────────────────

void XAudio2AudioManager::notifyOfAudioCompletion(UnsignedInt /*audioCompleted*/, UnsignedInt /*flags*/)
{
}

// ── Provider / speaker (simple storage) ─────────────────────────

UnsignedInt XAudio2AudioManager::getProviderCount() const { return 1; }

AsciiString XAudio2AudioManager::getProviderName(UnsignedInt /*providerNum*/) const
{
	AsciiString name("Default");
	return name;
}

UnsignedInt XAudio2AudioManager::getProviderIndex(AsciiString /*providerName*/) const { return 0; }

void XAudio2AudioManager::selectProvider(UnsignedInt providerNdx) { m_selectedProvider = providerNdx; }
void XAudio2AudioManager::unselectProvider() { m_selectedProvider = 0; }
UnsignedInt XAudio2AudioManager::getSelectedProvider() const { return m_selectedProvider; }

void XAudio2AudioManager::setSpeakerType(UnsignedInt speakerType) { m_selectedSpeakerType = speakerType; }
UnsignedInt XAudio2AudioManager::getSpeakerType() { return m_selectedSpeakerType; }

UnsignedInt XAudio2AudioManager::getNum2DSamples() const { return theAudio().getNum2DSamples(); }
UnsignedInt XAudio2AudioManager::getNum3DSamples() const { return theAudio().getNum3DSamples(); }
UnsignedInt XAudio2AudioManager::getNumStreams() const { return theAudio().getNumStreams(); }
UnsignedInt XAudio2AudioManager::getNumAvailable2DSamples() const { return getNum2DSamples(); }
UnsignedInt XAudio2AudioManager::getNumAvailable3DSamples() const { return getNum3DSamples(); }

// ── Limit / priority checking ───────────────────────────────────

Bool XAudio2AudioManager::doesViolateLimit(AudioEventRTS *event) const
{
	if (!event) return FALSE;
	const AudioEventInfo *info = event->getAudioEventInfo();
	if (!info || info->m_limit == 0) return FALSE;

	Int limit = info->m_limit;
	Int count = 0;
	Int pendingCount = 0;
	AudioHandle oldestHandle = 0;

	// Count currently playing sounds with the same event name
	for (const auto &pei : m_playingEvents)
	{
		if (pei.eventName == event->getEventName() && theAudio().isPlaying(pei.handle))
		{
			if (count == 0)
				oldestHandle = pei.handle; // oldest is first in list
			++count;
		}
	}

	for (const auto *request : m_audioRequests)
	{
		if (request->m_pendingEvent &&
			request->m_pendingEvent->getEventName() == event->getEventName())
		{
			++pendingCount;
			++count;
		}
	}

	if (count < limit)
	{
		event->setHandleToKill(0);
		return false;
	}

	// Limit exceeded — set handleToKill so caller can replace the oldest
	event->setHandleToKill(oldestHandle);

	if (info->m_control & AC_INTERRUPT)
		return pendingCount >= limit;

	return true;
}

Bool XAudio2AudioManager::isPlayingLowerPriority(AudioEventRTS *event) const
{
	if (!event) return FALSE;
	const AudioEventInfo *info = event->getAudioEventInfo();
	if (!info) return FALSE;

	AudioPriority priority = info->m_priority;
	if (priority == AP_LOWEST) return FALSE;

	// Check if any currently playing sound of the same positional type
	// has lower priority than the new event
	for (const auto &pei : m_playingEvents)
	{
		if (!theAudio().isPlaying(pei.handle)) continue;
		// Look up the event info for this playing sound
		const AudioEventInfo *playingInfo = findAudioEventInfo(pei.eventName);
		if (playingInfo && playingInfo->m_priority < priority)
			return TRUE;
	}

	return FALSE;
}

Bool XAudio2AudioManager::isPlayingAlready(AudioEventRTS *event) const
{
	if (!event) return FALSE;
	for (const auto &pei : m_playingEvents)
	{
		if (pei.eventName == event->getEventName() && theAudio().isPlaying(pei.handle))
			return TRUE;
	}
	return FALSE;
}

Bool XAudio2AudioManager::isObjectPlayingVoice(UnsignedInt objID) const
{
	if (objID == 0) return FALSE;
	for (const auto &pei : m_playingEvents)
	{
		if (pei.objectID == objID && (pei.soundType & ST_VOICE) && theAudio().isPlaying(pei.handle))
			return TRUE;
	}
	return FALSE;
}

void XAudio2AudioManager::prunePlayingEvents()
{
	for (auto it = m_playingEvents.begin(); it != m_playingEvents.end(); )
	{
		if (!theAudio().isPlaying(it->handle))
			it = m_playingEvents.erase(it);
		else
			++it;
	}
}

// ── Volume / enabled forwarding ─────────────────────────────────

void XAudio2AudioManager::setVolume(Real volume, AudioAffect whichToAffect)
{
	AudioManager::setVolume(volume, whichToAffect);
	syncBusState();
}

void XAudio2AudioManager::setOn(Bool turnOn, AudioAffect whichToAffect)
{
	AudioManager::setOn(turnOn, whichToAffect);
	syncBusState();
}

void XAudio2AudioManager::syncBusState()
{
	// Forward RTS volume and enabled state to AudioEngine buses.
	// RTS volumes are already computed as system × script products.
	theAudio().setBusVolume(AudioBus::Sound, m_soundVolume);
	theAudio().setBusVolume(AudioBus::Sound3D, m_sound3DVolume);
	theAudio().setBusVolume(AudioBus::Music, m_musicVolume);
	theAudio().setBusVolume(AudioBus::Speech, m_speechVolume);

	theAudio().setBusEnabled(AudioBus::Sound, isOn(AudioAffect_Sound));
	theAudio().setBusEnabled(AudioBus::Sound3D, isOn(AudioAffect_Sound3D));
	theAudio().setBusEnabled(AudioBus::Music, isOn(AudioAffect_Music));
	theAudio().setBusEnabled(AudioBus::Speech, isOn(AudioAffect_Speech));

	const AudioSettings *settings = getAudioSettings();
	if (settings)
	{
		theAudio().setBusMaxVoices(AudioBus::Sound, settings->m_sampleCount2D);
		theAudio().setBusMaxVoices(AudioBus::Sound3D, settings->m_sampleCount3D);
		theAudio().setBusMaxVoices(AudioBus::Speech, settings->m_streamCount);
	}
}

// ── Volume / playing audio adjustments ──────────────────────────

void XAudio2AudioManager::adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume)
{
	// eventName is RTS event name; we use filename-based matching in AudioEngine
	(void)eventName;
	(void)newVolume;
}

void XAudio2AudioManager::removePlayingAudio(AsciiString eventName)
{
	(void)eventName;
}

void XAudio2AudioManager::removeAllDisabledAudio()
{
	if (!isOn(AudioAffect_Sound))
		theAudio().stopBus(AudioBus::Sound);
	if (!isOn(AudioAffect_Sound3D))
		theAudio().stopBus(AudioBus::Sound3D);
	if (!isOn(AudioAffect_Music))
		theAudio().stopBus(AudioBus::Music);
	if (!isOn(AudioAffect_Speech))
		theAudio().stopBus(AudioBus::Speech);
}

Bool XAudio2AudioManager::has3DSensitiveStreamsPlaying() const
{
	return FALSE;
}

// ── Video ───────────────────────────────────────────────────────

void *XAudio2AudioManager::getHandleForVideo()
{
	if (!m_videoStream)
		m_videoStream = theAudio().createVideoStream();
	return m_videoStream;
}

void XAudio2AudioManager::releaseHandleForVideo()
{
	if (m_videoStream)
	{
		theAudio().releaseVideoStream(m_videoStream);
		m_videoStream = nullptr;
	}
}

// ── Force play ──────────────────────────────────────────────────

void XAudio2AudioManager::friend_forcePlayAudioEventRTS(const AudioEventRTS *eventToPlay)
{
	AudioEventRTS *event = const_cast<AudioEventRTS *>(eventToPlay);
	getInfoForAudioEvent(event);
	 event->generateFilename();
	 event->generatePlayInfo();
	AsciiString generatedFilename = event->getFilename();
	AudioEvent ae = makeAudioEvent(event);
	ae.filename = generatedFilename.str();
	AudioHandle handle = theAudio().play(ae);
	if (handle != AUDIO_HANDLE_INVALID)
		event->setPlayingHandle(handle);
}

// ── Preferences / settings ──────────────────────────────────────

void XAudio2AudioManager::setPreferredProvider(AsciiString provider) { m_prefProvider = provider; }
void XAudio2AudioManager::setPreferredSpeaker(AsciiString speaker) { m_prefSpeaker = speaker; }

void XAudio2AudioManager::setHardwareAccelerated(Bool accel)
{
	AudioManager::setHardwareAccelerated(accel);
	m_hardwareAccel = accel;
}

Bool XAudio2AudioManager::getHardwareAccelerated()
{
	return m_hardwareAccel;
}

void XAudio2AudioManager::setSpeakerSurround(Bool surround)
{
	AudioManager::setSpeakerSurround(surround);
	m_surroundSpeakers = surround;
}

Bool XAudio2AudioManager::getSpeakerSurround()
{
	return m_surroundSpeakers;
}

// ── File queries ────────────────────────────────────────────────

Real XAudio2AudioManager::getFileLengthMS(AsciiString strToLoad) const
{
	return theAudio().getFileDurationMs(strToLoad.str());
}

void XAudio2AudioManager::closeAnySamplesUsingFile(const void * /*fileToClose*/)
{
}

// ── Event Registration ──────────────────────────────────────────

/// Convert RTS AudioEventInfo to engine-level AudioEventDefinition.
static AudioEventDefinition infoToEventDef(const AudioEventInfo *info)
{
	static_assert(static_cast<int>(AudioPriorityLevel::Lowest)  == AP_LOWEST,  "Priority enum mismatch");
	static_assert(static_cast<int>(AudioPriorityLevel::Critical) == AP_CRITICAL, "Priority enum mismatch");
	static_assert(AudioControlFlags::Loop == AC_LOOP, "Control flag mismatch");
	static_assert(AudioControlFlags::Random == AC_RANDOM, "Control flag mismatch");
	static_assert(AudioTypeFlags::UI == ST_UI, "Type flag mismatch");
	static_assert(AudioTypeFlags::Voice == ST_VOICE, "Type flag mismatch");

	AudioEventDefinition def;
	def.name = info->m_audioName.str();

	// Music and streaming events store one filename in m_filename.  Sound
	// effects store their alternatives in m_sounds.  Keeping this distinction
	// is required by the XAudio2 event registry and precache/file lookup.
	if (info->m_soundType == AT_Music || info->m_soundType == AT_Streaming)
	{
		if (!info->m_filename.isEmpty())
			def.filenames.push_back(info->m_filename.str());
	}
	else
	{
		for (const auto &s : info->m_sounds)
			def.filenames.push_back(s.str());
	}

	// Bus routing
	switch (info->m_soundType)
	{
	case AT_Music:      def.bus = AudioBus::Music;   break;
	case AT_Streaming:  def.bus = AudioBus::Speech;  break;
	case AT_SoundEffect:
	default:
		def.bus = (info->m_type & ST_WORLD) ? AudioBus::Sound3D : AudioBus::Sound;
		break;
	}

	def.is3D = (info->m_type & ST_WORLD) != 0;
	def.volume = info->m_volume;
	def.volumeShift = info->m_volumeShift;
	def.minVolume = info->m_minVolume;
	def.minDistance = info->m_minDistance;
	def.maxDistance = info->m_maxDistance;
	def.pitchMin = 1.0f + info->m_pitchShiftMin;
	def.pitchMax = 1.0f + info->m_pitchShiftMax;
	def.priority = static_cast<AudioPriorityLevel>(info->m_priority);
	def.limit = info->m_limit;
	def.loopCount = info->m_loopCount;
	def.control = info->m_control;
	def.type = info->m_type;
	def.delayMinMs = info->m_delayMin;
	def.delayMaxMs = info->m_delayMax;
	def.lowPassFreq = info->m_lowPassFreq;
	return def;
}

void XAudio2AudioManager::addAudioEventInfo(AudioEventInfo *newEventInfo)
{
	AudioManager::addAudioEventInfo(newEventInfo);

	// Also register in the engine event registry (no precache — files loaded on demand)
	AudioEventDefinition def = infoToEventDef(newEventInfo);
	theAudio().registerEvent(def, /*precache=*/false);
}

// ── Position Updates ────────────────────────────────────────────

void XAudio2AudioManager::updatePlayingPositions()
{
	auto it = m_playingPositional.begin();
	while (it != m_playingPositional.end())
	{
		// Check if the sound is still playing in the engine
		if (!theAudio().isPlaying(it->handle))
		{
			it = m_playingPositional.erase(it);
			continue;
		}

		const Coord3D *pos = nullptr;
		bool dead = false;

		switch (it->ownerType)
		{
		case OT_Object:
			if (TheGameLogic)
			{
				if (Object *obj = TheGameLogic->findObjectByID(it->objectID))
					pos = obj->getPosition();
				else
					dead = true;
			}
			break;
		case OT_Drawable:
			if (TheGameClient)
			{
				if (Drawable *draw = TheGameClient->findDrawableByID(it->drawableID))
					pos = draw->getPosition();
				else
					dead = true;
			}
			break;
		default:
			break;
		}

		if (dead)
		{
			theAudio().stop(it->handle);
			it = m_playingPositional.erase(it);
			continue;
		}

		if (pos)
		{
			// RTS(x,y,z) → Audio(x,z,y)
			theAudio().setPosition(it->handle, -pos->x, pos->z, -pos->y);
		}

		++it;
	}
}

// ── Listener ────────────────────────────────────────────────────

void XAudio2AudioManager::setDeviceListenerPosition()
{
	const Coord3D *pos = getListenerPosition();
	if (!pos) return;

	const Coord3D &ori = m_listenerOrientation;

	theAudio().setListenerPosition(
		-pos->x, pos->z, -pos->y,
		-ori.x, ori.z, -ori.y,
		0.0f, 1.0f, 0.0f);
}

// ── Helpers ─────────────────────────────────────────────────────

AudioEvent XAudio2AudioManager::makeAudioEvent(AudioEventRTS *event) const
{
	AudioEvent ae;
	ae.filename = event->getFilename().str();

	const AudioEventInfo *info = event->getAudioEventInfo();
	if (info)
	{
		switch (info->m_soundType)
		{
		case AT_Music:
			ae.bus = AudioBus::Music;
			ae.loopCount = 0;
			break;
		case AT_Streaming: ae.bus = AudioBus::Speech; break;
		case AT_SoundEffect:
		default:
			ae.bus = event->isPositionalAudio() ? AudioBus::Sound3D : AudioBus::Sound;
			break;
		}
	}

	if (event->isPositionalAudio() && (!info || info->m_soundType != AT_Music))
	{
		const Coord3D *pos = event->getCurrentPosition();
		if (pos)
		{
			ae.is3D = true;
			ae.posX = -pos->x;
			ae.posY = pos->z;
			ae.posZ = -pos->y;
		}
	}

	// Volume: event volume × volume shift (bus volume handled by AudioEngine)
	ae.volume = event->getVolume() * event->getVolumeShift();

	// Pitch: clamp to valid range for XAudio2 (must be > 0, typically 0.5–2.0)
	float pitch = event->getPitchShift();
	ae.pitchShift = (pitch > 0.01f) ? pitch : 1.0f;
	ae.lowPassFreq = info ? info->m_lowPassFreq : 0.0f;

	if (info)
	{
		if (info->m_type & ST_GLOBAL)
		{
			const AudioSettings *settings = getAudioSettings();
			if (settings)
			{
				ae.minDistance = static_cast<float>(settings->m_globalMinRange);
				ae.maxDistance = static_cast<float>(settings->m_globalMaxRange);
			}
		}
		else
		{
			ae.minDistance = info->m_minDistance;
			ae.maxDistance = info->m_maxDistance;
		}

	ae.loopCount = (info->m_soundType == AT_Music) ? 0 :
			((info->m_control & AC_LOOP) ? info->m_loopCount : 1);
		if (info->m_soundType == AT_SoundEffect &&
			(info->m_delayMin > 0 || info->m_delayMax > 0))
			ae.loopCount = 1;
		ae.priority = static_cast<float>(info->m_priority) / static_cast<float>(AP_CRITICAL);
	}

	return ae;
}

AudioBus XAudio2AudioManager::getBusForEvent(AudioEventRTS *event) const
{
	if (!event) return AudioBus::Sound;
	const AudioEventInfo *info = event->getAudioEventInfo();
	if (!info) return AudioBus::Sound;

	switch (info->m_soundType)
	{
	case AT_Music:    return AudioBus::Music;
	case AT_Streaming: return AudioBus::Speech;
	case AT_SoundEffect:
	default:
		return event->isPositionalAudio() ? AudioBus::Sound3D : AudioBus::Sound;
	}
}
