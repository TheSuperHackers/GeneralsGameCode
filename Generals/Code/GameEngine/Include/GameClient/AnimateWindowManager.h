/*
**	Command & Conquer Generals(tm)
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

// FILE: AnimateWindowManager.h /////////////////////////////////////////////////
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
//	Filename: AnimateWindowManager.h
//
//	author:		Chris Huybregts
//	
//	purpose:	The Animate Window class will be used by registering a window with
//						the manager with stating what kind of animation to do. Then on every
//						update, we'll move the windows.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ANIMATEWINDOWMANAGER_H_
#define __ANIMATEWINDOWMANAGER_H_

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Lib/BaseType.h"
#include "Common/SubsystemInterface.h"
#include "Common/GameMemory.h"

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class GameWindow;
class ProcessAnimateWindowSlideFromBottom;
class ProcessAnimateWindowSlideFromBottomTimed;
class ProcessAnimateWindowSlideFromTop;
class ProcessAnimateWindowSlideFromLeft;
class ProcessAnimateWindowSlideFromRight;
class ProcessAnimateWindowSlideFromRightFast;
class ProcessAnimateWindowSpiral;
class ProcessAnimateWindowSlideFromTopFast;
class ProcessAnimateWindowSideSelect;
class ProcessAnimateWindow;

//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

enum AnimTypes
{
	WIN_ANIMATION_NONE = 0,
	WIN_ANIMATION_SLIDE_RIGHT,
	WIN_ANIMATION_SLIDE_RIGHT_FAST,
	WIN_ANIMATION_SLIDE_LEFT,
	WIN_ANIMATION_SLIDE_TOP,
	WIN_ANIMATION_SLIDE_BOTTOM,
	WIN_ANIMATION_SPIRAL,
	WIN_ANIMATION_SLIDE_BOTTOM_TIMED,
	WIN_ANIMATION_SLIDE_TOP_FAST,
	WIN_ANIMATION_COUNT
} ;

//-----------------------------------------------------------------------------
class AnimateWindowWW : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AnimateWindowWW, "AnimateWindow")		
public:
	AnimateWindowWW( void );
	//~AnimateWindow( void );
	
	void setAnimData( ICoord2D startPos, ICoord2D endPos, ICoord2D curPos, ICoord2D restPos, Coord2D vel, UnsignedInt startTime, UnsignedInt endTime);

	ICoord2D		getStartPos( void );							///< Get the Start Position 2D coord
	ICoord2D		getCurPos( void );								///< Get the Current Position 2D coord
	ICoord2D		getEndPos( void );								///< Get the End Position 2D coord
	ICoord2D		getRestPos( void );								///< Get the Rest Position 2D coord
	GameWindow *getGameWindow( void );						///< Get the GameWindow that will be animating
	AnimTypes		getAnimType( void );							///< Get the Animation type
	UnsignedInt	getDelay( void );									///< Get the Time Delay
	Coord2D			getVel( void );										///< Get the Velocity Position 2D coord
	UnsignedInt getStartTime( void );							///< Get the start time of the time-based anim
	UnsignedInt getEndTime( void );								///< Get the end time of the time-based anim

	void	setStartPos( ICoord2D starPos);					///< Set the Start Position 2D coord
	void	setCurPos( ICoord2D curPos);						///< Set the Current Position 2D coord
	void	setEndPos( ICoord2D endPos);						///< Set the End Position 2D coord
	void	setRestPos( ICoord2D restPos);					///< Set the Rest Position 2D coord
	void	setGameWindow( GameWindow *win);				///< Set the GameWindow that will be animating
	void	setAnimType( AnimTypes animType);				///< Set the Animation type
	void	setDelay( UnsignedInt delay);						///< Set the Time Delay
	void	setVel( Coord2D vel);										///< Set the Velocity Position 2D coord
	void	setStartTime( UnsignedInt t);						///< Set the start time of the time-based anim
	void	setEndTime( UnsignedInt t);							///< Set the end time of the time-based anim

	void setFinished(Bool finished);							///< Set if the animation has finished
	Bool isFinished( void );											///< Return if the animation has finished or not.
	void setNeedsToFinish( Bool needsToFinish);		///< set if we need this animation to finish for the manager to return true
	Bool needsToFinish( void );										///< set if the animation has finished

private:
	UnsignedInt m_delay;													///< Holds the delay time in which the animation will start (in milliseconds)
	ICoord2D m_startPos;													///< Holds the starting position of the animation 
																								///<(usuall is also the end position of the animation when the animation is reversed)
	ICoord2D m_endPos;														///< Holds the target End Position (usually is the same as the rest position)
	ICoord2D m_curPos;														///< It's Current Position
	ICoord2D m_restPos;														///< When the Manager Resets, It sets the window's position to this position
	GameWindow *m_win;														///< the window that this animation is happening on
	Coord2D m_vel;																///< the Velocity of the animation
	UnsignedInt m_startTime;											///< time we started the time-based anim
	UnsignedInt m_endTime;												///< time we should end the time-based anim
	AnimTypes m_animType;													///< The type of animation that will happen
	Bool m_needsToFinish;													///< Flag to tell the manager if we need to finish before it's done with it's animation
	Bool m_isFinished;														///< We're finished
};



//-----------------------------------------------------------------------------
typedef	std::list<AnimateWindowWW *>	AnimateWindowList;

//-----------------------------------------------------------------------------
class AnimateWindowManager : public SubsystemInterface
{
public:
	AnimateWindowManager( void );
	~AnimateWindowManager( void );

	// Inhertited from subsystem ====================================================================
	virtual void init( void );			
	virtual void reset( void );			
	virtual void update( void );		
	//===============================================================================================

	void registerGameWindow(GameWindow *win, AnimTypes animType, Bool needsToFinish, UnsignedInt ms = 0, UnsignedInt delayMs = 0);			// Registers a new window to animate.
	Bool isFinished( void );										///< Are all the animations that need to be finished, finished?
	void reverseAnimateWindow( void );					///< tell each animation type to setup the windows to run in reverse
	void resetToRestPosition( void );						///< Reset all windows to their rest position
	Bool isReversed( void );										///< Returns whether or not we're in our reversed state.
	Bool isEmpty( void );
protected:
	AnimateWindowList	m_winList;								///< A list of AnimationWindows that we don't care if their finished animating
	AnimateWindowList m_winMustFinishList;			///< A list of AnimationWindows that we do care about
	Bool m_needsUpdate;													///< If we're done animating all our monitored windows, then this will be false
	Bool m_reverse;															///< Are we in a reverse state?
	ProcessAnimateWindowSlideFromRight *m_slideFromRight;			///< Holds the process in which the windows slide from the right
	ProcessAnimateWindowSlideFromRightFast *m_slideFromRightFast;
	ProcessAnimateWindowSlideFromTop *m_slideFromTop;					///< Holds the process in which the windows slide from the Top
	ProcessAnimateWindowSlideFromLeft *m_slideFromLeft;				///< Holds the process in which the windows slide from the Left
	ProcessAnimateWindowSlideFromBottom *m_slideFromBottom;		///< Holds the process in which the windows slide from the Bottom
	ProcessAnimateWindowSpiral *m_spiral;											///< Holds the process in which the windows Spiral onto the screen
	ProcessAnimateWindowSlideFromBottomTimed *m_slideFromBottomTimed;		///< Holds the process in which the windows slide from the Bottom in a time-based fashion
	ProcessAnimateWindowSlideFromTopFast *m_slideFromTopFast;			///< holds the process in wich the windows slide from the top,fast
	ProcessAnimateWindow *getProcessAnimate( AnimTypes animType);		///< returns the process for the kind of animation we need.
	
};

//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
	inline ICoord2D			AnimateWindowWW::getStartPos( void )	{ return m_startPos; };
	inline ICoord2D			AnimateWindowWW::getCurPos( void )		{ return m_curPos; };
	inline ICoord2D			AnimateWindowWW::getEndPos( void )		{ return m_endPos; };
	inline ICoord2D			AnimateWindowWW::getRestPos( void )		{ return m_restPos; };
	inline GameWindow  *AnimateWindowWW::getGameWindow( void ){ return m_win; };
	inline AnimTypes		AnimateWindowWW::getAnimType( void )	{ return m_animType; };
	inline UnsignedInt	AnimateWindowWW::getDelay( void )			{ return m_delay; };
	inline Coord2D			AnimateWindowWW::getVel( void )				{ return m_vel; };
	inline UnsignedInt	AnimateWindowWW::getStartTime( void )	{ return m_startTime; };
	inline UnsignedInt	AnimateWindowWW::getEndTime( void )		{ return m_endTime; };

	inline void	AnimateWindowWW::setStartPos( ICoord2D startPos)		{ m_startPos = startPos; };
	inline void	AnimateWindowWW::setCurPos( ICoord2D curPos)				{ m_curPos = curPos; };
	inline void	AnimateWindowWW::setEndPos( ICoord2D endPos)				{ m_endPos = endPos; };
	inline void	AnimateWindowWW::setRestPos( ICoord2D restPos)			{ m_restPos = restPos; };
	inline void	AnimateWindowWW::setGameWindow( GameWindow *win)		{ m_win = win; };
	inline void	AnimateWindowWW::setAnimType( AnimTypes animType)		{ m_animType = animType; };
	inline void	AnimateWindowWW::setDelay( UnsignedInt delay)				{ m_delay = delay; };
	inline void	AnimateWindowWW::setVel( Coord2D vel)								{ m_vel = vel; };
	inline void	AnimateWindowWW::setStartTime( UnsignedInt t )			{ m_startTime = t; }
	inline void	AnimateWindowWW::setEndTime( UnsignedInt t )				{ m_endTime = t; }

	inline void	AnimateWindowWW::setFinished( Bool finished)				{ m_isFinished = finished; };
	inline Bool	AnimateWindowWW::isFinished( void )									{ return m_isFinished; };
	inline void	AnimateWindowWW::setNeedsToFinish( Bool needsToFinish)		{ m_needsToFinish = needsToFinish; };
	inline Bool	AnimateWindowWW::needsToFinish( void )							{ return m_needsToFinish; };
	
	inline Bool AnimateWindowManager::isFinished( void )					{ return !m_needsUpdate;	};
	inline Bool AnimateWindowManager::isReversed( void )						{ return m_reverse;	};
	inline Bool AnimateWindowManager::isEmpty( void ){return (m_winList.size() == 0 && m_winMustFinishList.size() == 0);	}
//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

#endif // __ANIMATEWINDOWMANAGER_H_
