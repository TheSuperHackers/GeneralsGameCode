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

// Babylon.h : main header file for the BABYLON application
//

#if !defined(AFX_BABYLON_H__2BF3124B_3BA1_11D3_B9DA_006097B90D93__INCLUDED_)
#define AFX_BABYLON_H__2BF3124B_3BA1_11D3_B9DA_006097B90D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "TransDB.h"

/////////////////////////////////////////////////////////////////////////////
// CBabylonApp:
// See Babylon.cpp for the implementation of this class
//

class CBabylonApp : public CWinApp
{
public:
	CBabylonApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBabylonApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CBabylonApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

int ExcelRunning( void );
extern TransDB				*BabylonstrDB;
extern TransDB				*MainDB;
extern char						BabylonstrFilename[];
extern char						MainXLSFilename[];
extern char						RootPath[];
extern char						DialogPath[];
extern LangID					CurrentLanguage;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BABYLON_H__2BF3124B_3BA1_11D3_B9DA_006097B90D93__INCLUDED_)
