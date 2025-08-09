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

// FILE: GridSettings.cpp /////////////////////////////////////////////////////
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
// File name:  GridSettings.cpp
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

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static RGBColorInt gridColor = { 0 };

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// initGridSettings ===========================================================
/** Initialize the dialog values */
//=============================================================================
static void initGridSettings( HWND hWndDialog )
{

	// set resolution
	SetDlgItemInt( hWndDialog, EDIT_RESOLUTION,
								 TheEditor->getGridResolution(), FALSE );

	// check box for on/off
	if( TheEditor->isGridVisible() == TRUE )
		CheckDlgButton( hWndDialog, CHECK_VISIBLE, BST_CHECKED );

	// check box for grid snap on/off
	if( TheEditor->isGridSnapOn() == TRUE )
		CheckDlgButton( hWndDialog, CHECK_SNAP_TO_GRID, BST_CHECKED );

	// style
	CheckDlgButton( hWndDialog, RADIO_LINES, BST_CHECKED );

	// color
	RGBColorInt *color = TheEditor->getGridColor();
	gridColor = *color;

}  // end initGridSettings

// GridSettingsDialogProc =====================================================
/** Dialog procedure for grid settings dialog */
//=============================================================================
BOOL CALLBACK GridSettingsDialogProc( HWND hWndDialog, UINT message,
																			WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_INITDIALOG:
		{

			// initialize the values for the the dialog
			initGridSettings( hWndDialog );
			return TRUE;

		}  // end init dialog

		// ------------------------------------------------------------------------
		case WM_DRAWITEM:
		{
      UINT controlID = (UINT)wParam;  // control identifier
      LPDRAWITEMSTRUCT drawItem = (LPDRAWITEMSTRUCT)lParam; // item drawing
			RGBColorInt *color = &gridColor;

			// we only care about color button controls
			if( color )
			{
				HBRUSH hBrushNew, hBrushOld;
				RECT rect;
				HWND hWndControl = GetDlgItem( hWndDialog, controlID );

				// if this control is disabled just let windows handle drawing
				if( IsWindowEnabled( hWndControl ) == FALSE )
					return FALSE;

				// Get the area we have to draw in
				GetClientRect( hWndControl, &rect );

        // create a new brush and select it into DC
        hBrushNew = CreateSolidBrush (RGB ((BYTE)color->red,
                                           (BYTE)color->green,
                                           (BYTE)color->blue));
        hBrushOld = (HBRUSH)SelectObject( drawItem->hDC, hBrushNew );

        // draw the rectangle
        Rectangle( drawItem->hDC, rect.left, rect.top, rect.right, rect.bottom );

        // put the old brush back and delete the new one
        SelectObject( drawItem->hDC, hBrushOld );
        DeleteObject( hBrushNew );

        // validate this new area
        ValidateRect( hWndControl, NULL );

				// we have taken care of it
				return TRUE;

			}  // end if

			return FALSE;

		}  // end draw item

		// ------------------------------------------------------------------------
    case WM_COMMAND:
    {
//			Int notifyCode = HIWORD( wParam );  // notification code
//			Int controlID = LOWORD( wParam );  // control ID
			HWND hWndControl = (HWND)lParam;  // control window handle

      switch( LOWORD( wParam ) )
      {

				// --------------------------------------------------------------------
				case BUTTON_COLOR:
				{
					RGBColorInt *currColor = &gridColor;

					// bring up color selector for this color control at the mouse
					if( currColor )
					{
						RGBColorInt *newColor;
						POINT mouse;

						GetCursorPos( &mouse );
						newColor = SelectColor( currColor->red, currColor->green,
																		currColor->blue, currColor->alpha,
																		mouse.x, mouse.y );

						if( newColor )
						{

							gridColor = *newColor;
							InvalidateRect( hWndControl, NULL, TRUE );

						}  // end if

					}  // end if

					break;

				}  // end color buttons

				// --------------------------------------------------------------------
        case IDOK:
				{
					Int value;

					// get the pixels between marks
					value = GetDlgItemInt( hWndDialog, EDIT_RESOLUTION, NULL, FALSE );
					TheEditor->setGridResolution( value );

					// get grid on/off flag
					value = IsDlgButtonChecked( hWndDialog, CHECK_VISIBLE );
					TheEditor->setGridVisible( value );

					// get snap on/off flag
					value = IsDlgButtonChecked( hWndDialog, CHECK_SNAP_TO_GRID );
					TheEditor->setGridSnap( value );

					// grid color
					TheEditor->setGridColor( &gridColor );

					// end this dialog
					EndDialog( hWndDialog, TRUE );

          break;

				}  // end ok

				// --------------------------------------------------------------------
        case IDCANCEL:
				{

					EndDialog( hWndDialog, FALSE );
          break;

				}  // end cancel

      }  // end switch( LOWORD( wParam ) )

      return 0;

    } // end of WM_COMMAND

		// ------------------------------------------------------------------------
    case WM_CLOSE:
		{

			EndDialog( hWndDialog, FALSE );
      return 0;

		}  // end close

		// ------------------------------------------------------------------------
		default:
			return 0;

  }  // end of switch

}  // end GridSettingsDialogProc

