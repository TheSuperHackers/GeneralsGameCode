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
// Module:    GameClient
//
// File name: VideoPlayer.cpp
//
// Created:   10/22/01	TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Lib/BaseType.h"
#include "GameClient/VideoPlayer.h"

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------

VideoPlayerInterface *TheVideoPlayer = NULL;

//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//============================================================================
// VideoBuffer::VideoBuffer
//============================================================================

VideoBuffer::VideoBuffer( Type format)
: m_width(0),
	m_height(0),
	m_textureWidth(0),
	m_textureHeight(0),
	m_format(format),
	m_pitch(0),
	m_xPos(0),
	m_yPos(0)
{

	if ( m_format >= NUM_TYPES || m_format < 0 )
	{
		m_format = TYPE_UNKNOWN;
	}

}

//============================================================================
// VideoBuffer::Rect
//============================================================================

RectClass VideoBuffer::Rect( Real x1, Real y1, Real x2, Real y2 )
{
	RectClass rect(0,0,0,0);

	if ( valid() )
	{
		rect.Set(
						((Real)m_width/(Real)m_textureWidth)*x1, ((Real)m_height/(Real)m_textureHeight)*y1,
						((Real)m_width/(Real)m_textureWidth)*x2, ((Real)m_height/(Real)m_textureHeight)*y2
					);
	}

	return rect;

}

//============================================================================
// VideoBuffer::free
//============================================================================

void	VideoBuffer::free( void )
{
	m_width = 0;
	m_height = 0;
	m_textureWidth = 0;
	m_textureHeight = 0;
}

//============================================================================
// VideoPlayer::VideoPlayer
//============================================================================

VideoPlayer::VideoPlayer()
: m_firstStream(NULL)
{

}

//============================================================================
// VideoPlayer::~VideoPlayer
//============================================================================

VideoPlayer::~VideoPlayer()
{
	deinit();
	// Set the video player to null if its us. (WB requires this.)
	if (this == TheVideoPlayer) {
		TheVideoPlayer = NULL;
	}
}

//============================================================================
// VideoPlayer::init
//============================================================================

void	VideoPlayer::init( void )
{
	// Load this here so that WB doesn't have to link to BinkLib, costing us (potentially)
	// an extra license.
	INI ini;
	ini.loadFileDirectory( AsciiString( "Data\\INI\\Default\\Video" ), INI_LOAD_OVERWRITE, NULL );
	ini.loadFileDirectory( AsciiString( "Data\\INI\\Video" ), INI_LOAD_OVERWRITE, NULL );
}

//============================================================================
// VideoPlayer::deinit
//============================================================================

void VideoPlayer::deinit( void )
{
}

//============================================================================
// VideoPlayer::reset
//============================================================================

void	VideoPlayer::reset( void )
{
	closeAllStreams();
}

//============================================================================
// VideoPlayer::update
//============================================================================

void	VideoPlayer::update( void )
{

	VideoStreamInterface *stream = firstStream();

	while ( stream )
	{
		stream->update();
		stream = stream->next();
	}

}

//============================================================================
// VideoPlayer::loseFocus
//============================================================================

void	VideoPlayer::loseFocus( void )
{

}

//============================================================================
// VideoPlayer::regainFocus
//============================================================================

void	VideoPlayer::regainFocus( void )
{

}

//============================================================================
// VideoPlayer::open
//============================================================================

VideoStreamInterface*	VideoPlayer::open( AsciiString movieTitle )
{
	return NULL;
}

//============================================================================
// VideoPlayer::load
//============================================================================

VideoStreamInterface*	VideoPlayer::load( AsciiString movieTitle )
{
	return NULL;
}

//============================================================================
// VideoPlayer::firstStream
//============================================================================

VideoStreamInterface* VideoPlayer::firstStream( void )
{
	return m_firstStream;
}

//============================================================================
// VideoPlayer::closeAllStreams
//============================================================================

void	VideoPlayer::closeAllStreams( void )
{
	VideoStreamInterface *stream ;

	while ( (stream = firstStream()) != 0 )
	{
		stream->close();
	}
}

//============================================================================
// VideoPlayer::remove
//============================================================================

