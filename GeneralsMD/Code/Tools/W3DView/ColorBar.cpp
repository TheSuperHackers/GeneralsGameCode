/*
**	Command & Conquer Renegade(tm)
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

/////////////////////////////////////////////////////////////////////////////////
//
// ColorBar.cpp : implementation file
//
//

#include "stdafx.h"
#include "colorbar.h"
#include "colorutils.h"
#include "resource.h"
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern HINSTANCE _hinstance;


/////////////////////////////////////////////////////////////////////////////
//
// ColorBarClass
//
ColorBarClass::ColorBarClass (void)
	: m_hBitmap (NULL),
	  m_iBMPWidth (0),
	  m_iBMPHeight (0),
	  m_pBits (NULL),
	  m_hMemDC (NULL),
	  m_iColorPoints (0),
	  m_iMarkerWidth (0),
	  m_iMarkerHeight (0),
	  m_KeyFrameDIB (NULL),
	  m_pKeyFrameBits (NULL),
	  m_iCurrentKey (0),
	  m_MinPos (0),
	  m_MaxPos (1),
	  m_iScanlineSize (0),
	  m_bMoving (false),
	  m_bMoved (false),
	  m_SelectionPos (0),
	  m_bRedraw (true),
	  CWnd ()
{
	::memset (m_ColorPoints, 0, sizeof (m_ColorPoints));

	m_iColorPoints = 1;
	m_ColorPoints[0].PosPercent = 0;
	m_ColorPoints[0].StartRed = 0;
	m_ColorPoints[0].StartGreen = 0;
	m_ColorPoints[0].StartBlue = 0;
	m_ColorPoints[0].flags = POINT_VISIBLE;

	/*m_iColorPoints = 7;
	m_ColorPoints[0].PosPercent = 0;
	m_ColorPoints[0].StartRed = 255;
	m_ColorPoints[0].StartGreen = 0;
	m_ColorPoints[0].StartBlue = 0;
	m_ColorPoints[0].flags = POINT_VISIBLE;

	m_ColorPoints[1].PosPercent = (1.0F / 6.0F);
	m_ColorPoints[1].StartRed = 255;
	m_ColorPoints[1].StartGreen = 255;
	m_ColorPoints[1].StartBlue = 0;
	m_ColorPoints[1].flags = POINT_VISIBLE | POINT_CAN_MOVE;

	m_ColorPoints[2].PosPercent = (2.0F / 6.0F);
	m_ColorPoints[2].StartRed = 0;
	m_ColorPoints[2].StartGreen = 255;
	m_ColorPoints[2].StartBlue = 0;
	m_ColorPoints[2].flags = POINT_VISIBLE | POINT_CAN_MOVE;

	m_ColorPoints[3].PosPercent = (3.0F / 6.0F);
	m_ColorPoints[3].StartRed = 0;
	m_ColorPoints[3].StartGreen = 255;
	m_ColorPoints[3].StartBlue = 255;
	m_ColorPoints[3].flags = POINT_VISIBLE | POINT_CAN_MOVE;

	m_ColorPoints[4].PosPercent = (4.0F / 6.0F);
	m_ColorPoints[4].StartRed = 0;
	m_ColorPoints[4].StartGreen = 0;
	m_ColorPoints[4].StartBlue = 255;
	m_ColorPoints[4].flags = POINT_VISIBLE | POINT_CAN_MOVE;

	m_ColorPoints[5].PosPercent = (5.0F / 6.0F);
	m_ColorPoints[5].StartRed = 255;
	m_ColorPoints[5].StartGreen = 0;
	m_ColorPoints[5].StartBlue = 255;
	m_ColorPoints[5].flags = POINT_VISIBLE | POINT_CAN_MOVE;

	m_ColorPoints[6].PosPercent = 1;
	m_ColorPoints[6].StartRed = 255;
	m_ColorPoints[6].StartGreen = 0;
	m_ColorPoints[6].StartBlue = 0;
	m_ColorPoints[6].flags = POINT_VISIBLE | POINT_CAN_MOVE;*/
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~ColorBarClass
//
ColorBarClass::~ColorBarClass (void)
{
	if (m_hMemDC != NULL) {
		::DeleteObject (m_hMemDC);
		m_hMemDC = NULL;
	}
		
	Free_Marker_Bitmap ();
	Free_Bitmap ();
	return ;
}


BEGIN_MESSAGE_MAP(ColorBarClass, CWnd)
	//{{AFX_MSG_MAP(ColorBarClass)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// RegisterColorBar
