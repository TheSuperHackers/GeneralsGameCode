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

// ScriptDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "CUndoable.h"
#include "ScriptDialog.h"
#include "GameLogic/ScriptEngine.h"
#include "ScriptProperties.h"
#include "ScriptConditions.h"
#include "ScriptActionsTrue.h"
#include "ScriptActionsFalse.h"
#include "CFixTeamOwnerDialog.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/PolygonTrigger.h"
#include "Common/WellKnownKeys.h"
#include "Common/DataChunk.h"
#include "Common/FileSystem.h"
#include "EditGroup.h"
#include "EditParameter.h"
#include "ExportScriptsOptions.h"
#include "Common/ThingFactory.h"
#include "WaypointOptions.h"
#include "Common/UnicodeString.h"

#include "Common/GlobalData.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// static Bool g_didScriptWarningsUpdate = false;

static const Int K_LOCAL_TEAMS_VERSION_1 = 1;

#define SCRIPT_DIALOG_SECTION "ScriptDialog"

static const char* NEUTRAL_NAME_STR = "(neutral)";
ScriptDialog *ScriptDialog::m_staticThis = NULL;

static AsciiString formatScriptLabel(Script *pScr) {
	AsciiString fmt;
	if (pScr->isSubroutine()) {
		fmt.concat("[S "); 
	} else {
		fmt.concat("[ns "); 
	}
	if (pScr->isActive()) {
		fmt.concat("A "); 
	} else {
		fmt.concat("na "); 
	}
	if (pScr->isOneShot()) {
		fmt.concat("D] ["); 
	} else {
		fmt.concat("nd] ["); 
	}
	if (pScr->isEasy()) {
		fmt.concat("E "); 
	} 
	if (pScr->isNormal()) {
		fmt.concat("N "); 
	} 
	if (pScr->isHard()) {
		fmt.concat("H]"); 
	} else {
		fmt.concat("]");
	}
	fmt.concat(pScr->getName().str());
	return fmt;
}

static AsciiString formatScriptLabel(ScriptGroup *pScrGrp) {
	AsciiString fmt;
	if (pScrGrp->isSubroutine())
	{
		fmt.concat("[S "); 
	}
	else
	{
		fmt.concat("[ns "); 
	}
	if (pScrGrp->isActive())
	{
		fmt.concat("A]"); 
	}
	else
	{
		fmt.concat("na]"); 
	}
	fmt.concat(pScrGrp->getName().str());
	return fmt;
}

// CSDTreeCtrl Implementation /////////////////////////////////////////////////////////////////////

/** This function reacts to a right button push on a
		script or group to create a drop down menu */
void CSDTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
	/// first, if there's something under the mouse, select it.
	HTREEITEM item = HitTest(point);
	SelectItem(item);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_SCRIPTDIALOGPOPUP));
	CMenu* pPopup = menu.GetSubMenu(0);
	if (!pPopup)
	{
		return;
	}
	ClientToScreen(&point);

	/// Display a check mark to signify status of active-ness
	if (item)
	{
		ScriptDialog *sd = (ScriptDialog*) GetParent();
		if (sd->friend_getCurScript() != NULL)
		{
			Bool active = sd->friend_getCurScript()->isActive();
			pPopup->CheckMenuItem(ID_SCRIPTACTIVATE, MF_BYCOMMAND | (active ? MF_CHECKED : MF_UNCHECKED));
		}
		else if (sd->friend_getCurGroup() != NULL)
		{
			Bool active = sd->friend_getCurGroup()->isActive();
			pPopup->CheckMenuItem(ID_SCRIPTACTIVATE, MF_BYCOMMAND | (active ? MF_CHECKED : MF_UNCHECKED));
		}

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, GetParent());
	}
}

BEGIN_MESSAGE_MAP(CSDTreeCtrl, CTreeCtrl)
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptDialog dialog


ScriptDialog::ScriptDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ScriptDialog::IDD, pParent)
{
	m_draggingTreeView = false;
	m_autoUpdateWarnings = true;
	m_pOldFont = NULL; 
	m_bCompressed = false;
	m_bNewIcons = false;
	//{{AFX_DATA_INIT(ScriptDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ScriptDialog::~ScriptDialog()
{
	EditParameter::setCurSidesList(NULL);
	m_staticThis=NULL;
}


void ScriptDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScriptDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ScriptDialog, CDialog)
	//{{AFX_MSG_MAP(ScriptDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SCRIPT_TREE, OnSelchangedScriptTree)
	ON_BN_CLICKED(IDC_NEW_FOLDER, OnNewFolder)
	ON_BN_CLICKED(IDC_NEW_SCRIPT, OnNewScript)
	ON_BN_CLICKED(IDC_EDIT_SCRIPT, OnEditScript)
	ON_BN_CLICKED(IDC_COPY_SCRIPT, OnCopyScript)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_VERIFY, OnVerify)
	ON_BN_CLICKED(IDC_VERIFYALL, OnVerifyAll)
	ON_BN_CLICKED(IDC_PATCH_GC, OnPatchGC)
	ON_BN_CLICKED(IDC_AUTO_VERIFY, OnAutoVerify)
	ON_BN_CLICKED(IDC_COMPRESS, OnCompress)
	ON_BN_CLICKED(IDC_NEWICONS, OnNewIcons)
	ON_BN_CLICKED(IDC_DEEPSCAN, OnDisableDeepScan)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_NOTIFY(NM_DBLCLK, IDC_SCRIPT_TREE, OnDblclkScriptTree)
	ON_NOTIFY(TVN_BEGINDRAG, IDC_SCRIPT_TREE, OnBegindragScriptTree)
	ON_COMMAND(ID_SCRIPTACTIVATE, OnScriptActivate)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScriptDialog message handlers

inline bool asciiStringContains(const AsciiString& haystack, const char* needle)
{
	if (!needle || !haystack.str()) return false;
	return strstr(haystack.str(), needle) != NULL;
}

AsciiString parseLineBreaks(const AsciiString& input)
{
	AsciiString output = input;
	const char* p = output.str();
	AsciiString result;

	while (*p) {
		if (*p == '\n') {
			result.concat("\r\n");
		} else {
			char temp[2] = {*p, '\0'};
			result.concat(temp);
		}
		++p;
	}
	return result;
}

bool alreadyListed(const AsciiString& usedByTag, const AsciiString& scriptName)
{
	const char* tagStr = usedByTag.str();
	const char* nameStr = scriptName.str();

	// simple substring search with comma or bracket after
	AsciiString pattern(", ");
	pattern.concat(scriptName);

	if (strstr(tagStr, pattern.str()) != NULL)
		return true;

	// also check start-of-line case
	pattern = "[Referenced in:";
	pattern.concat(scriptName);
	if (strstr(tagStr, pattern.str()) != NULL)
		return true;

	return false;
}

void ScriptDialog::OnSelchangedScriptTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	if (pNMTreeView->itemNew.hItem==NULL) {
		m_curSelection.IntToList(0);
		m_curSelection.m_objType = ListType::PLAYER_TYPE;
	} else {
		m_curSelection.IntToList(pNMTreeView->itemNew.lParam);
		DEBUG_ASSERTCRASH(m_curSelection.m_playerIndex <m_sides.getNumSides(),("")); 
		DEBUG_ASSERTCRASH(m_curSelection.m_objType != ListType::BOGUS_TYPE, ("")); 
	}
	if (!this->m_draggingTreeView) {
		pTree->SelectDropTarget(pNMTreeView->itemNew.hItem); 
	}
	Script *pScript = getCurScript();
	ScriptGroup *pGroup = getCurGroup();

	CWnd *pWnd = GetDlgItem(IDC_EDIT_SCRIPT);
	pWnd->EnableWindow(pScript!=NULL || pGroup!=NULL);
	
	pWnd = GetDlgItem(IDC_COPY_SCRIPT);
	pWnd->EnableWindow(pScript!=NULL || pGroup!=NULL);

	pWnd = GetDlgItem(IDC_DELETE);
	pWnd->EnableWindow(m_curSelection.m_objType != ListType::PLAYER_TYPE);

	AsciiString scriptComment;
	AsciiString scriptText;
	if (pScript) {
		scriptComment = pScript->getComment();
		scriptText = pScript->getUiText();

		AsciiString usedByTag;
		usedByTag.concat("[Referenced in:");

		AsciiString targetScriptName = pScript->getName();
		Bool foundUse = false;

		for (int i = 0; i < m_sides.getNumSides(); ++i) {
			ScriptList* pSL = m_sides.getSideInfo(i)->getScriptList();
			if (!pSL) continue;

			// --- Check non-grouped scripts
			for (Script* s = pSL->getScript(); s; s = s->getNext()) {
				if (s == pScript) continue;
				bool referenced = false;

				// Check conditions
				for (OrCondition* pOr = s->getOrCondition(); pOr && !referenced; pOr = pOr->getNextOrCondition()) {
					for (Condition* c = pOr->getFirstAndCondition(); c && !referenced; c = c->getNext()) {
						for (int p = 0; p < c->getNumParameters(); ++p) {
							Parameter* param = c->getParameter(p);
							if (param && param->getParameterType() == Parameter::SCRIPT &&
								param->getString() == targetScriptName) {
								referenced = true;
								break;
							}
						}
					}
				}

				// Check actions if not already found
				for (ScriptAction* a = s->getAction(); a && !referenced; a = a->getNext()) {
					for (int p = 0; p < a->getNumParameters(); ++p) {
						Parameter* param = a->getParameter(p);
						if (param && param->getParameterType() == Parameter::SCRIPT &&
							param->getString() == targetScriptName) {
							referenced = true;
							break;
						}
					}
				}

				if (referenced && !alreadyListed(usedByTag, s->getName())) {
					if (foundUse) {
						usedByTag.concat(", ");
					} else {
						foundUse = true;
					}
					usedByTag.concat(s->getName());
				}
			}

			// --- Check grouped scripts
			for (ScriptGroup* g = pSL->getScriptGroup(); g; g = g->getNext()) {
				for (Script* s = g->getScript(); s; s = s->getNext()) {
					if (s == pScript) continue;
					bool referenced = false;

					// Conditions
					for (OrCondition* pOr = s->getOrCondition(); pOr && !referenced; pOr = pOr->getNextOrCondition()) {
						for (Condition* c = pOr->getFirstAndCondition(); c && !referenced; c = c->getNext()) {
							for (int p = 0; p < c->getNumParameters(); ++p) {
								Parameter* param = c->getParameter(p);
								if (param && param->getParameterType() == Parameter::SCRIPT &&
									param->getString() == targetScriptName) {
									referenced = true;
									break;
								}
							}
						}
					}

					// Actions
					for (ScriptAction* a = s->getAction(); a && !referenced; a = a->getNext()) {
						for (int p = 0; p < a->getNumParameters(); ++p) {
							Parameter* param = a->getParameter(p);
							if (param && param->getParameterType() == Parameter::SCRIPT &&
								param->getString() == targetScriptName) {
								referenced = true;
								break;
							}
						}
					}

					if (referenced && !alreadyListed(usedByTag, s->getName())) {
						if (foundUse) {
							usedByTag.concat(", ");
						} else {
							foundUse = true;
						}
						usedByTag.concat(s->getName());
					}
				}
			}
		}

		usedByTag.concat(foundUse ? "]" : " None]");

		if(!scriptComment.isEmpty()){
			scriptComment.concat("\n\n");
		}
		scriptComment.concat(usedByTag);

		scriptComment = parseLineBreaks(scriptComment);
	}

	pWnd = GetDlgItem(IDC_SCRIPT_COMMENT);
	pWnd->SetWindowText(scriptComment.str());

	pWnd = GetDlgItem(IDC_SCRIPT_DESCRIPTION);
	pWnd->SetWindowText(scriptText.str());

	*pResult = 0;
}

