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

// BuildList.h : header file
//

#include "TerrainSwatches.h"
#include "OptionsPanel.h"
#include "WBPopupSlider.h"
#include "Common/AsciiString.h"
class WorldHeightMapEdit;
class BuildListInfo;
/////////////////////////////////////////////////////////////////////////////
// BuildList dialog

class BuildList : public COptionsPanel, public PopupSliderOwner
{
// Construction
public:
 	BuildList(CWnd* pParent = nullptr);   ///< standard constructor

	~BuildList(void);   ///< standard destructor
	enum { NAME_MAX_LEN = 64 };
// Dialog Data
	//{{AFX_DATA(BuildList)
	enum { IDD = IDD_BUILD_LIST_PANEL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BuildList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  ///< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; ///< Modeless dialogs don't close on ESC, so eat this for modeless.
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BuildList)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSidesCombo();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnAddBuilding();
	afx_msg void OnSelchangeBuildList();
	afx_msg void OnAlreadyBuild();
	afx_msg void OnDeleteBuilding();
	afx_msg void OnSelendokRebuilds();
	afx_msg void OnEditchangeRebuilds();
	afx_msg void OnDblclkBuildList();
	afx_msg void OnChangeZOffset();
	afx_msg void OnChangeAngle();
	afx_msg void OnExport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


protected:
	Int					m_curSide;
	Int					m_curBuildList;
	WBPopupSliderButton m_heightSlider;
	WBPopupSliderButton m_angleSlider;
	Real				m_angle;
	Real				m_height;

	static BuildList	*m_staticThis;
	static Bool			m_updating;

protected:
	void loadSides(void);
	void updateCurSide(void);

public:
	static void addBuilding(Coord3D loc, Real angle, AsciiString name);
	static void update(void) {if (m_staticThis) m_staticThis->loadSides();};
	static void setSelectedBuildList(BuildListInfo *pInfo);

	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
