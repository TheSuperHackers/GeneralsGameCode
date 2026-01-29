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

// FILE: ColorDialog.cpp //////////////////////////////////////////////////////
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
// File name:  ColorDialog.cpp
//
// Created:    Colin Day, March 1999
//						 Colin Day, July 2001
//
// Desc:       Color picker dialog
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GUIEdit.h"
#include "resource.h"
#include "GUIEditColor.h"
#include "Properties.h"
#include "Common/Debug.h"

// DEFINES ////////////////////////////////////////////////////////////////////
#define MODE_RGB 0
#define MODE_HSV 1

///////////////////////////////////////////////////////////////////////////////
// PRIVATE TYPES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static RGBColorInt selectedColor;  // picked color
static Int mode = MODE_HSV;  // color selection mode
static ICoord2D displayPos;  // where to open window

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// rgbToHSV ===================================================================
// Converts the RGB colors passed in to HSV values
// RGB           colors ranges from 0 - 1
// hue                  ranges from 0 - 360
// saturation and value ranges from 0 - 1
// ============================================================================
HSVColorReal rgbToHSV( RGBColorReal rgbColor )
{
  Real max, min;  // max and min of rgb
  Real red, green, blue;  // rgb alias
  Real hue, saturation, value;  // hsv alias
  HSVColorReal hsvColor;

  red = rgbColor.red;
  green = rgbColor.green;
  blue = rgbColor.blue;

  // find the max and min of the rgb triplet
  max = red;
  if (green > max)
    max = green;
  if (blue > max)
    max = blue;

  min = red;
  if (green < red)
    min = green;
  if (blue < min)
    min = blue;

  // set the value
  value = max;

  // calculate saturation
  if (max != 0)
    saturation = (max - min) / max;
  else
    saturation = 0;  // saturation is 0 if all RGB are 0

  // calculate hue
  if (saturation == 0)  {
    hue = 0;  // hue is really undefined
  }
  else  {  // chromatic case, determine hue
    Real delta = max - min;

    if (red == max)
      hue = (green - blue) / delta;  // color between yellow and magenta
    else if (green == max)
      hue = 2 + (blue - red) / delta;  // color between cyan and yellow
    else if (blue == max)
      hue = 4 + (red - green) / delta;  // color between magenta and cyan

    hue = hue * 60;  //  convert hue to degrees
    if (hue < 0)
      hue += 360;  // make sure hue is non negative

  }

  // set and return an HSVColor
  hsvColor.hue        = hue;
  hsvColor.saturation = saturation;
  hsvColor.value      = value;

  // values aren't very nice to lets make them at least 1 ... isn't not
  // technically correct but it works nicer for light color dialog
  if (hsvColor.hue == 0.0f)
    hsvColor.hue = 1.0f;
  if (hsvColor.saturation == 0.0f)
    hsvColor.saturation += 0.01f;
  if (hsvColor.value == 0.0f)
    hsvColor.value      += 0.01f;

	// copy over alpha
	hsvColor.alpha = rgbColor.alpha;

  return hsvColor;

}

// hsvToRGB ===================================================================
// Converts the HSV colors passed in to RGB values
// RGB           colors ranges from 0 - 1
// hue                  ranges from 0 - 360
// saturation and value ranges from 0 - 1
// ============================================================================
RGBColorReal hsvToRGB( HSVColorReal hsvColor )
{
  Int i;
  Real f, p, q, t;
  Real red, green, blue;  // rgb alias
  Real hue, saturation, value;  // hsv alias
  RGBColorReal rgbColor;

  hue = hsvColor.hue;
  saturation = hsvColor.saturation;
  value = hsvColor.value;

  if( saturation == 0.0f )
	{
		// the colors is on the black and white center line
    if( hue == 0.0f )
		{  // achromatic color ... there is no hue
      red = green = blue = value;
    }
    else
		{
      DEBUG_LOG(( "HSVToRGB error, hue should be undefined" ));
    }

  }
  else
	{

    if( hue == 360.0f )
      hue = 0.0f;
    hue = hue / 60.0f;  // h is now in [0, 6)
    i = REAL_TO_INT_FLOOR(hue);  // largest int <= h
    f = hue - (Real) i;  // f is the fractional part of h
    p = value * (1.0f - saturation);
    q = value * (1.0f - (saturation * f));
    t = value * (1.0f - (saturation * (1.0f - f)));
    switch (i)  {
      case 0:
        red = value;
        green = t;
        blue = p;
        break;
      case 1:
        red = q;
        green = value;
        blue = p;
        break;
      case 2:
        red = p;
        green = value;
        blue = t;
        break;
      case 3:
        red = p;
        green = q;
        blue = value;
        break;
      case 4:
        red = t;
        green = p;
        blue = value;
        break;
      case 5:
        red = value;
        green = p;
        blue = q;
        break;
    }

  }

  // store and return and RGB color
  rgbColor.red   = red;
  rgbColor.green = green;
  rgbColor.blue  = blue;
  rgbColor.alpha = hsvColor.alpha;

  return rgbColor;

}

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////
BOOL CALLBACK SelectColorDlgProc( HWND hWnd, UINT uMsg,
                                  WPARAM wParam, LPARAM lParam );

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// SelectColor ================================================================
/** Bring up the color selection requestor.
	*
	* Returns:
	* Pointer to selected color
	* nullptr for canceled request
	*/
