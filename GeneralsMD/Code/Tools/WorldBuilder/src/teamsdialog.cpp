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

// teamsdialog.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "teamsdialog.h"
#include "CFixTeamOwnerDialog.h"

#include "Common/WellKnownKeys.h"
#include "GameLogic/SidesList.h"
#include "TeamBehavior.h"
#include "TeamGeneric.h"
#include "TeamIdentity.h"
#include "TeamReinforcement.h"
#include "TeamObjectProperties.h"
#include "WorldBuilderDoc.h"
#include "CUndoable.h"
#include "wbview3d.h"

static Int thePrevCurTeam = 0;

static const char* NEUTRAL_NAME_STR = "(neutral)";

/////////////////////////////////////////////////////////////////////////////
// CTeamsDialog dialog


CTeamsDialog::CTeamsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTeamsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTeamsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTeamsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTeamsDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTeamsDialog, CDialog)
	//{{AFX_MSG_MAP(CTeamsDialog)
	ON_BN_CLICKED(IDC_NEWTEAM, OnNewteam)
	ON_BN_CLICKED(IDC_DELETETEAM, OnDeleteteam)
	ON_LBN_SELCHANGE(IDC_PLAYER_LIST, OnSelchangePlayerList)
	ON_NOTIFY(NM_CLICK, IDC_TEAMS_LIST, OnClickTeamsList)
	ON_NOTIFY(NM_DBLCLK, IDC_TEAMS_LIST, OnDblclkTeamsList)
	ON_BN_CLICKED(IDC_COPYTEAM, OnCopyteam)
	ON_BN_CLICKED(IDC_SelectTeamMembers, OnSelectTeamMembers)
	ON_BN_CLICKED(IDC_MOVEDOWNTEAM, OnMoveDownTeam)
	ON_BN_CLICKED(IDC_MOVEUPTEAM, OnMoveUpTeam)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTeamsDialog message handlers

static Bool isPlayerDefaultTeamIndex(SidesList& sides, Int i)
{
	return sides.isPlayerDefaultTeam(sides.getTeamInfo(i));
}

static AsciiString playerNameForUI(SidesList& sides, int i)
{
	AsciiString b = sides.getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName);
	if (b.isEmpty())
		b = NEUTRAL_NAME_STR;
	return b;
}

static AsciiString teamNameForUI(SidesList& sides, int i)
{
	TeamsInfo *ti = sides.getTeamInfo(i);
	if (sides.isPlayerDefaultTeam(ti))
	{
		AsciiString ostr = ti->getDict()->getAsciiString(TheKey_teamOwner);
		if (ostr.isEmpty())
			ostr = NEUTRAL_NAME_STR;
		AsciiString n;
		n.format("(default team)");
		return n;
	}

	return ti->getDict()->getAsciiString(TheKey_teamName);
}

static AsciiString UIToInternal(SidesList& sides, const AsciiString& n)
{
	Int i;
	for (i = 0; i < sides.getNumSides(); i++)
	{
		if (playerNameForUI(sides, i) == n)
			return sides.getSideInfo(i)->getDict()->getAsciiString(TheKey_playerName);
	}

	DEBUG_CRASH(("ui name not found"));
	return AsciiString::TheEmptyString;
}

static Int findTeamParentIndex(SidesList& sides, Int i)
{
	AsciiString oname = sides.getTeamInfo(i)->getDict()->getAsciiString(TheKey_teamOwner);

	for (int j = 0; j < sides.getNumSides(); j++)
	{
		if (oname == sides.getSideInfo(j)->getDict()->getAsciiString(TheKey_playerName))
		{
			return -(j+1);
		}
	}
	DEBUG_CRASH(("hmm"));
	return 0;
}

