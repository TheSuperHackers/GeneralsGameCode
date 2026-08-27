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

#include "AudioBus.h"
#include "AudioHandle.h"

///
/// AudioEvent — game-agnostic audio play request.
///
/// This is the unified type used by all games to request audio playback.
/// Replaces both AudioEventRTS (for new direct-play API) and
/// AudibleSoundClass (Renegade) at the engine interface boundary.
///
/// Inspired by modern audio middleware (FMOD, Wwise): an event describes
/// WHAT to play and HOW, while AudioHandle tracks the playing instance.
///
struct AudioEvent
{
	/// Audio file to play. Must remain valid until playAudioEvent() returns.
	const char *filename = nullptr;

	AudioBus bus = AudioBus::Sound;          // Volume category / bus routing

	// 3D positioning (if is3D == false, plays as 2D head-relative)
	bool is3D = false;
	float posX = 0.0f;
	float posY = 0.0f;
	float posZ = 0.0f;
	float minDistance = 1.0f;                // Distance at which volume is 100%
	float maxDistance = 300.0f;              // Distance beyond which volume is 0%

	// Playback parameters
	float volume = 1.0f;                    // 0.0 – 1.0
	float priority = 0.5f;                  // 0.0 (lowest) – 1.0 (highest)
	float pitchShift = 1.0f;                // 1.0 = normal speed
	float lowPassFreq = 0.0f;
	int loopCount = 1;                      // 1 = play once, 0 = infinite loop

	// Fade (in seconds, 0 = instant)
	float fadeInSeconds = 0.0f;
	float fadeOutSeconds = 0.0f;
};

/// Completion callback signature.
/// Called when a playing audio event finishes (not called for looping sounds
/// unless they are explicitly stopped).
typedef void (*AudioCompletionCallback)(AudioHandle handle, void *userData);

