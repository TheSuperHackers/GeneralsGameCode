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
//  Toolbar.H
//
//  Declaration of a 'fancy' toolbar using hi-color buttons
//

#ifndef __FANCYTOOLBAR_H
#define __FANCYTOOLBAR_H


//////////////////////////////////////////////////////////////
//
//  Constants
//
const int BUTTON_WIDTH      = 42;
const int BUTTON_HEIGHT     = 36;
const int BUTTON_MIDDLE     = 18;
const int BORDER_LEFT       = 8;
const int BORDER_RIGHT      = 8;
const int BORDER_TOP        = 8;
const int BORDER_BOTTOM     = 8;


//////////////////////////////////////////////////////////////
//
//  CFancyToolbar
//
class CFancyToolbar : public CControlBar
{
    ////////////////////////////////////////////////////////
    //
    //  MFC Junk
    //

    public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFancyToolbar)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

    public:

        ////////////////////////////////////////////////////////
        //
        //  Public Data Types
        //
        typedef enum
        {
            StateUp = 0,
            StateDn = 1
        } STATE_INFO;

        typedef enum
        {
            TypeNormal = 0,
            Type2State = 1
        } BUTTON_TYPE;


        ////////////////////////////////////////////////////////
        //
        //  Public Contructors
        //
        CFancyToolbar ();
        virtual ~CFancyToolbar ();


        ////////////////////////////////////////////////////////
        //
        //  Public Methods
        //

        //
        //  Required methods
        //
        CSize CalcFixedLayout (BOOL, BOOL)
            { return CSize (m_iButtons*BUTTON_WIDTH + BORDER_LEFT + BORDER_RIGHT, BUTTON_HEIGHT + BORDER_TOP + BORDER_BOTTOM); }

        CSize CalcDynamicLayout( int nLength, DWORD dwMode )
            { return CSize (m_iButtons*BUTTON_WIDTH + BORDER_LEFT + BORDER_RIGHT, BUTTON_HEIGHT + BORDER_TOP + BORDER_BOTTOM); }

        void OnUpdateCmdUI (class CFrameWnd*, int) {}

        //
        //  Creation routines
        //
        void AddButton (UINT iBMPUp, UINT iBMPDn, int iCommandID, BUTTON_TYPE buttonType = TypeNormal);
        BOOL Create (LPCTSTR pszWindowName, CWnd *pCParentWnd, UINT uiID);

        //
        //  State management routines
        //
        void SetButtonState (int iCommandID, STATE_INFO newState, BOOL bRepaint = TRUE);
        STATE_INFO GetButtonState (int iCommandID) const;

    protected:

        ////////////////////////////////////////////////////////
        //
        //  Protected Methods
        //
        void Paint (void);
        void DrawButton (HDC hDC, int iXPos, int iYPos, HBITMAP hBMP);
        int ButtonFromPoint (const CPoint &point);
        void RegisterFancyToolbarClass (void);

	    //{{AFX_MSG(CFancyToolbar)
        afx_msg void OnPaint();
	    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	    //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

        ////////////////////////////////////////////////////////
        //
        //  Static Methods
        //
        static LRESULT CALLBACK fnMessageProc (HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam);

    private:

        ////////////////////////////////////////////////////////
        //
        //  Private Data Types
        //
        typedef struct
        {
            HBITMAP hBMPUp;
            HBITMAP hBMPDn;
            int iCommandID;
            STATE_INFO currentState;
            BUTTON_TYPE buttonType;
            BOOL bVisible;
        } BUTTON_INFO;


        ////////////////////////////////////////////////////////
        //
        //  Private Methods
        //
        BUTTON_INFO m_pButtonArray[10];
        int m_iButtons;
        int m_iCurrentButton;
};

#endif // __FANCYTOOLBAR_H
