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

// TerrainMaterial.cpp : implementation file
//

#define DEFINE_TERRAIN_TYPE_NAMES

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "TerrainMaterial.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "TileTool.h"				
#include "wbview3d.h"
#include "Common/TerrainTypes.h"
#include "W3DDevice/GameClient/TerrainTex.h"	  
#include "W3DDevice/GameClient/HeightMap.h"

TerrainMaterial *TerrainMaterial::m_staticThis = NULL;

static Int defaultMaterialIndex = 0;

/////////////////////////////////////////////////////////////////////////////
// TerrainMaterial dialog

Int TerrainMaterial::m_currentFgTexture(3);
Int TerrainMaterial::m_currentBgTexture(6);

Bool TerrainMaterial::m_paintingPathingInfo;
Bool TerrainMaterial::m_paintingPassable;

Bool TerrainMaterial::m_onCopyApplyMode(false);
Bool TerrainMaterial::m_onCopySelectMode(false);
Int TerrainMaterial::m_copyRotation(0);

TerrainMaterial::TerrainMaterial(CWnd* pParent /*=NULL*/) :
	m_updating(false),
	m_lastTool(""),
	m_currentWidth(BigTileTool::getTileToolWidth())
{
	//{{AFX_DATA_INIT(TerrainMaterial)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void TerrainMaterial::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TerrainMaterial)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TerrainMaterial, COptionsPanel)
	//{{AFX_MSG_MAP(TerrainMaterial)
	ON_BN_CLICKED(IDC_SWAP_TEXTURES, OnSwapTextures)
	ON_EN_CHANGE(IDC_SIZE_EDIT, OnChangeSizeEdit)
	ON_BN_CLICKED(IDC_IMPASSABLE, OnImpassable)
	ON_BN_CLICKED(IDC_PASSABLE_CHECK, OnPassableCheck)
	ON_BN_CLICKED(IDC_PASSABLE, OnPassable)

	ON_BN_CLICKED(IDC_COPY_MODE, OnCopyMode)
	ON_BN_CLICKED(IDC_TERRAIN_COPY_SELECT, OnCopySelect)
	ON_BN_CLICKED(IDC_TERRAIN_COPY_APPLY, OnCopyApply)

	ON_BN_CLICKED(IDC_TERRAIN_ROTATE1, OnRotate0)
	ON_BN_CLICKED(IDC_TERRAIN_ROTATE2, OnRotate90)
	ON_BN_CLICKED(IDC_TERRAIN_ROTATE3, OnRotate180)
	ON_BN_CLICKED(IDC_TERRAIN_ROTATE4, OnRotate270)

	ON_BN_CLICKED(IDC_SET_FAV, OnSetFavorite)
	ON_BN_CLICKED(IDC_DEL_FAV, OnDeleteFavorite)

	ON_BN_CLICKED(IDC_REL_FAV, OnImportFavoritesFromMapFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TerrainMaterial data access method.

/// Set foreground texture and invalidate swatches.
void TerrainMaterial::setFgTexClass(Int texClass) 
{
	if (m_staticThis) {
		m_staticThis->m_currentFgTexture=texClass;
		m_staticThis->m_terrainSwatches.Invalidate();
		updateTextureSelection();
	}
}


/// Set backgroundground texture and invalidate swatches.
void TerrainMaterial::setBgTexClass(Int texClass) 
{
	if (m_staticThis) {
		m_staticThis->m_currentBgTexture=texClass;
		m_staticThis->m_terrainSwatches.Invalidate();
	}
}

/// Sets the setWidth value in the dialog.
/** Update the value in the edit control and the slider. */
void TerrainMaterial::setWidth(Int width) 
{ 
	CString buf;
	buf.Format("%d", width);
	if (m_staticThis && !m_staticThis->m_updating) {
		m_staticThis->m_currentWidth = width;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		if (pEdit) pEdit->SetWindowText(buf);
	}
}

/// Sets the tool option - single & multi tile use this panel, 
// and only multi tile uses the width.
/** Update the ui for the tool. */
void TerrainMaterial::setToolOptions(Bool singleCell, Bool floodfill) 
{ 
	CString buf;
	if (m_staticThis ) {
		m_staticThis->m_updating = true;

		/**
		 * Adriane [Deathscythe]
		 * We want to suppress saving if the value is already saved but not yet initialized in the UI.
		 * Since changes in the UI are monitored by OnChangeSizeEdit(), we want to avoid triggering too many write actions.
		 * 
		 * This is similar to how we suppress saving the width to the World Builder settings 
		 * when switching between the single tile tool and the big tile tool,
		 * as that transition is considered part of the initialization process.
		 */
		m_staticThis->m_suppressWidthEditSave = true;

		CString newTool = singleCell ? "tile" : "bigtile";
		if (m_staticThis->m_lastTool.IsEmpty()) {
			m_staticThis->m_lastTool = newTool;
		}

		if (m_staticThis->m_lastTool != newTool) {
			m_staticThis->m_lastTool = newTool;
			BigTileTool::setWidth(BigTileTool::getTileToolWidth());
			m_staticThis->m_suppressWidthEditSave = false;
		}

		CButton* pCheckBox = (CButton*)m_staticThis->GetDlgItem(IDC_COPY_MODE);
		Bool isChecked = (pCheckBox->GetCheck() != 0);
		if(isChecked && singleCell){
			pCheckBox->SetCheck(FALSE);
			m_staticThis->m_terrainSwatches.EnableWindow(TRUE);
			m_staticThis->m_terrainTreeView.EnableWindow(TRUE);
		}
		pCheckBox->EnableWindow(!singleCell);
		
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		if (pEdit) {
			pEdit->EnableWindow(!singleCell);
			if (singleCell) {
				pEdit->SetWindowText("1");

				pEdit = m_staticThis->GetDlgItem(IDC_TERRAIN_COPY_SELECT);
				pEdit->EnableWindow(!singleCell);

				pEdit = m_staticThis->GetDlgItem(IDC_TERRAIN_COPY_APPLY);
				pEdit->EnableWindow(!singleCell);

				pEdit = m_staticThis->GetDlgItem(IDC_TERRAIN_ROTATE1);
				pEdit->EnableWindow(!singleCell);

				pEdit = m_staticThis->GetDlgItem(IDC_TERRAIN_ROTATE2);
				pEdit->EnableWindow(!singleCell);

				pEdit = m_staticThis->GetDlgItem(IDC_TERRAIN_ROTATE3);
				pEdit->EnableWindow(!singleCell);

				pEdit = m_staticThis->GetDlgItem(IDC_TERRAIN_ROTATE4);
				pEdit->EnableWindow(!singleCell);

			}
		}
		pEdit = m_staticThis->GetDlgItem(IDC_SIZE_POPUP);
		if (pEdit) {
			pEdit->EnableWindow(!singleCell);
		}

		pEdit = m_staticThis->GetDlgItem(IDC_DEV_NOTE);
		if(pEdit){
			pEdit->SetWindowText(floodfill ? 
				"Hold Shift and click texture to replace" : 
				"Hold Shift to swap textures");
		}

		// pEdit = m_staticThis->GetDlgItem(IDC_COPY_MODE);
		// pEdit->EnableWindow(!singleCell);

		if(singleCell){
			m_onCopySelectMode = false;
			m_onCopyApplyMode = false;
		}

		m_staticThis->m_updating = false;
	}
}

void TerrainMaterial::updateLabel(void)
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (!pDoc) return;

	AsciiString name = pDoc->GetHeightMap()->getTexClassUiName(m_currentFgTexture);
	const char *tName = name.str();
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

void TerrainMaterial::updateTextureSelection(void)
{
	if (m_staticThis) {
		m_staticThis->setTerrainTreeViewSelection(TVI_ROOT, m_staticThis->m_currentFgTexture);
		m_staticThis->updateLabel();
	}
}

/// Set the selected texture in the tree view.
Bool TerrainMaterial::setTerrainTreeViewSelection(HTREEITEM parent, Int selection)
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

void TerrainMaterial::OnSetFavorite()
{
    HTREEITEM hSelected = m_terrainTreeView.GetSelectedItem();
    if (!hSelected)
        return;

    CString itemText = m_terrainTreeView.GetItemText(hSelected);
    DWORD_PTR itemData = m_terrainTreeView.GetItemData(hSelected);

    // Check if item already exists in favorites
    HTREEITEM hItem = m_favTreeView.GetRootItem();
    while (hItem)
    {
        CString favText = m_favTreeView.GetItemText(hItem);
        DWORD_PTR favData = m_favTreeView.GetItemData(hItem);

        if (favText == itemText && favData == itemData)
        {
            ::MessageBeep(MB_ICONEXCLAMATION); // Already exists
            return;
        }

        hItem = m_favTreeView.GetNextSiblingItem(hItem);
    }

    // Add to favorites
    HTREEITEM hNewItem = m_favTreeView.InsertItem(itemText);
    m_favTreeView.SetItemData(hNewItem, itemData);
	DEBUG_LOG(("Added favorite: Name=%s, ID=%d\n", itemText, (int)itemData));
	SaveFavoritesToMapFolder();
}

void TerrainMaterial::OnDeleteFavorite()
{
    HTREEITEM hSelected = m_favTreeView.GetSelectedItem();
    if (!hSelected)
    {
        ::MessageBeep(MB_ICONEXCLAMATION); // Nothing selected
        return;
    }

    m_favTreeView.DeleteItem(hSelected);
}

void TerrainMaterial::SaveFavoritesToMapFolder()
{
    CString mapPath = CWorldBuilderDoc::GetActiveDoc()->GetPathName();
    if (mapPath.IsEmpty()) return;

    char folderPath[_MAX_PATH];
    strncpy(folderPath, mapPath, _MAX_PATH - 1);
    folderPath[_MAX_PATH - 1] = '\0';
    char* lastSlash = strrchr(folderPath, '\\');
    if (lastSlash) *lastSlash = '\0';

    CString iniPath;
    iniPath.Format("%s\\texture_favorites.ini", folderPath);

    // Open the file manually for writing
    FILE* file = fopen(iniPath, "w");
    if (!file) return;

    // Write a comment at the top of the file
    fprintf(file, "; This file is autogenerated by Adriane's Worldbuilder\n");
    fprintf(file, "; You can copy this file to another map and import it as your own, Please dont mess with format will ya?\n\n");
    fprintf(file, "; DO NOT fucking toy with the values unless you know what you are doing!\n");

    // Write the section header
    fprintf(file, "[FavoriteTextures]\n");

    int index = 0;
    HTREEITEM hItem = m_favTreeView.GetRootItem();
    while (hItem)
    {
        int textureId = (int)m_favTreeView.GetItemData(hItem);
        CString name = m_favTreeView.GetItemText(hItem);

        CString key, value;
        key.Format("Texture_%d", index++);
        value.Format("%d|%s", textureId, name);

        fprintf(file, "%s=%s\n", key, value);

        hItem = m_favTreeView.GetNextSiblingItem(hItem);
    }

    fclose(file);
}

void TerrainMaterial::OnImportFavoritesFromMapFolder()
{
    int result = ::MessageBox(
        AfxGetMainWnd()->GetSafeHwnd(),
        "This will import the custom favorite textures from this map if there's one.\n\nDo you want to continue?",
        "Import Favorites",
        MB_YESNO | MB_ICONQUESTION
    );

    if (result == IDNO)
    {
        return;
    }

    CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
    if (!pDoc) return;

    CString mapPath = pDoc->getMapPath();
    if (mapPath.IsEmpty()) return;

    char folderPath[_MAX_PATH];
    strncpy(folderPath, mapPath, _MAX_PATH - 1);
    folderPath[_MAX_PATH - 1] = '\0';
    char* lastSlash = strrchr(folderPath, '\\');
    if (lastSlash) *lastSlash = '\0';

    CString iniPath;
    iniPath.Format("%s\\texture_favorites.ini", folderPath);

    for (int i = 0; i < 100; ++i)
    {
        CString key;
        key.Format("Texture_%d", i);

        DEBUG_LOG(("Loading Favorites!"));

        char buffer[512] = { 0 };
        ::GetPrivateProfileString("FavoriteTextures", key, "", buffer, sizeof(buffer) - 1, iniPath);
        if (strlen(buffer) == 0)
            break;

        char* sep = strchr(buffer, '|');
        if (!sep) continue;

        *sep = '\0';
        int textureId = atoi(buffer);
        CString name = sep + 1;

        if (textureId <= 0 || name.IsEmpty())
            continue;

        // Check if already in favorites
        bool alreadyExists = false;
        HTREEITEM hItem = m_favTreeView.GetRootItem();
        while (hItem)
        {
            CString existingName = m_favTreeView.GetItemText(hItem);
            DWORD_PTR existingId = m_favTreeView.GetItemData(hItem);

            if (existingId == (DWORD_PTR)textureId && existingName == name)
            {
                alreadyExists = true;
                break;
            }

            hItem = m_favTreeView.GetNextSiblingItem(hItem);
        }

        if (alreadyExists)
        {
            DEBUG_LOG(("Skipped duplicate favorite: ID=%d, Name=%s\n", textureId, name));
            continue;
        }

        HTREEITEM hNewItem = m_favTreeView.InsertItem(name);
        if (hNewItem)
            m_favTreeView.SetItemData(hNewItem, textureId);

        DEBUG_LOG(("Loaded favorite: ID=%d, Name=%s\n", textureId, name));
    }
}
/////////////////////////////////////////////////////////////////////////////
// TerrainMaterial message handlers

/// Setup the controls in the dialog.
BOOL TerrainMaterial::OnInitDialog() 
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

	CWnd* pFavWnd = GetDlgItem(IDC_TERRAIN_TREEVIEW_FAV);
	CRect favRect;
	pFavWnd->GetWindowRect(&favRect);
	ScreenToClient(&favRect);
	favRect.DeflateRect(2, 2, 2, 2);

	m_favTreeView.Create(
		TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
		favRect, this, IDC_TERRAIN_TREEVIEW_FAV);
	m_favTreeView.ShowWindow(SW_SHOW);

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_terrainSwatches.Create(NULL, "", WS_CHILD, rect, this, IDC_TERRAIN_SWATCHES);
	m_terrainSwatches.ShowWindow(SW_SHOW);

	m_paintingPathingInfo = false;
	m_paintingPassable = false;

	m_onCopySelectMode = false;
	m_onCopyApplyMode = false;
	m_copyRotation = 0;

	CButton *button = (CButton *)GetDlgItem(IDC_PASSABLE_CHECK);
	button->SetCheck(false);
	button = (CButton *)GetDlgItem(IDC_PASSABLE);
	button->SetCheck(false);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_IMPASSABLE);
	button->SetCheck(true);
	button->EnableWindow(false);

	button = (CButton *)GetDlgItem(IDC_COPY_MODE);
	button->SetCheck(false);
	button = (CButton *)GetDlgItem(IDC_TERRAIN_COPY_SELECT);
	button->SetCheck(false);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_TERRAIN_COPY_APPLY);
	button->SetCheck(false);
	button->EnableWindow(false);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE1);
	button->SetCheck(true);
	button->EnableWindow(false);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE2);
	button->EnableWindow(false);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE3);
	button->EnableWindow(false);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE4);
	button->EnableWindow(false);

	m_widthPopup.SetupPopSliderButton(this, IDC_SIZE_POPUP, this);
	m_staticThis = this;
	m_updating = false;
	setWidth(m_currentWidth);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/** Locate the child item in tree item parent with name pLabel.  If not
found, add it.  Either way, return child. */
HTREEITEM TerrainMaterial::findOrAdd(HTREEITEM parent, const char *pLabel)
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
void TerrainMaterial::addTerrain(char *pPath, Int terrainNdx, HTREEITEM parent)
{
	TerrainType *terrain = TheTerrainTypes->findTerrain( WorldHeightMapEdit::getTexClassName( terrainNdx ) );
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
		if (terrain->isBlendEdge()) {
			return;	 // Don't add blend edges to the materials list.
		}
		for( TerrainClass i = TERRAIN_NONE; i < TERRAIN_NUM_CLASSES; i = (TerrainClass)(i + 1) )
		{

			if( terrain->getClass() == i )
			{

				parent = findOrAdd( parent, terrainTypeNames[ i ] );
				break;  // exit for

			}  // end if

		}  // end for i

		// set the name in the tree view to that of the entry
		strcpy( buffer, terrain->getName().str() );

		doAdd = TRUE;
	}  // end if
 	else if (!WorldHeightMapEdit::getTexClassIsBlendEdge(terrainNdx)) 
	{

		// all these old entries we will put in a tree for eval textures
		parent = findOrAdd( parent, "**Eval" );
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
			buffer[i+1] = 0;  // terminate at next character
			doAdd = TRUE;
			i++;
		}
	}  // end else

	Int tilesPerRow = TEXTURE_WIDTH/(2*TILE_PIXEL_EXTENT+TILE_OFFSET);
	Int availableTiles = 4 * tilesPerRow * tilesPerRow;
	Int percent = (WorldHeightMapEdit::getTexClassNumTiles(terrainNdx)*100 + availableTiles/2) / availableTiles;

	char label[_MAX_PATH];
	sprintf(label, "%d%% %s", percent, buffer);


	if( doAdd )
	{
		if (percent<3 && defaultMaterialIndex==0) {
			defaultMaterialIndex = terrainNdx;
		}
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
void TerrainMaterial::updateTextures(WorldHeightMapEdit *pMap)
{
#if 1 
	if (m_staticThis) {
		m_staticThis->m_updating = true;
		m_staticThis->m_terrainTreeView.DeleteAllItems();
		Int i;
		for (i=0; i<pMap->getNumTexClasses(); i++) {
			char path[_MAX_PATH];
			AsciiString uiName = pMap->getTexClassUiName(i);
			strncpy(path, uiName.str(), _MAX_PATH-2);
			m_staticThis->addTerrain(path, i, TVI_ROOT);
		}
		m_staticThis->m_updating = false;
		m_staticThis->m_currentFgTexture = defaultMaterialIndex;
		updateTextureSelection();
	}
#endif
}

/** Swap the foreground and background textures. */
void TerrainMaterial::OnSwapTextures() 
{
	
	Int tmp = m_currentFgTexture;
	m_currentFgTexture = m_currentBgTexture;
	m_currentBgTexture = tmp;
	m_terrainSwatches.Invalidate();	
	updateTextureSelection();
}

/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void TerrainMaterial::OnChangeSizeEdit() 
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int width;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &width)) {
				m_currentWidth = width;
				if(!m_suppressWidthEditSave){
					if( isCopySelectMode() || isCopyApplyMode() ){
						BigTileTool::setCopyModeWidth(width);
					} else {
						BigTileTool::setTileToolWidth(width);
					}
				}
				BigTileTool::setWidth(m_currentWidth);
				sprintf(buffer, "%.1f FEET.", m_currentWidth*MAP_XY_FACTOR);
				pEdit = m_staticThis->GetDlgItem(IDC_WIDTH_LABEL);
				if (pEdit) pEdit->SetWindowText(buffer);
			}
			m_updating = false;
		}
}

