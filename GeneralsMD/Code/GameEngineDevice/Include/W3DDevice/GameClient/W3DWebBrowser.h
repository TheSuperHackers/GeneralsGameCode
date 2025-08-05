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

///// W3DWebBrowser.h ////////////////////////
// July 2002, Bryan Cleveland

#pragma once

#ifndef W3DWEBBROWSER_H
#define W3DWEBBROWSER_H

#include "GameNetwork/WOLBrowser/WebBrowser.h"

class TextureClass;
class Image;
class GameWindow;

class W3DWebBrowser : public WebBrowser
{
	public:
		W3DWebBrowser();

		virtual Bool createBrowserWindow(const char *tag, GameWindow *win);
		virtual void closeBrowserWindow(GameWindow *win);

};

#endif // #ifndef W3DWEBBROWSER_H
