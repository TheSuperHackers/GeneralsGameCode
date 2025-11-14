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

#pragma once

// ContourOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
/// ContourOptions modeless (floating) dialog - allows entry and display of brush width and feather.

class ContourOptions : public CDialog
{
// Construction
public:
	enum {MIN_CONTOUR_STEP=1,
				MAX_CONTOUR_STEP=10,
				MIN_CONTOUR_OFFSET=-10,
				MAX_CONTOUR_OFFSET=+10,
				MIN_WIDTH=1,
				MAX_WIDTH=6};

	ContourOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ContourOptions)
	enum { IDD = IDD_CONTOUR_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ContourOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ContourOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnShowContours();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	static Int m_contourStep;					///< current contour step in the ui.
	static Int m_contourOffset;				///< current contour offset in the ui.
	static Int m_contourWidth;				///< current contour offset in the ui.

	Bool		m_updating; ///<true if the ui is updating itself.
	CSliderCtrl m_contourStepSlider; ///< Feather slider control.
	CSliderCtrl m_contourOffsetSlider;		///< Brush width slider control.
	CSliderCtrl m_contourWidthSlider;		///< Brush width slider control.

public:
	static Int getContourWidth(void) {return m_contourWidth;};  ///<Returns width.
	static Int getContourOffset(void) {return m_contourOffset;}; ///<Returns offset.
	static Int getContourStep(void) {return m_contourStep;}; ///<Returns step.


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
