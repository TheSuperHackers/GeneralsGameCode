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

// FILE: CallbackEditor.cpp ///////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    GUIEdit
//
// File name:  CallbackEditor.cpp
//
// Created:    Colin Day, Sepember 2001
//
// Desc:			 A handy dandy little dialog to just edit the callbacks for
//						 user windows ... a super convient luxury at a bargain price!
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "Common/FunctionLexicon.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"
#include "GUIEdit.h"
#include "resource.h"
#include "Properties.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static const char *noNameWindowString = "Un-named Window";
static GameWindow *currentWindow = NULL;  ///< current window we're editing

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// SaveCallbacks ==============================================================
/** save the current callbacks for the selected window */
void SaveCallbacks( GameWindow *window, HWND dialog )
{

	// sanity
	if( window == NULL || dialog == NULL )
		return;

	// get edit data for window
	GameWindowEditData *editData = window->winGetEditData();
	DEBUG_ASSERTCRASH( editData, ("No edit data for window saving callbacks!\n") );

	// get the currently selected item from each of the combos and save
	Int index;
	char buffer[ 256 ];

	// system
	index = SendDlgItemMessage( dialog, COMBO_SYSTEM, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( dialog, COMBO_SYSTEM, CB_GETLBTEXT, index, (LPARAM)buffer );
	editData->systemCallbackString = buffer;

	// input
	index = SendDlgItemMessage( dialog, COMBO_INPUT, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( dialog, COMBO_INPUT, CB_GETLBTEXT, index, (LPARAM)buffer );
	editData->inputCallbackString = buffer;

	// tooltip
	index = SendDlgItemMessage( dialog, COMBO_TOOLTIP, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( dialog, COMBO_TOOLTIP, CB_GETLBTEXT, index, (LPARAM)buffer );
	editData->tooltipCallbackString = buffer;

	// draw
	index = SendDlgItemMessage( dialog, COMBO_DRAW, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( dialog, COMBO_DRAW, CB_GETLBTEXT, index, (LPARAM)buffer );
	editData->drawCallbackString = buffer;

	// if there was a window we have a change 
	if( window )
		TheEditor->setUnsaved( TRUE );

}  // end SaveCallbacks

// setCurrentWindow ===========================================================
/** Set the window passed in as the active window for editing */
//=============================================================================
static void setCurrentWindow( GameWindow *window, HWND dialog )
{
	GameWindowEditData *editData = NULL;

	// get edit data from window if present
	if( window )
		editData = window->winGetEditData();

	// save window
	currentWindow = window;

	// sanity
	if( dialog == NULL )
		return;

	// enable the callback combo boxes
	EnableWindow( GetDlgItem( dialog, COMBO_SYSTEM ), TRUE );
	EnableWindow( GetDlgItem( dialog, COMBO_INPUT ), TRUE );
	EnableWindow( GetDlgItem( dialog, COMBO_TOOLTIP ), TRUE );
	EnableWindow( GetDlgItem( dialog, COMBO_DRAW ), TRUE );

	//
	// select the assigned callbacks, if no callback is assigned
	// in a slot then we select the "none string" for the combo box
	//
	AsciiString name;

	// system 
	if( editData )
		name = editData->systemCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendDlgItemMessage( dialog, COMBO_SYSTEM, 
											CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// input
	name = NULL;
	if( editData )
		name = editData->inputCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendDlgItemMessage( dialog, COMBO_INPUT, 
											CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// tooltip
	name = NULL;
	if( editData )
		name = editData->tooltipCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendDlgItemMessage( dialog, COMBO_TOOLTIP, 
											CB_SELECTSTRING, -1, (LPARAM)name.str() );

	// draw
	name = NULL;
	if( editData )
		name = editData->drawCallbackString;
	if( name.isEmpty() )
		name = GUIEDIT_NONE_STRING;
	SendDlgItemMessage( dialog, COMBO_DRAW, 
											CB_SELECTSTRING, -1, (LPARAM)name.str() );

	//
	// set the name of the window in the static control above
	// the callback editor boxes
	//
	name = GUIEDIT_NONE_STRING;
	if( window )
	{
		WinInstanceData *instData = window->winGetInstanceData();

		if( !instData->m_decoratedNameString.isEmpty() )
			name = instData->m_decoratedNameString;
		else
			name = noNameWindowString;

	}  // end if
	SetWindowText( GetDlgItem( dialog, STATIC_WINDOW ), name.str() );

}  // end setCurrentWindow

// loadUserWindows ============================================================
/** Given the window list passed in, load the list box passed with the
	* names of USER windows found in the hierarchy. */
//=============================================================================
static void loadUserWindows( HWND listbox, GameWindow *root )
{

	// end recursion
	if( root == NULL )	
		return;

	// is this a candidate
	if( TheEditor->windowIsGadget( root ) == FALSE )
	{
		WinInstanceData *instData = root->winGetInstanceData();
		Int index;

		AsciiString name;
		//
		// add name to the listbox, if there is no name we can only put
		// an unnamed label in there
		//
		if( !instData->m_decoratedNameString.isEmpty() )
			name = instData->m_decoratedNameString;
		else
			name = noNameWindowString;
		index = SendMessage( listbox, LB_ADDSTRING, 0, (LPARAM)name.str() );

		// add data pointer to the window at the index just added
		SendMessage( listbox, LB_SETITEMDATA, index, (LPARAM)root );

		// check the children
		loadUserWindows( listbox, root->winGetChild() );

	}  // end if

	// check the rest of the list
	loadUserWindows( listbox, root->winGetNext() );

}  // end loadUserWindows

//-------------------------------------------------------------------------------------------------
/** save the layout callbacks */
//-------------------------------------------------------------------------------------------------
static void saveLayoutCallbacks( HWND dialog )
{
	char buffer[ MAX_LAYOUT_FUNC_LEN ];
	Int sel;

	// layout init
	sel = SendDlgItemMessage( dialog, COMBO_INIT, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( dialog, COMBO_INIT, CB_GETLBTEXT, sel, (LPARAM)buffer );
	TheEditor->setLayoutInit( AsciiString(buffer) );

	// layout update
	sel = SendDlgItemMessage( dialog, COMBO_UPDATE, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( dialog, COMBO_UPDATE, CB_GETLBTEXT, sel, (LPARAM)buffer );
	TheEditor->setLayoutUpdate( AsciiString(buffer) );

	// layout shutdown
	sel = SendDlgItemMessage( dialog, COMBO_SHUTDOWN, CB_GETCURSEL, 0, 0 );
	SendDlgItemMessage( dialog, COMBO_SHUTDOWN, CB_GETLBTEXT, sel, (LPARAM)buffer );
	TheEditor->setLayoutShutdown( AsciiString(buffer) );

}  // end saveLayoutCallbacks

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// CallbackEditorDialogProc ===================================================
/** Dialog procedure for grid settings dialog */
//=============================================================================
BOOL CALLBACK CallbackEditorDialogProc( HWND hWndDialog, UINT message, 
																				WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_INITDIALOG:
		{

			// load the combos with the callbacks
			InitCallbackCombos( hWndDialog, NULL );

			// select the none string at the top index in each combo
			SendDlgItemMessage( hWndDialog, COMBO_SYSTEM, CB_SETCURSEL, 0, 0 );
			SendDlgItemMessage( hWndDialog, COMBO_INPUT, CB_SETCURSEL, 0, 0 );
			SendDlgItemMessage( hWndDialog, COMBO_TOOLTIP, CB_SETCURSEL, 0, 0 );
			SendDlgItemMessage( hWndDialog, COMBO_DRAW, CB_SETCURSEL, 0, 0 );

			// load the listbox with all the USER windows in the edit window
			loadUserWindows( GetDlgItem( hWndDialog, LIST_WINDOWS ), 
											 TheWindowManager->winGetWindowList() );

			// no current window
			setCurrentWindow( NULL, hWndDialog );

			return TRUE;

		}  // end init dialog

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {
			Int notifyCode = HIWORD( wParam );  // notification code
//			Int controlID = LOWORD( wParam );  // control ID
			HWND hWndControl = (HWND)lParam;  // control window handle

      switch( LOWORD( wParam ) )
      {

				// --------------------------------------------------------------------
				case LIST_WINDOWS:
				{
		
					switch( notifyCode )
					{

						// ----------------------------------------------------------------
						case LBN_SELCHANGE:
						{
							Int selected;
							GameWindow *win;

							// get the current selection of the window list
							selected = SendMessage( hWndControl, LB_GETCURSEL, 0, 0 );

							// get the window of the selected listbox item
							win = (GameWindow *)SendMessage( hWndControl, LB_GETITEMDATA,		
																							 selected, 0 );
							

							// sanity
							DEBUG_ASSERTCRASH( win, ("NULL window set in listbox item data") );

							// save the callbacks for the curent window selected
							SaveCallbacks( currentWindow, hWndDialog );

							// set the current window to the new selection
							setCurrentWindow( win, hWndDialog );
							
							break;

						}  // end case selection change

					}  // end switch

					break;

				}  // end window listbox

				// --------------------------------------------------------------------
        case IDOK:
				{

					// save callbacks, set current window to empty and end dialog
					SaveCallbacks( currentWindow, hWndDialog );
					setCurrentWindow( NULL, hWndDialog );

					// save the layout callbacks
					saveLayoutCallbacks( hWndDialog );

					// end dialog
					EndDialog( hWndDialog, TRUE );

          break;

				}  // end ok

      }  // end switch( LOWORD( wParam ) )

      return 0;

    } // end of WM_COMMAND

		// ------------------------------------------------------------------------
		default:
			return 0;

  }  // end of switch

}  // end CallbackEditorDialogProc


