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

//******************************************************************************************
//
// Earth And Beyond
// Copyright (c) 2002 Electronic Arts , Inc.  -  Westwood Studios
//
// File Name		: dx8webbrowser.h
// Description		: Implementation of D3D Embedded Browser Wrapper
// Author			: Darren Schueller
// Date of Creation	: 6/4/2002
//
//******************************************************************************************
// $Header: $ 
//******************************************************************************************

#ifndef DX8_WEBBROWSER_H
#define DX8_WEBBROWSER_H

#include <windows.h>
#include "d3d8.h"

// ***********************************
// Set this to 0 to remove all embedded browser code.
//
#define ENABLE_EMBEDDED_BROWSER		1
//
// ***********************************

#if ENABLE_EMBEDDED_BROWSER

// These options must match the browser option bits defined in the BrowserEngine code.
// Look in febrowserengine.h
#define BROWSEROPTION_SCROLLBARS		0x0001
#define BROWSEROPTION_3DBORDER		0x0002

struct IDirect3DDevice8;

/**
** DX8WebBrowser
**
** DX8 interface wrapper class.  This encapsulates the BrowserEngine interface.
*/
class DX8WebBrowser
{
public:

	static bool			Initialize(	const char* badpageurl = 0,
											const char* loadingpageurl = 0,
											const char* mousefilename = 0,
											const char* mousebusyfilename = 0);			//Initialize the Embedded Browser

	static void			Shutdown(void);			// Shutdown the embedded browser.  Will close any open browsers.

	static void			Update(void);				// Copies all browser contexts to D3D Image surfaces.
	static void			Render(int backbufferindex);	//Draws all browsers to the backbuffer.

	// Creates a browser with the specified name
	static void			CreateBrowser(const char* browsername, const char* url, int x, int y, int w, int h, int updateticks = 0, LONG options = BROWSEROPTION_SCROLLBARS | BROWSEROPTION_3DBORDER, LPDISPATCH gamedispatch = 0);

	// Destroys the browser with the specified name
	static void			DestroyBrowser(const char* browsername);

	// Returns true if a browser with the specified name is open.
	static bool			Is_Browser_Open(const char* browsername);

	// Navigates the specified browser to the specified page.
	static void			Navigate(const char* browsername, const char* url);

private:
	// The window handle of the application.  This is initialized by Initialize().
	static				HWND						hWnd;
};

#endif

#endif