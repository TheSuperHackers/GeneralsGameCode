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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "winblows.h"


HINSTANCE Global_instance;
LPSTR     Global_commandline;
int       Global_commandshow;


/*
 * WinMain - initialization, message loop
 */
int PASCAL WinMain( HINSTANCE instance, HINSTANCE, char *command_line, int command_show)
{
    //////MSG         msg;

    Global_instance = instance;
    Global_commandline = command_line;
    Global_commandshow = command_show;

    int         argc;
    char       *argv[64];

    char        path_to_exe[512];
    GetModuleFileName(instance,(char *)&path_to_exe,512);
    argc=1;
    argv[0]=path_to_exe;


    int   command_scan=0;
    char  command_char;
    do 
    {
      /*
      ** Scan for non-space character on command line
      */
      do 
      {
        command_char = *( command_line+command_scan++ );
      } while ( command_char==' ' );

      if ( command_char!=0 && command_char != 13 )
      {
        argv[argc++]=command_line+command_scan-1;

        /*
        ** Scan for space character on command line
        */
        do
        {
          command_char = *( command_line+command_scan++ );
        } while ( command_char!=' ' && command_char != 0 && command_char!=13);
                *( command_line+command_scan-1 ) = 0;
      }

    } while ( command_char != 0 && command_char != 13 && argc<20 );

    return(main(argc,argv));

} /* WinMain */


int Print_WM(UINT message,char *out)
{
  switch(message)
  {
    case WM_NULL:
      sprintf(out,"WM_NULL");
      break;
    case WM_CREATE:
      sprintf(out,"WM_CREATE");
      break;
    case WM_DESTROY:
      sprintf(out,"WM_DESTROY");
      break;
    case WM_CANCELMODE:
      sprintf(out,"WM_CANCELMODE");
      break;
    case WM_ERASEBKGND:
      sprintf(out,"WM_ERASEBKGND");
      break;
    case WM_GETTEXT:
      sprintf(out,"WM_GETTEXT");
      break;
    case WM_QUERYOPEN:
      sprintf(out,"WM_QUERYOPEN");
      break;
    case WM_MOVE:
      sprintf(out,"WM_MOVE");
      break;
    case WM_SIZE:
      sprintf(out,"WM_SIZE");
      break;
    case WM_ACTIVATE:
      sprintf(out,"WM_ACTIVATE");
      break;
    case WM_SETFOCUS:
      sprintf(out,"WM_SETFOCUS");
      break;
    case WM_KILLFOCUS:
      sprintf(out,"WM_KILLFOCUS");
      break;
    case WM_ENABLE:
      sprintf(out,"WM_ENABLE");
      break;
    case WM_SETREDRAW:
      sprintf(out,"WM_REDRAW");
      break;
    case WM_PAINT:
      sprintf(out,"WM_PAINT");
      break;
    case WM_CLOSE:
      sprintf(out,"WM_CLOSE");
      break;
    case WM_QUIT:
      sprintf(out,"WM_QUIT");
      break;
    case WM_ACTIVATEAPP:
      sprintf(out,"WM_ACTIVATEAPP");
      break;
    case WM_SETCURSOR:
      sprintf(out,"WM_SETCURSOR");
      break;
    case WM_KEYDOWN:
      sprintf(out,"WM_KEYDOWN");
      break;
    case WM_MOUSEMOVE:
      sprintf(out,"WM_MOUSEMOVE");
      break;
    case WM_WINDOWPOSCHANGING:
      sprintf(out,"WM_WINDOWPOSCHANGING");
      break;
    case WM_WINDOWPOSCHANGED:
      sprintf(out,"WM_WINDOWPOSCHANGED");
      break;
    case WM_DISPLAYCHANGE:
      sprintf(out,"WM_DISPLAYCHANGE");
      break;
    case WM_NCPAINT:
      sprintf(out,"WM_NCPAINT");
      break;
    case WM_PALETTEISCHANGING:
      sprintf(out,"WM_PALETTEISCHANGING");
      break;
    case WM_PALETTECHANGED:
      sprintf(out,"WM_PALETTECHANGED");
      break;
    case WM_NCACTIVATE:
      sprintf(out,"WM_NCACTIVATE");
      break;
    case WM_NCCALCSIZE:
      sprintf(out,"WM_NCCALCSIZE");
      break;
    case WM_SYSCOMMAND:
      sprintf(out,"WM_SYSCOMMAND");
      break;
    default:
      sprintf(out,"? UNKNOWN ?");
      return(-1);
  }
  return(0);
}
