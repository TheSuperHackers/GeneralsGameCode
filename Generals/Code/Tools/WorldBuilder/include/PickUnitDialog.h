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

#pragma once

// PickUnitDialog.h : header file
//

#include "Common/AsciiString.h"
#include "Common/ThingSort.h"

#define  BUILD_PICK_PANEL_SECTION "PickUnitWindow"

class MapObject;
class ThingTemplate;
/////////////////////////////////////////////////////////////////////////////
// PickUnitDialog dialog

class PickUnitDialog : public CDialog
{
protected:
	enum { NAME_MAX_LEN = 64 };
	void addObject( MapObject *mapObject, const char *pPath, Int index, HTREEITEM parent );
	HTREEITEM findOrAdd(HTREEITEM parent, const char *pLabel);
	Int				m_currentObjectIndex;
	char			m_currentObjectName[NAME_MAX_LEN];
	CTreeCtrl		m_objectTreeView;
	MapObject		*m_objectsList;
	Bool			m_allowable[ES_NUM_SORTING_TYPES];
	Bool			m_factionOnly;

// Construction
public:
	PickUnitDialog(CWnd* pParent = nullptr);   // standard constructor
	PickUnitDialog(UINT id, CWnd* pParent = nullptr);   // standard constructor
	~PickUnitDialog(void);   ///< standard destructor

// Dialog Data
	//{{AFX_DATA(PickUnitDialog)
	enum { IDD = IDD_PICKUNIT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PickUnitDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PickUnitDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	AsciiString getPickedUnit(void);
	Bool IsAllowableType(EditorSortingType sort, Bool isBuildable);
	void SetAllowableType(EditorSortingType sort);
	const ThingTemplate* getPickedThing(void);
	void SetFactionOnly(Bool faction) {m_factionOnly = faction;}
	void SetupAsPanel(void);
};

class ReplaceUnitDialog : public PickUnitDialog
{
public:
	ReplaceUnitDialog(CWnd* pParent = nullptr);   // standard constructor

	void setMissing(AsciiString name) {m_missingName = name;};

protected:

	// Generated message map functions
	//{{AFX_MSG(ReplaceUnitDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	AsciiString m_missingName;

	enum { IDD = IDD_REPLACEUNIT };
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
