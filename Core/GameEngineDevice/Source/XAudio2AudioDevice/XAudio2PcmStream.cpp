#if defined(_WIN32) && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0602)

#include "XAudio2PcmStream.h"

#include <xaudio2.h>

#include <algorithm>
#include <cstring>

XAudio2PcmStream::XAudio2PcmStream(IXAudio2 *xaudio, IXAudio2SubmixVoice *submix) noexcept
	: m_xaudio(xaudio)
	, m_submix(submix)
{
}

XAudio2PcmStream::~XAudio2PcmStream()
{
	Destroy_Voice();
}

bool XAudio2PcmStream::Ensure_Voice(AudioStreamFormat format)
{
	if (m_xaudio == nullptr || format.sampleRate == 0 || format.channels == 0
		|| format.channels > 32 || format.bitsPerSample == 0
		|| format.bitsPerSample % 8 != 0)
		return false;

	if (m_source_voice != nullptr
		&& m_format.sampleRate == format.sampleRate
		&& m_format.channels == format.channels
		&& m_format.bitsPerSample == format.bitsPerSample)
		return true;

	const bool was_started = m_started;
	const bool was_paused = m_paused;
	Destroy_Voice();
	m_started = was_started;
	m_paused = was_paused;

	WAVEFORMATEX wave_format{};
	wave_format.wFormatTag = WAVE_FORMAT_PCM;
	wave_format.nChannels = format.channels;
	wave_format.nSamplesPerSec = format.sampleRate;
	wave_format.wBitsPerSample = format.bitsPerSample;
	wave_format.nBlockAlign = static_cast<WORD>(
		static_cast<std::uint32_t>(wave_format.nChannels) * wave_format.wBitsPerSample / 8u);
	wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;

	XAUDIO2_SEND_DESCRIPTOR send{};
	send.pOutputVoice = m_submix;
	XAUDIO2_VOICE_SENDS sends{};
	sends.SendCount = m_submix != nullptr ? 1u : 0u;
	sends.pSends = m_submix != nullptr ? &send : nullptr;

	if (FAILED(m_xaudio->CreateSourceVoice(
		&m_source_voice, &wave_format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr,
		m_submix != nullptr ? &sends : nullptr)))
		return false;

	m_format = format;
	return true;
}

bool XAudio2PcmStream::Queue(const void *data, std::uint32_t sizeBytes, AudioStreamFormat format)
{
	if (data == nullptr || sizeBytes == 0)
		return false;

	Update();
	if (m_queued_count >= Buffer_Count || !Ensure_Voice(format))
		return false;

	Buffer &buffer = m_buffers[(m_read_index + m_queued_count) % Buffer_Count];
	if (buffer.data.size() < sizeBytes)
		buffer.data.resize(sizeBytes);
	std::memcpy(buffer.data.data(), data, sizeBytes);

	XAUDIO2_BUFFER xaudio_buffer{};
	xaudio_buffer.AudioBytes = sizeBytes;
	xaudio_buffer.pAudioData = buffer.data.data();
	if (FAILED(m_source_voice->SubmitSourceBuffer(&xaudio_buffer)))
		return false;

	++m_queued_count;
	if (m_started && !m_paused)
		m_source_voice->Start(0);
	return true;
}

void XAudio2PcmStream::Start() noexcept
{
	m_started = true;
	m_paused = false;
	if (m_source_voice != nullptr && m_queued_count != 0)
		m_source_voice->Start(0);
}

void XAudio2PcmStream::Pause() noexcept
{
	m_paused = true;
	if (m_source_voice != nullptr)
		m_source_voice->Stop(0);
}

void XAudio2PcmStream::Resume() noexcept
{
	m_paused = false;
	m_started = true;
	if (m_source_voice != nullptr && m_queued_count != 0)
		m_source_voice->Start(0);
}

void XAudio2PcmStream::Stop() noexcept
{
	if (m_source_voice != nullptr) {
		m_source_voice->Stop(0);
		m_source_voice->FlushSourceBuffers();
	}
	m_read_index = 0;
	m_queued_count = 0;
	m_started = false;
	m_paused = false;
}

void XAudio2PcmStream::Reset() noexcept
{
	Stop();
}

void XAudio2PcmStream::Update() noexcept
{
	if (m_source_voice == nullptr || m_queued_count == 0)
		return;

	XAUDIO2_VOICE_STATE state{};
	m_source_voice->GetState(&state, 0);
	const std::size_t queued = std::min<std::size_t>(state.BuffersQueued, m_queued_count);
	const std::size_t completed = m_queued_count - queued;
	m_read_index = (m_read_index + completed) % Buffer_Count;
	m_queued_count = queued;
}

bool XAudio2PcmStream::Is_Playing() const noexcept
{
	if (m_source_voice == nullptr)
		return false;

	XAUDIO2_VOICE_STATE state{};
	m_source_voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
	return state.BuffersQueued != 0;
}

void XAudio2PcmStream::Destroy_Voice() noexcept
{
	if (m_source_voice != nullptr) {
		m_source_voice->Stop(0);
		m_source_voice->FlushSourceBuffers();
		m_source_voice->DestroyVoice();
		m_source_voice = nullptr;
	}
	m_format = {};
	m_read_index = 0;
	m_queued_count = 0;
	m_started = false;
	m_paused = false;
}

#endif // _WIN32
