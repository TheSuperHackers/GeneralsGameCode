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

#if !defined(AFX_EDITCONDITION_H__64465BA2_AD81_4EFD_BAB4_93F66C90ECD1__INCLUDED_)
#define AFX_EDITCONDITION_H__64465BA2_AD81_4EFD_BAB4_93F66C90ECD1__INCLUDED_

#pragma once
// EditCondition.h : header file
//


class Condition;
class SidesList;

class CMyTreeCtrl : public CTreeCtrl
{
public:
	virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
};

/////////////////////////////////////////////////////////////////////////////
// EditCondition dialog

class EditCondition : public CDialog
{
// Construction
public:
	EditCondition(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(EditCondition)
	enum { IDD = IDD_ScriptCondition };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditCondition)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	void setCondition(Condition *pCond) {m_condition = pCond;}

protected:
	void formatConditionText(Int parmNdx);
protected:
	Condition *m_condition;
	Bool			m_updating;
	Bool			m_modifiedTextColor;
	CRichEditCtrl m_myEditCtrl;
	CHARRANGE m_curLinkChrg;
	Int				m_curEditParameter;
	CTreeCtrl	m_conditionTreeView;

protected:

	// Generated message map functions
	//{{AFX_MSG(EditCondition)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeConditionType();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITCONDITION_H__64465BA2_AD81_4EFD_BAB4_93F66C90ECD1__INCLUDED_)
