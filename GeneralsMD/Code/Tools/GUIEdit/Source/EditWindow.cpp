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

// FILE: EditWindow.cpp ///////////////////////////////////////////////////////
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
// File name:  EditWindow.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       Main edit window for the GUI editing tool
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindowManager.h"
#include "W3DDevice/GameClient/W3DFileSystem.h"
#include "resource.h"
#include "EditWindow.h"
#include "GUIEdit.h"
#include "WinMain.h"
#include "HierarchyView.h"
#include "Properties.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/render2d.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Bool EditWindow::m_classRegistered = FALSE;  ///< class registered flag
const char *EditWindow::m_className = "EditWindowClass";  ///< edit window class name

///////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
EditWindow *TheEditWindow = NULL;  ///< edit window singleton

///////////////////////////////////////////////////////////////////////////////
// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// editProc ===================================================================
/** Window procedure for the edit window */
//=============================================================================
LRESULT CALLBACK EditWindow::editProc( HWND hWnd, UINT message, 
																			 WPARAM wParam, LPARAM lParam )
{
	
	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_TIMER:
		{
			Int timerID = wParam;

			if( TheEditWindow )
			{
			
				if( timerID == TIMER_EDIT_WINDOW_PULSE )
					TheEditWindow->updatePulse();

			}  // end if

			return 0;

		}  // end timer

		//-------------------------------------------------------------------------
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{

			TheEditWindow->mouseEvent( message, wParam, lParam );
			return 0;

		}  // end mouse events

		// ------------------------------------------------------------------------
		case WM_COMMAND:
		{
			Int controlID = LOWORD( wParam );
//			Int nofifyCode = HIWORD( wParam );
//			HWND hWndControl = (HWND)lParam;

			switch( controlID )
			{

				// --------------------------------------------------------------------
				case POPUP_MENU_NEW_WINDOW:
				case POPUP_MENU_NEW_PUSH_BUTTON:
				case POPUP_MENU_NEW_RADIO_BUTTON:
				case POPUP_MENU_NEW_TAB_CONTROL:
				case POPUP_MENU_NEW_CHECK_BOX:
				case POPUP_MENU_NEW_LISTBOX:
				case POPUP_MENU_NEW_COMBO_BOX:
				case POPUP_MENU_NEW_HORIZONTAL_SLIDER:
				case POPUP_MENU_NEW_VERTICAL_SLIDER:
				case POPUP_MENU_NEW_PROGRESS_BAR:
				case POPUP_MENU_NEW_TEXT_ENTRY:
				case POPUP_MENU_NEW_STATIC_TEXT:
				case POPUP_MENU_PROPERTIES:
				case POPUP_MENU_DELETE:
				case POPUP_MENU_BRING_TO_TOP:
				{
					ICoord2D pos;
					GameWindow *window;

					// get position the menu was initiated at
					TheEditWindow->getPopupMenuClickPos( &pos );

					//
					// if the position the user clicked at was on another window,
					// that window will become the parent of the new one
					//
					window = TheEditor->getWindowAtPos( pos.x, pos.y );

					// create new window at that location
					switch( controlID )
					{
						
						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_WINDOW:
							TheEditor->newWindow( GWS_USER_WINDOW, 
																		window, pos.x, pos.y, 
																		15 * TheEditor->getGridResolution(),
																		15 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_PUSH_BUTTON:
							TheEditor->newWindow( GWS_PUSH_BUTTON,
																		window, pos.x, pos.y, 
																		15 * TheEditor->getGridResolution(), 
																		3 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_CHECK_BOX:
							TheEditor->newWindow( GWS_CHECK_BOX,
																		window, pos.x, pos.y, 
																		15 * TheEditor->getGridResolution(),
																		3 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_RADIO_BUTTON:
							TheEditor->newWindow( GWS_RADIO_BUTTON,
																		window, pos.x, pos.y, 
																		15 * TheEditor->getGridResolution(), 
																		3 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_TAB_CONTROL:
							TheEditor->newWindow( GWS_TAB_CONTROL,
																		window, pos.x, pos.y, 
																		45 * TheEditor->getGridResolution(), 
																		30 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_LISTBOX:
							TheEditor->newWindow( GWS_SCROLL_LISTBOX,
																		window, pos.x, pos.y, 
																		20 * TheEditor->getGridResolution(), 
																		20 * TheEditor->getGridResolution() );
							break;
						
						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_COMBO_BOX:
							TheEditor->newWindow( GWS_COMBO_BOX,
																		window, pos.x, pos.y, 
																		15 * TheEditor->getGridResolution(), 
																		3 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_HORIZONTAL_SLIDER:
							TheEditor->newWindow( GWS_HORZ_SLIDER,
																		window, pos.x, pos.y, 
																		20 * TheEditor->getGridResolution(), 
																		GADGET_SIZE );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_VERTICAL_SLIDER:
							TheEditor->newWindow( GWS_VERT_SLIDER,
																		window, pos.x, pos.y, 
																		GADGET_SIZE, 
																		20 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_PROGRESS_BAR:
							TheEditor->newWindow( GWS_PROGRESS_BAR,
																		window, pos.x, pos.y, 
																		40 * TheEditor->getGridResolution(), 
																		GADGET_SIZE );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_TEXT_ENTRY:
							TheEditor->newWindow( GWS_ENTRY_FIELD,
																		window, pos.x, pos.y, 
																		20 * TheEditor->getGridResolution(), 
																		25 );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_NEW_STATIC_TEXT:
							TheEditor->newWindow( GWS_STATIC_TEXT,
																		window, pos.x, pos.y, 
																		15 * TheEditor->getGridResolution(), 
																		15 * TheEditor->getGridResolution() );
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_DELETE:
							TheEditor->deleteSelected();
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_BRING_TO_TOP:
							TheEditor->bringSelectedToTop();
							break;

						// ----------------------------------------------------------------
						case POPUP_MENU_PROPERTIES:
							if( window )
								InitPropertiesDialog( window, pos.x, pos.y );
							break;

					}  // end switch

					break;

				}  // end new window

			}  // end switch on control id

			return 0;

		}  // end command

		// ------------------------------------------------------------------------
		default:
		{
			
			break;

		}  // end default

	}  // end switch( message )

	return DefWindowProc( hWnd, message, wParam, lParam );

}  // end editProc

// EditWindow::registerEditWindowClass ========================================
/** Register a class with the windows OS for an edit window */
//=============================================================================
void EditWindow::registerEditWindowClass( void )
{
	WNDCLASSEX wcex;
	ATOM atom;
	HINSTANCE hInst = TheEditor->getInstance();

	wcex.cbSize = sizeof( WNDCLASSEX ); 

	wcex.style					= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc		= (WNDPROC)editProc;
	wcex.cbClsExtra			= 0;
	wcex.cbWndExtra			= 0;
	wcex.hInstance			= hInst;
	wcex.hIcon					= LoadIcon( hInst, (LPCTSTR)IDI_GUIEDIT );
	wcex.hCursor				= NULL;  //LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject( BLACK_BRUSH );
	wcex.lpszMenuName		=	NULL;
	wcex.lpszClassName	= m_className;
	wcex.hIconSm				= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	atom = RegisterClassEx( &wcex );
	
	// if successfully registered we don't ever need to do this again
	if( atom != 0 )
		m_classRegistered = TRUE;

}  // end registerEditWindowClass

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// EditWindow::EditWindow =====================================================
/** */
//=============================================================================
EditWindow::EditWindow( void )
{

	m_pulse = 0;
	m_size.x = 0;
	m_size.y = 0;
	m_bitDepth = 32;
	m_editWindowHWnd = NULL;
	m_assetManager = NULL;
	m_2DRender = NULL;
	m_w3dInitialized = FALSE;

	m_popupMenuClickPos.x = 0;
	m_popupMenuClickPos.y = 0;

	m_pickedWindow = NULL;

	m_dragMoveOrigin.x = 0;
	m_dragMoveOrigin.y = 0;
	m_dragMoveDest.x = 0;
	m_dragMoveDest.y = 0;

	m_dragSelecting = FALSE;
	m_selectRegion.lo.x = 0;
	m_selectRegion.lo.y = 0;
	m_selectRegion.hi.x = 0;
	m_selectRegion.hi.y = 0;

	m_resizingWindow = FALSE;
	m_windowToResize = NULL;
	m_resizeOrigin.x = 0;
	m_resizeOrigin.y = 0;
	m_resizeDest.x = 0;
	m_resizeDest.y = 0;

	m_backgroundColor.red   = 0.0f;
	m_backgroundColor.green = 0.3f;
	m_backgroundColor.blue  = 0.3f;
	m_backgroundColor.alpha = 1.0f;

	m_clipRegion.lo.x = 0;
	m_clipRegion.lo.y = 0;
	m_clipRegion.hi.x = 0;
	m_clipRegion.hi.y = 0;
	m_isClippedEnabled = FALSE;

}  // end EditWindow

// EditWindow::~EditWindow ====================================================
/** */
//=============================================================================
EditWindow::~EditWindow( void )
{

	// call the shutdown
	shutdown();

}  // end ~EditWindow

// EditWindow::init ===========================================================
/** Initialize the edit window */
//=============================================================================
void EditWindow::init( UnsignedInt clientWidth, UnsignedInt clientHeight )
{
	UnsignedInt windowStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER | WS_CAPTION;
	UnsignedInt extendedWindowStyle = 0;
	ICoord2D size;

	// register an edit window class with windows if not already done
	if( m_classRegistered == FALSE )
		registerEditWindowClass();

	// create 2D renderer
	m_2DRender = new Render2DClass;

	// save width and height
	size.x = clientWidth;
	size.y = clientHeight;
	setSize( &size );

	//
	// we want a client area the specified width and height, figure out how
	// big our window rectangle needs to be in order to have a client are
	// of that size
	//
	RECT clientRect;
	clientRect.left = 0;
	clientRect.top = 0;
	clientRect.right = m_size.x;
	clientRect.bottom = m_size.y;
	AdjustWindowRect( &clientRect, windowStyle, FALSE );

	// create the window
	m_editWindowHWnd = CreateWindowEx( extendedWindowStyle,  // extended window style
																		 m_className,  // class name
																		 "Edit Window",  // window name
																		 windowStyle,  // style bits
																		 0,  // x location
																		 0,  // y location
																		 clientRect.right - clientRect.left,  // width
																		 clientRect.bottom - clientRect.top,  // height,
																		 TheEditor->getWindowHandle(),  // parent
																		 NULL,  // menu
																		 TheEditor->getInstance(),  // instance
																		 NULL );  // creation parameters

	// display the window
	ShowWindow( m_editWindowHWnd, SW_SHOW );

	// create the file system for our asset file locations
	TheW3DFileSystem = new W3DFileSystem;  // our own file system for asset locations

	// initialize W3D
	WWMath::Init();
	WW3D::Init( m_editWindowHWnd );
	WW3D::Set_Screen_UV_Bias( TRUE );  ///< this makes text look good :)
	if( WW3D::Set_Render_Device( 0,
															 m_size.x,
															 m_size.y,
															 m_bitDepth,
															 TRUE ) != WW3D_ERROR_OK )
	{

		assert( 0 );
		shutdown();
		return;

	}  // end if

	// create asset manager
	m_assetManager = new WW3DAssetManager;
	assert( m_assetManager );
	m_assetManager->Set_WW3D_Load_On_Demand( true );

	// W3D is now initialized
	m_w3dInitialized = TRUE;

	// set a timer for updating visual pulse drawing
	SetTimer( m_editWindowHWnd, TIMER_EDIT_WINDOW_PULSE, 5, NULL );

}  // end init

// EditWindow::shutdown =======================================================
/** Shutdown edit window */
//=============================================================================
void EditWindow::shutdown( void )
{

	// delete 2d renderer
	delete m_2DRender;
	m_2DRender = NULL;

	// delete asset manager
	m_assetManager->Free_Assets();
	delete m_assetManager;

	// shutdown WW3D
	WW3D::Shutdown();
	WWMath::Shutdown();

	// delete the w3d file system
	delete TheW3DFileSystem;
	TheW3DFileSystem = NULL;

	// destroy the edit window
	if( m_editWindowHWnd )
		DestroyWindow( m_editWindowHWnd );
	m_editWindowHWnd = NULL;

	// unregister our edit window class
	UnregisterClass( m_className, TheEditor->getInstance() );
	m_classRegistered = FALSE;

}  // EditWindowShutdown

// EditWindow::updatePulse ====================================================
/** Update pulse from timer message */
//=============================================================================
void EditWindow::updatePulse( void )
{
	static Bool dir = 1;
	static Int stepSize = 4;
	static Int pulseMax = 175,
						 pulseMin = 75;

	// this is used for drawing pusling lines for moving stuff
	if( dir == 1 )
	{
		m_pulse += stepSize;
		if( m_pulse >= pulseMax )
			dir = 0;
	}  // end if
	else
	{
		m_pulse -= stepSize;
		if( m_pulse <= pulseMin )
			dir = 1;
	}  // end else

}  // end updatePulse

// EditWindow::mouseEvent =====================================================
/** A mouse event has occurred from our window procedure */
//=============================================================================
void EditWindow::mouseEvent( UnsignedInt windowsMessage,
														 WPARAM wParam, LPARAM lParam )
{
	Int x = LOWORD( lParam );
	Int y = HIWORD( lParam );
	Bool controlDown = BitIsSet( GetKeyState( VK_CONTROL ), 0x1000 );
	ICoord2D mouse;

	// setup mouse in nice struct
	mouse.x = x;
	mouse.y = y;

	// for mouse move messges always update the status bar
	if( windowsMessage == WM_MOUSEMOVE )
	{
		char buffer[ 64 ];
		ICoord2D mousePrint;

		// only print mouse positions in the edit window
		mousePrint.x = x;
		mousePrint.y = y;
		if( mousePrint.x < 0 )
			mousePrint.x = 0;
		if( mousePrint.x > m_size.x )
			mousePrint.x = m_size.x;
		if( mousePrint.y < 0 )
			mousePrint.y = 0;
		if( mousePrint.y > m_size.y )
			mousePrint.y = m_size.y;

		sprintf( buffer, "Mouse Location (X = %d, Y = %d)", 
						 mousePrint.x, mousePrint.y );
		TheEditor->statusMessage( STATUS_MOUSE_COORDS, buffer );

		// keep focus in our app
		if( GetFocus() != TheEditor->getWindowHandle() )
			SetFocus( TheEditor->getWindowHandle() );

	}  // end if

	//
	// if we're in test mode just pump all input through to the 
	// window system so we can see how everything acts
	//
	if( TheEditor->getMode() == MODE_TEST_RUN )
	{
		if( TheWin32Mouse )
			TheWin32Mouse->addWin32Event( windowsMessage, wParam, lParam, NO_TIME_FROM_WINDOWS);
		
		return;

	}  // end if

	//
	// If we're in the keyboard move, ignore the mouse
	//
	if (TheEditor->getMode() == MODE_KEYBOARD_MOVE)
		return;

	// do logic for each of the mouse messages
	switch( windowsMessage )
	{

		// ------------------------------------------------------------------------
		case WM_MOUSEMOVE:
		{

			//
			// this is really stupid, but the tree controls just don't
			// give enough hooks into all the events we need ... it's possible
			// to be in drag and drop mode due to menus opening up and the 
			// user clicking on blank area etc.  So if the mouse is in the
			// the edit window just make sure all our drag and drop stuff is
			// clear in the hierarchy
			//
			TheHierarchyView->setDragWindow( NULL );
			TheHierarchyView->setDragTarget( NULL );
			TheHierarchyView->setPopupTarget( NULL );

			if( TheEditor->getMode() == MODE_DRAG_MOVE )
			{

				// update destination for drag move
				m_dragMoveDest = mouse;

			}  // end if
			else if( m_dragSelecting )
			{

				// update drag selection region
				m_selectRegion.hi.x = x;
				m_selectRegion.hi.y = y;

			}  // end else if
			else if( m_resizingWindow )
			{

					// save the position of our mouse for resizing
					m_resizeDest = mouse;

			}  // end else if
			else
			{

				//
				// if we have ONE window selected and are close to an anchor corner
				// to resize it change the cursor to resize cursor
				//
				TheEditWindow->handleResizeAvailable( x, y );

			}  // end else

			break;

		}  // end mouse move

		// ------------------------------------------------------------------------
		case WM_LBUTTONDOWN:
		{

			//
			// if we're in drag move mode ignore this command ... usually this
			// doesn't happen cause you usually get into drag move mode by pressing
			// down, holding down, and dragging ... but it's possible to move
			// windows via the hierarchy view for those hard to reach windows
			//
			if( TheEditor->getMode() == MODE_DRAG_MOVE &&
					TheEditor->selectionCount() == 1 &&
					TheHierarchyView->getPopupTarget() != NULL )
				break;

			//
			// if we are in one of the resize modes then this click will
			// resize the window selected 
			//
			if( TheEditor->getMode() == MODE_RESIZE_TOP_LEFT ||
					TheEditor->getMode() == MODE_RESIZE_BOTTOM_RIGHT ||
					TheEditor->getMode() == MODE_RESIZE_TOP_RIGHT ||
					TheEditor->getMode() == MODE_RESIZE_BOTTOM_LEFT ||
					TheEditor->getMode() == MODE_RESIZE_TOP ||
					TheEditor->getMode() == MODE_RESIZE_BOTTOM ||
					TheEditor->getMode() == MODE_RESIZE_RIGHT ||
					TheEditor->getMode() == MODE_RESIZE_LEFT )
			{

				// mouse movements will now resize the window in the selection list
				m_windowToResize = TheEditor->getFirstSelected();
				if( m_windowToResize )
					m_resizingWindow = TRUE;

				// save our mouse position for the resizing process
				m_resizeOrigin = mouse;
				m_resizeDest = mouse;

			}  // end if
			else
			{
				GameWindow *window = TheEditor->getWindowAtPos( x, y );
				if( window )
				{

					//
					// if this window is not selected, then this window will become
					// the selected window instead of anything else that is selected.
					//
					if( TheEditor->isWindowSelected( window ) == FALSE )
					{

						//
						// if control key is not down we clear selections, otherwise
						// we will add this one to the select list
						//
						if( controlDown == FALSE )
							TheEditor->clearSelections();

						// select this window
						TheEditor->selectWindow( window );

					}  // end if
					else
					{

						//
						// this window is selected, if control is down we will
						// unselect this window
						//
						if( controlDown == TRUE )
						{
							TheEditor->unSelectWindow( window );
						}

						// only proceed into drag mode if we have something selected
						else if( TheEditor->isWindowSelected( window ) )
						{
							// set move locations
							m_dragMoveOrigin = mouse;
							m_dragMoveDest = mouse;

							// capture the mouse
							SetCapture( m_editWindowHWnd );

							// change to drag move mode and switch cursor
							TheEditor->setMode( MODE_DRAG_MOVE );

						}  // end if

					}  // end else


				}  // end if
				else
				{

					// start a drag selection box
					m_dragSelecting = TRUE;
					m_selectRegion.lo.x = x;
					m_selectRegion.lo.y = y;
					m_selectRegion.hi.x = x;
					m_selectRegion.hi.y = y;

				}  // end else

			}  // end if

			break;

		}  // end left button down 

		// ------------------------------------------------------------------------
		case WM_LBUTTONUP:
		{

			// exit drag move mode
			if( TheEditor->getMode() == MODE_DRAG_MOVE )
			{

				// move the windows
				if ((m_dragMoveOrigin.x != m_dragMoveDest.x) || (m_dragMoveOrigin.y != m_dragMoveDest.y)) {
					TheEditor->dragMoveSelectedWindows( &m_dragMoveOrigin, &m_dragMoveDest );
				}
				// release capture
				SetCapture( NULL );

				// go back to normal mode
				TheEditor->setMode( MODE_EDIT );

			}  // end if
			else if( m_dragSelecting )
			{

				// select the windows in the region
				TheEditor->selectWindowsInRegion( &m_selectRegion );

				// stop a drag selection if in progress
				m_dragSelecting = FALSE;

			}  // end else
			else if( m_resizingWindow )
			{
				GameWindow *window = TheEditor->getFirstSelected();
				DEBUG_ASSERTCRASH(window, ("No window selected for resize!"));

				if (window)
				{
					ICoord2D resultLoc, resultSize;
					ICoord2D dest = m_resizeDest;

					// adjust resize dest by the grid if it's on
					if( TheEditor->isGridSnapOn() )
						TheEditor->gridSnapLocation( &dest, &dest );

					// compute the location to resize it at
					TheEditor->computeResizeLocation( TheEditor->getMode(),
																						window,
																						&m_resizeOrigin, &dest,
																						&resultLoc, &resultSize );

					// move the window
					TheEditor->moveWindowTo( window, resultLoc.x, resultLoc.y );

					// resize the window
					window->winSetSize( resultSize.x, resultSize.y );
				}
							
				// go back to normal
				m_resizingWindow = FALSE;
				TheEditor->setMode( MODE_EDIT );

			}  // end resizing window

			break;

		}  // end left button up

		// ------------------------------------------------------------------------
		case WM_MBUTTONDOWN:
		{

			break;

		}  // end middle button down

		// ------------------------------------------------------------------------
		case WM_MBUTTONUP:
		{

			break;

		}  // end middle button up

		// ------------------------------------------------------------------------
		case WM_RBUTTONDOWN:
		{
			ICoord2D clickPos = mouse;
			GameWindow *window;

			// adjust the mouse pos if we're on a grid
			if( TheEditor->isGridSnapOn() )
				TheEditor->gridSnapLocation( &mouse, &clickPos );

			// get the window at the click pos
			window = TheEditor->getWindowAtPos( clickPos.x, clickPos.y );
					
			//
			// if there is a window here and it is not part of the selection
			// list, everything else is unselected
			//
			if( window )
			{

				if( TheEditor->isWindowSelected( window ) == FALSE )
					TheEditor->clearSelections();

				// select this window
				TheEditor->selectWindow( window );

			}  // end if
			else
			{

				// no window here, clear selections anyway
				TheEditor->clearSelections();

			}  // end else
												
			// open right click menu
			TheEditWindow->openPopupMenu( clickPos.x, clickPos.y );

			break;

		}  // end right button down

		// ------------------------------------------------------------------------
		case WM_RBUTTONUP:
		{

			break;

		}  // end right button up

		// ------------------------------------------------------------------------
		default:
		{

			break;

		}  // end default

	}  // end switch on widnows message

}  // end mouseEvent

// EditWindow::inCornerTolerance ==============================================
/** If the 'dest' point is within 'tolerance' distance to the 'source'
	* point this returns TRUE */
//=============================================================================
Bool EditWindow::inCornerTolerance( ICoord2D *dest, ICoord2D *source,
																		Int tolerance )
{
	IRegion2D region;

	// sanity
	if( dest == NULL || source == NULL )
		return FALSE;

	/// @todo we should write PointInRegion() stuff again like it was in Nox

	// build the region around the source point
	region.lo.x = source->x - tolerance;
	region.lo.y = source->y - tolerance;
	region.hi.x = source->x + tolerance;
	region.hi.y = source->y + tolerance;

	// check if in region
	if( dest->x >= region.lo.x &&
			dest->x <= region.hi.x &&
			dest->y >= region.lo.y &&
			dest->y <= region.hi.y )
		return TRUE;
		
	return FALSE;
		
}  // end inCornerTolerance

// EditWindow::inLineTolerance ================================================
/** If the 'dest' point is within the region defined around the
	* line with th specified tolerance return TRUE */
//=============================================================================
Bool EditWindow::inLineTolerance( ICoord2D *dest,
																	ICoord2D *lineStart, ICoord2D *lineEnd,
																	Int tolerance )
{
	IRegion2D region;

	// sanity
	if( dest == NULL || lineStart == NULL || lineEnd == NULL )	
		return FALSE;

	// setup region
	region.lo.x = lineStart->x - tolerance;
	region.lo.y = lineStart->y - tolerance;
	region.hi.x = lineEnd->x + tolerance;
	region.hi.y = lineEnd->y + tolerance;

	// check if in region
	if( dest->x >= region.lo.x &&
			dest->x <= region.hi.x &&
			dest->y >= region.lo.y &&
			dest->y <= region.hi.y )
		return TRUE;
		
	return FALSE;

}  // end inLineTolerance

// EditWindow::handleResizeAvailable ==========================================
/** Given the mouse position, if it is close enough to a corner of a
	* SINGLE selected window then we change the icon to the appropriate
	* resize icon.  If the mouse is not moved from this position then
	* the next left click will allow for a resize of the window */
//=============================================================================
void EditWindow::handleResizeAvailable( Int mouseX, Int mouseY )
{
	GameWindow *window;
	ICoord2D origin, size, mouse, point;
	const Int tol = 5;
		
	// if there is zero or more than 1 window selected we can't resize
	if( TheEditor->selectionCount() != 1 )
		return;

	// get mouse data in a nice coord 2d
	mouse.x = mouseX;
	mouse.y = mouseY;

	// get the selected window data
	window = TheEditor->getFirstSelected();
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );
		
	// check for around top left corner
	point = origin;
	if( inCornerTolerance( &mouse, &point, tol ) == TRUE )
	{

		TheEditor->setMode( MODE_RESIZE_TOP_LEFT );
		return;

	}  // end if

	// check for around bottom right corner
	point.x = origin.x + size.x;
	point.y = origin.y + size.y;
	if( inCornerTolerance( &mouse, &point, tol ) == TRUE )
	{

		TheEditor->setMode( MODE_RESIZE_BOTTOM_RIGHT );
		return;

	}  // end if

	// check for around top right corner
	point.x = origin.x + size.x;
	point.y = origin.y;
	if( inCornerTolerance( &mouse, &point, tol ) == TRUE )
	{
		
		TheEditor->setMode( MODE_RESIZE_TOP_RIGHT );
		return;

	}  // end if

	// check for around bottom left corner
	point.x = origin.x;
	point.y = origin.y + size.y;
	if( inCornerTolerance( &mouse, &point, tol ) == TRUE )
	{

		TheEditor->setMode( MODE_RESIZE_BOTTOM_LEFT );
		return;

	}  // end if

	ICoord2D lineStart, lineEnd;

	// check for along top edge
	lineStart = origin;
	lineEnd.x = origin.x + size.x;
	lineEnd.y = origin.y;
	if( inLineTolerance( &mouse, &lineStart, &lineEnd, tol ) == TRUE )
	{

		TheEditor->setMode( MODE_RESIZE_TOP );
		return;

	}  // end if

	// check for along bottom edge
	lineStart.x = origin.x;
	lineStart.y = origin.y + size.y;
	lineEnd.x = origin.x + size.x;
	lineEnd.y = origin.y + size.y;
	if( inLineTolerance( &mouse, &lineStart, &lineEnd, tol ) == TRUE )
	{

		TheEditor->setMode( MODE_RESIZE_BOTTOM );
		return;

	}  // end if

	// check for along left edge
	lineStart = origin;
	lineEnd.x = origin.x;
	lineEnd.y = origin.y + size.y;
	if( inLineTolerance( &mouse, &lineStart, &lineEnd, tol ) == TRUE )
	{

		TheEditor->setMode( MODE_RESIZE_LEFT );
		return;

	}  // end if

	// check for along right edge
	lineStart.x = origin.x + size.x;
	lineStart.y = origin.y;
	lineEnd.x = origin.x + size.x;
	lineEnd.y = origin.y + size.y;
	if( inLineTolerance( &mouse, &lineStart, &lineEnd, tol ) == TRUE )
	{

		TheEditor->setMode( MODE_RESIZE_RIGHT );
		return;

	}  // end if

	// we are not resizing anything at all, set us to normal mode
	TheEditor->setMode( MODE_EDIT );

}  // end handleResizeAvailable

// EditWindow::drawSeeThruOutlines ============================================
/** Draw an outline for a window that is see thru so we can still work
	* with it in the editor */
//=============================================================================
void EditWindow::drawSeeThruOutlines( GameWindow *windowList, Color c )
{

	// end recursion
	if( windowList == NULL )
		return;

	// draw outline for this window
	if( BitIsSet( windowList->winGetStatus(), WIN_STATUS_SEE_THRU ) )
	{
		ICoord2D pos;
		ICoord2D size;

		// get position and size
		windowList->winGetScreenPosition( &pos.x, &pos.y );
		windowList->winGetSize( &size.x, &size.y );

		// draw a box on the window
		drawOpenRect( pos.x, pos.y, size.x, size.y, 1, c );

	}  // end if

	// check window children
	GameWindow *child;
	for( child = windowList->winGetChild(); child; child = child->winGetNext() )
		drawSeeThruOutlines( child, c );

	// go to siblings
	drawSeeThruOutlines( windowList->winGetNext(), c );

}  // end drawSeeThruOutlines

// EditWindow::drawHiddenOutlines =============================================
/** Draw an outline for a window that is hidden so we can still work
	* with it in the editor */
//=============================================================================
void EditWindow::drawHiddenOutlines( GameWindow *windowList, Color c )
{

	// end recursion
	if( windowList == NULL )
		return;

	//
	// draw outline for this window, we are hidden if we have the hidden
	// status or any of our parents are hidden
	//
	Bool hidden = FALSE;
	GameWindow *parent= windowList->winGetParent();
	while( parent )
	{

		if( BitIsSet( parent->winGetStatus(), WIN_STATUS_HIDDEN ) )
			hidden = TRUE;
		parent = parent->winGetParent();

	}  // end while
	if( BitIsSet( windowList->winGetStatus(), WIN_STATUS_HIDDEN ) )
		hidden = TRUE;
	if( hidden )
	{
		ICoord2D pos;
		ICoord2D size;

		// get position and size
		windowList->winGetScreenPosition( &pos.x, &pos.y );
		windowList->winGetSize( &size.x, &size.y );

		// draw a box on the window
		drawOpenRect( pos.x, pos.y, size.x, size.y, 2, c );

	}  // end if

	// check window children
	GameWindow *child;
	for( child = windowList->winGetChild(); child; child = child->winGetNext() )
		drawHiddenOutlines( child, c );

	// go to siblings
	drawHiddenOutlines( windowList->winGetNext(), c );

}  // end drawHiddenOutlines

// EditWindow::drawUIback =====================================================
/** Draw any visual feedback to the user about selection boxes or windows
	* that are selected, draggin windows etc */
//=============================================================================
void EditWindow::drawUIFeedback( void )
{
	WindowSelectionEntry *select;
	Int color = m_pulse * 2;

	if( color > 255 )
		color = 255;

	// draw hidden window outlines if requested
	if( TheEditor->getShowHiddenOutlines() )
		drawHiddenOutlines( TheWindowManager->winGetWindowList(),
												GameMakeColor( color, 64, 64, 255 ) );

	// draw see-thru window outlines if requested
	if( TheEditor->getShowSeeThruOutlines() )
		drawSeeThruOutlines( TheWindowManager->winGetWindowList(),
												 GameMakeColor( 64, 64, color, 255 ) );

	// if the grid is visible draw it on top of everything
	if( TheEditor->isGridVisible() == TRUE )
		drawGrid();

	// draw drag selection box
	if( m_dragSelecting )
	{
		Int width, height;
		Real selectBoxWidth = 2.0f;
		Color selectBoxColor = GameMakeColor( 0, 255, 0, 255 );

		width = m_selectRegion.hi.x - m_selectRegion.lo.x;
		height = m_selectRegion.hi.y - m_selectRegion.lo.y;
		drawOpenRect( m_selectRegion.lo.x, m_selectRegion.lo.y,
		              width, height, selectBoxWidth, selectBoxColor );

	}  // end if

	// draw select lines on selected windows
	select = TheEditor->getSelectList();
	while( select )
	{
		ICoord2D origin, size;
		GameWindow *window;
		Real windowSelectWidth = 2.0f;
		Color windowSelectColor;
		
		windowSelectColor = GameMakeColor( 0, color, 0, 255 );

		// get window properties
		window = select->window;
		window->winGetScreenPosition( &origin.x, &origin.y );
		window->winGetSize( &size.x, &size.y );

		// draw a box on the window
		drawOpenRect( origin.x, origin.y, size.x, size.y, 
									windowSelectWidth, windowSelectColor );		

		// go to next selection
		select = select->next;

	}  // end while

	//
	// if we're drag moving, draw outlines of all the windows in the
	// select list offset by the start to end of the drag move
	//
	if( TheEditor->getMode() == MODE_DRAG_MOVE || TheEditor->getMode() == MODE_KEYBOARD_MOVE )
	{
		ICoord2D origin, size;
		ICoord2D moveLoc, safeLoc;
		GameWindow *window, *parent;
		Real outlineWidth = 1.0f;
		Color outlineColor;

		// determine outline using pulse counter
		outlineColor = GameMakeColor( m_pulse, m_pulse, m_pulse + 25, 255 );

		// traverse selection list
		select = TheEditor->getSelectList();
		while( select )
		{

			// get window data
			window = select->window;
			window->winGetScreenPosition( &origin.x, &origin.y );
			window->winGetSize( &size.x, &size.y );

			// figure out the destination of the window from this move
 			ICoord2D change;
 			change.x = (m_dragMoveDest.x - m_dragMoveOrigin.x);
 			change.y = (m_dragMoveDest.y - m_dragMoveOrigin.y);
 
 		// [SKB: Jun 02 2003 @ 2:7pm] :
 		// Don't move the object unless we have moved the mouse,
 		// this is to avoid the irritating movement of a window when just
 		// clicking on a window.
 			if (change.x || change.y)
 			{
 				moveLoc.x = origin.x + change.x;
 				moveLoc.y = origin.y + change.y;

				// snap move location to grid if on
 				if( (TheEditor->getMode() == MODE_DRAG_MOVE) && TheEditor->isGridSnapOn() )
					TheEditor->gridSnapLocation( &moveLoc, &moveLoc );


				// keep location legal
 				TheEditor->computeSafeLocation( window, moveLoc.x, moveLoc.y, 
																				&safeLoc.x, &safeLoc.y );

				// adjust location by parent location if present
				parent = window->winGetParent();
				if( parent )
				{
					ICoord2D parentOrigin;

					parent->winGetScreenPosition( &parentOrigin.x, &parentOrigin.y );
					safeLoc.x += parentOrigin.x;
					safeLoc.y += parentOrigin.y;

				}  // end if

				// draw outline of window at what would be the drag move destination
				drawOpenRect( safeLoc.x, safeLoc.y, size.x, size.y, 
											outlineWidth, outlineColor );
			}

			// go to next selection
			select = select->next;

		}  // end while

	}  // end if

	// if resizing a window draw that resize representation
	if( m_resizingWindow )
	{
		GameWindow *window = m_windowToResize;
		ICoord2D loc, size;
		Color outlineColor;
		Real outlineWidth = 1.0f;
		GameWindow *parent;
		ICoord2D dest = m_resizeDest;

		// adjust resize dest by the grid if it's on
		if( TheEditor->isGridSnapOn() )
			TheEditor->gridSnapLocation( &dest, &dest );

		//
		// given the location that we started dragging at and the destination
		// drag location along with the mode of resizing compute what the
		// new location and the new size of the selected window to resize
		//
		TheEditor->computeResizeLocation( TheEditor->getMode(),
																		  window,
																		  &m_resizeOrigin, &dest,
																		  &loc, &size );

		// adjust location by parent location if present
		parent = window->winGetParent();
		if( parent )
		{
			ICoord2D parentOrigin;

			parent->winGetScreenPosition( &parentOrigin.x, &parentOrigin.y );
			loc.x += parentOrigin.x;
			loc.y += parentOrigin.y;

		}  // end if

		// draw a box for the window at its new location
		outlineColor = GameMakeColor( m_pulse, m_pulse, m_pulse, 255 );
		drawOpenRect( loc.x, loc.y, size.x, size.y, outlineWidth, outlineColor );

	}  // end if

	// draw lines around any drag source and drag targets in the hierarchy view
	GameWindow *dragSource = TheHierarchyView->getDragWindow();
	Color dragColor = GameMakeColor( color, 0, color, 255 );
	if( dragSource )
	{
		ICoord2D origin, size;

		// get size and position
		dragSource->winGetScreenPosition( &origin.x, &origin.y );
		dragSource->winGetSize( &size.x, &size.y );

		// draw box
		drawOpenRect( origin.x, origin.y, size.x, size.y, 2, dragColor );

	}  // end if

	// drag target
	GameWindow *dragTarget = TheHierarchyView->getDragTarget();
	if( dragTarget )
	{
		ICoord2D origin, size;
		
		// get size and position
		dragTarget->winGetScreenPosition( &origin.x, &origin.y );
		dragTarget->winGetSize( &size.x, &size.y );

		// draw box
		drawOpenRect( origin.x, origin.y, size.x, size.y, 2, dragColor );

	}  // end if

}  // end drawUIFeedback

// EditWindow::drawGrid =======================================================
/** Draw the grid */
//=============================================================================
void EditWindow::drawGrid( void )
{
//	HDC hdc = GetDC( getWindowHandle() );
	Int res = TheEditor->getGridResolution();
	Int x, y;
	RGBColorInt *gridColor = TheEditor->getGridColor();
	Color color = GameMakeColor( gridColor->red, gridColor->green,	
															 gridColor->blue, gridColor->alpha );

	// set us to invert where we draw
//	SetROP2( hdc, R2_NOT );
//	SelectObject( hdc, (HPEN)GetStockObject( WHITE_PEN ) );

	for( y = 0; y < m_size.y; y += res )
	{

		TheDisplay->drawLine( 0, y, m_size.x, y, 1, color );
//		MoveToEx( hdc, 0, y, NULL );
//		LineTo( hdc, m_size.x, y );

	} 

	for( x = 0; x < m_size.x; x += res )
	{

		TheDisplay->drawLine( x, 0, x, m_size.y, 1, color );
//		MoveToEx( hdc, x, 0, NULL );
//		LineTo( hdc, x, m_size.y );

	} 

/*
	for( y = 0; y < m_size.y; y += res )
	{

		for( x = 0; x < m_size.x; x += res )
		{

			MoveToEx( hdc, x, y, NULL );
			LineTo( hdc, x + 1, y + 1 );

//				TheDisplay->drawLine( x, y, x + 1, y + 1, 1, 0xFFFFFFFF );

		}  // end for x

	}  // end for y
*/

	// release the dc
//	ReleaseDC( getWindowHandle(), hdc );

}  // end drawGrid

// EditWindow::draw ===========================================================
/** Draw the edit window */
//=============================================================================
void EditWindow::draw( void )
{
	static UnsignedInt syncTime = 0;

	// allow W3D to update its internals
	WW3D::Sync( syncTime );

	// for now, use constant time steps to avoid animations running independent of framerate
	syncTime += 50;

	// start render block
	WW3D::Begin_Render( true, true, Vector3( m_backgroundColor.red, 
																					 m_backgroundColor.green, 
																					 m_backgroundColor.blue ) );

	// draw the windows
	TheWindowManager->winRepaint();

	// draw selected drag box and any window selection boxes
	if( TheEditor->getMode() != MODE_TEST_RUN )
		drawUIFeedback();

	// render is all done!
	WW3D::End_Render();

}  // end draw

// EditWindow::setSize ========================================================
/** The edit window should now be logically consider this size */
//=============================================================================
void EditWindow::setSize( ICoord2D *size )
{

	// save the size
	m_size = *size;

	// the display should reflect the new size as well
	if( TheDisplay )
	{

		TheDisplay->setWidth( m_size.x );
		TheDisplay->setHeight( m_size.y );

	}  // end if

	// set the extents for our 2D renderer
	if( m_2DRender )
		m_2DRender->Set_Coordinate_Range( RectClass( 0,
																								 0,
																								 m_size.x,
																								 m_size.y ) );

}  // end setSize

// EditWindow::openPopupMenu ==================================================
/** Open the new control menu that comes up when the user right clicks
	* in the workspace to create new controls */
//=============================================================================
void EditWindow::openPopupMenu( Int x, Int y )
{
	HMENU menu, subMenu;
	POINT screen;
	Int selectCount = TheEditor->selectionCount();

	// get the menu with the new control items
	menu = LoadMenu( TheEditor->getInstance(), (LPCTSTR)POPUP_MENU );
	subMenu = GetSubMenu( menu, 0 );

	// enable/disable menu items
	if( selectCount == 0 )
	{

		EnableMenuItem( subMenu, POPUP_MENU_DELETE, MF_GRAYED );
		EnableMenuItem( subMenu, POPUP_MENU_PROPERTIES, MF_GRAYED );
		EnableMenuItem( subMenu, POPUP_MENU_BRING_TO_TOP, MF_GRAYED );

	}  // end if
	else
	{

		EnableMenuItem( subMenu, POPUP_MENU_DELETE, MF_ENABLED );
		EnableMenuItem( subMenu, POPUP_MENU_PROPERTIES, MF_ENABLED );
		EnableMenuItem( subMenu, POPUP_MENU_BRING_TO_TOP, MF_ENABLED );

	}  // end else
	
	//
	// open up right mouse track menu, note that we have to translate the
	// x,y of this mouse click which is local to this window into
	// screen coordinates
	//
	screen.x = x;
	screen.y = y;
	ClientToScreen( m_editWindowHWnd, &screen );
	TrackPopupMenuEx( subMenu, 0, screen.x, screen.y, m_editWindowHWnd, NULL );

	// save the location click for the creation of the popup menu
	m_popupMenuClickPos.x = x;
	m_popupMenuClickPos.y = y;

}  // end openPopupMenu

// EditWindow::drawLine =======================================================
/** draw a line on the display in pixel coordinates with the specified color */
//=============================================================================
void EditWindow::drawLine( Int startX, Int startY, 
													 Int endX, Int endY, 
													 Real lineWidth, UnsignedInt lineColor )
{

	m_2DRender->Reset();
	m_2DRender->Enable_Texturing( FALSE );
	m_2DRender->Add_Line( Vector2( startX, startY ), Vector2( endX, endY ), 
												lineWidth, lineColor );
	m_2DRender->Render();
	
}  // end drawLIne

// EditWindow::drawOpenRect ===================================================
/** draw a rect border on the display in pixel coordinates with the 
	* specified color */
//=============================================================================
void EditWindow::drawOpenRect( Int startX, Int startY, 
															 Int width, Int height,
															 Real lineWidth, UnsignedInt lineColor )
{

	m_2DRender->Reset();		
	m_2DRender->Enable_Texturing( FALSE );
	m_2DRender->Add_Outline( RectClass( startX, startY, 
																			startX + width, startY + height ), 
													 lineWidth, lineColor );

	// render it now!
	m_2DRender->Render();

}  // end drawOpenRect

// EditWindow::drawFillRect ===================================================
/** draw a filled rect on the display in pixel coords with the 
	* specified color */
//=============================================================================
void EditWindow::drawFillRect( Int startX, Int startY, 
															 Int width, Int height,
															 UnsignedInt color )
{

	m_2DRender->Reset();		
	m_2DRender->Enable_Texturing( FALSE );
	m_2DRender->Add_Rect( RectClass( startX, startY, 
																	 startX + width, startY + height ), 
												0, 0, color );

	// render it now!
	m_2DRender->Render();

}  // end drawFillRect

// EditWindow::drawImage ======================================================
/** draw an image fit within the screen coordinates */
//=============================================================================
void EditWindow::drawImage( const Image *image, 
														Int startX, Int startY, 
														Int endX, Int endY,
														Color color )
{

	// sanity
	if( image == NULL )
		return;

	const Region2D *uv = image->getUV();

	m_2DRender->Reset();
	m_2DRender->Enable_Texturing( TRUE );
	m_2DRender->Set_Texture( image->getFilename().str() );

	RectClass screen_rect(startX,startY,endX,endY);
	RectClass uv_rect(uv->lo.x,uv->lo.y,uv->hi.x,uv->hi.y);

	if (m_isClippedEnabled)
	{	//need to clip this quad to clip rectangle

		//
		//	Check for completely clipped
		//
		if (	endX <= m_clipRegion.lo.x ||
				endY <= m_clipRegion.lo.y)
		{
			return;	//nothing to render
		} else {

			//
			//	Clip the polygons to the specified area
			//
			RectClass clipped_rect;
			clipped_rect.Left		= __max (screen_rect.Left, m_clipRegion.lo.x);
			clipped_rect.Right	= __min (screen_rect.Right, m_clipRegion.hi.x);
			clipped_rect.Top		= __max (screen_rect.Top, m_clipRegion.lo.y);
			clipped_rect.Bottom	= __min (screen_rect.Bottom, m_clipRegion.hi.y);

			//
			//	Clip the texture to the specified area
			//
			RectClass clipped_uv_rect;
			float percent				= ((clipped_rect.Left - screen_rect.Left) / screen_rect.Width ());
			clipped_uv_rect.Left		= uv_rect.Left + (uv_rect.Width () * percent);

			percent						= ((clipped_rect.Right - screen_rect.Left) / screen_rect.Width ());
			clipped_uv_rect.Right	= uv_rect.Left + (uv_rect.Width () * percent);

			percent						= ((clipped_rect.Top - screen_rect.Top) / screen_rect.Height ());
			clipped_uv_rect.Top		= uv_rect.Top + (uv_rect.Height () * percent);

			percent						= ((clipped_rect.Bottom - screen_rect.Top) / screen_rect.Height ());
			clipped_uv_rect.Bottom	= uv_rect.Top + (uv_rect.Height () * percent);

			//
			//	Use the clipped rectangles to render
			//
			screen_rect = clipped_rect;
			uv_rect		= clipped_uv_rect;
		}
	}

	// if rotated 90 degrees clockwise we have to adjust the uv coords
	if( BitIsSet( image->getStatus(), IMAGE_STATUS_ROTATED_90_CLOCKWISE ) )
	{

		m_2DRender->Add_Tri( Vector2( screen_rect.Left, screen_rect.Top ), 
												 Vector2( screen_rect.Left, screen_rect.Bottom ),
												 Vector2( screen_rect.Right, screen_rect.Top ),
												 Vector2( uv_rect.Right, uv_rect.Top),
												 Vector2( uv_rect.Left, uv_rect.Top),
												 Vector2( uv_rect.Right, uv_rect.Bottom ),
												 color );

		m_2DRender->Add_Tri( Vector2( screen_rect.Right, screen_rect.Bottom ),
												 Vector2( screen_rect.Right, screen_rect.Top ),
												 Vector2( screen_rect.Left, screen_rect.Bottom ),
												 Vector2( uv_rect.Left, uv_rect.Bottom ),
												 Vector2( uv_rect.Right, uv_rect.Bottom ),
												 Vector2( uv_rect.Left, uv_rect.Top ),
												 color );

	}  // end if
	else
	{

		// just draw as normal
		m_2DRender->Add_Quad( screen_rect, uv_rect, color );

	}  // end else

	m_2DRender->Render();

}  // end drawImage

// EditWindow::getBackgroundColor =============================================
/** Get the background color for the edit window */
//=============================================================================
RGBColorReal EditWindow::getBackgroundColor( void )
{
	
	return m_backgroundColor;

}  // end getBackgroundColor

// EditWindow::setBackgroundColor =============================================
/** Set the background color for the edit window */
//=============================================================================
void EditWindow::setBackgroundColor( RGBColorReal color )
{

	m_backgroundColor = color;

}  // end setBackgroundColor

// EditWindow::notifyWindowDeleted ============================================
/** A window has been deleted from the editor and the editor is now
	* notifying the edit window about it in case anything must be
	* cleaned up */
//=============================================================================
void EditWindow::notifyWindowDeleted( GameWindow *window )
{

	// sanity
	if( window == NULL )
		return;

	// check to see if the resizing window was deleted
	if( m_windowToResize == window )
	{

		// get back to normal mode and clean up
		m_windowToResize = NULL;
		m_resizingWindow = FALSE;
		TheEditor->setMode( MODE_EDIT );

	}  // end if

	// null out picked window if needed
	if( m_pickedWindow == window )
		m_pickedWindow = NULL;

	//
	// go back to edit mode, this keeps us from staying in a resize mode
	// after deleting the window under the cursor
	//
	TheEditor->setMode( MODE_EDIT );

}  // end notifyWindowDeleted
