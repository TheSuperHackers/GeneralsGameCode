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

// FILE: GameWindowTransitions.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Dec 2002
//
//	Filename: 	GameWindowTransitions.cpp
//
//	author:		Chris Huybregts
//
//	purpose:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameLogic/GameLogic.h"
#include "GameClient/GameWindowTransitions.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
GameWindowTransitionsHandler *TheTransitionHandler = NULL;
const FieldParse GameWindowTransitionsHandler::m_gameWindowTransitionsFieldParseTable[] =
{

	{ "Window",		GameWindowTransitionsHandler::parseWindow,	NULL, NULL	},
	{ "FireOnce",	INI::parseBool,															NULL, offsetof( TransitionGroup, m_fireOnce) 	},

	{ NULL,										NULL,													NULL, 0 }

};

void INI::parseWindowTransitions( INI* ini )
{
	AsciiString name;
	TransitionGroup *g;

	// read the name
	const char* c = ini->getNextToken();
	name.set( c );

	// find existing item if present

	DEBUG_ASSERTCRASH( TheTransitionHandler, ("parseWindowTransitions: TheTransitionHandler doesn't exist yet") );
	if( !TheTransitionHandler )
		return;

	// If we have a previously allocated control bar, this will return a cleared out pointer to it so we
	// can overwrite it
	g = TheTransitionHandler->getNewGroup( name );

	// sanity
	DEBUG_ASSERTCRASH( g, ("parseWindowTransitions: Unable to allocate group '%s'", name.str()) );

	// parse the ini definition
	ini->initFromINI( g, TheTransitionHandler->getFieldParse() );


}
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
Transition *getTransitionForStyle( Int style )
{
	switch (style) {
	case TRANSITION_FLASH:
		return NEW FlashTransition;
	case BUTTON_TRANSITION_FLASH:
		return NEW ButtonFlashTransition;
	case WIN_FADE_TRANSITION:
		return NEW FadeTransition;
	case WIN_SCALE_UP_TRANSITION:
		return NEW ScaleUpTransition;
	case MAINMENU_SCALE_UP_TRANSITION:
		return NEW MainMenuScaleUpTransition;
	case TEXT_TYPE_TRANSITION:
		return NEW TextTypeTransition;
	case SCREEN_FADE_TRANSITION:
		return NEW ScreenFadeTransition;
	case COUNT_UP_TRANSITION:
		return NEW CountUpTransition;
	case FULL_FADE_TRANSITION:
		return NEW FullFadeTransition;
	case TEXT_ON_FRAME_TRANSITION:
		return NEW TextOnFrameTransition;
	case REVERSE_SOUND_TRANSITION:
		return NEW ReverseSoundTransition;

	case MAINMENU_MEDIUM_SCALE_UP_TRANSITION:
		return NEW MainMenuMediumScaleUpTransition;
	case MAINMENU_SMALL_SCALE_DOWN_TRANSITION:
		return NEW MainMenuSmallScaleDownTransition;
	case CONTROL_BAR_ARROW_TRANSITION:
		return NEW ControlBarArrowTransition;
	case SCORE_SCALE_UP_TRANSITION:
		return NEW ScoreScaleUpTransition;

	default:
		DEBUG_ASSERTCRASH(FALSE, ("getTransitionForStyle:: An invalid style was passed in. Style = %d", style));
		return NULL;
	}
	return NULL;
}

TransitionWindow::TransitionWindow( void )
{
	m_currentFrameDelay = m_frameDelay = 0;
	m_style = 0;

	m_winID = NAMEKEY_INVALID;
	m_win = NULL;
	m_transition = NULL;
}

TransitionWindow::~TransitionWindow( void )
{
	if (m_win)
		m_win->unlinkTransitionWindow(this);

	m_win = NULL;

	delete m_transition;
	m_transition = NULL;
}

Bool TransitionWindow::init( void )
{
	m_winID = TheNameKeyGenerator->nameToKey(m_winName);
	m_win		= TheWindowManager->winGetWindowFromId(NULL, m_winID);
	m_currentFrameDelay = m_frameDelay;
//	DEBUG_ASSERTCRASH( m_win, ("TransitionWindow::init Failed to find window %s", m_winName.str()));
//	if( !m_win )
//		return FALSE;

	delete m_transition;
	m_transition = getTransitionForStyle( m_style );
	m_transition->init(m_win);

	// TheSuperHackers @fix Mauller 15/05/2025 Link TransitionWindow to the GameWindow so the GameWindow can unlink itself when it is destroyed
	if(m_win)
		m_win->linkTransitionWindow(this);

	return TRUE;
}

void TransitionWindow::update( Int frame )
{
	if(frame < m_currentFrameDelay || frame > (m_currentFrameDelay + m_transition->getFrameLength()))
		return;

	if(m_transition)
		m_transition->update( frame - m_currentFrameDelay);
}

