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

// brushoptions.h : header file
//

#include "WBPopupSlider.h"
#include "OptionsPanel.h"
/////////////////////////////////////////////////////////////////////////////
/// BrushOptions modeless (floating) dialog - allows entry and display of brush width and feather.

class BrushOptions : public COptionsPanel	, public PopupSliderOwner
{
// Construction
public:
	enum {MIN_BRUSH_SIZE=1,
				MAX_BRUSH_SIZE=51,
				FREQ_BRUSH_TICKS=10,
				MIN_FEATHER=0,
				FREQ_FEATHER_TICKS=4,
				MAX_FEATHER=20};

	BrushOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(BrushOptions)
	enum { IDD = IDD_BRUSH_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BrushOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  //!< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; //!< Modeless dialogs don't close on ESC, so eat this for modeless.
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BrushOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFeatherEdit();
	afx_msg void OnChangeSizeEdit();
	afx_msg void OnChangeHeightEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	static BrushOptions *m_staticThis;  ///< Reference to the floating panel so SetWidth and SetFeather can be static.
	static Int m_currentWidth;					///< current brush width in the ui.
	static Int m_currentFeather;				///< current feather width in the ui.
	static Int m_currentHeight;

	Bool		m_updating; ///<true if the ui is updating itself.
	WBPopupSliderButton m_brushWidthPopup;
	WBPopupSliderButton m_brushFeatherPopup;
	WBPopupSliderButton m_brushHeightPopup;

public:
	static void setWidth(Int width);
	static void setFeather(Int feather);
	static void setHeight(Int height);

public:

	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
