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
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/RingPropertySheet.h            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/14/00 10:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef __RING_PROPERTY_SHEET_H
#define __RING_PROPERTY_SHEET_H

#include "RingColorPropPage.h"
#include "RingGeneralPropPage.h"
#include "RingSizePropPage.h"


// Forward declarations
class ParticleEmitterClass;
class EmitterInstanceListClass;
class AssetInfoClass;


/////////////////////////////////////////////////////////////////////////////
//
// RingPropertySheetClass
//
/////////////////////////////////////////////////////////////////////////////
class RingPropertySheetClass : public CPropertySheet
{
	DECLARE_DYNAMIC(RingPropertySheetClass)

// Construction
public:
	RingPropertySheetClass (RingRenderObjClass *ring, UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	RingPropertySheetClass (RingRenderObjClass *ring, LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RingPropertySheetClass)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~RingPropertySheetClass();

	// Generated message map functions
protected:
	//{{AFX_MSG(RingPropertySheetClass)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////

protected:

	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	void						Initialize (void);
	RingRenderObjClass *	Create_Object (void);
	void						Update_Object (void);
	void						Add_Object_To_Viewer (void);
	void						Create_New_Object (void);

private:

	//////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////
	RingGeneralPropPageClass	m_GeneralPage;
	RingColorPropPageClass		m_ColorPage;
	RingSizePropPageClass		m_ScalePage;
	RingRenderObjClass *			m_RenderObj;
	CString							m_LastSavedName;
};

/////////////////////////////////////////////////////////////////////////////

#endif //__RING_PROPERTY_SHEET_H
