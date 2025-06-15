/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

// FILE: OpenALAudioFileCache.h //////////////////////////////////////////////////////////////////////////
// OpenALAudioCache implementation
// Author: Stephan Vedder, April 2025
#pragma once

#include "OpenALDevice/OpenALAudioManager.h"

#ifndef RTS_HAS_FFMPEG
#error "RTS_HAS_FFMPEG must be defined to use the OpenALAudioFileCache."
#endif

#include "VideoDevice/FFmpeg/FFmpegFile.h"

#include <mutex>
#include <unordered_map>

class OpenALAudioStream;
struct PlayingAudio
{
	union
	{
		ALuint m_source;
		OpenALAudioStream* m_stream;
	};

	PlayingAudioType m_type;
	AudioEventRTS* m_audioEventRTS;
	FFmpegFile* m_ffmpegFile;
	// The created OpenAL buffer handle for this file
	ALuint m_bufferHandle;
	Bool m_requestStop;
	Bool m_cleanupAudioEventRTS;
	Int m_framesFaded;

	PlayingAudio() :
		m_type(PAT_INVALID),
		m_audioEventRTS(NULL),
		m_ffmpegFile(NULL),
		m_bufferHandle(0),
		m_requestStop(false),
		m_cleanupAudioEventRTS(true),
		m_stream(NULL),
		m_framesFaded(0)
	{ }
};

struct OpenAudioFile
{
	ALuint m_buffer = 0;
	FFmpegFile* m_ffmpegFile = NULL;
	UnsignedInt m_openCount;
	UnsignedInt m_fileSize;
	UnsignedInt m_channels = 0;
	UnsignedInt m_bitsPerSample = 0;
	UnsignedInt m_freq = 0;

	// Note: OpenAudioFile does not own this m_eventInfo, and should not delete it.
	const AudioEventInfo *m_eventInfo;	// Not mutable, unlike the one on AudioEventRTS.
	int m_totalSamples = 0;
	float m_duration = 0.0f;
};

struct OpenFileInfo
{
	AsciiString* filename;
	AudioEventRTS* event;

	OpenFileInfo(AsciiString *filename) : filename(filename),
																				event(NULL)
	{
	}

	OpenFileInfo(AudioEventRTS *event) : filename(NULL),
																			 event(event)
	{
	}
};

typedef std::hash_map< AsciiString, OpenAudioFile, rts::hash<AsciiString>, rts::equal_to<AsciiString> > OpenFilesHash;
typedef OpenFilesHash::iterator OpenFilesHashIt;

class OpenALAudioFileCache
{
	public:
		OpenALAudioFileCache();

		// Protected by mutex
		virtual ~OpenALAudioFileCache();
		ALuint getBufferForFile(const OpenFileInfo& fileToOpenFrom);
		void closeBuffer(ALuint bufferToClose);
		void setMaxSize(UnsignedInt size);
		float getBufferLength(ALuint handle);
		// End Protected by mutex

		// Note: These functions should be used for informational purposes only. For speed reasons,
		// they are not protected by the mutex, so they are not guarenteed to be valid if called from
		// outside the audio cache. They should be used as a rough estimate only.
		UnsignedInt getCurrentlyUsedSize() const { return m_currentlyUsedSize; }
		UnsignedInt getMaxSize() const { return m_maxSize; }

		static void getWaveData(void* wave_data,
			uint8_t*& data,
			UnsignedInt& size,
			UnsignedInt& freq,
			UnsignedInt& channels,
			UnsignedInt& bitsPerSample);

	protected:
		void releaseOpenAudioFile(OpenAudioFile* fileToRelease);

		// This function will return TRUE if it was able to free enough space, and FALSE otherwise.
		Bool freeEnoughSpaceForSample(const OpenAudioFile& sampleThatNeedsSpace);

		// FFmpeg related
		Bool decodeFFmpeg(OpenAudioFile* fileToDecode);

		OpenFilesHash m_openFiles;
		UnsignedInt m_currentlyUsedSize;
		UnsignedInt m_maxSize;
};
