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

// noxstring.h : main header file for the NOXSTRING application
//

#if !defined(AFX_NOXSTRING_H__2BF3124B_3BA1_11D3_B9DA_006097B90D93__INCLUDED_)
#define AFX_NOXSTRING_H__2BF3124B_3BA1_11D3_B9DA_006097B90D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "TransDB.h"

/////////////////////////////////////////////////////////////////////////////
// CNoxstringApp:
// See noxstring.cpp for the implementation of this class
//

class CNoxstringApp : public CWinApp
{
public:
	CNoxstringApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNoxstringApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CNoxstringApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

int ExcelRunning( void );
extern TransDB				*NoxstrDB;
extern TransDB				*MainDB; 
extern char						NoxstrFilename[];
extern char						MainXLSFilename[];
extern char						RootPath[];
extern char						DialogPath[];
extern LangID					CurrentLanguage;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NOXSTRING_H__2BF3124B_3BA1_11D3_B9DA_006097B90D93__INCLUDED_)