void CTeamsDialog::updateUI(Int whatToRebuild)
{
	if (m_updating)
		return;

	++m_updating;

	// make sure everything is canonical.
	Bool modified = m_sides.validateSides();
	DEBUG_ASSERTLOG(!modified,("had to clean up sides in CTeamsDialog::updateUI! (caller should do this)"));
	if (modified)
	{
		whatToRebuild = REBUILD_ALL;	// assume the worst.
	}

	// constrain team index.
	if (m_curTeam < 0) m_curTeam = 0;
	if (m_curTeam >= m_sides.getNumTeams())
		m_curTeam = m_sides.getNumTeams()-1;

	if (whatToRebuild & REBUILD_TEAMS)
	{
		UpdateTeamsList();
	}

	Bool isDefault = true;

	if (m_curTeam >= 0) {
		isDefault = isPlayerDefaultTeamIndex(m_sides, m_curTeam);
	}

	// update delete button
	CButton *del = (CButton*)GetDlgItem(IDC_DELETETEAM);
	del->EnableWindow(!isDefault);	// toplevel team names are delete-able, but "default" teams are not
	CButton *copyteam = (CButton*)GetDlgItem(IDC_COPYTEAM);
	copyteam->EnableWindow(!isDefault);	// toplevel team names are delete-able, but "default" teams are not

	//update move up and move down buttons
	CButton *moveup = (CButton*)GetDlgItem(IDC_MOVEUPTEAM);
	moveup->EnableWindow(!isDefault);
	CButton *movedown = (CButton*)GetDlgItem(IDC_MOVEDOWNTEAM);
	movedown->EnableWindow(!isDefault);

	CListBox *players = (CListBox*)GetDlgItem(IDC_PLAYER_LIST);
	Int whichPlayer = players->GetCurSel();

	CButton *newteam = (CButton*)GetDlgItem(IDC_NEWTEAM);
	newteam->EnableWindow(whichPlayer>0);	// toplevel team names are delete-able, but "default" teams are not


	--m_updating;
}

BOOL CTeamsDialog::OnInitDialog()
{
	CRect rect;

	CDialog::OnInitDialog();

	// default values for our vars
	m_updating = 0;
	m_sides = *TheSidesList;
	m_curTeam = thePrevCurTeam;

	CListCtrl *pList = (CListCtrl *)GetDlgItem(IDC_TEAMS_LIST);
	pList->InsertColumn(0, "Team Name", LVCFMT_LEFT, 150, 0);
	pList->InsertColumn(1, "Origin", LVCFMT_LEFT, 150, 1);
	pList->InsertColumn(2, "Priority", LVCFMT_LEFT, 50, 2);
	pList->InsertColumn(3, "Script", LVCFMT_LEFT, 150, 3);
	pList->InsertColumn(4, "Trigger", LVCFMT_LEFT, 150, 4);


	CListBox *players = (CListBox*)GetDlgItem(IDC_PLAYER_LIST);
	players->ResetContent();
	for (int i = 0; i < m_sides.getNumSides(); i++)
	{
		players->AddString(playerNameForUI(m_sides, i).str());
	}

	validateTeamOwners();

	updateUI(REBUILD_ALL);

	return TRUE;
}

void CTeamsDialog::OnOK()
{
	Bool modified = m_sides.validateSides();
	(void)modified;
	DEBUG_ASSERTLOG(!modified,("had to clean up sides in CTeamsDialog::OnOK"));

	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	SidesListUndoable *pUndo = new SidesListUndoable(m_sides, pDoc);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.

	thePrevCurTeam = m_curTeam;

	CDialog::OnOK();
}

void CTeamsDialog::OnCancel()
{
	CDialog::OnCancel();
}

void CTeamsDialog::OnNewteam()
{
	Int num = 1;
	AsciiString tname;
	do
	{
		tname.format("team%04d",num++);
	}
	while (m_sides.findTeamInfo(tname));

	AsciiString oname = m_sides.getTeamInfo(m_curTeam)->getDict()->getAsciiString(TheKey_teamOwner);

	Dict d;
	d.setAsciiString(TheKey_teamName, tname);
	d.setAsciiString(TheKey_teamOwner, oname);	// owned by the parent of whatever is selected.
	d.setBool(TheKey_teamIsSingleton, false);

	m_sides.addTeam(&d);
	Int i;
	if (m_sides.findTeamInfo(tname, &i)) {
		m_curTeam = i;
		OnEditTemplate();
	}
	updateUI(REBUILD_ALL);
}