Bool TransitionWindow::isFinished( void )
{
	if(m_transition)
		return m_transition->isFinished();
	return TRUE;
}

void TransitionWindow::reverse( Int totalFrames )
{
	//m_currentFrameDelay = totalFrames - (m_transition->getFrameLength() + m_frameDelay);
	if(m_transition)
		m_transition->reverse();
}

void TransitionWindow::skip( void )
{
	if(m_transition)
		m_transition->skip();
}

void TransitionWindow::draw( void )
{
	if(m_transition)
		m_transition->draw();
}

void TransitionWindow::unlinkGameWindow(GameWindow* win)
{
	if (m_win != win)
		return;

	m_transition->unlinkGameWindow(win);
	m_win = NULL;
}

Int TransitionWindow::getTotalFrames( void )
{
	if(m_transition)
	{
		return m_frameDelay + m_transition->getFrameLength();
	}

	return m_frameDelay;
}

//-----------------------------------------------------------------------------
TransitionGroup::TransitionGroup( void )
{
	m_currentFrame = 0;
	m_fireOnce = FALSE;
}

TransitionGroup::~TransitionGroup( void )
{
	TransitionWindowList::iterator it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		delete tWin;
		it = m_transitionWindowList.erase(it);
	}
}

void TransitionGroup::init( void )
{
	m_currentFrame = 0;
	m_directionMultiplier = 1;
	TransitionWindowList::iterator it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		tWin->init();
		it++;
	}

}

void TransitionGroup::update( void )
{
	m_currentFrame += m_directionMultiplier; // we go forward or backwards depending.
	TransitionWindowList::iterator it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		tWin->update(m_currentFrame);
		it++;
	}
}

Bool TransitionGroup::isFinished( void )
{
	TransitionWindowList::iterator it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		if(tWin->isFinished() == FALSE)
			return FALSE;
		it++;
	}

	return TRUE;
}

void TransitionGroup::reverse( void )
{
	Int totalFrames =0;
	m_directionMultiplier = -1;

	TransitionWindowList::iterator it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		Int winFrames = tWin->getTotalFrames();
		if(winFrames > totalFrames)
			totalFrames = winFrames;
		it++;
	}
	it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		tWin->reverse(totalFrames);
		it++;
	}
	m_currentFrame = totalFrames;
//	m_currentFrame ++;
}

Bool TransitionGroup::isReversed( void )
{
	if(m_directionMultiplier < 0)
		return TRUE;
	return FALSE;
}

void TransitionGroup::skip ( void )
{
	TransitionWindowList::iterator it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		tWin->skip();
		it++;
	}
}

void TransitionGroup::draw ( void )
{
	TransitionWindowList::iterator it = m_transitionWindowList.begin();
	while (it != m_transitionWindowList.end())
	{
		TransitionWindow *tWin = *it;
		tWin->draw();
		it++;
	}
}

void TransitionGroup::addWindow( TransitionWindow *transWin )
{
	if(!transWin)
		return;
	m_transitionWindowList.push_back(transWin);
}

//-----------------------------------------------------------------------------

GameWindowTransitionsHandler::GameWindowTransitionsHandler(void)
{
	m_currentGroup = NULL;
	m_pendingGroup = NULL;
	m_drawGroup = NULL;
	m_secondaryDrawGroup = NULL;

}

GameWindowTransitionsHandler::~GameWindowTransitionsHandler( void )
{
	m_currentGroup = NULL;
	m_pendingGroup = NULL;
	m_drawGroup = NULL;
	m_secondaryDrawGroup = NULL;

	TransitionGroupList::iterator it = m_transitionGroupList.begin();
	while( it != m_transitionGroupList.end() )
	{
		TransitionGroup *g = *it;
		delete g;
		it = m_transitionGroupList.erase(it);
	}
}

void GameWindowTransitionsHandler::init(void )
{
	m_currentGroup = NULL;
	m_pendingGroup = NULL;
	m_drawGroup = NULL;
	m_secondaryDrawGroup = NULL;
}

void GameWindowTransitionsHandler::load(void )
{
	INI ini;
	// Read from INI all the ControlBarSchemes
	ini.loadFileDirectory( "Data\\INI\\WindowTransitions", INI_LOAD_OVERWRITE, NULL );

}

void GameWindowTransitionsHandler::reset( void )
{
	m_currentGroup = NULL;
	m_pendingGroup = NULL;
	m_drawGroup = NULL;
	m_secondaryDrawGroup = NULL;

}

