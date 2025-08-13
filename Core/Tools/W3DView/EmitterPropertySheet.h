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

#if !defined(AFX_EMITTERPROPERTYSHEET_H__83A8B844_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
#define AFX_EMITTERPROPERTYSHEET_H__83A8B844_BA3B_11D2_9FFA_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmitterPropertySheet.h : header file
//

#include "EmitterColorPropPage.h"
#include "EmitterGeneralPropPage.h"
#include "EmitterParticlePropPage.h"
#include "EmitterPhysicsPropPage.h"
#include "EmitterSizePropPage.h"
#include "EmitterUserPropPage.h"
#include "EmitterLinePropPage.h"
#include "EmitterRotationPropPage.h"
#include "EmitterFramePropPage.h"
#include "EmitterLineGroupPropPage.h"

// Forward declarations
class ParticleEmitterClass;
class EmitterInstanceListClass;
class AssetInfoClass;


/////////////////////////////////////////////////////////////////////////////
//
//	EmitterPropertySheetClass
//
class EmitterPropertySheetClass : public CPropertySheet
{
	DECLARE_DYNAMIC(EmitterPropertySheetClass)

// Construction
public:
	EmitterPropertySheetClass (EmitterInstanceListClass *emitter_list, UINT nIDCaption, CWnd* pParentWnd = NULL);
	EmitterPropertySheetClass (EmitterInstanceListClass *emitter_list, LPCTSTR pszCaption, CWnd* pParentWnd = NULL);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EmitterPropertySheetClass)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~EmitterPropertySheetClass();

	// Generated message map functions
protected:
	//{{AFX_MSG(EmitterPropertySheetClass)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		//////////////////////////////////////////////////////////////////////
		//
		//	Public methods
		//
		void							Notify_Render_Mode_Changed(int new_mode);

	protected:

		//////////////////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void							Initialize (void);
		ParticleEmitterClass *	Create_Emitter (void);
		void							Update_Emitter (void);
		void							Add_Emitter_To_Viewer (void);
		void							Create_New_Emitter (void);

	private:

		//////////////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		EmitterGeneralPropPageClass		m_GeneralPage;
		EmitterParticlePropPageClass		m_ParticlePage;
		EmitterPhysicsPropPageClass		m_PhysicsPage;
		EmitterColorPropPageClass			m_ColorPage;
		EmitterUserPropPageClass			m_UserPage;
		EmitterSizePropPageClass			m_SizePage;
		EmitterLinePropPageClass			m_LinePage;
		EmitterRotationPropPageClass		m_RotationPage;
		EmitterFramePropPageClass			m_FramePage;
		EmitterLineGroupPropPageClass		m_LineGroupPage;

		EmitterInstanceListClass *			m_pEmitterList;
		CString									m_LastSavedName;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERPROPERTYSHEET_H__83A8B844_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
