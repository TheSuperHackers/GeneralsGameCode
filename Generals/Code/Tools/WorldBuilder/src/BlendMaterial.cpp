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

// BlendMaterial.cpp : implementation file
//

#define DEFINE_TERRAIN_TYPE_NAMES

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "BlendMaterial.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "TileTool.h"
#include "Common/TerrainTypes.h"
#include "W3DDevice/GameClient/TerrainTex.h"	  

BlendMaterial *BlendMaterial::m_staticThis = NULL;

static Int defaultMaterialIndex = -1;

/////////////////////////////////////////////////////////////////////////////
// BlendMaterial dialog

Int BlendMaterial::m_currentBlendTexture(-1);

BlendMaterial::BlendMaterial(CWnd* pParent /*=NULL*/) :
	m_updating(false)
{
	//{{AFX_DATA_INIT(BlendMaterial)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void BlendMaterial::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BlendMaterial)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BlendMaterial, COptionsPanel)
	//{{AFX_MSG_MAP(BlendMaterial)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BlendMaterial data access method.

/// Set foreground texture and invalidate swatches.
void BlendMaterial::setBlendTexClass(Int texClass) 
{
	if (m_staticThis) {
		m_staticThis->m_currentBlendTexture=texClass;
		m_staticThis->setTerrainTreeViewSelection(TVI_ROOT, texClass);
	}
}



/// Set the selected texture in the tree view.
Bool BlendMaterial::setTerrainTreeViewSelection(HTREEITEM parent, Int selection)
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


/////////////////////////////////////////////////////////////////////////////
// BlendMaterial message handlers

/// Setup the controls in the dialog.
BOOL BlendMaterial::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_updating = true;
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
	//m_terrainSwatches.Create(NULL, "", WS_CHILD, rect, this, IDC_TERRAIN_SWATCHES);
	//m_terrainSwatches.ShowWindow(SW_SHOW);

	m_staticThis = this;
	updateTextures();
	m_updating = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM BlendMaterial::findOrAdd(HTREEITEM parent, const char *pLabel)
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
	ins.item.pszText = const_cast<char*>(pLabel);
	ins.item.cchTextMax = strlen(pLabel);				
	child = m_terrainTreeView.InsertItem(&ins);
	return(child);
}

/** Add the terrain path to the tree view. */
void BlendMaterial::addTerrain(const char *pPath, Int terrainNdx, HTREEITEM parent)
{
	AsciiString className;
	if (terrainNdx >= 0) {
		className = WorldHeightMapEdit::getTexClassName( terrainNdx );
	}
	TerrainType *terrain = TheTerrainTypes->findTerrain( className );
	Bool doAdd = FALSE;
	char buffer[_MAX_PATH];
	//
	// if we have a 'terrain' entry, it means that our terrain index was properly defined
	// in an INI file, otherwise it was from eval textures.  We will sort all of
	// the eval texture entries in a tree leaf all their own while the others are
	// sorted according to a field specified in INI
	//
	if( terrain )
	{
		if (!terrain->isBlendEdge()) {
			return;	 // Only do blend edges to the blend material list.
		}

		// set the name in the tree view to that of the entry
		strcpy( buffer, terrain->getName().str() );

		doAdd = TRUE;
	} else if (terrainNdx==-1) {
		strcpy(buffer, pPath);
		doAdd = true;
	} else if (WorldHeightMapEdit::getTexClassIsBlendEdge(terrainNdx)) {
		parent = findOrAdd( parent, "**EVAL**" );
		strcpy(buffer, pPath);
		doAdd = true;
	}  // end if

//	Int tilesPerRow = TEXTURE_WIDTH/(2*TILE_PIXEL_EXTENT+TILE_OFFSET);
//	Int availableTiles = 4 * tilesPerRow * tilesPerRow;
//	Int percent = (WorldHeightMapEdit::getTexClassNumTiles(terrainNdx)*100 + availableTiles/2) / availableTiles;

	char label[_MAX_PATH];
	sprintf(label, "%s", buffer);


	if( doAdd )
	{
		TVINSERTSTRUCT ins;

		::memset(&ins, 0, sizeof(ins));
		ins.hParent = parent;
		ins.hInsertAfter = TVI_LAST;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT;
		ins.item.lParam = terrainNdx;
		ins.item.pszText = label;
		ins.item.cchTextMax = strlen(label)+2;				
		m_terrainTreeView.InsertItem(&ins);
	}

}

//* Create the tree view of textures from the textures in pMap. */
void BlendMaterial::updateTextures(void)
{
	m_updating = true;
	m_terrainTreeView.DeleteAllItems();
	Int i;
	CString label;
	label.LoadString(IDS_ALPHA_BLEND);
	addTerrain(label, -1, TVI_ROOT);
	for (i=WorldHeightMapEdit::getNumTexClasses()-1; i>=0; i--) {
		char path[_MAX_PATH];
		AsciiString uiName = WorldHeightMapEdit::getTexClassUiName(i);
		strncpy(path, uiName.str(), _MAX_PATH-2);
		addTerrain(path, i, TVI_ROOT);
	}
	m_updating = false;
	m_currentBlendTexture = defaultMaterialIndex;
	setTerrainTreeViewSelection(TVI_ROOT, m_currentBlendTexture);	
}




BOOL BlendMaterial::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
			if (item.lParam >= -1) {
				Int texClass = item.lParam;
				CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
				if (!pDoc) return 0;

				WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
				if (!pMap) return 0;
				if (m_updating) return 0;
				m_currentBlendTexture = texClass;
			}	else if (!(item.state & TVIS_EXPANDEDONCE) ) {
				HTREEITEM child = m_terrainTreeView.GetChildItem(hItem);
				while (child != NULL) {
					hItem = child;
					child = m_terrainTreeView.GetChildItem(hItem);
				}
				if (hItem != m_terrainTreeView.GetSelectedItem()) {
					m_terrainTreeView.SelectItem(hItem);
				}
			}
		}
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}
