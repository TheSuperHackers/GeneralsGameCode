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
struct IXAudio2MasteringVoice;
struct IXAudio2SubmixVoice;

/// Mastering and submix bus routing for XAudio2.
/// Owns the mastering voice and four submix voices (Music/Sound/Sound3D/Speech).
class XAudio2Mastering
{
public:
	enum Bus
	{
		BUS_MUSIC,
		BUS_SOUND,
		BUS_SOUND3D,
		BUS_SPEECH,
		BUS_COUNT
	};

	XAudio2Mastering();
	~XAudio2Mastering();

	bool init(IXAudio2 *xaudio);
	void shutdown();

	void setMasterVolume(float volume);
	void setBusVolume(Bus bus, float volume);
	float getBusVolume(Bus bus) const;

	void muteBus(Bus bus, bool mute);

	IXAudio2MasteringVoice *getMasterVoice() const { return m_masterVoice; }
	IXAudio2SubmixVoice *getSubmixVoice(Bus bus) const { return m_submixVoice[bus]; }

	uint32_t getChannelCount() const { return m_channelCount; }
	uint32_t getChannelMask() const { return m_channelMask; }
	uint32_t getSampleRate() const { return m_sampleRate; }

private:
	IXAudio2MasteringVoice *m_masterVoice;
	IXAudio2SubmixVoice *m_submixVoice[BUS_COUNT];
	float m_busVolume[BUS_COUNT];
	bool m_busMuted[BUS_COUNT];
	uint32_t m_channelCount;
	uint32_t m_channelMask;
	uint32_t m_sampleRate;
};


