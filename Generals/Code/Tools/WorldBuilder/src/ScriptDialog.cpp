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

static const Int K_LOCAL_TEAMS_VERSION_1 = 1;

#define SCRIPT_DIALOG_SECTION "ScriptDialog"

static const char* NEUTRAL_NAME_STR = "(neutral)";
ScriptDialog *ScriptDialog::m_staticThis = nullptr;

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
		if (sd->friend_getCurScript() != nullptr)
		{
			Bool active = sd->friend_getCurScript()->isActive();
			pPopup->CheckMenuItem(ID_SCRIPTACTIVATE, MF_BYCOMMAND | (active ? MF_CHECKED : MF_UNCHECKED));
		}
		else if (sd->friend_getCurGroup() != nullptr)
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


ScriptDialog::ScriptDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(ScriptDialog::IDD, pParent)
{
	m_draggingTreeView = false;
	//{{AFX_DATA_INIT(ScriptDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

ScriptDialog::~ScriptDialog()
{
	EditParameter::setCurSidesList(nullptr);
	m_staticThis=nullptr;
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

void ScriptDialog::OnSelchangedScriptTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	if (pNMTreeView->itemNew.hItem==nullptr) {
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
	pWnd->EnableWindow(pScript!=nullptr || pGroup!=nullptr);

	pWnd = GetDlgItem(IDC_COPY_SCRIPT);
	pWnd->EnableWindow(pScript!=nullptr);

	pWnd = GetDlgItem(IDC_DELETE);
	pWnd->EnableWindow(m_curSelection.m_objType != ListType::PLAYER_TYPE);

	AsciiString scriptComment;
	AsciiString scriptText;
	if (pScript) {
		scriptComment = pScript->getComment();
		scriptText = pScript->getUiText();
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
			Script *pScr=nullptr;
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
	return nullptr;
}

ScriptGroup *ScriptDialog::getCurGroup(void)
{
	ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	if (m_curSelection.m_objType == ListType::PLAYER_TYPE) {
		return nullptr;
	}
	if (m_curSelection.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE) {
		return nullptr;
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
	return nullptr;
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
				warning = EditParameter::getWarningText(pCondition->getParameter(i));
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
			warning = EditParameter::getWarningText(pAction->getParameter(i));
			if (!warning.isEmpty()) {
				pScript->setWarnings(true);
				pAction->setWarnings(true);
			}
		}
	}
}

/** Updates the warning flags in the scripts, script groups & script conditions & actions. */
void ScriptDialog::updateWarnings()
{
	SidesList *sidesListP = TheSidesList;
	Int i;
	if (m_staticThis) sidesListP = &m_staticThis->m_sides;
	for (i=0; i<sidesListP->getNumSides(); i++) {
		ScriptList *pSL = sidesListP->getSideInfo(i)->getScriptList();
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			updateScriptWarning(pScr);
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			pGroup->setWarnings(false);
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				updateScriptWarning(pScr);
				if (pScr->hasWarnings()) {
					pGroup->setWarnings(true);
				}
			}
		}
	}
}

BOOL ScriptDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_staticThis = this;
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
	updateWarnings();
	if (pTree) {
		m_imageList.Create(IDB_FOLDERSCRIPT, 16, 2, ILC_COLOR4);
		pTree->SetImageList(&m_imageList, TVSIL_STATE);
		for (i=0; i<m_sides.getNumSides(); i++) {
			HTREEITEM hItem = addPlayer(i);
			if (!didSelect && hItem != nullptr) {
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
	SetWindowPos(nullptr, top.left, top.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);

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

	if (getCurGroup()->isActive())
		pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);

	if (!getCurGroup()->isActive())
		pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(5), TVIS_STATEIMAGEMASK);

	if (getCurGroup()->hasWarnings())
		pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);

	return;
}

void ScriptDialog::setIconScript(HTREEITEM item)
{
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);

	if (getCurScript()->isActive())
		pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(2), TVIS_STATEIMAGEMASK);

	if (!getCurScript()->isActive())
		pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(6), TVIS_STATEIMAGEMASK);

	if (getCurScript()->hasWarnings())
		pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(4), TVIS_STATEIMAGEMASK);

	return;
}

