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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   Generals
//
// Module:    VideoDevice
//
// File name: BinkDevice.cpp
//
// Created:   10/22/01	TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "Lib/BaseType.h"
#include "VideoDevice/Bink/BinkVideoPlayer.h"
#include "Common/AudioAffect.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/GlobalData.h"
#include "Common/Registry.h"

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------
#define VIDEO_LANG_PATH_FORMAT "Data/%s/Movies/%s.%s"
#define VIDEO_PATH	"Data\\Movies"
#define VIDEO_EXT		"bik"



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------

// TheSuperHackers @bugfix ZsoltFeher 19/07/2026 Compute the Bink audio volume from the
// speech (voice) volume setting. Bink expects 0..32768, where 32768 is full volume.
// The 0.8 scale matches the original game's intended movie loudness. The original
// formula could never go below 327 (about 1 percent), so movie audio remained audible
// with the voice slider at 0. Clamp to a minimum of 1 instead, which is inaudible,
// but never pass 0, as Bink will interpret that as "play at full volume".
static Int calculateMovieAudioVolume()
{
	Int volume = (Int) (TheAudio->getVolume(AudioAffect_Speech) * 0.8f * 32768.0f);

	if (volume < 1)
	{
		volume = 1;
	}
	else if (volume > 32768)
	{
		volume = 32768;
	}

	return volume;
}


//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------


//============================================================================
// BinkVideoPlayer::BinkVideoPlayer
//============================================================================

BinkVideoPlayer::BinkVideoPlayer()
{

}

//============================================================================
// BinkVideoPlayer::~BinkVideoPlayer
//============================================================================

BinkVideoPlayer::~BinkVideoPlayer()
{
	deinit();
}

//============================================================================
// BinkVideoPlayer::init
//============================================================================

void	BinkVideoPlayer::init()
{
	// Need to load the stuff from the ini file.
	VideoPlayer::init();

	initializeBinkWithMiles();
}

//============================================================================
// BinkVideoPlayer::deinit
//============================================================================

void BinkVideoPlayer::deinit()
{
	TheAudio->releaseHandleForBink();
	VideoPlayer::deinit();
}

//============================================================================
// BinkVideoPlayer::reset
//============================================================================

void	BinkVideoPlayer::reset()
{
	VideoPlayer::reset();
}

//============================================================================
// BinkVideoPlayer::update
//============================================================================

void	BinkVideoPlayer::update()
{
	VideoPlayer::update();

}

//============================================================================
// BinkVideoPlayer::loseFocus
//============================================================================

void	BinkVideoPlayer::loseFocus()
{
	VideoPlayer::loseFocus();
}

//============================================================================
// BinkVideoPlayer::regainFocus
//============================================================================

void	BinkVideoPlayer::regainFocus()
{
	VideoPlayer::regainFocus();
}

//============================================================================
// BinkVideoPlayer::createStream
//============================================================================

VideoStreamInterface* BinkVideoPlayer::createStream( HBINK handle )
{

	if ( handle == nullptr )
	{
		return nullptr;
	}

	BinkVideoStream *stream = NEW BinkVideoStream;

	if ( stream )
	{

		stream->m_handle = handle;
		stream->m_next = m_firstStream;
		stream->m_player = this;
		m_firstStream = stream;

		Int volume = calculateMovieAudioVolume();
		DEBUG_LOG(("BinkVideoPlayer::createStream() - About to set volume (%g -> %d",
			TheAudio->getVolume(AudioAffect_Speech), volume));
		BinkSetVolume( stream->m_handle, 0, volume );
		DEBUG_LOG(("BinkVideoPlayer::createStream() - set volume"));
	}

	return stream;
}

//============================================================================
// BinkVideoPlayer::open
//============================================================================

VideoStreamInterface*	BinkVideoPlayer::open( AsciiString movieTitle )
{
	VideoStreamInterface*	stream = nullptr;

	const Video* pVideo = getVideo(movieTitle);
	if (pVideo) {
		DEBUG_LOG(("BinkVideoPlayer::createStream() - About to open bink file"));

		if (TheGlobalData->m_modDir.isNotEmpty())
		{
			char filePath[ _MAX_PATH ];
			snprintf( filePath, ARRAY_SIZE(filePath), "%s%s\\%s.%s", TheGlobalData->m_modDir.str(), VIDEO_PATH, pVideo->m_filename.str(), VIDEO_EXT );
			HBINK handle = BinkOpen(filePath , BINKPRELOADALL );
			DEBUG_ASSERTLOG(!handle, ("opened bink file %s", filePath));
			if (handle)
			{
				return createStream( handle );
			}
		}

		char localizedFilePath[ _MAX_PATH ];
		snprintf( localizedFilePath, ARRAY_SIZE(localizedFilePath), VIDEO_LANG_PATH_FORMAT, GetRegistryLanguage().str(), pVideo->m_filename.str(), VIDEO_EXT );
		HBINK handle = BinkOpen(localizedFilePath , BINKPRELOADALL );
		DEBUG_ASSERTLOG(!handle, ("opened localized bink file %s", localizedFilePath));
		if (!handle)
		{
			char filePath[ _MAX_PATH ];
			snprintf( filePath, ARRAY_SIZE(filePath), "%s\\%s.%s", VIDEO_PATH, pVideo->m_filename.str(), VIDEO_EXT );
			handle = BinkOpen(filePath , BINKPRELOADALL );
			DEBUG_ASSERTLOG(!handle, ("opened bink file %s", localizedFilePath));
		}

		DEBUG_LOG(("BinkVideoPlayer::createStream() - About to create stream"));
		stream = createStream( handle );
	}

	return stream;
}

