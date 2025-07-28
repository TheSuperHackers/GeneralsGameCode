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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// EA Pacific
// John McDonald, Jr
// Do not distribute

#pragma once

#ifndef _AUDIOSETTINGS_H_
#define _AUDIOSETTINGS_H_

#include "Common/AsciiString.h"

enum { MAX_HW_PROVIDERS = 4 };

// TheSuperHackers @tweak xezon 23/07/2025 Adds setting to modify the volume of money deposit and withdraw sounds

struct AudioSettings
{
	AudioSettings()
#if RTS_GENERALS
		: m_defaultMoneyTransactionVolume(1.0f)
#elif RTS_ZEROHOUR
		: m_defaultMoneyTransactionVolume(0.0f) // Uses zero volume by default because originally the money sounds did not work in Zero Hour
#endif
	{
	}

	AsciiString m_audioRoot;
	AsciiString m_soundsFolder;
	AsciiString m_musicFolder;
	AsciiString m_streamingFolder;
	AsciiString m_soundsExtension;
	Bool m_useDigital;
	Bool m_useMidi;
	Int m_outputRate;
	Int m_outputBits;
	Int m_outputChannels;
	Int m_sampleCount2D;
	Int m_sampleCount3D;
	Int m_streamCount;
	Int m_globalMinRange;
	Int m_globalMaxRange;
	Int m_drawableAmbientFrames;
	Int m_fadeAudioFrames;
	UnsignedInt m_maxCacheSize;

	Real m_minVolume;		// At volumes less than this, the sample will be culled.
	
	AsciiString m_preferred3DProvider[MAX_HW_PROVIDERS + 1];

	//Defaults actually don't ever get changed!
	Real m_relative2DVolume;		//2D volume compared to 3D
	Real m_defaultSoundVolume;
	Real m_default3DSoundVolume;
	Real m_defaultSpeechVolume;
	Real m_defaultMusicVolume;
	Real m_defaultMoneyTransactionVolume;
	UnsignedInt m_defaultSpeakerType2D;
	UnsignedInt m_defaultSpeakerType3D;

	//If you want to change a value, store it somewhere else (like here)
	Real m_preferredSoundVolume;
	Real m_preferred3DSoundVolume;
	Real m_preferredSpeechVolume;
	Real m_preferredMusicVolume;
	Real m_preferredMoneyTransactionVolume;

	//The desired altitude of the microphone to improve panning relative to terrain.
	Real m_microphoneDesiredHeightAboveTerrain;

	//When tracing a line between the ground look-at-point and the camera, we want
	//to ensure a maximum percentage, so the microphone never goes behind the camera.
	Real m_microphoneMaxPercentageBetweenGroundAndCamera;

	//Handles changing sound volume whenever the camera is close to the microphone.
  Real m_zoomMinDistance;			//If we're closer than the minimum distance, then apply the full bonus no matter how close.
  Real m_zoomMaxDistance;			//The maximum distance from microphone we need to be before benefiting from any bonus.
	
  //NOTE: The higher this value is, the lower normal sounds will be! If you specify a sound volume value of 25%, then sounds will play
	//between 75% and 100%, not 100% to 125%!
  Real m_zoomSoundVolumePercentageAmount;	//The amount of sound volume dedicated to zooming.
};

#endif // _AUDIOSETTINGS_H_
