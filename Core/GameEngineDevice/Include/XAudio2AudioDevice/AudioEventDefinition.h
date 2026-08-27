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

///
/// AudioEventDefinition — data-driven description of a sound event.
///
/// This is the engine-level type shared by all games. Each game registers its
/// sound definitions (Renegade's AudibleSoundDefinitionClass, Generals'
/// AudioEventInfo) as AudioEventDefinitions at load time. The AudioEngine
/// stores them in a registry and pre-caches the audio data.
///
/// At runtime, game code plays sounds by event name:
///   theAudio().playEvent("weapon_pistol_fire", x, y, z);
///
/// The engine looks up the definition, selects a file (with optional
/// randomization), and plays it immediately from cache.
///

#include "AudioBus.h"

#include <cstdint>
#include <string>
#include <vector>

/// Priority levels for audio events.
enum class AudioPriorityLevel : int
{
	Lowest  = 0,
	Low     = 1,
	Normal  = 2,
	High    = 3,
	Critical = 4,

	Count
};

/// Sound type flags — used for game-level filtering (e.g. "only play if visible").
namespace AudioTypeFlags
{
	constexpr uint32_t UI       = 0x0001;
	constexpr uint32_t World    = 0x0002;
	constexpr uint32_t Shrouded = 0x0004;
	constexpr uint32_t Global   = 0x0008;
	constexpr uint32_t Voice    = 0x0010;
	constexpr uint32_t Player   = 0x0020;
	constexpr uint32_t Allies   = 0x0040;
	constexpr uint32_t Enemies  = 0x0080;
	constexpr uint32_t Everyone = 0x0100;
}

/// Control flags for audio event playback behavior.
namespace AudioControlFlags
{
	constexpr uint32_t Loop      = 0x0001;
	constexpr uint32_t Random    = 0x0002; ///< Pick randomly from filenames
	constexpr uint32_t All       = 0x0004; ///< Play all files sequentially
	constexpr uint32_t PostDelay = 0x0008;
	constexpr uint32_t Interrupt = 0x0010; ///< Can interrupt lower-priority sounds
}

struct AudioEventDefinition
{
	/// Unique event name (e.g. "weapon_pistol_fire", "GDI_Harvester_VoiceAttack").
	std::string name;

	/// One or more audio files. If multiple, selection depends on control flags
	/// (Random = pick one at random, All = play sequentially, else first).
	std::vector<std::string> filenames;

	AudioBus bus = AudioBus::Sound;
	bool is3D = false;

	// Volume
	float volume = 1.0f;
	float volumeShift = 0.0f;     ///< Random ± volume adjustment per play
	float minVolume = 0.0f;       ///< Floor clamp (useful when fading)

	// 3D distance
	float minDistance = 1.0f;     ///< Full-volume radius
	float maxDistance = 300.0f;   ///< Silence radius

	// Pitch
	float pitchMin = 1.0f;       ///< Minimum pitch multiplier (1.0 = normal)
	float pitchMax = 1.0f;       ///< Maximum pitch multiplier

	// Priority and limits
	AudioPriorityLevel priority = AudioPriorityLevel::Normal;
	int limit = 0;                ///< Max simultaneous instances (0 = unlimited)
	int loopCount = 1;            ///< 1 = play once, 0 = infinite loop

	// Control and type flags
	uint32_t control = 0;         ///< AudioControlFlags bitmask
	uint32_t type = 0;            ///< AudioTypeFlags bitmask

	// Delay between repeats (milliseconds)
	int delayMinMs = 0;
	int delayMaxMs = 0;

	// Low-pass filter
	float lowPassFreq = 0.0f;    ///< 0 = no filter

	/// Convert priority enum to 0.0–1.0 float for AudioEvent.
	float priorityAsFloat() const
	{
		return static_cast<float>(priority) / static_cast<float>(AudioPriorityLevel::Critical);
	}

	/// Check if this event loops.
	bool isLooping() const
	{
		return (control & AudioControlFlags::Loop) != 0 || loopCount == 0;
	}

	/// Check if random file selection is enabled.
	bool isRandom() const
	{
		return (control & AudioControlFlags::Random) != 0;
	}
};


