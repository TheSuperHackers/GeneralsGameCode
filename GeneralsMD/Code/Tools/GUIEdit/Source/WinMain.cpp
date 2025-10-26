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

// FILE: WinMain.cpp //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  GUIEdit.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       GUI Edit and window layout entry point
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "Common/FramePacer.h"
#include "Common/GameMemory.h"
#include "GameClient/GameWindowManager.h"
#include "Win32Device/GameClient/Win32Mouse.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "GUIEdit.h"
#include "EditWindow.h"
#include "DialogProc.h"
#include "LayoutScheme.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////
static const char *szWindowClass = "GUIEdit";

// PUBLIC DATA ////////////////////////////////////////////////////////////////
HINSTANCE ApplicationHInstance;				///< main application instance
HWND ApplicationHWnd;							///< main application HWnd
Win32Mouse *TheWin32Mouse = NULL;	///< for Win32 mouse
const char *gAppPrefix = "ge_"; /// So GuiEdit can have a different debug log file name if we need it

const Char *g_strFile = "data\\Generals.str";
const Char *g_csfFile = "data\\%s\\Generals.csf";

///////////////////////////////////////////////////////////////////////////////
// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutCallback(HWND, UINT, WPARAM, LPARAM);

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// initInstance ===============================================================
//
//   FUNCTION: initInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
//=============================================================================
static BOOL initInstance( HINSTANCE hInstance, int nCmdShow )
{

	// store application instance
	ApplicationHInstance = hInstance;

	// create app window and keep handle
	ApplicationHWnd = CreateWindowEx( 0,											// extended style
																		szWindowClass,					// window class name
																		"GUIEdit",							// window name
																		WS_OVERLAPPEDWINDOW,		// window styles
																		0,											// x position
																		0,											// y position
																		GetSystemMetrics( SM_CXSCREEN ), // width
																		GetSystemMetrics( SM_CYSCREEN ),  // height
																		NULL,										// parent
																		NULL,										// menu
																		ApplicationHInstance,		// instance
																		NULL );									// creation data

	if( ApplicationHWnd == NULL )
		return FALSE;

	// display the window
	ShowWindow( ApplicationHWnd, SW_MAXIMIZE );
	UpdateWindow( ApplicationHWnd );

	return TRUE;

}

