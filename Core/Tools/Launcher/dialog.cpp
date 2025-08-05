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

//
// Create the dialog used during the patching process.
//
#include"winblows.h"
#include"resource.h"
#include"loadbmp.h"
#include<commctrl.h>

HWND PatchDialog;
BOOL CALLBACK Patch_Window_Proc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

HWND Create_Patch_Dialog(void)
{
  PatchDialog=CreateDialog(Global_instance, MAKEINTRESOURCE(IDD_PATCHPROGRESS),
    NULL, (DLGPROC)Patch_Window_Proc);

  ShowWindow(PatchDialog, SW_NORMAL);
  SetForegroundWindow(PatchDialog);
  return(PatchDialog);
}

BOOL CALLBACK Patch_Window_Proc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

  static LoadBmp bmpLoader;

  switch(iMsg) {
    case WM_INITDIALOG:
      // progress bar
      SendMessage(GetDlgItem(hwnd,IDC_PROGRESS2),PBM_SETRANGE,
        0,MAKELPARAM(0,100));
      SendMessage(GetDlgItem(hwnd,IDC_PROGRESS2),PBM_SETPOS,0,0);
      SendMessage(GetDlgItem(hwnd,IDC_PROGRESS2),PBM_SETSTEP,10,0);

      bmpLoader.init("launcher.bmp",GetDlgItem(hwnd,IDC_SPLASH));
      return(TRUE);   // True means windows handles focus issues
    break;
    case WM_PAINT:
      bmpLoader.drawBmp();
    break;
    case WM_COMMAND:
      /* May want to add cancel later
      switch(wParam) {
        case IDCANCEL:
        {
          // do some stuff
          return(TRUE);
        }
        default:
        break;
      }
      default:
      *************/
    break;
    case WM_CLOSE:
      DestroyWindow(hwnd);
      PostQuitMessage(0);
      exit(0);
    break;
  }
  return(FALSE);
}