//
void
RegisterColorBar (HINSTANCE hinst)
{
	// Has the class already been registered?
	WNDCLASS wndclass = { 0 };
	if (::GetClassInfo (hinst, "WWCOLORBAR", &wndclass) == FALSE) {
		
		wndclass.style = CS_GLOBALCLASS | CS_OWNDC | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		wndclass.lpfnWndProc = fnColorBarProc;
		wndclass.hInstance = hinst;
		wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
		wndclass.hCursor = ::LoadCursor (NULL, IDC_ARROW);
		wndclass.lpszClassName = "WWCOLORBAR";
		
		// Let the windows manager know about this global class
		::RegisterClass (&wndclass);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// fnColorBarProc
//
LRESULT WINAPI
fnColorBarProc
(
	HWND hwnd,
	UINT message,
	WPARAM wparam,
	LPARAM lparam
)
{
	switch (message)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT pcreate_info = (LPCREATESTRUCT)lparam;
			if (pcreate_info != NULL) {

				// Should we create a new class manager for this window?
				ColorBarClass *pwnd = (ColorBarClass *)pcreate_info->lpCreateParams;
				BOOL created = FALSE;
				if (pwnd == NULL) {
					pwnd = new ColorBarClass;
					created = TRUE;
				}

				// Pull some hacks to get MFC to use the message map
				pwnd->Attach (hwnd);

				// Let the window know its being created
				pwnd->OnCreate (pcreate_info);
				
				WNDPROC *pOldWndProc = pwnd->GetSuperWndProcAddr ();
				if (pOldWndProc) {
					WNDPROC pold_proc = (WNDPROC)::SetWindowLong (hwnd, GWL_WNDPROC, (DWORD)::AfxGetAfxWndProc ());
					ASSERT (pold_proc != NULL);
					(*pOldWndProc) = pold_proc;
				}

				// Store some information in the window handle
				::SetProp (hwnd, "CLASSPOINTER", (HANDLE)pwnd);
				::SetProp (hwnd, "CREATED", (HANDLE)created);
			}
		}
		break;

		case WM_DESTROY:
		{
			// Get the creation information from the window handle
			ColorBarClass *pwnd = (ColorBarClass *)::GetProp (hwnd, "CLASSPOINTER");
			BOOL created = (BOOL)::GetProp (hwnd, "CREATED");

			if (pwnd != NULL) {
				pwnd->Detach ();

				WNDPROC *pOldWndProc = pwnd->GetSuperWndProcAddr ();
				if (pOldWndProc) {
					::SetWindowLong (hwnd, GWL_WNDPROC, (DWORD)(*pOldWndProc));
					(*pOldWndProc) = NULL;
				}

				if (created) {
					delete pwnd;
					pwnd = NULL;
				}
			}
		}
		break;
	}

	// Allow default processing to occur
	return ::DefWindowProc (hwnd, message, wparam, lparam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCreate
//
int
ColorBarClass::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate (lpCreateStruct) == -1)
		return -1;
	
	m_hMemDC = 	::CreateCompatibleDC (NULL);
	Create_Bitmap ();
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create
//
BOOL
ColorBarClass::Create
(
	LPCTSTR /*lpszClassName*/,
	LPCTSTR lpszWindowName,
	DWORD dwStyle,
	const RECT &rect,
	CWnd *pparent_wnd,
	UINT nID,
	CCreateContext * /*pContext*/
)
{
	// Create the window (it will force the message map and everthing)
	HWND hparent_wnd = (pparent_wnd != NULL) ? pparent_wnd->m_hWnd : NULL;
	HWND hwnd = ::CreateWindow ("WWCOLORBAR",
										 lpszWindowName,
										 dwStyle,
										 rect.left,
										 rect.top,
										 rect.right - rect.left,
										 rect.bottom - rect.top,
										 hparent_wnd,
										 (HMENU)nID,
										 ::AfxGetInstanceHandle (),
										 this);

	// Return the true/false result code
	return (hwnd != NULL);
}


/////////////////////////////////////////////////////////////////////////////
//
// Create_Bitmap
//
void
ColorBarClass::Create_Bitmap (void)
{
	// Start fresh
	Free_Bitmap ();

	CRect rect;
	GetClientRect (&rect);
	m_iBMPWidth = rect.Width ();
	m_iBMPHeight = rect.Height ();

	// Assume the color portion of the control takes the whole client aread
	m_ColorArea.left = 0;
	m_ColorArea.top = 0;
	m_ColorArea.right = m_iBMPWidth;
	m_ColorArea.bottom = m_iBMPHeight;
	
	//
	//	Deflate the rect to make room for any frame
	//
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_FRAME_MASK) {
		m_ColorArea.InflateRect (-1, -1);
	}

	//
	//	Decrease the width or height if frame markers are shown
	//	
	if (style & CBRS_SHOW_FRAMES) {
		Load_Key_Frame_BMP ();
		if (style & CBRS_HORZ) {
			m_ColorArea.left += (m_iMarkerWidth >> 1);
			m_ColorArea.right -= (m_iMarkerWidth >> 1);
			m_ColorArea.bottom -= (m_iMarkerHeight >> 1);
		} else {
			m_ColorArea.top += (m_iMarkerHeight >> 1);
			m_ColorArea.bottom  -= (m_iMarkerHeight >> 1);
			m_ColorArea.right -= (m_iMarkerWidth >> 1);
		}
	} else if (style & CBRS_HAS_SEL) {
		if (style & CBRS_HORZ) {
			m_ColorArea.left += 2;
			m_ColorArea.right -= 2;
		} else {
			m_ColorArea.top += 2;
			m_ColorArea.bottom -= 2;
		}		
	}

	// Set-up the fields of the BITMAPINFOHEADER
	BITMAPINFOHEADER bitmap_info;
	bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth = m_iBMPWidth;
	bitmap_info.biHeight = -m_iBMPHeight; // Top-down DIB uses negative height
	bitmap_info.biPlanes = 1;
	bitmap_info.biBitCount = 24;
	bitmap_info.biCompression = BI_RGB;
	bitmap_info.biSizeImage = ((m_iBMPWidth * m_iBMPHeight) * 3);
	bitmap_info.biXPelsPerMeter = 0;
	bitmap_info.biYPelsPerMeter = 0;
	bitmap_info.biClrUsed = 0;
	bitmap_info.biClrImportant = 0;

	// Get a temporary screen DC
	HDC hscreen_dc = ::GetDC (NULL);

	// Create a bitmap that we can access the bits directly of
	m_hBitmap = ::CreateDIBSection (hscreen_dc,
											  (const BITMAPINFO *)&bitmap_info,
											  DIB_RGB_COLORS,
											  (void **)&m_pBits,
											  NULL,
											  0L);

	// Release our temporary screen DC
	::ReleaseDC (NULL, hscreen_dc);

	// Window's bitmaps are DWORD aligned, so make sure
	// we take that into account.
	int alignment_offset = (m_iBMPWidth * 3) % 4;
	alignment_offset = (alignment_offset != 0) ? (4 - alignment_offset) : 0;
	m_iScanlineSize = (m_iBMPWidth * 3) + alignment_offset;

	Update_Point_Info ();
	return ;
}



