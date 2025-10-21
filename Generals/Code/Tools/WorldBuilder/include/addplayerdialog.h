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

#if !defined(AFX_ADDPLAYERDIALOG_H__96179004_35C9_436A_A302_A777E77F8F04__INCLUDED_)
#define AFX_ADDPLAYERDIALOG_H__96179004_35C9_436A_A302_A777E77F8F04__INCLUDED_

#pragma once
// addplayerdialog.h : header file
//

#include "GameLogic/SidesList.h"

/////////////////////////////////////////////////////////////////////////////
// AddPlayerDialog dialog

class AddPlayerDialog : public CDialog
{
private:
	AsciiString m_side;
	AsciiString m_addedSide;

// Construction
public:
	AddPlayerDialog(AsciiString side, CWnd* pParent = NULL);   // standard constructor

	AsciiString getAddedSide() { return m_addedSide; }

// Dialog Data
	//{{AFX_DATA(AddPlayerDialog)
	enum { IDD = IDD_ADDPLAYER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AddPlayerDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AddPlayerDialog)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeCombo1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDPLAYERDIALOG_H__96179004_35C9_436A_A302_A777E77F8F04__INCLUDED_)
