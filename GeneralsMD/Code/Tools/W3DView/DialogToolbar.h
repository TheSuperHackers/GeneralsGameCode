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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : LevelEdit                                                    *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/DialogToolbar.h                $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 2/23/99 10:58a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif


#ifndef __DIALOGTOOLBAR_H
#define __DIALOGTOOLBAR_H

#include "Resource.H"

/////////////////////////////////////////////////////////////////
//
//	DialogToolbarClass
//
class DialogToolbarClass : public CToolBar
{
	public:
		DialogToolbarClass (void);
		virtual ~DialogToolbarClass (void) {}

// Form Data
public:
	//{{AFX_DATA(DialogToolbarClass)
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogToolbarClass)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(DialogToolbarClass)
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnInitialUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	BOOL OnNeedToolTipText (UINT id, NMHDR *pTTTStruct, LRESULT *pResult);



	public:
		
		///////////////////////////////////////////////////////
		//
		//	Public methods
		//
		void						Enable_Button (int id, bool benable = true);
};


#endif //__DIALOGTOOLBAR_H

