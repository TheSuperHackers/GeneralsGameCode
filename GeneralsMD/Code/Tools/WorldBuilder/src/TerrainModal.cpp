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

// TerrainModal.cpp : implementation file
//

#define DEFINE_TERRAIN_TYPE_NAMES

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "TerrainModal.h"
#include "TerrainMaterial.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "Common/TerrainTypes.h"

/////////////////////////////////////////////////////////////////////////////
// TerrainModal dialog


TerrainModal::TerrainModal(AsciiString path, WorldHeightMapEdit *pMap, CWnd* pParent  /*=NULL*/)
	: CDialog(TerrainModal::IDD, pParent)
{
	//{{AFX_DATA_INIT(TerrainModal)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pathToReplace = path;
	m_map = pMap;
}


void TerrainModal::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TerrainModal)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


void TerrainModal::updateLabel(void)
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (!pDoc) return;

	const char *tName = pDoc->GetHeightMap()->getTexClassUiName(m_currentFgTexture).str();
	if (tName == NULL || tName[0] == 0) {
		tName = pDoc->GetHeightMap()->getTexClassUiName(m_currentFgTexture).str();
	}
	if (tName == NULL) {
		return;
	}
	const char *leaf = tName;
	while (*tName) {
		if ((tName[0] == '\\' || tName[0] == '/')&& tName[1]) {
			leaf = tName+1;
		}
		tName++;
	}
	CWnd *pLabel = GetDlgItem(IDC_TERRAIN_NAME);
	if (pLabel) {
		pLabel->SetWindowText(leaf);
	}
}

/// Setup the controls in the dialog.
BOOL TerrainModal::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd *pWnd = GetDlgItem(IDC_TERRAIN_TREEVIEW);
	CRect rect;
	pWnd->GetWindowRect(&rect);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_terrainTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, rect, this, IDC_TERRAIN_TREEVIEW);
	m_terrainTreeView.ShowWindow(SW_SHOW);

	pWnd = GetDlgItem(IDC_TERRAIN_SWATCHES);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_terrainSwatches.Create(NULL, "", WS_CHILD, rect, this, IDC_TERRAIN_SWATCHES);
	m_terrainSwatches.ShowWindow(SW_SHOW);

	pWnd = GetDlgItem(IDC_MISSING_NAME);
	if (pWnd) pWnd->SetWindowText(m_pathToReplace.str());
	m_currentFgTexture = 0;
	updateTextures();
	TerrainMaterial::setFgTexClass(m_currentFgTexture);
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM TerrainModal::findOrAdd(HTREEITEM parent, const char *pLabel)
{
	TVINSERTSTRUCT ins;
	char buffer[_MAX_PATH];
	::memset(&ins, 0, sizeof(ins));
	HTREEITEM child = m_terrainTreeView.GetChildItem(parent);
	while (child != NULL) {
		ins.item.mask = TVIF_HANDLE|TVIF_TEXT;
		ins.item.hItem = child;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = sizeof(buffer)-2;
		m_terrainTreeView.GetItem(&ins.item);
		if (strcmp(buffer, pLabel) == 0) {
			return(child);
		}
		child = m_terrainTreeView.GetNextSiblingItem(child);
	}

	// not found, so add it.
	::memset(&ins, 0, sizeof(ins));
	ins.hParent = parent;
	ins.hInsertAfter = TVI_LAST;
	ins.item.mask = TVIF_PARAM|TVIF_TEXT;
	ins.item.lParam = -1;
	ins.item.pszText = const_cast<LPSTR>(pLabel);
	ins.item.cchTextMax = strlen(pLabel);
	child = m_terrainTreeView.InsertItem(&ins);
	return(child);
}

