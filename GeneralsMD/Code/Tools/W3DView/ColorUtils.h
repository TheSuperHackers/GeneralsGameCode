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

//////////////////////////////////////////////////////////////////////////////////////////
//
//	ColorUtils.H
//
//

#ifndef __COLORUTILS_H
#define __COLORUTILS_H

/////////////////////////////////////////////////////////////////////////////
//	Callbacks
/////////////////////////////////////////////////////////////////////////////
typedef void (*WWCTRL_COLORCALLBACK)(int,int,int,void*);

/////////////////////////////////////////////////////////////////////////////
// Prototypes
/////////////////////////////////////////////////////////////////////////////
void		Frame_Rect (UCHAR *pbits, const RECT &rect, COLORREF color, int scanline_size);
void		Draw_Vert_Line (UCHAR *pbits, int x, int y, int len, COLORREF color, int scanline_size);
void		Draw_Horz_Line (UCHAR *pbits, int x, int y, int len, COLORREF color, int scanline_size);
void		Draw_Sunken_Rect (UCHAR *pbits, const RECT &rect, int scanline_size);
void		Draw_Raised_Rect (UCHAR *pbits, const RECT &rect, int scanline_size);
BOOL		Show_Color_Picker (int *red, int *green, int *blue);
HWND		Create_Color_Picker_Form (HWND parent, int red, int green, int blue);
BOOL		Get_Form_Color (HWND form_wnd, int *red, int *green, int *blue);
BOOL		Set_Form_Color (HWND form_wnd, int red, int green, int blue);
BOOL		Set_Form_Original_Color (HWND form_wnd, int red, int green, int blue);
BOOL		Set_Update_Callback (HWND form_wnd, WWCTRL_COLORCALLBACK callback, void *arg=NULL);
void		RegisterColorPicker (HINSTANCE hinst);
void		RegisterColorBar (HINSTANCE hinst);


#endif //__COLORUTILS_H