/////////////////////////////////////////////////////////////////////////////
//
// Free_Bitmap
//
void
ColorBarClass::Free_Bitmap (void)
{
	if (m_hBitmap != NULL) {
		::DeleteObject (m_hBitmap);
		m_hBitmap = NULL;
		m_pBits = NULL;
	}

	m_iBMPWidth = 0;
	m_iBMPHeight = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
void
ColorBarClass::OnSize
(
	UINT nType,
	int cx,
	int cy
) 
{
	// Allow the base class to process this message
	CWnd::OnSize (nType, cx, cy);

	// Recreate the BMP to reflect the new window size
	Create_Bitmap ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Bar_Vert
//
void
ColorBarClass::Paint_Bar_Vert
(
	int x_pos,
	int y_pos,
	int width,
	int height,
	UCHAR *pbits
)
{
	// Loop through all the color switches
	int row = y_pos;
	for (int color_point = 0; color_point < m_iColorPoints; color_point ++) {
		
		// Get the starting red, green, and blue values for this color lerp
		float red = m_ColorPoints[color_point].StartRed;
		float green = m_ColorPoints[color_point].StartGreen;
		float blue = m_ColorPoints[color_point].StartBlue;

		// Index into the bitmap's bits
		int bitmap_index = (row * m_iScanlineSize) + (x_pos * 3);

		// Loop through all rows of color in this 'switch'
		int end_pos = m_ColorPoints[color_point].EndPos;
		end_pos = (end_pos > (y_pos + height)) ? (y_pos + height) : end_pos;
		for (; row < m_ColorPoints[color_point].EndPos; row ++) {

			// Paint a complete row of color data
			int bitmap_offset = 0;
			for (int col = x_pos; col < (x_pos + width); col ++) {

				// Paint the pixel
				pbits[bitmap_index + bitmap_offset]		= UCHAR(((int)blue) & 0xFF);
				pbits[bitmap_index + bitmap_offset+1]	= UCHAR(((int)green) & 0xFF);
				pbits[bitmap_index + bitmap_offset+2]	= UCHAR(((int)red) & 0xFF);

				// Move to the next col
				bitmap_offset += 3;
			}

			// Advance to the next row
			bitmap_index += m_iScanlineSize;

			// Increment the row's color
			red += m_ColorPoints[color_point].RedInc;
			green += m_ColorPoints[color_point].GreenInc;
			blue += m_ColorPoints[color_point].BlueInc;
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Bar_Horz
//
void
ColorBarClass::Paint_Bar_Horz
(
	int x_pos,
	int y_pos,
	int width,
	int height,
	UCHAR *pbits
)
{
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_PAINT_GRAPH) {

		// Loop through all the color switches
		int col = x_pos;	
		for (int color_point = 0; color_point < m_iColorPoints; color_point ++) {			
			float graph_percent = m_ColorPoints[color_point].StartGraphPercent;

			// Index into the bitmap's bits
			int bitmap_index = (y_pos * m_iScanlineSize) + (col * 3);

			// Loop through all columns of color in this 'switch'
			int end_pos = m_ColorPoints[color_point].EndPos;
			end_pos = (end_pos > (x_pos + width)) ? (x_pos + width) : end_pos;
			for (; col < m_ColorPoints[color_point].EndPos; col ++) {
				
				// Paint a complete column of color data
				int bitmap_offset = 0;
				bool prev_was = true;
				for (int row = y_pos; row < (y_pos + height); row ++) {
					
					float percent = 1 - graph_percent;
					float height_percent = (((float)row) / ((float)height));
					if (percent < height_percent) {

						// Paint the pixel
						if (prev_was == false) {
							pbits[bitmap_index + bitmap_offset]		= 0;
							pbits[bitmap_index + bitmap_offset+1]	= 0;
							pbits[bitmap_index + bitmap_offset+2]	= 0;
						} else {
							pbits[bitmap_index + bitmap_offset]		= 255;
							pbits[bitmap_index + bitmap_offset+1]	= 128 + UCHAR(((int)((1-height_percent) * 128)) & 0xFF);
							pbits[bitmap_index + bitmap_offset+2]	= UCHAR(((int)((1-height_percent) * 255)) & 0xFF);
						}
						prev_was = true;
					} else {

						// Paint the pixel
						pbits[bitmap_index + bitmap_offset]		= 128;
						pbits[bitmap_index + bitmap_offset+1]	= 128;
						pbits[bitmap_index + bitmap_offset+2]	= 128;
						prev_was = false;
					}

					// Move down to the next row
					bitmap_offset += m_iScanlineSize;
				}

				// Advance to the next column
				bitmap_index += 3;

				graph_percent += m_ColorPoints[color_point].GraphPercentInc;
			}
		}

	} else {

		// Loop through all the color switches
		int col = x_pos;	
		for (int color_point = 0; color_point < m_iColorPoints; color_point ++) {
			
			// Get the starting red, green, and blue values for this color lerp
			float red = m_ColorPoints[color_point].StartRed;
			float green = m_ColorPoints[color_point].StartGreen;
			float blue = m_ColorPoints[color_point].StartBlue;

			// Index into the bitmap's bits
			int bitmap_index = (y_pos * m_iScanlineSize) + (col * 3);

			// Loop through all columns of color in this 'switch'
			int end_pos = m_ColorPoints[color_point].EndPos;
			end_pos = (end_pos > (x_pos + width)) ? (x_pos + width) : end_pos;
			for (; col < m_ColorPoints[color_point].EndPos; col ++) {
				
				// Paint a complete column of color data
				int bitmap_offset = 0;
				for (int row = y_pos; row < (y_pos + height); row ++) {
					
					// Paint the pixel
					pbits[bitmap_index + bitmap_offset]		= UCHAR(((int)blue) & 0xFF);
					pbits[bitmap_index + bitmap_offset+1]	= UCHAR(((int)green) & 0xFF);
					pbits[bitmap_index + bitmap_offset+2]	= UCHAR(((int)red) & 0xFF);

					// Move down to the next row
					bitmap_offset += m_iScanlineSize;
				}

				// Advance to the next column
				bitmap_index += 3;

				// Increment the column's color
				red += m_ColorPoints[color_point].RedInc;
				green += m_ColorPoints[color_point].GreenInc;
				blue += m_ColorPoints[color_point].BlueInc;
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_DIB
//
void
ColorBarClass::Paint_DIB (void)
{
	CRect frame_rect = m_ColorArea;
	frame_rect.InflateRect (1, 1);

	//
	//	Erase the background to start...
	//
	CRect fill_rect (0, 0, m_iBMPWidth, m_iBMPHeight);
	HBITMAP hold_bmp = (HBITMAP)::SelectObject (m_hMemDC, m_hBitmap);
	::FillRect (m_hMemDC, &fill_rect, (HBRUSH)(COLOR_3DFACE + 1));
	::SelectObject (m_hMemDC, hold_bmp);	

	//
	//	Paint the border (if any)
	//
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_SUNKEN) {
		::Draw_Sunken_Rect (m_pBits, frame_rect, m_iScanlineSize);
	} else if (style & CBRS_RAISED) {
		::Draw_Raised_Rect (m_pBits, frame_rect, m_iScanlineSize);
	} else if (style & CBRS_FRAME) {
		::Frame_Rect (m_pBits, frame_rect, RGB (0, 0, 0), m_iScanlineSize);
	}

	//
	// Now draw the color areas
	//
	if (style & CBRS_HORZ) {
		Paint_Bar_Horz (m_ColorArea.left,
							 m_ColorArea.top,
							 m_ColorArea.Width (),
							 m_ColorArea.Height (),
							 m_pBits);
	} else {
		Paint_Bar_Vert (m_ColorArea.left,
							 m_ColorArea.top,
							 m_ColorArea.Width (),
							 m_ColorArea.Height (),
							 m_pBits);
	}


	//
	//	Do we need to paint the keyframe markers?
	//	
	if (style & CBRS_SHOW_FRAMES) {		

		//
		//	Setup the marker positions
		//
		int x_pos = 0;
		int y_pos = 0;
		int *position = NULL;
		int offset = 0;

		if (style & CBRS_HORZ) {
			position = &x_pos;
			x_pos = m_ColorArea.left;
			y_pos = m_ColorArea.Height () - (m_iMarkerHeight >> 1);
			offset = -(m_iMarkerWidth >> 1);
		} else {
			position = &y_pos;
			x_pos = m_ColorArea.Width () - (m_iMarkerWidth >> 1);
			y_pos = m_ColorArea.top;
			offset = -(m_iMarkerHeight >> 1);
		}

		//
		// Paint all the keyframe markers
		//
		for (int color_point = 0; color_point < m_iColorPoints; color_point ++) {
			(*position) = m_ColorPoints[color_point].StartPos;
			(*position) += offset;
			Paint_Key_Frame (x_pos, y_pos);
		}

	} else if (style & CBRS_HAS_SEL) {
		
		CRect frame;
		Get_Selection_Rectangle (frame);
		
		//
		//	Paint the selection rectangles
		//
		::Frame_Rect (m_pBits, frame, RGB (0, 0, 0), m_iScanlineSize);
		frame.InflateRect (-1, -1);
		::Frame_Rect (m_pBits, frame, RGB (255, 255, 255), m_iScanlineSize);
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnPaint
//
void
ColorBarClass::OnPaint (void)
{
	CPaintDC dc (this);
	Paint_Screen (dc);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Screen
//
void
ColorBarClass::Paint_Screen (HDC hwnd_dc)
{
	if (m_hMemDC != NULL) {
				
		//
		//	Blit the actual color bar to the screen
		//
		HBITMAP hold_bmp = (HBITMAP)::SelectObject (m_hMemDC, m_hBitmap);
		::BitBlt (hwnd_dc, 0, 0, m_iBMPWidth, m_iBMPHeight, m_hMemDC, 0, 0, SRCCOPY);
		::SelectObject (m_hMemDC, hold_bmp);

		//
		//	Do we have the focus?
		//
		if (::GetFocus () == m_hWnd) {
						
			LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
			if (style & CBRS_SHOW_FRAMES) {
				
				//
				//	Calculate the current frame marker's current rectangle
				//
				CRect focus_rect;
				LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
				if (style & CBRS_HORZ) {
					focus_rect.left = m_ColorPoints[m_iCurrentKey].StartPos - (m_iMarkerWidth >> 1);
					focus_rect.top = m_ColorArea.Height () - (m_iMarkerHeight >> 1);
				} else {
					focus_rect.left = m_ColorArea.Width () - (m_iMarkerWidth >> 1);
					focus_rect.top = m_ColorPoints[m_iCurrentKey].StartPos - (m_iMarkerHeight >> 1);
				}
				focus_rect.right = focus_rect.left + m_iMarkerWidth;
				focus_rect.bottom = focus_rect.top + m_iMarkerHeight;	

				//
				//	Paint the focus rectangle
				//
				::DrawFocusRect (hwnd_dc, focus_rect);
			} else if (style & CBRS_HAS_SEL) {
				
				//
				//	Paint the focus rectangle
				//				
				CRect focus_rect;
				Get_Selection_Rectangle (focus_rect);
				::DrawFocusRect (hwnd_dc, focus_rect);				
			}
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Point
//
bool
ColorBarClass::Get_Point
(
	int index,
	float *position,
	float *red,
	float *green,
	float *blue
)
{
	// Assume failure
	bool retval = false;

	if ((index >= 0) && (index < m_iColorPoints)) {
		
		// Return the position to the caller if requested
		if (position != NULL) {
			(*position) = m_MinPos + (m_ColorPoints[index].PosPercent * (m_MaxPos - m_MinPos));
		}

		// Return the red value to the caller if requested
		if (red != NULL) {
			(*red) = m_ColorPoints[index].StartRed;
		}

		// Return the green value to the caller if requested
		if (green != NULL) {
			(*green) = m_ColorPoints[index].StartGreen;
		}

		// Return the blue value to the caller if requested
		if (blue != NULL) {
			(*blue) = m_ColorPoints[index].StartBlue;
		}
	}
	
	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Point
//
bool
ColorBarClass::Insert_Point (CPoint point, DWORD flags)
{
	int new_index = 0;
	int position = 0;
	float percent = 0;

	float red = 0;
	float green = 0;
	float blue = 0;

	//
	//	Determine what properties the new point should have
	//	
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_HORZ) {
		position = point.x - m_ColorArea.left;
		percent = ((float)(point.x - m_ColorArea.left)) / ((float)m_ColorArea.Width ());

		int bmp_index = ((m_ColorArea.top + 1) * m_iScanlineSize) + (point.x * 3);
		blue	= m_pBits[bmp_index];
		green = m_pBits[bmp_index+1];
		red	= m_pBits[bmp_index+2];
	} else {
		position = point.y - m_ColorArea.top;
		percent = ((float)(point.y - m_ColorArea.top)) / ((float)m_ColorArea.Height ());

		int bmp_index = (point.y * m_iScanlineSize) + (m_ColorArea.left * 3);
		blue	= m_pBits[bmp_index];
		green = m_pBits[bmp_index+1];
		red	= m_pBits[bmp_index+2];
	}	

	//
	//	Find the index where this point should be inserted
	//	
	bool found = false;
	float graph_percent = 0;
	for (int index = 0; (index < m_iColorPoints) && !found; index ++) {
		if ((position > m_ColorPoints[index].StartPos) &&
			 (position < m_ColorPoints[index].EndPos)) {
			new_index = index + 1;
			found = true;

			//
			//	Determine what graph percent to use if the control
			// is in graph mode
			//
			float key_percent = (((float)position) - ((float)m_ColorPoints[index].StartPos)) / float(m_ColorPoints[index].EndPos - m_ColorPoints[index].StartPos);
			float graph_start = m_ColorPoints[index].StartGraphPercent;
			float graph_delta = (m_ColorPoints[index].EndPos - m_ColorPoints[index].StartPos) * m_ColorPoints[index].GraphPercentInc;
			graph_percent = graph_start + (key_percent * graph_delta);			
		}
	}

	bool retval = false;
	if (found) {
		Set_Redraw (false);

		//
		// Insert the new point
		//
		float position = m_MinPos + ((m_MaxPos - m_MinPos) * percent);
		retval = Insert_Point (new_index, position, red, green, blue);
		if (retval) {
			Set_Graph_Percent (new_index, graph_percent);
			m_iCurrentKey = new_index;
		}

		Set_Redraw (true);
	}
	
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Insert_Point
//
bool
ColorBarClass::Insert_Point
(
	int index,
	float position,
	float red,
	float green,
	float blue,
	DWORD flags
)
{
	// Assume failure
	bool retval = false;

	// Params valid?
	if (((m_iColorPoints + 1) < MAX_COLOR_POINTS) &&
		 (index >= 0) &&
		 (index <= m_iColorPoints)) {

		// Bump all the points up one position in the array
		::memmove (&m_ColorPoints[index + 1],
					  &m_ColorPoints[index],
					  sizeof (COLOR_POINT) * (m_iColorPoints - index));

		// Increment the point count
		m_iColorPoints ++;

		// Now all we have to do is modify the contents of the new point
		retval = Modify_Point (index, position, red, green, blue, flags);
	}
	
	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Delete_Point
//
bool
ColorBarClass::Delete_Point (int index)
{
	// Assume failure
	bool retval = false;

	// Params valid?
	if ((index > 0) &&
		 (index < m_iColorPoints)) {

		// Bump all the points up one position in the array
		::memmove (&m_ColorPoints[index],
					  &m_ColorPoints[index + 1],
					  sizeof (COLOR_POINT) * (m_iColorPoints - (index + 1)));

		// Decrement the point count
		m_iColorPoints --;
		if (m_iCurrentKey > 0) {
			m_iCurrentKey --;
		}

		// Rebuild any color information we may need to
		Update_Point_Info ();

		// Force the window to be repainted
		Repaint ();
		retval = true;
	}
	
	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Modify_Point
//
bool
ColorBarClass::Modify_Point
(
	int index,
	float position,
	float red,
	float green,
	float blue,
	DWORD flags
)
{
	// Assume failure
	bool retval = false;

	if ((index >= 0) && (index < m_iColorPoints)) {
		
		// Update the information about this color point
		m_ColorPoints[index].PosPercent = ((position - m_MinPos) / (m_MaxPos - m_MinPos));
		m_ColorPoints[index].StartRed = red;
		m_ColorPoints[index].StartGreen = green;
		m_ColorPoints[index].StartBlue = blue;
		m_ColorPoints[index].flags = flags;

		if (index == 0) {
			m_ColorPoints[index].flags &= ~POINT_CAN_MOVE;
		}

		// Rebuild any color information we may need to
		Update_Point_Info ();

		// Force the window to be repainted
		Repaint ();
		retval = true;
	} else if (index >= m_iColorPoints) {

		// If the user wants to tag on onto the end, then insert a new point...
		retval = Insert_Point (m_iColorPoints, position, red, green, blue);
	}
	
	// Return the true/false result code
	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Range
//
void
ColorBarClass::Set_Range
(
	float min,
	float max
)
{
	m_MinPos = min;
	m_MaxPos = max;

	m_SelectionPos = m_MinPos;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Point_Info
//
void
ColorBarClass::Update_Point_Info (void)
{	
	int width = m_ColorArea.Width ();
	int height = m_ColorArea.Height ();	

	LONG style = GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_HORZ) {

		// Loop through all the color points
		for (int index = 0; index < m_iColorPoints; index ++) {

			// Update the absolute starting position for this point
			m_ColorPoints[index].StartPos = m_ColorArea.left + int(m_ColorPoints[index].PosPercent * width);

			// Update the color information for the previous point
			if (index > 0) {			
				m_ColorPoints[index-1].EndPos = m_ColorPoints[index].StartPos;
				
				//
				// Assign color increments to the previous color point
				//
				int width = m_ColorPoints[index-1].EndPos - m_ColorPoints[index-1].StartPos;
				m_ColorPoints[index-1].RedInc = (m_ColorPoints[index].StartRed - m_ColorPoints[index-1].StartRed) / ((float)width);
				m_ColorPoints[index-1].GreenInc = (m_ColorPoints[index].StartGreen - m_ColorPoints[index-1].StartGreen) / ((float)width);
				m_ColorPoints[index-1].BlueInc = (m_ColorPoints[index].StartBlue - m_ColorPoints[index-1].StartBlue) / ((float)width);

				// Assign the graph increment to the previous point
				m_ColorPoints[index-1].GraphPercentInc = (m_ColorPoints[index].StartGraphPercent - m_ColorPoints[index-1].StartGraphPercent) / ((float)width);
			}
		}

		m_ColorPoints[index-1].RedInc				= 0;
		m_ColorPoints[index-1].GreenInc			= 0;
		m_ColorPoints[index-1].BlueInc			= 0;
		m_ColorPoints[index-1].GraphPercentInc = 0;
		m_ColorPoints[index-1].EndPos				= m_ColorArea.right;

	} else {

		// Loop through all the color points
		for (int index = 0; index < m_iColorPoints; index ++) {

			// Update the absolute starting position for this point
			m_ColorPoints[index].StartPos = m_ColorArea.top + int(m_ColorPoints[index].PosPercent * height);

			// Update the color information for the previous point
			if (index > 0) {			
				m_ColorPoints[index-1].EndPos = m_ColorPoints[index].StartPos;
				
				// Assign color increments to the previous color point
				int height = m_ColorPoints[index-1].EndPos - m_ColorPoints[index-1].StartPos;
				m_ColorPoints[index-1].RedInc = (m_ColorPoints[index].StartRed - m_ColorPoints[index-1].StartRed) / ((float)height);
				m_ColorPoints[index-1].GreenInc = (m_ColorPoints[index].StartGreen - m_ColorPoints[index-1].StartGreen) / ((float)height);
				m_ColorPoints[index-1].BlueInc = (m_ColorPoints[index].StartBlue - m_ColorPoints[index-1].StartBlue) / ((float)height);
			}
		}

		m_ColorPoints[index-1].RedInc		= 0;
		m_ColorPoints[index-1].GreenInc	= 0;
		m_ColorPoints[index-1].BlueInc	= 0;
		m_ColorPoints[index-1].EndPos		= m_ColorArea.bottom;
	}

	// Repaint the color bar
	Paint_DIB ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Free_Marker_Bitmap
//
void
ColorBarClass::Free_Marker_Bitmap (void)
{
	if (m_KeyFrameDIB != NULL) {
		::DeleteObject (m_KeyFrameDIB);
		m_KeyFrameDIB = NULL;
		m_pKeyFrameBits = NULL;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Load_Key_Frame_BMP
//
void
ColorBarClass::Load_Key_Frame_BMP (void)
{
	Free_Marker_Bitmap ();

	//
	//	Load the appropriate BMP based on the barstyle
	//
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	HBITMAP hbmp = NULL;
	if (style & CBRS_HORZ) {
		hbmp = ::LoadBitmap (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDB_KEYFRAME_V));
	} else {
		hbmp = ::LoadBitmap (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDB_KEYFRAME_H));
	}

	//
	//	Get the dimensions of the BMP
	//
	BITMAP bmp_info = { 0 };
	::GetObject (hbmp, sizeof (BITMAP), (LPVOID)&bmp_info);
	m_iMarkerWidth = bmp_info.bmWidth;
	m_iMarkerHeight = bmp_info.bmHeight;


	// Set-up the fields of the BITMAPINFOHEADER
	BITMAPINFOHEADER bitmap_info;
	bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth = m_iMarkerWidth;
	bitmap_info.biHeight = -m_iMarkerHeight; // Top-down DIB uses negative height
	bitmap_info.biPlanes = 1;
	bitmap_info.biBitCount = 24;
	bitmap_info.biCompression = BI_RGB;
	bitmap_info.biSizeImage = ((m_iMarkerWidth * m_iMarkerHeight) * 3);
	bitmap_info.biXPelsPerMeter = 0;
	bitmap_info.biYPelsPerMeter = 0;
	bitmap_info.biClrUsed = 0;
	bitmap_info.biClrImportant = 0;

	// Get a temporary screen DC
	HDC hscreen_dc = ::GetDC (NULL);

	// Create a bitmap that we can access the bits directly of
	m_KeyFrameDIB = ::CreateDIBSection (hscreen_dc,
													(const BITMAPINFO *)&bitmap_info,
													DIB_RGB_COLORS,
													(void **)&m_pKeyFrameBits,
													NULL,
													0L);

	// Release our temporary screen DC
	::ReleaseDC (NULL, hscreen_dc);

	// Initialize 2 temp DCs so we can copy from the BMP to the DIB section
	HDC htemp_dc = ::CreateCompatibleDC (NULL);
	HBITMAP hold_bmp1 = (HBITMAP)::SelectObject (m_hMemDC, m_KeyFrameDIB);
	HBITMAP hold_bmp2 = (HBITMAP)::SelectObject (htemp_dc, hbmp);

	// Copy the marker from the BMP to the DIB section
	::BitBlt (m_hMemDC, 0, 0, m_iMarkerWidth, m_iMarkerHeight, htemp_dc, 0, 0, SRCCOPY);
	
	//	Restore the DCs
	::SelectObject (m_hMemDC, hold_bmp1);
	::SelectObject (htemp_dc, hold_bmp2);
	::DeleteDC (htemp_dc);
	::DeleteObject (hbmp);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Key_Frame
//
void
ColorBarClass::Paint_Key_Frame (int x_pos, int y_pos)
{
	// Window's bitmaps are DWORD aligned, so make sure
	// we take that into account.
	int alignment_offset = (m_iMarkerWidth * 3) % 4;
	alignment_offset = (alignment_offset != 0) ? (4 - alignment_offset) : 0;
	int marker_scanline = (m_iMarkerWidth * 3) + alignment_offset;
	int width_in_bytes = m_iMarkerWidth * 3;

	if ((m_pBits != NULL) && (m_pKeyFrameBits != NULL)) {		
		int dest_index = (m_iScanlineSize * y_pos) + (x_pos * 3);
		int src_index = 0;

		//
		//	'Blit' the image from the source buffer to the dest buffer
		//
		for (int scanline = 0; scanline < m_iMarkerHeight; scanline ++) {			
			for (int pixel = 0; pixel < m_iMarkerWidth; pixel ++) {
				BYTE blue	= m_pKeyFrameBits[src_index ++];
				BYTE green	= m_pKeyFrameBits[src_index ++];
				BYTE red		= m_pKeyFrameBits[src_index ++];
				
				if (blue == 255 && green == 0 && red == 255) {
					dest_index += 3;
				} else {
					m_pBits[dest_index ++] = blue;
					m_pBits[dest_index ++] = green;
					m_pBits[dest_index ++] = red;
				}
			}

			// Skip to the next scanline
			dest_index += (m_iScanlineSize - width_in_bytes);
			src_index += alignment_offset;
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Marker_From_Point
//
int
ColorBarClass::Marker_From_Point (CPoint point)
{
	int marker_index = -1;

	//
	//	Setup the data we need for the search
	//
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	int accept_dist = 0;
	int position = 0;
	if (style & CBRS_HORZ) {
		position = point.x;// - m_ColorArea.left;
		accept_dist = (m_iMarkerWidth + 1) >> 1;
	} else {
		position = point.y;// - m_ColorArea.top;
		accept_dist = (m_iMarkerHeight + 1) >> 1;
	}	
	
	//
	//	Search through all the points, trying to find the closest
	//
	int closest = -1;
	int closest_dist = 1024;
	for (int index = 0; index < m_iColorPoints; index ++) {
		int delta = abs (m_ColorPoints[index].StartPos - position);
		if (delta < closest_dist) {
			closest_dist = delta;
			closest = index;
		}
	}

	//
	//	Did we find a marker at this point?
	//
	if (closest_dist < accept_dist) {
		marker_index = closest;
	}

	// Return the index of the marker at the specified point (-1 if not found).
	return marker_index;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonDown
//
void
ColorBarClass::OnLButtonDown
(
	UINT nFlags,
	CPoint point
)
{
	//
	//	Ensure we have the focus
	//
	if (::GetFocus () != m_hWnd) {
		::SetFocus (m_hWnd);
	}

	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_SHOW_FRAMES) {
		
		//
		//	Find the key frame the user clicked on
		//
		int marker_clicked = Marker_From_Point (point);
		if (marker_clicked != -1) {

			// Did the user select a new key?
			if (m_iCurrentKey != marker_clicked) {
				m_iCurrentKey = marker_clicked;

				// Repaint the window
				Repaint ();
			}

			// Should we prepare to move this marker?
			if (m_ColorPoints[m_iCurrentKey].flags & POINT_CAN_MOVE) {
				m_bMoving = true;
				m_bMoved = false;
				::SetCapture (m_hWnd);
			}

		} else {

			//
			//	Insert a new point at this location if the user has the Ctrl key down
			//
			if (::GetAsyncKeyState (VK_CONTROL) < 0) {
				Insert_Point (point);
				Send_Notification (CBRN_INSERTED_POINT, m_iCurrentKey);
			}

			m_bMoving = false;
			m_bMoved = false;
			::ReleaseCapture ();
		}
	} else if (style & CBRS_HAS_SEL) {
		
		Move_Selection (point);
//		CRect rect;
		//Get_Selection_Rectangle (rect);
		//if (rect.PtInRect (point)) {
			m_bMoving = true;
			m_bMoved = false;
			::SetCapture (m_hWnd);
		//}
	}

	CWnd::OnLButtonDown (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonUp
//
void
ColorBarClass::OnLButtonUp
(
	UINT nFlags,
	CPoint point
)
{
	if (m_bMoving) {
		::ReleaseCapture ();
		m_bMoving = false;

		//
		//	Let the parent know one of the points was dragged
		//
		if (m_bMoved) {
			Send_Notification (CBRN_MOVED_POINT, m_iCurrentKey);
		}
	}

	CWnd::OnLButtonUp (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnMouseMove
//
void
ColorBarClass::OnMouseMove
(
	UINT nFlags,
	CPoint point
)
{
	if (m_bMoving) {

		LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
		if (style & CBRS_SHOW_FRAMES)	{
			float min_percent = 0;
			float max_percent = 1.0F;

			//
			//	Determine the range this marker can move
			//
			if (m_iCurrentKey > 0) {
				min_percent = m_ColorPoints[m_iCurrentKey - 1].PosPercent + 0.01F;
			}

			if (m_iCurrentKey < (m_iColorPoints - 1)) {
				max_percent = m_ColorPoints[m_iCurrentKey + 1].PosPercent - 0.01F;
			}
			
			//
			//	Determine where the marker should be
			//
			float new_percent = 0;
			LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
			if (style & CBRS_HORZ) {
				new_percent = ((float)(point.x - m_ColorArea.left)) / ((float)m_ColorArea.Width ());
			} else {
				new_percent = ((float)(point.y - m_ColorArea.top)) / ((float)m_ColorArea.Height ());
			}

			//
			//	Ensure the marker is in bounds
			//
			new_percent = max (min_percent, new_percent);
			new_percent = min (max_percent, new_percent);

			//
			//	Move the marker
			//
			m_ColorPoints[m_iCurrentKey].PosPercent = new_percent;
			Update_Point_Info ();

			//
			//	Repaint the screen
			//
			HDC hwnd_dc = ::GetDC (m_hWnd);
			Paint_Screen (hwnd_dc);
			::ReleaseDC (m_hWnd, hwnd_dc);
			
			//
			//	Notify the parent window that the user dragged
			// one of the keyframes
			//
			Send_Notification (CBRN_MOVING_POINT, m_iCurrentKey);
			m_bMoved = true;

		} else if (style & CBRS_HAS_SEL) {
			Move_Selection (point);
		}
	}

	CWnd::OnMouseMove (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Send_Notification
//
LRESULT
ColorBarClass::Send_Notification (int code, int key)
{
	//
	//	Fill in the nofitication structure
	//
	LONG id = ::GetWindowLong (m_hWnd, GWL_ID);
	CBR_NMHDR notify_hdr = { 0 };
	notify_hdr.hdr.hwndFrom = m_hWnd;
	notify_hdr.hdr.idFrom = id;
	notify_hdr.hdr.code = code;
	notify_hdr.key_index = key;
	notify_hdr.red = m_ColorPoints[key].StartRed;
	notify_hdr.green = m_ColorPoints[key].StartGreen;
	notify_hdr.blue = m_ColorPoints[key].StartBlue;
	notify_hdr.position = m_MinPos + (m_ColorPoints[key].PosPercent * (m_MaxPos - m_MinPos));

	//
	//	Send the notification to the parent window
	//
	return ::SendMessage (::GetParent (m_hWnd), WM_NOTIFY, id, (LPARAM)&notify_hdr);
}

/////////////////////////////////////////////////////////////////////////////
//
// OnKillFocus
//
void
ColorBarClass::OnKillFocus (CWnd *pNewWnd) 
{
	// Force a repaint
	Repaint ();
	
	CWnd::OnKillFocus (pNewWnd);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSetFocus
//
void
ColorBarClass::OnSetFocus (CWnd *pOldWnd) 
{
	// Force a repaint
	Repaint ();

	CWnd::OnSetFocus(pOldWnd);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnKeyDown
//
void
ColorBarClass::OnKeyDown
(
	UINT nChar,
	UINT nRepCnt,
	UINT nFlags
)
{
	if ((nChar == VK_DELETE) && (::GetFocus () == m_hWnd)) {		

		//
		//	Notify the parent window that we are deleting a key
		// and ask them if its OK to contiue
		//
		if (Send_Notification (CBRN_DEL_POINT, m_iCurrentKey) != STOP_EVENT) {
			
			// Go ahead and delete the point
			Delete_Point (m_iCurrentKey);			
			Send_Notification (CBRN_DELETED_POINT, m_iCurrentKey);
		}
	}/* else if (::GetFocus () == m_hWnd) {
		if (nChar == VK_RIGHT) && 
	}*/
	
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonDblClk
//
void
ColorBarClass::OnLButtonDblClk
(
	UINT nFlags,
	CPoint point
) 
{
	int marker_clicked = Marker_From_Point (point);
	if (marker_clicked != -1) {
		
		//
		//	Notify the parent window that the user double-clicked
		// one of the keyframes
		//		
		Send_Notification (CBRN_DBLCLK_POINT, marker_clicked);
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Selection_Rectangle
//
void
ColorBarClass::Get_Selection_Rectangle (CRect &rect)
{
	float pos_percent = ((float)(m_SelectionPos - m_MinPos)) / ((float)(m_MaxPos - m_MinPos));
	rect = m_ColorArea;

	//
	//	Determine the bounding rectangle for the selection
	//
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_HORZ) {
		rect.left	= (m_ColorArea.left + int(m_ColorArea.Width () * pos_percent)) - 3;
		rect.right	= rect.left + 6;

		if (style & CBRS_FRAME_MASK) {
			rect.top --;
			rect.bottom ++;
		}
	} else {
		rect.top		= (m_ColorArea.top + int(m_ColorArea.Height () * pos_percent)) - 3;
		rect.bottom	= rect.top + 6;

		if (style & CBRS_FRAME_MASK) {
			rect.left --;
			rect.right ++;
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Selection_Pos
//
void
ColorBarClass::Set_Selection_Pos (float pos)
{
	// Ensure the new position is in bounds
	pos = max (pos, m_MinPos);
	pos = min (pos, m_MaxPos);

	// Move the selection
	Move_Selection (pos, false);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Move_Selection
//
void
ColorBarClass::Move_Selection (CPoint point, bool send_notify)
{
	//
	//	Determine the selection's new position
	//
	float percent = 0;
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (style & CBRS_HORZ) {
		percent = (((float)(point.x - m_ColorArea.left)) / ((float)m_ColorArea.Width ()));
	} else {
		percent = (((float)(point.y - m_ColorArea.top)) / ((float)m_ColorArea.Height ()));				
	}
	float new_pos = m_MinPos + (percent * (m_MaxPos - m_MinPos));			
	new_pos = max (new_pos, m_MinPos);
	new_pos = min (new_pos, m_MaxPos);

	// Do the actual move
	Move_Selection (new_pos, send_notify);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Move_Selection
//
void
ColorBarClass::Move_Selection (float new_pos, bool send_notify)
{
	if (new_pos != m_SelectionPos) {
		m_SelectionPos = new_pos;
		
		//
		//	Notify the parent window that the user changed the selection
		//
		if (send_notify) {
			LONG id = ::GetWindowLong (m_hWnd, GWL_ID);
			CBR_NMHDR notify_hdr = { 0 };
			notify_hdr.hdr.hwndFrom = m_hWnd;
			notify_hdr.hdr.idFrom = id;
			notify_hdr.hdr.code = CBRN_SEL_CHANGED;
			notify_hdr.position = m_SelectionPos;
			
			// Determine the color are this position
			Get_Color (m_SelectionPos,
							&notify_hdr.red,
							&notify_hdr.green,
							&notify_hdr.blue);

			//
			//	Send the notification to the parent
			//
			::SendMessage (::GetParent (m_hWnd),
								WM_NOTIFY,
								id,
								(LPARAM)&notify_hdr);
		}
		
		// Repaint the color bar with the new selection rectangle
		Paint_DIB ();	

		//
		//	Repaint the screen
		//
		HDC hwnd_dc = ::GetDC (m_hWnd);
		Paint_Screen (hwnd_dc);
		::ReleaseDC (m_hWnd, hwnd_dc);
	}
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Color
//
void
ColorBarClass::Get_Color
(
	float position,
	float *red,
	float *green,
	float *blue
)
{
	//
	//	Index into the color bitmap at the correct location...
	//
	LONG style = ::GetWindowLong (m_hWnd, GWL_STYLE);
	int pixel_pos = 0;
	if (style & CBRS_HORZ) {
		float percent = m_MinPos + (position - m_MinPos) / (m_MaxPos - m_MinPos);
		pixel_pos = int(m_ColorArea.left + (percent * (float)m_ColorArea.Width ()));

		/*int bmp_index = ((m_ColorArea.top + 1) * m_iScanlineSize) + (x * 3);
		(*blue)	= m_pBits[bmp_index];
		(*green) = m_pBits[bmp_index+1];
		(*red)	= m_pBits[bmp_index+2];*/
	} else {
		float percent = m_MinPos + (position - m_MinPos) / (m_MaxPos - m_MinPos);
		pixel_pos = int(m_ColorArea.top + (percent * (float)m_ColorArea.Height ()));

		/*int bmp_index = (y * m_iScanlineSize) + (m_ColorArea.left * 3);
		(*blue)	= m_pBits[bmp_index];
		(*green) = m_pBits[bmp_index+1];
		(*red)	= m_pBits[bmp_index+2];*/
	}	

	//
	//	Find the index where this point should be inserted
	//	
	bool found = false;
	int key_index = 0;
	for (int index = 0; (index < m_iColorPoints) && !found; index ++) {
		if ((pixel_pos >= m_ColorPoints[index].StartPos) &&
			 (pixel_pos <= m_ColorPoints[index].EndPos)) {
			key_index = index;
			found = true;
		}
	}

	if (found) {
		int start_pos = m_ColorPoints[key_index].StartPos;
		int end_pos = m_ColorPoints[key_index].EndPos;

		//float ticks = pixel_pos - start_pos) / float(end_pos - start_pos);
		float ticks = float(pixel_pos) - float(m_ColorPoints[key_index].StartPos);

		(*red) = m_ColorPoints[key_index].StartRed;
		(*green) = m_ColorPoints[key_index].StartGreen;
		(*blue) = m_ColorPoints[key_index].StartBlue;

		(*red) += ticks * m_ColorPoints[key_index].RedInc;
		(*green) += ticks * m_ColorPoints[key_index].GreenInc;
		(*blue) += ticks * m_ColorPoints[key_index].BlueInc;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Clear_Points
//
void
ColorBarClass::Clear_Points (void)
{
	// Reset the point count
	m_iColorPoints = 1;

	// Rebuild the color information
	Update_Point_Info ();

	// Force the window to be repainted
	Repaint ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_User_Data
//
bool
ColorBarClass::Set_User_Data (int index, DWORD data)
{
	bool retval = false;
	if ((index >= 0) && (index < m_iColorPoints)) {		
		m_ColorPoints[index].user_data = data;
		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_User_Data
//
DWORD
ColorBarClass::Get_User_Data (int index)
{
	DWORD data = 0;
	if ((index >= 0) && (index < m_iColorPoints)) {		
		data = m_ColorPoints[index].user_data;
	}

	return data;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Graph_Percent
//
bool
ColorBarClass::Set_Graph_Percent (int index, float percent)
{
	bool retval = false;
	if ((index >= 0) && (index < m_iColorPoints)) {
		m_ColorPoints[index].StartGraphPercent = percent;

		// Rebuild the color information
		Update_Point_Info ();

		// Force the window to be repainted
		Repaint ();

		retval = true;
	}

	return retval;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Graph_Percent
//
float
ColorBarClass::Get_Graph_Percent (int index)
{
	float percent = 0;
	if ((index >= 0) && (index < m_iColorPoints)) {
		percent = m_ColorPoints[index].StartGraphPercent;
	}

	return percent;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Redraw
//
void
ColorBarClass::Set_Redraw (bool redraw)
{
	m_bRedraw = redraw;
	if (m_bRedraw) {
		UpdateWindow ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Repaint
//
void
ColorBarClass::Repaint (void)
{
	InvalidateRect (NULL, FALSE);
	if (m_bRedraw) {		
		UpdateWindow ();
	}

	return ;
}
