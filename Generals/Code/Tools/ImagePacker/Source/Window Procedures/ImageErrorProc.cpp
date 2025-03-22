/*
**	Command & Conquer Generals(tm)
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

// FILE: ImageErrorProc.cpp ///////////////////////////////////////////////////
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
// File name:  ImageErrorProc.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Window procedure for the error dialog
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "ImagePacker.h"
#include "Resource.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ImageErrorProc =============================================================
/** Dialog proc for the error window */
//=============================================================================
BOOL CALLBACK ImageErrorProc( HWND hWndDialog, UINT message,
														  WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

		// ------------------------------------------------------------------------
		case WM_INITDIALOG:
		{

			//
			// load the listbox with images that could not be processed
			// and the reasons for it
			//
			
			// sanity
			if( TheImagePacker == NULL )
				return TRUE;

			// go through all images
			Int i, count;
			HWND list = GetDlgItem( hWndDialog, LIST_IMAGES );
			char buffer[ _MAX_PATH + 256 ];
			char reason[ 32 ];
			ImageInfo *image;

			count = TheImagePacker->getImageCount();
			for( i = 0; i < count; i++ )
			{

				// get image
				image = TheImagePacker->getImage( i );

				// sanity
				if( image == NULL )	
					continue;

				// if image can't be processed find out why
				if( BitIsSet( image->m_status, ImageInfo::CANTPROCESS ) )
				{
					
					if( BitIsSet( image->m_status, ImageInfo::TOOBIG ) )
						sprintf( reason, "Too Big" );
					else if( BitIsSet( image->m_status, ImageInfo::INVALIDCOLORDEPTH ) )
						sprintf( reason, "Unsupported Color Depth" );
					else
						sprintf( reason, "Unknown Reason" );

					sprintf( buffer, "%s: (%dx%dx%d) %s",
									 reason, image->m_size.x, image->m_size.y, image->m_colorDepth,
									 image->m_path );

					SendMessage( list, LB_INSERTSTRING, -1, (LPARAM)buffer );

				}  // end if

			}  // end for i

			// set the extents for the horizontal scroll bar in the listbox
			SendMessage( list, LB_SETHORIZONTALEXTENT, 1280, 0 );

			return TRUE;

		}  // end init

		// ------------------------------------------------------------------------
		case WM_COMMAND:
		{
			Int controlID = LOWORD( wParam );
//			Int notifyCode = HIWORD( wParam );
//			HWND hWndControl = (HWND)lParam;

			switch( controlID )
			{

				// --------------------------------------------------------------------
				case BUTTON_PROCEED:
				{

					EndDialog( hWndDialog, TRUE );
					break;

				}  // end proceed

				// --------------------------------------------------------------------
				case BUTTON_CANCEL:
				{

					EndDialog( hWndDialog, FALSE );
					break;

				}  // end cancel

			}  // end switch

			break;

		}  // end command

	}  // end switch message

	return 0;

}  // end ImageErrorProc


