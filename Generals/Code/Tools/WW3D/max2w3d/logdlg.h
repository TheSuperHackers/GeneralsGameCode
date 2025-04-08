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

/* $Header: /Commando/Code/Tools/max2w3d/logdlg.h 4     11/07/00 5:40p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/logdlg.h                       $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 11/07/00 3:15p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 4                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef LOGDLG_H
#define LOGDLG_H

#include <windows.h>


class LogDataDialogClass
{
public:

	LogDataDialogClass(HWND parent);
	~LogDataDialogClass();
	
   void	Wait_OK();	// wait for user to hit OK
   
   void	printf(const char *, ...);
	void	printf(const char * text, va_list args);
	void  rprintf(const char *, ...);
	void	rprintf(const char *text, va_list args);
	
	void	updatebar(float position, float total);
   
	bool	Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);

public:
// public variables
	HWND		Hwnd;
	HWND		ParentHwnd;

private:

	void Dialog_Init();

private:

	HANDLE	ThreadHandle;
	DWORD		ThreadID;

	int	last_buffer_index;
	int	buffer_index;

volatile int status;
  
};


#endif

// EOF - logdlg.h
