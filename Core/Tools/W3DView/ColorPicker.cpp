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
// ColorPicker.cpp : implementation file
//
//

#include "stdafx.h"
#include "colorpicker.h"
#include "colorpickerdialogclass.h"
#include <math.h>
#include "utils.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//extern HINSTANCE _hinstance;

/////////////////////////////////////////////////////////////////////////////
//
// ColorPickerClass
//
ColorPickerClass::ColorPickerClass (void)
	: m_hBitmap (NULL),
	  m_iWidth (0),
	  m_iHeight (0),
	  m_CurrentPoint (0, 0),
	  m_CurrentColor (0),
	  m_bSelecting (false),
	  m_pBits (NULL),
	  m_hMemDC (NULL),
	  m_CurrentHue (0),
	  CWnd ()
{
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~ColorPickerClass
//
ColorPickerClass::~ColorPickerClass (void)
{
	if (m_hMemDC != NULL) {
		::DeleteObject (m_hMemDC);
		m_hMemDC = NULL;
	}
		
	Free_Bitmap ();
	return ;
}


BEGIN_MESSAGE_MAP(ColorPickerClass, CWnd)
	//{{AFX_MSG_MAP(ColorPickerClass)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnPaint
//
void
ColorPickerClass::OnPaint (void)
{
	CPaintDC dc (this);

	if (m_hMemDC != NULL) {
		
		HBITMAP hold_bmp = (HBITMAP)::SelectObject (m_hMemDC, m_hBitmap);

		CRect rect;
		GetClientRect (&rect);
		::BitBlt (dc, 0, 0, rect.Width (), rect.Height (), m_hMemDC, 0, 0, SRCCOPY);

		::SelectObject (m_hMemDC, hold_bmp);
		Paint_Marker ();
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////

LRESULT WINAPI fnColorPickerProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

/////////////////////////////////////////////////////////////////////////////
//
// RegisterColorPicker
//
void
RegisterColorPicker (HINSTANCE hinst)
{
	// Has the class already been registered?
	WNDCLASS wndclass = { 0 };
	if (::GetClassInfo (hinst, "WWCOLORPICKER", &wndclass) == FALSE) {
		
		wndclass.style = CS_GLOBALCLASS | CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		wndclass.lpfnWndProc = fnColorPickerProc;
		wndclass.hInstance = hinst;
		wndclass.hbrBackground = (HBRUSH)COLOR_3DFACE + 1;
		wndclass.hCursor = ::LoadCursor (NULL, IDC_ARROW);
		wndclass.lpszClassName = "WWCOLORPICKER";
		
		// Let the windows manager know about this global class
		::RegisterClass (&wndclass);
	}

	return ;
}




/////////////////////////////////////////////////////////////////////////////
//
// fnColorPickerProc
//
LRESULT WINAPI
fnColorPickerProc
(
	HWND hwnd,
	UINT message,
	WPARAM wparam,
	LPARAM lparam
)
{
	//static ColorPickerClass *pwnd = NULL;
	//static bool bcreated = false;

	switch (message)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT pcreate_info = (LPCREATESTRUCT)lparam;
			if (pcreate_info != NULL) {

				// Should we create a new class manager for this window?
				ColorPickerClass *pwnd = (ColorPickerClass *)pcreate_info->lpCreateParams;
				BOOL created = FALSE;
				if (pwnd == NULL) {
					pwnd = new ColorPickerClass;
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
			/*pwnd->Detach ();

			WNDPROC *pOldWndProc = pwnd->GetSuperWndProcAddr ();
			if (pOldWndProc) {
				::SetWindowLong (hwnd, GWL_WNDPROC, (DWORD)(*pOldWndProc));
				(*pOldWndProc) = NULL;
			}

			if (bcreated) {
				delete pwnd;
				pwnd = NULL;
			}*/

			// Get the creation information from the window handle
			ColorPickerClass *pwnd = (ColorPickerClass *)::GetProp (hwnd, "CLASSPOINTER");
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
/////////////////////////////////////////////////////////////////////////////
int
ColorPickerClass::OnCreate (LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_hMemDC = 	::CreateCompatibleDC (NULL);
	Create_Bitmap ();
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ColorPickerClass::Create
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
	HWND hwnd = ::CreateWindow ("WWCOLORPICKER",
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
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Create_Bitmap (void)
{
	// Start fresh
	Free_Bitmap ();

	CRect rect;
	GetClientRect (&rect);
	m_iWidth = rect.Width ();
	m_iHeight = rect.Height ();

	// Set-up the fields of the BITMAPINFOHEADER
	BITMAPINFOHEADER bitmap_info;
	bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth = m_iWidth;
	bitmap_info.biHeight = -m_iHeight; // Top-down DIB uses negative height
	bitmap_info.biPlanes = 1;
	bitmap_info.biBitCount = 24;
	bitmap_info.biCompression = BI_RGB;
	bitmap_info.biSizeImage = ((m_iWidth * m_iHeight) * 3);
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

	// Paint the color range into this bitmap
	Paint_DIB (m_iWidth, m_iHeight, m_pBits);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Rect
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Fill_Rect
(
	UCHAR *pbits,
	const RECT &rect,
	COLORREF color,
	int scanline_size
)
{
	UCHAR red = GetRValue (color);
	UCHAR green = GetRValue (color);
	UCHAR blue = GetRValue (color);

	for (int irow = rect.top; irow < rect.bottom; irow ++) {				
		int index = irow * scanline_size;
		index += (rect.left * 3);

		for (int col = rect.left; col < rect.right; col ++) {
			pbits[index++] = blue;
			pbits[index++] = green;
			pbits[index++] = red;
		}
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Color_From_Point
//
/////////////////////////////////////////////////////////////////////////////
COLORREF
ColorPickerClass::Color_From_Point
(
	int x,
	int y
)
{
	COLORREF color = RGB (0,0,0);

	// x,y location inside boundaries?
	if ((x >= 0) && (x < m_iWidth) &&
		 (y >= 0) && (y < m_iHeight)) {

		// Window's bitmaps are DWORD aligned, so make sure
		// we take that into account.
		int alignment_offset = (m_iWidth * 3) % 4;
		alignment_offset = (alignment_offset != 0) ? (4 - alignment_offset) : 0;
		int scanline_size = (m_iWidth * 3) + alignment_offset;	
		
		// Read the color value straight from the BMP
		int index = (scanline_size * y) + (x * 3);
		int blue = m_pBits[index];
		int green = m_pBits[index + 1];
		int red = m_pBits[index + 2];

		// Turn the individual compenents into a color
		color = RGB (red, green, blue);
		
		RECT rect;
		Calc_Display_Rect (rect);
		int width = rect.right-rect.left;
		m_CurrentHue = ((float)x) / ((float)width);
	}

	// Return the color
	return color;
}


/////////////////////////////////////////////////////////////////////////////
//
// Polar_To_Rect
//
/////////////////////////////////////////////////////////////////////////////
void
Polar_To_Rect (float radius, float angle, float &x, float &y)
{
	x = radius * float(cos(angle));
	y = radius * float(sin(angle));
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Rect_To_Polar
//
/////////////////////////////////////////////////////////////////////////////
void
Rect_To_Polar (float x, float y, float &radius, float &angle)
{
	if (x == 0 && y == 0) {
		radius = 0;
		angle = 0;
	} else {
		radius = (float)::sqrt ((x * x) + (y * y));
		angle = (float)::atan2 (y, x);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// RGB_to_Hue
//
/////////////////////////////////////////////////////////////////////////////
void
RGB_to_Hue (int red_val, int green_val, int blue_val, float &hue, float &value)
{
	float red_radius = (float)red_val;
	float green_radius = (float)green_val;
	float blue_radius = (float)blue_val;

	float red_angle = 0;
	float green_angle = (3.1415926535F * 2.0F) / 3.0F;
	float blue_angle = (3.1415926535F * 4.0F) / 3.0F;

	struct Color2D
	{
		float x;
		float y;
	};

	Color2D red = { 0 };
	Color2D green = { 0 };
	Color2D blue = { 0 };
	Color2D result = { 0 };

	::Polar_To_Rect (red_radius, red_angle, red.x, red.y);
	::Polar_To_Rect (green_radius, green_angle, green.x, green.y);
	::Polar_To_Rect (blue_radius, blue_angle, blue.x, blue.y);

	result.x = red.x + green.x + blue.x;
	result.y = red.y + green.y + blue.y;

	float hue_angle = 0;
	float hue_radius = 0;
	::Rect_To_Polar (result.x, result.y, hue_radius, hue_angle);
	
	hue = hue_angle / (3.14159265359F * 2.0F);
	if (hue < 0) {
		hue += 1;
	} else if (hue > 1) {
		hue -= 1;
	}

	value = (float)::fabs (hue_radius / 255);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Point_From_Color
//
/////////////////////////////////////////////////////////////////////////////
CPoint
ColorPickerClass::Point_From_Color (COLORREF color)
{
	// Window's bitmaps are DWORD aligned, so make sure
	// we take that into account.
	int alignment_offset = (m_iWidth * 3) % 4;
	alignment_offset = (alignment_offset != 0) ? (4 - alignment_offset) : 0;
	int scanline_size = (m_iWidth * 3) + alignment_offset;	

	int red = GetRValue (color);
	int green = GetGValue (color);
	int blue = GetBValue (color);

	float hue = 0;
	float value = 0;
	RGB_to_Hue (red, green, blue, hue, value);

	RECT rect;
	Calc_Display_Rect (rect);
	int width = rect.right-rect.left;
	int height = rect.bottom-rect.top;

	float whiteness = (float)min (min (red, green), blue);	
	float percent = whiteness / 255;
	float darkness = 0;

	//
	//	Determine what the 'darkness' of the hue should be
	//
	if (percent != 1) {
		float start_red = (red - whiteness) / (1 - percent);
		float start_green = (green - whiteness) / (1 - percent);
		float start_blue = (blue - whiteness) / (1 - percent);
		darkness = max (max (start_red, start_green), start_blue);
	}

	int x = int(width * hue);
	int y = ((255 - (int)darkness) * height) / 255;

	// Return the point
	return CPoint (x, y);
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_DIB
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Paint_DIB
(
	int width,
	int height,
	UCHAR *pbits
)
{
	// Window's bitmaps are DWORD aligned, so make sure
	// we take that into account.
	int alignment_offset = (width * 3) % 4;
	alignment_offset = (alignment_offset != 0) ? (4 - alignment_offset) : 0;
	int scanline_size = (width * 3) + alignment_offset;	

	//
	//	Paint the border (if any)
	//
	CRect rect (0, 0, width, height);	
	LONG lstyle = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if (lstyle & CPS_SUNKEN) {
		::Draw_Sunken_Rect (pbits, rect, scanline_size);
		rect.DeflateRect (1, 1);
	} else if (lstyle & CPS_RAISED) {
		::Draw_Raised_Rect (pbits, rect, scanline_size);
		rect.DeflateRect (1, 1);
	} else if (lstyle & WS_BORDER) {
		::Frame_Rect (pbits, rect, RGB (255, 255, 255), scanline_size);
		rect.DeflateRect (1, 1);
	}

	// Recalc the width and height (it could of changed)
	width = rect.Width ();
	height = rect.Height ();
	
	// Build an array of column indicies where we will switch color
	// components...
	int col_remainder = (width % 6);
	int channel_switch_cols[6];
	for (int channel = 0; channel < 6; channel ++) {
		
		// Each column has at least X/6 pixels
		channel_switch_cols[channel] = width / 6;

		// Do we need to add a remainder to this col?
		// (Occurs when the width isn't evenly divisible by 6)
		if (col_remainder > 0) {
			channel_switch_cols[channel] += 1;
			col_remainder --;
		}

		// Add the previous column's switching index
		if (channel > 0) {
			channel_switch_cols[channel] += channel_switch_cols[channel - 1];
		}
	}

	// Start with max red
	float red = 255.0F;
	float green = 0;
	float blue = 0;

	// Calculate some starting channel variables
	int curr_channel = 0;
	float curr_inc = (255.0F / ((float)channel_switch_cols[0]));
	float *pcurr_component = &green;

	//
	//	Paint the image
	//
	for (int icol = rect.left; icol < rect.right; icol ++) {
		
		// Determine how much to 'darken' the current hue by for each row (goes to black)
		float red_dec = -((float)(red) / (float)(height-1));
		float green_dec = -((float)(green) / (float)(height-1));
		float blue_dec = -((float)(blue) / (float)(height-1));

		// Start with the normal hue color
		float curr_red = red;
		float curr_green = green;
		float curr_blue = blue;
		
		// Paint all pixels in this row
		int bitmap_index = (icol * 3) + (rect.left * scanline_size);
		for (int irow = rect.top; irow < rect.bottom; irow ++) {
										
			// Put these values into the bitmap
			pbits[bitmap_index]		= UCHAR(((int)curr_blue) & 0xFF);
			pbits[bitmap_index + 1] = UCHAR(((int)curr_green) & 0xFF);
			pbits[bitmap_index + 2]	= UCHAR(((int)curr_red) & 0xFF);				

			// Determine the current red, green, and blue values
			curr_red = curr_red + red_dec;
			curr_green = curr_green + green_dec;
			curr_blue = curr_blue + blue_dec;

			// Increment our offset into the bitmap
			bitmap_index += scanline_size;
		}

		// Is it time to switch to a new color channel?
		if ((icol - rect.left) == channel_switch_cols[curr_channel]) {
			
			// Recompute values for the new channel
			curr_channel ++;
			curr_inc = (255.0F / ((float)(channel_switch_cols[curr_channel] - channel_switch_cols[curr_channel-1])));

			// Which channel are we painting?
			if (curr_channel == 1) {
				pcurr_component = &red;
				curr_inc = -curr_inc;
			} else if (curr_channel == 2) {
				pcurr_component = &blue;
			} else if (curr_channel == 3) {
				pcurr_component = &green;
				curr_inc = -curr_inc;
			} else if (curr_channel == 4) {
				pcurr_component = &red;
			} else if (curr_channel == 5) {
				pcurr_component = &blue;
				curr_inc = -curr_inc;
			}
		}

		// Increment the current color component
		(*pcurr_component) = (*pcurr_component) + curr_inc;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Free_Bitmap
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Free_Bitmap (void)
{
	if (m_hBitmap != NULL) {
		::DeleteObject (m_hBitmap);
		m_hBitmap = NULL;
		m_pBits = NULL;
	}

	m_iWidth = 0;
	m_iHeight = 0;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnSize
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::OnSize
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

	// Determine the new point based on the current color
	//m_CurrentColor = RGB (128, 222, 199);
	//m_CurrentPoint = Point_From_Color (m_CurrentColor);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnEraseBkgnd
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ColorPickerClass::OnEraseBkgnd (CDC * /*pDC*/)
{
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonDown
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::OnLButtonDown
(
	UINT nFlags,
	CPoint point
)
{
	SetCapture ();
	m_bSelecting = true;

	RECT rect;
	Calc_Display_Rect (rect);
	ClientToScreen (&rect);
	::ClipCursor (&rect);

	Erase_Marker ();
	
	m_CurrentPoint = point;
	m_CurrentColor = Color_From_Point (point.x, point.y);

	//
	//	Notify the parent window that the user double-clicked
	// one of the keyframes
	//
	LONG id = ::GetWindowLong (m_hWnd, GWL_ID);
	CP_NMHDR notify_hdr = { 0 };
	notify_hdr.hdr.hwndFrom = m_hWnd;
	notify_hdr.hdr.idFrom = id;
	notify_hdr.hdr.code = CPN_COLORCHANGE;
	notify_hdr.red		= GetRValue (m_CurrentColor);
	notify_hdr.green	= GetGValue (m_CurrentColor);
	notify_hdr.blue	= GetBValue (m_CurrentColor);
	notify_hdr.hue		= m_CurrentHue;
	::SendMessage (::GetParent (m_hWnd),
						WM_NOTIFY,
						id,
						(LPARAM)&notify_hdr);

	Paint_Marker ();

	// Allow the base class to process this message
	CWnd::OnLButtonDown (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnLButtonUp
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::OnLButtonUp
(
	UINT nFlags,
	CPoint point
)
{
	if (m_bSelecting) {
		::ClipCursor (NULL);
		ReleaseCapture ();
		m_bSelecting = false;		
	}

	// Allow the base class to process this message
	CWnd::OnLButtonUp (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnMouseMove
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::OnMouseMove
(
	UINT nFlags,
	CPoint point
)
{
	if (m_bSelecting) {
		Erase_Marker ();

		m_CurrentPoint = point;
		m_CurrentColor = Color_From_Point (point.x, point.y);		

		//
		//	Notify the parent window that the user double-clicked
		// one of the keyframes
		//
		LONG id = ::GetWindowLong (m_hWnd, GWL_ID);
		CP_NMHDR notify_hdr = { 0 };
		notify_hdr.hdr.hwndFrom = m_hWnd;
		notify_hdr.hdr.idFrom = id;
		notify_hdr.hdr.code = CPN_COLORCHANGE;
		notify_hdr.red		= GetRValue (m_CurrentColor);
		notify_hdr.green	= GetGValue (m_CurrentColor);
		notify_hdr.blue	= GetBValue (m_CurrentColor);
		notify_hdr.hue		= m_CurrentHue;
		::SendMessage (::GetParent (m_hWnd),
							WM_NOTIFY,
							id,
							(LPARAM)&notify_hdr);

		Paint_Marker ();
	}

	// Allow the base class to process this message
	CWnd::OnMouseMove (nFlags, point);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Calc_Display_Rect
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Calc_Display_Rect (RECT &rect)
{
	GetClientRect (&rect);

	LONG lstyle = ::GetWindowLong (m_hWnd, GWL_STYLE);
	if ((lstyle & CPS_SUNKEN) || (lstyle & CPS_RAISED) || (lstyle & WS_BORDER)) {
		rect.left += 1;
		rect.right -= 1;
		rect.top += 1;
		rect.bottom -= 1;
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Erase_Marker
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Erase_Marker (void)
{
	HDC hdc = ::GetDC (m_hWnd);
	if (m_hMemDC != NULL) {

		HBITMAP hold_bmp = (HBITMAP)::SelectObject (m_hMemDC, m_hBitmap);

		CRect rect;
		GetClientRect (&rect);
		::BitBlt (hdc,
					 m_CurrentPoint.x - 5,
					 m_CurrentPoint.y - 5,
					 11,
					 11,
					 m_hMemDC,
					 m_CurrentPoint.x - 5,
					 m_CurrentPoint.y - 5,
					 SRCCOPY);

		::SelectObject (m_hMemDC, hold_bmp);
	}

	::ReleaseDC (m_hWnd, hdc);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Marker
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Paint_Marker (void)
{
	HDC hdc = ::GetDC (m_hWnd);
	if (m_hMemDC != NULL) {

		HBITMAP hmarker_bmp = ::LoadBitmap (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDB_MARKER));
		HBITMAP hold_bmp = (HBITMAP)::SelectObject (m_hMemDC, hmarker_bmp);

		CRect rect;
		GetClientRect (&rect);
		::BitBlt (hdc,
					 m_CurrentPoint.x - 5,
					 m_CurrentPoint.y - 5,
					 11,
					 11,
					 m_hMemDC,
					 0,
					 0,
					 SRCINVERT);

		::SelectObject (m_hMemDC, hold_bmp);
		::DeleteObject (hmarker_bmp);
	}

	::ReleaseDC (m_hWnd, hdc);	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Select_Color
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Select_Color (int red, int green, int blue)
{
	m_CurrentColor = RGB (red, green, blue);
	m_CurrentPoint = Point_From_Color (m_CurrentColor);
	m_CurrentColor = Color_From_Point (m_CurrentPoint.x, m_CurrentPoint.y);

	// Refresh the window
	InvalidateRect (NULL, FALSE);
	UpdateWindow ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Get_Current_Color
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerClass::Get_Current_Color (int *red, int *green, int *blue)
{
	(*red)	= GetRValue (m_CurrentColor);
	(*green)	= GetGValue (m_CurrentColor);
	(*blue)	= GetBValue (m_CurrentColor);
	return ;
}
