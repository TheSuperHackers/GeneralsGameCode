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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Bay/Tools/max2w3d/GameMtlPassDlg.cpp                       $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 10/22/99 9:54a                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PassDlgProc -- dialog proc which thunks into a GameMtlPassDlg                             *
 *   GameMtlPassDlg::GameMtlPassDlg -- constructor                                             *
 *   GameMtlPassDlg::~GameMtlPassDlg -- destructor                                             *
 *   GameMtlPassDlg::DialogProc -- windows message handler                                     *
 *   GameMtlPassDlg::Invalidate -- invalidate the dialog                                       *
 *   GameMtlPassDlg::ReloadDialog -- update the contents of all of the controls                *
 *   GameMtlPassDlg::ClassID -- returns classID of the object being edited                     *
 *   GameMtlPassDlg::SetThing -- set the material being edited                                 *
 *   GameMtlPassDlg::ActivateDlg -- activate or deactivate the dialog                          *
 *   GameMtlPassDlg::SetTime -- set the current time                                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "GameMtlPassDlg.h"
#include "dllmain.h"
#include "resource.h"
#include "gamemtl.h"
#include "GameMtlDlg.h"
#include "GameMtlShaderDlg.h"
#include "PS2GameMtlShaderDlg.h"
#include "GameMtlTextureDlg.h"
#include "GameMtlVertexMaterialDlg.h"
#include "w3d_file.h"


static int _Pass_Index_To_Flag[] = 
{
	GAMEMTL_PASS0_ROLLUP_OPEN,
	GAMEMTL_PASS1_ROLLUP_OPEN,
	GAMEMTL_PASS2_ROLLUP_OPEN,
	GAMEMTL_PASS3_ROLLUP_OPEN,
};


