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

// FILE: MilesAudioFileCache.h //////////////////////////////////////////////////////////////////////////
// MilesAudioManager implementation
// Author: John K. McDonald, July 2002
#pragma once

#include "MilesAudioDevice/MilesAudioManager.h"

struct PlayingAudio
{
	union
	{
		HSAMPLE m_sample;
		H3DSAMPLE m_3DSample;
		HSTREAM m_stream;
	};

	PlayingAudioType m_type;
	volatile PlayingStatus m_status;	// This member is adjusted by another running thread.
	AudioEventRTS *m_audioEventRTS;
	void *m_file;		// The file that was opened to play this
	Bool m_requestStop;
	Bool m_cleanupAudioEventRTS;
	Int m_framesFaded;

	PlayingAudio() :
		m_type(PAT_INVALID),
		m_audioEventRTS(NULL),
		m_requestStop(false),
		m_cleanupAudioEventRTS(true),
		m_sample(NULL), 
		m_framesFaded(0)
	{ }
};

struct OpenAudioFile
{
	AILSOUNDINFO m_soundInfo;
	void *m_file;
	UnsignedInt m_openCount;
	UnsignedInt m_fileSize;

	Bool m_compressed;	// if the file was compressed, then we need to free it with a miles function.

	// Note: OpenAudioFile does not own this m_eventInfo, and should not delete it.
	const AudioEventInfo *m_eventInfo;	// Not mutable, unlike the one on AudioEventRTS.
};

typedef std::hash_map< AsciiString, OpenAudioFile, rts::hash<AsciiString>, rts::equal_to<AsciiString> > OpenFilesHash;
typedef OpenFilesHash::iterator OpenFilesHashIt;

class MilesAudioFileCache
{
	public:
		MilesAudioFileCache();

		// Protected by mutex
		virtual ~MilesAudioFileCache();
		void *openFile( AudioEventRTS *eventToOpenFrom );
		void closeFile( void *fileToClose );
		void setMaxSize( UnsignedInt size );
		// End Protected by mutex

		// Note: These functions should be used for informational purposes only. For speed reasons,
		// they are not protected by the mutex, so they are not guarenteed to be valid if called from
		// outside the audio cache. They should be used as a rough estimate only.
		UnsignedInt getCurrentlyUsedSize() const { return m_currentlyUsedSize; }
		UnsignedInt getMaxSize() const { return m_maxSize; }

	protected:
		void releaseOpenAudioFile( OpenAudioFile *fileToRelease );

		// This function will return TRUE if it was able to free enough space, and FALSE otherwise.
		Bool freeEnoughSpaceForSample(const OpenAudioFile& sampleThatNeedsSpace);

		OpenFilesHash m_openFiles;
		UnsignedInt m_currentlyUsedSize;
		UnsignedInt m_maxSize;
		HANDLE m_mutex;
		const char *m_mutexName;
};
