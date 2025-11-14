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
 *                 Project Name : FormClass                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/FormClass.cpp                  $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/09/98 3:02a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FormClass::Create_Form -- Loads the dialog template and initializes                       *
 *   FormClass::fnFormProc -- windows proc which thunks into the virtual Dialog_Proc           *
 *   FormClass::ExecuteDlgInit -- Initializes controls in the dialog template                  *
 *   FormClass::ExecuteDlgInit -- Initializes the controls in the dialog template              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "FormClass.h"
#include "dllmain.h"

// hard-coded resource id which VC special cases for MFC... >:-)
#define RT_DLGINIT  MAKEINTRESOURCE(240)


/***********************************************************************************************
 * FormClass::Create_Form -- Loads the dialog template and initializes                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/98    GTH : Created.                                                                 *
 *=============================================================================================*/
HWND
FormClass::Create_Form
(
	HWND parent_wnd,
	UINT template_id
)
{
	// call PreCreateWindow to get prefered extended style
	CREATESTRUCT cs = { 0 };
	cs.style = WS_CHILD;

	m_hWnd = ::CreateDialogParam(	AppInstance,
											MAKEINTRESOURCE (template_id),
											parent_wnd,
											fnFormProc,
											(LPARAM)this);

	assert(m_hWnd);

	// Remove the caption from the dialog (if there was any)
	::SetWindowLong (m_hWnd,
						  GWL_STYLE,
						  ::GetWindowLong (m_hWnd, GWL_STYLE) & (~WS_CAPTION));

	::GetWindowRect (m_hWnd, &m_FormRect);

	ExecuteDlgInit(MAKEINTRESOURCE(template_id));

	return m_hWnd;
}


/***********************************************************************************************
 * FormClass::fnFormProc -- windows proc which thunks into the virtual Dialog_Proc             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/98    GTH : Created.                                                                 *
 *=============================================================================================*/
BOOL WINAPI
FormClass::fnFormProc
(
	HWND dlg_wnd,
	UINT message,
	WPARAM wparam,
	LPARAM lparam
)
{
	FormClass *pform = (FormClass *)::GetProp (dlg_wnd, "FORMCLASS");

	if (message == WM_INITDIALOG) {
		pform = (FormClass *)lparam;
		::SetProp (dlg_wnd, "FORMCLASS", (HANDLE)pform);
	} else if (message == WM_DESTROY) {
		::RemoveProp (dlg_wnd, "FORMCLASS");
	}

	BOOL retval = FALSE;
	if (pform) {
		retval = pform->Dialog_Proc (dlg_wnd, message, wparam, lparam);
	}

	return retval;
}


/***********************************************************************************************
 * FormClass::ExecuteDlgInit -- Initializes controls in the dialog template                    *
 *                                                                                             *
 * This code was lifted straight out of MFC.  It does some "interesting" things like putting   *
 * strings into combo boxes which are typed in the resource editor.                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/98    GTH : Created.                                                                 *
 *=============================================================================================*/
BOOL FormClass::ExecuteDlgInit(LPCTSTR lpszResourceName)
{
	// find resource handle
	LPVOID lpResource = NULL;
	HGLOBAL hResource = NULL;
	if (lpszResourceName != NULL)
	{
		HINSTANCE hInst = AppInstance;
		HRSRC hDlgInit = ::FindResource(hInst, lpszResourceName, RT_DLGINIT);
		if (hDlgInit != NULL)
		{
			// load it
			hResource = LoadResource(hInst, hDlgInit);
			if (hResource == NULL)
				return FALSE;
			// lock it
			lpResource = LockResource(hResource);
			assert(lpResource != NULL);
		}
	}

	// execute it
	BOOL bResult = ExecuteDlgInit(lpResource);

	// cleanup
	if (lpResource != NULL && hResource != NULL)
	{
		UnlockResource(hResource);
		FreeResource(hResource);
	}
	return bResult;
}


/***********************************************************************************************
 * FormClass::ExecuteDlgInit -- Initializes the controls in the dialog template                *
 *                                                                                             *
 * As the above ExecuteDlgInit function, this was lifted from MFC...                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/98    GTH : Created.                                                                 *
 *=============================================================================================*/
BOOL FormClass::ExecuteDlgInit(LPVOID lpResource)
{
	BOOL bSuccess = TRUE;
	if (lpResource != NULL)
	{
		UNALIGNED WORD* lpnRes = (WORD*)lpResource;
		while (bSuccess && *lpnRes != 0)
		{
			WORD nIDC = *lpnRes++;
			WORD nMsg = *lpnRes++;
			DWORD dwLen = *((UNALIGNED DWORD*&)lpnRes)++;

			// In Win32 the WM_ messages have changed.  They have
			// to be translated from the 32-bit values to 16-bit
			// values here.
			#define WIN16_LB_ADDSTRING  0x0401
			#define WIN16_CB_ADDSTRING  0x0403

			if (nMsg == WIN16_LB_ADDSTRING)
				nMsg = LB_ADDSTRING;
			else if (nMsg == WIN16_CB_ADDSTRING)
				nMsg = CB_ADDSTRING;

			// check for invalid/unknown message types
			assert(nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING);

			if (nMsg == LB_ADDSTRING || nMsg == CB_ADDSTRING)
			{
				// List/Combobox returns -1 for error
				if (::SendDlgItemMessageA(m_hWnd, nIDC, nMsg, 0, (LONG)lpnRes) == -1)
					bSuccess = FALSE;
			}

			// skip past data
			lpnRes = (WORD*)((LPBYTE)lpnRes + (UINT)dwLen);
		}
	}

	return bSuccess;
}
