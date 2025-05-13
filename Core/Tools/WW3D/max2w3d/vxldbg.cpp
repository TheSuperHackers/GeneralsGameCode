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

/* $Header: /Commando/Code/Tools/max2w3d/vxldbg.cpp 3     10/28/97 6:08p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D Engine                                       * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/vxldbg.cpp                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/14/97 3:07p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 3                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "vxldbg.h"
#include "resource.h"
#include "dllmain.h"

/*
** Static functions
*/
static BOOL CALLBACK		_dialog_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

static PaletteClass _VoxelPalette;

VoxelDebugWindowClass::VoxelDebugWindowClass(VoxelClass * vxl) :
	CurLayer(0),
	Bitmap(NULL),
	Voxel(vxl),
	WindowHWND(0),
	ViewportHWND(0),
	LayerSpin(NULL)
{
	_VoxelPalette[0] = RGBClass(0,0,0);
	_VoxelPalette[1] = RGBClass(128,255,128);
}

VoxelDebugWindowClass::~VoxelDebugWindowClass(void)
{
	ReleaseISpinner(LayerSpin);
}

void VoxelDebugWindowClass::Display_Window(void) 
{
	DialogBoxParam
						(
							AppInstance,
							MAKEINTRESOURCE (IDD_VOXEL_DEBUG_DIALOG),
							NULL,
							(DLGPROC) _dialog_proc,
							(LPARAM) this
						);
}


bool VoxelDebugWindowClass::Dialog_Proc
(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM 
)
{
	RECT crect;

	switch (message )	{

		/*******************************************************************
		* WM_INITDIALOG
		*
		* Initialize all of the custom controls for the dialog box
		*
		*******************************************************************/
		case WM_INITDIALOG:

			// keep a handle to the window
			WindowHWND = hWnd;

			// keep a handle to the control that I'm using for the voxel viewport.
			ViewportHWND = GetDlgItem(WindowHWND,IDC_VOXEL_VIEWPORT);

			// create a DIB that will be drawn on top of the VOXEL_VIEWPORT control.
			GetClientRect(ViewportHWND,&crect);
			Bitmap = new SimpleDIBClass(ViewportHWND,Voxel->Get_Width(),Voxel->Get_Height(),_VoxelPalette);

			// initialize the layer spinner
			LayerSpin = SetupIntSpinner
			(
				hWnd,
				IDC_LAYER_SPIN,
				IDC_LAYER_EDIT,
				0,
				Voxel->Num_Layers(),
				0
			);

			update_display();

			SetCursor(LoadCursor (NULL, IDC_ARROW));

			return 1;


		/*******************************************************************
		* WM_COMMAND
		*
		*
		*******************************************************************/
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:

					// done!
					SetCursor(LoadCursor (NULL, IDC_WAIT));
					EndDialog(hWnd, 1);
					break;
			}
			return 1;

		/*******************************************************************
		* CC_SPINNER_CHANGE
		*
		* Max custom spinner controls
		*
		*******************************************************************/
		case CC_SPINNER_CHANGE:

			switch (LOWORD(wParam) )
			{
				case IDC_LAYER_SPIN:

					CurLayer = LayerSpin->GetIVal();
					update_display();
					break;
			}

			
		/*******************************************************************
		* WM_PAINT
		*
		*
		*******************************************************************/
		case WM_PAINT:

			update_display();

			GetClientRect(ViewportHWND,&crect);
			ValidateRect(ViewportHWND,&crect);
			
			break;
			
	}
	return 0;
}


void VoxelDebugWindowClass::update_display(void)
{
	int i,j;
	
	/*
	** Bail out if everything isn't right
	*/
	if ((Bitmap == NULL) || (Voxel == NULL)) {
		return;
	}

	/*
	** Update the contents of the DIB based on 
	** the contents of the current voxel layer.
	*/

	Bitmap->Clear(0);

	for (j=0; j < Voxel->Get_Height(); j++) {
		for (i=0; i < Voxel->Get_Width(); i++) {
			if (Voxel->Is_Solid(i,j,CurLayer)) {
				Bitmap->Set_Pixel(i,j,1);
			}
		}
	}

	/*
	** Blit the DIB onto the dialog box.
	*/
	HDC			hdcwindow = GetDC(ViewportHWND);
	HDC			hdcdib = CreateCompatibleDC(hdcwindow);
	HBITMAP		holdbitmap = (HBITMAP)SelectObject(hdcdib, Bitmap->Get_Handle());
	RECT			crect;

	GetClientRect(ViewportHWND,&crect);
	int cx = (crect.right - crect.left) / 2;
	int cy = (crect.bottom - crect.top) / 2;
	int x0 = 0; //cx - Bitmap->Get_Width();
	int y0 = 0; //cy - Bitmap->Get_Height();
	int x1 = 2 * Bitmap->Get_Width(); //cx + Bitmap->Get_Width();
	int y1 = 2 * Bitmap->Get_Height(); //cy + Bitmap->Get_Height();

//	BitBlt(hdcwindow,0,0,Bitmap->Get_Width(),Bitmap->Get_Height(),hdcdib,0,0,SRCCOPY);
	StretchBlt(		hdcwindow, x0, y0, x1, y1, 
						hdcdib, 0, 0, Bitmap->Get_Width(), Bitmap->Get_Height(), SRCCOPY);

	SelectObject(hdcdib, holdbitmap);

	ReleaseDC(WindowHWND, hdcwindow);
	DeleteDC(hdcdib);
}


BOOL CALLBACK _dialog_proc
(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	static VoxelDebugWindowClass * window = NULL;

	if (message == WM_INITDIALOG) {
		window = (VoxelDebugWindowClass *) lParam;
	}

	if (window) {
		return window->Dialog_Proc(hWnd, message, wParam, lParam);
	} else {
		return FALSE;
	}
}

