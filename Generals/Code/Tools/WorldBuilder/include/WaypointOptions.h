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

#if !defined(AFX_WaypointOptions_H__6B56E20C_582E_4132_A251_879097C8852C__INCLUDED_)
#define AFX_WaypointOptions_H__6B56E20C_582E_4132_A251_879097C8852C__INCLUDED_

#pragma once
// WaypointOptions.h : header file
//
#include "OptionsPanel.h"
#include "Common/WellKnownKeys.h"

class MapObject;
class PolygonTrigger;
class MovePolygonUndoable;
/////////////////////////////////////////////////////////////////////////////
// WaypointOptions dialog

class WaypointOptions : public COptionsPanel
{

// Construction
public:
	WaypointOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(WaypointOptions)
	enum { IDD = IDD_WAYPOINT_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WaypointOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  //!< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; //!< Modeless dialogs don't close on ESC, so eat this for modeless.
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WaypointOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeWaypointnameEdit();
	afx_msg void OnChangeSelectedWaypoint();
	afx_msg void OnEditWaypointLocationX();
	afx_msg void OnEditWaypointLocationY();
	afx_msg void OnEditchangeWaypointlabel1Edit();
	afx_msg void OnEditchangeWaypointlabel2Edit();
	afx_msg void OnEditchangeWaypointlabel3Edit();
	afx_msg void OnWaypointBidirectional();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	static WaypointOptions *m_staticThis;  ///< Reference to the floating panel so SetWidth and SetFeather can be static.
	Bool		m_updating; ///<true if the ui is updating itself.
	MovePolygonUndoable *m_moveUndoable;
	Int									 m_originalHeight;
protected:
	void updateTheUI(void);
	void changeWaypointLabel(Int editControlID, NameKeyType key);

public:
	static void update(void);
	static MapObject *getSingleSelectedWaypoint(void);
	static PolygonTrigger *getSingleSelectedPolygon(void);
	static Bool isUnique(AsciiString name, MapObject* theMapObj = NULL);
	static AsciiString GenerateUniqueName(Int id);


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WaypointOptions_H__6B56E20C_582E_4030_A251_879097C8853C__INCLUDED_)
