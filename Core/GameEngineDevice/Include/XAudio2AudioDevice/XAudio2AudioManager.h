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

#include "Common/GameAudio.h"
#include "Common/AudioEventRTS.h"
#include "core/AudioEvent.h"
#include "core/AudioHandle.h"
#include "core/AudioBus.h"
#include "XAudio2AudioDevice/RTSAudioFileProvider.h"

#include <list>

/// XAudio2AudioManager: thin RTS-to-AudioEngine adapter.
///
/// Translates RTS AudioManager API calls into AudioEngine calls.
/// All scheduling, voice management, fading, and volume logic lives
/// in AudioEngine. The bridge only converts RTS types (AudioEventRTS,
/// AudioAffect, AudioRequest) into game-agnostic AudioEngine API calls.
///
/// Lives in the game layer, not the engine. Zero engine dependencies
/// leak from bridge back to engine.
class XAudio2AudioManager : public AudioManager
{
public:
	XAudio2AudioManager();
	~XAudio2AudioManager() override;

	// ── SubsystemInterface ────────────────────────────────────
	void init() override;
	void postProcessLoad() override;
	void reset() override;
	void update() override;

	// ── Device-dependent (delegated to AudioEngine) ──
#if defined(RTS_DEBUG)
	void audioDebugDisplay(DebugDisplayInterface *dd, void *userData, FILE *fp = nullptr) override;
#endif

	void stopAudio(AudioAffect which) override;
	void pauseAudio(AudioAffect which) override;
	void resumeAudio(AudioAffect which) override;
	void pauseAmbient(Bool shouldPause) override;

	void killAudioEventImmediately(AudioHandle audioEvent) override;
	Bool isCurrentlyPlaying(AudioHandle handle) override;

	AsciiString nextMusicTrack() override;
	AsciiString prevMusicTrack() override;
	Bool isMusicPlaying() const override;
	Bool hasMusicTrackCompleted(const AsciiString &trackName, Int numberOfTimes) const override;

	void openDevice() override;
	void closeDevice() override;
	void *getDevice() override;

	void notifyOfAudioCompletion(UnsignedInt audioCompleted, UnsignedInt flags) override;

	UnsignedInt getProviderCount() const override;
	AsciiString getProviderName(UnsignedInt providerNum) const override;
	UnsignedInt getProviderIndex(AsciiString providerName) const override;
	void selectProvider(UnsignedInt providerNdx) override;
	void unselectProvider() override;
	UnsignedInt getSelectedProvider() const override;

	void setSpeakerType(UnsignedInt speakerType) override;
	UnsignedInt getSpeakerType() override;

	UnsignedInt getNum2DSamples() const override;
	UnsignedInt getNum3DSamples() const override;
	UnsignedInt getNumStreams() const override;
	UnsignedInt getNumAvailable2DSamples() const override;
	UnsignedInt getNumAvailable3DSamples() const override;

	Bool doesViolateLimit(AudioEventRTS *event) const override;
	Bool isPlayingLowerPriority(AudioEventRTS *event) const override;
	Bool isPlayingAlready(AudioEventRTS *event) const override;
	Bool isObjectPlayingVoice(UnsignedInt objID) const override;

	void setVolume(Real volume, AudioAffect whichToAffect) override;
	void setOn(Bool turnOn, AudioAffect whichToAffect) override;

	void adjustVolumeOfPlayingAudio(AsciiString eventName, Real newVolume) override;
	void removePlayingAudio(AsciiString eventName) override;
	void removeAllDisabledAudio() override;

	Bool has3DSensitiveStreamsPlaying() const override;

	void *getHandleForVideo() override;
	void releaseHandleForVideo() override;

	void friend_forcePlayAudioEventRTS(const AudioEventRTS *eventToPlay) override;

	void setPreferredProvider(AsciiString providerNdx) override;
	void setPreferredSpeaker(AsciiString speakerType) override;

	void addAudioEventInfo(AudioEventInfo *newEventInfo) override;

	void setHardwareAccelerated(Bool accel) override;
	Bool getHardwareAccelerated() override;

	void setSpeakerSurround(Bool surround) override;
	Bool getSpeakerSurround() override;

	Real getFileLengthMS(AsciiString strToLoad) const override;
	void closeAnySamplesUsingFile(const void *fileToClose) override;

protected:
	void setDeviceListenerPosition() override;
	void processRequestList() override;

private:
	/// Convert an RTS AudioEventRTS into a game-agnostic AudioEvent.
	AudioEvent makeAudioEvent(AudioEventRTS *event) const;

	/// Get AudioBus for an RTS event based on its type.
	AudioBus getBusForEvent(AudioEventRTS *event) const;

	/// Synchronize AudioManager bus volumes/enabled state to AudioEngine.
	void syncBusState();

	/// Update 3D positions of playing sounds attached to moving objects.
	void updatePlayingPositions();

	/// Tracking for positional sounds attached to game objects/drawables.
	struct PlayingPositional
	{
		AudioHandle handle;
		ObjectID objectID;
		DrawableID drawableID;
		OwnerType ownerType;
	};
	std::list<PlayingPositional> m_playingPositional;

	/// Tracking for all playing sounds — maps handle → event name, objectID, sound type.
	/// Used by doesViolateLimit, isObjectPlayingVoice, etc.
	struct PlayingEventInfo
	{
		AudioHandle handle;
		AsciiString eventName;
		UnsignedInt objectID;
		UnsignedInt soundType; // ST_VOICE etc from AudioEventInfo::m_type
	};
	std::list<PlayingEventInfo> m_playingEvents;

	/// Prune entries for sounds that have finished playing.
	void prunePlayingEvents();

	AsciiString m_prefProvider;
	AsciiString m_prefSpeaker;
	UnsignedInt m_selectedProvider = 0;
	UnsignedInt m_selectedSpeakerType = 0;
	Bool m_hardwareAccel = FALSE;
	Bool m_surroundSpeakers = FALSE;
	class AudioVideoStream *m_videoStream = nullptr;
	RTSAudioFileProvider m_fileProvider;
};
