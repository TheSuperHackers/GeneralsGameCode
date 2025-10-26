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

// AnimReportPage.h : header file
//

class HAnimClass;
class CAdvancedAnimSheet;

/////////////////////////////////////////////////////////////////////////////
// CAnimReportPage dialog

class CAnimReportPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CAnimReportPage)

// Construction
public:
	CAnimReportPage(CAdvancedAnimSheet *sheet = NULL);
	~CAnimReportPage();

// Dialog Data
	//{{AFX_DATA(CAnimReportPage)
	enum { IDD = IDD_PROP_PAGE_ADVANIM_REPORT };
	CListCtrl	m_AnimReport;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CAnimReportPage)
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void		FillListControl (void);
	int		FindItem (const char *item_name);
	void		MakeChannelStr(int bone_idx, HAnimClass* hanim, char channels[6]);

	CAdvancedAnimSheet *m_Sheet;

	// Generated message map functions
	//{{AFX_MSG(CAnimReportPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
