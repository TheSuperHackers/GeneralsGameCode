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

// BaseBuildProps.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BaseBuildProps dialog

class BaseBuildProps : public CDialog
{
// Construction
public:
	BaseBuildProps(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(BaseBuildProps)
	enum { IDD = IDD_BASE_BUILDING_PROPS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BaseBuildProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(BaseBuildProps)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	AsciiString m_name;
	AsciiString m_script;
	Int m_health;
	Bool m_unsellable;

public:
	void setProps(AsciiString name, AsciiString script, Int health, Bool unsellable);
	AsciiString getName(void) {return m_name;}
	AsciiString getScript(void) {return m_script;}
	Int getHealth(void) {return m_health;}
	Bool getUnsellable(void) {return m_unsellable;}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