/* The purpose of these two functions is to allow
the inner class CSDTreeCtrl the ability to check
what Script and ScriptGroup belong to the cursor location */
Script *ScriptDialog::friend_getCurScript(void)
{
	return getCurScript();
}

ScriptGroup *ScriptDialog::friend_getCurGroup(void)
{
	return getCurGroup();
}

Script *ScriptDialog::getCurScript(void)
{
	if (m_curSelection.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE || m_curSelection.m_objType == ListType::SCRIPT_IN_GROUP_TYPE) {
		ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
		if (pSL) {
			Script *pScr=NULL;
			if (m_curSelection.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE) {
				pScr = pSL->getScript();
			}	else {
				Int groupNdx;
				ScriptGroup *pGroup = pSL->getScriptGroup();
				for (groupNdx = 0; pGroup; groupNdx++,pGroup=pGroup->getNext()) {
					if (groupNdx == m_curSelection.m_groupIndex) {
						pScr = pGroup->getScript();
						break;
					}
				}
			}
			Int scriptNdx;
			for (scriptNdx = 0; pScr; scriptNdx++,pScr=pScr->getNext()) {
				if (scriptNdx == m_curSelection.m_scriptIndex) {
					return pScr;
				}
			}
		}
	} 
	return NULL;
}

ScriptGroup *ScriptDialog::getCurGroup(void)
{
	ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	if (m_curSelection.m_objType == ListType::PLAYER_TYPE) {
		return NULL;
	}
	if (m_curSelection.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE) {
		return NULL;
	}
	if (pSL) {
		Int groupNdx;
		ScriptGroup *pGroup = pSL->getScriptGroup();
		for (groupNdx = 0; pGroup; groupNdx++,pGroup=pGroup->getNext()) {
			if (groupNdx == m_curSelection.m_groupIndex) {
				return pGroup;
			}
		}
	}
	return NULL;
}

/** Updates the warning flags in a script, & script conditions & actions. */
void ScriptDialog::updateScriptWarning(Script *pScript)
{
	pScript->setWarnings(false);
	OrCondition *pOr;
	for (pOr= pScript->getOrCondition(); pOr; pOr = pOr->getNextOrCondition()) {
		Condition *pCondition;
		for (pCondition = pOr->getFirstAndCondition(); pCondition; pCondition = pCondition->getNext()) {
			pCondition->setWarnings(false);
			Int i;
			for (i=0; i<pCondition->getNumParameters(); i++) {
				AsciiString warning;
				warning = EditParameter::getWarningText(pCondition->getParameter(i), FALSE);
				if (!warning.isEmpty()) {
					pScript->setWarnings(true);
					pCondition->setWarnings(true);
				}	
			}
		}
	}
	ScriptAction *pAction;
	for (pAction = pScript->getAction(); pAction; pAction = pAction->getNext()) {
		pAction->setWarnings(false);
		Int i;
		for (i=0; i<pAction->getNumParameters(); i++) {
			AsciiString warning;
			warning = EditParameter::getWarningText(pAction->getParameter(i), TRUE);
			if (!warning.isEmpty()) {
				pScript->setWarnings(true);
				pAction->setWarnings(true);
			}	
		}
	}
}

void ScriptDialog::OnPatchGC()
{
	checkParametersForGC();
	updateIcons(TVI_ROOT);
/*  //Put up a dialog asking for search/replace parameters instead of hard-coded GC_ prefix.
	ReplaceParameter editDlg();
	if (IDOK==editDlg.DoModal())
	{	

	}*/
}

void ScriptDialog::OnFindNext()
{
    UpdateData(TRUE); // Sync UI to variables if you're using DDX

    CEdit* pEdit = (CEdit*)GetDlgItem(IDC_SCRIPT_SEARCH);
    CString searchText;
    pEdit->GetWindowText(searchText);
    searchText.MakeLower();

    CTreeCtrl* pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
    HTREEITEM hItem = m_lastFoundItem ? pTree->GetNextItem(m_lastFoundItem, TVGN_NEXT) : pTree->GetRootItem();

    while (hItem)
    {
		CString text = pTree->GetItemText(hItem);
		text.MakeLower();
		if (text.Find(searchText) != -1)
		{		
            m_lastFoundItem = hItem;
            pTree->SelectItem(hItem);
            pTree->EnsureVisible(hItem);
            return;
        }

        // Search child if it exists
        HTREEITEM hChild = pTree->GetChildItem(hItem);
        if (hChild)
        {
            hItem = hChild;
        }
        else
        {
            // Otherwise, go to the next sibling or climb up until we find one
            while (hItem && !pTree->GetNextSiblingItem(hItem))
                hItem = pTree->GetParentItem(hItem);
            if (hItem)
                hItem = pTree->GetNextSiblingItem(hItem);
        }
    }

    MessageBox("No more matches found.", "Search", MB_OK | MB_ICONINFORMATION);
    m_lastFoundItem = NULL;
}

/**Force a pass over all the scripts to make sure no warnings.  I moved this
to user control because this function is VERY slow. 7-15-03 -MW*/
void ScriptDialog::OnVerify()
{
	// Flag to indicate if cache was successfully loaded
	Bool loadedFromCache = false;

	if(m_autoUpdateWarnings){
		// Try loading cached warning state first
		if (LoadScriptWarningsState()) {
			DEBUG_LOG(("ScriptDialog: Loaded script warning state from cache.\n"));
			loadedFromCache = true;
		} else {
			DEBUG_LOG(("ScriptDialog: No valid cache found. Will update warnings normally.\n"));
		}

		// If cache didn't load, fall back to the expensive update
		// if (!g_didScriptWarningsUpdate) {
			if (!loadedFromCache) {
				updateWarnings(true);
				DEBUG_LOG(("ScriptDialog: Ran updateWarnings(true) due to missing cache.\n"));
			}
			// g_didScriptWarningsUpdate = true;
		// }
	}

	updateIcons(TVI_ROOT);
}

void ScriptDialog::OnVerifyAll()
{
	updateWarnings(true);
	updateIcons(TVI_ROOT);
}

void ScriptDialog::OnAutoVerify()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_AUTO_VERIFY);
	m_autoUpdateWarnings=(pButton->GetCheck()==1);
	::AfxGetApp()->WriteProfileInt(SCRIPT_DIALOG_SECTION, "AutoVerifyScripts", m_autoUpdateWarnings?1:0);
	//if user wants to check warnings manually, enable the verify button
	CWnd *pWnd = GetDlgItem(IDC_VERIFY);
	pWnd->EnableWindow(!m_autoUpdateWarnings);
}

void ScriptDialog::OnNewIcons()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_NEWICONS);
	m_bNewIcons = (pButton->GetCheck() == 1);
	::AfxGetApp()->WriteProfileInt(SCRIPT_DIALOG_SECTION, "NewIcons", m_bNewIcons ? 1 : 0);

	CTreeCtrl* pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	if (pTree)
	{
		// Delete old image list if it exists
		if (m_imageList.GetSafeHandle())
		{
			m_imageList.DeleteImageList();
		}

		if (m_bNewIcons)
			m_imageList.Create(IDB_FOLDERSCRIPTB, 16, 2, ILC_COLOR4); // new icons
		else
			m_imageList.Create(IDB_FOLDERSCRIPT, 16, 2, ILC_COLOR4); // default icons

		pTree->SetImageList(&m_imageList, TVSIL_STATE);
	}
}

void ScriptDialog::OnDisableDeepScan()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_DEEPSCAN);
	m_bDisableDeepScan = (pButton->GetCheck() == 1);
    ::AfxGetApp()->WriteProfileInt(SCRIPT_DIALOG_SECTION, "DisableDeepScan", m_bDisableDeepScan ? 1 : 0);
}

void ScriptDialog::OnCompress()
{
	CButton *pButton = (CButton*)GetDlgItem(IDC_COMPRESS);
	m_bCompressed = (pButton->GetCheck() == 1);
    ::AfxGetApp()->WriteProfileInt(SCRIPT_DIALOG_SECTION, "CompressScripts", m_bCompressed ? 1 : 0);

    CTreeCtrl* pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
    if (!pTree)
        return;

    if (m_bCompressed)
    {
        // Save original font (only once)
        if (!m_pOldFont)
        {
            CFont* pFont = pTree->GetFont();
            if (pFont)
                m_pOldFont = pFont;
        }

        // Create and apply custom font
        if (m_treeFont.GetSafeHandle())
            m_treeFont.DeleteObject();  // Clean up if reusing

        m_treeFont.CreateFont(
            14,                         // Height
            0,                          // Width
            0,                          // Escapement
            0,                          // Orientation
            FW_MEDIUM,               // Weight
            FALSE,                      // Italic
            FALSE,                      // Underline
            0,                          // StrikeOut
            ANSI_CHARSET,               // CharSet
            OUT_DEFAULT_PRECIS,         // OutPrecision
            CLIP_DEFAULT_PRECIS,        // ClipPrecision
            DEFAULT_QUALITY,            // Quality
            DEFAULT_PITCH | FF_SWISS,   // PitchAndFamily
            _T("Segoe UI")              // Font face
        );

        pTree->SetFont(&m_treeFont);
    }
    else
    {
        // Revert to old font
        if (m_pOldFont)
            pTree->SetFont(m_pOldFont);
    }

    // Force redraw
    pTree->Invalidate();
    pTree->UpdateWindow();
}

// Load em cached status baby Adriane [Deathscythe]
Bool ScriptDialog::LoadScriptWarningsState()
{
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (!pDoc) {
		DEBUG_LOG(("LoadScriptWarningsState: No active doc!\n"));
		return false;
	}

	DEBUG_LOG(("LoadScriptWarningsState: Map Path = %s\n", pDoc->getMapPath()));

	CString path = pDoc->getMapPath();
	if (path.IsEmpty()) {
		DEBUG_LOG(("LoadScriptWarningsState: Empty map path.\n"));
		return false;
	}

	int lastSlash = path.ReverseFind('\\');
	if (lastSlash != -1)
		path = path.Left(lastSlash + 1);
	else {
		DEBUG_LOG(("LoadScriptWarningsState: Malformed path - no backslash found.\n"));
		return false;
	}

	CString cacheFile = path + "AdrianeScriptWarningsCache.txt";
	DEBUG_LOG(("LoadScriptWarningsState: Final cache file = %s\n", cacheFile));

	CStdioFile file;
	if (!file.Open(cacheFile, CFile::modeRead | CFile::typeText)) {
		DEBUG_LOG(("LoadScriptWarningsState: Failed to open file for reading.\n"));
		return false;
	}

	CString line;
	std::map<std::string, Bool> warningMap;

	while (file.ReadString(line))
	{
		int delim = line.Find(',');
		if (delim > 0)
		{
			CString key = line.Left(delim);
			CString value = line.Mid(delim + 1);
			value.TrimLeft(); value.TrimRight();

			warningMap[(const char*)key] = (value == "1");
		}
		else {
			DEBUG_LOG(("LoadScriptWarningsState: Malformed line: %s\n", line));
		}
	}
	file.Close();

	SidesList* sidesListP = TheSidesList;
	if (m_staticThis) sidesListP = &m_staticThis->m_sides;

	for (int i = 0; i < sidesListP->getNumSides(); ++i)
	{
		ScriptList* pSL = sidesListP->getSideInfo(i)->getScriptList();
		if (!pSL) continue;

		char sideIndexStr[16];
		sprintf(sideIndexStr, "%d", i);

		Script* pScr;
		for (pScr = pSL->getScript(); pScr; pScr = pScr->getNext())
		{
			std::string key = std::string(sideIndexStr) + "|" + pScr->getName().str();
			if (warningMap.find(key) != warningMap.end())
			{
				pScr->setWarnings(warningMap[key]);
				pScr->setDirty(false);

				if(m_staticThis && m_staticThis->m_bDisableDeepScan != 1){
					appendWarningHintLazy(pScr);
				}

				// Expensive 
				// if (pScr->hasWarnings()) {
				// // 	updateScriptWarning(pScr);  // Force regen of warning messages like [???]
				// }
			}
		}

		ScriptGroup* pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup = pGroup->getNext())
		{
			for (pScr = pGroup->getScript(); pScr; pScr = pScr->getNext())
			{
				std::string key = std::string(sideIndexStr) + "|" + pScr->getName().str();
				if (warningMap.find(key) != warningMap.end())
				{
					pScr->setWarnings(warningMap[key]);
					pScr->setDirty(false);

					if(m_staticThis && m_staticThis->m_bDisableDeepScan != 1){
						appendWarningHintLazy(pScr);
					}

					// Expensive
					// if (pScr->hasWarnings()) {
					// 	// updateScriptWarning(pScr);  // Force regen of warning messages like [???]
					// }
				}
			}
		}
	}

	DEBUG_LOG(("LoadScriptWarningsState: Finished loading.\n"));
	return true;
}


