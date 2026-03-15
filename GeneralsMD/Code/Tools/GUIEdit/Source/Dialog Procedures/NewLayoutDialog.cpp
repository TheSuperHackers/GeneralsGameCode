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

// FILE: NewLayoutDialog.cpp //////////////////////////////////////////////////
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
// File name:  NewLayoutDialog.cpp
//
// Created:    Colin Day, July 2001
//
// Desc:       New layout dialog procedure
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "resource.h"
#include "EditWindow.h"
#include "GUIEdit.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// initNewLayoutDialog ========================================================
/** The new layout dialog is being shown, initialize anything we need to */
//=============================================================================
static void initNewLayoutDialog( HWND hWndDialog )
{

	// set default keyboard focus
	SetFocus( GetDlgItem( hWndDialog, IDOK ) );

}

// NewLayoutDialogProc ========================================================
/** Dialog procedure for the new layout dialog when starting an entire
	* new layout in the editor */
//=============================================================================
LRESULT CALLBACK NewLayoutDialogProc( HWND hWndDialog, UINT message,
																			WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_INITDIALOG:
		{

			// initialize the values for the the dialog
			initNewLayoutDialog( hWndDialog );
			return FALSE;

		}

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {

      switch( LOWORD( wParam ) )
      {

				// --------------------------------------------------------------------
        case IDOK:
				{

					// reset the editor
					TheEditor->newLayout();

					// end this dialog
					EndDialog( hWndDialog, TRUE );

          break;

				}

				// --------------------------------------------------------------------
        case IDCANCEL:
				{

					EndDialog( hWndDialog, FALSE );
          break;

				}

      }

      return 0;

    }

		// ------------------------------------------------------------------------
    case WM_CLOSE:
		{

			EndDialog( hWndDialog, FALSE );
      return 0;

		}

		// ------------------------------------------------------------------------
		default:
			return 0;

  }

}

