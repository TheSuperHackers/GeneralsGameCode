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

// AdvancedAnimSheet.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "W3DViewDoc.h"
#include "AdvancedAnimSheet.h"

#include "assetmgr.h"
#include "hanim.h"
#include "htree.h"
#include "Utils.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// QSort comparison function for HAnimClass pointers.
// Will compare their names.
static int anim_name_compare (const void *arg1, const void *arg2)
{
	ASSERT(arg1 != NULL);
	ASSERT(arg2 != NULL);
	HAnimClass *a1 = *(HAnimClass**)arg1;
	HAnimClass *a2 = *(HAnimClass**)arg2;
	return _stricmp( a1->Get_Name(), a2->Get_Name() );
}



/////////////////////////////////////////////////////////////////////////////
// CAdvancedAnimSheet

IMPLEMENT_DYNAMIC(CAdvancedAnimSheet, CPropertySheet)

CAdvancedAnimSheet::CAdvancedAnimSheet(CWnd* pParentWnd, UINT iSelectPage)
:	CPropertySheet("Advanced Animation", pParentWnd, iSelectPage),
	m_MixingPage(this),
	m_ReportPage(this),
	AnimsValid(false),
	AnimCount(0)
{
	// Blank out the array of animation pointers.
	ZeroMemory(Anims, sizeof(Anims));

	// Add the property pages into this property sheet.
	AddPage(&m_MixingPage);
	AddPage(&m_ReportPage);
}

CAdvancedAnimSheet::~CAdvancedAnimSheet()
{
	if (AnimsValid)
	{
		for (int i = 0; i < AnimCount; i++)
		{
			MEMBER_RELEASE(Anims[i]);
		}
		AnimsValid = false;
		AnimCount = 0;
	}
}


BEGIN_MESSAGE_MAP(CAdvancedAnimSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CAdvancedAnimSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAdvancedAnimSheet message handlers

int CAdvancedAnimSheet::GetAnimCount (void)
{
	if (AnimsValid)
		return AnimCount;

	LoadAnims();

	if (AnimsValid)
		return AnimCount;
	else
		return 0;
}


HAnimClass ** CAdvancedAnimSheet::GetAnims (void)
{
	if (AnimsValid)
		return Anims;

	LoadAnims();

	// Return the array regardless of validity. If the entries are
	// invalid, they'll all be NULL, but the array itself is cool.
	return Anims;
}


void CAdvancedAnimSheet::LoadAnims (void)
{
	// Get the current render object and it's HTree. If it doesn't have
	// an HTree, then it's not animating and we're not interested.
	RenderObjClass *robj = ::GetCurrentDocument()->GetDisplayedObject();
	if (robj == NULL)
		return;
	const HTreeClass *htree = robj->Get_HTree();
	if (htree == NULL)
		return;
	const char *htree_name = htree->Get_Name();

	/*
	** Figure out which animations apply to the current object, and
	** add each one to the array of animations which we will later sort.
	*/

	// Get an iterator from the asset manager that we can
	// use to enumerate the currently loaded assets
	AssetIterator *pAnimEnum = WW3DAssetManager::Get_Instance()->Create_HAnim_Iterator();
	ASSERT(pAnimEnum != NULL);
	if (pAnimEnum)
	{
		// Loop through all the animations in the manager
		for (pAnimEnum->First(); pAnimEnum->Is_Done() == FALSE; pAnimEnum->Next())
		{
			LPCTSTR pszAnimName = pAnimEnum->Current_Item_Name();

			// Get an instance of the animation object
			HAnimClass *pHierarchyAnim = WW3DAssetManager::Get_Instance()->Get_HAnim(pszAnimName);

			ASSERT(pHierarchyAnim != NULL);
			if (pHierarchyAnim)
			{
				// Does this animation apply to the current model's HTree?
				if (stricmp(htree_name, pHierarchyAnim->Get_HName()) == 0)
				{
					// Add this Anims pointer to the array.
					if (AnimCount < MAX_REPORT_ANIMS)
					{
						MEMBER_ADD(Anims[AnimCount], pHierarchyAnim);
						AnimCount++;
					}
					else
					{
						char msg[256];
						sprintf(msg, "Error: Only %d animations are supported in this report. "
							" There are more than that loaded...", MAX_REPORT_ANIMS);
						MessageBox(msg, "Too Many Animations");
						break;
					}
				}

				// Release our hold on this animation.
				MEMBER_RELEASE(pHierarchyAnim);
			}
		}

		// Free the object
		delete pAnimEnum;
		pAnimEnum = NULL;
	}

	/*
	** Sort the array of animations in alphabetical order.
	*/
	qsort(Anims, AnimCount, sizeof(HAnimClass*), anim_name_compare);


	AnimsValid = true;
}


