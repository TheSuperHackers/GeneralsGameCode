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

#include "XAudio2Mastering.h"

#include <xaudio2.h>

XAudio2Mastering::XAudio2Mastering()
	: m_masterVoice(nullptr)
	, m_channelCount(0)
	, m_channelMask(0)
	, m_sampleRate(0)
{
	for (int i = 0; i < BUS_COUNT; ++i)
	{
		m_submixVoice[i] = nullptr;
		m_busVolume[i] = 1.0f;
		m_busMuted[i] = false;
	}
}

XAudio2Mastering::~XAudio2Mastering()
{
	shutdown();
}

bool XAudio2Mastering::init(IXAudio2 *xaudio)
{
	if (!xaudio)
		return false;

	HRESULT hr = xaudio->CreateMasteringVoice(&m_masterVoice);
	if (FAILED(hr))
		return false;

	XAUDIO2_VOICE_DETAILS details;
	m_masterVoice->GetVoiceDetails(&details);
	m_channelCount = details.InputChannels;
	m_sampleRate = details.InputSampleRate;

	// Query actual output channel mask for proper X3DAudio spatialization
	DWORD channelMask = 0;
	m_masterVoice->GetChannelMask(&channelMask);
	m_channelMask = channelMask;

	// Create submix voices for each bus, routed to master
	XAUDIO2_SEND_DESCRIPTOR sendDesc = {0, m_masterVoice};
	XAUDIO2_VOICE_SENDS sendList = {1, &sendDesc};

	for (int i = 0; i < BUS_COUNT; ++i)
	{
		hr = xaudio->CreateSubmixVoice(&m_submixVoice[i], m_channelCount, m_sampleRate, 0, 0, &sendList);
		if (FAILED(hr))
		{
			shutdown();
			return false;
		}
	}
	return true;
}

void XAudio2Mastering::shutdown()
{
	for (int i = 0; i < BUS_COUNT; ++i)
	{
		if (m_submixVoice[i])
		{
			m_submixVoice[i]->DestroyVoice();
			m_submixVoice[i] = nullptr;
		}
	}
	if (m_masterVoice)
	{
		m_masterVoice->DestroyVoice();
		m_masterVoice = nullptr;
	}
}

void XAudio2Mastering::setMasterVolume(float volume)
{
	if (m_masterVoice)
		m_masterVoice->SetVolume(volume);
}

void XAudio2Mastering::setBusVolume(Bus bus, float volume)
{
	m_busVolume[bus] = volume;
	if (m_submixVoice[bus] && !m_busMuted[bus])
		m_submixVoice[bus]->SetVolume(volume);
}

float XAudio2Mastering::getBusVolume(Bus bus) const
{
	return m_busVolume[bus];
}

void XAudio2Mastering::muteBus(Bus bus, bool mute)
{
	m_busMuted[bus] = mute;
	if (m_submixVoice[bus])
		m_submixVoice[bus]->SetVolume(mute ? 0.0f : m_busVolume[bus]);
}

#endif // _WIN32


