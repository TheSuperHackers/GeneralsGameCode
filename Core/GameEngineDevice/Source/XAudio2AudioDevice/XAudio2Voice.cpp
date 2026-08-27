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

#include "XAudio2Voice.h"

#include <xaudio2.h>
#include <x3daudio.h>
#include <cmath>
#include <cstring>

XAudio2Voice::XAudio2Voice()
	: m_sourceVoice(nullptr)
	, m_submixVoice(nullptr)
	, m_type(VOICE_2D)
	, m_channels(1)
	, m_playing(false)
	, m_looping(false)
	, m_hasLastPosition(false)
{
	m_position[0] = m_position[1] = m_position[2] = 0.0f;
	m_prevPosition[0] = m_prevPosition[1] = m_prevPosition[2] = 0.0f;
	m_velocity[0] = m_velocity[1] = m_velocity[2] = 0.0f;
}

XAudio2Voice::~XAudio2Voice()
{
	destroy();
}

bool XAudio2Voice::create(IXAudio2 *xaudio, uint32_t channels, uint32_t sampleRate,
	IXAudio2SubmixVoice *submix, VoiceType type)
{
	if (!xaudio)
		return false;

	m_type = type;
	m_submixVoice = submix;
	m_channels = channels;

	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
	wfx.nChannels = static_cast<WORD>(channels);
	wfx.nSamplesPerSec = sampleRate;
	wfx.wBitsPerSample = 32;
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	XAUDIO2_SEND_DESCRIPTOR sendDesc = {0, submix};
	XAUDIO2_VOICE_SENDS sendList = {1, &sendDesc};

	// Enable built-in filter for distance-based low-pass on 3D voices
	UINT32 voiceFlags = (type == VOICE_3D) ? XAUDIO2_VOICE_USEFILTER : 0;
	HRESULT hr = xaudio->CreateSourceVoice(&m_sourceVoice, &wfx, voiceFlags, 4.0f, nullptr, &sendList);
	return SUCCEEDED(hr);
}

void XAudio2Voice::destroy()
{
	if (m_sourceVoice)
	{
		m_sourceVoice->Stop();
		m_sourceVoice->FlushSourceBuffers();
		m_sourceVoice->DestroyVoice();
		m_sourceVoice = nullptr;
	}
	m_playing = false;
}

void XAudio2Voice::play()
{
	if (m_sourceVoice)
	{
		m_sourceVoice->Start();
		m_playing = true;
	}
}

void XAudio2Voice::stop()
{
	if (m_sourceVoice)
	{
		m_sourceVoice->Stop();
		m_sourceVoice->FlushSourceBuffers();
		m_playing = false;
	}
}

void XAudio2Voice::pause()
{
	if (m_sourceVoice)
	{
		m_sourceVoice->Stop();
		// Don't flush buffers so resume continues from where we left off
	}
}

void XAudio2Voice::resume()
{
	if (m_sourceVoice)
		m_sourceVoice->Start();
}

void XAudio2Voice::setVolume(float volume)
{
	if (m_sourceVoice)
		m_sourceVoice->SetVolume(volume);
}

void XAudio2Voice::setPitch(float pitch)
{
	if (m_sourceVoice)
		m_sourceVoice->SetFrequencyRatio(pitch);
}

void XAudio2Voice::setLowPass(float frequency)
{
	if (!m_sourceVoice || frequency <= 0.0f)
		return;

	XAUDIO2_FILTER_PARAMETERS filter = {};
	filter.Type = LowPassFilter;
	filter.Frequency = (frequency < 1.0f) ? frequency : 1.0f;
	filter.OneOverQ = 1.0f;
	m_sourceVoice->SetFilterParameters(&filter);
}

void XAudio2Voice::setLooping(bool loop)
{
	m_looping = loop;
}

void XAudio2Voice::setPosition(float x, float y, float z)
{
	// Compute velocity from position delta (used for Doppler)
	if (m_hasLastPosition)
	{
		// Assume ~30 Hz update rate for velocity estimation
		constexpr float updateRate = 30.0f;
		m_velocity[0] = (x - m_position[0]) * updateRate;
		m_velocity[1] = (y - m_position[1]) * updateRate;
		m_velocity[2] = (z - m_position[2]) * updateRate;
	}
	m_prevPosition[0] = m_position[0];
	m_prevPosition[1] = m_position[1];
	m_prevPosition[2] = m_position[2];
	m_position[0] = x;
	m_position[1] = y;
	m_position[2] = z;
	m_hasLastPosition = true;
}

bool XAudio2Voice::submitBuffer(const uint8_t *data, uint32_t sizeBytes, bool endOfStream)
{
	if (!m_sourceVoice || !data || sizeBytes == 0)
		return false;

	XAUDIO2_BUFFER buf = {};
	buf.AudioBytes = sizeBytes;
	buf.pAudioData = data;
	if (endOfStream)
		buf.Flags = XAUDIO2_END_OF_STREAM;
	if (m_looping)
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;

	HRESULT hr = m_sourceVoice->SubmitSourceBuffer(&buf);
	return SUCCEEDED(hr);
}

