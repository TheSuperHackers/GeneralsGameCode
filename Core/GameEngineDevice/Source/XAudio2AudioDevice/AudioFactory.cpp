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

#include "AudioFactory.h"

#include "Lib/BaseType.h"

// Include all backends
#if defined(_WIN32)
#include "XAudio2AudioSystem.h"
#endif
#include "NullAudioSystem.h"

#include <cstdlib>
#include <string>

/// Create an audio system with fallback chain:
///   Windows:  XAudio2 → Null
///   Other:    Null
std::unique_ptr<AudioSystem> createAudioSystem(const AudioBackendConfig &config)
{
	auto applyConfig = [&](AudioSystem *sys) {
		sys->setFileProvider(config.fileProvider);
	};

	if (config.headless)
	{
		auto sys = std::make_unique<NullAudioSystem>();
		applyConfig(sys.get());
		return sys;
	}

	// Check for explicit override via environment variable
	AudioBackendType requested = config.preferred;
	const char *envOverride = std::getenv("SAGE_AUDIO_BACKEND");
	if (envOverride)
	{
		std::string env(envOverride);
		if (env == "xaudio2")
			requested = AudioBackendType::XAudio2;
		else if (env == "null")
		{
			auto sys = std::make_unique<NullAudioSystem>();
			applyConfig(sys.get());
			return sys;
		}
	}

#if defined(_WIN32) && (!defined(_WIN32_WINNT) || _WIN32_WINNT >= 0x0602)
	// Windows default: try XAudio2 first
	if (requested == AudioBackendType::Auto || requested == AudioBackendType::XAudio2)
	{
		auto xaudio = std::make_unique<XAudio2AudioSystem>();
		applyConfig(xaudio.get());
		xaudio->openDevice();
		if (xaudio->getDevice() != nullptr)
		{
			DEBUG_LOG(("Audio: Using XAudio2 backend\n"));
			return xaudio;
		}
		DEBUG_LOG(("Audio: XAudio2 init failed, falling back to Null\n"));
	}
#endif

	// Fallback to null
	DEBUG_LOG(("Audio: Using Null backend (no audio)\n"));
	auto sys = std::make_unique<NullAudioSystem>();
	applyConfig(sys.get());
	return sys;
}
