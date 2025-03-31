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

// FILE: PreviewProc.cpp //////////////////////////////////////////////////////
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
// File name:  PreviewProc.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Window procedure for the preview window
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "ImagePacker.h"
#include "WinMain.h"

///////////////////////////////////////////////////////////////////////////////
// DEFINES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define PREVIEW_STYLE WS_CAPTION

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// PreviewProc ================================================================
/** */
//=============================================================================
LRESULT CALLBACK PreviewProc( HWND hWnd, UINT message, 
														  WPARAM wParam, LPARAM lParam )
{

	switch( message )
	{

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			HBRUSH whiteBrush = (HBRUSH)GetStockObject( WHITE_BRUSH );

			hdc = BeginPaint(hWnd, &ps);

			SelectObject( hdc, GetStockObject( WHITE_PEN ) );

			// find the target texture page
			TexturePage *page;
			for( page = TheImagePacker->getFirstTexturePage();
					 page; 
					 page = page->m_next )
			{

				// if this is not the target page, ignore it
				if( page->getID() != TheImagePacker->getTargetPreviewPage() )
					continue;

				// draw image based on the generated texture or silhouette
				if( TheImagePacker->getUseTexturePreview() )
				{
					Int x, y;
					Byte r, g, b;
					HPEN prevPen, pen;

					for( y = 0; y < page->getHeight(); y++ )
					{

						for( x = 0; x < page->getWidth(); x++ )
						{

							// get the color here
							page->getPixel( x, y, &r, &g, &b );

							// create a new pen of the right color
							pen = CreatePen( 1, 1, RGB( r, g, b, ) );
							
							// select pen into hdc
							prevPen = (HPEN)SelectObject( hdc, pen );
							
							// draw ... what is the Win32 put pixel function???
							MoveToEx( hdc, x, y, NULL );
							LineTo( hdc, x + 1, y );

							// put the old pen back
							SelectObject( hdc, prevPen );
							
							// delete the created pen
							DeleteObject( pen );
									
						}  // end for x

					}  // end for y

				}  // end if
				else
				{

					// go through all the images on this page
					ImageInfo *image;
					for( image = page->getFirstImage();
							 image;
							 image = image->m_nextPageImage )
					{
						RECT rect;

						rect.left		= image->m_pagePos.lo.x;
						rect.top		= image->m_pagePos.lo.y;
						rect.right	= image->m_pagePos.hi.x + 1;  // FillRect not inclusive
						rect.bottom = image->m_pagePos.hi.y + 1;  // FillRect not inclusive
						FillRect( hdc, &rect, whiteBrush );

					}  // end for image
				
				}  // end else

			}  // end for page

			EndPaint( hWnd, &ps );
			break;

		}  // end paint

	}  // end switch

	return DefWindowProc( hWnd, message, wParam, lParam );

}  // end PreviewProc

// MakePreviewDisplay =========================================================
/** */
//=============================================================================
HWND MakePreviewDisplay( void )
{
	WNDCLASSEX wcex;
	const char *className = "PreviewDisplay";
	HWND hWnd;

	wcex.cbSize = sizeof( WNDCLASSEX ); 

	wcex.style					= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc		= (WNDPROC)PreviewProc;
	wcex.cbClsExtra			= 0;
	wcex.cbWndExtra			= 0;
	wcex.hInstance			= ApplicationHInstance;
	wcex.hIcon					= NULL;
	wcex.hCursor				= LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground	= (HBRUSH)GetStockObject( BLACK_BRUSH );
	wcex.lpszMenuName		=	NULL;
	wcex.lpszClassName	= className;
	wcex.hIconSm				= NULL;

	RegisterClassEx( &wcex );

	// create app window and keep handle
	hWnd = CreateWindowEx( 0,											// extended style
												 className,							// window class name
												 "Preview",							// window name
												 PREVIEW_STYLE,					// window styles
												 30,											// x position
												 30,											// y position
												 TheImagePacker->getTargetWidth(),
												 TheImagePacker->getTargetHeight(),
												 NULL,									// parent
												 NULL,									// menu
												 ApplicationHInstance,	// instance
												 NULL );								// creation data

	if( hWnd == NULL )
		return NULL;

	// display the window
	ShowWindow( hWnd, SW_SHOW );

	return hWnd;

}  // end MakePreviewDisplay

// UpdatePreviewWindow ========================================================
/** Update the preview window, if present */
//=============================================================================
void UpdatePreviewWindow( void )
{
	HWND preview;

	// sanity
	if( TheImagePacker == NULL )
		return;

	// get preview window
	preview = TheImagePacker->getPreviewWindow();

	// if window not here don't bother
	if( preview == NULL )
		return;

	// make the title
	char title[ 256 ];

	// construct title
	sprintf( title, "Page #%d of %d", 
					 TheImagePacker->getTargetPreviewPage(), 
					 TheImagePacker->getPageCount() );
	SetWindowText( preview, title );

	// adjust the window rect so the client area is the target packed size
	RECT clientRect;
	POINT p;

	p.x = 0;
	p.y = 0;
	ClientToScreen( preview, &p );
	clientRect.left = p.x;
	clientRect.right = clientRect.left + TheImagePacker->getTargetWidth();
	clientRect.top = p.y;
	clientRect.bottom = clientRect.top + TheImagePacker->getTargetHeight();
	AdjustWindowRect( &clientRect, PREVIEW_STYLE, FALSE );
	MoveWindow( preview,
							clientRect.left, 
							clientRect.top,
							clientRect.right - clientRect.left,
							clientRect.bottom - clientRect.top,
							TRUE );

	// invalidate the client area for redraw
	InvalidateRect( preview, NULL, TRUE );

}  // end UpdatePreviewWindow
