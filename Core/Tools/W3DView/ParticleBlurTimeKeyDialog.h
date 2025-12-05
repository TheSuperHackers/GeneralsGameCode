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

// ParticleBlurTimeKeyDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ParticleBlurTimeKeyDialogClass dialog

class ParticleBlurTimeKeyDialogClass : public CDialog
{
// Construction
public:
	ParticleBlurTimeKeyDialogClass(float blur_time, CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ParticleBlurTimeKeyDialogClass)
	enum { IDD = IDD_PARTICLE_BLUR_TIME_KEY };
	CSpinButtonCtrl	m_BlurTimeSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ParticleBlurTimeKeyDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ParticleBlurTimeKeyDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnOk2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////
	float					Get_Blur_Time (void) const { return m_BlurTime; }

private:

	/////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////
	float					m_BlurTime;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
