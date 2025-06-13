/*
**	Command & Conquer Renegade(tm)
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

// AnimMixingPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "W3DViewDoc.h"
#include "AdvancedAnimSheet.h"
#include "AnimMixingPage.h"

#include "rendobj.h"
#include "htree.h"
#include "hanim.h"
#include "Utils.h"
#include "assetmgr.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimMixingPage property page

IMPLEMENT_DYNCREATE(CAnimMixingPage, CPropertyPage)

CAnimMixingPage::CAnimMixingPage(CAdvancedAnimSheet *sheet)
:	CPropertyPage(CAnimMixingPage::IDD),
	m_Sheet(sheet)
{
	//{{AFX_DATA_INIT(CAnimMixingPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAnimMixingPage::~CAnimMixingPage()
{
}

void CAnimMixingPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimMixingPage)
	DDX_Control(pDX, IDC_ANIMATION_LIST, m_AnimList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimMixingPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAnimMixingPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimMixingPage message handlers

BOOL CAnimMixingPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	ASSERT(m_Sheet != NULL);
	FillListCtrl();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CAnimMixingPage::FillListCtrl (void)
{
	// Get the current render object and it's HTree. If it doesn't have
	// an HTree, then it's not animating and we're not interested.
	RenderObjClass *robj = ::GetCurrentDocument()->GetDisplayedObject();
	if (robj == NULL)
		return;
	const HTreeClass *htree = robj->Get_HTree();
	if (htree == NULL)
		return;

	// Get a sorted array of animations that affect the currently active object.
	HAnimClass **anim = m_Sheet->GetAnims();
	int anim_count = m_Sheet->GetAnimCount();

	// Add an item to the list view for each animation.
	int i;
	for (i = 0; i < anim_count; i++)
		m_AnimList.InsertItem(i, anim[i]->Get_Name());
}

void CAnimMixingPage::OnOK() 
{
	/*
	** Create a new HAnimCombo class containing the animations selected by the user.
	*/
	int num_selected = m_AnimList.GetSelectedCount();
	RenderObjClass *current_obj = ::GetCurrentDocument()->GetDisplayedObject();
	const char *obj_name = current_obj->Get_Name();
	RenderObjClass *robj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(obj_name);
	if (num_selected > 0 && robj != NULL)
	{
		HAnimClass **anim = m_Sheet->GetAnims();

		HAnimComboClass *combo = new HAnimComboClass(num_selected);
		ASSERT(combo != NULL);

		POSITION pos = m_AnimList.GetFirstSelectedItemPosition();
		int array_idx, idx = 0;
		while (pos)
		{
			array_idx = m_AnimList.GetNextSelectedItem(pos);
			combo->Set_Motion(idx, anim[array_idx]);
			combo->Set_Weight(idx, 1.0f);
			combo->Peek_Anim_Combo_Data(idx)->Build_Active_Pivot_Map();
			idx++;
		}

		/*
		** Set this new combo to be used by the doc.
		*/
		::GetCurrentDocument()->PlayAnimation(robj, combo);
		robj->Release_Ref();
	}
	
	CPropertyPage::OnOK();
}

BOOL CAnimMixingPage::OnKillActive() 
{
	/*
	** Update the parent with info on the current selection.
	*/
	m_Sheet->m_SelectedAnims.Clear();
	m_Sheet->m_SelectedAnims.Resize(m_AnimList.GetSelectedCount());

	int selected_idx, i=0;
	POSITION pos = m_AnimList.GetFirstSelectedItemPosition();
	while (pos)
	{
		selected_idx = m_AnimList.GetNextSelectedItem(pos);
		if (!m_Sheet->m_SelectedAnims.Add(selected_idx))
		{
			char buf[128];
			sprintf(buf, "Failed to insert item %d! Talk to Andre if "\
				"you see this message.", i);
			MessageBox(buf);
		}
		i++;
	}
	
	return CPropertyPage::OnKillActive();
}