void ScriptDialog::appendWarningHintLazy(Script* pScript)
{
	if (!pScript || !pScript->hasWarnings()) return;

	for (OrCondition* pOr = pScript->getOrCondition(); pOr; pOr = pOr->getNextOrCondition()) {
		for (Condition* pCond = pOr->getFirstAndCondition(); pCond; pCond = pCond->getNext()) {
			for (int i = 0; i < pCond->getNumParameters(); ++i) {
				Parameter* param = pCond->getParameter(i);
				AsciiString warn = EditParameter::getWarningText(param, FALSE);
				if (!warn.isEmpty()) {
					pCond->setWarnings(true);
					break; // one warning is enough
				}
			}
		}
	}

	for (ScriptAction* pAct = pScript->getAction(); pAct; pAct = pAct->getNext()) {
		for (int i = 0; i < pAct->getNumParameters(); ++i) {
			Parameter* param = pAct->getParameter(i);
			AsciiString warn = EditParameter::getWarningText(param, TRUE);
			if (!warn.isEmpty()) {
				pAct->setWarnings(true);
				break;
			}
		}
	}
}

void ScriptDialog::SaveScriptWarningsState()
{
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (!pDoc) return;

	DEBUG_LOG(("Map Path %s\n", pDoc->getMapPath()));

	CString path = pDoc->getMapPath();
	if (path.IsEmpty()) return;

	int lastSlash = path.ReverseFind('\\');
	if (lastSlash != -1)
		path = path.Left(lastSlash + 1);
	else
		return;

	CString cacheFile = path + "AdrianeScriptWarningsCache.txt";

	CStdioFile file;
	if (!file.Open(cacheFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
		return;

	SidesList* sidesListP = TheSidesList;
	if (m_staticThis) sidesListP = &m_staticThis->m_sides;

	for (int i = 0; i < sidesListP->getNumSides(); ++i)
	{
		ScriptList* pSL = sidesListP->getSideInfo(i)->getScriptList();
		if (!pSL) continue;

		char sideIndexStr[16];
		sprintf(sideIndexStr, "%d", i);

		Script* pScr;
		for (pScr = pSL->getScript(); pScr; pScr = pScr->getNext())
		{
			CString line;
			line.Format("%s|%s,%d\n", sideIndexStr, pScr->getName().str(), pScr->hasWarnings() ? 1 : 0);
			file.WriteString(line);
		}

		ScriptGroup* pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup = pGroup->getNext())
		{
			for (pScr = pGroup->getScript(); pScr; pScr = pScr->getNext())
			{
				CString line;
				line.Format("%s|%s,%d\n", sideIndexStr, pScr->getName().str(), pScr->hasWarnings() ? 1 : 0);
				file.WriteString(line);
			}
		}
	}

	file.Close();
}


/** Updates the warning flags in the scripts, script groups & script conditions & actions. */
void ScriptDialog::updateWarnings(Bool forceUpdate)
{
	if (m_staticThis && m_staticThis->m_autoUpdateWarnings == false && forceUpdate == false)
		return;	//user has disabled warnings to speed up the script editor

	SidesList *sidesListP = TheSidesList;
	Int i;
	if (m_staticThis) sidesListP = &m_staticThis->m_sides;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			if (pScr->isDirty()) {
				updateScriptWarning(pScr);
				pScr->setDirty(false);
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			pGroup->setWarnings(false);
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				if (pScr->isDirty()) {
					updateScriptWarning(pScr);
					pScr->setDirty(false);
				}
				if (pScr->hasWarnings()) {
					pGroup->setWarnings(true);
				}
			}
		}
	}	
}

extern AsciiString ConvertToNonGCName(AsciiString name, Bool checkTemplate=true);

void ScriptDialog::patchScriptParametersForGC(Script *pScript)
{
	AsciiString swapString;
	pScript->setWarnings(false);
	OrCondition *pOr;
	for (pOr= pScript->getOrCondition(); pOr; pOr = pOr->getNextOrCondition()) {
		Condition *pCondition;
		for (pCondition = pOr->getFirstAndCondition(); pCondition; pCondition = pCondition->getNext()) {
			pCondition->setWarnings(false);
			Int i;
			for (i=0; i<pCondition->getNumParameters(); i++) {
				AsciiString warning;
				Parameter *pParm = pCondition->getParameter(i);
				warning = EditParameter::getWarningText(pParm, FALSE);
				if (!warning.isEmpty()) {
					if (pParm->getParameterType() == Parameter::OBJECT_TYPE)
					{	//see if removing the GC prefix fixes this warning:
						AsciiString uiString = pParm->getString();
						if (uiString.isEmpty()) 
							uiString = "???";
						if (uiString.startsWith("GC_"))
						{	swapString = ConvertToNonGCName(uiString, false);
							pParm->friend_setString(swapString);
							warning = EditParameter::getWarningText(pParm, FALSE);
							if (!warning.isEmpty())
							{	//Removing GC prefix didn't help, so restore original
								pParm->friend_setString(uiString);
							}
							else
								continue;	//warning was fixed so leave swapped parameter.
						}
					}
					pScript->setWarnings(true);
					pCondition->setWarnings(true);
				}	
			}
		}
	}
	ScriptAction *pAction;
	for (pAction = pScript->getAction(); pAction; pAction = pAction->getNext()) {
		pAction->setWarnings(false);
		Int i;
		for (i=0; i<pAction->getNumParameters(); i++) {
			AsciiString warning;
			Parameter *pParm=pAction->getParameter(i);
			warning = EditParameter::getWarningText(pParm, TRUE);
			if (!warning.isEmpty()) {
				if (pParm->getParameterType() == Parameter::OBJECT_TYPE)
				{	//see if removing the GC prefix fixes this warning:
					AsciiString uiString = pParm->getString();
					if (uiString.isEmpty()) 
						uiString = "???";
					if (uiString.startsWith("GC_"))
					{	swapString = ConvertToNonGCName(uiString,false);
						pParm->friend_setString(swapString);
						warning = EditParameter::getWarningText(pParm, FALSE);
						if (!warning.isEmpty())
						{	//Removing GC prefix didn't help, so restore original
							pParm->friend_setString(uiString);
						}
						else
							continue;	//warning was fixed so leave swapped parameter.
					}
				}
				pScript->setWarnings(true);
				pAction->setWarnings(true);
			}	
		}
	}
}

/*Checks all script parameters for obsolete values (example: mission disk using GC_ templates)*/
void ScriptDialog::checkParametersForGC(void)
{
	SidesList *sidesListP = TheSidesList;
	Int i;
	if (m_staticThis) sidesListP = &m_staticThis->m_sides;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			updateScriptWarning(pScr);
			if (pScr->hasWarnings())
			{	//check if this is using invalid GC parameters
				patchScriptParametersForGC(pScr);
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			pGroup->setWarnings(false);
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				updateScriptWarning(pScr);
				if (pScr->hasWarnings()) {
					//check if this is using invalid GC parameters.
					patchScriptParametersForGC(pScr);
					if (pScr->hasWarnings())	//patching may have removed warning
						pGroup->setWarnings(true);
				}
			}
		}
	}	
}

BOOL ScriptDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_autoUpdateWarnings=::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "AutoVerifyScripts", 1);

	CButton *pButton = (CButton*)GetDlgItem(IDC_AUTO_VERIFY);
	pButton->SetCheck(m_autoUpdateWarnings ? 1:0);

	//if user wants to check warnings manually, enable the verify button
	CWnd *pWnd = GetDlgItem(IDC_VERIFY);
	pWnd->EnableWindow(!m_autoUpdateWarnings);

	m_staticThis = this;

	m_bDisableDeepScan=::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "DisableDeepScan", 1);
	pButton = (CButton*)GetDlgItem(IDC_DEEPSCAN);
	pButton->SetCheck(m_bDisableDeepScan ? 1:0);
	OnDisableDeepScan();

	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);

	// replace current Tree Dialog with CSDTreeCtrl

	CRect rect;
	mTree = new CSDTreeCtrl;
	pTree->GetWindowRect(&rect);
	ScreenToClient(&rect);

	DWORD style = pTree->GetStyle();
	mTree->Create(style, rect, this, IDC_SCRIPT_TREE);
	pTree->DestroyWindow();

	pTree = (CTreeCtrl*) GetDlgItem(IDC_SCRIPT_TREE);
	// pTree should == mTree now.

	Bool didSelect = false;
	ScriptList::updateDefaults();
	m_sides = *TheSidesList;
	EditParameter::setCurSidesList(&m_sides);
	Int i;

	// Flag to indicate if cache was successfully loaded
	Bool loadedFromCache = false;

	if(m_autoUpdateWarnings){
		// Try loading cached warning state first
		if (LoadScriptWarningsState()) {
			DEBUG_LOG(("ScriptDialog: Loaded script warning state from cache.\n"));
			loadedFromCache = true;
			updateWarnings(true);
			updateIcons(TVI_ROOT);
		} else {
			DEBUG_LOG(("ScriptDialog: No valid cache found. Will update warnings normally.\n"));
		}

		// If cache didn't load, fall back to the expensive update
		// if (!g_didScriptWarningsUpdate) {
			if (!loadedFromCache) {
				updateWarnings(true);
				DEBUG_LOG(("ScriptDialog: Ran updateWarnings(true) due to missing cache.\n"));
			}
			// g_didScriptWarningsUpdate = true;
		// }
	}

	if (pTree) {
		m_imageList.Create(IDB_FOLDERSCRIPT, 16, 2, ILC_COLOR4);
		pTree->SetImageList(&m_imageList, TVSIL_STATE);
		for (i=0; i<m_sides.getNumSides(); i++) {
			HTREEITEM hItem = addPlayer(i);
			if (!didSelect && hItem != NULL) {
				pTree->SelectItem(hItem);
				didSelect = true;
			}
		}
		pTree->SetFocus();
	}

	CRect top;
	GetWindowRect(&top);
	top.top = ::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "Top", top.top);
	top.left =::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "Left", top.left);
	SetWindowPos(NULL, top.left, top.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	

	m_bCompressed=::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "CompressScripts", 1);
	pButton = (CButton*)GetDlgItem(IDC_COMPRESS);
	pButton->SetCheck(m_bCompressed ? 1:0);
	OnCompress();

	m_bNewIcons=::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "NewIcons", 1);
	pButton = (CButton*)GetDlgItem(IDC_NEWICONS);
	pButton->SetCheck(m_bNewIcons ? 1:0);
	OnNewIcons();

	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HTREEITEM ScriptDialog::addPlayer(Int playerIndx)
{

	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	TVINSERTSTRUCT ins;
	Dict *d = m_sides.getSideInfo(playerIndx)->getDict();
	AsciiString name = d->getAsciiString(TheKey_playerName);
	UnicodeString uni = d->getUnicodeString(TheKey_playerDisplayName);
	AsciiString fmt;
	if (name.isEmpty())
		fmt.format("%s", NEUTRAL_NAME_STR);
	else
		fmt.format("%s",name.str());
	::memset(&ins, 0, sizeof(ins));
	ListType lt;
	lt.m_objType=ListType::PLAYER_TYPE;
	lt.m_playerIndex = playerIndx;
	ins.hParent = TVI_ROOT;
	ins.hInsertAfter = TVI_LAST;
	ins.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
	ins.item.state = INDEXTOSTATEIMAGEMASK(1);
	ins.item.stateMask = TVIS_STATEIMAGEMASK ;
	ins.item.lParam = lt.ListToInt();
	ins.item.pszText = (char *)fmt.str();
	ins.item.cchTextMax = 0;				
	HTREEITEM hItem = pTree->InsertItem(&ins);
	ScriptList *pSL = m_sides.getSideInfo(playerIndx)->getScriptList();
	if (pSL) {
		addScriptList(hItem, playerIndx, pSL);
	}
	updateIcons(TVI_ROOT);
	return hItem;
}		

