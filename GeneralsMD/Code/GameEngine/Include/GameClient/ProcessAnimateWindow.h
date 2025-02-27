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

// FILE: ProcessAnimateWindow.h /////////////////////////////////////////////////
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
//	Filename: 	ProcessAnimateWindow.h
//
//	author:		Chris Huybregts
//
//	purpose:	If a new animation is wanted to be added for the windows, All you
//						have to do is create a new class derived from ProcessAnimateWindow.
//						Then setup each of the virtual classes to process an AnimateWindow
//						class.  The Update adn reverse functions get called every frame
//						by the shell and will continue to process the AdminWin until the
//						isFinished flag on the adminWin is set to true.
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
#include "Lib/BaseType.h"
//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
namespace wnd
{
class AnimateWindow;
}
class GameWindow;
//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class ProcessAnimateWindow
{
public:

	ProcessAnimateWindow( void ){};
	virtual ~ProcessAnimateWindow( void ){};

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin ) = 0;
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 ) = 0;
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin ) = 0;
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin ) = 0;
	virtual void setMaxDuration(UnsignedInt maxDuration) { }
};

//-----------------------------------------------------------------------------

class ProcessAnimateWindowSlideFromRight : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSlideFromRight( void );
	virtual ~ProcessAnimateWindowSlideFromRight( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
private:
Coord2D m_maxVel;  // top speed windows travel in x and y
Int m_slowDownThreshold;  // when widnows get this close to their resting
																		// positions they start to slow down
Real m_slowDownRatio;  // how fast the windows slow down (smaller slows quicker)
Real m_speedUpRatio;  // how fast the windows speed up

};


//-----------------------------------------------------------------------------

class ProcessAnimateWindowSlideFromLeft : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSlideFromLeft( void );
	virtual ~ProcessAnimateWindowSlideFromLeft( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
private:
Coord2D m_maxVel;  // top speed windows travel in x and y
Int m_slowDownThreshold;  // when widnows get this close to their resting
																		// positions they start to slow down
Real m_slowDownRatio;  // how fast the windows slow down (smaller slows quicker)
Real m_speedUpRatio;  // how fast the windows speed up

};


//-----------------------------------------------------------------------------

class ProcessAnimateWindowSlideFromTop : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSlideFromTop( void );
	virtual ~ProcessAnimateWindowSlideFromTop( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
private:
Coord2D m_maxVel;  // top speed windows travel in x and y
Int m_slowDownThreshold;  // when widnows get this close to their resting
																		// positions they start to slow down
Real m_slowDownRatio;  // how fast the windows slow down (smaller slows quicker)
Real m_speedUpRatio;  // how fast the windows speed up

};

//-----------------------------------------------------------------------------
class ProcessAnimateWindowSlideFromTopFast : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSlideFromTopFast( void );
	virtual ~ProcessAnimateWindowSlideFromTopFast( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
private:
Coord2D m_maxVel;  // top speed windows travel in x and y
Int m_slowDownThreshold;  // when widnows get this close to their resting
																		// positions they start to slow down
Real m_slowDownRatio;  // how fast the windows slow down (smaller slows quicker)
Real m_speedUpRatio;  // how fast the windows speed up

};


//-----------------------------------------------------------------------------

class ProcessAnimateWindowSlideFromBottom : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSlideFromBottom( void );
	virtual ~ProcessAnimateWindowSlideFromBottom( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
private:
Coord2D m_maxVel;  // top speed windows travel in x and y
Int m_slowDownThreshold;  // when widnows get this close to their resting
																		// positions they start to slow down
Real m_slowDownRatio;  // how fast the windows slow down (smaller slows quicker)
Real m_speedUpRatio;  // how fast the windows speed up

};

//-----------------------------------------------------------------------------

class ProcessAnimateWindowSpiral : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSpiral( void );
	virtual ~ProcessAnimateWindowSpiral( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
private:
	Real m_deltaTheta;
	Int m_maxR;
};

//-----------------------------------------------------------------------------

class ProcessAnimateWindowSlideFromBottomTimed : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSlideFromBottomTimed( void );
	virtual ~ProcessAnimateWindowSlideFromBottomTimed( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void setMaxDuration(UnsignedInt maxDuration) { m_maxDuration = maxDuration; }

private:
	UnsignedInt m_maxDuration;

};

class ProcessAnimateWindowSlideFromRightFast : public ProcessAnimateWindow
{
public:

	ProcessAnimateWindowSlideFromRightFast( void );
	virtual ~ProcessAnimateWindowSlideFromRightFast( void );

	virtual void initAnimateWindow( wnd::AnimateWindow *animWin );
	virtual void initReverseAnimateWindow( wnd::AnimateWindow *animWin, UnsignedInt maxDelay = 0 );
	virtual Bool updateAnimateWindow( wnd::AnimateWindow *animWin );
	virtual Bool reverseAnimateWindow( wnd::AnimateWindow *animWin );
private:
Coord2D m_maxVel;  // top speed windows travel in x and y
Int m_slowDownThreshold;  // when widnows get this close to their resting
																		// positions they start to slow down
Real m_slowDownRatio;  // how fast the windows slow down (smaller slows quicker)
Real m_speedUpRatio;  // how fast the windows speed up

};


//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