void TerrainMaterial::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			*pMin = MIN_TILE_SIZE;
			*pMax = MAX_TILE_SIZE;
			*pInitial = m_currentWidth;
			*pLineSize = 1;
			break;
		default:
			break;
	}	// switch
}


void TerrainMaterial::PopSliderChanged(const long sliderID, long theVal)
{
	CString str;
	CWnd *pEdit;
	switch (sliderID) {

		case IDC_SIZE_POPUP:
			m_currentWidth = theVal;
			str.Format("%d",m_currentWidth);
			pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			BigTileTool::setWidth(m_currentWidth);
			break;

		default:
			break;
	}	// switch
}

void TerrainMaterial::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_SIZE_POPUP:
			break;

		default:
			break;
	}	// switch

}


BOOL TerrainMaterial::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
				Int texClass = item.lParam;
				CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
				if (!pDoc) return 0;

				WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
				if (!pMap) return 0;
				if (m_updating) return 0;
				if (pMap->canFitTexture(texClass)) {
					m_currentFgTexture = texClass;
					updateLabel();
					m_terrainSwatches.Invalidate();
				} else {
					if (m_currentFgTexture != texClass) {
						// Tried to switch to a too large texture.
						::AfxMessageBox(IDS_TEXTURE_TOO_LARGE);
						::AfxGetMainWnd()->SetFocus();
					} 
					m_currentFgTexture = texClass;
					updateLabel();
					m_terrainSwatches.Invalidate();
				}
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

	if (pHdr->hdr.hwndFrom == m_favTreeView.m_hWnd) {
		if (pHdr->hdr.code == TVN_SELCHANGED) {
			HTREEITEM hItem = m_favTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE | TVIF_PARAM;
			item.hItem = hItem;
			m_favTreeView.GetItem(&item);

			if (item.lParam >= 0) {
				Int texClass = item.lParam;

				CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
				if (!pDoc) return 0;
				WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
				if (!pMap) return 0;
				if (m_updating) return 0;

				if (pMap->canFitTexture(texClass)) {
					m_currentFgTexture = texClass;
					updateLabel();
					m_terrainSwatches.Invalidate();
				} else {
					if (m_currentFgTexture != texClass) {
						::AfxMessageBox(IDS_TEXTURE_TOO_LARGE);
						::AfxGetMainWnd()->SetFocus();
					}
					m_currentFgTexture = texClass;
					updateLabel();
					m_terrainSwatches.Invalidate();
				}
			}
		}
	}
	
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void TerrainMaterial::OnImpassable() 
{
	m_paintingPassable = false;
	CButton *button = (CButton *)GetDlgItem(IDC_PASSABLE);
	button->SetCheck(0);
}

