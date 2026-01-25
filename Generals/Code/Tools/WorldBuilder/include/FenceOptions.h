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

// FenceOptions.h : header file
//

#include "TerrainSwatches.h"
#include "OptionsPanel.h"
#include "Common/AsciiString.h"
class WorldHeightMapEdit;
class MapObject;
/////////////////////////////////////////////////////////////////////////////
// FenceOptions dialog

class FenceOptions : public COptionsPanel
{
// Construction
public:
	FenceOptions(CWnd* pParent = nullptr);   ///< standard constructor

	~FenceOptions(void);   ///< standard destructor
	enum { NAME_MAX_LEN = 64 };
// Dialog Data
	//{{AFX_DATA(FenceOptions)
	enum { IDD = IDD_FENCE_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FenceOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  ///< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; ///< Modeless dialogs don't close on ESC, so eat this for modeless.
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FenceOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeFenceSpacingEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


protected:
	static FenceOptions *m_staticThis;
	static Bool				m_updating;
	static Int				m_currentObjectIndex;
	static Real				m_fenceSpacing;
	static Real				m_fenceOffset;

	CTreeCtrl					m_objectTreeView;
	MapObject					*m_objectsList;
	Bool							m_customSpacing;

protected:
	void addObject( MapObject *mapObject, const char *pPath, const char *name,
									Int objectNdx, HTREEITEM parent );
	HTREEITEM findOrAdd(HTREEITEM parent, const char *pLabel);
	Bool setObjectTreeViewSelection(HTREEITEM parent, Int selection);
	void updateObjectOptions();

public:
	static void update(void);
	static Bool hasSelectedObject(void);
	static Real getFenceSpacing(void) {return m_fenceSpacing;}
	static Real getFenceOffset(void) {return m_fenceOffset;}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
