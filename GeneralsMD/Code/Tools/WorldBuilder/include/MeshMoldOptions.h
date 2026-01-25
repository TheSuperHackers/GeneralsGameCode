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

// MeshMoldOptions.h : header file
//
#include "WBPopupSlider.h"
#include "OptionsPanel.h"
#include "Common/AsciiString.h"

/////////////////////////////////////////////////////////////////////////////
// MeshMoldOptions dialog

class MeshMoldOptions : public COptionsPanel , public PopupSliderOwner
{
// Construction
public:
	MeshMoldOptions(CWnd* pParent = nullptr);   // standard constructor
	enum {MIN_ANGLE=-180,
				MAX_ANGLE=180,
				MIN_HEIGHT=-10,
				MAX_HEIGHT=256,
				MIN_SCALE=1,
				MAX_SCALE=200};

// Dialog Data
	//{{AFX_DATA(MeshMoldOptions)
	enum { IDD = IDD_MESHMOLD_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MeshMoldOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK(){return;};  //!< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; //!< Modeless dialogs don't close on ESC, so eat this for modeless.
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(MeshMoldOptions)
	afx_msg void OnPreview();
	afx_msg void OnApplyMesh();
	afx_msg void OnChangeScaleEdit();
	afx_msg void OnChangeHeightEdit();
	afx_msg void OnChangeAngleEdit();
	afx_msg void OnRaise();
	afx_msg void OnRaiseLower();
	afx_msg void OnLower();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	WBPopupSliderButton m_anglePopup;
	WBPopupSliderButton m_scalePopup;
	WBPopupSliderButton m_HeightPopup;
	CTreeCtrl						m_moldTreeView;
	Bool								m_updating;
	AsciiString					m_meshModelName;

	static Real m_currentHeight;
	static Real m_currentScale;
	static Int m_currentAngle;
	static Bool m_doingPreview;
	static Bool m_raiseOnly;
	static Bool m_lowerOnly;

public:
	static void setScale(Real scale);
	static void setHeight(Real height);
	static void setAngle(Int angle);

	static Real getHeight(void) {return m_currentHeight;};
	static Real getScale(void) {return m_currentScale;};
	static Int getAngle(void) {return m_currentAngle;};
	static MeshMoldOptions *m_staticThis;
	static Bool isDoingPreview(void) {return m_doingPreview;};
	static Bool isRaisingOnly(void) {return m_raiseOnly;};
	static Bool isLoweringOnly(void) {return m_lowerOnly;};
	static AsciiString getModelName(void) {if (m_staticThis) return m_staticThis->m_meshModelName; return "";};

public:	 //PopupSliderOwner methods.
	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
