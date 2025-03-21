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

#if !defined(AFX_VERIFYDLG_H__88E9C121_599B_11D3_B9DA_006097B90D93__INCLUDED_)
#define AFX_VERIFYDLG_H__88E9C121_599B_11D3_B9DA_006097B90D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VerifyDlg.h : header file
//

#include "TransDB.h"
#define IDSKIP		100

/////////////////////////////////////////////////////////////////////////////
// VerifyDlg dialog

class VerifyDlg : public CDialog
{
	CBitmapButton stop;
	CBitmapButton play;
	CBitmapButton pause;
	CWnd					*wave;
	CStatic				*text;
	NoxText				*nox_text;
	LANGINFO			*linfo;
	UINT					timer;
	char					wavefile[1024];
	CSliderCtrl *slider;
// Construction
public:
	VerifyDlg(NoxText *ntext, LangID langid, const char *path, CWnd* pParent = NULL);   // standard constructor
	void CloseAudio ( void );

// Dialog Data
	//{{AFX_DATA(VerifyDlg)
	enum { IDD = IDD_MATCHDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VerifyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(VerifyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNomatch();
	afx_msg void OnMatch();
	virtual void OnCancel();
	afx_msg void OnStop();
	afx_msg void OnPlay();
	afx_msg void OnPause();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VERIFYDLG_H__88E9C121_599B_11D3_B9DA_006097B90D93__INCLUDED_)