//============================================================================
// BinkVideoPlayer::load
//============================================================================

VideoStreamInterface*	BinkVideoPlayer::load( AsciiString movieTitle )
{
	return open(movieTitle); // load() used to have the same body as open(), so I'm combining them.  Munkee.
}

//============================================================================
//============================================================================
void BinkVideoPlayer::notifyVideoPlayerOfNewProvider( Bool nowHasValid )
{
	if (!nowHasValid) {
		TheAudio->releaseHandleForBink();
		BinkSetSoundTrack(0, nullptr);
	} else {
		initializeBinkWithMiles();
	}
}

//============================================================================
//============================================================================
void BinkVideoPlayer::initializeBinkWithMiles()
{
	Int retVal = 0;
	void *driver = TheAudio->getHandleForBink();

	if ( driver )
	{
		retVal = BinkSoundUseDirectSound(driver);
	}
	if( !driver || retVal == 0)
	{
		BinkSetSoundTrack ( 0,nullptr );
	}
}

//============================================================================
// BinkVideoStream::BinkVideoStream
//============================================================================

BinkVideoStream::BinkVideoStream()
: m_handle(nullptr)
{

}

//============================================================================
// BinkVideoStream::~BinkVideoStream
//============================================================================

BinkVideoStream::~BinkVideoStream()
{
	if ( m_handle != nullptr )
	{
		BinkClose( m_handle );
		m_handle = nullptr;
	}
}

//============================================================================
// BinkVideoStream::update
//============================================================================

void BinkVideoStream::update()
{
	BinkWait( m_handle );
}

//============================================================================
// BinkVideoStream::isFrameReady
//============================================================================

Bool BinkVideoStream::isFrameReady()
{
	return !BinkWait( m_handle );
}

//============================================================================
// BinkVideoStream::frameDecompress
//============================================================================

void BinkVideoStream::frameDecompress()
{
		BinkDoFrame( m_handle );

		// TheSuperHackers @bugfix ZsoltFeher 19/07/2026 Reapply the audio volume on every decoded
		// frame. The volume set once on stream creation did not reliably take effect, so movie
		// audio played at full volume regardless of the Options volume sliders. Reapplying it
		// during playback keeps movie audio in sync with the speech (voice) volume setting and
		// also makes volume changes take effect while a movie is playing.
		BinkSetVolume( m_handle, 0, calculateMovieAudioVolume() );
}

//============================================================================
// BinkVideoStream::frameRender
//============================================================================

void BinkVideoStream::frameRender( VideoBuffer *buffer )
{
	if ( buffer )
	{
		void *mem = buffer->lock();

		u32 flags;

		switch ( buffer->format())
		{
			case VideoBuffer::TYPE_X8R8G8B8:
				flags = BINKSURFACE32;
				break;

			case VideoBuffer::TYPE_R8G8B8:
				flags = BINKSURFACE24;
				break;

			case VideoBuffer::TYPE_R5G6B5:
				flags = BINKSURFACE565;
				break;

			case VideoBuffer::TYPE_X1R5G5B5:
				flags = BINKSURFACE555;
				break;

			default:
				return;
		}

		if ( mem != nullptr )
		{

			BinkCopyToBuffer ( m_handle, mem, buffer->pitch(), buffer->height(),
													buffer->xPos(), buffer->yPos(), flags );
			buffer->unlock();
		}
	}

}

//============================================================================
// BinkVideoStream::frameNext
//============================================================================

void BinkVideoStream::frameNext()
{
	BinkNextFrame( m_handle );
}

//============================================================================
// BinkVideoStream::frameIndex
//============================================================================

Int BinkVideoStream::frameIndex()
{
	return m_handle->FrameNum - 1;
}

//============================================================================
// BinkVideoStream::totalFrames
//============================================================================

Int	BinkVideoStream::frameCount()
{
	return m_handle->Frames;
}

//============================================================================
// BinkVideoStream::frameGoto
//============================================================================

void BinkVideoStream::frameGoto( Int index )
{
	BinkGoto(m_handle, index, 0 );
}

//============================================================================
// VideoStream::height
//============================================================================

Int		BinkVideoStream::height()
{
	return m_handle->Height;
}

//============================================================================
// VideoStream::width
//============================================================================

Int		BinkVideoStream::width()
{
	return m_handle->Width;
}



