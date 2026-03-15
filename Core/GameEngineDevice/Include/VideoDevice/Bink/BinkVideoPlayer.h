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

#include "GameClient/VideoPlayer.h"
#include "bink.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class BinkVideoPlayer;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// BinkVideoStream
//===============================

class BinkVideoStream : public VideoStream
{
	friend class BinkVideoPlayer;

	protected:

		HBINK					m_handle;														///< Bink streaming handle;
		Char					*m_memFile;													///< Pointer to memory resident file

		BinkVideoStream();																///< only BinkVideoPlayer can create these
		virtual ~BinkVideoStream() override;

	public:

		virtual void update() override;											///< Update bink stream

		virtual Bool	isFrameReady() override;								///< Is the frame ready to be displayed
		virtual void	frameDecompress() override;						///< Render current frame in to buffer
		virtual void	frameRender( VideoBuffer *buffer ) override; ///< Render current frame in to buffer
		virtual void	frameNext() override;									///< Advance to next frame
		virtual Int		frameIndex() override;									///< Returns zero based index of current frame
		virtual Int		frameCount() override;									///< Returns the total number of frames in the stream
		virtual void	frameGoto( Int index ) override;							///< Go to the spcified frame index
		virtual Int		height() override;											///< Return the height of the video
		virtual Int		width() override;											///< Return the width of the video


};

//===============================
// BinkVideoPlayer
//===============================
/**
  *	Bink video playback code.
	*/
//===============================

class BinkVideoPlayer : public VideoPlayer
{

	protected:

		VideoStreamInterface* createStream( HBINK handle );

	public:

		// subsytem requirements
		virtual void	init() override;														///< Initialize video playback code
		virtual void	reset() override;													///< Reset video playback
		virtual void	update() override;													///< Services all audio tasks. Should be called frequently

		virtual void	deinit() override;													///< Close down player


		BinkVideoPlayer();
		virtual ~BinkVideoPlayer() override;

		// service
		virtual void	loseFocus() override;											///< Should be called when application loses focus
		virtual void	regainFocus() override;										///< Should be called when application regains focus

		virtual VideoStreamInterface*	open( AsciiString movieTitle ) override;	///< Open video file for playback
		virtual VideoStreamInterface*	load( AsciiString movieTitle ) override;	///< Load video file in to memory for playback

		virtual void notifyVideoPlayerOfNewProvider( Bool nowHasValid ) override;
		virtual void initializeBinkWithMiles();
};


//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
