/*
**	Command & Conquer Generals(tm)
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

/* $Header: /Commando/Code/Tools/max2w3d/logdlg.cpp 5     11/07/00 5:40p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/logdlg.cpp                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/07/00 4:24p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 5                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "logdlg.h"
#include "resource.h"
#include "dllmain.h"
#include "w3dexp.h"
#include "util.h"
#include "rawfile.h"
#include "units.h"


/*
** Static functions
*/
static BOOL CALLBACK		_logdata_dialog_proc(HWND Hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static DWORD WINAPI		_logdata_thread_function(LPVOID log_obj_ptr);


/*********************************************************************************************** 
 * LogDataDialogClass::LogDataDialogClass -- constructor for the options dialog object         *  
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/09/2000 JGA  : Created.                                                                * 
 *=============================================================================================*/
LogDataDialogClass::LogDataDialogClass(HWND parent):
 Hwnd(NULL),
 ParentHwnd(parent),
 buffer_index(0),
 last_buffer_index(0),
 status(0)
{	
	ThreadHandle = CreateThread(NULL, 0, _logdata_thread_function, (LPVOID)this, 0, &ThreadID);

	if (ThreadHandle) {
		while (status == 0) {
			// sync, wait for init
		}
	}
}

LogDataDialogClass::~LogDataDialogClass(void)
{	
	status = 3;
	if (::IsWindow(Hwnd)) {
		SendMessage( Hwnd, WM_CLOSE, 0, 0 );
	}
}


/*********************************************************************************************** 
 * LogDataDialogClass::printf -- handles doing printfs into the current log window             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/11/2000 JGA  : Created.                                                                 * 
 *=============================================================================================*/
void LogDataDialogClass::printf(const char *text, ...)
{
	va_list arguments;
	va_start(arguments, text);
}	// printf

void LogDataDialogClass::printf(const char * text, const va_list & args)
{
	static char string_buffer[256];

	vsprintf(string_buffer, text, args);

	HWND ctrlHwnd = GetDlgItem(Hwnd, IDC_ANIM_LOG_RICHEDIT);

	SendMessage(ctrlHwnd, EM_SETSEL, -1, -1 );
	SendMessage(ctrlHwnd, EM_REPLACESEL, FALSE, (long)string_buffer);

	last_buffer_index = buffer_index;
	buffer_index+=strlen(string_buffer);

	//int min,max,pos;
	//GetScrollRange(ctrlHwnd, SB_VERT, &min, &max);
	//pos = GetScrollPos(ctrlHwnd, SB_VERT);

	//if (pos == max) {
		SendMessage(GetDlgItem(Hwnd,IDC_ANIM_LOG_RICHEDIT), EM_SCROLLCARET, 0, 0);
	//}


}

/*********************************************************************************************** 
 * LogDataDialogClass::rprintf -- replace last printf, with this new printf                    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/14/2000 JGA  : Created.                                                                 * 
 *=============================================================================================*/
void LogDataDialogClass::rprintf(const char *text, ...)
{
	va_list arguments;
	va_start(arguments, text);

	rprintf(text,arguments);
}

void LogDataDialogClass::rprintf(const char *text, const va_list & args)
{
	static char string_buffer[256];
	vsprintf(string_buffer, text, args);

	HWND ctrlHwnd = GetDlgItem(Hwnd, IDC_ANIM_LOG_RICHEDIT);

	SendMessage(ctrlHwnd, EM_SETSEL, last_buffer_index, buffer_index );
	SendMessage(ctrlHwnd, EM_REPLACESEL, FALSE, (long)string_buffer);

	buffer_index = strlen(string_buffer) + last_buffer_index;

	//int min,max,pos;
	//GetScrollRange(ctrlHwnd, SB_VERT, &min, &max);
	//pos = GetScrollPos(ctrlHwnd, SB_VERT);

	//SendMessage(GetDlgItem(Hwnd,IDC_ANIM_LOG_RICHEDIT), EM_SCROLLCARET, 0, 0);

}	// rprintf


/*********************************************************************************************** 
 * LogDataDialogClass::updatebar - send message to progress meter                              * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/14/2000 JGA  : Created.                                                                 * 
 *=============================================================================================*/
void	LogDataDialogClass::updatebar(float position, float total)
{
	int pos;

	pos = ((position / total) * 100.0f);

	HWND ctrlHwnd = GetDlgItem(Hwnd, IDC_ANIM_COMPRESS_PROGRESS);

	SendMessage(ctrlHwnd, PBM_SETPOS, pos, 0 );

}	// updatebar


/*********************************************************************************************** 
 * LogDataDialogClass::Wait_OK - Give user a chance to review log, then hit ok                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/14/2000 JGA  : Created.                                                                 * 
 *=============================================================================================*/
void LogDataDialogClass::Wait_OK()
{
	::EnableWindow(GetDlgItem(Hwnd,IDOK),TRUE);
	::SetForegroundWindow(Hwnd);

	while (status < 2) {
		// wait for the OK
	}

} // Wait_OK



/*********************************************************************************************** 
 * LogDataDialogClass::Dialog_Proc -- Handles the windows message for the options dialog    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool LogDataDialogClass::Dialog_Proc
(
	HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM 
)
{
	int code = HIWORD(wParam);

	switch (message )	{

		/*******************************************************************
		* WM_INITDIALOG
		*
		* Initialize all of the custom controls for the dialog box
		*
		*******************************************************************/
		case WM_INITDIALOG:

			Dialog_Init();
			return TRUE;


		/*******************************************************************
		* WM_COMMAND
		*
		*
		*******************************************************************/
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:

					status = 2;

					EndDialog(Hwnd, 1);
					Hwnd = NULL;
					return TRUE;
					break;

			}
			break;

		//case WM_VSCROLL:
		//	return TRUE;
		//	break;

		case WM_CLOSE:

			if (status >= 2) {
				EndDialog(Hwnd, 1);
				Hwnd = NULL;
			}
				
			return TRUE;
			break;

	}
	return FALSE; 
   
}	// Dialog_Proc