/** Add the terrain path to the tree view. */
void TerrainModal::addTerrain(char *pPath, Int terrainNdx, HTREEITEM parent)
{
	TerrainType *terrain = TheTerrainTypes->findTerrain( WorldHeightMapEdit::getTexClassName( terrainNdx ) );
	Bool doAdd = FALSE;
	char buffer[_MAX_PATH];

	//
	// if we have a 'terrain' entry, it means that our terrain index was properly defined
	// in an INI file, otherwise it was from legacy GDF stuff.  We will sort all of
	// the legacy GDF terrain entries in a tree leaf all their own while the others are
	// sorted according to a field specified in INI
	//
	if( terrain )
	{

		for( TerrainClass i = TERRAIN_NONE; i < TERRAIN_NUM_CLASSES; i = (TerrainClass)(i + 1) )
		{

			if( terrain->getClass() == i )
			{

				parent = findOrAdd( parent, terrainTypeNames[ i ] );
				break;  // exit for

			}  // end if

		}  // end for i

		strcpy( buffer, terrain->getName().str() );

		doAdd = TRUE;

	}  // end if
	else
	{

		// all these old entries we will put in a tree for legacy GDF items
		parent = findOrAdd( parent, "**LegacyGDF" );

		Int i=0;
		while (pPath[i] && i<sizeof(buffer)) {
			if (pPath[i] == 0) {
				return;
			}
			if (pPath[i] == '\\' || pPath[i] == '/') {
				if (i>0 && (i>1 || buffer[0]!='.') ) { // skip the "." directory.
					buffer[i] = 0;
					parent = findOrAdd(parent, buffer);
				}
				pPath+= i+1;
				i = 0;
			}
			buffer[i] = pPath[i];
			buffer[i+1] = 0;  // terminate at next char
			i++;
			doAdd = TRUE;
		}

	}  // end else

	if (doAdd)
	{
		TVINSERTSTRUCT ins;

		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_LAST;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = terrainNdx;
		ins.item.pszText = buffer;
		ins.item.cchTextMax = strlen(buffer)+2;
		m_terrainTreeView.InsertItem(&ins);
	}

}

//* Create the tree view of textures from the textures in pMap. */
void TerrainModal::updateTextures(void)
{
	m_terrainTreeView.DeleteAllItems();
	Int i;
	for (i=0; i<WorldHeightMapEdit::getNumTexClasses(); i++) {
		if (m_map->isTexClassUsed(i)) {
			if (m_currentFgTexture == i) {
				m_currentFgTexture++;
			}
			// continue;
		}
		const char *tName = WorldHeightMapEdit::getTexClassName(i).str();
		char path[_MAX_PATH];
		strncpy(path, tName, _MAX_PATH-2);
		addTerrain(path, i, TVI_ROOT);
	}
	setTerrainTreeViewSelection(TVI_ROOT, m_currentFgTexture);
	updateLabel();
}
/// Set the selected texture in the tree view.
Bool TerrainModal::setTerrainTreeViewSelection(HTREEITEM parent, Int selection)
{
	TVITEM item;
	char buffer[_MAX_PATH];
	::memset(&item, 0, sizeof(item));
	HTREEITEM child = m_terrainTreeView.GetChildItem(parent);
	while (child != NULL) {
		item.mask = TVIF_HANDLE|TVIF_PARAM;
		item.hItem = child;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer)-2;
		m_terrainTreeView.GetItem(&item);
		if (item.lParam == selection) {
			m_terrainTreeView.SelectItem(child);
			return(true);
		}
		if (setTerrainTreeViewSelection(child, selection)) {
			return(true);
		}
		child = m_terrainTreeView.GetNextSiblingItem(child);
	}
	return(false);
}



BOOL TerrainModal::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;
	if (pHdr->hdr.hwndFrom == m_terrainTreeView.m_hWnd) {
		if (pHdr->hdr.code == TVN_SELCHANGED) {
			HTREEITEM hItem = m_terrainTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM;
			item.hItem = hItem;
			m_terrainTreeView.GetItem(&item);
			if (item.lParam >= 0) {
				m_currentFgTexture = item.lParam;
				TerrainMaterial::setFgTexClass(m_currentFgTexture);
				updateLabel();
				m_terrainSwatches.Invalidate();
			}
		}
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}

BEGIN_MESSAGE_MAP(TerrainModal, CDialog)
	//{{AFX_MSG_MAP(TerrainModal)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TerrainModal message handlers
