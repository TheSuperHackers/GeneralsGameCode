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

#if !defined(AFX_RoadOptions_H__D3FF66C5_7107_4DAC_8A29_5EBAB5C3A24E__INCLUDED_)
#define AFX_RoadOptions_H__D3FF66C5_7107_4DAC_8A29_5EBAB5C3A24E__INCLUDED_

#pragma once
// RoadOptions.h : header file
//

#include "TerrainSwatches.h"
#include "OptionsPanel.h"
#include "Common/AsciiString.h"

class WorldHeightMapEdit;
/////////////////////////////////////////////////////////////////////////////
// RoadOptions dialog

class RoadOptions : public COptionsPanel
{
// Construction
public:
	RoadOptions(CWnd* pParent = NULL);   ///< standard constructor

	~RoadOptions(void);   ///< standard destructor
	enum { NAME_MAX_LEN = 64 };
// Dialog Data
	//{{AFX_DATA(RoadOptions)
	enum { IDD = IDD_ROAD_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RoadOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  ///< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; ///< Modeless dialogs don't close on ESC, so eat this for modeless.
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(RoadOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnTightCurve();
	afx_msg void OnAngled();
	afx_msg void OnBroadCurve();
	afx_msg void OnJoin();
	afx_msg void OnApplyRoad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


protected:
	static RoadOptions *m_staticThis;
	static Bool				m_updating;
	static Int				m_currentRoadIndex;
	static Int				m_numberOfRoads;
	static Int				m_numberOfBridges;
	static AsciiString	m_currentRoadName;
	static Bool				m_angleCorners;
	static Bool				m_tightCurve;
	static Bool				m_doJoin;
	CTreeCtrl					m_roadTreeView;

protected:
	void addRoad(char *pPath, Int objectNdx, HTREEITEM parent);
	HTREEITEM findOrAdd(HTREEITEM parent, const char *pLabel);
	Bool findAndSelect(HTREEITEM parent, AsciiString label);
	Bool setRoadTreeViewSelection(HTREEITEM parent, Int selection);
	void updateLabel(void);
	void ChangeRoadType(AsciiString newRoad);
	void SelectConnected(void);

public:
	static AsciiString getCurRoadName(void) {return m_currentRoadName;}
	static Bool isBridge(void) {return (m_currentRoadIndex >= m_numberOfRoads);}
	static Bool isAngled(void) {return m_angleCorners;}
	static Bool isTightCurve(void) {return m_tightCurve;}
	static Bool isJoin(void) {return m_doJoin;}
	static void updateSelection(void);
	static Bool selectionIsRoadsOnly(void);
	void applyToSelection(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RoadOptions_H__D3FF66C5_711D_4DAC_8A29_5EAAB5C3A23E__INCLUDED_)
