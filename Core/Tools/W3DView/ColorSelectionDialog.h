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

// ColorSelectionDialog.h : header file
//

#include "resource.h"
#include "vector3.h"


/////////////////////////////////////////////////////////////////////////////
//
// ColorSelectionDialogClass dialog
//
class ColorSelectionDialogClass : public CDialog
{
// Construction
public:
	ColorSelectionDialogClass (const Vector3 &def_color, CWnd *pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ColorSelectionDialogClass)
	enum { IDD = IDD_COLOR_SEL };
	CSpinButtonCtrl	m_BlueSpin;
	CSpinButtonCtrl	m_GreenSpin;
	CSpinButtonCtrl	m_RedSpin;
	CStatic	m_ColorWindow;
	CSliderCtrl	m_BlueSlider;
	CSliderCtrl	m_GreenSlider;
	CSliderCtrl	m_RedSlider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ColorSelectionDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ColorSelectionDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg void OnGrayscaleCheck();
	afx_msg void OnChangeBlueEdit();
	afx_msg void OnChangeGreenEdit();
	afx_msg void OnChangeRedEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		///////////////////////////////////////////////////////
		//
		//	Public methods
		//
		const Vector3 &		Get_Color (void) const				{ return m_Color; }
		void						Set_Color (const Vector3 &color) { m_Color = color; }

	protected:

		///////////////////////////////////////////////////////
		//
		//	Inline accessors
		//
		void						Paint_Color_Window (void);
		void						Update_Sliders (int slider_id);

	private:

		///////////////////////////////////////////////////////
		//
		//	Private member data
		//
		Vector3					m_Color;
		Vector3					m_PaintColor;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