// ============================================================================
RGBColorInt *SelectColor( Int red, Int green, Int blue, Int alpha,
													Int mouseX, Int mouseY )
{

  selectedColor.red = red;
  selectedColor.green = green;
  selectedColor.blue = blue;
	selectedColor.alpha = alpha;

	displayPos.x = mouseX;
	displayPos.y = mouseY;

  if( DialogBox( TheEditor->getInstance(), (LPCTSTR)COLOR_SELECT_DIALOG,
								 TheEditor->getWindowHandle(), SelectColorDlgProc ) )
		return &selectedColor;
  else
    return nullptr;

}

// SelectColorDlgProc =========================================================
/** Dialog procedure for color selector dialog */
// ============================================================================
BOOL CALLBACK SelectColorDlgProc( HWND hWndDlg, UINT uMsg,
                                  WPARAM wParam, LPARAM lParam )
{
  static HWND hWndScrollColor1;  // red / hue
  static HWND hWndScrollColor2;  // green / saturation
  static HWND hWndScrollColor3;  // blue / value
	static HWND hWndScrollAlpha;	 // alpha scrollbar
  static HWND hWndColorBar1;  // red / hue
  static HWND hWndColorBar2;  // green / saturation
  static HWND hWndColorBar3;  // blue / value
  static HWND hWndPreview;

  switch (uMsg)  {

    // ------------------------------------------------------------------------
    case WM_INITDIALOG:
		{

      // save some window handles for later comparing during processing
      hWndScrollColor1 = GetDlgItem( hWndDlg, SCROLL_COLOR1 );
      hWndScrollColor2 = GetDlgItem( hWndDlg, SCROLL_COLOR2 );
      hWndScrollColor3 = GetDlgItem( hWndDlg, SCROLL_COLOR3 );
			hWndScrollAlpha	 = GetDlgItem( hWndDlg, SCROLL_ALPHA );
      hWndColorBar1    = GetDlgItem( hWndDlg, BUTTON_COLORBAR1 );
      hWndColorBar2    = GetDlgItem( hWndDlg, BUTTON_COLORBAR2 );
      hWndColorBar3    = GetDlgItem( hWndDlg, BUTTON_COLORBAR3 );
      hWndPreview = GetDlgItem (hWndDlg, BUTTON_PREVIEW);

      // init the scroll bars and labels to the current color
			if (mode == MODE_HSV)
			{
				RGBColorReal rgbColor;
				HSVColorReal hsvColor;

				rgbColor.red   = (Real) selectedColor.red   / 255.0f;
				rgbColor.green = (Real) selectedColor.green / 255.0f;
				rgbColor.blue  = (Real) selectedColor.blue  / 255.0f;
				rgbColor.alpha = (Real) selectedColor.alpha;
				hsvColor = rgbToHSV (rgbColor);
				hsvColor.saturation *= 100.0f;
				hsvColor.value      *= 100.0f;

				// init the HSV and intensity scroll bar extents
				SendMessage( hWndScrollColor1, SBM_SETRANGE, 1, 360 );
				SendMessage( hWndScrollColor2, SBM_SETRANGE, 1, 100 );
				SendMessage( hWndScrollColor3, SBM_SETRANGE, 1, 100 );
				SendMessage( hWndScrollAlpha, SBM_SETRANGE, 0, 255 );

				// set the scroll bars and labels
				SetScrollPos (hWndScrollColor1, SB_CTL, (Int) hsvColor.hue, TRUE);
				SetDlgItemInt (hWndDlg, LABEL_COLOR1,
											 (Int) hsvColor.hue, FALSE);
				SetScrollPos (hWndScrollColor2, SB_CTL, (Int) hsvColor.saturation, TRUE);
				SetDlgItemInt (hWndDlg, LABEL_COLOR2,
											 (Int) hsvColor.saturation, FALSE);
				SetScrollPos (hWndScrollColor3, SB_CTL, (Int) hsvColor.value, TRUE);
				SetDlgItemInt (hWndDlg, LABEL_COLOR3,
											 (Int) hsvColor.value, FALSE);
				SetScrollPos (hWndScrollAlpha, SB_CTL, (Int) hsvColor.alpha, TRUE);
				SetDlgItemInt (hWndDlg, LABEL_ALPHA,
											 (Int) hsvColor.alpha, FALSE);

      }
      else
			{

        // init the RGB and intensity scroll bar extents
        SendMessage( hWndScrollColor1, SBM_SETRANGE, 1, 255 );
        SendMessage( hWndScrollColor2, SBM_SETRANGE, 1, 255 );
        SendMessage( hWndScrollColor3, SBM_SETRANGE, 1, 255 );
				SendMessage( hWndScrollAlpha, SBM_SETRANGE, 0, 255 );
        SetScrollPos (hWndScrollColor1, SB_CTL, selectedColor.red, TRUE);
        SetDlgItemInt (hWndDlg, LABEL_COLOR1,
                       selectedColor.red, FALSE);
        SetScrollPos (hWndScrollColor2, SB_CTL, selectedColor.green, TRUE);
        SetDlgItemInt (hWndDlg, LABEL_COLOR2,
                       selectedColor.green, FALSE);
        SetScrollPos (hWndScrollColor3, SB_CTL, selectedColor.blue, TRUE);
        SetDlgItemInt (hWndDlg, LABEL_COLOR3,
                       selectedColor.blue, FALSE);
        SetScrollPos (hWndScrollAlpha, SB_CTL, selectedColor.alpha, TRUE);
        SetDlgItemInt (hWndDlg, LABEL_ALPHA,
                       selectedColor.alpha, FALSE);

      }

			//
			// move the window to the display position, but keep the whole
			// window on the screen
			//
			PositionWindowOnScreen( hWndDlg, displayPos.x, displayPos.y );

      return TRUE;

    }

    // ------------------------------------------------------------------------
    case WM_DRAWITEM:  {
      UINT idCtl = (UINT) wParam;             // control identifier
      LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; // item drawing
      HWND hWndControl;
      RECT rect;
      ICoord2D center;
      Int radius;

      // Get the area we have to draw in
      hWndControl = GetDlgItem (hWndDlg, idCtl);
      GetClientRect (hWndControl, &rect);
      center.x = (rect.right - rect.left) / 2;
      center.y = (rect.bottom - rect.top) / 2;

      // record radius we have to work with
      radius = (rect.right - rect.left) / 2;

      switch (idCtl)  {

        case BUTTON_PREVIEW:  {
          RGBColorReal rgbColor;
          HSVColorReal hsvColor;
          HBRUSH hBrushOld, hBrushNew;

          if (mode == MODE_RGB)  {
            rgbColor.red =   (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1,
                                                    nullptr, FALSE);
            rgbColor.green = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR2,
                                                    nullptr, FALSE);
            rgbColor.blue =  (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR3,
                                                    nullptr, FALSE);
          }
          else  {
            hsvColor.hue        = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1,
                                                         nullptr, FALSE);
            hsvColor.saturation = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR2,
                                                         nullptr, FALSE);
            hsvColor.value      = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR3,
                                                         nullptr, FALSE);
            // convert to ranges 0 - 1 for RGB conversion
            hsvColor.saturation /= 100.0f;
            hsvColor.value      /= 100.0f;
            rgbColor = hsvToRGB (hsvColor);
            // convert RGB ranges to 0 - 255
            rgbColor.red   *= 255;
            rgbColor.green *= 255;
            rgbColor.blue  *= 255;
          }

          // create a new brush and select it into DC
          hBrushNew = CreateSolidBrush (RGB ((BYTE) rgbColor.red,
                                             (BYTE) rgbColor.green,
                                             (BYTE) rgbColor.blue));
          hBrushOld = (HBRUSH)SelectObject( lpdis->hDC, hBrushNew );

          // draw the rectangle
          Rectangle (lpdis->hDC, rect.left, rect.top,
                     rect.right, rect.bottom);

          // put the old brush back and delete the new one
          SelectObject (lpdis->hDC, hBrushOld);
          DeleteObject (hBrushNew);

          // validate this new area
          ValidateRect (hWndControl, nullptr);

          break;

        }

        // --------------------------------------------------------------------
        // Draw the bar of either HUE or RED next to the scroll bar
        // --------------------------------------------------------------------
        case BUTTON_COLORBAR1:  {
          Real step;
          Int x, y;
          RGBColorReal rgbColor;
          HSVColorReal hsvColor;

          // compute how big of a red increment for each line as we step
          // down the bar
          if (mode == MODE_HSV)
            step = 360.0f / (Real) (rect.right - rect.left);
          else
            step = 255.0f / (Real) (rect.right - rect.left);

          // compute the first color, create pen for it, and save the
          // original pen
          if (mode == MODE_HSV)  {
            hsvColor.hue = 1;
            hsvColor.saturation = 1;
            hsvColor.value = 1;
            rgbColor = hsvToRGB (hsvColor);
            rgbColor.red   *= 255.0f;
            rgbColor.green *= 255.0f;
            rgbColor.blue  *= 255.0f;
          }
          else  {
            rgbColor.red = 0;
            rgbColor.green = 0;
            rgbColor.blue = 0;
          }

          // loop through each horizontal line available in the bar drawing
          // the correct color there
          for (x = 0; x < (rect.right - rect.left) - 1;  x++)  {

            // draw a horizontal row of pixels with this color
            for (y = 0; y < rect.bottom; y++)
              SetPixel (lpdis->hDC, x, y, RGB ((BYTE) rgbColor.red,
                                                 (BYTE) rgbColor.green,
                                                 (BYTE) rgbColor.blue));

            // increment the color, create new pen, and delete old pen
            if (mode == MODE_HSV)  {
              hsvColor.hue += step;
              rgbColor = hsvToRGB (hsvColor);
              rgbColor.red   *= 255;
              rgbColor.green *= 255;
              rgbColor.blue  *= 255;
            }
            else  {
              rgbColor.red += step;
            }

          }

          break;

        }

        // --------------------------------------------------------------------
        // Draw the bar of either SATURATION or GREEN next to the scroll bar
        // --------------------------------------------------------------------
        case BUTTON_COLORBAR2:  {
          Real step;
          Int x, y;
          RGBColorReal rgbColor;
          HSVColorReal hsvColor;

          // compute how big of a increment for each line as we step
          // down the bar
          if (mode == MODE_HSV)
            step = 1.0f / (Real) (rect.right - rect.left);
          else
            step = 255.0f / (Real) (rect.right - rect.left);

          // compute the first color, create pen for it, and save the
          // original pen
          if (mode == MODE_HSV)  {
            hsvColor.hue = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1,
                                                  nullptr, FALSE);
            hsvColor.saturation = 1.0f / 100.0f;
            hsvColor.value = 1;
            rgbColor = hsvToRGB (hsvColor);
            rgbColor.red   *= 255;
            rgbColor.green *= 255;
            rgbColor.blue  *= 255;
          }
          else  {
            rgbColor.red = 0;
            rgbColor.green = 0;
            rgbColor.blue = 0;
          }

          // loop through each horizontal line available in the bar drawing
          // the correct color there
          for (x = 0; x < (rect.right - rect.left) - 1;  x++)  {

            // draw a horizontal row of pixels with this color
            for (y = 0; y < rect.bottom; y++)
              SetPixel (lpdis->hDC, x, y, RGB ((BYTE) rgbColor.red,
                                                 (BYTE) rgbColor.green,
                                                 (BYTE) rgbColor.blue));

            // increment the color, create new pen, and delete old pen
            if (mode == MODE_HSV)  {
              hsvColor.saturation += step;
              rgbColor = hsvToRGB (hsvColor);
              rgbColor.red   *= 255;
              rgbColor.green *= 255;
              rgbColor.blue  *= 255;
            }
            else  {
              rgbColor.green += step;
            }

          }

          break;

        }

        // --------------------------------------------------------------------
        // Draw the bar of either VALUE or BLUE next to the scroll bar
        // --------------------------------------------------------------------
        case BUTTON_COLORBAR3:  {
          Real step;
          Int x, y;
          RGBColorReal rgbColor;
          HSVColorReal hsvColor;

          // compute how big of a increment for each line as we step
          // down the bar
          if (mode == MODE_HSV)
            step = 1.0f / (Real) (rect.right - rect.left);
          else
            step = 255.0f / (Real) (rect.right - rect.left);

          // compute the first color, create pen for it, and save the
          // original pen
          if (mode == MODE_HSV)  {
            hsvColor.hue = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1,
                                                  nullptr, FALSE);
            hsvColor.saturation =
              (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR2, nullptr, FALSE) / 100.0f;
            hsvColor.value = 1.0f / 100.0f;
            rgbColor = hsvToRGB (hsvColor);
            rgbColor.red   *= 255.0f;
            rgbColor.green *= 255.0f;
            rgbColor.blue  *= 255.0f;
          }
          else  {
            rgbColor.red = 0;
            rgbColor.green = 0;
            rgbColor.blue = 0;
          }

          // loop through each horizontal line available in the bar drawing
          // the correct color there
          for (x = 0; x < (rect.right - rect.left) - 1;  x++)  {

            // draw a horizontal row of pixels with this color
            for (y = 0; y < rect.bottom; y++)
              SetPixel (lpdis->hDC, x, y, RGB ((BYTE) rgbColor.red,
                                               (BYTE) rgbColor.green,
                                               (BYTE) rgbColor.blue));

            // increment the color, create new pen, and delete old pen
            if (mode == MODE_HSV)  {
              hsvColor.value += step;
              rgbColor = hsvToRGB (hsvColor);
              rgbColor.red   *= 255;
              rgbColor.green *= 255;
              rgbColor.blue  *= 255;
            }
            else  {
              rgbColor.blue += step;
            }

          }

          break;

        }

      }

      return TRUE;

    }

    // ------------------------------------------------------------------------
    // horizontal scrolling on the color bars
    // ------------------------------------------------------------------------
    case WM_HSCROLL:
		{
      Int  nScrollCode = (Int) LOWORD (wParam);        // scroll bar value
      Short nPos  = (Short) HIWORD (wParam);   // for thumbtrack only
      HWND hWndScroll = (HWND) lParam;                // handle of scroll bar
      Int  labelID;         // identifier of the text label for this scroll bar
      Int  thumbPos;        // current thumb position
      Int  minPos, maxPos;  // max and min of this scrollbar

      // get the thumb position for the scrollbar
      thumbPos = GetScrollPos (hWndScroll, SB_CTL);

      // find out which scroll bar we're talking about and set the correct
      // labelID for that control
      if (hWndScroll == hWndScrollColor1)
        labelID = LABEL_COLOR1;
      else if (hWndScroll == hWndScrollColor2)
        labelID = LABEL_COLOR2;
      else if (hWndScroll == hWndScrollColor3)
        labelID = LABEL_COLOR3;
			else if( hWndScroll == hWndScrollAlpha )
				labelID = LABEL_ALPHA;

      // find the max and min extents for this scroll bar
      SendMessage (hWndScroll, SBM_GETRANGE, (WPARAM) &minPos, (LPARAM) &maxPos);

      switch (nScrollCode)  {
        case SB_LINELEFT:  {
          if (thumbPos > minPos)
            thumbPos--;
          break;
        }
        case SB_PAGELEFT:  {
          if (thumbPos - 45 >= minPos)
            thumbPos -= 45;
          else
            thumbPos = minPos;
          break;
        }
        case SB_LINERIGHT:  {
          if (thumbPos < maxPos)
            thumbPos++;
          break;
        }
        case SB_PAGERIGHT:  {
          if (thumbPos + 45 < maxPos)
            thumbPos += 45;
          else
            thumbPos = maxPos;
          break;
        }
        case SB_THUMBTRACK:  {
          thumbPos = nPos;
          break;
        }
        default:  {
          return 0;
        }
      }

      // set the new scrollbar position and the text with it
      SendMessage (hWndScroll, SBM_SETPOS, (WPARAM) thumbPos, (LPARAM) TRUE);
      SetDlgItemInt (hWndDlg, labelID, thumbPos, FALSE);

      // if this was a color scroll bar, save the color change in
      // the appropriate color bead
      if (hWndScroll == hWndScrollColor1 ||
          hWndScroll == hWndScrollColor2 ||
          hWndScroll == hWndScrollColor3 ||
					hWndScroll == hWndScrollAlpha )
			{

        RGBColorReal rgbColor;
        HSVColorReal hsvColor;

        if (mode == MODE_RGB)  {
          rgbColor.red =   (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1,
                                                  nullptr, FALSE);
          rgbColor.green = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR2,
                                                  nullptr, FALSE);
          rgbColor.blue =  (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR3,
                                                  nullptr, FALSE);
					rgbColor.alpha = (Real) GetDlgItemInt( hWndDlg, LABEL_ALPHA,
																								  nullptr, FALSE );

        }
        else  {
          hsvColor.hue        = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1,
                                                       nullptr, FALSE);
          hsvColor.saturation = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR2,
                                                       nullptr, FALSE);
          hsvColor.value      = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR3,
                                                       nullptr, FALSE);
					hsvColor.alpha			= (Real) GetDlgItemInt( hWndDlg, LABEL_ALPHA,
																											 nullptr, FALSE );

          // convert to ranges 0 - 1 for RGB conversion
          hsvColor.saturation /= 100.0f;
          hsvColor.value      /= 100.0f;
          rgbColor = hsvToRGB (hsvColor);
          // convert RGB ranges to 0 - 255
          rgbColor.red   *= 255;
          rgbColor.green *= 255;
          rgbColor.blue  *= 255;
        }

        // store the color
        selectedColor.red   = (Int) rgbColor.red;
        selectedColor.green = (Int) rgbColor.green;
        selectedColor.blue  = (Int) rgbColor.blue;
				selectedColor.alpha = (Int) rgbColor.alpha;

        // force update of preview box
        // invalidate the preview box to force an update of its color
        InvalidateRect( hWndPreview, nullptr, FALSE);
        UpdateWindow (hWndPreview);

        // force updates of the colorbars
        InvalidateRect (hWndColorBar1, nullptr, FALSE);
        InvalidateRect (hWndColorBar2, nullptr, FALSE);
        InvalidateRect (hWndColorBar3, nullptr, FALSE);
        UpdateWindow (hWndColorBar1);
        UpdateWindow (hWndColorBar2);
        UpdateWindow (hWndColorBar3);

      }

      return 0;

    }

    // ------------------------------------------------------------------------
    case WM_COMMAND:  {
//      Int wNotifyCode = HIWORD(wParam); // notification code
      Int wID = LOWORD(wParam);         // id of control
//      HWND hWndControl = (HWND) lParam; // handle of control

      switch (wID)  {

        // --------------------------------------------------------------------
        // color ok
        // --------------------------------------------------------------------
        case IDOK:  {

          EndDialog( hWndDlg, TRUE );  // color selected
          break;

        }

        // --------------------------------------------------------------------
        case IDCANCEL:  {

          EndDialog( hWndDlg, FALSE );  // selection cancelled
          break;

        }

        // --------------------------------------------------------------------
        // Change from RGB mode to HSV mode and vice versa
        // --------------------------------------------------------------------
        case BUTTON_RGB_HSV:  {
          HWND hWndScroll;
          RGBColorReal rgbColor;
          HSVColorReal hsvColor;

          if (mode == MODE_RGB)  {  // switch to HSV
            rgbColor.red = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1, nullptr, FALSE);
            rgbColor.green = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR2, nullptr, FALSE);
            rgbColor.blue = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR3, nullptr, FALSE);

            // convert rgb to range 0 - 1
            rgbColor.red   /= 255.0f;
            rgbColor.green /= 255.0f;
            rgbColor.blue  /= 255.0f;
            // convert the RGB to HSV
            hsvColor = rgbToHSV (rgbColor);
            // turn saturation and value to 0 - 100 ranges
            hsvColor.saturation *= 100.0f;
            hsvColor.value      *= 100.0f;

            // change the scrollbar extents and positions
            hWndScroll = GetDlgItem (hWndDlg, SCROLL_COLOR1);
            SetScrollRange (hWndScroll, SB_CTL, 1, 360, FALSE);
            SetScrollPos (hWndScroll, SB_CTL, (Int) hsvColor.hue, TRUE);
            SetDlgItemInt (hWndDlg, LABEL_COLOR1, (Int) hsvColor.hue, FALSE);

            hWndScroll = GetDlgItem (hWndDlg, SCROLL_COLOR2);
            SetScrollRange (hWndScroll, SB_CTL, 1, 100, FALSE);
            SetScrollPos (hWndScroll, SB_CTL, (Int) hsvColor.saturation, TRUE);
            SetDlgItemInt (hWndDlg, LABEL_COLOR2, (Int) hsvColor.saturation, FALSE);

            hWndScroll = GetDlgItem (hWndDlg, SCROLL_COLOR3);
            SetScrollRange (hWndScroll, SB_CTL, 1, 100, FALSE);
            SetScrollPos (hWndScroll, SB_CTL, (Int) hsvColor.value, TRUE);
            SetDlgItemInt (hWndDlg, LABEL_COLOR3, (Int) hsvColor.value, FALSE);

            mode = MODE_HSV;

            // change the text for the button
            SetWindowText (GetDlgItem (hWndDlg, BUTTON_RGB_HSV),
                           "Switch to RGB");

          }
          else  {  // switch to RGB
            hsvColor.hue = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR1, nullptr, FALSE);
            hsvColor.saturation = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR2, nullptr, FALSE);
            hsvColor.value = (Real) GetDlgItemInt (hWndDlg, LABEL_COLOR3, nullptr, FALSE);

            // convert saturation and value to range 0 - 1
            hsvColor.saturation /= 100.0f;
            hsvColor.value      /= 100.0f;
            // convert the HSV to RGB
            rgbColor = hsvToRGB (hsvColor);
            // turn the rgb into 0 - 255 range
            rgbColor.red   *= 255.0f;
            rgbColor.green *= 255.0f;
            rgbColor.blue  *= 255.0f;

            // change the scrollbar extents and positions
            hWndScroll = GetDlgItem (hWndDlg, SCROLL_COLOR1);
            SetScrollRange (hWndScroll, SB_CTL, 1, 255, FALSE);
            SetScrollPos (hWndScroll, SB_CTL, (Int) rgbColor.red, TRUE);
            SetDlgItemInt (hWndDlg, LABEL_COLOR1, (Int) rgbColor.red, FALSE);

            hWndScroll = GetDlgItem (hWndDlg, SCROLL_COLOR2);
            SetScrollRange (hWndScroll, SB_CTL, 1, 255, FALSE);
            SetScrollPos (hWndScroll, SB_CTL, (Int) rgbColor.green, TRUE);
            SetDlgItemInt (hWndDlg, LABEL_COLOR2, (Int) rgbColor.green, FALSE);

            hWndScroll = GetDlgItem (hWndDlg, SCROLL_COLOR3);
            SetScrollRange (hWndScroll, SB_CTL, 1, 255, FALSE);
            SetScrollPos (hWndScroll, SB_CTL, (Int) rgbColor.blue, TRUE);
            SetDlgItemInt (hWndDlg, LABEL_COLOR3, (Int) rgbColor.blue, FALSE);

            // change the text for the button
            SetWindowText (GetDlgItem (hWndDlg, BUTTON_RGB_HSV),
                           "Switch to HSV");

            mode = MODE_RGB;

          }

          // invalidate all the vertical color bars so they are redrawn
          InvalidateRect (hWndColorBar1, nullptr, TRUE);
          InvalidateRect (hWndColorBar2, nullptr, TRUE);
          InvalidateRect (hWndColorBar3, nullptr, TRUE);

        }

      }

      return 0;

    }

    // ------------------------------------------------------------------------
    // Only hide the window on a close rather than destroy it since it will
    // probably be needed again.
    // ------------------------------------------------------------------------
    case WM_CLOSE:
      ShowWindow(hWndDlg, SW_HIDE);
      return 0;

    default:
      return 0;  // for all messages that are not processed

  }

}
