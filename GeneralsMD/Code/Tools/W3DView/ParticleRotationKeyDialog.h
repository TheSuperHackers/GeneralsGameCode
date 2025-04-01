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

#if !defined(AFX_PARTICLEROTATIONKEYDIALOG_H__2B0B9CFD_19BC_4910_8AB2_813120740C69__INCLUDED_)
#define AFX_PARTICLEROTATIONKEYDIALOG_H__2B0B9CFD_19BC_4910_8AB2_813120740C69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParticleRotationKeyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ParticleRotationKeyDialogClass dialog

class ParticleRotationKeyDialogClass : public CDialog
{
// Construction
public:
	ParticleRotationKeyDialogClass(float rotation,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ParticleRotationKeyDialogClass)
	enum { IDD = IDD_PARTICLE_ROTATION_KEY };
	CSpinButtonCtrl	m_RotationSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ParticleRotationKeyDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ParticleRotationKeyDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	
	/////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////
	float					Get_Rotation (void) const { return m_Rotation; }

private:

	/////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////
	float					m_Rotation;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLEROTATIONKEYDIALOG_H__2B0B9CFD_19BC_4910_8AB2_813120740C69__INCLUDED_)
