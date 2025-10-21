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

#if !defined(AFX_ADVANCEDANIMSHEET_H__67C01724_7C0D_11D3_9A3A_0090272E6F90__INCLUDED_)
#define AFX_ADVANCEDANIMSHEET_H__67C01724_7C0D_11D3_9A3A_0090272E6F90__INCLUDED_

#pragma once
// AdvancedAnimSheet.h : header file
//

#include "AnimMixingPage.h"
#include "AnimReportPage.h"

#define MAX_REPORT_ANIMS		128



/////////////////////////////////////////////////////////////////////////////
// CAdvancedAnimSheet

class CAdvancedAnimSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CAdvancedAnimSheet)

// Construction
public:
	CAdvancedAnimSheet(CWnd *pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

	// Pages in the property sheet.
	CAnimMixingPage	m_MixingPage;
	CAnimReportPage	m_ReportPage;

	// Indeces of animations selected in the mixing page.
	DynamicVectorClass<int>	m_SelectedAnims;

// Operations
public:

	int				GetAnimCount (void);
	HAnimClass **	GetAnims (void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvancedAnimSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAdvancedAnimSheet();

	// Generated message map functions
protected:

	void				LoadAnims (void);

	HAnimClass		*Anims[MAX_REPORT_ANIMS];
	int				AnimCount;
	bool				AnimsValid;


	//{{AFX_MSG(CAdvancedAnimSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVANCEDANIMSHEET_H__67C01724_7C0D_11D3_9A3A_0090272E6F90__INCLUDED_)
