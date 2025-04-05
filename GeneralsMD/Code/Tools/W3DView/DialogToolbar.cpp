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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/DialogToolbar.cpp              $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/23/99 10:58a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.H"
#include "DialogToolbar.H"
#include "AfxPriv.H"


BEGIN_MESSAGE_MAP(DialogToolbarClass, CToolBar)
	//{{AFX_MSG_MAP(DialogToolbarClass)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE_VOID(WM_INITIALUPDATE, OnInitialUpdate)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnNeedToolTipText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


DialogToolbarClass::DialogToolbarClass (void)
	: CToolBar ()
{
	//{{AFX_DATA_INIT(DialogToolbarClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	return ;
}

#ifdef _DEBUG
void DialogToolbarClass::AssertValid() const
{
	CToolBar::AssertValid();
}

void DialogToolbarClass::Dump(CDumpContext& dc) const
{
	CToolBar::Dump(dc);
}
#endif //_DEBUG


///////////////////////////////////////////////////////////////////
//
//	Enable_Button
//
void
DialogToolbarClass::Enable_Button
(
	int id,
	bool benable
)
{
	// Get the button's style (we enable by using a style bit)
	int index = CommandToIndex (id);	
	UINT style = GetButtonStyle (index) & (~TBBS_DISABLED);
	
	// If we are disabling the button, set the 
	// disabled style bit.
	if (benable == false) {
		style |= TBBS_DISABLED;
		style &= ~TBBS_PRESSED;
	}

	// If the button isn't a separator, then modify its style
	if (!(style & TBBS_SEPARATOR)) {
		SetButtonStyle (index, style);
	}

	return ;
}


///////////////////////////////////////////////////////////////////
//
//	OnIdleUpdateCmdUI
//
LRESULT
DialogToolbarClass::OnIdleUpdateCmdUI (WPARAM, LPARAM)
{
	return 0L;
}


///////////////////////////////////////////////////////////////////
//
//	OnInitialUpdate
//
void
DialogToolbarClass::OnInitialUpdate (void)
{
	return ;
}


///////////////////////////////////////////////////////////////////
//
//	OnNotify
//
BOOL
DialogToolbarClass::OnNeedToolTipText
(
	UINT id,
	NMHDR *pTTTStruct,
	LRESULT *pResult
)
{
	if (pTTTStruct->code == TTN_NEEDTEXTA) {
		
		TOOLTIPTEXTA *ptooltip_info = (TOOLTIPTEXTA*)pTTTStruct;
		::lstrcpy (ptooltip_info->szText, "test");
	}

	(*pResult) = 0L;
	return TRUE;
}
