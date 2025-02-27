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

// EditAction.h : header file
//


class ScriptAction;
class SidesList;

/////////////////////////////////////////////////////////////////////////////
// EditAction dialog

class EditAction : public CDialog
{
// Construction
public:
	EditAction(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(EditAction)
	enum { IDD = IDD_ScriptAction };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditAction)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	void setAction(ScriptAction *pAction) {m_action = pAction;}

protected:
	void formatScriptActionText(Int parmNdx);
protected:
	ScriptAction *m_action;
	Bool			m_updating;
	Bool			m_modifiedTextColor;
	CRichEditCtrl m_myEditCtrl;
	CHARRANGE m_curLinkChrg;
	Int				m_curEditParameter;
	CTreeCtrl	m_actionTreeView;

protected:

	// Generated message map functions
	//{{AFX_MSG(EditAction)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeScriptActionType();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
