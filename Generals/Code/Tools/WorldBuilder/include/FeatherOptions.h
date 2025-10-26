/*
**	Command & Conquer Generals(tm)
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
/// FeatherOptions modeless (floating) dialog - allows entry and display of brush width and feather.

class FeatherOptions : public COptionsPanel	, public PopupSliderOwner
{
// Construction
public:
	enum {MIN_FEATHER_SIZE=2,
				MAX_FEATHER_SIZE=51,
				MIN_RATE=1,
				MAX_RATE=10,
				MIN_RADIUS=1,
				MAX_RADIUS=5};

	FeatherOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(FeatherOptions)
	enum { IDD = IDD_FEATHER_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FeatherOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  //!< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; //!< Modeless dialogs don't close on ESC, so eat this for modeless.
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FeatherOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSizeEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	static FeatherOptions *m_staticThis;  ///< Reference to the floating panel so SetWidth and SetFeather can be static.
	static Int m_currentFeather;					///< current brush width in the ui.
	static Int m_currentRadius;					///< current smoothing radius value.
	static Int m_currentRate;					///< current smoothing rate.

	Bool		m_updating; ///<true if the ui is updating itself.
	WBPopupSliderButton m_featherPopup;
	WBPopupSliderButton m_radiusPopup;
	WBPopupSliderButton m_ratePopup;

public:
	static void setFeather(Int feather);
	static void setRate(Int rate);
	static void setRadius(Int radius);

public:

	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
