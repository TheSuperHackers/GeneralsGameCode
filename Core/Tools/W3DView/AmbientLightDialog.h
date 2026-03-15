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

#pragma once

// AmbientLightDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAmbientLightDialog dialog

class CAmbientLightDialog : public CDialog
{
// Construction
public:
	CAmbientLightDialog(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAmbientLightDialog)
	enum { IDD = IDD_LIGHT_AMBIENT_DIALOG };
	CSliderCtrl	m_blueSlider;
	CSliderCtrl	m_greenSlider;
	CSliderCtrl	m_redSlider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAmbientLightDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAmbientLightDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnCancel();
	afx_msg void OnGrayscaleCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    private:
        int m_initialRed;
        int m_initialGreen;
        int m_initialBlue;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
