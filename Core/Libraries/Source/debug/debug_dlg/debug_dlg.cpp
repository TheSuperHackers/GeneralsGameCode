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

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Source/debug/debug_dlg/debug_dlg.cpp $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Debug exception dialog test program
//////////////////////////////////////////////////////////////////////////////
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

// Pardon my coding here but this is for testing only...

BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
    case WM_INITDIALOG:
      {
        SendDlgItemMessage(hWnd,100,WM_SETTEXT,0,(LPARAM)"EXCEPTION_TYPE");
        SendDlgItemMessage(hWnd,101,WM_SETTEXT,0,(LPARAM)"Explains exception type further here...\n"
                                  "123456789012345678901234567890123456789012345678901234567890");
        SendDlgItemMessage(hWnd,102,WM_SETTEXT,0,(LPARAM)"Module/File/Line, Address");
        SendDlgItemMessage(hWnd,103,WM_SETTEXT,0,(LPARAM)"File version, build type");

        HWND list;
        list=GetDlgItem(hWnd,104);

        LVCOLUMN c;
        c.mask=LVCF_TEXT|LVCF_WIDTH;
        c.pszText="";
        c.cx=0;
        ListView_InsertColumn(list,0,&c);

        c.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
        c.pszText="Address";
        c.cx=60;
        c.fmt=LVCFMT_RIGHT;
        ListView_InsertColumn(list,1,&c);

        c.mask=LVCF_TEXT|LVCF_WIDTH;
        c.pszText="Module";
        c.cx=120;
        ListView_InsertColumn(list,2,&c);

        c.pszText="Symbol";
        c.cx=300;
        ListView_InsertColumn(list,3,&c);

        c.pszText="File";
        c.cx=130;
        ListView_InsertColumn(list,4,&c);

        c.pszText="Line";
        c.cx=80;
        ListView_InsertColumn(list,5,&c);

        LVITEM item;
        item.iItem=0;
        item.iSubItem=0;
        item.mask=0;

        item.iItem=ListView_InsertItem(list,&item);
        item.mask=LVIF_TEXT;

        item.iSubItem++;
        item.pszText="01234567";
        ListView_SetItem(list,&item);

        item.iSubItem++;
        item.pszText="MSVCRTD.dll+0xad38";
        ListView_SetItem(list,&item);

        item.iSubItem++;
        item.pszText="mainCRTStartupSuperLongSymbolOYeahThisIsCool+0xd23e0";
        ListView_SetItem(list,&item);

        item.iSubItem++;
        item.pszText="reallyreallyverylongfilename.cpp";
        ListView_SetItem(list,&item);

        item.iSubItem++;
        item.pszText="5748+0xad38";
        ListView_SetItem(list,&item);

        for (int k=1;k<30;k++)
        {
          item.iItem=k;
          item.iSubItem=0;
          item.iItem=ListView_InsertItem(list,&item);
          item.mask=LVIF_TEXT;

          item.iSubItem++;
          item.pszText="88888888";
          ListView_SetItem(list,&item);

          item.iSubItem++;
          item.pszText="MSVCRTD.dll+0xad38";
          ListView_SetItem(list,&item);

          item.iSubItem++;
          item.pszText="Debug::DebugException::Symbol+0xd23e0";
          ListView_SetItem(list,&item);

          item.iSubItem++;
          item.pszText="regularfilename.cpp";
          ListView_SetItem(list,&item);

          item.iSubItem++;
          item.pszText="5748+0x38";
          ListView_SetItem(list,&item);
        }

        HFONT hf;
        hf=CreateFont(13,0,0,0,FW_NORMAL,
                      FALSE,FALSE,FALSE,ANSI_CHARSET,
                      OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY,FIXED_PITCH|FF_MODERN,NULL);
        SendDlgItemMessage(hWnd,105,WM_SETFONT,(WPARAM)hf,MAKELPARAM(TRUE,0));
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"EAX:0x00000666 EBX:0x7ffdf000 ECX:0x00000000");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"EDX:0x00422208 ESI:0x02100210 EDI:0x0012fec4");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"EIP:0x0040103d ESP:0x0012fe78 EBP:0x0012fec4");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"Flags:%00000000000000010000001000000110");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"CS:0x001b DS:0x0023 SS:0x0023");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ES:0x0023 FS:0x0038 GS:0x0000");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"CW:%0000001001111111");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"SW:%0000000000000000");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"TW:%1111111111111111");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ErrOfs:      0x00000000 ErrSel:  0x03020000");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"DataOfs:     0x00000000 DataSel: 0xffff0000");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"Cr0NpxState: 0x00000000");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(0) 10020203100210021002 -1.#IND00");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(1) 01031002010320020103 -1.#IND00");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(2) 10021002100210021002 -1.#IND00");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(3) 02031002020320020203 -1.#IND00");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(4) 10021002100210021002 -1.#IND00");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(5) 12031002100210021002 -1.#IND00");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(6) 14021402100212031002 -1.#IND00");
        SendDlgItemMessage(hWnd,105,LB_ADDSTRING,0,(LPARAM)"ST(7) 12031002100210021002 -1.#IND00");
      }
      return TRUE;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
        case IDOK:
          EndDialog(hWnd,IDOK);
          break;
      }
      return FALSE;
    default:
      return FALSE;
  }
}

int CALLBACK WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
  // show dialog box first
  InitCommonControls();
  DialogBox(hInst,MAKEINTRESOURCE(100),NULL,DialogProc);

  // write out resource data (if possible)
  FILE *f=fopen("..\\rc_exception.inl","wt");
  if (f)
  {
    fprintf(f,"static unsigned char rcException[]={ // program generated, do not edit\n");

    HRSRC h=FindResource(hInst,MAKEINTRESOURCE(100),RT_DIALOG);
    DWORD size=SizeofResource(hInst,h);

    unsigned char *data=(unsigned char *)LockResource(LoadResource(hInst,h));
    for (unsigned k=0;k<size;k+=8)
    {
      for (unsigned i=0;i<8;i++)
        fprintf(f,"0x%02x,",k+i<size?data[k+i]:0);
      fprintf(f,"\n");
    }

    fprintf(f,"0 };\n");
    fclose(f);
  }

  return 0;
}
