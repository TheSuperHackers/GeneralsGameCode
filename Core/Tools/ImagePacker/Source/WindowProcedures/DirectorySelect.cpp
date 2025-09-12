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

// FILE: DirectorySelect.cpp //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    ImagePacker
//
// File name:  DirectorySelect.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Directory selection dialog
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "ImagePacker.h"
#include "Resource.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static char startDir[ _MAX_PATH ];


// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// selectDrive ================================================================
/** Based on the drive of the current working directory, select the
	* drive item in the COMBO_DRIVE combo box matching the current
	* drive letter */
//=============================================================================
static void selectDrive( HWND dialog )
{
	char currDir[ _MAX_PATH ];
	char drive;

	// get current directory
	GetCurrentDirectory( _MAX_PATH, currDir );

	// drive letter is the first character
	drive = currDir[ 0 ];

	// construct string to match in the combo
	char string[ 32 ];
	sprintf( string, "[-%c-]", drive );

	// select the string in the combo
	Int index;
	index = SendDlgItemMessage( dialog, COMBO_DRIVE,
															CB_FINDSTRINGEXACT, -1, (LPARAM)string );
	if( index != CB_ERR )
		SendDlgItemMessage( dialog, COMBO_DRIVE, CB_SETCURSEL, index, 0 );

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// DirectorySelectProc ========================================================
/** Directory selection dialog */
//=============================================================================
BOOL CALLBACK DirectorySelectProc( HWND hWndDialog, UINT message,
																	 WPARAM wParam, LPARAM lParam )
{
	static char buffer[ _MAX_PATH ];

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_INITDIALOG:
		{

			// save our current directory as we start up
			GetCurrentDirectory( _MAX_PATH, startDir );

			// set the extents for the horizontal scroll bar in the listbox
			SendDlgItemMessage( hWndDialog, LIST_DIR,
													LB_SETHORIZONTALEXTENT, 1280, 0 );

			// load the listbox with directory items
			SendDlgItemMessage( hWndDialog, LIST_DIR,
													LB_DIR,
													DDL_DIRECTORY | DDL_EXCLUSIVE,
													(LPARAM)"*.*" );

			// set the current directory in the top label
			GetCurrentDirectory( _MAX_PATH, buffer );
			if( buffer[ strlen( buffer ) - 1 ] != '\\' )
				strcat( buffer, "\\" );
			SetDlgItemText( hWndDialog, STATIC_CURRENT_DIR, buffer );

			// load the drive box
			SendDlgItemMessage( hWndDialog, COMBO_DRIVE,
													CB_DIR,
													DDL_DRIVES | DDL_EXCLUSIVE,
													(LPARAM)"*.*" );

			// select the right drive in the drive combo box
			selectDrive( hWndDialog );

			return TRUE;

		}

		// ------------------------------------------------------------------------
		case WM_COMMAND:
		{
			Int notifyCode = HIWORD( wParam );
			Int controlID = LOWORD( wParam );
//			HWND hWndControl = (HWND)lParam;

			switch( controlID )
			{

				// --------------------------------------------------------------------
				case BUTTON_ADD:
				{
					Int count;
					char text[ _MAX_PATH ];
					char toAdd[ _MAX_PATH ];

					// get number of items in listbox
					count = SendDlgItemMessage( hWndDialog, LIST_DIR, LB_GETCOUNT, 0, 0 );

					// for each selected item add that directory
					for( Int i = 0; i < count; i++ )
					{

						// is this item selected
						if( SendDlgItemMessage( hWndDialog, LIST_DIR, LB_GETSEL, i, 0 ) > 0 )
						{

							// get the text
							SendDlgItemMessage( hWndDialog, LIST_DIR, LB_GETTEXT,
																	i, (LPARAM)text );

							// remove the brackets on the text
							Int j, len = strlen( text );
							for( j = 0; j < len - 1; j++ )
								text[ j ] = text[ j + 1 ];
							text[ len - 2 ] = '\0';

							// ignore the ".." directory
							if( strcmp( text, ".." ) == 0 )
								continue;

							//
							// construct directory to add, make sure we have a '\' on the
							// end cause we assume that in the image file name code
							//
							sprintf( toAdd, "%s%s\\", buffer, text );

							// do not add directory if it's already in the listbox
							if( SendDlgItemMessage( TheImagePacker->getWindowHandle(),
																			LIST_FOLDERS,
																			LB_FINDSTRINGEXACT, -1, (LPARAM)toAdd ) != LB_ERR )
							{
								char message[ _MAX_PATH + 32 ];

								sprintf( message, "Ignoring folder '%s', already in list.", toAdd );
								MessageBox( NULL, message, "Folder Already In List",
														MB_OK | MB_ICONINFORMATION );
								continue;

							}

							// add path to the listbox
							SendDlgItemMessage( TheImagePacker->getWindowHandle(),
																	LIST_FOLDERS,
																	LB_INSERTSTRING, -1, (LPARAM)toAdd );

						}

					}

					SetCurrentDirectory( startDir );
					EndDialog( hWndDialog, TRUE );
					break;

				}

				// --------------------------------------------------------------------
				case BUTTON_CANCEL:
				{

					SetCurrentDirectory( startDir );
					EndDialog( hWndDialog, FALSE );
					break;

				}

				// --------------------------------------------------------------------
				case COMBO_DRIVE:
				{

					if( notifyCode == CBN_SELCHANGE )
					{
						Int selected;

						// get selected index
						selected = SendDlgItemMessage( hWndDialog, COMBO_DRIVE,
																					 CB_GETCURSEL, 0, 0 );
						if( selected != CB_ERR )
						{
							char string[ 32 ];

							SendDlgItemMessage( hWndDialog, COMBO_DRIVE,
																	CB_GETLBTEXT, selected, (LPARAM)string );

							//
							// construct a drive letter and colon from the combo
							// box strign [-x-]
							//
							string[ 0 ] = string[ 2 ];  // the drive letter in the form of "[-x-]"
							string[ 1 ] = ':';
							string[ 2 ] = '\0';

							// switch to that drive
							SetCurrentDirectory( string );

							// construct new direcotry name and update status text
							GetCurrentDirectory( _MAX_PATH, buffer );
							if( buffer[ strlen( buffer ) - 1 ] != '\\' )
								strcat( buffer, "\\" );
							SetDlgItemText( hWndDialog, STATIC_CURRENT_DIR, buffer );
							EnableWindow( GetDlgItem( hWndDialog, BUTTON_ADD ), FALSE );

							// reset the content of the listbox and reload
							SendDlgItemMessage( hWndDialog, LIST_DIR,
																	LB_RESETCONTENT, 0, 0 );
							SendDlgItemMessage( hWndDialog, LIST_DIR,
																	LB_DIR,
																	DDL_DIRECTORY | DDL_EXCLUSIVE,
																	(LPARAM)"*.*" );


						}

					}

					break;

				}

				// --------------------------------------------------------------------
				case LIST_DIR:
				{

					if( notifyCode == LBN_SELCHANGE )
					{
						Int selCount;
						Bool enable;

						// get selected count
						selCount = SendDlgItemMessage( hWndDialog, LIST_DIR,
																					 LB_GETSELCOUNT, 0, 0 );

						// if we have selected items, enable the add button
						if( selCount > 0 )
						{

							//
							// if the selected item is only the ".." directory we won't
							// enable it, we know that _IF_ ".." is present it is at the
							// top zero index position
							//
							enable = TRUE;
							Int index = 0;
							if( selCount == 1 &&
									SendDlgItemMessage( hWndDialog, LIST_DIR,
																			LB_GETSEL, index, 0 ) > 0 )
							{
								char text[ _MAX_PATH ];

								SendDlgItemMessage( hWndDialog, LIST_DIR, LB_GETTEXT,
																		index, (LPARAM)text );
								if( strcmp( text, "[..]" ) == 0 )
									enable = FALSE;

							}

						}
						else
							enable = FALSE;

						// do the enable
						EnableWindow( GetDlgItem( hWndDialog, BUTTON_ADD ), enable );

					}
					if( notifyCode == LBN_DBLCLK )
					{
						Int selected;
						char text[ _MAX_PATH ];

						// get currently selected item
						selected = SendDlgItemMessage( hWndDialog, LIST_DIR,
																					 LB_GETCURSEL, 0, 0 );

						// get text of selected item
						SendDlgItemMessage( hWndDialog, LIST_DIR, LB_GETTEXT,
																selected, (LPARAM)text );

						// strip the backets off the directory listing
						Int len = strlen( text );
						for( Int i = 0; i < len - 1; i++ )
							text[ i ] = text[ i + 1 ];
						text[ len - 2 ] = '\0';

						// go into that directory
						SetCurrentDirectory( text );

						// construct new direcotry name and update status text
						GetCurrentDirectory( _MAX_PATH, buffer );
						if( buffer[ strlen( buffer ) - 1 ] != '\\' )
							strcat( buffer, "\\" );
						SetDlgItemText( hWndDialog, STATIC_CURRENT_DIR, buffer );
						EnableWindow( GetDlgItem( hWndDialog, BUTTON_ADD ), FALSE );

						// reset the content of the listbox and reload
						SendDlgItemMessage( hWndDialog, LIST_DIR,
																LB_RESETCONTENT, 0, 0 );
						SendDlgItemMessage( hWndDialog, LIST_DIR,
																LB_DIR,
																DDL_DIRECTORY | DDL_EXCLUSIVE,
																(LPARAM)"*.*" );

					}

					break;

				}

			}

			break;

		}

	}

	return 0;

}