void XAudio2Voice::apply3D(const void *x3dInstance, const void *listener, const void *emitter, uint32_t dstChannels, float minDistance)
{
	if (!m_sourceVoice || m_type != VOICE_3D)
		return;

	const auto *lst = static_cast<const X3DAUDIO_LISTENER *>(listener);
	const auto *emit = static_cast<const X3DAUDIO_EMITTER *>(emitter);
	float dx = m_position[0] - lst->Position.x;
	float dy = m_position[1] - lst->Position.y;
	float dz = m_position[2] - lst->Position.z;
	float dist = sqrtf(dx * dx + dy * dy + dz * dz);
	float minDist = (minDistance > 0.0f) ? minDistance : 1.0f;
	float maxDist = (emit->CurveDistanceScaler > minDist) ? emit->CurveDistanceScaler : minDist;
	float distanceGain = 1.0f;
	if (dist >= maxDist)
		distanceGain = 0.0f;
	else if (dist > minDist)
		distanceGain = minDist / dist;

	// Non-mono 3D sources: uniform attenuation only (no X3DAudio panning)
	if (m_channels != 1 || dstChannels == 0 || dstChannels > 8)
	{
		float coeffs[16] = {};
		uint32_t srcCh = m_channels;
		uint32_t dstCh = (dstChannels > 0 && dstChannels <= 8) ? dstChannels : 2;
		for (uint32_t s = 0; s < srcCh && s < dstCh; ++s)
			coeffs[s * dstCh + s] = distanceGain;
		if (srcCh * dstCh <= 16)
			m_sourceVoice->SetOutputMatrix(m_submixVoice, srcCh, dstCh, coeffs);
		return;
	}

	// Full X3DAudio DSP: panning + LPF + Doppler (mono sources)
	X3DAUDIO_DSP_SETTINGS dspSettings = {};
	float matrixCoefficients[8] = {};
	dspSettings.pMatrixCoefficients = matrixCoefficients;
	dspSettings.SrcChannelCount = 1;
	dspSettings.DstChannelCount = dstChannels;

	X3DAUDIO_EMITTER emitterCopy = *emit;
	emitterCopy.Position.x = m_position[0];
	emitterCopy.Position.y = m_position[1];
	emitterCopy.Position.z = m_position[2];
	emitterCopy.Velocity.x = m_velocity[0];
	emitterCopy.Velocity.y = m_velocity[1];
	emitterCopy.Velocity.z = m_velocity[2];
	emitterCopy.DopplerScaler = 1.0f;

	// Inner radius: smooth transition near emitter to avoid hard panning snaps
	emitterCopy.InnerRadius = minDist * 0.5f;
	emitterCopy.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;

	UINT32 calcFlags = X3DAUDIO_CALCULATE_MATRIX
		| X3DAUDIO_CALCULATE_LPF_DIRECT
		| X3DAUDIO_CALCULATE_DOPPLER;

	X3DAudioCalculate(
		*static_cast<const X3DAUDIO_HANDLE *>(x3dInstance),
		lst,
		&emitterCopy,
		calcFlags,
		&dspSettings);

	for (uint32_t i = 0; i < dstChannels; ++i)
		dspSettings.pMatrixCoefficients[i] *= distanceGain;

	m_sourceVoice->SetOutputMatrix(m_submixVoice, 1, dstChannels, dspSettings.pMatrixCoefficients);

	// Distance-based low-pass filter: distant sounds lose high frequencies
	{
		XAUDIO2_FILTER_PARAMETERS filter = {};
		filter.Type = LowPassFilter;
		// X3DAudio LPFDirectCoefficient: 1 = no filtering (near), 0 = full cutoff (far)
		// Convert to XAudio2 cutoff frequency using Microsoft's recommended formula
		float freq = 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient);
		filter.Frequency = (freq > 0.01f) ? freq : 0.01f;  // clamp to avoid silence
		filter.OneOverQ = 1.0f;
		m_sourceVoice->SetFilterParameters(&filter);
	}

	// Doppler pitch shift
	if (dspSettings.DopplerFactor > 0.0f && dspSettings.DopplerFactor < 4.0f)
		m_sourceVoice->SetFrequencyRatio(dspSettings.DopplerFactor);
}

bool XAudio2Voice::isPlaying() const
{
	if (!m_sourceVoice)
		return false;
	XAUDIO2_VOICE_STATE state;
	m_sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
	return state.BuffersQueued > 0;
}

bool XAudio2Voice::isStopped() const
{
	return !isPlaying();
}

#endif // _WIN32
