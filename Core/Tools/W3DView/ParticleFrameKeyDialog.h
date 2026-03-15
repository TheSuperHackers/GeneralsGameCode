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

// ParticleFrameKeyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ParticleFrameKeyDialogClass dialog

class ParticleFrameKeyDialogClass : public CDialog
{
// Construction
public:
	ParticleFrameKeyDialogClass(float frame,CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ParticleFrameKeyDialogClass)
	enum { IDD = IDD_PARTICLE_FRAME_KEY };
	CSpinButtonCtrl	m_FrameSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ParticleFrameKeyDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ParticleFrameKeyDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////
	float					Get_Frame (void) const { return m_Frame; }

private:

	/////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////
	float					m_Frame;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
