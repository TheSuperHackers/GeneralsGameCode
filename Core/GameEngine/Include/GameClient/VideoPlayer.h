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
// Project:    Generals
//
// File name:  GameClient/VideoPlayer.h
//
// Created:    10/22/01
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include <Lib/BaseType.h>
#include "WWMath/rect.h"
#include "Common/SubsystemInterface.h"
#include "Common/AsciiString.h"
#include "Common/INI.h"
#include "Common/STLTypedefs.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

struct Video;
class VideoPlayer;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------
typedef std::vector<Video>	VecVideo;
typedef std::vector<Video>::iterator	VecVideoIt;

//----------------------------------------------------------------------------
//           Video (Struct)
//----------------------------------------------------------------------------
struct Video
{
	AsciiString m_filename;																	///< should be filled with the filename on disk
	AsciiString m_internalName;															///< should be our internal reference name
	AsciiString m_commentForWB;
};

//===============================
// VideoBuffer
//===============================
/**
  * Video buffer interface class. The VideoPlayer uses this buffer abstraction
	* in order to be able to render a video stream.
	*/
//===============================

class VideoBuffer
{
	public:

	// Enumeration of buffer pixel formats
		enum Type
		{
			TYPE_UNKNOWN,
			TYPE_R8G8B8,
			TYPE_X8R8G8B8,
			TYPE_R5G6B5,
			TYPE_X1R5G5B5,
			NUM_TYPES
		};


	protected:

		UnsignedInt m_xPos;					///< X pixel buffer offset
		UnsignedInt m_yPos;					///< Y pixel buffer offset
		UnsignedInt m_width;				///< Buffer visible width
		UnsignedInt m_height;				///< Buffer height
		UnsignedInt m_textureWidth;	///< Buffer visible width
		UnsignedInt m_textureHeight;///< Buffer height
		UnsignedInt m_pitch;				///< buffer pitch

		Type m_format;			///< buffer pixel format

	public:

		VideoBuffer( Type format );
		virtual ~VideoBuffer() {};

		virtual	Bool		allocate( UnsignedInt width, UnsignedInt Height ) = 0; ///< Allocate buffer
		virtual void		free( void ) = 0;			///< Free the buffer
		virtual	void*		lock( void ) = 0;			///< Returns memory pointer to start of buffer
		virtual void		unlock( void ) = 0;		///< Release buffer
		virtual Bool		valid( void ) = 0;		///< Is the buffer valid to use

		UnsignedInt			xPos( void ) { return m_xPos;};///< X pixel offset to draw into
		UnsignedInt			yPos( void ) { return m_yPos;};///< Y pixel offset to draw into
		void		setPos( UnsignedInt x, UnsignedInt y ) { m_xPos = x; m_yPos = y;};	///< Set the x and y buffer offset
		UnsignedInt			width( void ) { return m_width;};		///< Returns pixel width of visible texture
		UnsignedInt			height( void ) { return m_height;};	///< Returns pixel height of visible texture
		UnsignedInt			textureWidth( void ) { return m_textureWidth;};		///< Returns pixel width of texture
		UnsignedInt			textureHeight( void ) { return m_textureHeight;};	///< Returns pixel height of texture
		UnsignedInt			pitch( void ) { return m_pitch;};		///< Returns buffer pitch in bytes
		Type		format( void ) { return m_format;};	///< Returns buffer pixel format

		RectClass				Rect( Real x1, Real y1, Real x2, Real y2 );

};


//===============================
// VideoStreamInterface
//===============================
/**
  * Video stream interface class.
	*/
//===============================

class VideoStreamInterface
{
	friend class VideoPlayerInterface;

	protected:

		virtual ~VideoStreamInterface(){};								///< only VideoPlayerInterfaces can create these

	public:


		virtual	VideoStreamInterface* next( void ) = 0;		///< Returns next open stream
		virtual void update( void ) = 0;									///< Update stream
		virtual void close( void ) = 0;										///< Close and free stream

		virtual Bool	isFrameReady( void ) = 0;						///< Is the frame ready to be displayed
		virtual void	frameDecompress( void ) = 0;				///< Render current frame in to buffer
		virtual void	frameRender( VideoBuffer *buffer ) = 0; ///< Render current frame in to buffer
		virtual void	frameNext( void ) = 0;							///< Advance to next frame
		virtual Int		frameIndex( void ) = 0;							///< Returns zero based index of current frame
		virtual Int		frameCount( void ) = 0;							///< Returns the total number of frames in the stream
		virtual void	frameGoto( Int index ) = 0;					///< Go to the spcified frame index
		virtual Int		height( void ) = 0;									///< Return the height of the video
		virtual Int		width( void ) = 0;									///< Return the width of the video

};

//===============================
// VideoStream
//===============================



class VideoStream : public VideoStreamInterface
{
	friend class VideoPlayer;

	protected:

