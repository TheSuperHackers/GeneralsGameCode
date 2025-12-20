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

// propedit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PropEdit dialog

class PropEdit : public CDialog
{
// Construction
public:
	PropEdit(AsciiString* key, Dict::DataType* type, AsciiString* value, Bool valueOnly, CWnd *parent = NULL);

// Dialog Data
	//{{AFX_DATA(PropEdit)
	enum { IDD = IDD_PROPEDIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PropEdit)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	AsciiString* m_key;
	Dict::DataType* m_type;
	AsciiString* m_value;
	Bool m_valueOnly;
	int m_updating;

	void validate();

	// Generated message map functions
	//{{AFX_MSG(PropEdit)
	afx_msg void OnChangeKeyname();
	afx_msg void OnEditchangeKeytype();
	afx_msg void OnCloseupKeytype();
	afx_msg void OnSelchangeKeytype();
	afx_msg void OnChangeValue();
	virtual BOOL OnInitDialog();
	afx_msg void OnPropbool();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
