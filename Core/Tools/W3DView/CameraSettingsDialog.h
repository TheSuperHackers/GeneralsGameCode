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

#if !defined(AFX_CAMERASETTINGSDIALOG_H__7E43F424_8191_11D3_A064_00104B791122__INCLUDED_)
#define AFX_CAMERASETTINGSDIALOG_H__7E43F424_8191_11D3_A064_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CameraSettingsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CameraSettingsDialogClass dialog

class CameraSettingsDialogClass : public CDialog
{
// Construction
public:
	CameraSettingsDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CameraSettingsDialogClass)
	enum { IDD = IDD_CAMERA_SETTINGS };
	CSpinButtonCtrl	m_LensSpin;
	CSpinButtonCtrl	m_FarClipSpin;
	CSpinButtonCtrl	m_VFOVSpin;
	CSpinButtonCtrl	m_NearClipSpin;
	CSpinButtonCtrl	m_HFOVSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CameraSettingsDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CameraSettingsDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnFovCheck();
	afx_msg void OnClipPlaneCheck();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	///////////////////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////////////////
	void			Update_Camera_Lens (void);
	void			Update_FOV (void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERASETTINGSDIALOG_H__7E43F424_8191_11D3_A064_00104B791122__INCLUDED_)