void CTeamsDialog::OnDeleteteam()
{
	if (m_curTeam < 0)
		return;

	Bool isDefault = isPlayerDefaultTeamIndex(m_sides, m_curTeam);
	if (isDefault)
	{
		DEBUG_CRASH(("should not be allowed"));
		return;
	}

	AsciiString tname = m_sides.getTeamInfo(m_curTeam)->getDict()->getAsciiString(TheKey_teamName);
	Int count = MapObject::countMapObjectsWithOwner(tname);
	if (count > 0)
	{
		CString msg;
		msg.Format(IDS_REMOVING_INUSE_TEAM, count);
		if (::AfxMessageBox(msg, MB_YESNO) == IDNO)
			return;
	}

	m_sides.removeTeam(m_curTeam);
	updateUI(REBUILD_ALL);
}

void CTeamsDialog::OnEditTemplate()
{
	CPropertySheet editDialog;
	editDialog.Construct("Edit Team Template.");
	TeamIdentity identity;
	identity.setTeamDict(m_sides.getTeamInfo(m_curTeam)->getDict());
	identity.setSidesList(&m_sides);
	TeamReinforcement reinforcements;
	reinforcements.setTeamDict(m_sides.getTeamInfo(m_curTeam)->getDict());
	TeamBehavior behavior;
	behavior.setTeamDict(m_sides.getTeamInfo(m_curTeam)->getDict());

	TeamGeneric generic;
	generic.setTeamDict(m_sides.getTeamInfo(m_curTeam)->getDict());

	TeamObjectProperties object(m_sides.getTeamInfo(m_curTeam)->getDict());

	editDialog.AddPage(&identity);
	editDialog.AddPage(&reinforcements);
	editDialog.AddPage(&behavior);
	editDialog.AddPage(&generic);
	editDialog.AddPage(&object);

	if (IDOK == editDialog.DoModal()) {
	}
	updateUI(REBUILD_ALL);
}

void CTeamsDialog::UpdateTeamsList()
{
	CListCtrl *pList = (CListCtrl *)GetDlgItem(IDC_TEAMS_LIST);
	pList->DeleteAllItems();
	CListBox *players = (CListBox*)GetDlgItem(IDC_PLAYER_LIST);

	Int which = players->GetCurSel();
	if (which < 0)
		return;

	Int numTeams = m_sides.getNumTeams();
	Bool selected = false;
	Int inserted = 0;

	for (Int i=0; i<numTeams; i++)
	{
		TeamsInfo *ti = m_sides.getTeamInfo(i);
		if (ti->getDict()->getAsciiString(TheKey_teamOwner) == playerNameForUI(m_sides, which).str())
		{
			Bool exists;
			AsciiString teamName = teamNameForUI(m_sides, i);
			AsciiString waypoint = ti->getDict()->getAsciiString(TheKey_teamHome, &exists);
			AsciiString script = ti->getDict()->getAsciiString(TheKey_teamOnCreateScript, &exists);
			CString pri;
			pri.Format(TEXT("%d"), ti->getDict()->getInt(TheKey_teamProductionPriority, &exists));
			AsciiString trigger = ti->getDict()->getAsciiString(TheKey_teamProductionCondition, &exists);

			pList->InsertItem(LVIF_TEXT, inserted, teamName.str(), 0, 0, 0, 0);
			pList->SetItemText(inserted, 1, waypoint.str());
			pList->SetItemText(inserted, 2, pri);
			pList->SetItemText(inserted, 3, script.str());
			pList->SetItemText(inserted, 4, trigger.str());

			pList->SetItemData(inserted, i);
			if (m_curTeam == i) {
				selected = true;
				pList->SetItemState(inserted, LVIS_SELECTED, LVIS_SELECTED);
				pList->EnsureVisible(inserted, false);
			}
			inserted++;
		}
	}
	if (!selected) {
		m_curTeam = -1;
		if (inserted > 0) {
			m_curTeam = pList->GetItemData(0);
			pList->SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
			pList->EnsureVisible(0, false);
		}
	}
}

void CTeamsDialog::OnSelchangePlayerList()
{
	updateUI(REBUILD_ALL);
}

