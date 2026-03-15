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

//////////////////////////////////////////////////////////////////////
//
//  Toolbar.cpp
//
//  Implementation of a 'fancy' toolbar using hi-color buttons
//

#include "StdAfx.h"
#include "Toolbar.h"



BEGIN_MESSAGE_MAP(CFancyToolbar, CControlBar)
	//{{AFX_MSG_MAP(CFancyToolbar)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////
//
//  Local Constants
//
const TCHAR * const TOOLBAR_CLASS_NAME      = TEXT ("FANCYTOOLBAR");


//////////////////////////////////////////////////////////////
//
//  CFancyToolbar
//
CFancyToolbar::CFancyToolbar (void)
    : m_iButtons (0),
      m_iCurrentButton (-1)
{
    // Ensure that the toolbar window class is registered
    RegisterFancyToolbarClass ();

    ::memset (m_pButtonArray, 0, sizeof (m_pButtonArray));
    return ;
}

//////////////////////////////////////////////////////////////
//
//  ~CFancyToolbar
//
CFancyToolbar::~CFancyToolbar (void)
{
    for (int iButton = 0; iButton < m_iButtons; iButton ++)
    {
        if (m_pButtonArray[iButton].hBMPUp)
        {
            // Free the BMP for this button
            ::DeleteObject (m_pButtonArray[iButton].hBMPUp);
            m_pButtonArray[iButton].hBMPUp = nullptr;
        }

        if (m_pButtonArray[iButton].hBMPDn)
        {
            // Free the BMP for this button
            ::DeleteObject (m_pButtonArray[iButton].hBMPDn);
            m_pButtonArray[iButton].hBMPDn = nullptr;
        }
    }

    return ;
}

//////////////////////////////////////////////////////////////
//
//  RegisterFancyToolbarClass
//
void
CFancyToolbar::RegisterFancyToolbarClass (void)
{
    // Is this class already registered?
    WNDCLASS classInfo = { 0 };
    if (::GetClassInfo (::AfxGetInstanceHandle (),
                        TOOLBAR_CLASS_NAME,
                        &classInfo) != TRUE)
    {
        classInfo.style = CS_PARENTDC;
        classInfo.lpfnWndProc = ::DefWindowProc;
        classInfo.hInstance = ::AfxGetInstanceHandle ();
        classInfo.hCursor = ::LoadCursor (nullptr, IDC_ARROW);
        classInfo.hbrBackground = (HBRUSH)COLOR_BTNFACE;
        classInfo.lpszClassName = TOOLBAR_CLASS_NAME;

        // Register the class with windows
        ::RegisterClass (&classInfo);
    }

    return ;
}

//////////////////////////////////////////////////////////////
//
//  Create
//
void
CFancyToolbar::OnPaint ()
{
    Paint ();
    return ;
}


