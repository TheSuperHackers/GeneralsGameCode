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
 *                 Project Name : max2w3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/FormClass.h                    $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/13/98 9:34a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>


class FormClass : public ParamDlg
{
	public:
		FormClass (void)
			: m_hWnd (nullptr) {}
		~FormClass (void) {}

		HWND						Create_Form (HWND parent_wnd, UINT template_id);
		void						Show (bool show_flag = true) { ::ShowWindow (m_hWnd, show_flag ? SW_SHOW : SW_HIDE); }
		virtual BOOL			Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam) = 0;
		HWND						Get_Hwnd(void) { return m_hWnd; }
		virtual void			Invalidate(void) { InvalidateRect(m_hWnd,nullptr,0); }

	protected:

		BOOL						ExecuteDlgInit(LPVOID lpResource);
		BOOL						ExecuteDlgInit(LPCTSTR lpszResourceName);

		static BOOL	WINAPI	fnFormProc (HWND dlg_wnd, UINT message, WPARAM wparam,  LPARAM lparam);

		HWND						m_hWnd;
		RECT						m_FormRect;
};