// registerClass ==============================================================
//
//  FUNCTION: registerClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
//=============================================================================
static ATOM registerClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof( WNDCLASSEX );

	wcex.style					= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc		= (WNDPROC)WndProc;
	wcex.cbClsExtra			= 0;
	wcex.cbWndExtra			= 0;
	wcex.hInstance			= hInstance;
	wcex.hIcon					= LoadIcon(hInstance, (LPCTSTR)GUIEDIT_LARGE_ICON);
	wcex.hCursor				= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject( BLACK_BRUSH );
	wcex.lpszMenuName		=	(LPCSTR)GUIEDIT_MENU;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm				= LoadIcon(wcex.hInstance, (LPCTSTR)GUIEDIT_SMALL_ICON);

	return RegisterClassEx( &wcex );

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// WinMain ====================================================================
/** Entry point for application */
//=============================================================================
Int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	Int returnValue;
	HACCEL hAccelTable;
	Bool quit = FALSE;

	/// @todo remove this force set of working directory later
	Char buffer[ _MAX_PATH ];
	GetModuleFileName( NULL, buffer, sizeof( buffer ) );
	Char *pEnd = buffer + strlen( buffer );
	while( pEnd != buffer )
	{
		if( *pEnd == '\\' )
		{
			*pEnd = 0;
			break;
		}
		pEnd--;
	}
	::SetCurrentDirectory(buffer);

	// initialize the memory manager early
	initMemoryManager();

	// register a class for our window with the OS
	registerClass( hInstance );

	// Perform application initialization:
	if( !initInstance( hInstance, nCmdShow ) )
		return FALSE;

	// load accellerator table
	hAccelTable = LoadAccelerators( hInstance, (LPCTSTR)GUIEDIT_ACCELERATORS );

	// initialize the common controls
	INITCOMMONCONTROLSEX controls;
	controls.dwSize = sizeof( controls );
	controls.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx( &controls );

	// initialize GUIEdit data
	TheEditor = new GUIEdit;
	if( TheEditor == NULL )
		return FALSE;
	TheEditor->init();

	TheFramePacer = new FramePacer();

	//
	// see if we have any messages to process, a NULL window handle tells the
	// OS to look at the main window associated with the calling thread, us!
	//
	while( quit == FALSE )
	{

		// is there is message ready for us?
		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{

			// process ALL messages waiting
			while( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{

				// get the message
				returnValue = GetMessage( &msg, NULL, 0, 0 );

				// check for quitting
				if( returnValue == 0 )
					quit = TRUE;

				// translate accelerator messages
				if( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
				{

					// translate and dispatch the message
					TranslateMessage( &msg );
					DispatchMessage( &msg );

				}

			}

		}
		else
		{

			// update our universe
			TheEditor->update();
			Sleep(1);

		}

	}

	// shutdown GUIEdit data
	delete TheFramePacer;
	TheFramePacer = NULL;

	delete TheEditor;
	TheEditor = NULL;

	shutdownMemoryManager();

	return msg.wParam;

}

// WndProc ====================================================================
//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//=============================================================================
LRESULT CALLBACK WndProc( HWND hWnd, UINT message,
													WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_MOUSEMOVE:
		{

			// keep focus in our app
			if( GetFocus() != hWnd )
				SetFocus( hWnd );
			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_COMMAND:
		{
			Int controlID = LOWORD( wParam );
//			Int nofifyCode = HIWORD( wParam );
//			HWND hWndControl = (HWND)lParam;

			switch( controlID )
			{

				// --------------------------------------------------------------------
				case MENU_EXIT:
					TheEditor->menuExit();
					break;

				//---------------------------------------------------------------------
				case MENU_NEW:
					TheEditor->menuNew();
					break;

				// --------------------------------------------------------------------
				case MENU_OPEN:
					TheEditor->menuOpen();
					break;

				// --------------------------------------------------------------------
				case MENU_SAVE:
					TheEditor->menuSave();
					break;

				// --------------------------------------------------------------------
				case MENU_SAVEAS:
					TheEditor->menuSaveAs();
					break;

				// --------------------------------------------------------------------
				case MENU_COPY:
					TheEditor->menuCopy();
					break;

				// --------------------------------------------------------------------
				case MENU_CUT:
					TheEditor->menuCut();
					break;

				// --------------------------------------------------------------------
				case MENU_PASTE:
					TheEditor->menuPaste();
					break;

				// --------------------------------------------------------------------
				case MENU_CALLBACKS:
					DialogBox( TheEditor->getInstance(), (LPCTSTR)CALLBACK_EDITOR_DIALOG,
										 TheEditor->getWindowHandle(), CallbackEditorDialogProc );
					break;

				// --------------------------------------------------------------------
				case MENU_GRID_SETTINGS:
					DialogBox( TheEditor->getInstance(), (LPCTSTR)GRID_SETTINGS_DIALOG,
										 TheEditor->getWindowHandle(), GridSettingsDialogProc );
					break;

				// --------------------------------------------------------------------
				case MENU_SHOW_HIDDEN_OUTLINES:
					TheEditor->setShowHiddenOutlines( !TheEditor->getShowHiddenOutlines() );
					break;

				// --------------------------------------------------------------------
				case MENU_SHOW_SEE_THRU_OUTLINES:
					TheEditor->setShowSeeThruOutlines( !TheEditor->getShowSeeThruOutlines() );
					break;

				// --------------------------------------------------------------------
				case MENU_TEST_MODE:

					// switch to test mode or edit mode
					if( TheEditor)
					{
						EditMode mode = TheEditor->getMode();

						if( mode != MODE_TEST_RUN )
							TheEditor->setMode( MODE_TEST_RUN );
						else
							TheEditor->setMode( MODE_EDIT );

					}
					break;

				// --------------------------------------------------------------------
				case MENU_BACKGROUND_COLOR:

					if( TheEditWindow )
					{
						RGBColorReal color = TheEditWindow->getBackgroundColor();
						RGBColorInt *newColor;
						POINT mouse;

						// get mouse point
						GetCursorPos( &mouse );

						// select a new color
						newColor = SelectColor( (Int)(255.0f * color.red),
																		(Int)(255.0f * color.green),
																		(Int)(255.0f * color.blue),
																		(Int)(255.0f * color.alpha),
																		mouse.x, mouse.y );
						if( newColor )
						{

							color.red   = (Real)newColor->red / 255.0f;
							color.green = (Real)newColor->green / 255.0f;
							color.blue  = (Real)newColor->blue / 255.0f;
							color.alpha = (Real)newColor->alpha / 255.0f;
							TheEditWindow->setBackgroundColor( color );

						}

					}

					break;

				// --------------------------------------------------------------------
				case MENU_SCHEME:
				{

					// open the default scheme information
					if( TheDefaultScheme )
						TheDefaultScheme->openDialog();
					break;

				}

				// --------------------------------------------------------------------
				case MENU_ABOUT:
				{

					DialogBox( ApplicationHInstance, (LPCTSTR)ABOUT_DIALOG,
										 hWnd, (DLGPROC)AboutCallback );
					break;

				}


				// --------------------------------------------------------------------
				default:
				{

					return DefWindowProc( hWnd, message, wParam, lParam );

				}

			}

			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_CLOSE:
		{

			// close gets initiated from the 'x' in the top right of the window
			TheEditor->menuExit();
			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_KEYDOWN:
		{
			Int virtualKey = wParam;
//			Int keyData = lParam;
			Bool controlDown = BitIsSet( GetKeyState( VK_CONTROL ), 0x1000 );

			switch( virtualKey )
			{

				// --------------------------------------------------------------------
				case VK_ESCAPE:
				{

					// unselect all windows
					if( TheEditor )
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
							TheEditor->setMode( MODE_EDIT );
						else
							TheEditor->clearSelections();
					break;

				}

				// --------------------------------------------------------------------
				case VK_DELETE:
				{

					// delete all selected windows
					if( TheEditor )
						TheEditor->deleteSelected();
					break;

				}

				// --------------------------------------------------------------------
				case VK_LEFT:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{

							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);

							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;

							temp = TheEditWindow->getDragMoveDest();
							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.x -= TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.x--;
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}
				// --------------------------------------------------------------------
				case VK_RIGHT:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{

							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);

							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;

							temp = TheEditWindow->getDragMoveDest();
							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.x += TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.x++;
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}
				// --------------------------------------------------------------------
				case VK_UP:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{

							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);

							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;

							temp = TheEditWindow->getDragMoveDest();

							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.y -= TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.y--;
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}
				// --------------------------------------------------------------------
				case VK_DOWN:
				{

					// delete all selected windows
					if( TheEditor && TheEditWindow)
					{
						if(TheEditor->getMode() == MODE_EDIT)
						{

							if(TheEditor->selectionCount() <= 0 )
								break;
							ICoord2D zero;
							zero.x = zero.y = 0;
							TheEditWindow->setDragMoveOrigin(&zero);
							TheEditWindow->setDragMoveDest(&zero);

							TheEditor->setMode(MODE_KEYBOARD_MOVE);
						}
						if(TheEditor->getMode() == MODE_KEYBOARD_MOVE)
						{
							ICoord2D temp;

							temp = TheEditWindow->getDragMoveDest();
							if(!controlDown && TheEditor->isGridSnapOn())
							{
								temp.y += TheEditor->getGridResolution();
								TheEditor->gridSnapLocation(&temp,&temp);
							}
							else
							{
								temp.y++;
							}
							TheEditWindow->setDragMoveDest(&temp);
						}

					}
					break;

				}

				// --------------------------------------------------------------------
				case VK_RETURN:
				{
					if( TheEditor && TheEditWindow && TheEditor->getMode() == MODE_KEYBOARD_MOVE )
					{
						ICoord2D tempOrigin, tempDest;
						tempDest = TheEditWindow->getDragMoveDest();
						tempOrigin = TheEditWindow->getDragMoveOrigin();

						// move the windows
						TheEditor->dragMoveSelectedWindows( &tempOrigin, &tempDest );

						// go back to normal mode
						TheEditor->setMode( MODE_EDIT );
					}
					break;
				}

			}

			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_SIZE:
		{
			Int width  = LOWORD( lParam );
			Int height = HIWORD( lParam );

			// resize status bar
			if( TheEditor )
			{
				HWND statusBar = TheEditor->getStatusBarWindowHandle();

				if( statusBar )
				{
					RECT rect;
					Int barX, barY;
					Int barWidth, barHeight;

					// keep status window height the same
					GetWindowRect( statusBar, &rect );
					barWidth = width;
					barHeight = rect.bottom - rect.top;

					barX = 0;
					barY = height - barHeight;
					MoveWindow( statusBar, barX, barY, barWidth, barHeight, TRUE );

				}

			}

			return 0;

		}

		// ------------------------------------------------------------------------
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;

		}

		// ------------------------------------------------------------------------
		case WM_DESTROY:
		{

			PostQuitMessage(0);
			break;

		}

		// ------------------------------------------------------------------------
		default:
		{

			return DefWindowProc(hWnd, message, wParam, lParam);

		}

	}

	return DefWindowProc( hWnd, message, wParam, lParam );

}

// AboutCallback ==============================================================
/** Mesage handler for about box. */
//=============================================================================
LRESULT CALLBACK AboutCallback( HWND hDlg, UINT message,
																WPARAM wParam, LPARAM lParam )
{

	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;

}

