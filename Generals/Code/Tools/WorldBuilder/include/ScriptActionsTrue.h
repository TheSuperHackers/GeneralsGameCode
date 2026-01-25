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

// ScriptActionsTrue.h : header file
//
class Script;
class ScriptAction;
class SidesList;
/////////////////////////////////////////////////////////////////////////////
// ScriptActionsTrue dialog

class ScriptActionsTrue : public CPropertyPage
{
	DECLARE_DYNCREATE(ScriptActionsTrue)

// Construction
public:
	ScriptActionsTrue();
	~ScriptActionsTrue();

// Dialog Data
	//{{AFX_DATA(ScriptActionsTrue)
	enum { IDD = IDD_ScriptActionsTrue };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(ScriptActionsTrue)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void setScript(Script *pScript) {m_script = pScript;}

protected:
	Script *m_script;	 // Doesn't change.
	ScriptAction *m_action; // Currently selected action.
	Int					m_index; // Index of whatever is currently selected.

protected:
	void enableUI(void);
	void loadList(void);
	Bool doMoveDown(void);

protected:
	// Generated message map functions
	//{{AFX_MSG(ScriptActionsTrue)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditAction();
	afx_msg void OnSelchangeActionList();
	afx_msg void OnDblclkActionList();
	afx_msg void OnNew();
	afx_msg void OnDelete();
	afx_msg void OnCopy();
	afx_msg void OnMoveDown();
	afx_msg void OnMoveUp();
	afx_msg void OnChangeEditComment();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
