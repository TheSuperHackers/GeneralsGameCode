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

// EmitterUserPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "EmitterUserPropPage.h"
#include "w3d_file.h"
#include "part_emt.h"
#include "EmitterInstanceList.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterUserPropPageClass property page

IMPLEMENT_DYNCREATE(EmitterUserPropPageClass, CPropertyPage)

/////////////////////////////////////////////////////////////
//
//  EmitterUserPropPageClass
//
EmitterUserPropPageClass::EmitterUserPropPageClass (EmitterInstanceListClass *pemitter)
	: m_pEmitterList (NULL),
	  m_bValid (true),
	  m_iType (EMITTER_TYPEID_DEFAULT),
	  CPropertyPage(EmitterUserPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(EmitterUserPropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  ~EmitterUserPropPageClass
//
EmitterUserPropPageClass::~EmitterUserPropPageClass (void)
{
	return;
}


/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
EmitterUserPropPageClass::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterUserPropPageClass)
	DDX_Control(pDX, IDC_TYPE_COMBO, m_TypeCombo);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EmitterUserPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterUserPropPageClass)
	ON_EN_CHANGE(IDC_PROGRAMMER_SETTINGS_EDIT, OnChangeProgrammerSettingsEdit)
	ON_CBN_SELCHANGE(IDC_TYPE_COMBO, OnSelchangeTypeCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
void
EmitterUserPropPageClass::Initialize (void)
{
	if (m_pEmitterList != NULL) {
		
		// Record the user information from the emitter (if it exists)
		m_iType			= m_pEmitterList->Get_User_Type ();
		m_UserString	= m_pEmitterList->Get_User_String ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
EmitterUserPropPageClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();

	// Add the list of user-types to the combobox
	for (int index = 0; index < EMITTER_TYPEID_COUNT; index ++) {
		m_TypeCombo.AddString (EMITTER_TYPE_NAMES[index]);
	}

	// Select the correct entry in the combobox
	m_TypeCombo.SetCurSel (m_iType);
		
	// Fill in the user-box
	SetDlgItemText (IDC_PROGRAMMER_SETTINGS_EDIT, m_UserString);
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
BOOL
EmitterUserPropPageClass::OnApply (void)
{
	// Get the settings from the controls
	m_iType = m_TypeCombo.GetCurSel ();
	GetDlgItemText (IDC_PROGRAMMER_SETTINGS_EDIT, m_UserString);

	//
	//	Pass the new settings onto the emitter
	//
	m_pEmitterList->Set_User_Type (m_iType);
	m_pEmitterList->Set_User_String (m_UserString);

	// Allow the base class to process this message
	return CPropertyPage::OnApply ();
}


/////////////////////////////////////////////////////////////
//
//  OnChangeProgrammerSettingsEdit
//
void
EmitterUserPropPageClass::OnChangeProgrammerSettingsEdit (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnSelchangeTypeCombo
//
void
EmitterUserPropPageClass::OnSelchangeTypeCombo (void)
{
	SetModified ();
	return ;
}