void ScriptDialog::setIconGroup(HTREEITEM item)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	int iconIndex = 0;

	if (getCurGroup()->hasWarnings() && getCurGroup()->isActive() == FALSE )
		iconIndex = 7;
	else if (getCurGroup()->hasWarnings())
		iconIndex = 3;
	else if (getCurGroup()->isActive())
		iconIndex = 1;
	else
		iconIndex = 5;

	pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(iconIndex), TVIS_STATEIMAGEMASK);
}

void ScriptDialog::setIconScript(HTREEITEM item)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	int iconIndex = 0;

	if (getCurScript()->hasWarnings() && getCurScript()->isActive() == FALSE)
		iconIndex = 8;
	else if (getCurScript()->hasWarnings())
		iconIndex = 4;
	else if (getCurScript()->isActive())
		iconIndex = 2;
	else
		iconIndex = 6;

	pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(iconIndex), TVIS_STATEIMAGEMASK);
}

void ScriptDialog::SetItemIconIfDifferent(CTreeCtrl* pTree, HTREEITEM hItem, int desiredIndex)
{
	int currentState = (pTree->GetItemState(hItem, TVIS_STATEIMAGEMASK) & TVIS_STATEIMAGEMASK) >> 12;
	if (currentState != desiredIndex) {
		pTree->SetItemState(hItem, INDEXTOSTATEIMAGEMASK(desiredIndex), TVIS_STATEIMAGEMASK);
	}
}

Bool ScriptDialog::updateIcons(HTREEITEM hItem)
{
	const ListType saveList = m_curSelection;
	Bool warnings = false;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	HTREEITEM child = pTree->GetChildItem(hItem);

	while (child != NULL) {
		ListType lt;
		lt.IntToList(pTree->GetItemData(child));

		/// player type
		if (lt.m_objType == ListType::PLAYER_TYPE)
		{
			if (updateIcons(child)) {
				// pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);
				SetItemIconIfDifferent(pTree, child, 3);
			} else {
				// pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
				SetItemIconIfDifferent(pTree, child, 1);
			}
		}

		/// script group
		else if (lt.m_objType == ListType::GROUP_TYPE)
		{
			m_curSelection = lt;
			if (updateIcons(child)) {
				// pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);
				// SetItemIconIfDifferent(pTree, child, 3);
				warnings = true;
			}
			setIconGroup(child);
		}

		/// script
		else
		{
			m_curSelection = lt;
			Script *pScr = getCurScript();
			DEBUG_ASSERTCRASH(pScr, ("Unexpected."));
			if (pScr) {
				if (pScr->hasWarnings()) {
					// pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(4), TVIS_STATEIMAGEMASK);
					// SetItemIconIfDifferent(pTree, child, 4);
					warnings = true;
				}
				setIconScript(child);
			}
		}

		child = pTree->GetNextSiblingItem(child);
	}
	m_curSelection = saveList;
	return warnings;
}

void ScriptDialog::addScriptList(HTREEITEM hPlayer, Int playerIndex, ScriptList *pSL)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	TVINSERTSTRUCT ins;
	Int groupNdx;
	ScriptGroup *pGroup = pSL->getScriptGroup();
	Bool warnings = false;
	for (groupNdx = 0; pGroup; groupNdx++,pGroup=pGroup->getNext()) {
		AsciiString fmt;
		if (pGroup->getName().isEmpty())
			continue;
		else
			fmt = formatScriptLabel(pGroup);
		::memset(&ins, 0, sizeof(ins));
		ListType lt;
		lt.m_objType=ListType::GROUP_TYPE;
		lt.m_playerIndex = playerIndex;
		lt.m_groupIndex = groupNdx;
		ins.hParent = hPlayer;
		ins.hInsertAfter = TVI_LAST;
		ins.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
		ins.item.lParam = lt.ListToInt();
		ins.item.pszText = (char *)fmt.str();
		ins.item.cchTextMax = 0;				
		ins.item.state = INDEXTOSTATEIMAGEMASK(1);
		if (pGroup->hasWarnings()) {
			ins.item.state = INDEXTOSTATEIMAGEMASK(3);
			warnings = true;
		}
		ins.item.stateMask = TVIS_STATEIMAGEMASK ;
		HTREEITEM hItem = pTree->InsertItem(&ins);
		Script *pScr = pGroup->getScript();
		if (pScr) {
			Int scriptNdx;
			for (scriptNdx = 0; pScr; scriptNdx++,pScr=pScr->getNext()) {
				AsciiString fmt;
				if (pScr->getName().isEmpty())
					continue;
				fmt = formatScriptLabel(pScr);
				::memset(&ins, 0, sizeof(ins));
				ListType lt;
				lt.m_objType=ListType::SCRIPT_IN_GROUP_TYPE;
				lt.m_playerIndex = playerIndex;
				lt.m_groupIndex = groupNdx;
				lt.m_scriptIndex = scriptNdx;
				ins.hParent = hItem;
				ins.hInsertAfter = TVI_LAST;
				ins.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
				ins.item.state = INDEXTOSTATEIMAGEMASK(2);
				if (pScr->hasWarnings()) {
					ins.item.state = INDEXTOSTATEIMAGEMASK(4);
					warnings = true;
				}
				ins.item.stateMask = TVIS_STATEIMAGEMASK ;
				ins.item.lParam = lt.ListToInt();
				ins.item.pszText = (char *)fmt.str();
				ins.item.cchTextMax = 0;				
				/*HTREEITEM hItem =*/ pTree->InsertItem(&ins);
			}
		}
	}
	Script *pScr = pSL->getScript();
	if (pScr) {
		Int scriptNdx;
		for (scriptNdx = 0; pScr; scriptNdx++,pScr=pScr->getNext()) {
			AsciiString fmt;
			if (pScr->getName().isEmpty())
				continue;
			fmt = formatScriptLabel(pScr);
			::memset(&ins, 0, sizeof(ins));
			ListType lt;
			lt.m_objType=ListType::SCRIPT_IN_PLAYER_TYPE;
			lt.m_playerIndex = playerIndex;
			lt.m_groupIndex = 0;
			lt.m_scriptIndex = scriptNdx;
			ins.hParent = hPlayer;
			ins.hInsertAfter = TVI_LAST;
			ins.item.mask = TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
			ins.item.state = INDEXTOSTATEIMAGEMASK(2);
			if (pScr->hasWarnings()) {
				ins.item.state = INDEXTOSTATEIMAGEMASK(4);
				warnings = true;
			}
			ins.item.stateMask = TVIS_STATEIMAGEMASK ;
			ins.item.lParam = lt.ListToInt();
			ins.item.pszText = (char *)fmt.str();
			ins.item.cchTextMax = 0;				
			/*HTREEITEM hItem =*/ pTree->InsertItem(&ins);
		}
	}
	if (warnings) {
		pTree->SetItemState(hPlayer, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);
	} else {
		pTree->SetItemState(hPlayer, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
	}
}

void ScriptDialog::reloadPlayer(Int playerIndex, ScriptList *pSL)
{
	updateWarnings();
	
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);

	// Disable redraw to prevent flickering
	pTree->SetRedraw(FALSE);

	HTREEITEM player = pTree->GetChildItem(TVI_ROOT);
	while (player != NULL) {
		TVITEM item;
		::memset(&item, 0, sizeof(item));
		item.mask = TVIF_HANDLE|TVIF_PARAM;
		item.hItem = player;
		pTree->GetItem(&item);
		ListType lt;
		lt.IntToList(item.lParam);
		if (lt.m_playerIndex==playerIndex) {
			break;
		}
		player = pTree->GetNextSiblingItem(player);
	}
	DEBUG_ASSERTCRASH(player, ("Couldn't find player."));
	if (!player) {
		pTree->SetRedraw(TRUE);
		return;
	}

	ListType currentSel = m_curSelection;
	if (currentSel.m_objType == ListType::SCRIPT_IN_GROUP_TYPE && currentSel.m_scriptIndex > 0) {
		--currentSel.m_scriptIndex;
	}

	// Delete all children under the player item
	HTREEITEM child;
	do {
		child = pTree->GetChildItem(player);
		if (child) pTree->DeleteItem(child);
	} while (child);

	// Restore selection and add scripts
	m_curSelection = currentSel;
	addScriptList(player, playerIndex, pSL);

	// Re-enable redraw
	pTree->SetRedraw(TRUE);
	pTree->Invalidate();
	pTree->UpdateWindow();
}

void ScriptDialog::updateSelection(ListType sel)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	HTREEITEM item = findItem(sel, TRUE);
	if (item) {
		pTree->SelectItem(item);
	}
}