void TerrainMaterial::OnPassableCheck() 
{
	CButton *owner = (CButton*) GetDlgItem(IDC_PASSABLE_CHECK);
	Bool isChecked = (owner->GetCheck() != 0);
	CButton *button = (CButton *)GetDlgItem(IDC_PASSABLE);
	button->EnableWindow(isChecked);
	button = (CButton *)GetDlgItem(IDC_IMPASSABLE);
	button->EnableWindow(isChecked);
	Bool showImpassable = false;
	if (TheTerrainRenderObject) {
		showImpassable = TheTerrainRenderObject->getShowImpassableAreas();
	}
	m_terrainSwatches.EnableWindow(!isChecked);
	m_terrainTreeView.EnableWindow(!isChecked);
	m_paintingPathingInfo = isChecked;
	if (showImpassable != isChecked) {
		TheTerrainRenderObject->setShowImpassableAreas(isChecked);
		// Force the entire terrain mesh to be rerendered.
		IRegion2D range = {0,0,0,0};
		CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
		if (pDoc) {
			WbView3d *p3View = pDoc->GetActive3DView();
			if (p3View) {
				p3View->updateHeightMapInView(pDoc->GetHeightMap(), false, range);
			}
		}
	}

	button = (CButton *)GetDlgItem(IDC_COPY_MODE);
	button->EnableWindow(!isChecked);

	if(isChecked){
		m_onCopyApplyMode = false;
		m_onCopySelectMode = false;
	}
}

