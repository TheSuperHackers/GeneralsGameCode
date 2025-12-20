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

/////////////////////////////////////////////////////////////////////////////
//
//  Utils.h
//
//  Module containing usefull misc. utility functions
//

#pragma once

#include "Vector.h"

// Forward declarations
class RenderObjClass;


/////////////////////////////////////////////////////////////////////////////
//
// Macros
//
#define SAFE_DELETE(pobject) { delete pobject; pobject = NULL; }
#define SAFE_DELETE_ARRAY(pobject) { delete [] pobject; pobject = NULL; }

#define COM_RELEASE(pobject)					\
			if (pobject) {							\
				pobject->Release ();				\
			}											\
			pobject = NULL;						\

#define SAFE_CLOSE(handle)								\
			if (handle != INVALID_HANDLE_VALUE) {	\
				::CloseHandle (handle);					\
				handle = INVALID_HANDLE_VALUE;		\
			}													\

#define SANITY_CHECK(expr)		\
			ASSERT (expr);			\
			if (!expr)

/////////////////////////////////////////////////////////////////////////////
//
// Inlines
//
/////////////////////////////////////////////////////////////////////////////

__inline void Delimit_Path (LPTSTR path)
{
	if (::lstrlen (path) > 0 && path[::lstrlen (path) - 1] != '\\') {
		::lstrcat (path, "\\");
	}
	return ;
}

__inline void Delimit_Path (CString &path)
{
	if (path[::lstrlen (path) - 1] != '\\') {
		path += CString ("\\");
	}
	return ;
}


// Forward declarations
class TextureClass;
class CGraphicView;


/////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//
class CW3DViewDoc *	GetCurrentDocument (void);
CGraphicView *			Get_Graphic_View (void);
void						Paint_Gradient (HWND hWnd, BYTE baseRed, BYTE baseGreen, BYTE baseBlue);
void						CenterDialogAroundTreeView (HWND hDlg);

//
// Dialog routines
//
void						SetDlgItemFloat (HWND hdlg, UINT child_id, float value);
float						GetDlgItemFloat (HWND hdlg, UINT child_id);
void						SetWindowFloat (HWND hwnd, float value);
float						GetWindowFloat (HWND hwnd);
void						Initialize_Spinner (CSpinButtonCtrl &ctrl, float pos = 0, float min = 0, float max = 1);
void						Update_Spinner_Buddy (CSpinButtonCtrl &ctrl);
void						Update_Spinner_Buddy (HWND hspinner, int delta);
void						Enable_Dialog_Controls (HWND dlg,bool onoff);

//
//	String manipulation routines
//
CString					Get_Filename_From_Path (LPCTSTR path);
CString					Strip_Filename_From_Path (LPCTSTR path);
CString					Asset_Name_From_Filename (LPCTSTR filename);
CString					Filename_From_Asset_Name (LPCTSTR asset_name);

//
//	File routines
//
bool						Get_File_Time (LPCTSTR path, LPFILETIME pcreation_time, LPFILETIME paccess_time = NULL, LPFILETIME pwrite_time = NULL);
bool						Are_Glide_Drivers_Acceptable (void);
bool						Copy_File (LPCTSTR existing_filename, LPCTSTR new_filename, bool bforce_copy = false);

//
//	Texture routines
//
HBITMAP					Make_Bitmap_From_Texture (TextureClass &texture, int width, int height);
CString					Get_Texture_Name (TextureClass &texture);
TextureClass *			Load_RC_Texture (LPCTSTR resource_name);
void						Find_Missing_Textures (DynamicVectorClass<CString> &list, LPCTSTR filename, int frame_count = 1);


// Emitter routines
void						Build_Emitter_List (RenderObjClass &render_obj, DynamicVectorClass<CString> &list);

// Identification routines
bool						Is_Aggregate (const char *asset_name);
bool						Is_Real_LOD (const char *asset_name);

// Prototype routines
void						Rename_Aggregate_Prototype (const char *old_name, const char *new_name);