HTREEITEM ScriptDialog::findItem(ListType sel, Bool failSafe)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	HTREEITEM player = pTree->GetChildItem(TVI_ROOT);
	TVITEM item;
	while (player != NULL) {
		::memset(&item, 0, sizeof(item));
		item.mask = TVIF_HANDLE|TVIF_PARAM;
		item.hItem = player;
		pTree->GetItem(&item);
		ListType lt;
		lt.IntToList(item.lParam);
		if (lt.m_playerIndex==sel.m_playerIndex) {
			break;
		}
		player = pTree->GetNextSiblingItem(player);
	}
	DEBUG_ASSERTCRASH(player, ("Couldn't find player."));
	if (!player) return NULL;
	if (sel.m_objType == ListType::PLAYER_TYPE) {
		return player;
	}

	HTREEITEM group;
	if (sel.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE) {
		group = player; // top level scripts are grouped under player.
	} else {
		group = pTree->GetChildItem(player);
		while (group != NULL) {
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM;
			item.hItem = group;
			pTree->GetItem(&item);
			ListType lt;
			lt.IntToList(item.lParam);
			if (lt.m_groupIndex==sel.m_groupIndex) {
				break;
			}
			DEBUG_ASSERTCRASH(lt.m_objType == ListType::GROUP_TYPE, ("Not group"));
			group = pTree->GetNextSiblingItem(group);
		}
	} 
	DEBUG_ASSERTCRASH(group, ("Couldn't find group."));
	if (!group) return NULL;
	if (sel.m_objType == ListType::GROUP_TYPE) {
		return group;
	}

	HTREEITEM script;
	for (script = pTree->GetChildItem(group); script != NULL; script = pTree->GetNextSiblingItem(script)) {
		::memset(&item, 0, sizeof(item));
		item.mask = TVIF_HANDLE|TVIF_PARAM;
		item.hItem = script;
		pTree->GetItem(&item);
		ListType lt;
		lt.IntToList(item.lParam);
		if (sel.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE && lt.m_objType == ListType::GROUP_TYPE) {
			continue;
		}
		DEBUG_ASSERTCRASH(lt.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE || lt.m_objType == ListType::SCRIPT_IN_GROUP_TYPE, ("Not script"));
		if (lt.m_scriptIndex==sel.m_scriptIndex) {
			break;
		}
	}

	if (script || !failSafe) {
		DEBUG_ASSERTCRASH(script, ("Couldn't find script.")); 
		return script;
	}

	// at least select the group if possible.
	return group;
}

void ScriptDialog::OnNewFolder() 
{
	Int ndx;
	if (m_curSelection.m_objType == ListType::PLAYER_TYPE) {
		ndx = 0;
	} else {
		ndx = m_curSelection.m_groupIndex+1;
	}
	ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	if (pSL) {
		ListType savSel = m_curSelection;
		ScriptGroup *pNewGroup = newInstance( ScriptGroup);
		EditGroup editDlg(pNewGroup);
		if (IDOK==editDlg.DoModal()) {
			pSL->addGroup(pNewGroup, ndx);
			reloadPlayer(savSel.m_playerIndex, pSL);
			savSel.m_groupIndex = ndx;
			savSel.m_objType = ListType::GROUP_TYPE;
			updateSelection(savSel);
		} else {
			pNewGroup->deleteInstance();
		}
	}
	updateIcons(TVI_ROOT);
}

void ScriptDialog::OnNewScript() 
{
	Script *pNewScript = newInstance( Script);

	Int id = ScriptList::getNextID();
	AsciiString name;
	name.format("Script %d", id);
	pNewScript->setName(name);

	Condition *pFalse1 = newInstance( Condition)(Condition::CONDITION_TRUE);
	OrCondition *pOr = newInstance( OrCondition);
	pOr->setFirstAndCondition(pFalse1);
	pNewScript->setOrCondition(pOr);

	ScriptAction *action = newInstance( ScriptAction)(ScriptAction::NO_OP);
	pNewScript->setAction(action);

	CPropertySheet editDialog;
	editDialog.Construct(name.str());
	ScriptProperties sp;
	sp.setScript(pNewScript);
	ScriptConditionsDlg sc;
	sc.setScript(pNewScript);
	ScriptActionsTrue st;
	st.setScript(pNewScript);
	ScriptActionsFalse sf;
	sf.setScript(pNewScript);
	editDialog.AddPage(&sp);
	editDialog.AddPage(&sc);
	editDialog.AddPage(&st);
	editDialog.AddPage(&sf);

	if (IDOK == editDialog.DoModal()) {
		insertScript(pNewScript);
	}	else {
		pNewScript->deleteInstance();
	}
	updateIcons(TVI_ROOT);
}		

void ScriptDialog::insertScript(Script *pNewScript)
{
	Int ndx;
	ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	if (pSL) {
		ListType savSel = m_curSelection;
		Bool inGroup = savSel.m_objType == ListType::GROUP_TYPE	||
			savSel.m_objType == ListType::SCRIPT_IN_GROUP_TYPE;
		if (inGroup) {
			if (savSel.m_objType == ListType::GROUP_TYPE ) {
				ndx = 0;
			} else {
				ndx = savSel.m_scriptIndex+1;
			}
			Int groupNdx;
			ScriptGroup *pGroup = pSL->getScriptGroup();
			for (groupNdx = 0; pGroup; groupNdx++,pGroup=pGroup->getNext()) {
				if (groupNdx == savSel.m_groupIndex) {
					pGroup->addScript(pNewScript, ndx);
					savSel.m_objType = ListType::SCRIPT_IN_GROUP_TYPE;
					savSel.m_scriptIndex = ndx;
					break;
				}
			}
		} else {
			if (m_curSelection.m_objType == ListType::PLAYER_TYPE ) {
				ndx = 0;
			} else {
				ndx = m_curSelection.m_scriptIndex+1;
			}
			pSL->addScript(pNewScript, ndx);
			savSel.m_objType = ListType::SCRIPT_IN_PLAYER_TYPE;
			savSel.m_scriptIndex = ndx;
		}

		reloadPlayer(savSel.m_playerIndex, pSL);
		updateSelection(savSel);
	}
	updateIcons(TVI_ROOT);
}

void ScriptDialog::OnEditScript() 
{
	Script *pScript = getCurScript();
	ScriptGroup *pGroup = getCurGroup();
	DEBUG_ASSERTCRASH(pScript || pGroup, ("Null script."));
	if (pScript == NULL) {
		CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
		HTREEITEM item = findItem(m_curSelection);
		if (pGroup) {
			EditGroup editDlg(pGroup);
			if (IDOK==editDlg.DoModal()) {
				if (item) {
					pTree->SetItemText(item, pGroup->getName().str());
					pTree->SelectItem(NULL);
					updateWarnings();
					pTree->SelectItem(item);
				}
			}
		}
		updateIcons(TVI_ROOT);
		pTree->SetItemText(item, formatScriptLabel(pGroup).str());
		return;
	}

	Script *pDup = pScript->duplicate();

	CPropertySheet editDialog;
	editDialog.Construct(pScript->getName().str());
	ScriptProperties sp;
	sp.setScript(pDup);
	ScriptConditionsDlg sc;
	sc.setScript(pDup);
	ScriptActionsTrue st;
	st.setScript(pDup);
	ScriptActionsFalse sf;
	sf.setScript(pDup);
	editDialog.AddPage(&sp);
	editDialog.AddPage(&sc);
	editDialog.AddPage(&st);
	editDialog.AddPage(&sf);

	if (IDOK == editDialog.DoModal()) {
		pScript->updateFrom(pDup);
		CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
		HTREEITEM item = findItem(m_curSelection);
		if (item) {
			pTree->SetItemText(item, formatScriptLabel(pScript).str());
			pTree->SelectItem(NULL);
			pScript->setDirty(true);
			updateWarnings();
			pTree->SelectItem(item); // Updates the comment field & text field.
		}
	}
	updateIcons(TVI_ROOT);
	pDup->deleteInstance();
}

void ScriptDialog::OnCopyScript() 
{
    Script *pScript = getCurScript();
    ScriptGroup *pGroup = getCurGroup();

    // If a script is selected, copy just that script
	if (pScript) {
		Script *pDup = pScript->duplicate();
		AsciiString newName = pDup->getName();
		newName.concat(" C");
		pDup->setName(newName);
		insertScript(pDup);
		updateIcons(TVI_ROOT);
		return;
	}

    // If a folder/group is selected, copy the entire group
    if (pGroup && m_curSelection.m_objType == ListType::GROUP_TYPE) {
        ScriptGroup* pNewGroup = newInstance(ScriptGroup);
        AsciiString newGroupName = pGroup->getName();
        newGroupName.concat(" Copy");
        pNewGroup->setName(newGroupName);
        pNewGroup->setActive(pGroup->isActive());
        pNewGroup->setSubroutine(pGroup->isSubroutine());

        // Copy all scripts in group, preserving order
        Int scriptIndex = 0;
        for (Script* pScr = pGroup->getScript(); pScr; pScr = pScr->getNext(), ++scriptIndex) {
            Script* pDup = pScr->duplicate();
            AsciiString scriptName = pDup->getName();
            scriptName.concat(" C");
            pDup->setName(scriptName);
            pNewGroup->addScript(pDup, scriptIndex); // Indexed insert
        }

        ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
        if (pSL) {
            Int insertIndex = m_curSelection.m_groupIndex + 1; // insert after current group
            pSL->addGroup(pNewGroup, insertIndex);
            reloadPlayer(m_curSelection.m_playerIndex, pSL);
        }

        updateIcons(TVI_ROOT);
    }
}

void ScriptDialog::OnDelete() 
{
	Script *pScript = getCurScript();
	ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	if (pSL) {
		Bool inGroup = m_curSelection.m_objType != ListType::SCRIPT_IN_PLAYER_TYPE;
		if (inGroup) {
			Int groupNdx;
			ScriptGroup *pGroup = pSL->getScriptGroup();
			for (groupNdx = 0; pGroup; groupNdx++,pGroup=pGroup->getNext()) {
				if (groupNdx == m_curSelection.m_groupIndex) {
					if (m_curSelection.m_objType == ListType::GROUP_TYPE) {
						pSL->deleteGroup(pGroup);
						m_curSelection.m_objType = ListType::PLAYER_TYPE;
					} else {
						pGroup->deleteScript(pScript);
						if (pGroup->getScript()==NULL) {
							m_curSelection.m_objType = ListType::GROUP_TYPE;
						}
					}
					break;
				}
			}
		} else {
			pSL->deleteScript(pScript);
			if (pSL->getScript()==NULL) {
				m_curSelection.m_objType = ListType::PLAYER_TYPE;
			}
		}
 		reloadPlayer(m_curSelection.m_playerIndex, pSL);
		updateSelection(m_curSelection);
	}
	updateIcons(TVI_ROOT);
}

class LocalMFCFileOutputStream : public OutputStream
{
protected:
	CFile *m_file;
public:
	LocalMFCFileOutputStream(CFile *pFile):m_file(pFile) {};
	virtual Int write(const void *pData, Int numBytes) {
		Int numBytesWritten = 0;
		try {
			m_file->Write(pData, numBytes);
			numBytesWritten = numBytes;
		} catch(...) {
			DEBUG_CRASH(("threw exception in LocalMFCFileOutputStream"));
		}
		return(numBytesWritten);
	};
};

void ScriptDialog::markWaypoint(MapObject *pObj)
{
	Bool exists;
	if (!pObj) return;
	if (pObj->isWaypoint() && !pObj->getProperties()->getBool(TheKey_exportWithScript, &exists)) {
		pObj->getProperties()->setBool(TheKey_exportWithScript, true);
		CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
		Int curID = pObj->getWaypointID();
		Int i;
		for (i = 0; i<pDoc->getNumWaypointLinks(); i++) {
//			Bool gotLocation=false;
			Int waypointID1, waypointID2;
			pDoc->getWaypointLink(i, &waypointID1, &waypointID2);
			if (curID == waypointID1) {
				markWaypoint(pDoc->getWaypointByID(waypointID2));
			}
			if (curID == waypointID2) {
				markWaypoint(pDoc->getWaypointByID(waypointID1));
			}
		}
	}
}