void TerrainMaterial::OnPassable() 
{
	m_paintingPassable = true;
	CButton *button = (CButton *)GetDlgItem(IDC_IMPASSABLE);
	button->SetCheck(0);
}


void TerrainMaterial::OnCopyMode() 
{
	// DEBUG_LOG(("Copy Mode toggled\n"));

	m_onCopyApplyMode = false;
	m_onCopySelectMode = false;

	CButton *owner = (CButton*) GetDlgItem(IDC_COPY_MODE);
	Bool isChecked = (owner->GetCheck() != 0);

	CButton *button = (CButton *)GetDlgItem(IDC_TERRAIN_COPY_APPLY);
	button->EnableWindow(isChecked);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_COPY_SELECT);
	button->EnableWindow(isChecked);

	// Disable pathing controls when copy mode is active
	button = (CButton *)GetDlgItem(IDC_PASSABLE_CHECK);
	button->EnableWindow(!isChecked);
	m_terrainSwatches.EnableWindow(!isChecked);
	m_terrainTreeView.EnableWindow(!isChecked);

	m_suppressWidthEditSave = true;
	if (isChecked) {
		TerrainMaterial::setWidth(BigTileTool::getCopyModeWidth());

		// Default to "Select" mode when Copy Mode is enabled
		CButton* selectBtn = (CButton*)GetDlgItem(IDC_TERRAIN_COPY_SELECT);
		selectBtn->SetCheck(TRUE);

		CButton* applyBtn = (CButton*)GetDlgItem(IDC_TERRAIN_COPY_APPLY);
		applyBtn->SetCheck(FALSE);

		OnCopySelect(); // Activate select mode logic
	} else {
		TerrainMaterial::setWidth(BigTileTool::getTileToolWidth());
	}
	m_suppressWidthEditSave = false;

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE1);
	// button->SetCheck(true);
	button->EnableWindow(isChecked);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE2);
	button->EnableWindow(isChecked);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE3);
	button->EnableWindow(isChecked);

	button = (CButton *)GetDlgItem(IDC_TERRAIN_ROTATE4);
	button->EnableWindow(isChecked);
}

void TerrainMaterial::OnCopySelect() 
{
	m_onCopyApplyMode = false;
	m_onCopySelectMode = true;
	// DEBUG_LOG(("Copy Select Mode\n"));
}

void TerrainMaterial::OnCopyApply() 
{
	m_onCopySelectMode = false;
	m_onCopyApplyMode = true;
	// DEBUG_LOG(("Copy Apply Mode\n"));
}


void TerrainMaterial::OnRotate0() 
{
	m_copyRotation = 0;
}

void TerrainMaterial::OnRotate90() 
{
	m_copyRotation = 90;
}

void TerrainMaterial::OnRotate180() 
{
	m_copyRotation = 180;
}

void TerrainMaterial::OnRotate270() 
{
	m_copyRotation = 270;
}