void GameWindowTransitionsHandler::update( void )
{
	if(m_drawGroup != m_currentGroup)
		m_secondaryDrawGroup = m_drawGroup;
	else
		m_secondaryDrawGroup = NULL;

	m_drawGroup = m_currentGroup;
	if(m_currentGroup && !m_currentGroup->isFinished())
		m_currentGroup->update();

	if(m_currentGroup && m_currentGroup->isFinished() && m_currentGroup->isFireOnce())
	{
		m_currentGroup = NULL;
	}

	if(m_currentGroup && m_pendingGroup && m_currentGroup->isFinished())
	{
		m_currentGroup = m_pendingGroup;
		m_pendingGroup = NULL;
	}

	if(!m_currentGroup && m_pendingGroup)
	{
		m_currentGroup = m_pendingGroup;
		m_pendingGroup = NULL;
	}

	if(m_currentGroup && m_currentGroup->isFinished() && m_currentGroup->isReversed())
		m_currentGroup = NULL;
}


void GameWindowTransitionsHandler::draw( void )
{
//	if( TheGameLogic->getFrame() > 0 )//if( areTransitionsEnabled() ) //KRIS
	if(m_drawGroup)
			m_drawGroup->draw();
	if(m_secondaryDrawGroup)
		m_secondaryDrawGroup->draw();
}

void GameWindowTransitionsHandler::setGroup(AsciiString groupName, Bool immidiate )
{
	if(groupName.isEmpty() && immidiate)
		m_currentGroup = NULL;
	if(immidiate && m_currentGroup)
	{
		m_currentGroup->skip();
		m_currentGroup = findGroup(groupName);
		if(m_currentGroup)
			m_currentGroup->init();
		return;
	}

	if(m_currentGroup)
	{
		if(!m_currentGroup->isFireOnce() && !m_currentGroup->isReversed())
			m_currentGroup->reverse();
		m_pendingGroup = findGroup(groupName);
		if(m_pendingGroup)
			m_pendingGroup->init();
		return;
	}

	m_currentGroup = findGroup(groupName);
	if(m_currentGroup)
		m_currentGroup->init();



}

void GameWindowTransitionsHandler::reverse( AsciiString groupName )
{
	TransitionGroup *g = findGroup(groupName);
	if( m_currentGroup == g )
	{
		m_currentGroup->reverse();
		return;
	}
	if( m_pendingGroup == g)
	{
		m_pendingGroup = NULL;
		return;
	}
	if(m_currentGroup)
		m_currentGroup->skip();
	if(m_pendingGroup)
		m_pendingGroup->skip();

	m_currentGroup = g;
	m_currentGroup->init();
	m_currentGroup->skip();
	m_currentGroup->reverse();
	m_pendingGroup = NULL;
}

void GameWindowTransitionsHandler::remove( AsciiString groupName,  Bool skipPending )
{
	TransitionGroup *g = findGroup(groupName);
	if(m_pendingGroup == g)
	{
		if(skipPending)
			m_pendingGroup->skip();

		m_pendingGroup = NULL;
	}
	if(m_currentGroup == g)
	{
		m_currentGroup->skip();
		m_currentGroup = NULL;
		if(m_pendingGroup)
			m_currentGroup = m_pendingGroup;
	}
}

TransitionGroup *GameWindowTransitionsHandler::getNewGroup( AsciiString name )
{
	if(name.isEmpty())
		return NULL;

	// test to see if we're trying to add an already exisitng group.
	if(findGroup(name))
	{
		DEBUG_ASSERTCRASH(FALSE, ("GameWindowTransitionsHandler::getNewGroup - We already have a group %s", name.str()));
		return NULL;
	}
	TransitionGroup *g = NEW TransitionGroup;
	g->setName(name);
	m_transitionGroupList.push_back(g);
	return g;
}

Bool GameWindowTransitionsHandler::isFinished( void )
{
	if(m_currentGroup)
		return m_currentGroup->isFinished();
	return TRUE;
}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
TransitionGroup *GameWindowTransitionsHandler::findGroup( AsciiString groupName )
{
	if(groupName.isEmpty())
		return NULL;

	TransitionGroupList::iterator it = m_transitionGroupList.begin();
	while( it != m_transitionGroupList.end() )
	{
		TransitionGroup *g = *it;
		if(groupName.compareNoCase(g->getName()) == 0)
			return g;
		it++;
	}
	return NULL;
}

void GameWindowTransitionsHandler::parseWindow( INI* ini, void *instance, void *store, const void *userData )
{
	static const FieldParse myFieldParse[] =
		{
			{ "WinName",				INI::parseAsciiString,		NULL,									offsetof( TransitionWindow, m_winName ) },
      { "Style",					INI::parseLookupList,			TransitionStyleNames,	offsetof( TransitionWindow, m_style ) },
			{ "FrameDelay",			INI::parseInt,						NULL,									offsetof( TransitionWindow, m_frameDelay ) },
			{ NULL,							NULL,											NULL, 0 }
		};
	TransitionWindow *transWin = NEW TransitionWindow;
	ini->initFromINI(transWin, myFieldParse);
	((TransitionGroup*)instance)->addWindow(transWin);
}

