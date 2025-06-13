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
//  Utils.CPP
//
//  Module containing usefull misc. utility functions
//


#include "StdAfx.h"
#include "W3DViewDoc.h"
#include "MainFrm.h"
#include "DataTreeView.h"
#include "Utils.h"
#include "texture.h"
#include "assetmgr.h"
#include "agg_def.h"
#include "hlod.h"
#include <VFW.H>
#include "rcfile.h"


////////////////////////////////////////////////////////////////////////////
//
//  GetCurrentDocument
//
////////////////////////////////////////////////////////////////////////////
CW3DViewDoc *
GetCurrentDocument (void)
{
    // Assume failure
    CW3DViewDoc *pCDoc = NULL;

    // Get a pointer to the main window
    CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
    
    ASSERT (pCMainWnd);
    if (pCMainWnd)
    {
        // Use the main window pointer to get a pointer
        // to the current doc.
        pCDoc = (CW3DViewDoc *)pCMainWnd->GetActiveDocument ();
        ASSERT (pCDoc);
    }    
    
    // Return the doc pointer
    return pCDoc;
}

/////////////////////////////////////////////////////////////
//
//  CenterDialogAroundTreeView
//
void
CenterDialogAroundTreeView (HWND hDlg)
{
    // Params OK?
    if (::IsWindow (hDlg))
    {
        // Get a pointer to the main window
        CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
    
        ASSERT (pCMainWnd);
        if (pCMainWnd)
        {
            // Get the tree view pane so we can get its rectangle
            CDataTreeView *pCDataTreeView = (CDataTreeView *)pCMainWnd->GetPane (0, 0);
            
            ASSERT (pCDataTreeView);
            if (pCDataTreeView)
            {
                // Get the bounding rectangle of the data tree view
                RECT rect;
                pCDataTreeView->GetWindowRect (&rect);

                // Get the bounding rectangle of the dialog
                RECT dialogRect;
                ::GetWindowRect (hDlg, &dialogRect);

                // Move the dialog so its centered in the data tree view
                ::SetWindowPos (hDlg,
                                NULL,
                                rect.left + ((rect.right-rect.left) >> 1) - ((dialogRect.right-dialogRect.left) >> 1),
                                rect.top + ((rect.bottom-rect.top) >> 1) - ((dialogRect.bottom-dialogRect.top) >> 1),
                                0,
                                0,
                                SWP_NOSIZE | SWP_NOZORDER);
            }
        }
    }

    return ;
}

