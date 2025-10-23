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

// FILE: WindowVideoManager.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Mar 2002
//
//	Filename: 	WindowVideoManager.h
//
//	author:		Chris Huybregts
//
//	purpose:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class GameWindow;
class AsciiString;
class VideoStreamInterface;
class VideoBuffer;

//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------


enum WindowVideoPlayType CPP_11(: Int)
{
	WINDOW_PLAY_MOVIE_ONCE = 0,
	WINDOW_PLAY_MOVIE_LOOP,
	WINDOW_PLAY_MOVIE_SHOW_LAST_FRAME,

	WINDOW_PLAY_MOVIE_COUNT
};

enum WindowVideoStates CPP_11(: Int)
{
	WINDOW_VIDEO_STATE_START = 0,
	WINDOW_VIDEO_STATE_STOP,
	WINDOW_VIDEO_STATE_PAUSE,
	WINDOW_VIDEO_STATE_PLAY,
	WINDOW_VIDEO_STATE_HIDDEN,

	WINDOW_VIDEO_STATE_COUNT
};

class WindowVideo
{
public:
	WindowVideo( void );
	~WindowVideo( void );

	VideoStreamInterface *getVideoStream( void );
	VideoBuffer *getVideoBuffer( void );
	GameWindow *getWin( void );
	AsciiString getMovieName( void );
	WindowVideoPlayType getPlayType ( void );
	WindowVideoStates getState( void );

	void setPlayType(WindowVideoPlayType playType);
	void setWindowState( WindowVideoStates state );

	void init( GameWindow *win, AsciiString movieName, WindowVideoPlayType playType,VideoBuffer *videoBuffer,
	VideoStreamInterface *videoStream);

private:
	WindowVideoPlayType m_playType;
	GameWindow *m_win;
	VideoBuffer *m_videoBuffer;
	VideoStreamInterface *m_videoStream;
	AsciiString m_movieName;
	WindowVideoStates m_state;

};


class WindowVideoManager : public SubsystemInterface
{
public:
	WindowVideoManager( void );
	~WindowVideoManager( void );

	// Inhertited from subsystem ====================================================================
	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );
	//===============================================================================================


	void playMovie( GameWindow *win, AsciiString movieName, WindowVideoPlayType playType );
	void hideMovie( GameWindow *win );							///< If the window becomes hidden while we're playing, stop the movie but test to see if we should resume
	void pauseMovie( GameWindow *win );							///< Pause a movie and display it's current frame
	void resumeMovie( GameWindow *win );						///< If a movie has been stopped, resume it.
	void stopMovie( GameWindow *win );							///< Stop a movie
	void stopAndRemoveMovie( GameWindow *win );			///< Stop a movie, and remove it from the manager
	void stopAllMovies( void );											///< Stop all playing movies
	void pauseAllMovies( void );										///< Pauses all movies on their current frame
	void resumeAllMovies( void );										///< Resume Playing all movies
	Int getWinState( GameWindow *win );			///< return the current state of the window.

private:

	typedef const GameWindow* ConstGameWindowPtr;
	// use special class for hashing, since std::hash won't compile for arbitrary ptrs
	struct hashConstGameWindowPtr
	{
	size_t operator()(ConstGameWindowPtr p) const
	{
		std::hash<UnsignedInt> hasher;
		return hasher((UnsignedInt)p);
	}
	};

	typedef std::hash_map< ConstGameWindowPtr, WindowVideo *, hashConstGameWindowPtr, std::equal_to<ConstGameWindowPtr> > WindowVideoMap;

	WindowVideoMap m_playingVideos;								///< List of currently playin Videos
	//WindowVideoMap m_pausedVideos;									///< List of currently paused Videos
	Bool m_stopAllMovies;														///< Maintains is we're in a stop all Movies State
	Bool m_pauseAllMovies;													///< Maintains if we're in a pause all movies state


};

//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
inline VideoStreamInterface *WindowVideo::getVideoStream( void ){ return m_videoStream; };
inline VideoBuffer *WindowVideo::getVideoBuffer( void ){ return m_videoBuffer; };
inline GameWindow *WindowVideo::getWin( void ){ return m_win; };
inline AsciiString WindowVideo::getMovieName( void ){ return m_movieName; };
inline WindowVideoPlayType WindowVideo::getPlayType ( void ){ return m_playType; };
inline WindowVideoStates WindowVideo::getState( void ){ return m_state; };

inline void WindowVideo::setPlayType(WindowVideoPlayType playType){ m_playType = playType; };



//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
