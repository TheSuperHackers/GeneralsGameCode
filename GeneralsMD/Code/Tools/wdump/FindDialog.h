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

#if !defined(AFX_FINDDIALOG_H__C38C0FF8_E89B_11D3_A398_009027447394__INCLUDED_)
#define AFX_FINDDIALOG_H__C38C0FF8_E89B_11D3_A398_009027447394__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FindDialog.h : header file
//

// Defines.
#define MAX_FIND_STRING_LENGTH 255

/////////////////////////////////////////////////////////////////////////////
// FindDialog dialog

class FindDialog : public CDialog
{
// Construction
public:
	FindDialog(CWnd* pParent = NULL);   // standard constructor

	static const char *String()
	{
		return (_FindString);
	}

	static void Compare (const char *string)
	{
		_Found |= (strstr (string, _FindString) != NULL);
	}

	static bool Found()
	{
		return (_Found);
	}

	static void Found (bool found)
	{
		_Found = found;
	}

// Dialog Data
	//{{AFX_DATA(FindDialog)
	enum { IDD = IDD_TOOLS_FIND };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FindDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FindDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFindString();
	afx_msg void OnUpdateFindString();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	static bool	_Found;
	static char _FindString [MAX_FIND_STRING_LENGTH + 1];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FINDDIALOG_H__C38C0FF8_E89B_11D3_A398_009027447394__INCLUDED_)
