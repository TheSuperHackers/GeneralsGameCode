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

// TeamBehavior.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TeamBehavior dialog

class TeamBehavior : public CPropertyPage
{
// Construction
public:
	TeamBehavior();   // standard constructor

// Dialog Data
	//{{AFX_DATA(TeamBehavior)
	enum { IDD = IDD_TeamBehavior };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TeamBehavior)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	Dict *m_teamDict;
protected:
	void setupScript(NameKeyType keyScript, int idcScript);
	void updateScript(NameKeyType keyScript, int idcScript);
public:
	void setTeamDict(Dict *pDict) {m_teamDict = pDict;};
protected:

	// Generated message map functions
	//{{AFX_MSG(TeamBehavior)
	afx_msg void OnSelchangeOnCreateScript();
	virtual BOOL OnInitDialog();
	afx_msg void OnTransportsReturn();
	afx_msg void OnAvoidThreats();
	afx_msg void OnSelchangeOnEnemySighted();
	afx_msg void OnSelchangeOnDestroyed();
	afx_msg void OnSelchangeOnUnitDestroyed();
	afx_msg void OnPerimeterDefense();
	afx_msg void OnBaseDefense();
	afx_msg void OnChangePercentDestroyed();
	afx_msg void OnSelchangeEnemyInteractions();
	afx_msg void OnSelchangeOnAllClear();
	afx_msg void OnSelchangeOnIdleScript();
	afx_msg void OnAttackCommonTarget();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
