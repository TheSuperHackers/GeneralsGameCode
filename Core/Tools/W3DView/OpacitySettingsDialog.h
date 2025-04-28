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

#if !defined(AFX_OPACITYSETTINGSDIALOG_H__52B0C804_0D48_11D3_A031_00104B791122__INCLUDED_)
#define AFX_OPACITYSETTINGSDIALOG_H__52B0C804_0D48_11D3_A031_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpacitySettingsDialog.h : header file
//

// Forward declarations
class ColorBarClass;

/////////////////////////////////////////////////////////////////////////////
// OpacitySettingsDialogClass dialog

class OpacitySettingsDialogClass : public CDialog
{
// Construction
public:
	OpacitySettingsDialogClass(float opacity, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(OpacitySettingsDialogClass)
	enum { IDD = IDD_OPACITY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OpacitySettingsDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OpacitySettingsDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		////////////////////////////////////////////////////////////////
		//	Public methods
		////////////////////////////////////////////////////////////////
		float					Get_Opacity (void) const	{ return m_Opacity; }
		
	private:

		////////////////////////////////////////////////////////////////
		//	Private member data
		////////////////////////////////////////////////////////////////
		ColorBarClass *	m_OpacityBar;
		float					m_Opacity;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPACITYSETTINGSDIALOG_H__52B0C804_0D48_11D3_A031_00104B791122__INCLUDED_)
