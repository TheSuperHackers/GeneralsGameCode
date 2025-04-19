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

#if !defined(AFX_VOLUMERANDOMDIALOG_H__F22DB07B_0EFF_11D3_A034_00104B791122__INCLUDED_)
#define AFX_VOLUMERANDOMDIALOG_H__F22DB07B_0EFF_11D3_A034_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VolumeRandomDialog.h : header file
//

// Forward declarations
class Vector3Randomizer;

/////////////////////////////////////////////////////////////////////////////
//
// VolumeRandomDialogClass dialog
//
/////////////////////////////////////////////////////////////////////////////
class VolumeRandomDialogClass : public CDialog
{
// Construction
public:
	VolumeRandomDialogClass (Vector3Randomizer *randomizer, CWnd *pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(VolumeRandomDialogClass)
	enum { IDD = IDD_VOLUME_RANDOMIZER };
	CSpinButtonCtrl	m_SphereRadiusSpin;
	CSpinButtonCtrl	m_CylinderRadiusSpin;
	CSpinButtonCtrl	m_CylinderHeightSpin;
	CSpinButtonCtrl	m_BoxZSpin;
	CSpinButtonCtrl	m_BoxYSpin;
	CSpinButtonCtrl	m_BoxXSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VolumeRandomDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VolumeRandomDialogClass)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnBoxRadio();
	afx_msg void OnCylinderRadio();
	afx_msg void OnSphereRadio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		//////////////////////////////////////////////////////////////////////////////
		//	Public methods
		//////////////////////////////////////////////////////////////////////////////
		Vector3Randomizer *	Get_Randomizer (void) const { return m_Randomizer; }

	protected:

		//////////////////////////////////////////////////////////////////////////////
		//	Protected methods
		//////////////////////////////////////////////////////////////////////////////
		void						Update_Enable_State (void);

	private:

		//////////////////////////////////////////////////////////////////////////////
		//	Private member data
		//////////////////////////////////////////////////////////////////////////////
		Vector3Randomizer *	m_Randomizer;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOLUMERANDOMDIALOG_H__F22DB07B_0EFF_11D3_A034_00104B791122__INCLUDED_)
