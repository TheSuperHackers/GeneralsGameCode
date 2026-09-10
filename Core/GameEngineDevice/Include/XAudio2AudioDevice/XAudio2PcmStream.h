/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
*/

#pragma once

#include "core/AudioStream.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

struct IXAudio2;
struct IXAudio2SourceVoice;
struct IXAudio2SubmixVoice;

/// XAudio2 implementation of the generic PCM stream contract.
class XAudio2PcmStream final : public AudioStream
{
public:
	XAudio2PcmStream(IXAudio2 *xaudio, IXAudio2SubmixVoice *submix) noexcept;
	~XAudio2PcmStream() override;

	bool Queue(const void *data, std::uint32_t sizeBytes, AudioStreamFormat format) override;
	void Start() noexcept override;
	void Pause() noexcept override;
	void Resume() noexcept override;
	void Stop() noexcept override;
	void Reset() noexcept override;
	void Update() noexcept override;
	bool Is_Playing() const noexcept override;

private:
	static constexpr std::size_t Buffer_Count = 16;

	bool Ensure_Voice(AudioStreamFormat format);
	void Destroy_Voice() noexcept;

	struct Buffer final
	{
		std::vector<std::uint8_t> data;
	};

	IXAudio2 *m_xaudio = nullptr;
	IXAudio2SubmixVoice *m_submix = nullptr;
	IXAudio2SourceVoice *m_source_voice = nullptr;
	AudioStreamFormat m_format{};
	std::array<Buffer, Buffer_Count> m_buffers{};
	std::size_t m_read_index = 0;
	std::size_t m_queued_count = 0;
	bool m_started = false;
	bool m_paused = false;
};