//////////////////////////////////////////////////////////////
//
//  Create
//
BOOL
CFancyToolbar::Create
(
    LPCTSTR pszWindowName,
    CWnd *pCParentWnd,
    UINT uiID
)
{
    // Create the toolbar window using our own special window class
    RECT rect = { 0 };
    BOOL bReturn = CWnd::Create (TOOLBAR_CLASS_NAME, pszWindowName, WS_CHILD | WS_VISIBLE, rect, pCParentWnd, uiID);

    // Were we successful?
    ASSERT (bReturn);
    if (bReturn)
    {
        // Set the bar style, but don't allow docking
        SetBarStyle (GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
        EnableDocking (CBRS_ALIGN_ANY);
    }

    // Return the TRUE/FALSE result code
    return bReturn;
}

//////////////////////////////////////////////////////////////
//
//  DrawButton
//
void
CFancyToolbar::DrawButton
(
    HDC hDC,
    int iXPos,
    int iYPos,
    HBITMAP hBMP
)
{
    // Create a screen-compatible DC
    HDC hMemDC = ::CreateCompatibleDC (hDC);
    if (hMemDC)
    {
        // Select the BMP to paint into the DC
        HBITMAP hOldBMP = (HBITMAP)::SelectObject (hMemDC, hBMP);

        // Paint the BMP in its slot
        ::BitBlt (hDC, iXPos, iYPos, BUTTON_WIDTH, BUTTON_HEIGHT, hMemDC, 0, 0, SRCCOPY);

        // Select the old BMP back into the DC and free the DC
        ::SelectObject (hMemDC, hOldBMP);
        DeleteDC (hMemDC);
    }

    return ;
}

//////////////////////////////////////////////////////////////
//
//  AddButton
//
void
CFancyToolbar::AddButton
(
    UINT iBMPUp,
    UINT iBMPDn,
    int iCommandID,
    BUTTON_TYPE buttonType
)
{
    // Increment the button count
    int iButton = m_iButtons ++;

    // Fill in the internal button structure for this entry
    m_pButtonArray[iButton].hBMPUp = ::LoadBitmap (::AfxGetResourceHandle (), MAKEINTRESOURCE (iBMPUp));
    m_pButtonArray[iButton].hBMPDn = ::LoadBitmap (::AfxGetResourceHandle (), MAKEINTRESOURCE (iBMPDn));
    m_pButtonArray[iButton].iCommandID = iCommandID;
    m_pButtonArray[iButton].buttonType = buttonType;
    m_pButtonArray[iButton].currentState = StateUp;
    m_pButtonArray[iButton].bVisible = TRUE;
    return ;
}

//////////////////////////////////////////////////////////////
//
//  Paint
//
void
CFancyToolbar::Paint (void)
{
    // Get the window's DC
    HDC hDC = ::GetDC (m_hWnd);
    if (hDC)
    {
        // Get the bounding rectangle of the window
        RECT rect;
        ::GetClientRect (m_hWnd, &rect);

        // Paint the background light gray
        HBRUSH hBrush = ::CreateSolidBrush (RGB (192, 192, 192));
        ::FillRect (hDC, &rect, hBrush);
        ::DeleteObject (hBrush);

        // Loop through each button and paint it
        int iXPos = BORDER_LEFT;
        for (int iButton = 0; iButton < m_iButtons; iButton ++)
        {
            // Is this button visible?
            if (m_pButtonArray[iButton].bVisible)
            {
                // Determine which BMP to use
                HBITMAP hBMP = m_pButtonArray[iButton].hBMPUp;
                if (m_pButtonArray[iButton].currentState == StateDn)
                {
                    hBMP = m_pButtonArray[iButton].hBMPDn;
                }

                // Draw this button
                DrawButton (hDC,
                            iXPos,
                            ((rect.bottom-rect.top) >> 1) - BUTTON_MIDDLE,
                            hBMP);

                // Increment the current x position
                iXPos += BUTTON_WIDTH;
            }
        }

        // Free the windows DC
        ::ReleaseDC (m_hWnd, hDC);
    }

    // Let the window know its done painting
    ::ValidateRect (m_hWnd, nullptr);
    return ;
}

//////////////////////////////////////////////////////////////
//
//  ButtonFromPoint
//
int
CFancyToolbar::ButtonFromPoint (const CPoint &point)
{
    int iIndex = -1;

    // Loop through all the buttons until we've found the
    // one we're looking for
    int iXPos = BORDER_LEFT;
    for (int iButton = 0; (iButton < m_iButtons) && (iIndex == -1); iButton ++)
    {
        // Is this the button we're looking for?
        if (m_pButtonArray[iButton].bVisible &&
            (point.x >= iXPos) && (point.x <= iXPos + BUTTON_WIDTH) &&
            (point.y >= BORDER_TOP) && (point.y <= BORDER_TOP+BUTTON_HEIGHT))
        {
            // Yup, this is the button we're looking for
            iIndex = iButton;
        }

        // Increment the current position
        iXPos += BUTTON_WIDTH;
    }

    // Return the zero based index of the button we're looking for
    return iIndex;
}

//////////////////////////////////////////////////////////////
//
//  HandleLButtonUp
//
void
CFancyToolbar::OnLButtonDown
(
    UINT nFlags,
    CPoint point
)
{
    // Determine which button was clicked
    int iButton = ButtonFromPoint (point);
    if (iButton >= 0)
    {
        // Flip the button state
        m_pButtonArray[iButton].currentState = (STATE_INFO)!m_pButtonArray[iButton].currentState;

        // Determine which BMP to paint
        HBITMAP hBMP = m_pButtonArray[iButton].hBMPUp;
        if (m_pButtonArray[iButton].currentState == StateDn)
        {
            hBMP = m_pButtonArray[iButton].hBMPDn;
        }

        // Get the DC for the window
        HDC hDC = ::GetDC (m_hWnd);
        if (hDC)
        {
            // Get the bounding rectangle for the window
            RECT rect;
            ::GetClientRect (m_hWnd, &rect);

            // Draw the selected button
            DrawButton (hDC,
                        BORDER_LEFT + iButton*BUTTON_WIDTH,
                        ((rect.bottom-rect.top) >> 1) - BUTTON_MIDDLE,
                        hBMP);

            // Release the window's DC
            ::ReleaseDC (m_hWnd, hDC);
        }

        // Is this a 'normal' or 2 state button?
        if (m_pButtonArray[iButton].buttonType == TypeNormal)
        {
            // Normal button, so trap all mouse message so if the user
            // lets up the mouse button outside the button rectangle we don't do anything
            m_iCurrentButton = iButton;
            ::SetCapture (m_hWnd);
        }
        else
        {
            // 2 state button
            m_iCurrentButton = -1;

            // Send the message to the window's parent to let them know a command has occurred
            ::AfxGetMainWnd ()->PostMessage (WM_COMMAND,
                                             MAKELONG (m_pButtonArray[iButton].iCommandID, BN_CLICKED),
                                             (LPARAM)m_hWnd);
        }
    }
    else
    {
        CControlBar::OnLButtonDown (nFlags, point);
    }

    return ;
}

//////////////////////////////////////////////////////////////
//
//  HandleLButtonUp
//
void
CFancyToolbar::OnLButtonUp
(
    UINT nFlags,
    CPoint point
)
{
    // Were we currently processing a button?
    if (m_iCurrentButton >= 0)
    {
        // Which button were we processing?
        int iButton = ButtonFromPoint (point);
        if (iButton == m_iCurrentButton)
        {
            // Fire a command to the parent
            ::AfxGetMainWnd ()->PostMessage (WM_COMMAND,
                                             MAKELONG (m_pButtonArray[iButton].iCommandID, BN_CLICKED),
                                             (LPARAM)m_hWnd);
        }

        // Reset the button state
        m_pButtonArray[m_iCurrentButton].currentState = StateUp;

        // Get the window's DC
        HDC hDC = ::GetDC (m_hWnd);
        if (hDC)
        {
            // Get the bounding rectangle of the window
            RECT rect;
            ::GetClientRect (m_hWnd, &rect);

            // Paint the button
            DrawButton (hDC,
                        BORDER_LEFT + m_iCurrentButton*BUTTON_WIDTH,
                        ((rect.bottom-rect.top) >> 1) - BUTTON_MIDDLE,
                        m_pButtonArray[m_iCurrentButton].hBMPUp);

            // Free the window's DC
            ::ReleaseDC (m_hWnd, hDC);
        }

        // Let go of the mouse capture
        ::ReleaseCapture ();
    }
    else
    {
        CControlBar::OnLButtonUp (nFlags, point);
    }

    return ;
}

//////////////////////////////////////////////////////////////
//
//  OnDraw
//
void
CFancyToolbar::OnDraw (CDC* pDC)
{
    return ;
}

//////////////////////////////////////////////////////////////
//
//  PreCreateWindow
//
BOOL
CFancyToolbar::PreCreateWindow (CREATESTRUCT& cs)
{
    // Allow the base class to process this message
    return CControlBar::PreCreateWindow (cs);
}

//////////////////////////////////////////////////////////////
//
//  SetButtonState
//
void
CFancyToolbar::SetButtonState
(
    int iCommandID,
    STATE_INFO newState,
    BOOL bRepaint
)
{
    BOOL bFound = FALSE;

    // Loop through all the buttons until we've found the one we're looking for
    for (int iButton = 0;
         (iButton < m_iButtons) && (bFound == FALSE);
         iButton ++)
    {
        if (m_pButtonArray[iButton].iCommandID == iCommandID)
        {
            // Set the new state
            m_pButtonArray[iButton].currentState = newState;

            if (bRepaint)
            {
                // Repaint the toolbar
                //Paint ();
                InvalidateRect (nullptr);
                UpdateWindow ();
            }

            // Found it!
            bFound = TRUE;
        }
    }

    return ;
}

//////////////////////////////////////////////////////////////
//
//  GetButtonState
//
CFancyToolbar::STATE_INFO
CFancyToolbar::GetButtonState (int iCommandID) const
{
    STATE_INFO stateInfo = StateUp;

    // Loop through all the buttons until we've found the one we're looking for
    BOOL bFound = FALSE;
    for (int iButton = 0;
         (iButton < m_iButtons) && (bFound == FALSE);
         iButton ++)
    {
        if (m_pButtonArray[iButton].iCommandID == iCommandID)
        {
            // Return the current state to the caller
            stateInfo = m_pButtonArray[iButton].currentState;

            // Found it!
            bFound = TRUE;
        }
    }

    // Return the state of the requested button
    return stateInfo;
}

