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

// TeamIdentity.h : header file
//
class Dict;
class SidesList;

/////////////////////////////////////////////////////////////////////////////
// TeamIdentity dialog

class TeamIdentity : public CPropertyPage
{
// Construction
public:
	TeamIdentity();   // standard constructor

// Dialog Data
	//{{AFX_DATA(TeamIdentity)
	enum { IDD = IDD_TeamIdentity };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TeamIdentity)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

protected:
	Dict		*m_teamDict;
	SidesList	*m_sides;
protected:
	void loadUnitsInfo(int idcMinUnit, NameKeyType keyMinUnit,
								int idcMaxUnit, NameKeyType keyMaxUnit,
								int idcUnitType, NameKeyType keyUnitType);
	void OnUnitTypeButton(Int idcUnitType);

public:
	void setTeamDict(Dict *pDict) {m_teamDict = pDict;};
	void setSidesList(SidesList *pSides) {m_sides = pSides;};
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TeamIdentity)
	virtual BOOL OnInitDialog();
	afx_msg void OnAiRecruitable();
	afx_msg void OnAutoReinforce();
	afx_msg void OnBaseDefense();
	afx_msg void OnChangeDescription();
	afx_msg void OnChangeMax();
	afx_msg void OnChangePriorityDecrease();
	afx_msg void OnChangePriorityIncrease();
	afx_msg void OnSelchangeProductionCondition();
	afx_msg void OnChangeProductionPriority();
	afx_msg void OnSelchangeHomeWaypoint();
	afx_msg void OnUnitType1Button();
	afx_msg void OnUnitType2Button();
	afx_msg void OnUnitType3Button();
	afx_msg void OnUnitType4Button();
	afx_msg void OnUnitType5Button();
	afx_msg void OnUnitType6Button();
	afx_msg void OnUnitType7Button();
	afx_msg void OnExecuteActions();
	afx_msg void OnChangeTeamName();
	afx_msg void OnTeamSingleton();
	afx_msg void OnKillfocusTeamName();
	afx_msg void OnSelendokTeamowner();
	afx_msg void OnChangeTeamBuildFrames();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