void VideoPlayer::remove( VideoStream *stream_to_remove )
{
	VideoStream *last = NULL;
	VideoStream *stream = (VideoStream*) firstStream();

	while ( stream != NULL && stream != stream_to_remove )
	{
		last = stream;
		stream = (VideoStream*) stream->next();
	}

	if ( stream )
	{
		if ( last )
		{
			last->m_next = stream->m_next;
		}
		else
		{
			m_firstStream = stream->m_next;
		}
	}
}

//============================================================================
// VideoPlayer::addVideo
//============================================================================
void VideoPlayer::addVideo( Video* videoToAdd )
{
	for (VecVideoIt it = mVideosAvailableForPlay.begin(); it != mVideosAvailableForPlay.end(); ++it) {
		if (it->m_internalName == videoToAdd->m_internalName) {
			(*it) = (*videoToAdd);
			return;
		}
	}

	// That internal name hasn't been used, so push a new entry on the back
	mVideosAvailableForPlay.push_back(*videoToAdd);
}

//============================================================================
// VideoPlayer::removeVideo
//============================================================================
void VideoPlayer::removeVideo( Video* videoToRemove )
{
	for (VecVideoIt it = mVideosAvailableForPlay.begin(); it != mVideosAvailableForPlay.end(); ++it) {
		if (it->m_internalName == videoToRemove->m_internalName) {
			mVideosAvailableForPlay.erase(it);
			return;
		}
	}
}

//============================================================================
// VideoPlayer::getNumVideos
//============================================================================
Int VideoPlayer::getNumVideos( void )
{
	return mVideosAvailableForPlay.size();
}

//============================================================================
// VideoPlayer::removeVideo
//============================================================================
const Video* VideoPlayer::getVideo( AsciiString movieTitle )
{
	for (VecVideoIt it = mVideosAvailableForPlay.begin(); it != mVideosAvailableForPlay.end(); ++it) {
		if (it->m_internalName == movieTitle) {
			return &(*it);
		}
	}
	return NULL;
}

//============================================================================
// VideoPlayer::getVideo
//============================================================================
const Video* VideoPlayer::getVideo( Int index )
{
	if (index < 0 || index >= mVideosAvailableForPlay.size()) {
		return NULL;
	}

	return &mVideosAvailableForPlay[index];
}

//============================================================================
// VideoStream::VideoStream
//============================================================================

VideoStream::VideoStream()
: m_next(NULL),
	m_player(NULL)
{

}

//============================================================================
// VideoStream::~VideoStream
//============================================================================

VideoStream::~VideoStream()
{

	if ( m_player )
	{
		m_player->remove( this );
		m_player = NULL;
	}

}

//============================================================================
// VideoStream::next
//============================================================================

VideoStreamInterface* VideoStream::next( void )
{
	return m_next;
}

//============================================================================
// VideoStream::update
//============================================================================

void VideoStream::update( void )
{
}

//============================================================================
// VideoStream::close
//============================================================================

void VideoStream::close( void )
{
	delete this;
}

//============================================================================
// VideoStream::isFrameReady
//============================================================================

Bool VideoStream::isFrameReady( void )
{
	return TRUE;
}

//============================================================================
// VideoStream::frameDecompress
//============================================================================

void VideoStream::frameDecompress( void )
{

}

//============================================================================
// VideoStream::frameRender
//============================================================================

void VideoStream::frameRender( VideoBuffer *buffer )
{

}

//============================================================================
// VideoStream::frameNext
//============================================================================

void VideoStream::frameNext( void )
{

}

//============================================================================
// VideoStream::frameIndex
//============================================================================

Int VideoStream::frameIndex( void )
{
	return 0;
}

//============================================================================
// VideoStream::totalFrames
//============================================================================

Int	VideoStream::frameCount( void )
{
	return 0;
}

//============================================================================
// VideoStream::frameGoto
//============================================================================

void VideoStream::frameGoto( Int index )
{

}

//============================================================================
// VideoStream::height
//============================================================================

Int		VideoStream::height( void )
{
	return 0;
}

//============================================================================
// VideoStream::width
//============================================================================

Int		VideoStream::width( void )
{
	return 0;
}


const FieldParse VideoPlayer::m_videoFieldParseTable[] =
{
	{ "Filename",								INI::parseAsciiString,							NULL, offsetof( Video, m_filename) },
	{ "Comment",								INI::parseAsciiString,							NULL, offsetof( Video, m_commentForWB) },
	{ NULL,											NULL,																NULL, 0 },
};