Bool ScriptDialog::updateIcons(HTREEITEM hItem)
{
	const ListType saveList = m_curSelection;
	Bool warnings = false;
	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	HTREEITEM child = pTree->GetChildItem(hItem);

	while (child != nullptr) {
		ListType lt;
		lt.IntToList(pTree->GetItemData(child));

		/// player type
		if (lt.m_objType == ListType::PLAYER_TYPE)
		{
			if (updateIcons(child)) {
				pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);
			} else {
					pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
			}
		}

		/// script group
		else if (lt.m_objType == ListType::GROUP_TYPE)
		{
			m_curSelection = lt;
			if (updateIcons(child)) {
				pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);
				warnings = true;
			} else {
					setIconGroup(child);
			}
		}

		/// script
		else
		{
			m_curSelection = lt;
			Script *pScr = getCurScript();
			DEBUG_ASSERTCRASH(pScr, ("Unexpected."));
			if (pScr) {
				if (pScr->hasWarnings()) {
					pTree->SetItemState(child, INDEXTOSTATEIMAGEMASK(4), TVIS_STATEIMAGEMASK);
					warnings = true;
				} else {
						setIconScript(child);
				}
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
//	Dict *d = m_sides.getSideInfo(playerIndex)->getDict();
	updateWarnings();

	CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
	HTREEITEM player = pTree->GetChildItem(TVI_ROOT);
	while (player != nullptr) {
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
	if (!player) return;
	HTREEITEM child;
	ListType currentSel = m_curSelection;

	if (currentSel.m_objType == ListType::SCRIPT_IN_GROUP_TYPE) {
		if (currentSel.m_scriptIndex > 0) {
			--currentSel.m_scriptIndex;
		}
	}

	do {
		child = pTree->GetChildItem(player);
		if (child) pTree->DeleteItem(child);
	} while (child);
	m_curSelection = currentSel;
	addScriptList(player, playerIndex, pSL);
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
	while (player != nullptr) {
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
	if (!player) return nullptr;
	if (sel.m_objType == ListType::PLAYER_TYPE) {
		return player;
	}

	HTREEITEM group;
	if (sel.m_objType == ListType::SCRIPT_IN_PLAYER_TYPE) {
		group = player; // top level scripts are grouped under player.
	} else {
		group = pTree->GetChildItem(player);
		while (group != nullptr) {
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
	if (!group) return nullptr;
	if (sel.m_objType == ListType::GROUP_TYPE) {
		return group;
	}

	HTREEITEM script;
	for (script = pTree->GetChildItem(group); script != nullptr; script = pTree->GetNextSiblingItem(script)) {
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
			deleteInstance(pNewGroup);
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
		deleteInstance(pNewScript);
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
	if (pScript == nullptr) {
		CTreeCtrl *pTree = (CTreeCtrl*)GetDlgItem(IDC_SCRIPT_TREE);
		HTREEITEM item = findItem(m_curSelection);
		if (pGroup) {
			EditGroup editDlg(pGroup);
			if (IDOK==editDlg.DoModal()) {
				if (item) {
					pTree->SetItemText(item, pGroup->getName().str());
					pTree->SelectItem(nullptr);
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
			pTree->SelectItem(nullptr);
			updateWarnings();
			pTree->SelectItem(item); // Updates the comment field & text field.
		}
	}
	updateIcons(TVI_ROOT);
	deleteInstance(pDup);
}

void ScriptDialog::OnCopyScript()
{
	Script *pScript = getCurScript();
	DEBUG_ASSERTCRASH(pScript, ("Null script."));
	if (pScript == nullptr) return;
	Script *pDup = pScript->duplicate();
	AsciiString newName = pDup->getName();
	newName.concat(" C");
	pDup->setName(newName);
	insertScript(pDup);
	updateIcons(TVI_ROOT);
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
						if (pGroup->getScript()==nullptr) {
							m_curSelection.m_objType = ListType::GROUP_TYPE;
						}
					}
					break;
				}
			}
		} else {
			pSL->deleteScript(pScript);
			if (pSL->getScript()==nullptr) {
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
void ScriptDialog::scanParmForWaypointsAndTeams(Parameter *pParm, Bool doUnits, Bool doWaypoints, Bool doTriggers)
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
		if (pInfo) {
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
void ScriptDialog::scanForWaypointsAndTeams(Script *pScript, Bool doUnits, Bool doWaypoints, Bool doTriggers)
{
	pScript->setWarnings(false);
	OrCondition *pOr;
	for (pOr= pScript->getOrCondition(); pOr; pOr = pOr->getNextOrCondition()) {
		Condition *pCondition;
		for (pCondition = pOr->getFirstAndCondition(); pCondition; pCondition = pCondition->getNext()) {
			Int i;
			for (i=0; i<pCondition->getNumParameters(); i++) {
				scanParmForWaypointsAndTeams(pCondition->getParameter(i), doUnits, doWaypoints, doTriggers);
			}
		}
	}
	ScriptAction *pAction;
	for (pAction = pScript->getAction(); pAction; pAction = pAction->getNext()) {
		pAction->setWarnings(false);
		Int i;
		for (i=0; i<pAction->getNumParameters(); i++) {
			scanParmForWaypointsAndTeams(pAction->getParameter(i), doUnits, doWaypoints, doTriggers);
		}
	}
}

#define K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_1 1

/** Write out selected scripts, and possibly waypoints, trigger areas & teams. */
void ScriptDialog::OnSave()
{
	Bool doWaypoints = true;
	Bool doTriggerAreas = true;
	Bool doUnits = true;
	Bool doAllScripts = true;
	Int	 i;

	ExportScriptsOptions optionsDlg;
	if (IDCANCEL == optionsDlg.DoModal()) {
		return;
	}
	doWaypoints = optionsDlg.getDoWaypoints();
	doUnits = optionsDlg.getDoUnits();
	doTriggerAreas = optionsDlg.getDoTriggers();
	doAllScripts = optionsDlg.getDoAllScripts();

	Script *pScript = getCurScript();
	ScriptGroup *pGroup = getCurGroup();

	ScriptList *scripts[MAX_PLAYER_COUNT];
	for (i=0; i<MAX_PLAYER_COUNT; i++) {
		scripts[i] = nullptr;
	}

	CFileDialog fileDlg(false, ".scb", nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Script files (.scb)|*.scb||", this);

	Int result = fileDlg.DoModal();

	// Open document dialog may change working directory,
	// change it back.
	char buf[_MAX_PATH];
	::GetModuleFileName(nullptr, buf, sizeof(buf));
	if (char *pEnd = strrchr(buf, '\\')) {
		*pEnd = 0;
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
		if (scripts[0] == nullptr) {
			::AfxMessageBox("No scripts selected - aborting export.", MB_OK);
			return;
		}
	}
	for (i=0; i<numScriptLists; i++) {
		ScriptList *pSL = scripts[i];
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			scanForWaypointsAndTeams(pScr, doUnits, doWaypoints, doTriggerAreas);
		}
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				scanForWaypointsAndTeams(pScr, doUnits, doWaypoints, doTriggerAreas);
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
		chunkWriter.openDataChunk("ScriptsPlayers", 	K_PLAYERS_NAMES_FOR_SCRIPTS_VERSION_1);
		if (doAllScripts) {
			chunkWriter.writeInt(m_sides.getNumSides());
			for (i=0; i<m_sides.getNumSides(); i++) {
				AsciiString name = m_sides.getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName);
				chunkWriter.writeAsciiString(name);
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
		deleteInstance(scripts[0]);
	}
	theFile.Close();
}

void ScriptDialog::OnLoad()
{
	CFileDialog fileDlg(true, ".scb", nullptr, 0,
		"Script files (.scb)|*.scb||", this);

	Int result = fileDlg.DoModal();

	// Open document dialog may change working directory,
	// change it back.
	char buf[_MAX_PATH];
	::GetModuleFileName(nullptr, buf, sizeof(buf));
	if (char *pEnd = strrchr(buf, '\\')) {
		*pEnd = 0;
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
		m_firstReadObject = nullptr;
		m_firstTrigger = nullptr;
		m_waypointBase = pDoc->getNextWaypointID();
		m_maxWaypoint = m_waypointBase;
		file.registerParser( "PlayerScriptsList", AsciiString::TheEmptyString, ScriptList::ParseScriptsDataChunk );
		file.registerParser( "ObjectsList", AsciiString::TheEmptyString, ParseObjectsDataChunk );
		file.registerParser( "PolygonTriggers", AsciiString::TheEmptyString, ParsePolygonTriggersDataChunk );
		file.registerParser( "WaypointsList", AsciiString::TheEmptyString, ParseWaypointDataChunk );
		file.registerParser( "ScriptTeams", AsciiString::TheEmptyString, ParseTeamsDataChunk );
		file.registerParser( "ScriptsPlayers", AsciiString::TheEmptyString, ParsePlayersDataChunk );
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
			m_firstReadObject = nullptr; // undoable owns it now.
		}
		PolygonTrigger *pTrig;
		PolygonTrigger *pNextTrig;
		for (pTrig=m_firstTrigger; pTrig; pTrig = pNextTrig) {
			pNextTrig = pTrig->getNext();
			pTrig->setNextPoly(nullptr);
			PolygonTrigger::addPolygonTrigger(pTrig);
		}

		ScriptList *scripts[MAX_PLAYER_COUNT];
		Int count = ScriptList::getReadScripts(scripts);
		Int i;
		for (i=0; i<count; i++) {
			if (scripts[i]->getScript() == nullptr && scripts[i]->getScriptGroup()==nullptr) continue;
			Int curSide = -1;
			if (count==1) {
				curSide = m_curSelection.m_playerIndex;
			} else {
				Int j;
				for (j=0; j<m_sides.getNumSides(); j++) {
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

			Script *pScr;
			Script *pNextScr;
			Int j=0;
			for (pScr = scripts[i]->getScript(); pScr; pScr=pNextScr) {
				pNextScr=pScr->getNext();
				pScr->setNextScript(nullptr);
				pSL->addScript(pScr, j); //unlink it and add.
				j++;
			}
			j=0;
			ScriptGroup *pGroup;
			ScriptGroup *pNextGroup;
			for (pGroup = scripts[i]->getScriptGroup(); pGroup; pGroup=pNextGroup) {
				pNextGroup=pGroup->getNext();
				pGroup->setNextGroup(nullptr);
				pSL->addGroup(pGroup, j);
				j++;
			}
			scripts[i]->discard(); /* Frees the script list, but none of it's children, as they have been
														copied into the current scripts. */
			scripts[i] = nullptr;
			if (pSL) {
				reloadPlayer(curSide, pSL);
			}
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
	file.m_currentObject = nullptr;
	file.registerParser( "Object", info->label, ParseObjectDataChunk );
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

	DEBUG_LOG(("Adding object %s (%s)", name.str(), pThisOne->getProperties()->getAsciiString(TheKey_originalOwner).str()));
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
		deleteInstance(pThisOne);
		return true;
	}

	if (pPrevious) {
		DEBUG_ASSERTCRASH(pThis->m_firstReadObject != nullptr && pPrevious->getNext() == nullptr, ("Bad linkage."));
		pPrevious->setNextMap(pThisOne);
	}	else {
		DEBUG_ASSERTCRASH(pThis->m_firstReadObject == nullptr, ("Bad linkage."));
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
		DEBUG_LOG(("Adding team %s", teamName.str()));
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
			if (fix.DoModal() == IDOK) {
				if (fix.pickedValidTeam()) {
					teamDict.setAsciiString(TheKey_teamOwner, fix.getSelectedOwner());
				}
			}

			AsciiString neutralPlayerName; // neutral player name is empty string
			// player doesn't exist, so add it to the neutral player.
			teamDict.setAsciiString(TheKey_teamOwner, neutralPlayerName);
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
	Int numNames = file.readInt();
	Int i;
	for (i=0; i<numNames; i++) {
		if (i>=MAX_PLAYER_COUNT) break;
		pThis->m_readPlayerNames[i] = file.readAsciiString();
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
	pThis->m_firstTrigger = nullptr;
	PolygonTrigger *pPrevTrig = nullptr;
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
			deleteInstance(pTrig);
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
	if (pScript == nullptr && pGroup == nullptr) return;
	OnEditScript();
	*pResult = 0;
}

void ScriptDialog::OnOK()
{
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	SidesListUndoable *pUndo = new SidesListUndoable(m_sides, pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	CDialog::OnOK();
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

		const Int CENTER_OFFSET = 12;
		point.y -= CENTER_OFFSET;
    tvht.pt = point;
    if ((htiTarget = pTree->HitTest( &tvht)) != nullptr) {
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

		const Int CENTER_OFFSET = 12;
		point.y -= CENTER_OFFSET;
    tvht.pt = point;
    if ((htiTarget = pTree->HitTest( &tvht)) != nullptr) {
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

	Script *dragScript = nullptr;
	ScriptGroup *dragGroup = nullptr;
	m_curSelection = drag;
	Script *pScript = getCurScript();
	ScriptList *pSL = m_sides.getSideInfo(m_curSelection.m_playerIndex)->getScriptList();
	ScriptGroup *pGroup = getCurGroup();
	if (pSL == nullptr) return;
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
	if (pSL == nullptr) return;

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

	if (getCurScript() != nullptr)
	{
		/// Updates attributes
		active = getCurScript()->isActive();
		getCurScript()->setActive(!active);

		/// Updates screen to reflect change
		Script *pScript = getCurScript();
		pTree->SetItemText(item, formatScriptLabel(pScript).str());

		if (getCurScript()->hasWarnings())
		{
			pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(4), TVIS_STATEIMAGEMASK);
		}
		else
		{
			if (getCurScript()->isActive())
				pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(2), TVIS_STATEIMAGEMASK);
			else
				pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(6), TVIS_STATEIMAGEMASK);
		}
	}
	else if (getCurGroup() != nullptr)
	{
		/// Updates attributes
		active = getCurGroup()->isActive();
		getCurGroup()->setActive(!active);

		/// Updates screen to reflect change
		ScriptGroup *pScriptGroup = getCurGroup();
		pTree->SetItemText(item, formatScriptLabel(pScriptGroup).str());

		if (getCurGroup()->hasWarnings())
		{
			pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(3), TVIS_STATEIMAGEMASK);
		}
		else
		{
			if (getCurGroup()->isActive())
				pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
			else
				pTree->SetItemState(item, INDEXTOSTATEIMAGEMASK(5), TVIS_STATEIMAGEMASK);
		}
	}
}