/////////////////////////////////////////////////////////////
//
//  Paint_Gradient
//
void
Paint_Gradient
(
	HWND hWnd,
	BYTE baseRed,
	BYTE baseGreen,
	BYTE baseBlue
)
{
    // Get the bounding rectangle so we know how much to paint
    RECT rect;
    ::GetClientRect (hWnd, &rect);

    // Determine the width, height, and width per each shade
    int iWidth = rect.right-rect.left;
    int iHeight = rect.bottom-rect.top;
    float widthPerShade = ((float)iWidth) / 256.00F;

    // Pull a hack to get the CDC for the window
    HDC hDC = ::GetDC (hWnd);
    CDC cDC;
    cDC.Attach(hDC);

    // Loop through each shade and paint its sliver
    float posX = 0.00F;
    for (int iShade = 0; iShade < 256; iShade ++)
    {
        // Paint this sliver
        cDC.FillSolidRect ((int)posX,
                           0,
                           (widthPerShade >= 1.00F) ? ((int)widthPerShade)+1 : 1,
                           iHeight,
                           RGB (iShade*baseRed, iShade*baseGreen, iShade*baseBlue));

        // Increment the current position
        posX += widthPerShade;
    }
    
    // Release the DC
    cDC.Detach ();    
    ::ReleaseDC (hWnd, hDC);

    // Validate the contents of the window so the control won't paint itself
    ::ValidateRect (hWnd, NULL);
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetDlgItemFloat
//
void
SetDlgItemFloat
(
	HWND hdlg,
	UINT child_id,
	float value
)
{
	// Convert the float to a string
	CString text;
	text.Format ("%.2f", value);

	// Pass the string onto the dialog control
	::SetDlgItemText (hdlg, child_id, text);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  GetDlgItemFloat
//
float
GetDlgItemFloat
(
	HWND hdlg,
	UINT child_id
)
{
	// Get the string from the window
	TCHAR string_value[20];
	::GetDlgItemText (hdlg, child_id, string_value, sizeof (string_value));

	// Convert the string to a float and return the value
	return ::atof (string_value);
}


////////////////////////////////////////////////////////////////////////////
//
//  Initialize_Spinner
//
void
Initialize_Spinner
(
	CSpinButtonCtrl &ctrl,
	float pos,
	float min,
	float max
)
{
	//
	//	Convert the floats to ints and pass the settings onto the controls
	//	
	ctrl.SetRange32 (int(min * 100), int(max * 100));
	ctrl.SetPos (int(pos * 100));

	//
	//	Set the buddy's text accordingly
	//
	CWnd *buddy = ctrl.GetBuddy ();
	if (buddy != NULL) {
		::SetWindowFloat (*buddy, pos);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Spinner_Buddy
//
void
Update_Spinner_Buddy (CSpinButtonCtrl &ctrl, int delta)
{
	//
	//	Only perform this service if the spinner isn't an auto buddy
	//
	if ((::GetWindowLong (ctrl, GWL_STYLE) & UDS_SETBUDDYINT) == 0) {
		CWnd *buddy = ctrl.GetBuddy ();
		if (buddy != NULL) {

			// Get the current value, increment it, and put it back into the control
			float value = ::GetWindowFloat (*buddy);
			value += (((float)(delta)) / 100.0F);

			//
			//	Validate the new position
			//
			int int_min = 0;
			int int_max = 0;
			ctrl.GetRange32 (int_min, int_max);
			float float_min = ((float)int_min) / 100;
			float float_max = ((float)int_max) / 100;
			value = max (float_min, value);
			value = min (float_max, value);
			
			// Pass the value onto the buddy window
			::SetWindowFloat (*buddy, value);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Update_Spinner_Buddy
//
void
Update_Spinner_Buddy (HWND hspinner, int delta)
{
	//
	//	Only perform this service if the spinner isn't an auto buddy
	//
	if ((::GetWindowLong (hspinner, GWL_STYLE) & UDS_SETBUDDYINT) == 0) {
		HWND hbuddy_wnd = (HWND)SendMessage (hspinner, UDM_GETBUDDY, 0, 0L);
		if (::IsWindow (hbuddy_wnd)) {

			// Get the current value, increment it, and put it back into the control
			float value = ::GetWindowFloat (hbuddy_wnd);
			value += (((float)(delta)) / 100.0F);

			//
			//	Validate the new position
			//
			int int_min = 0;
			int int_max = 0;
			SendMessage (hspinner, UDM_GETRANGE32, (WPARAM)&int_min, (LPARAM)&int_max);
			float float_min = ((float)int_min) / 100;
			float float_max = ((float)int_max) / 100;
			value = max (float_min, value);			
			value = min (float_max, value);

			// Pass the value onto the buddy window
			::SetWindowFloat (hbuddy_wnd, value);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Enable_Dialog_Controls
//
void
Enable_Dialog_Controls (HWND dlg,bool onoff)
{
	//
	// Loop over all sub-windows enable/disabling everything except for
	// the static text controls
	//
	for (HWND child = ::GetWindow(dlg,GW_CHILD) ; child != NULL ; child = ::GetWindow(child,GW_HWNDNEXT)) {
		char buf[64];
		::GetClassName(child,buf,sizeof(buf));
		if (stricmp(buf,"STATIC") != 0) {
			::EnableWindow(child,onoff);
		}
	}
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetWindowFloat
//
void
SetWindowFloat
(
	HWND hwnd,
	float value
)
{
	// Convert the float to a string
	CString text;
	text.Format ("%.3f", value);

	// Pass the string onto the window
	::SetWindowText (hwnd, text);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  GetWindowFloat
//
float
GetWindowFloat (HWND hwnd)
{
	// Get the string from the window
	TCHAR string_value[20];
	::GetWindowText (hwnd, string_value, sizeof (string_value));

	// Convert the string to a float and return the value
	return ::atof (string_value);
}


////////////////////////////////////////////////////////////////////////////
//
//  Asset_Name_From_Filename
//
CString
Asset_Name_From_Filename (LPCTSTR filename)
{
	// Get the filename from this path
	CString asset_name = ::Get_Filename_From_Path (filename);
	
	// Find the index of the extension (if exists)
	int extension = asset_name.ReverseFind ('.');	
	
	// Strip off the extension
	if (extension != -1) {
		asset_name = asset_name.Left (extension);
	}

	// Return the name of the asset
	return asset_name;
}


////////////////////////////////////////////////////////////////////////////
//
//  Filename_From_Asset_Name
//
CString
Filename_From_Asset_Name (LPCTSTR asset_name)
{
	// The filename is simply the asset name plus the .w3d extension
	CString filename = asset_name + CString (".w3d");

	// Return the filename
	return filename;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Filename_From_Path
//
CString
Get_Filename_From_Path (LPCTSTR path)
{
	// Find the last occurance of the directory deliminator
	LPCTSTR filename = ::strrchr (path, '\\');
	if (filename != NULL) {
		// Increment past the directory deliminator
		filename ++;
	} else {
		filename = path;
	}

	// Return the filename part of the path
	return CString (filename);
}


////////////////////////////////////////////////////////////////////////////
//
//  Strip_Filename_From_Path
//
CString
Strip_Filename_From_Path (LPCTSTR path)
{
	// Copy the path to a buffer we can modify
	TCHAR temp_path[MAX_PATH];
	::lstrcpy (temp_path, path);

	// Find the last occurance of the directory deliminator
	LPTSTR filename = ::strrchr (temp_path, '\\');
	if (filename != NULL) {
		// Strip off the filename
		filename[0] = 0;
	}

	// Return the path only
	return CString (temp_path);
}


////////////////////////////////////////////////////////////////////////////
//
//  Create_DIB_Section
//
HBITMAP
Create_DIB_Section
(
	UCHAR **pbits,
	int width,
	int height
)
{
	// Set-up the fields of the BITMAPINFOHEADER
	BITMAPINFOHEADER bitmap_info;
	bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
	bitmap_info.biWidth = width;
	bitmap_info.biHeight = -height; // Top-down DIB uses negative height
	bitmap_info.biPlanes = 1;
	bitmap_info.biBitCount = 24;
	bitmap_info.biCompression = BI_RGB;
	bitmap_info.biSizeImage = ((width * height) * 3);
	bitmap_info.biXPelsPerMeter = 0;
	bitmap_info.biYPelsPerMeter = 0;
	bitmap_info.biClrUsed = 0;
	bitmap_info.biClrImportant = 0;

	// Get a temporary screen DC
	HDC hscreen_dc = ::GetDC (NULL);

	// Create a bitmap that we can access the bits directly of
	HBITMAP hbitmap = ::CreateDIBSection (hscreen_dc,
													  (const BITMAPINFO *)&bitmap_info,
													  DIB_RGB_COLORS,
													  (void **)pbits,
													  NULL,
													  0L);

	// Release our temporary screen DC
	::ReleaseDC (NULL, hscreen_dc);
	return hbitmap;
}


////////////////////////////////////////////////////////////////////////////
//
//  Make_Bitmap_From_Texture
//
HBITMAP
Make_Bitmap_From_Texture (TextureClass &texture, int width, int height)
{
	// TheSuperHackers @info Not implemented
	HBITMAP hbitmap = NULL;
	// Return a handle to the bitmap
	return hbitmap;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Texture_Name
//
CString
Get_Texture_Name (TextureClass &texture)
{
	CString name;

	name = texture.Get_Texture_Name();

	// Return the texture's name
	return name;
}


////////////////////////////////////////////////////////////////////////////
//
//  Build_Emitter_List
//
void
Build_Emitter_List
(
	RenderObjClass &render_obj,
	DynamicVectorClass<CString> &list
)
{
	// Loop through all this render obj's sub-obj's
	for (int index = 0; index < render_obj.Get_Num_Sub_Objects (); index ++) {
		RenderObjClass *psub_obj = render_obj.Get_Sub_Object (index);
		if (psub_obj != NULL) {

			// Is this sub-obj an emitter?
			if (psub_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
				
				// Is this emitter already in the list?
				bool found = false;
				for (int list_index = 0; (list_index < list.Count ()) && !found; list_index++) {
					if (::lstrcmpi (list[list_index], psub_obj->Get_Name ()) == 0) {
						found = true;
					}
				}

				// Add this emitter to the list if necessary
				if (!found) {
					list.Add (psub_obj->Get_Name ());
				}
			}

			// Recursivly add emitters to the list
			Build_Emitter_List (*psub_obj, list);
			MEMBER_RELEASE (psub_obj);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Is_Aggregate
//
bool
Is_Aggregate (const char *asset_name)
{
	// Assume that the asset isn't an aggregate
	bool retval = false;

	// Check to see if this object is an aggregate
	RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (asset_name);
	if ((prender_obj != NULL) &&
		 (prender_obj->Get_Base_Model_Name () != NULL))
	{
		retval = true;
	}

	// Free our hold on the temporary render object
	MEMBER_RELEASE (prender_obj);
	
	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Rename_Aggregate_Prototype
//
void
Rename_Aggregate_Prototype
(
	const char *old_name,
	const char *new_name
)
{
	// Params valid?
	if ((old_name != NULL) &&
		 (new_name != NULL) &&
		 (::lstrcmpi (old_name, new_name) != 0)) {

		// Get the prototype from the asset manager
		AggregatePrototypeClass *proto = NULL;
		proto = (AggregatePrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (old_name);
		if (proto != NULL) {

			// Copy the definition from the prototype and remove the prototype
			AggregateDefClass *pdefinition = proto->Get_Definition ();
			AggregateDefClass *pnew_definition = pdefinition->Clone ();
			WW3DAssetManager::Get_Instance ()->Remove_Prototype (old_name);

			// Rename the definition, create a new prototype, and add it to the asset manager
			pnew_definition->Set_Name (new_name);
			proto = new AggregatePrototypeClass (pnew_definition);
			WW3DAssetManager::Get_Instance ()->Add_Prototype (proto);
		}		
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Is_Real_LOD
//
bool
Is_Real_LOD (const char *asset_name)
{
	// Assume that the asset isn't a true LOD (HLOD w/ more than one 
	bool retval = false;

	// Check to see if this object is an aggregate
	RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj (asset_name);
	if ((prender_obj != NULL) &&
		 (prender_obj->Class_ID () == RenderObjClass::CLASSID_HLOD) &&
		 (((HLodClass *)prender_obj)->Get_LOD_Count () > 1)) {
		retval = true;
	}

	// Free our hold on the temporary render object
	MEMBER_RELEASE (prender_obj);
	
	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_File_Time
//
bool
Get_File_Time
(
	LPCTSTR path,
	LPFILETIME pcreation_time,
	LPFILETIME paccess_time,
	LPFILETIME pwrite_time
)
{
	// Assume failure
	bool retval = false;

	// Attempt to open the file
	HANDLE hfile = ::CreateFile (path,
										  0,
										  0,
										  NULL,
										  OPEN_EXISTING,
										  0L,
										  NULL);
	
	ASSERT (hfile != INVALID_HANDLE_VALUE);
	if (hfile != INVALID_HANDLE_VALUE) {

		// Get the mod times for this file
		retval = (::GetFileTime (hfile, pcreation_time, paccess_time, pwrite_time) == TRUE);

		// Close the file
		SAFE_CLOSE (hfile);
	}		
	
	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Are_Glide_Drivers_Acceptable
//
bool
Are_Glide_Drivers_Acceptable (void)
{
	// Assume success
	bool retval = true;

	// Is this windows NT?
	OSVERSIONINFO version = { sizeof (OSVERSIONINFO), 0 };
	if (::GetVersionEx (&version) && (version.dwPlatformId == VER_PLATFORM_WIN32_NT)) {
		
		// Now assume failure
		retval = false;

		// Get a path to the system directory
		TCHAR path[MAX_PATH];
		::GetSystemDirectory (path, sizeof (path));
		::Delimit_Path (path);

		// Build the full path of the 2 main drivers
		CString glide2x = CString (path) + "glide2x.dll";
		CString glide3x = CString (path) + "glide3x.dll";

		// Get the creation time of the glide2x driver
		FILETIME file_time = { 0 };
		if (::Get_File_Time (glide2x, NULL, NULL, &file_time)) {
			CTime time_obj (file_time);
			retval = ((time_obj.GetYear () == 1998) && (time_obj.GetMonth () == 12)) || (time_obj.GetYear () > 1998);
		}

		// Get the creation time of the glide3x driver
		if (::Get_File_Time (glide3x, NULL, NULL, &file_time)) {
			CTime time_obj (file_time);
			retval = ((time_obj.GetYear () == 1998) && (time_obj.GetMonth () == 12)) || (time_obj.GetYear () > 1998);
		}
	}
	
	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_RC_Texture
//
TextureClass *
Load_RC_Texture (LPCTSTR resource_name)
{
	TextureClass *texture = NULL;

	//
	//	Load the cursor file image from this binaries resources
	//
	ResourceFileClass resource_file (::AfxGetResourceHandle (), resource_name);
	unsigned char *res_data = resource_file.Peek_Data ();
	unsigned int data_size = resource_file.Size ();

	//
	//	Create a texture from the raw image data
	//

	// TheSuperHackers @info Not implemented
	
	// Reutrn a pointer to the new texture
	return texture;
}


////////////////////////////////////////////////////////////////////////////
//
//  Resolve_Path
//
////////////////////////////////////////////////////////////////////////////
void
Resolve_Path (CString &filename)
{
	if (filename.Find ('\\') == -1) {
		char path[MAX_PATH];
		::GetCurrentDirectory (MAX_PATH, path);
		::Delimit_Path (path);
		filename = CString (path) + filename;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Find_Missing_Textures
//
////////////////////////////////////////////////////////////////////////////
void
Find_Missing_Textures
(
	DynamicVectorClass<CString> &	list,
	LPCTSTR								name,
	int									frame_count
)
{
	//
	//	If this file doesn't exist, then add it to our list
	//
	if (::GetFileAttributes (name) == 0xFFFFFFFF) {
		CString full_path = name;
		Resolve_Path (full_path);
		list.Add (full_path);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Copy_File
//
////////////////////////////////////////////////////////////////////////////
bool
Copy_File
(
	LPCTSTR	existing_filename,
	LPCTSTR	new_filename,
	bool		force_copy
)
{
	SANITY_CHECK ((existing_filename != NULL && new_filename != NULL)) {
		return false;
	}

	// Assume failure
	bool retval = false;

	// Make sure we aren't copying over ourselves
	bool allow_copy = (::lstrcmpi (existing_filename, new_filename) != 0);
	
	// Strip the readonly bit off if necessary
	DWORD attributes = ::GetFileAttributes (new_filename);
	if (allow_copy &&
		 (attributes != 0xFFFFFFFF) &&
		 ((attributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY))
	{
		if (force_copy) {
			::SetFileAttributes (new_filename, attributes & (~FILE_ATTRIBUTE_READONLY));
		} else {
			allow_copy = false;
		}
	}

	// Perform the copy operation!
	if (allow_copy) {
		retval = (::CopyFile (existing_filename, new_filename, FALSE) == TRUE);
	}

	// Return the true/false result code
	return retval;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Graphic_View
//
////////////////////////////////////////////////////////////////////////////
CGraphicView *
Get_Graphic_View (void)
{
	CGraphicView *view = NULL;

	//
	//	Get the view from the current document
	//
	CW3DViewDoc *doc = GetCurrentDocument ();
	if (doc != NULL) {
		view = doc->GetGraphicView ();
	}

	return view;
}