		VideoPlayer							*m_player;								///< Video player we were created with
		VideoStream							*m_next;									///< Next open stream

		VideoStream();																		///< only VideoPlayer can create these
		virtual ~VideoStream();

	public:

 		virtual	VideoStreamInterface* next( void );				///< Returns next open stream
		virtual void update( void );											///< Update stream
		virtual void close( void );												///< Close and free stream

		virtual Bool	isFrameReady( void );								///< Is the frame ready to be displayed
		virtual void	frameDecompress( void );						///< Render current frame in to buffer
		virtual void	frameRender( VideoBuffer *buffer ); ///< Render current frame in to buffer
		virtual void	frameNext( void );									///< Advance to next frame
		virtual Int		frameIndex( void );									///< Returns zero based index of current frame
		virtual Int		frameCount( void );									///< Returns the total number of frames in the stream
		virtual void	frameGoto( Int index );							///< Go to the spcified frame index
		virtual Int		height( void );											///< Return the height of the video
		virtual Int		width( void );											///< Return the width of the video


};

//===============================
// VideoPlayerInterface
//===============================
/**
  *	Interface for video playback.
	*/
//===============================

class VideoPlayerInterface : public SubsystemInterface
{

	public:

		virtual void	init( void ) = 0;														///< Initialize video playback
		virtual void	reset( void ) = 0;													///< Reset video playback
		virtual void	update( void ) = 0;													///< Services all video tasks. Should be called frequently

		virtual void	deinit( void ) = 0;													///< Close down player

		virtual				~VideoPlayerInterface() {};

		// service
		virtual void	loseFocus( void ) = 0;											///< Should be called when application loses focus
		virtual void	regainFocus( void ) = 0;										///< Should be called when application regains focus

		virtual VideoStreamInterface*	open( AsciiString movieTitle ) = 0;	///< Open video file for playback
		virtual VideoStreamInterface*	load( AsciiString movieTitle ) = 0;	///< Load video file in to memory for playback

		virtual VideoStreamInterface* firstStream( void ) = 0;		///< Return the first open/loaded video stream

		virtual void	closeAllStreams( void ) = 0;								///< Close all open streams
		virtual void	addVideo( Video* videoToAdd ) = 0;					///< Add a video to the list of videos we can play
		virtual void	removeVideo( Video* videoToRemove ) = 0;		///< Remove a video to the list of videos we can play
		virtual Int getNumVideos( void ) = 0;											///< Retrieve info about the number of videos currently listed
		virtual const Video* getVideo( AsciiString movieTitle ) = 0;	///< Retrieve info about a movie based on internal name
		virtual const Video* getVideo( Int index ) = 0;						///< Retrieve info about a movie based on index

		virtual const FieldParse *getFieldParse( void ) const = 0;		///< Return the field parse info

		virtual void notifyVideoPlayerOfNewProvider( Bool nowHasValid ) = 0;		///< Notify the video player that they can now ask for an audio handle, or they need to give theirs up.
};


//===============================
// VideoPlayer
//===============================
/**
  *	Common video playback code.
	*/
//===============================

class VideoPlayer : public VideoPlayerInterface
{
	protected:
		VecVideo mVideosAvailableForPlay;
		VideoStream		*m_firstStream;
		static const FieldParse m_videoFieldParseTable[];

	public:

		// subsytem requirements
		virtual void	init( void );														///< Initialize video playback code
		virtual void	reset( void );													///< Reset video playback
		virtual void	update( void );													///< Services all audio tasks. Should be called frequently

		virtual void	deinit( void );													///< Close down player


		VideoPlayer();
		~VideoPlayer();

		// service
		virtual void	loseFocus( void );											///< Should be called when application loses focus
		virtual void	regainFocus( void );										///< Should be called when application regains focus

		virtual VideoStreamInterface*	open( AsciiString movieTitle );	///< Open video file for playback
		virtual VideoStreamInterface*	load( AsciiString movieTitle );	///< Load video file in to memory for playback
		virtual VideoStreamInterface* firstStream( void );		///< Return the first open/loaded video stream
		virtual void	closeAllStreams( void );								///< Close all open streams

		virtual void	addVideo( Video* videoToAdd );					///< Add a video to the list of videos we can play
		virtual void	removeVideo( Video* videoToRemove );		///< Remove a video to the list of videos we can play
		virtual Int getNumVideos( void );											///< Retrieve info about the number of videos currently listed
		virtual const Video* getVideo( AsciiString movieTitle );	///< Retrieve info about a movie based on internal name
		virtual const Video* getVideo( Int index );						///< Retrieve info about a movie based on index
		virtual const FieldParse *getFieldParse( void ) const { return m_videoFieldParseTable; }		///< Return the field parse info

		virtual void notifyVideoPlayerOfNewProvider( Bool nowHasValid ) { }

		// Implementation specific
		void remove( VideoStream *stream );										///< remove stream from active list

};

extern VideoPlayerInterface *TheVideoPlayer;

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
