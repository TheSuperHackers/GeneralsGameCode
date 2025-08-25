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

// FILE: ControlBarResizer.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Sep 2002
//
//	Filename: 	ControlBarResizer.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	We want a "squished" control bar, this is the methods that will do it
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameClient/ControlBar.h"
#include "GameClient/ControlBarResizer.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Display.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

const FieldParse ControlBarResizer::m_controlBarResizerParseTable[] =
{
	{ "AltPosition",		INI::parseICoord2D,						NULL, offsetof( ResizerWindow, m_altPos ) },
	{ "AltSize",				INI::parseICoord2D,						NULL, offsetof( ResizerWindow, m_altSize ) },
	{ NULL,										NULL,													NULL, 0 }  // keep this last

};
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

ResizerWindow::ResizerWindow(void)
{
	m_defaultPos.x = m_defaultPos.y = 0;
	m_defaultSize.x = m_defaultSize.y = 0;
	m_altSize.x = m_altSize.y = 0;
	m_altPos.x = m_altPos.y = 0;
}

ControlBarResizer::ControlBarResizer( void )
{

}
ControlBarResizer::~ControlBarResizer( void )
{
	ResizerWindowList::iterator it = m_resizerWindowsList.begin();
	while (it != m_resizerWindowsList.end())
	{
		ResizerWindow *rWin = *it;
		if( !rWin )
		{
			it = m_resizerWindowsList.erase(it);
			continue;
		}
		delete rWin;
		it = m_resizerWindowsList.erase(it);
	}
	m_resizerWindowsList.clear();
}

void ControlBarResizer::init( void )
{
	INI ini;
	// Read from INI all the ControlBarSchemes
	ini.loadFileDirectory( AsciiString( "Data\\INI\\ControlBarResizer" ), INI_LOAD_OVERWRITE, NULL );

}


ResizerWindow *ControlBarResizer::findResizerWindow( AsciiString name )
{
	ResizerWindowList::iterator it = m_resizerWindowsList.begin();

	while (it != m_resizerWindowsList.end())
	{
		ResizerWindow *rWin = *it;
		if( !rWin )
		{
			DEBUG_ASSERTCRASH(FALSE,("There's no resizerWindow in ControlBarResizer::findResizerWindow"));
			it++;
			continue;
		}
		// find the scheme that best matches our resolution
		if(rWin->m_name.compare(name) == 0)
		{
			return rWin;
		}
		it ++;
	}
	return NULL;
}

ResizerWindow *ControlBarResizer::newResizerWindow( AsciiString name )
{
	ResizerWindow *newRwin = NEW ResizerWindow;
	if(!newRwin)
		return NULL;

	newRwin->m_name = name;
	GameWindow *win = NULL;
	win = TheWindowManager->winGetWindowFromId(NULL, TheNameKeyGenerator->nameToKey(name));
	if( !win )
	{
		DEBUG_ASSERTCRASH(win,("ControlBarResizer::newResizerWindow could not find window %s Are you sure that window is loaded yet?", name.str()) );
		delete newRwin;
		return NULL;
	}
	win->winGetPosition(&newRwin->m_defaultPos.x,&newRwin->m_defaultPos.y);
	win->winGetSize(&newRwin->m_defaultSize.x,&newRwin->m_defaultSize.y);
	m_resizerWindowsList.push_back(newRwin);
	return newRwin;
}
void ControlBarResizer::sizeWindowsDefault( void )
{
	ResizerWindowList::iterator it = m_resizerWindowsList.begin();
	GameWindow *win = NULL;
	while (it != m_resizerWindowsList.end())
	{
		ResizerWindow *rWin = *it;
		if( !rWin )
		{
			DEBUG_ASSERTCRASH(FALSE,("There's no resizerWindow in ControlBarResizer::sizeWindowsDefault"));
			it++;
			continue;
		}
		win = TheWindowManager->winGetWindowFromId(NULL, TheNameKeyGenerator->nameToKey(rWin->m_name));
		if(!win)
		{
			it++;
			continue;
		}
		win->winSetPosition(rWin->m_defaultPos.x, rWin->m_defaultPos.y);
		win->winSetSize(rWin->m_defaultSize.x, rWin->m_defaultSize.y);
		DEBUG_LOG(("sizeWindowsDefault:%s pos X:%d pos Y: %d size X:%d sizeY: %d",rWin->m_name.str(),rWin->m_defaultPos.x, rWin->m_defaultPos.y,rWin->m_defaultSize.x, rWin->m_defaultSize.y ));
		it ++;
	}
}
void ControlBarResizer::sizeWindowsAlt( void )
{
	ResizerWindowList::iterator it = m_resizerWindowsList.begin();
	GameWindow *win = NULL;
	Real x = (Real)TheDisplay->getWidth() / DEFAULT_DISPLAY_WIDTH;
	Real y = (Real)TheDisplay->getHeight() / DEFAULT_DISPLAY_HEIGHT;
	while (it != m_resizerWindowsList.end())
	{
		ResizerWindow *rWin = *it;
		if( !rWin )
		{
			DEBUG_ASSERTCRASH(FALSE,("There's no resizerWindow in ControlBarResizer::sizeWindowsDefault"));
			it++;
			continue;
		}
		win = TheWindowManager->winGetWindowFromId(NULL, TheNameKeyGenerator->nameToKey(rWin->m_name));
		if(!win)
		{
			it++;
			continue;
		}

		win->winSetPosition(rWin->m_altPos.x * x, rWin->m_altPos.y * y);
		if(rWin->m_altSize.x >0 || rWin->m_altSize.y > 0)
			win->winSetSize(rWin->m_altSize.x *x, rWin->m_altSize.y *y);
		DEBUG_LOG(("sizeWindowsAlt:%s pos X:%d pos Y: %d size X:%d sizeY: %d",rWin->m_name.str(), rWin->m_altPos.x*x, rWin->m_altPos.y*y,rWin->m_altSize.x*x, rWin->m_altSize.y *y));
		it ++;
	}
}


void INI::parseControlBarResizerDefinition( INI* ini )
{
//	AsciiString name;
//	ResizerWindow *rWin = NULL;
//
//	// read the name
//	const char* c = ini->getNextToken();
//	name.set( c );
//
////	ControlBarResizer *resizer = TheControlBar->getControlBarResizer();
//	if( !resizer )
//	{
//		//We don't need it if we're in the builder... which doesn't have this.
//		return;
//	}
//	rWin = resizer->findResizerWindow( name );
//	if( rWin == NULL )
//	{
//
//		// image not found, create a new one
//		rWin = resizer->newResizerWindow(name);
//		DEBUG_ASSERTCRASH( rWin, ("parseControlBarResizerDefinition: unable to allocate ResizerWindow for '%s'",
//															name.str()) );
//
//	}  // end if
//
//	// parse the ini definition
//	ini->initFromINI( rWin, resizer->getFieldParse());
//
}  // end parseMappedImage


//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