void CTeamsDialog::OnClickTeamsList(NMHDR* pNMHDR, LRESULT* pResult)
{
	CListCtrl* pList = (CListCtrl*) GetDlgItem(IDC_TEAMS_LIST);

	int		nItem = pList->GetNextItem(-1, LVNI_SELECTED);
	if (nItem >= 0)
	{
		m_curTeam = pList->GetItemData(nItem);
		pList->SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	updateUI(REBUILD_ALL);

	*pResult = 0;
}

void CTeamsDialog::OnDblclkTeamsList(NMHDR* pNMHDR, LRESULT* pResult)
{
	CListCtrl* pList = (CListCtrl*) GetDlgItem(IDC_TEAMS_LIST);

	int		nItem = pList->GetNextItem(-1, LVNI_SELECTED);
	if (nItem >= 0)
	{
		m_curTeam = pList->GetItemData(nItem);
	}
	if (m_curTeam >= 0 && !isPlayerDefaultTeamIndex(m_sides, m_curTeam)) {
		OnEditTemplate();
	}

	*pResult = 0;
}

void CTeamsDialog::OnCopyteam()
{
	Dict d = *m_sides.getTeamInfo(m_curTeam)->getDict();
	AsciiString origName = d.getAsciiString(TheKey_teamName);

	Int num = 1;
	AsciiString tname;
	do
	{
		tname.format("%s.%2d",origName.str(), num++);
	}
	while (m_sides.findTeamInfo(tname));

	d.setAsciiString(TheKey_teamName, tname);
	m_sides.addTeam(&d);

	updateUI(REBUILD_ALL);
}

void CTeamsDialog::OnSelectTeamMembers()
{
	Int count = 0;
	Dict d = *m_sides.getTeamInfo(m_curTeam)->getDict();
	AsciiString teamName = d.getAsciiString(TheKey_teamName);
	Coord3D pos;
	MapObject *pObj;
	for (pObj=MapObject::getFirstMapObject(); pObj; pObj=pObj->getNext()) {
		pObj->setSelected(false);
		AsciiString objectsTeam = pObj->getProperties()->getAsciiString(TheKey_originalOwner);
		if (teamName==objectsTeam) {
			pObj->setSelected(true);
			pos = *pObj->getLocation();
			count++;
		}
	}
	CString info;
	info.Format(IDS_NUM_SELECTED_ON_TEAM, teamName.str(), count);
	if (count>0) {
		CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
		if (pDoc) {
			WbView3d *p3View = pDoc->GetActive3DView();
			if (p3View) {
				p3View->setCenterInView(pos.x/MAP_XY_FACTOR, pos.y/MAP_XY_FACTOR);
			}
		}
	}
	::AfxMessageBox(info, MB_OK);
}

/** This function moves a team up the list in the teams list dialog */
void CTeamsDialog::OnMoveUpTeam()
{
	// don't move up if already at top of list
	if (m_curTeam <= 1)
		return;

	Dict temp, current;
	int startRemove;
	int totalTeams = m_sides.getNumTeams();

	// iterates through all modified entries in the teams list
	for (int i=m_curTeam-1; i<totalTeams; i++)
	{

		/* saves the one right above the selected team, then deletes it
		 from the list */
		if (i == (m_curTeam-1)) {
			temp = *m_sides.getTeamInfo(i)->getDict();
			m_sides.removeTeam(i);
			startRemove = i;
		}

		/* saves the selected item, deletes from the list, then adds it
		 to the bottom of the list -- then adds the saved "temp" item
		 to the bottom of the list */
		else if (i == m_curTeam) {
			current = *m_sides.getTeamInfo(startRemove)->getDict();
			m_sides.removeTeam(startRemove);
			m_sides.addTeam(&current);
			m_sides.addTeam(&temp);
		}

		/* saves each following item, deletes from the list, and then
		 adds it to the bottom of the list */
		else if (i > m_curTeam) {
			current = *m_sides.getTeamInfo(startRemove)->getDict();
			m_sides.removeTeam(startRemove);
			m_sides.addTeam(&current);
		}
	}

	// modify cursor
	m_curTeam--;

	// rebuild user interface to reflect changes
/*
	LVITEM *pItem = NULL;
	CListCtrl* pList = (CListCtrl*) GetDlgItem(IDC_TEAMS_LIST);
	Bool result = pList->GetItem(pItem);
	pList->DeleteItem(m_curTeam);
	pList->InsertItem(pItem);
	for (i=0; i<m_sides.getNumTeams(); i++)
		pList->Update(i);
	pList->SetItemState(m_curTeam, LVIS_SELECTED, LVIS_SELECTED);
*/
	updateUI(REBUILD_ALL);
}

/// This function moves a team down the list in the teams list dialog
void CTeamsDialog::OnMoveDownTeam()
{
	// don't move down if already at bottom of list
	if (m_curTeam >= m_sides.getNumTeams()-1)
		return;

	Dict temp, current;
	int startRemove;
	int totalTeams = m_sides.getNumTeams();

	// iterates through all modified entries in the teams list
	for (int i=m_curTeam; i<totalTeams; i++)
	{

		/* saves the  selected team, then deletes it
		 from the list */
		if (i == m_curTeam) {
			temp = *m_sides.getTeamInfo(i)->getDict();
			m_sides.removeTeam(i);
			startRemove = i;
		}

		/* saves the one right after the selected item, deletes it from the list,
		 then adds it to the bottom of the list -- then adds the saved "temp" item
		 to the bottom of the list */
		else if (i == (m_curTeam+1)) {
			current = *m_sides.getTeamInfo(startRemove)->getDict();
			m_sides.removeTeam(startRemove);
			m_sides.addTeam(&current);
			m_sides.addTeam(&temp);
		}

		/* saves each following item, deletes from the list, and then
		 adds it to the bottom of the list */
		else if (i > m_curTeam+1) {
			current = *m_sides.getTeamInfo(startRemove)->getDict();
			m_sides.removeTeam(startRemove);
			m_sides.addTeam(&current);
		}
	}

	// modify cursor
	m_curTeam++;

	// rebuild user interface to reflect changes
/*	LVITEM *pItem = NULL;
	CListCtrl* pList = (CListCtrl*) GetDlgItem(IDC_TEAMS_LIST);
	Bool result = pList->GetItem(pItem);
	pList->DeleteItem(m_curTeam);
	pList->InsertItem(pItem);
	for (i=0; i<m_sides.getNumTeams(); i++)
		pList->Update(i);
	pList->SetItemState(m_curTeam, LVIS_SELECTED, LVIS_SELECTED);
*/
	updateUI(REBUILD_ALL);
}

void CTeamsDialog::validateTeamOwners( void )
{
	Int numTeams = m_sides.getNumTeams();
	for (Int i = 0; i < numTeams; ++i) {
		TeamsInfo *ti = m_sides.getTeamInfo(i);
		if (!ti) {
			continue;
		}

		Bool exists;
		AsciiString owner = ti->getDict()->getAsciiString(TheKey_teamOwner, &exists);

		if (exists) {
			if (isValidTeamOwner(owner)) {
				continue;
			}
		}

		doCorrectTeamOwnerDialog(ti);
	}

}

Bool CTeamsDialog::isValidTeamOwner( AsciiString ownerName )
{
	Int numOwners = m_sides.getNumSides();
	for (Int i = 0; i < numOwners; ++i) {
		SidesInfo *side = m_sides.getSideInfo(i);
		if (!side) {
			continue;
		}

		Bool exists;
		AsciiString sideOwnerName = side->getDict()->getAsciiString(TheKey_playerName, &exists);

		if (!exists) {
			continue;
		}

		if (ownerName == sideOwnerName) {
			return true;
		}

		if (sideOwnerName.isEmpty()) {
			sideOwnerName = NEUTRAL_NAME_STR;
		}

		if (ownerName == sideOwnerName) {
			return true;
		}
	}

	return false;
}

void CTeamsDialog::doCorrectTeamOwnerDialog( TeamsInfo *ti )
{
	CFixTeamOwnerDialog fix(ti, &m_sides);
	if (fix.DoModal() == IDOK) {
		if (fix.pickedValidTeam()) {
			ti->getDict()->setAsciiString(TheKey_teamOwner, fix.getSelectedOwner());
		}
	}
}

