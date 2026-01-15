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
 *                     $Archive:: /Commando/Code/Tools/W3DView/SpherePropertySheet.h          $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 3/10/00 7:02p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "SphereColorPropPage.h"
#include "SphereGeneralPropPage.h"
#include "SphereSizePropPage.h"


// Forward declarations
class ParticleEmitterClass;
class EmitterInstanceListClass;
class AssetInfoClass;


/////////////////////////////////////////////////////////////////////////////
//
// SpherePropertySheetClass
//
/////////////////////////////////////////////////////////////////////////////
class SpherePropertySheetClass : public CPropertySheet
{
	DECLARE_DYNAMIC(SpherePropertySheetClass)

// Construction
public:
	SpherePropertySheetClass (SphereRenderObjClass *sphere, UINT nIDCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);
	SpherePropertySheetClass (SphereRenderObjClass *sphere, LPCTSTR pszCaption, CWnd* pParentWnd = nullptr, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SpherePropertySheetClass)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SpherePropertySheetClass();

	// Generated message map functions
protected:
	//{{AFX_MSG(SpherePropertySheetClass)
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
	void							Initialize (void);
	SphereRenderObjClass *	Create_Object (void);
	void							Update_Object (void);
	void							Add_Object_To_Viewer (void);
	void							Create_New_Object (void);

private:

	//////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////
	SphereGeneralPropPageClass		m_GeneralPage;
	SphereColorPropPageClass		m_ColorPage;
	SphereSizePropPageClass			m_ScalePage;
	SphereRenderObjClass *			m_RenderObj;
	CString								m_LastSavedName;
};

/////////////////////////////////////////////////////////////////////////////
