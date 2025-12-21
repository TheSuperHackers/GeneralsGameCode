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

// AnimReportPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "W3DViewDoc.h"
#include "AdvancedAnimSheet.h"
#include "AnimReportPage.h"


#include "hanim.h"
#include "htree.h"
#include "Utils.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CAnimReportPage property page

IMPLEMENT_DYNCREATE(CAnimReportPage, CPropertyPage)

CAnimReportPage::CAnimReportPage(CAdvancedAnimSheet *sheet)
:	CPropertyPage(CAnimReportPage::IDD),
	m_Sheet(sheet)
{
	//{{AFX_DATA_INIT(CAnimReportPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CAnimReportPage::~CAnimReportPage()
{
}

void CAnimReportPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimReportPage)
	DDX_Control(pDX, IDC_ANIM_REPORT, m_AnimReport);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimReportPage, CPropertyPage)
	//{{AFX_MSG_MAP(CAnimReportPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimReportPage message handlers

BOOL CAnimReportPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnimReportPage::FillListControl()
{
	// Add the first column to the animation report. All bone names will
	// go in this column.
	m_AnimReport.InsertColumn(0, "Bone Name");

	// Get the current render object and it's HTree. If it doesn't have
	// an HTree, then it's not animating and we're not interested.
	RenderObjClass *robj = ::GetCurrentDocument()->GetDisplayedObject();
	if (robj == nullptr)
		return;
	const HTreeClass *htree = robj->Get_HTree();
	if (htree == nullptr)
		return;

	// Get a sorted array of animations that affect the currently active object.
	HAnimClass **anim = m_Sheet->GetAnims();
	int anim_count = m_Sheet->GetAnimCount();


	/*
	** Create a column in the report view for each animation.
	*/

	int	column_count;		// number of columns in the report view (EXCLUDING the bone column)
	bool	indirect;			// true if we have one column per animation selected in the mixing page
	if (m_Sheet->m_SelectedAnims.Count() > 0)
	{
		column_count = m_Sheet->m_SelectedAnims.Count();
		indirect = true;
	}
	else
	{
		column_count = anim_count;
		indirect = false;
	}

	int i, j;
	int anim_idx;	// column index, essentially (anim_idx + 1 == column for that anim)
	for (i = 0; i < column_count; i++)
	{
		// Add a new column with the name of this animation as the title.
		anim_idx = indirect ? m_Sheet->m_SelectedAnims[i] : i;
		if (m_AnimReport.InsertColumn(anim_idx+1, anim[anim_idx]->Get_Name(), LVCFMT_CENTER) == -1)
		{
			// Failed to add a new column to the list control.
			ASSERT(false);
		}
	}


	/*
	** Add a row to the report for each bone in the object affected by
	** an animation. It's entry (row/col) will contain the animation
	** channels with valid animation data.
	*/

	for (i = 0; i < column_count; i++)
	{
		// Grab the anim pointer directly.
		anim_idx = indirect ? m_Sheet->m_SelectedAnims[i] : i;
		HAnimClass *pAnim = anim[anim_idx];

		// Which bones does it affect, and how?
		int num_bones = robj->Get_Num_Bones();
		for (j = 1; j < num_bones; j++)	// skip bone 0, which is always the root bone
		{
			// Add each bone to the report (regardless of animation status)
			// if it isn't already there.
			int idx = FindItem(htree->Get_Bone_Name(j));
			if (idx == -1)
			{
				// Wasn't present, add a new item for this bone.
				idx = m_AnimReport.InsertItem(m_AnimReport.GetItemCount(),
					htree->Get_Bone_Name(j));
			}

			if (pAnim->Is_Node_Motion_Present(j))
			{
				// Add motion channel info to the appropriate column.
				char channels[6];	// strlen("XYZQV")+1
				ZeroMemory(channels, sizeof(channels));
				MakeChannelStr(j, pAnim, channels);
				m_AnimReport.SetItem(idx, i+1, LVIF_TEXT, channels, 0,0,0,0);
			}
		}
	}


	// Make the columns sized nicely.
	m_AnimReport.SetColumnWidth(0, LVSCW_AUTOSIZE);
	for (i = 0; i < column_count; i++)
		m_AnimReport.SetColumnWidth(i+1, LVSCW_AUTOSIZE_USEHEADER);

	// All done, the report view is set up.
}

int CAnimReportPage::FindItem (const char *item_name)
{
	LVFINDINFO	lvfi;
	lvfi.flags = LVFI_STRING;
	lvfi.psz = item_name;
	return m_AnimReport.FindItem(&lvfi);
}

void CAnimReportPage::MakeChannelStr (int bone_idx, HAnimClass *hanim, char channels[6])
{
	if (hanim->Has_X_Translation(bone_idx))
		strcat(channels, "X");
	if (hanim->Has_Y_Translation(bone_idx))
		strcat(channels, "Y");
	if (hanim->Has_Z_Translation(bone_idx))
		strcat(channels, "Z");
	if (hanim->Has_Rotation(bone_idx))
		strcat(channels, "Q");
	if (hanim->Has_Visibility(bone_idx))
		strcat(channels, "V");
}

BOOL CAnimReportPage::OnSetActive()
{
	// Delete all info in the report view.
	m_AnimReport.DeleteAllItems();
	while (m_AnimReport.DeleteColumn(0));

	// Fill the list control each time we're set active so that
	// a change in selection on the mixing page will have an
	// effect on this page.
	FillListControl();

	return CPropertyPage::OnSetActive();
}
