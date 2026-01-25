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

// GLOBALLIGHTOPTIONS.h : header file
//

#include "WBPopupSlider.h"
#include "resource.h"
#include "CButtonShowColor.h"
/////////////////////////////////////////////////////////////////////////////
/// GlobalLightOptions modeless (floating) dialog - allows entry and display of brush width and feather.

#define  GLOBALLIGHT_OPTIONS_PANEL_SECTION "LightOptionsWindow"

class GlobalLightOptions : public CDialog	, public PopupSliderOwner
{
// Construction
public:
	enum {K_TERRAIN=1, K_OBJECTS=2, K_BOTH=3};
	enum {K_SUN=0, K_ACCENT1=1, K_ACCENT2=2};
	int kUIRedIDs[3];// = {IDC_RD_EDIT, IDC_RD_EDIT1, IDC_RD_EDIT2};
	int kUIGreenIDs[3];// = {IDC_GD_EDIT, IDC_GD_EDIT1, IDC_GD_EDIT2};
	int kUIBlueIDs[3];// = {IDC_BD_EDIT, IDC_BD_EDIT1, IDC_BD_EDIT2};
	CButtonShowColor m_colorButton;

	GlobalLightOptions(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GlobalLightOptions)
	enum { IDD = IDD_GLOBAL_LIGHT_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GlobalLightOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GlobalLightOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnChangeFrontBackEdit();
	afx_msg void OnChangeLeftRightEdit();
	afx_msg void OnChangeColorEdit();
	afx_msg void OnRadioEverything();
	afx_msg void OnRadioObjects();
	afx_msg void OnRadioTerrain();
	afx_msg void OnColorPress();
	afx_msg void OnResetLights();
	afx_msg void OnClose();
	virtual void OnOK(){return;};  //!< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; //!< Modeless dialogs don't close on ESC, so eat this for modeless.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	Real	ComponentToPercent(Int component)
	{
			Real percent;
			if (component >= 255) {
				return 1.0;
			}
			if (component <= 0) {
				return 0.0;
			}
			percent = (Real)component/255.0;
			return percent;
	}
	Int		PercentToComponent(Real percent)
	{
			Int component;
			if (percent >= 1.0) {
				return 255;
			}
			if (percent <= 0.0) {
				return 0;
			}
			component = (percent * 255.0);
			return component;
	};
	BOOL	GetInt(Int ctrlID, Int *rVal);
	void	PutInt(Int ctrlID, Int val);

protected:

	Bool		m_updating; ///<true if the ui is updating itself.
	WBPopupSliderButton m_frontBackPopup;
	WBPopupSliderButton m_leftRightPopup;
	WBPopupSliderButton m_frontBackPopupAccent1;
	WBPopupSliderButton m_leftRightPopupAccent1;
	WBPopupSliderButton m_frontBackPopupAccent2;
	WBPopupSliderButton m_leftRightPopupAccent2;
	Int			m_angleAzimuth[3];
	Int			m_angleElevation[3];
	Int			m_lighting;

protected:
	void applyAngle(Int lightIndex=0);
	void showLightFeedback(Int lightIndex=0);
	void applyColor(Int lightIndex=0);
	void updateEditFields(void);
	void stuffValuesIntoFields(Int lightIndex = 0);
public:

	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
