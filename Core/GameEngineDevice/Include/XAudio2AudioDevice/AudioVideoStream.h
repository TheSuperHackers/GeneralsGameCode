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

// Backend-neutral interface for video audio streaming.
// Implementations use an XAudio2 source voice.
// provide the concrete queueing and playback logic.
class AudioVideoStream
{
public:
	virtual ~AudioVideoStream() = default;

	virtual bool init() = 0;
	virtual void reset() = 0;
	virtual void queueBuffer(const void *data, int dataSize, int sampleRate, int channels, int bitsPerSample) = 0;
	virtual void play() = 0;
	virtual void stop() = 0;
	virtual void update() = 0;
	virtual bool isPlaying() const = 0;
};

