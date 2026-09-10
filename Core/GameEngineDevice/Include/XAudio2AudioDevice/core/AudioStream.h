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
*/

#pragma once

#include <cstdint>

/// Format description for interleaved PCM submitted to an AudioStream.
struct AudioStreamFormat
{
	std::uint32_t sampleRate = 0;
	std::uint16_t channels = 0;
	std::uint16_t bitsPerSample = 0;
};

/// Generic streaming PCM output. The producer may reuse its input memory after
/// Queue returns; implementations retain any data required by the device.
class AudioStream
{
public:
	virtual ~AudioStream() = default;

	virtual bool Queue(const void *data, std::uint32_t sizeBytes, AudioStreamFormat format) = 0;
	virtual void Start() noexcept = 0;
	virtual void Pause() noexcept = 0;
	virtual void Resume() noexcept = 0;
	virtual void Stop() noexcept = 0;
	virtual void Reset() noexcept = 0;
	virtual void Update() noexcept = 0;
	virtual bool Is_Playing() const noexcept = 0;
};
