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
#include <vector>

class AudioFileProvider;

/// Decodes audio files (via FFmpeg) into raw float32 PCM at 48 kHz for XAudio2.
/// Thread-safe: cache is protected by a mutex for background decode support.
class XAudio2Decoder
{
public:
	static constexpr uint32_t OUTPUT_SAMPLE_RATE = 48000;
	static constexpr uint32_t OUTPUT_BITS_PER_SAMPLE = 32; // IEEE float

	struct DecodedBuffer
	{
		uint8_t *data;
		uint32_t sizeBytes;
		uint32_t channels;
		uint32_t sampleRate;
		uint32_t bitsPerSample;
	};

	struct ProviderIO;

	/// Decode audio file to PCM via AudioFileProvider. Returns cached result on subsequent calls.
	/// Thread-safe: may be called from any thread.
	static DecodedBuffer decode(const char *filename, AudioFileProvider *provider);

	/// Decode from an in-memory buffer. Thread-safe (no file I/O).
	static DecodedBuffer decodeFromMemory(const char *filename, const std::vector<uint8_t> &fileData);

	/// Check if a file is already decoded and cached. Thread-safe.
	static bool isCached(const char *filename);

	/// Try to get a cached decoded buffer. Returns {nullptr,...} if not cached. Thread-safe.
	static DecodedBuffer tryGetCached(const char *filename);

	/// Get duration of decoded audio in milliseconds.
	static float getDurationMs(const char *filename, AudioFileProvider *provider);

	/// Free all cached PCM data.
	static void clearCache();
};


