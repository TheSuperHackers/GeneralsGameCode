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

// ObjectOptions.h : header file
//

#include "ObjectPreview.h"
#include "OptionsPanel.h"
#include "Common/AsciiString.h"
class WorldHeightMapEdit;
class MapObject;
/////////////////////////////////////////////////////////////////////////////
// ObjectOptions dialog

class ObjectOptions : public COptionsPanel
{
// Construction
public:
	ObjectOptions(CWnd* pParent = NULL);   ///< standard constructor

	~ObjectOptions(void);   ///< standard destructor
	enum { NAME_MAX_LEN = 64 };
// Dialog Data
	//{{AFX_DATA(ObjectOptions)
	enum { IDD = IDD_OBJECT_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ObjectOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK(){return;};  ///< Modeless dialogs don't OK, so eat this for modeless.
	virtual void OnCancel(){return;}; ///< Modeless dialogs don't close on ESC, so eat this for modeless.
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ObjectOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditchangeOwningteam();
	afx_msg void OnCloseupOwningteam();
	afx_msg void OnSelchangeOwningteam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


protected:
	static ObjectOptions *m_staticThis;
	static Bool				m_updating;
	static Int				m_currentObjectIndex;
	static char				m_currentObjectName[NAME_MAX_LEN];
	static AsciiString	m_curOwnerName;

	CTreeCtrl					m_objectTreeView;
	MapObject					*m_objectsList;
	ObjectPreview			m_objectPreview;

protected:
	void addObject( MapObject *mapObject, const char *pPath,
									Int objectNdx, HTREEITEM parent );
	HTREEITEM findOrAdd(HTREEITEM parent, const char *pLabel);
	HTREEITEM findOrDont(const char *pLabel);
	HTREEITEM _FindOrDont(const char* pLabel, HTREEITEM startPoint);
	Bool setObjectTreeViewSelection(HTREEITEM parent, Int selection);
	void updateLabel();
	static MapObject *getCurMapObject(void);

public:
	static const char * getCurObjectName(void) {return m_currentObjectName;};
	static MapObject *duplicateCurMapObjectForPlace(const Coord3D* loc, Real angle, Bool checkPlayers = true);
	static MapObject *getObjectNamed(AsciiString name);
	static Int getObjectNamedIndex(const AsciiString& name);
	static void selectObject(const MapObject* pObj);
	static Real getCurObjectHeight(void);
	static void update();
	static AsciiString getCurGdfName(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