/** Looks for referenced waypoints & teams. */
void ScriptDialog::scanParmForWaypointsAndTeams(Parameter *pParm, Bool doUnits, Bool doWaypoints, Bool doTriggers, Bool doTeams)
{
	if (pParm->getParameterType() == Parameter::WAYPOINT && doWaypoints) {
		AsciiString waypointName  = pParm->getString();
		MapObject *pObj;
		for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
			if (pObj->isWaypoint() && pObj->getWaypointName()==waypointName) {
				markWaypoint(pObj);
			}
		}
	}
	if (pParm->getParameterType() == Parameter::WAYPOINT_PATH && doWaypoints) {
		AsciiString waypointPathLabel = pParm->getString();
		MapObject *pObj;
		for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
			if (pObj->isWaypoint() ) {
				Bool exists;
				if (waypointPathLabel == pObj->getProperties()->getAsciiString(TheKey_waypointPathLabel1, &exists)) {
					markWaypoint(pObj);
				}
				if (waypointPathLabel == pObj->getProperties()->getAsciiString(TheKey_waypointPathLabel2, &exists)) {
					markWaypoint(pObj);
				}
				if (waypointPathLabel == pObj->getProperties()->getAsciiString(TheKey_waypointPathLabel3, &exists)) {
					markWaypoint(pObj);
				}
			}
		}
	}
	if (pParm->getParameterType() == Parameter::TEAM) {
		AsciiString teamName  = pParm->getString();
		TeamsInfo * pInfo = m_sides.findTeamInfo(teamName);
		if (pInfo && doTeams) {
			pInfo->getDict()->setBool(TheKey_exportWithScript, true);
		}	 
		if (doUnits) {
			MapObject *pObj;
			for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
				Bool exists;
				AsciiString objsTeamName = pObj->getProperties()->getAsciiString(TheKey_originalOwner, &exists);
				if (objsTeamName==teamName) {
					pObj->getProperties()->setBool(TheKey_exportWithScript, true);
				}
			}
		}
	}
	if (pParm->getParameterType() == Parameter::UNIT) {
		AsciiString unitName  = pParm->getString();
		if (doUnits) {
			MapObject *pObj;
			for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
				Bool exists;
				AsciiString objsTeamName = pObj->getProperties()->getAsciiString(TheKey_originalOwner, &exists);
				AsciiString objsUnitName = pObj->getProperties()->getAsciiString(TheKey_objectName, &exists);
				if (objsUnitName==unitName) {
					pObj->getProperties()->setBool(TheKey_exportWithScript, true);
					TeamsInfo * pInfo = m_sides.findTeamInfo(objsTeamName);
					if (pInfo) {
						pInfo->getDict()->setBool(TheKey_exportWithScript, true);
					}	 
				}
			}
		}
	}
	if (pParm->getParameterType() == Parameter::TRIGGER_AREA && doTriggers) {
		PolygonTrigger *pTrig;
		for (pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
			if (pTrig->getTriggerName() == pParm->getString()) {
				pTrig->setDoExportWithScripts(true);
			}
		}
	}
}

/** Looks for referenced waypoints & teams. */
void ScriptDialog::scanForWaypointsAndTeams(Script *pScript, Bool doUnits, Bool doWaypoints, Bool doTriggers, Bool doTeams)
{
	pScript->setWarnings(false);
	OrCondition *pOr;
	for (pOr= pScript->getOrCondition(); pOr; pOr = pOr->getNextOrCondition()) {
		Condition *pCondition;
		for (pCondition = pOr->getFirstAndCondition(); pCondition; pCondition = pCondition->getNext()) {
			Int i;
			for (i=0; i<pCondition->getNumParameters(); i++) {
				scanParmForWaypointsAndTeams(pCondition->getParameter(i), doUnits, doWaypoints, doTriggers, doTeams);
			}
		}
	}
	ScriptAction *pAction;
	for (pAction = pScript->getAction(); pAction; pAction = pAction->getNext()) {
		pAction->setWarnings(false);
		Int i;
		for (i=0; i<pAction->getNumParameters(); i++) {
			scanParmForWaypointsAndTeams(pAction->getParameter(i), doUnits, doWaypoints, doTriggers, doTeams);
		}
	}
}

#define K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_1 1
#define K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_2 2

/** Write out selected scripts, and possibly waypoints, trigger areas & teams. */
void ScriptDialog::OnSave() 
{
	Bool doWaypoints = true;
	Bool doTriggerAreas = true;
	Bool doUnits = true;
	Bool doAllScripts = true;
	Bool doSides = true;
	Bool doTeams = true;
	Int	 i;

	ExportScriptsOptions optionsDlg;
	if (IDCANCEL == optionsDlg.DoModal()) {
		return;
	}
	doWaypoints = optionsDlg.getDoWaypoints();
	doUnits = optionsDlg.getDoUnits();
	doTeams = optionsDlg.getDoTeams(); // you'll implement this getter
	doTriggerAreas = optionsDlg.getDoTriggers();
	doAllScripts = optionsDlg.getDoAllScripts();
	doSides = optionsDlg.getDoSides();

	Script *pScript = getCurScript();
	ScriptGroup *pGroup = getCurGroup();

	ScriptList *scripts[MAX_PLAYER_COUNT];
	for (i=0; i<MAX_PLAYER_COUNT; i++) {
		scripts[i] = NULL;
	}

	CFileDialog fileDlg(false, ".scb", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"Script files (.scb)|*.scb||", this);

	Int result = fileDlg.DoModal();

	// Open document dialog may change working directory, 
	// change it back.
	char buf[_MAX_PATH];
	::GetModuleFileName(NULL, buf, sizeof(buf));
	char *pEnd = buf + strlen(buf);
	while (pEnd != buf) {
		if (*pEnd == '\\') {
			*pEnd = 0;
			break;
		}
		pEnd--;
	}
	::SetCurrentDirectory(buf);
	if (IDCANCEL==result) {
		return;
	}

	MapObject *pObj;
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		pObj->getProperties()->setBool(TheKey_exportWithScript, false);
		if (pObj->isWaypoint() && (doWaypoints&&doAllScripts)) {
			pObj->getProperties()->setBool(TheKey_exportWithScript, true);
		}
	}

	PolygonTrigger *pTrig;
	for (pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
		pTrig->setDoExportWithScripts(doAllScripts && doTriggerAreas);
		if (pTrig->isWaterArea()) {
			pTrig->setDoExportWithScripts(false); // don't export water.
		}
	}

	// DEBUG_LOG(("doTeams %s", doTeams ? "true" : "false"));
	// DEBUG_LOG(("doAllScripts %s", doAllScripts ? "true" : "false"));

	for (i = 0; i < m_sides.getNumTeams(); i++) {
		m_sides.getTeamInfo(i)->getDict()->setBool(TheKey_exportWithScript, doAllScripts);
	}
	Int numScriptLists = 0;
	if (doAllScripts) {
		numScriptLists = m_sides.getNumSides();
		for (i=0; i<numScriptLists; i++) {
			scripts[i] = m_sides.getSideInfo(i)->getScriptList();
		}
	} else {
		numScriptLists = 1;
		if (m_curSelection.m_objType == ListType::PLAYER_TYPE) {
			scripts[0] = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
			scripts[0] = scripts[0]->duplicate();
		}	else if (pGroup) {
			pGroup = pGroup->duplicate();
			scripts[0] = newInstance( ScriptList);
			scripts[0]->addGroup(pGroup, 0);
		} else if (pScript) {
			pScript = pScript->duplicate();
			scripts[0] = newInstance( ScriptList);
			scripts[0]->addScript(pScript, 0);
		}
		if (scripts[0] == NULL) {
			::AfxMessageBox("No scripts selected - aborting export.", MB_OK);
			return;
		}
	}
	for (i=0; i<numScriptLists; i++) {
		ScriptList *pSL = scripts[i];
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			scanForWaypointsAndTeams(pScr, doUnits, doWaypoints, doTriggerAreas, doTeams);
		}
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				scanForWaypointsAndTeams(pScr, doUnits, doWaypoints, doTriggerAreas, doTeams);
			}
		}
	}


	CString path = fileDlg.GetPathName();

	CFile theFile(path, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite|CFile::typeBinary);
	try {
		LocalMFCFileOutputStream theStream(&theFile);
		DataChunkOutput chunkWriter(&theStream);
		ScriptList::WriteScriptsDataChunk(chunkWriter, scripts, numScriptLists);

		/***************Players DATA ***************/
		chunkWriter.openDataChunk("ScriptsPlayers", 	K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_2);
		chunkWriter.writeInt(doSides);
		if (doAllScripts || doSides) {
			chunkWriter.writeInt(m_sides.getNumSides());
			for (i=0; i<m_sides.getNumSides(); i++) {
				AsciiString name = m_sides.getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName);
				chunkWriter.writeAsciiString(name);

				if (doSides) {
					// The user has requested that the sides get exported.
					chunkWriter.writeDict(*m_sides.getSideInfo(i)->getDict());
				}

			}
		} else  {
			chunkWriter.writeInt(1);
			chunkWriter.writeAsciiString("**SELECTION**");
		}
		chunkWriter.closeDataChunk();

		/***************OBJECTS DATA ***************/
		chunkWriter.openDataChunk("ObjectsList", 	K_OBJECTS_VERSION_3);
			
		for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) 
		{
			if (!pObj->getProperties()->getBool(TheKey_exportWithScript)) {
				continue;
			};
			chunkWriter.openDataChunk("Object", 	K_OBJECTS_VERSION_3);
				Coord3D loc = *pObj->getLocation();
				chunkWriter.writeReal( loc.x);
				chunkWriter.writeReal( loc.y);
				chunkWriter.writeReal( loc.z);
				chunkWriter.writeReal( pObj->getAngle());
				chunkWriter.writeInt(pObj->getFlags()); 
				chunkWriter.writeAsciiString(pObj->getName());	

				chunkWriter.writeDict(*pObj->getProperties());	

			chunkWriter.closeDataChunk();
		}
		chunkWriter.closeDataChunk();

		/***************POLYGON TRIGGERS DATA ***************/
		chunkWriter.openDataChunk("PolygonTriggers", 	K_TRIGGERS_VERSION_3);
			
			PolygonTrigger *pTrig;
			Int count = 0;
			for (pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
				if (pTrig->doExportWithScripts()) {
					count++;
				}
			}
			chunkWriter.writeInt(count); 
			for (pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
				if (!pTrig->doExportWithScripts()) continue;
				chunkWriter.writeAsciiString(pTrig->getTriggerName());	
				chunkWriter.writeInt(pTrig->getID()); 
				chunkWriter.writeByte(pTrig->isWaterArea());
				chunkWriter.writeByte(pTrig->isRiver());
				chunkWriter.writeInt(pTrig->getRiverStart());
				chunkWriter.writeInt(pTrig->getNumPoints()); 
				Int i;
				for (i=0; i<pTrig->getNumPoints(); i++) {
					ICoord3D loc = *pTrig->getPoint(i);
					chunkWriter.writeInt( loc.x);
					chunkWriter.writeInt( loc.y);
					chunkWriter.writeInt( loc.z);
				}
			}
		chunkWriter.closeDataChunk();
 		/***************TEAMS DATA ***************/
		chunkWriter.openDataChunk("ScriptTeams", 	K_LOCAL_TEAMS_VERSION_1);
			for (i = 0; i < m_sides.getNumTeams(); i++)
			{
				if (m_sides.getTeamInfo(i)->getDict()->getBool(TheKey_exportWithScript)) {
					chunkWriter.writeDict(*m_sides.getTeamInfo(i)->getDict());
				}
			}

			
		chunkWriter.closeDataChunk();
 		/***************WAYPOINTS DATA ***************/
			CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
			Int i;
			count = 0;
			for (i = 0; i<pDoc->getNumWaypointLinks(); i++) {
					Int waypointID1, waypointID2;
				MapObject *pWay1, *pWay2;
				pDoc->getWaypointLink(i, &waypointID1, &waypointID2);
				pWay1 = pDoc->getWaypointByID(waypointID1);
				pWay2 = pDoc->getWaypointByID(waypointID2);
				if (pWay1 && pWay2) {
					if (!pWay1->getProperties()->getBool(TheKey_exportWithScript)) {
						continue;
					};
					if (!pWay1->getProperties()->getBool(TheKey_exportWithScript)) {
						continue;
					};
					count++;
				}
			}

		chunkWriter.openDataChunk("WaypointsList", 	K_WAYPOINTS_VERSION_1);
			chunkWriter.writeInt(count);
			for (i = 0; i<pDoc->getNumWaypointLinks(); i++) {
					Int waypointID1, waypointID2;
				MapObject *pWay1, *pWay2;
				pDoc->getWaypointLink(i, &waypointID1, &waypointID2);
				pWay1 = pDoc->getWaypointByID(waypointID1);
				pWay2 = pDoc->getWaypointByID(waypointID2);
				if (pWay1 && pWay2) {
					if (!pWay1->getProperties()->getBool(TheKey_exportWithScript)) {
						continue;
					};
					if (!pWay1->getProperties()->getBool(TheKey_exportWithScript)) {
						continue;
					};
					chunkWriter.writeInt(waypointID1);
					chunkWriter.writeInt(waypointID2);
				}
			}
		chunkWriter.closeDataChunk();

	} catch(...) {
			DEBUG_CRASH(("threw exception in ScriptDialog::OnSave"));
	}
	if (!doAllScripts) {
		scripts[0]->deleteInstance();
	}
	theFile.Close();
}

