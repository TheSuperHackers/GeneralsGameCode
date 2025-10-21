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

#if !defined(AFX_EXPORTSCRIPTSOPTIONS_H__706D8D87_E01C_431A_ADB8_DFC4CA8A8422__INCLUDED_)
#define AFX_EXPORTSCRIPTSOPTIONS_H__706D8D87_E01C_431A_ADB8_DFC4CA8A8422__INCLUDED_

#pragma once
// ExportScriptsOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ExportScriptsOptions dialog

class ExportScriptsOptions : public CDialog
{
// Construction
public:
	ExportScriptsOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ExportScriptsOptions)
	enum { IDD = IDD_EXPORT_SCRIPTS_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ExportScriptsOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static Bool m_units;
	static Bool m_waypoints;
	static Bool m_triggers;
	static Bool m_allScripts;
	static Bool m_sides;

public:
	Bool getDoUnits(void) {return m_units;}
	Bool getDoWaypoints(void) {return m_waypoints;}
	Bool getDoTriggers(void) {return m_triggers;}
	Bool getDoAllScripts(void) {return m_allScripts;}
	Bool getDoSides(void) {return m_sides;}

protected:

	// Generated message map functions
	//{{AFX_MSG(ExportScriptsOptions)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTSCRIPTSOPTIONS_H__706D8D87_E01C_431A_ADB8_DFC4CA8A8422__INCLUDED_)
