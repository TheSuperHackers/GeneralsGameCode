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

// FILE: MilesAudioFileCache.cpp 
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  MilesAudioFileCache.cpp                                       */
/* Created:    John K. McDonald, Jr., 3/21/2002                              */
/* Desc:       This is the implementation for the MilesAudioManager, which   */
/*						 interfaces with the Miles Sound System.             */
/* Revision History:                                                         */
/*		7/18/2002 : Initial creation                                         */
/*      4/29/2025 : Moved AudioFileCache into a separate file and renamed    */
/*---------------------------------------------------------------------------*/

#include "MilesAudioFileCache.h"

#include "Common/AudioEventInfo.h"
#include "Common/AudioEventRTS.h"
#include "Common/File.h"
#include "Common/FileSystem.h"
#include "Common/ScopedMutex.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MilesAudioFileCache::MilesAudioFileCache() : m_maxSize(0), m_currentlyUsedSize(0), m_mutexName("AudioFileCacheMutex")
{
	m_mutex = CreateMutex(NULL, FALSE, m_mutexName);
}

//-------------------------------------------------------------------------------------------------
MilesAudioFileCache::~MilesAudioFileCache()
{
	{
		ScopedMutex mut(m_mutex);

		// Free all the samples that are open.
		OpenFilesHashIt it;
		for ( it = m_openFiles.begin(); it != m_openFiles.end(); ++it ) {
			if (it->second.m_openCount > 0) {
				DEBUG_CRASH(("Sample '%s' is still playing, and we're trying to quit.\n", it->second.m_eventInfo->m_audioName.str()));
			}

			releaseOpenAudioFile(&it->second);
			// Don't erase it from the map, cause it makes this whole process way more complicated, and 
			// we're about to go away anyways.
		}
	}

	CloseHandle(m_mutex);
}

//-------------------------------------------------------------------------------------------------
void *MilesAudioFileCache::openFile( AudioEventRTS *eventToOpenFrom )
{
	// Protect the entire openFile function
	ScopedMutex mut(m_mutex);

	AsciiString strToFind;
	switch (eventToOpenFrom->getNextPlayPortion())
	{
		case PP_Attack:
			strToFind = eventToOpenFrom->getAttackFilename();
			break;
		case PP_Sound:
			strToFind = eventToOpenFrom->getFilename();
			break;
		case PP_Decay:
			strToFind = eventToOpenFrom->getDecayFilename();
			break;
		case PP_Done:
			return NULL;
	}

	OpenFilesHash::iterator it;
	it = m_openFiles.find(strToFind);

	if (it != m_openFiles.end()) {
		++it->second.m_openCount;
		return it->second.m_file;
	}

	// Couldn't find the file, so actually open it.
	File *file = TheFileSystem->openFile(strToFind.str());
	if (!file) {
		DEBUG_ASSERTLOG(strToFind.isEmpty(), ("Missing Audio File: '%s'\n", strToFind.str()));
		return NULL;
	}

	UnsignedInt fileSize = file->size();
	char* buffer = file->readEntireAndClose();

	OpenAudioFile openedAudioFile;
	openedAudioFile.m_eventInfo = eventToOpenFrom->getAudioEventInfo();

	AILSOUNDINFO soundInfo;
	AIL_WAV_info(buffer, &soundInfo);

	if (eventToOpenFrom->isPositionalAudio()) {
		if (soundInfo.channels > 1) {
			DEBUG_CRASH(("Requested Positional Play of audio '%s', but it is in stereo.", strToFind.str()));
			delete [] buffer;
			return NULL;
		}
	}
	
	if (soundInfo.format == WAVE_FORMAT_IMA_ADPCM) {
		void *decompressFileBuffer;
		U32 newFileSize;
		AIL_decompress_ADPCM(&soundInfo, &decompressFileBuffer, &newFileSize);
		fileSize = newFileSize;
		openedAudioFile.m_compressed = TRUE;
		delete [] buffer;
		openedAudioFile.m_file = decompressFileBuffer;
		openedAudioFile.m_soundInfo = soundInfo;
		openedAudioFile.m_openCount = 1;
	} else if (soundInfo.format == WAVE_FORMAT_PCM) {
		openedAudioFile.m_compressed = FALSE;
		openedAudioFile.m_file = buffer;
		openedAudioFile.m_soundInfo = soundInfo;
		openedAudioFile.m_openCount = 1;
	} else {
		DEBUG_CRASH(("Unexpected compression type in '%s'\n", strToFind.str()));
		// prevent leaks
		delete [] buffer;
		return NULL;
	}

	openedAudioFile.m_fileSize = fileSize;
	m_currentlyUsedSize += openedAudioFile.m_fileSize;
	if (m_currentlyUsedSize > m_maxSize) {
		// We need to free some samples, or we're not going to be able to play this sound.
		if (!freeEnoughSpaceForSample(openedAudioFile)) {
			m_currentlyUsedSize -= openedAudioFile.m_fileSize;
			releaseOpenAudioFile(&openedAudioFile);
			return NULL;
		}
	}

	m_openFiles[strToFind] = openedAudioFile;
	return openedAudioFile.m_file;
}