void ScriptDialog::OnLoad() 
{
	CFileDialog fileDlg(true, ".scb", NULL, 0, 
		"Script files (.scb)|*.scb||", this);

	Int result = fileDlg.DoModal();

	// Open document dialog may change working directory, 
	// change it back.
	char buf[_MAX_PATH];
	::GetModuleFileName(NULL, buf, sizeof(buf));
	char *pEnd = buf + strlen(buf);
	while (pEnd != buf) {
		if (*pEnd == '\\') {
			*pEnd = 0;
			break;
		}
		pEnd--;
	}
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	::SetCurrentDirectory(buf);
	if (IDCANCEL==result) {
		return;
	}

	CString path = fileDlg.GetPathName();

	CachedFileInputStream theInputStream;
	if (theInputStream.open(AsciiString(path))) 
	try {
		ChunkInputStream *pStrm = &theInputStream;
		DataChunkInput file( pStrm );
		m_firstReadObject = NULL;
		m_firstTrigger = NULL;
		m_waypointBase = pDoc->getNextWaypointID();
		m_maxWaypoint = m_waypointBase;
		file.registerParser( AsciiString("PlayerScriptsList"), AsciiString::TheEmptyString, ScriptList::ParseScriptsDataChunk );
		file.registerParser( AsciiString("ObjectsList"), AsciiString::TheEmptyString, ParseObjectsDataChunk );
		file.registerParser( AsciiString("PolygonTriggers"), AsciiString::TheEmptyString, ParsePolygonTriggersDataChunk );
		file.registerParser( AsciiString("WaypointsList"), AsciiString::TheEmptyString, ParseWaypointDataChunk );
		file.registerParser( AsciiString("ScriptTeams"), AsciiString::TheEmptyString, ParseTeamsDataChunk );
		file.registerParser( AsciiString("ScriptsPlayers"), AsciiString::TheEmptyString, ParsePlayersDataChunk );
		if (!file.parse(this)) {
			throw(ERROR_CORRUPT_FILE_FORMAT);
		}
		pDoc->setNextWaypointID(m_maxWaypoint);

		CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
		SidesListUndoable *pUndo = new SidesListUndoable(m_sides, pDoc);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.		
		m_sides = *TheSidesList;

		if (m_firstReadObject) {
			AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, m_firstReadObject);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
			m_firstReadObject = NULL; // undoable owns it now.
		}
		PolygonTrigger *pTrig;
		PolygonTrigger *pNextTrig;
		for (pTrig=m_firstTrigger; pTrig; pTrig = pNextTrig) {
			pNextTrig = pTrig->getNext();
			pTrig->setNextPoly(NULL);
			PolygonTrigger::addPolygonTrigger(pTrig);
		}

		ScriptList *scripts[MAX_PLAYER_COUNT];
		Int count = ScriptList::getReadScripts(scripts);
		Int i;
		for (i=0; i<count; i++) {
			if (scripts[i]->getScript() == NULL && scripts[i]->getScriptGroup()==NULL) continue;
			Int curSide = -1;
			if (count==1) {
				curSide = m_curSelection.m_playerIndex;
			} else {
				Int j;
				for (j=0; j<m_sides.getNumSides(); j++) {
					// Using i as an index assumes that i < m_sides.getNumSides.  Is that safe???
 					AsciiString name = m_sides.getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName);
					if (name == m_readPlayerNames[j]) {
						curSide = j;
						break;
					}
				}
				if (curSide == -1) {
					CString msg = "Could not find player";
					msg += m_readPlayerNames[i].str();
					msg += ", discarding scripts for this player.";
					::AfxMessageBox(msg);
					continue;
				}
			}
			if (curSide>= m_sides.getNumSides()) {
				curSide = 0;
				::AfxMessageBox("Imported scripts came from more players than exist in this map.  Additional scripts moved to Neutral player.");
			}
			ScriptList *pSL = m_sides.getSideInfo(curSide)->getScriptList();

			if (pSL) {
				Script *pScr;
				Script *pNextScr;
				Int j=0;
				for (pScr = scripts[i]->getScript(); pScr; pScr=pNextScr) {
					pNextScr=pScr->getNext();
					pScr->setNextScript(NULL);
					pSL->addScript(pScr, j); //unlink it and add.
					j++;
				}
				j=0;
				ScriptGroup *pGroup;
				ScriptGroup *pNextGroup;
				for (pGroup = scripts[i]->getScriptGroup(); pGroup; pGroup=pNextGroup) {
					pNextGroup=pGroup->getNext();
					pGroup->setNextGroup(NULL);
					pSL->addGroup(pGroup, j);
					j++;
				}
				scripts[i]->discard(); /* Frees the script list, but none of it's children, as they have been
															copied into the current scripts. */
				scripts[i] = NULL;
				//reloadPlayer(curSide, pSL);
			}

		}

		for (i = 0; i < m_sides.getNumSides(); i++) {
			// Make sure that the dialog tree is updated.
			ScriptList *pSL = m_sides.getSideInfo(i)->getScriptList();
			reloadPlayer(i, pSL);
			updateIcons(TVI_ROOT);
		}


	} catch(...) {
   	  	DEBUG_CRASH(("threw exception in ScriptDialog::OnLoad"));
	}
}

/**
* ScriptDialog::ParseObjectsDataChunk - read an objects chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
Bool ScriptDialog::ParseObjectsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	file.m_currentObject = NULL;
	file.registerParser( AsciiString("Object"), info->label, ParseObjectDataChunk );
	return (file.parse(userData));
}

/**
* WorldHeightMap::ParseObjectData - read a object info chunk.
* Format is the newer CHUNKY format.
*	See WHeightMapEdit.cpp for the writer.
*	Input: DataChunkInput 
*		
*/
Bool ScriptDialog::ParseObjectDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	ScriptDialog *pThis = (ScriptDialog *)userData;
	MapObject *pPrevious = (MapObject *)file.m_currentObject;

	Coord3D loc;
	loc.x = file.readReal();
	loc.y = file.readReal();
	loc.z = file.readReal();
	Real angle = file.readReal();
	Int flags = file.readInt(); 
	AsciiString name = file.readAsciiString();
	Dict d;
	d = file.readDict();
	MapObject *pThisOne;

	// create the map object
	pThisOne = newInstance( MapObject)( loc, name, angle, flags, &d, 
														TheThingFactory->findTemplate( name ) );

	if (pThisOne->getProperties()->getType(TheKey_waypointID) == Dict::DICT_INT) {
		pThisOne->setIsWaypoint();
		pThisOne->setWaypointID(pThisOne->getWaypointID()+pThis->m_waypointBase);
		if (pThis->m_maxWaypoint < pThisOne->getWaypointID()) pThis->m_maxWaypoint = pThisOne->getWaypointID();
	}

	DEBUG_LOG(("Adding object %s (%s)\n", name.str(), pThisOne->getProperties()->getAsciiString(TheKey_originalOwner).str()));
	// Check for duplicates.

	MapObject *pObj;
	Bool duplicate = false;
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		Coord3D curLoc;
		curLoc = *pObj->getLocation();
		Bool locsMatch = (loc.x==curLoc.x&&loc.y==curLoc.y);
		// If the locations match, and they are both waypoints or both not waypoints, and the names match, 
		// They're duplicate.
		if (locsMatch && (pObj->isWaypoint() == pThisOne->isWaypoint()) && (pObj->getName() == pThisOne->getName())) {
			duplicate = true;
		}
		if (pThisOne->isWaypoint() && pObj->isWaypoint() ) {
			// If both waypoints, and the names match, are dupes.
			if (!duplicate && (pThisOne->getWaypointName()==pObj->getWaypointName())) {
				AsciiString warning;
				warning.format("Duplicate named waypoints '%s', renaming imported waypoint.", pThisOne->getWaypointName().str());
				::AfxMessageBox(warning.str(), MB_OK);
 				AsciiString name = WaypointOptions::GenerateUniqueName(pThisOne->getWaypointID());
				name.concat("-imp");
			}
		}	
		if (duplicate) break;
	}
	if (duplicate) {
		pThisOne->deleteInstance();
		return true;
	}

	if (pPrevious) {
		DEBUG_ASSERTCRASH(pThis->m_firstReadObject != NULL && pPrevious->getNext() == NULL, ("Bad linkage."));
		pPrevious->setNextMap(pThisOne);
	}	else {
		DEBUG_ASSERTCRASH(pThis->m_firstReadObject == NULL, ("Bad linkage."));
		pThis->m_firstReadObject = pThisOne;
	}
	file.m_currentObject = pThisOne;
	return true;
}

