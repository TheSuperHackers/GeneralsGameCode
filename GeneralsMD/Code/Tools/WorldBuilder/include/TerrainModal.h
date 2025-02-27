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

// TerrainModal.h : header file
//

#include "TerrainSwatches.h"
#include "Common/AsciiString.h"
class WorldHeightMapEdit;
/////////////////////////////////////////////////////////////////////////////
// TerrainModal dialog

class TerrainModal : public CDialog
{
// Construction
public:
	TerrainModal(AsciiString path, WorldHeightMapEdit *pMap, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TerrainModal)
	enum { IDD = IDD_TERRAIN_MODAL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TerrainModal)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TerrainModal)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	Int					m_currentFgTexture;
	AsciiString m_pathToReplace;
	CTreeCtrl		m_terrainTreeView;
	TerrainSwatches		m_terrainSwatches;
	WorldHeightMapEdit *m_map;

protected:
	void addTerrain(char *pPath, Int terrainNdx, HTREEITEM parent);
	HTREEITEM findOrAdd(HTREEITEM parent, const char *pLabel);
	void updateLabel(void);
	void updateTextures(void);
	Bool setTerrainTreeViewSelection(HTREEITEM parent, Int selection);

public:
	Int getNewNdx(void) {return m_currentFgTexture;};

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
