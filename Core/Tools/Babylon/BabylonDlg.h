/*
**	Command & Conquer Generals Zero Hour(tm)
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

// BabylonDlg.h : header file
//

#if !defined(AFX_BABYLONDLG_H__2BF3124D_3BA1_11D3_B9DA_006097B90D93__INCLUDED_)
#define AFX_BABYLONDLG_H__2BF3124D_3BA1_11D3_B9DA_006097B90D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#include "TransDB.h"

typedef enum
{
	SAME_LINE,
	NEW_LINE

} LogFormat;


typedef struct
{
	int new_strings;
	int deleted_strings;
	int modified_strings;
	int new_labels;
	int deleted_labels;
	int modified_labels;
	int skipped_labels;
	int updated_comments;
	int updated_contexts;
	int updated_waves;
	int updated_speakers;
	int updated_listeners;
	int updated_maxlen;
	int changes;



} UPDATEINFO;

class CBabylonDlgAutoProxy;

/////////////////////////////////////////////////////////////////////////////
// CBabylonDlg dialog

class CBabylonDlg : public CDialog
{
	DECLARE_DYNAMIC(CBabylonDlg);
	friend class CBabylonDlgAutoProxy;
	CProgressCtrl *progress;
	CStatic *percent;
	int progress_pos;
	int progress_range;
	int max_index;
	CComboBox *combo;
	int operate_always;



// Construction
public:
	int ValidateStrFile ( const char *filename );
	int MatchText ( BabylonText *text, BabylonLabel *label, BabylonText **match );
	int RetranslateText ( BabylonText *text, BabylonText *label );
	void VerifyDialog( TransDB *db, LangID langid);
	void VerifyTranslations( TransDB *db, LangID langid ) ;
	int	CanProceed ( void );
	int	CanOperate ( void );
	int SaveMainDB ( void );
	int UpdateLabel ( BabylonLabel *source, BabylonLabel *destination, UPDATEINFO &info, int update = TRUE, int skip = FALSE);
	int UpdateDB ( TransDB *source, TransDB *destination, int update = TRUE);
	void ProgressComplete ( void );
	void SetProgress ( int pos );
	void InitProgress ( int range );
	int SaveLog ( void );
	void Status ( const char *string, int log = TRUE);
	void Log ( const char *string, LogFormat format = NEW_LINE );
	CBabylonDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CBabylonDlg();
	int LoadStrFile ( TransDB *db, const char *fileaname, void (*cb ) (void ) = NULL );
	void Ready ( void ) { Status ( "Ready", FALSE ); ProgressComplete(); };

// Dialog Data
	//{{AFX_DATA(CBabylonDlg)
	enum { IDD = IDD_BABYLON_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBabylonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBabylonDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// Generated message map functions
	//{{AFX_MSG(CBabylonDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnViewdbs();
	afx_msg void OnReload();
	afx_msg void OnUpdate();
	afx_msg void OnSave();
	afx_msg void OnWarnings();
	afx_msg void OnErrors();
	afx_msg void OnChanges();
	afx_msg void OnExport();
	afx_msg void OnImport();
	afx_msg void OnGenerate();
	afx_msg void OnVerifyDialog();
	afx_msg void OnTranslations();
	afx_msg void OnSelchangeCombolang();
	afx_msg void OnReports();
	afx_msg void OnDblclkCombolang();
	afx_msg void OnReset();
	afx_msg void OnSent();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CBabylonDlg *MainDLG;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BABYLONDLG_H__2BF3124D_3BA1_11D3_B9DA_006097B90D93__INCLUDED_)