/**
* ScriptDialog::ParseWaypointData - read waypoint data chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
Bool ScriptDialog::ParseWaypointDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Int count = file.readInt();
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	ScriptDialog *pThis = (ScriptDialog *)userData;
	Int i;
	for (i=0; i<count; i++) {
		Int waypoint1 = file.readInt();
		Int waypoint2 = file.readInt();
		pDoc->addWaypointLink(waypoint1+pThis->m_waypointBase, waypoint2+pThis->m_waypointBase);
		if (pThis->m_maxWaypoint < waypoint1+pThis->m_waypointBase) pThis->m_maxWaypoint = waypoint1+pThis->m_waypointBase;
		if (pThis->m_maxWaypoint < waypoint2+pThis->m_waypointBase) pThis->m_maxWaypoint = waypoint1+pThis->m_waypointBase;
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	return true;
}

/**
* ScriptDialog::ParseTeamsDataChunk - read teams data chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
Bool ScriptDialog::ParseTeamsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	ScriptDialog *pThis = (ScriptDialog *)userData;
	while (!file.atEndOfChunk()) {
		Dict teamDict = file.readDict();
		AsciiString teamName = teamDict.getAsciiString(TheKey_teamName);
		if (pThis->m_sides.findTeamInfo(teamName)) {
			continue;
		}
		DEBUG_LOG(("Adding team %s\n", teamName.str()));
		AsciiString player = teamDict.getAsciiString(TheKey_teamOwner);
		if (pThis->m_sides.findSideInfo(player)) {
			// player exists, so just add it.
			pThis->m_sides.addTeam(&teamDict);
		} else {
			AsciiString warning;
			warning.format("Importing team %s of player %s.  Player %s doesn't exist, Select player..", 
				teamName.str(), player.str(), player.str());

			::AfxMessageBox(warning.str(), MB_OK);	
			TeamsInfo ti;	 
			ti.init(&teamDict);
			CFixTeamOwnerDialog fix(&ti, &pThis->m_sides);
			bool nameSet = false;
			if (fix.DoModal() == IDOK) {
				if (fix.pickedValidTeam()) {
					teamDict.setAsciiString(TheKey_teamOwner, fix.getSelectedOwner());
					nameSet = true;
				}
			}
						
			if (nameSet == false) {
				AsciiString neutralPlayerName; // neutral player name is empty string
				// player doesn't exist, so add it to the neutral player.
				teamDict.setAsciiString(TheKey_teamOwner, neutralPlayerName);
			}
			pThis->m_sides.addTeam(&teamDict);
		}
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	return true;
}

/**
* ScriptDialog::ParsePlayersDataChunk - read players names data chunk.
* Format is the newer CHUNKY format.
*	Input: DataChunkInput 
*		
*/
Bool ScriptDialog::ParsePlayersDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	ScriptDialog *pThis = (ScriptDialog *)userData;
	Int readDicts = 0;
	if (info->version >= K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_2) {
		readDicts = file.readInt();
	}
	Int numNames = file.readInt();
	Int i;
	for (i=0; i<numNames; i++) {
		if (i>=MAX_PLAYER_COUNT) break;
		pThis->m_readPlayerNames[i] = file.readAsciiString();
		if (readDicts) {
			Dict sideDict = file.readDict();
			bool nameFound = false;
			for (Int j=0; j < pThis->m_sides.getNumSides(); j++) {
				AsciiString name = pThis->m_sides.getSideInfo(j)->getDict()->getAsciiString(TheKey_playerName);

				if (name == pThis->m_readPlayerNames[i]) {
					// The side already exists so don't add it or overwrite the old data.
					nameFound = true;
					break;
				}
			}
			if (nameFound == false) {
				// This side doesn't currently exist, so add it.
				pThis->m_sides.addSide(&sideDict);
				ScriptList* pList = newInstance(ScriptList);
				SidesInfo* sides = pThis->m_sides.findSideInfo(pThis->m_readPlayerNames[i]);
				// A script list must be created.
				sides->setScriptList(pList);
				// Update the dialog.
				pThis->addPlayer(i);
			}
		}
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	return true;
}

/**
* ScriptDialog::ParsePolygonTriggersDataChunk - read a polygon triggers chunk.
* Format is the newer CHUNKY format.
*	See PolygonTrigger::WritePolygonTriggersDataChunk for the writer.
*	Input: DataChunkInput 
*		
*/
Bool ScriptDialog::ParsePolygonTriggersDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Int count;
	Int numPoints;
	Int triggerID;
//	Int maxTriggerId = 0;
	AsciiString triggerName;
	// Remove any existing polygon triggers, if any.
	ScriptDialog *pThis = (ScriptDialog *)userData;
	pThis->m_firstTrigger = NULL;
	PolygonTrigger *pPrevTrig = NULL;
	count = file.readInt(); 
	Bool isRiver;
	Int riverStart;
	while (count>0) {
		count--;
		Bool isWater = false;
		triggerName = file.readAsciiString();
		triggerID = file.readInt();
		if (info->version >= K_TRIGGERS_VERSION_2) {
			isWater = file.readByte();
		}
		isRiver = false;
		riverStart = 0;
		if (info->version >= K_TRIGGERS_VERSION_3) {
			isRiver = file.readByte();
			riverStart = file.readInt();
		}
		numPoints = file.readInt(); 
		PolygonTrigger *pTrig = newInstance(PolygonTrigger)(numPoints+1);
		pTrig->setTriggerName(triggerName);
		pTrig->setWaterArea(isWater);
		pTrig->setRiver(isRiver);
		pTrig->setRiverStart(riverStart);
		Int i;
		for (i=0; i<numPoints; i++) {
			ICoord3D loc;
			loc.x = file.readInt();
			loc.y = file.readInt();
			loc.z = file.readInt();
			pTrig->addPoint(loc);
		}
		// Check for duplicates. 
		Bool duplicate = false;
		PolygonTrigger *pCurrentTrigger;
		for (pCurrentTrigger=PolygonTrigger::getFirstPolygonTrigger(); pCurrentTrigger; pCurrentTrigger = pCurrentTrigger->getNext()) {
			if (triggerName == pCurrentTrigger->getTriggerName()) {
				duplicate = true;
				AsciiString warning;
				warning.format("Duplicated trigger named '%s' discarded.", triggerName.str());
				::AfxMessageBox(warning.str(), MB_OK);
				break;
			}
		}
		if (duplicate ) {
			pTrig->deleteInstance();
		} else {
			if (pPrevTrig) {
				pPrevTrig->setNextPoly(pTrig);
			} else {
				pThis->m_firstTrigger = pTrig;
			}
			pPrevTrig = pTrig;
		}
	}
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Incorrect data file length."));
	return true;
}


void ScriptDialog::OnDblclkScriptTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	Script *pScript = getCurScript();
	ScriptGroup *pGroup = getCurGroup();
	if (pScript == NULL && pGroup == NULL) return;
	OnEditScript();
	*pResult = 0;
}

void ScriptDialog::OnOK() 
{
    // Check if the IDC_OBJECT_SEARCH_EDIT control is focused
    if (GetFocus() == GetDlgItem(IDC_SCRIPT_SEARCH))
    {
        OnFindNext();  // Trigger search on "Enter" key press
    }
    else
    {
		CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
		SidesListUndoable *pUndo = new SidesListUndoable(m_sides, pDoc);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.

		SaveScriptWarningsState();
        CDialog::OnOK();  // Call the default OK behavior if the search box isn't focused
    }
}

void ScriptDialog::OnCancel() 
{
	
	CDialog::OnCancel();
}

void ScriptDialog::OnBegindragScriptTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);

	m_curSelection.IntToList(pNMTreeView->itemNew.lParam);
	if (m_curSelection.m_objType != ListType::PLAYER_TYPE) {
		m_dragItem = pNMTreeView->itemNew.hItem;
    pTree->SelectItem(m_dragItem); 
		m_draggingTreeView = true;
 		SetCapture();
	}
	*pResult = 0;
}

void ScriptDialog::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_draggingTreeView) {
		CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
 
    HTREEITEM htiTarget;  // handle to target item 
    TVHITTESTINFO tvht;  // hit test information 

// Adjust the drag point to align with the center of the tree item.
		const Int CENTER_OFFSET = 50;
		point.y -= CENTER_OFFSET;
    tvht.pt = point; 
    if ((htiTarget = pTree->HitTest( &tvht)) != NULL) {
			pTree->SelectDropTarget(htiTarget); 
    } 
  }
	
	CDialog::OnMouseMove(nFlags, point);
}

void ScriptDialog::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_draggingTreeView) {
		CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
		m_draggingTreeView = false;

		ReleaseCapture();
    HTREEITEM htiTarget;  // handle to target item 
    TVHITTESTINFO tvht;  // hit test information 

// Adjust the drag point to align with the center of the tree item.
		const Int CENTER_OFFSET = 50;
		point.y -= CENTER_OFFSET;
    tvht.pt = point; 
    if ((htiTarget = pTree->HitTest( &tvht)) != NULL) { 
      pTree->SelectItem(htiTarget); 
			pTree->SelectDropTarget(htiTarget);
			doDropOn(m_dragItem, htiTarget);
    } 
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void ScriptDialog::doDropOn(HTREEITEM hDrag, HTREEITEM hTarget) 
{
	if (hDrag == hTarget) return;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	ListType drag;
	drag.IntToList(pTree->GetItemData(hDrag));
	ListType target;
	target.IntToList(pTree->GetItemData(hTarget));			

	Script *dragScript = NULL;
	ScriptGroup *dragGroup = NULL;
	m_curSelection = drag;
	Script *pScript = getCurScript();
	ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	ScriptGroup *pGroup = getCurGroup();
	if (pSL == NULL) return;
	if (pScript) {
		dragScript = pScript->duplicate();
		if (pGroup) {
			pGroup->deleteScript(pScript);
		} else {
			pSL->deleteScript(pScript);
		}
		if (drag.m_objType == target.m_objType &&
				drag.m_playerIndex == target.m_playerIndex &&
				drag.m_groupIndex == target.m_groupIndex &&
				drag.m_scriptIndex < target.m_scriptIndex) {
				target.m_scriptIndex--;
		}
	}	else if (drag.m_objType == ListType::GROUP_TYPE) {
		dragGroup = pGroup->duplicate();
		pSL->deleteGroup(pGroup);
		if (drag.m_objType != ListType::SCRIPT_IN_PLAYER_TYPE &&
				drag.m_playerIndex == target.m_playerIndex &&
				drag.m_groupIndex < target.m_groupIndex) {
				target.m_groupIndex--;
		}
	}
	pTree->DeleteItem(hDrag);
	m_curSelection = target;
	pScript = getCurScript();
	pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	pGroup = getCurGroup();
	DEBUG_ASSERTCRASH((pSL), ("Hmm - bad data. jba."));
	if (pSL == NULL) return;

	// If we are dragging a group onto a script, adjust the group index so we add after.
	if (drag.m_objType == ListType::GROUP_TYPE) {
		if (target.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE) {
			target.m_groupIndex = 9999;
		}
		if (target.m_objType == ListType::SCRIPT_IN_GROUP_TYPE) {
			target.m_groupIndex++;
		}
		target.m_objType = ListType::GROUP_TYPE;
	}

	if (dragScript) {
		if (pGroup) { 
				pGroup->addScript(dragScript, target.m_scriptIndex);
		}	else {
			pSL->addScript(dragScript, target.m_scriptIndex);
		}
	} else if (dragGroup) {
		pSL->addGroup(dragGroup, target.m_groupIndex);
		Int count = 0;
		ScriptGroup *pGroup = pSL->getScriptGroup();
		while (pGroup->getNext()) {
			if (pGroup==dragGroup) break;
			count++;
			pGroup = pGroup->getNext();
		}
		target.m_groupIndex = count;
	}
	if (target.m_playerIndex != drag.m_playerIndex) {
		reloadPlayer(drag.m_playerIndex, m_sides.getSideInfo(drag.m_playerIndex)->getScriptList());
	}
	reloadPlayer(target.m_playerIndex, pSL);
	updateSelection(target);
	updateIcons(TVI_ROOT);
}

void ScriptDialog::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(SCRIPT_DIALOG_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(SCRIPT_DIALOG_SECTION, "Left", frameRect.left);
	}
	
}

/** This function reacts to the selection of "active" from
    the right click drop down menu */
void ScriptDialog::OnScriptActivate()
{
	AsciiString newName;
	Bool active;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	HTREEITEM item = findItem(m_curSelection);

	if (getCurScript() != NULL)
	{
		// Toggle active state
		active = getCurScript()->isActive();
		getCurScript()->setActive(!active);

		// Update label
		Script *pScript = getCurScript();
		pTree->SetItemText(item, formatScriptLabel(pScript).str());

		// Set icon using centralized logic
		setIconScript(item);
	}
	else if (getCurGroup() != NULL)
	{
		// Toggle active state
		active = getCurGroup()->isActive();
		getCurGroup()->setActive(!active);

		// Update label
		ScriptGroup *pScriptGroup = getCurGroup();
		pTree->SetItemText(item, formatScriptLabel(pScriptGroup).str());

		// Set icon using centralized logic
		setIconGroup(item);
	}
}
