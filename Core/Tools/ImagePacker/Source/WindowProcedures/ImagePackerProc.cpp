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

// FILE: ImagePackerProc.cpp //////////////////////////////////////////////////
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
// File name:  ImagePackerProc.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Window procedure for the image packer app
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>
#include <io.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "ImagePacker.h"
#include "Resource.h"
#include "WindowProc.h"
#include "WinMain.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ImagePackerProc ============================================================
/** Dialog procedure for the program flow */
//=============================================================================
BOOL CALLBACK ImagePackerProc( HWND hWndDialog, UINT message,
															 WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_INITDIALOG:
		{

			// we must have our program interface to continue
			if( TheImagePacker == NULL )
			{

				MessageBox( NULL, "Internal Error, 'TheImagePacker' not initialized",
										"Internal Error", MB_OK );
				EndDialog( hWndDialog, FALSE );

			}  // end if

			// save our window handlw
			TheImagePacker->setWindowHandle( hWndDialog );

			// get size of the screen
			Int x, y;
			x = GetSystemMetrics( SM_CXFULLSCREEN );
			y = GetSystemMetrics( SM_CYFULLSCREEN );

			// get size of our dialog
			RECT rect;
			ICoord2D size;
			GetWindowRect( hWndDialog, &rect );
			size.x = rect.right - rect.left;
			size.y = rect.bottom - rect.top;

			// center dialog on screen
			MoveWindow( hWndDialog,
									(x / 2) - (size.x / 2),
									(y / 2) - (size.y / 2),
									size.x,
									size.y,
									TRUE );

			// setup the status message
			TheImagePacker->statusMessage( "Select options and click 'Start'." );

			// set gutter size
			SetDlgItemInt( hWndDialog, EDIT_GUTTER, TheImagePacker->getGutter(), FALSE );

			// set alpha check
			if( TheImagePacker->getOutputAlpha() )
				CheckDlgButton( hWndDialog, CHECK_ALPHA, BST_CHECKED );

			// set the check for preview using the actual texture
			if( TheImagePacker->getUseTexturePreview() )
				CheckDlgButton( hWndDialog, CHECK_BITMAP_PREVIEW, BST_CHECKED );

			// set the check for creating INI definition
			if( TheImagePacker->createINIFile() )
				CheckDlgButton( hWndDialog, CHECK_INI, BST_CHECKED );

			// check the resursive checkbox by default
			CheckDlgButton( hWndDialog, CHECK_USE_SUB_FOLDERS, BST_CHECKED );

			// check the default image size radio button
			CheckDlgButton( hWndDialog, RADIO_512X512, BST_CHECKED );

			// fill out a default target filename
			SetDlgItemText( hWndDialog, EDIT_FILENAME, "NewImage" );

			// limit the size if the filename edit box
			SendDlgItemMessage( hWndDialog, EDIT_FILENAME,
													EM_LIMITTEXT, MAX_OUTPUT_FILE_LEN, 0 );

			// set options for compressed textures
			if( TheImagePacker->getCompressTextures() )
				CheckDlgButton( hWndDialog, CHECK_COMPRESS, BST_CHECKED );

			// set option checks for gap method
			UnsignedInt gapOptions = TheImagePacker->getGapMethod();
			if( BitIsSet( gapOptions, ImagePacker::GAP_METHOD_EXTEND_RGB ) )
				CheckDlgButton( hWndDialog, CHECK_GAP_EXTEND_RGB, BST_CHECKED );
			if( BitIsSet( gapOptions, ImagePacker::GAP_METHOD_GUTTER ) )
				CheckDlgButton( hWndDialog, CHECK_GAP_GUTTER, BST_CHECKED );

			return TRUE;

		}  // end init dialog

		// ------------------------------------------------------------------------
		case WM_COMMAND:
		{
			Int notifyCode = HIWORD( wParam );
			Int controlID = LOWORD( wParam );
//			HWND hWndControl = (HWND)lParam;

			switch( controlID )
			{

				// --------------------------------------------------------------------
				case BUTTON_PREVIOUS:
				{
					Int page = TheImagePacker->getTargetPreviewPage();

					// target preview page back one
					if( page > 1 )
					{

						page--;
						TheImagePacker->setTargetPreviewPage( page );
						UpdatePreviewWindow();

					}  // end if

					break;

				}  // end previous

				// --------------------------------------------------------------------
				case BUTTON_NEXT:
				{
					UnsignedInt page = TheImagePacker->getTargetPreviewPage();

					if( page < TheImagePacker->getPageCount() )
					{

						page++;
						TheImagePacker->setTargetPreviewPage( page );
						UpdatePreviewWindow();

					}  // end if

					break;

				}  // end next

				// --------------------------------------------------------------------
				case CHECK_BITMAP_PREVIEW:
				{
					Bool useBitmap;

					if( IsDlgButtonChecked( hWndDialog, controlID ) == BST_CHECKED )
						useBitmap = TRUE;
					else
						useBitmap = FALSE;

					TheImagePacker->setUseTexturePreview( useBitmap );
					UpdatePreviewWindow();

					break;

				}  // end preview using image

				// --------------------------------------------------------------------
				case CHECK_GAP_GUTTER:
				{
					Bool enable = FALSE;

					if( IsDlgButtonChecked( hWndDialog, controlID ) == BST_CHECKED )
						enable = TRUE;

					EnableWindow( GetDlgItem( hWndDialog, STATIC_GAP_INFO ), enable );
					EnableWindow( GetDlgItem( hWndDialog, EDIT_GUTTER ), enable );

					break;

				}  // end transparent gutter

				// --------------------------------------------------------------------
				case BUTTON_PREVIEW:
				{
					HWND preview = TheImagePacker->getPreviewWindow();

					if( preview )
					{

						// delete test display window
						DestroyWindow( preview );
						TheImagePacker->setPreviewWindow( NULL );
						SetDlgItemText( hWndDialog, BUTTON_PREVIEW, "Open Preview" );

					}  // end if
					else
					{
						HWND preview = MakePreviewDisplay();

						if( preview )
						{

							TheImagePacker->setPreviewWindow( preview );
							UpdatePreviewWindow();
							SetDlgItemText( hWndDialog, BUTTON_PREVIEW, "Close Preview" );

						}  // end if

					}  // end else

					break;

				}  // end test

				// --------------------------------------------------------------------
				case BUTTON_ADD_FOLDER:
				{

					// bring up the add directory dialog
					DialogBox( ApplicationHInstance,
										 (LPCTSTR)DIRECTORY_SELECT_DIALOG,
										 TheImagePacker->getWindowHandle(),
										 (DLGPROC)DirectorySelectProc );
					break;

				}  // end add folder

				// --------------------------------------------------------------------
				case BUTTON_REMOVE_FOLDER:
				{
					HWND folderList;

					// get the directory listbox
					folderList = GetDlgItem( hWndDialog, LIST_FOLDERS );
					if( folderList == NULL )
						break;;

					// get the selected item in the folder listbox
					Int selCount;
					selCount = SendMessage( folderList, LB_GETSELCOUNT, 0, 0 );
					if( selCount == 0 )
					{

						MessageBox( NULL, "You must first select a folder to remove it",
												"Select Folder First", MB_OK | MB_ICONINFORMATION );
						break;

					}  // end if

					//
					// start at the end of the listbox, delete any items that
					// are selected
					//
					Int itemCount = SendMessage( folderList, LB_GETCOUNT, 0, 0 );
					for( Int i = itemCount - 1; i >= 0; i-- )
						if( SendMessage( folderList, LB_GETSEL, i, 0 )  > 0 )
							SendMessage( folderList, LB_DELETESTRING, i, 0 );

					break;

				}  // end remove folder

				// --------------------------------------------------------------------
				case RADIO_128X128:
				case RADIO_256X256:
				case RADIO_512X512:
				case RADIO_TARGET_OTHER:
				{
					Bool enable;

					//
					// if this is the other button, enable the first edit box for
					// user size, if not, disable that section of the UI
					//
					enable =  IsDlgButtonChecked( hWndDialog, RADIO_TARGET_OTHER );
					EnableWindow( GetDlgItem( hWndDialog, EDIT_WIDTH ), enable );
					EnableWindow( GetDlgItem( hWndDialog, STATIC_X ), enable );

					break;

				}  // end target image size radio buttons

				// --------------------------------------------------------------------
				case EDIT_WIDTH:
				{

					switch( notifyCode )
					{

						// ----------------------------------------------------------------
						case EN_UPDATE:
						{
							char buffer[ 32 ];

							//
							// the text of the width has changed, since our requirements
							// are that the image must be square automatically update
							// the height box as well
							//
							GetDlgItemText( hWndDialog, EDIT_WIDTH, buffer, 32 );
							SetDlgItemText( hWndDialog, EDIT_HEIGHT, buffer );

							break;

						}  // end update

					}  // end switch

					break;

				}  // end user defined width

				// --------------------------------------------------------------------
				case BUTTON_START:
				{
					Bool success;

					// get all the options from the dialog into the image packer
					success = TheImagePacker->getSettingsFromDialog( hWndDialog );

					// run the packer if nothing went wrong
					if( success )
						TheImagePacker->process();

					break;

				}  // end execute

				// --------------------------------------------------------------------
				case BUTTON_EXIT:
				{

					// quit the program
					EndDialog( hWndDialog, TRUE );
					break;

				}  // end exit

			}  // end switch

			return 0;

		}  // end command

	}  // end switch

	return 0;

}  // end ImagePackerProc


