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

#pragma once

// EditCoordParameter.h : header file
//
#include "GameLogic/Scripts.h"
class SidesList;
/////////////////////////////////////////////////////////////////////////////
// EditCoordParameter dialog

class EditCoordParameter : public CDialog
{
friend class EditParameter;
// Construction
public:
	EditCoordParameter(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(EditCoordParameter)
	enum { IDD = IDD_EDIT_COORD_PARAMETER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EditCoordParameter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation


protected:

protected:
	Parameter		*m_parameter;
	Coord3D			 m_coord;

protected:

	// Generated message map functions
	//{{AFX_MSG(EditCoordParameter)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
