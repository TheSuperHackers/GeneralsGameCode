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

// SpherePropertySheet.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "SpherePropertySheet.h"
#include "Utils.h"
#include "W3DViewDoc.h"
#include "assetmgr.h"
#include "DataTreeView.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(SpherePropertySheetClass, CPropertySheet)


/////////////////////////////////////////////////////////////////////////////
//
// SpherePropertySheetClass
//
/////////////////////////////////////////////////////////////////////////////
SpherePropertySheetClass::SpherePropertySheetClass
(
	SphereRenderObjClass *	sphere,
	UINT							nIDCaption,
	CWnd *						pParentWnd,
	UINT							iSelectPage
)
	:	m_RenderObj (NULL),
		CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	MEMBER_ADD (m_RenderObj, sphere);
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// SpherePropertySheetClass
//
/////////////////////////////////////////////////////////////////////////////
SpherePropertySheetClass::SpherePropertySheetClass
(
	SphereRenderObjClass *		sphere,
	LPCTSTR							pszCaption,
	CWnd *							pParentWnd,
	UINT								iSelectPage
)
	:	m_RenderObj (NULL),		
		CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	MEMBER_ADD (m_RenderObj, sphere);
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~SpherePropertySheetClass
//
/////////////////////////////////////////////////////////////////////////////
SpherePropertySheetClass::~SpherePropertySheetClass (void)
{
	MEMBER_RELEASE (m_RenderObj);
	return ;
}


BEGIN_MESSAGE_MAP(SpherePropertySheetClass, CPropertySheet)
	//{{AFX_MSG_MAP(SpherePropertySheetClass)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SpherePropertySheetClass
/////////////////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////
//
//  WindowProc
//
/////////////////////////////////////////////////////////////
LRESULT
SpherePropertySheetClass::WindowProc
(
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (message)
	{
		// Is a control sending us an oldstyle notification?
		case WM_COMMAND:
		{
			// What control sent the notification?
			switch (LOWORD (wParam))
			{
				case IDCANCEL:
				{
					::GetCurrentDocument ()->Reload_Displayed_Object ();
				}
				break;

				/*case IDOK:
				{
					// If the apply button isn't enabled, then don't do the apply operation.
					if (::IsWindowEnabled (::GetDlgItem (m_hWnd, ID_APPLY_NOW)) == FALSE) {
						break;
					}
				}*/

				case IDOK:
				case ID_APPLY_NOW:
				{					
					// Did the user click the button?
					if (HIWORD (wParam) == BN_CLICKED) {
						LRESULT lresult = CPropertySheet::WindowProc (message, wParam, lParam);

						// If all the pages contain valid data, then update the emitter
						if (	m_GeneralPage.Is_Data_Valid () &&
								m_ColorPage.Is_Data_Valid () &&
								m_ScalePage.Is_Data_Valid ())
						{							
							// Update the current emitter to match the data
							Update_Object ();
						}
												
						return lresult;
					}
				}
				break;
			}
			break;
		}
		break;
	}

	// Allow the base class to process this message
	return CPropertySheet::WindowProc (message, wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  Add_Object_To_Viewer
//
/////////////////////////////////////////////////////////////
void
SpherePropertySheetClass::Add_Object_To_Viewer (void)
{
	CW3DViewDoc *doc = ::GetCurrentDocument ();
	if ((doc != NULL) && (m_RenderObj != NULL)) {
		
		//
		// Create a new prototype for this object
		//
		SpherePrototypeClass *prototype	= new SpherePrototypeClass (m_RenderObj);
		
		//
		// Update the asset manager with the new prototype
		//
		if (m_LastSavedName.GetLength () > 0) {
			WW3DAssetManager::Get_Instance()->Remove_Prototype (m_LastSavedName);
		}
		WW3DAssetManager::Get_Instance()->Add_Prototype (prototype);
		
		//
		// Add this object to the data tree
		//
		CDataTreeView *data_tree = doc->GetDataTreeView ();
		data_tree->Refresh_Asset (m_RenderObj->Get_Name (), m_LastSavedName, TypePrimitives);

		//
		// Display the object
		//
		doc->Reload_Displayed_Object ();
		m_LastSavedName = m_RenderObj->Get_Name ();
		MEMBER_ADD (m_RenderObj, (SphereRenderObjClass *)doc->GetDisplayedObject ());

		//
		// Pass the object along to the pages
		//
		m_GeneralPage.Set_Sphere (m_RenderObj);
		m_ColorPage.Set_Sphere (m_RenderObj);
		m_ScalePage.Set_Sphere (m_RenderObj);
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Object
//
/////////////////////////////////////////////////////////////
void
SpherePropertySheetClass::Update_Object (void)
{
	Add_Object_To_Viewer ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
SpherePropertySheetClass::Initialize (void)
{
	if (m_RenderObj == NULL) {
		Create_New_Object ();
	} else {
		m_LastSavedName = m_RenderObj->Get_Name ();
	}

	//
	// Pass the object along to the pages
	//
	m_GeneralPage.Set_Sphere (m_RenderObj);
	m_ColorPage.Set_Sphere (m_RenderObj);
	m_ScalePage.Set_Sphere (m_RenderObj);

	//
	// Add the pages to the sheet
	//
	AddPage (&m_GeneralPage);
	AddPage (&m_ColorPage);
	AddPage (&m_ScalePage);

	//
	//	Force the pages to be created up front
	//	
	m_GeneralPage.m_psp.dwFlags	|= PSP_PREMATURE;
	m_ColorPage.m_psp.dwFlags		|= PSP_PREMATURE;
	m_ScalePage.m_psp.dwFlags		|= PSP_PREMATURE;
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Create_New_Object
//
/////////////////////////////////////////////////////////////
void
SpherePropertySheetClass::Create_New_Object (void)
{
	m_RenderObj = new SphereRenderObjClass;
	m_RenderObj->Set_Name ("Sphere");

	//
	//	Display the new object
	//
	::GetCurrentDocument ()->DisplayObject (m_RenderObj);
	return ;
}

