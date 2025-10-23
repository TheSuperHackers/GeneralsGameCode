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

// TeamReinforcement.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TeamReinforcement dialog

class TeamReinforcement : public CPropertyPage
{
// Construction
public:
	TeamReinforcement();   // standard constructor

// Dialog Data
	//{{AFX_DATA(TeamReinforcement)
	enum { IDD = IDD_TeamReinforcement };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TeamReinforcement)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	Dict *m_teamDict;
public:
	void setTeamDict(Dict *pDict) {m_teamDict = pDict;};
protected:

	// Generated message map functions
	//{{AFX_MSG(TeamReinforcement)
	afx_msg void OnDeployBy();
	afx_msg void OnTeamStartsFull();
	afx_msg void OnSelchangeTransportCombo();
	afx_msg void OnTransportsExit();
	afx_msg void OnSelchangeVeterancy();
	afx_msg void OnSelchangeWaypointCombo();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
