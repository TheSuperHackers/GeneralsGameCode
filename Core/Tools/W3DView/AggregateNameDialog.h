/*
**	Command & Conquer Renegade(tm)
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

// AggregateNameDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AggregateNameDialogClass dialog

class AggregateNameDialogClass : public CDialog
{
// Construction
public:
	AggregateNameDialogClass(CWnd* pParent = nullptr);
	AggregateNameDialogClass(UINT resource_id, const CString &def_name, CWnd* pParent = nullptr);

// Dialog Data
	//{{AFX_DATA(AggregateNameDialogClass)
	enum { IDD = IDD_MAKE_AGGREGATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AggregateNameDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AggregateNameDialogClass)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		////////////////////////////////////////////////////////////////
		//
		//	Public methods
		//
		const CString &			Get_Name (void) const				{ return m_Name; }
		void							Set_Name (const CString &name)	{ m_Name = name; }

	private:

		////////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		CString						m_Name;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
