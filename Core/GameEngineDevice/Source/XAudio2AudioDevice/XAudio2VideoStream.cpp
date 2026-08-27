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

#include "XAudio2VideoStream.h"

#include <xaudio2.h>
#include <cstring>

XAudio2VideoStream::XAudio2VideoStream(IXAudio2 *xaudio, IXAudio2SubmixVoice *speechSubmix)
	: m_xaudio(xaudio)
	, m_speechSubmix(speechSubmix)
	, m_sourceVoice(nullptr)
	, m_currentSampleRate(0)
	, m_currentChannels(0)
	, m_currentBitsPerSample(0)
	, m_playing(false)
	, m_initialized(false)
	, m_stopped(false)
{
}

XAudio2VideoStream::~XAudio2VideoStream()
{
	stop();
	destroyVoice();
}

bool XAudio2VideoStream::init()
{
	m_initialized = true;
	return true;
}

void XAudio2VideoStream::reset()
{
	stop();
	destroyVoice();

	while (!m_pendingBuffers.empty())
		m_pendingBuffers.pop();

	m_currentSampleRate = 0;
	m_currentChannels = 0;
	m_currentBitsPerSample = 0;
	m_playing = false;
	m_stopped = false;
}

bool XAudio2VideoStream::ensureVoice(int sampleRate, int channels, int bitsPerSample)
{
	if (m_sourceVoice && m_currentSampleRate == sampleRate
		&& m_currentChannels == channels && m_currentBitsPerSample == bitsPerSample)
		return true;

	destroyVoice();

	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = static_cast<WORD>(channels);
	wfx.nSamplesPerSec = static_cast<DWORD>(sampleRate);
	wfx.wBitsPerSample = static_cast<WORD>(bitsPerSample);
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	XAUDIO2_SEND_DESCRIPTOR send = {};
	send.pOutputVoice = m_speechSubmix;
	XAUDIO2_VOICE_SENDS sendList = {};
	sendList.SendCount = 1;
	sendList.pSends = &send;

	XAUDIO2_VOICE_SENDS *pSends = m_speechSubmix ? &sendList : nullptr;

	HRESULT hr = m_xaudio->CreateSourceVoice(&m_sourceVoice, &wfx, 0,
		XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, pSends);
	if (FAILED(hr))
		return false;

	m_currentSampleRate = sampleRate;
	m_currentChannels = channels;
	m_currentBitsPerSample = bitsPerSample;
	return true;
}

void XAudio2VideoStream::destroyVoice()
{
	if (m_sourceVoice)
	{
		m_sourceVoice->SetVolume(0.0f);
		m_sourceVoice->Stop(0);
		m_sourceVoice->FlushSourceBuffers();
		m_sourceVoice->DestroyVoice();
		m_sourceVoice = nullptr;
	}

	while (!m_pendingBuffers.empty())
		m_pendingBuffers.pop();
}

void XAudio2VideoStream::queueBuffer(const void *data, int dataSize, int sampleRate, int channels, int bitsPerSample)
{
	if (m_stopped || !m_initialized || !m_xaudio || dataSize <= 0)
		return;

	if (!ensureVoice(sampleRate, channels, bitsPerSample))
		return;

	// Copy PCM data — the caller's buffer may be reused
	BufferEntry entry;
	entry.data.resize(static_cast<size_t>(dataSize));
	std::memcpy(entry.data.data(), data, static_cast<size_t>(dataSize));
	m_pendingBuffers.push(std::move(entry));

	// Submit to XAudio2
	XAUDIO2_BUFFER buf = {};
	buf.AudioBytes = static_cast<UINT32>(dataSize);
	buf.pAudioData = m_pendingBuffers.back().data.data();
	m_sourceVoice->SubmitSourceBuffer(&buf);
}

void XAudio2VideoStream::play()
{
	if (m_stopped || !m_sourceVoice)
		return;

	m_sourceVoice->SetVolume(1.0f);
	m_sourceVoice->Start(0);
	m_playing = true;
}

void XAudio2VideoStream::stop()
{
	if (m_sourceVoice)
	{
		m_sourceVoice->SetVolume(0.0f);
		m_sourceVoice->Stop(0);
		m_sourceVoice->FlushSourceBuffers();
	}
	m_stopped = true;

	while (!m_pendingBuffers.empty())
		m_pendingBuffers.pop();
	m_playing = false;
}

void XAudio2VideoStream::update()
{
	if (!m_sourceVoice)
		return;

	// Reclaim completed buffers
	XAUDIO2_VOICE_STATE state;
	m_sourceVoice->GetState(&state, 0);
	uint32_t queued = state.BuffersQueued;

	// The number of buffers we can remove = total pending - still queued
	while (m_pendingBuffers.size() > queued)
		m_pendingBuffers.pop();

	// Restart if we stalled
	if (m_playing && queued > 0)
	{
		// Check if voice is still running — XAudio2 auto-stops when it runs out
		// of buffers, but since we have queued buffers, just ensure it's started
		m_sourceVoice->Start(0);
	}
}

bool XAudio2VideoStream::isPlaying() const
{
	if (!m_sourceVoice)
		return false;

	XAUDIO2_VOICE_STATE state;
	m_sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
	return state.BuffersQueued > 0;
}