//-------------------------------------------------------------------------------------------------
void MilesAudioFileCache::closeFile( void *fileToClose )
{
	if (!fileToClose) {
		return;
	}

	// Protect the entire closeFile function
	ScopedMutex mut(m_mutex);

	OpenFilesHash::iterator it;
	for ( it = m_openFiles.begin(); it != m_openFiles.end(); ++it ) {
		if ( it->second.m_file == fileToClose ) {
			--it->second.m_openCount;
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void MilesAudioFileCache::setMaxSize( UnsignedInt size )
{
	// Protect the function, in case we're trying to use this value elsewhere.
	ScopedMutex mut(m_mutex);

	m_maxSize = size;
}

//-------------------------------------------------------------------------------------------------
void MilesAudioFileCache::releaseOpenAudioFile( OpenAudioFile *fileToRelease )
{
	if (fileToRelease->m_openCount > 0) {
		// This thing needs to be terminated IMMEDIATELY.
		TheAudio->closeAnySamplesUsingFile(fileToRelease->m_file);
	}

	if (fileToRelease->m_file) {
		if (fileToRelease->m_compressed) {
			// Files read in via AIL_decompress_ADPCM must be freed with AIL_mem_free_lock. 
			AIL_mem_free_lock(fileToRelease->m_file);
		} else { 
			// Otherwise, we read it, we own it, blow it away.
			delete [] fileToRelease->m_file;
		}
		fileToRelease->m_file = NULL;
		fileToRelease->m_eventInfo = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
Bool MilesAudioFileCache::freeEnoughSpaceForSample(const OpenAudioFile& sampleThatNeedsSpace)
{
	
	Int spaceRequired = m_currentlyUsedSize - m_maxSize;
	Int runningTotal = 0;

	std::list<AsciiString> filesToClose;
	// First, search for any samples that have ref counts of 0. They are low-hanging fruit, and 
	// should be considered immediately.
	OpenFilesHashIt it;
	for (it = m_openFiles.begin(); it != m_openFiles.end(); ++it) {
		if (it->second.m_openCount == 0) {
			// This is said low-hanging fruit.
			filesToClose.push_back(it->first);

			runningTotal += it->second.m_fileSize;

			if (runningTotal >= spaceRequired) {
				break;
			}
		}
	}

	// If we don't have enough space yet, then search through the events who have a count of 1 or more
	// and who are lower priority than this sound.
	// Mical said that at this point, sounds shouldn't care if other sounds are interruptable or not.
	// Kill any files of lower priority necessary to clear our the buffer.
	if (runningTotal < spaceRequired) {
		for (it = m_openFiles.begin(); it != m_openFiles.end(); ++it) {
			if (it->second.m_openCount > 0) {
				if (it->second.m_eventInfo->m_priority < sampleThatNeedsSpace.m_eventInfo->m_priority) {
					filesToClose.push_back(it->first);
					runningTotal += it->second.m_fileSize;
				
					if (runningTotal >= spaceRequired) {
						break;
					}
				}
			}
		}
	}

	// We weren't able to find enough sounds to truncate. Therefore, this sound is not going to play.
	if (runningTotal < spaceRequired) {
		return FALSE;
	}

	std::list<AsciiString>::iterator ait;
	for (ait = filesToClose.begin(); ait != filesToClose.end(); ++ait) {
		OpenFilesHashIt itToErase = m_openFiles.find(*ait);
		if (itToErase != m_openFiles.end()) {
			releaseOpenAudioFile(&itToErase->second);
			m_currentlyUsedSize -= itToErase->second.m_fileSize;
			m_openFiles.erase(itToErase);
		}
	}

	return TRUE;
}