/***********************************************************************************************
 * PassDlgProc -- dialog proc which thunks into a GameMtlPassDlg                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
static BOOL CALLBACK PassDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	GameMtlPassDlg *theDlg;

	if (msg==WM_INITDIALOG) {
		theDlg = (GameMtlPassDlg*)lParam;
		theDlg->HwndPanel = hwndDlg;
		SetWindowLong(hwndDlg, GWL_USERDATA,lParam);
	} else {
		if ((theDlg = (GameMtlPassDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
			return FALSE; 
		}
	}

	return theDlg->DialogProc(hwndDlg,msg,wParam,lParam);
}


/***********************************************************************************************
 * GameMtlPassDlg::GameMtlPassDlg -- constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
GameMtlPassDlg::GameMtlPassDlg(HWND hwMtlEdit, IMtlParams *imp, GameMtl *m,int pass)
{
	HwndEdit = hwMtlEdit;
	TheMtl = m;
	IParams = imp;
	PassIndex = pass;

	char title[200];
	sprintf(title, "Pass %d", pass + 1);

	HwndPanel = IParams->AddRollupPage( 
		AppInstance,
		MAKEINTRESOURCE(IDD_GAMEMTL_PASS),
		PassDlgProc,
		title,
		(LPARAM)this,
		TheMtl->Get_Flag(_Pass_Index_To_Flag[PassIndex]) ? 0 : APPENDROLL_CLOSED
	);
}


/***********************************************************************************************
 * GameMtlPassDlg::~GameMtlPassDlg -- destructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
GameMtlPassDlg::~GameMtlPassDlg()
{
	TheMtl->Set_Flag(_Pass_Index_To_Flag[PassIndex],IParams->IsRollupPanelOpen(HwndPanel));
	IParams->DeleteRollupPage(HwndPanel);
	SetWindowLong(HwndPanel, GWL_USERDATA, NULL);
}


/***********************************************************************************************
 * GameMtlPassDlg::DialogProc -- windows message handler                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
BOOL GameMtlPassDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i=0;
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);

	switch (message) {

		case WM_INITDIALOG:
		{
			Page[0] = new GameMtlVertexMaterialDlg(HwndPanel,IParams,TheMtl,PassIndex);
			if (TheMtl->Get_Shader_Type() == GameMtl::STE_PC_SHADER)	{
				Page[1] = new GameMtlShaderDlg(HwndPanel,IParams,TheMtl,PassIndex);
			} else {
				// The PS2 shader is different.
				Page[1] = new PS2GameMtlShaderDlg(HwndPanel,IParams,TheMtl,PassIndex);
			}

			Page[2] = new GameMtlTextureDlg(HwndPanel,IParams,TheMtl,PassIndex);

			for (i=0; i<PAGE_COUNT; i++) {

				HWND hwnd = Page[i]->Get_Hwnd();
			
				// set the tab names
				char name[64];
				::GetWindowText(hwnd,name,sizeof(name));
				TC_ITEM tcitem = { TCIF_TEXT,0,0,name,0 };
				TabCtrl_InsertItem(GetDlgItem(HwndPanel,IDC_GAMEMTL_TAB),i,&tcitem);
			}

			// Get the display rectangle of the tab control
			RECT rect;
			::GetWindowRect(GetDlgItem(HwndPanel,IDC_GAMEMTL_TAB),&rect);
			TabCtrl_AdjustRect(GetDlgItem(HwndPanel,IDC_GAMEMTL_TAB),FALSE, &rect);
  
			// Convert the display rectangle from screen to client coords
			ScreenToClient(HwndPanel,(POINT *)(&rect));
			ScreenToClient(HwndPanel, ((LPPOINT)&rect) + 1);

			for (i=0; i<PAGE_COUNT; i++) {

				HWND hwnd = Page[i]->Get_Hwnd();

				// Loop through all the tabs in the property sheet
				// Get a pointer to this tab				
				SetWindowPos(	hwnd, 
									NULL, 
									rect.left, rect.top, 
									rect.right - rect.left, rect.bottom - rect.top, 
									SWP_NOZORDER);				
			}
			
 			CurPage = 0;
			TabCtrl_SetCurSel(GetDlgItem(HwndPanel,IDC_GAMEMTL_TAB),CurPage); 
			Page[CurPage]->Show();

			break;
		}
	
		case WM_PAINT: 
		{
			if (!Valid) {
				Valid = TRUE;
				ReloadDialog();
			}
			return FALSE;
		}
		
		case WM_NOTIFY:
		{
			NMHDR * header = (NMHDR *)lParam;
			
			switch(header->code) { 
				case TCN_SELCHANGE:
				{
					int sel = TabCtrl_GetCurSel(GetDlgItem(HwndPanel,IDC_GAMEMTL_TAB)); 
					Page[sel]->Show();

					for (int i=0; i < PAGE_COUNT; i++) {
						if (i != sel) Page[i]->Show(false);
					} 
					CurPage = sel;
					TheMtl->Set_Current_Page(PassIndex,CurPage);
				}
			};
			break;
		}
	}
	return FALSE;
}


/***********************************************************************************************
 * GameMtlPassDlg::Invalidate -- invalidate the dialog                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtlPassDlg::Invalidate()
{
	Valid = FALSE;
	InvalidateRect(HwndPanel,NULL,0);
}


/***********************************************************************************************
 * GameMtlPassDlg::ReloadDialog -- update the contents of all of the controls                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtlPassDlg::ReloadDialog()
{
	int i;

	DebugPrint("GameMtlPassDlg::ReloadDialog\n");
	Interval v;
	TheMtl->Update(IParams->GetTime(),v);
	
	for (i=0; i<PAGE_COUNT; i++) {
		Page[i]->ReloadDialog();
	}

	CurPage = TheMtl->Get_Current_Page(PassIndex);
	TabCtrl_SetCurSel(GetDlgItem(HwndPanel,IDC_GAMEMTL_TAB),CurPage);
	Page[CurPage]->Show();
	for (i=0; i < PAGE_COUNT; i++) {
		if (i != CurPage) Page[i]->Show(false);
	} 
}


/***********************************************************************************************
 * GameMtlPassDlg::ClassID -- returns classID of the object being edited                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
Class_ID	GameMtlPassDlg::ClassID()
{
	return GameMaterialClassID;  
}


/***********************************************************************************************
 * GameMtlPassDlg::SetThing -- set the material being edited                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtlPassDlg::SetThing(ReferenceTarget* target)
{
	// Note, parent will "reload" when our "thing" changes :-)
	assert (target->SuperClassID()==MATERIAL_CLASS_ID);
	assert (target->ClassID()==GameMaterialClassID);

	TheMtl = (GameMtl *)target;
	
	for (int i=0; i<PAGE_COUNT; i++) {
		Page[i]->SetThing(target);
	}
}


/***********************************************************************************************
 * GameMtlPassDlg::ActivateDlg -- activate or deactivate the dialog                            *
 *                                                                                             *
 * some of the custom max controls need to be activated and deactivated                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtlPassDlg::ActivateDlg(BOOL onoff)
{
	for (int i=0; i<PAGE_COUNT; i++) {
		Page[i]->ActivateDlg(onoff);
	}
}


/***********************************************************************************************
 * GameMtlPassDlg::SetTime -- set the current time                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtlPassDlg::SetTime(TimeValue t)
{
	// parent dialog class keeps track of the validty and we 
	// don't have to do anything in this function (it will never
	// be called in fact)
}