void LogDataDialogClass::Dialog_Init()
{

	SetCursor(LoadCursor (NULL, IDC_ARROW));

	RECT desktop;
	RECT ourwin;

	GetWindowRect(GetDesktopWindow(), &desktop);
	GetWindowRect(Hwnd, &ourwin);

	int sx,sy,cx,cy;

	sx = ourwin.right   - ourwin.left;
	sy = ourwin.bottom - ourwin.top;

	cx = (((desktop.right - desktop.left) - sx)/2) + desktop.left;
	cy = (((desktop.bottom - desktop.top) - sy)/2) + desktop.top;
 
	//SetWindowPos(Hwnd, HWND_TOPMOST, cx, cy, 0, 0, SWP_NOSIZE);
	SetWindowPos(Hwnd, HWND_TOP, cx, cy, 0, 0, SWP_NOSIZE);
 
	EnableWindow(GetDlgItem(Hwnd,IDOK),FALSE);

	status = 1;	// signal init

 
}	// Dialog_Init



/*********************************************************************************************** 
 * _logdata_dialog_proc -- thunks into the logdata dialog class's windows message handler      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/09/2000 JGA  : Created.                                                                 * 
 *=============================================================================================*/
BOOL CALLBACK _logdata_dialog_proc
(
	HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (message == WM_INITDIALOG) {
		LogDataDialogClass * log = (LogDataDialogClass *)lParam;
		log->Hwnd = hwnd;
		::SetProp(hwnd,"LogDataDialogClass",(HANDLE)log);
	}

	LogDataDialogClass * log = (LogDataDialogClass *)::GetProp(hwnd,"LogDataDialogClass");

	if (message == WM_DESTROY) {
		::RemoveProp(hwnd,"LogDataDialogClass");
	}

	if (log) {
		return log->Dialog_Proc(hwnd,message,wParam,lParam);
	} else {
		return FALSE;
	}

} // _logdata_dialog_proc


DWORD WINAPI _logdata_thread_function(LPVOID log_obj_ptr)
{
	// put logdata dialog box (lpParameter is the "this" pointer of the object)
	DialogBoxParam( AppInstance,
   					 MAKEINTRESOURCE(IDD_W3D_LOG),
						 ((LogDataDialogClass*)log_obj_ptr)->ParentHwnd,
                   (DLGPROC) _logdata_dialog_proc,
                   (LPARAM) log_obj_ptr);
 

	// When this exits it should terminate the thread
	return(0);
}


// EOF - logdlg.cpp
