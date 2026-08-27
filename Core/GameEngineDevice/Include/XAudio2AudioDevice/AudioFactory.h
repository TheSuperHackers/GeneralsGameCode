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

#include "AudioTypes.h"
#include <memory>

class AudioFileProvider;
class AudioSystem;

/// Configuration for backend selection and dependency injection.
struct AudioBackendConfig
{
	AudioBackendType preferred = AudioBackendType::Auto;
	bool allowFallback = true;
	bool headless = false;

	/// File provider for loading audio assets.
	/// The caller retains ownership; must outlive the created AudioSystem.
	AudioFileProvider *fileProvider = nullptr;
};

/// Create an AudioSystem instance based on the given configuration.
///
/// Selection policy:
///   - headless=true → NullAudioSystem
///   - Windows default: XAudio2 → Null
///   - Non-Windows default: Null
///   - Explicit preferred backend is tried first, then fallback chain.
///
std::unique_ptr<AudioSystem> createAudioSystem(const AudioBackendConfig &config);

