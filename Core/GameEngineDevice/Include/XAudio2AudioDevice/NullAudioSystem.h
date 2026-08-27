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

#include "AudioSystem.h"

/// No-op audio backend for headless mode.
class NullAudioSystem : public AudioSystem
{
public:
	void openDevice() override {}
	void closeDevice() override {}
	void *getDevice() override { return nullptr; }

	void init() override {}
	void postProcessLoad() override {}
	void reset() override {}
	void update() override {}

	uint32_t getNum2DSamples() const override { return 0; }
	uint32_t getNum3DSamples() const override { return 0; }
	uint32_t getNumStreams() const override { return 0; }

	AudioBackendType getBackendType() const override { return AudioBackendType::Null; }

	AudioHandle playAudioEvent(const AudioEvent & /*event*/) override { return AUDIO_HANDLE_INVALID; }
	void stopAudioEvent(AudioHandle /*handle*/) override {}
	void pauseAudioEvent(AudioHandle /*handle*/) override {}
	void resumeAudioEvent(AudioHandle /*handle*/) override {}
	void setAudioPosition(AudioHandle /*handle*/, float /*x*/, float /*y*/, float /*z*/) override {}
	void setAudioVolume(AudioHandle /*handle*/, float /*volume*/) override {}
	void setAudioPitch(AudioHandle /*handle*/, float /*pitch*/) override {}
	bool isAudioPlaying(AudioHandle /*handle*/) const override { return false; }
	void setCompletionCallback(AudioHandle /*handle*/, AudioCompletionCallback /*cb*/, void * /*ud*/) override {}
	void setBusVolume(AudioBus /*bus*/, float /*volume*/) override {}
	float getBusVolume(AudioBus /*bus*/) const override { return 0.0f; }
	void setBusEnabled(AudioBus /*bus*/, bool /*enabled*/) override {}
	bool isBusEnabled(AudioBus /*bus*/) const override { return false; }
	float getFileDurationMs(const char * /*filename*/) const override { return 0.0f; }
	void setListenerPosition(float /*posX*/, float /*posY*/, float /*posZ*/,
		float /*fwdX*/, float /*fwdY*/, float /*fwdZ*/,
		float /*upX*/, float /*upY*/, float /*upZ*/) override {}
};


