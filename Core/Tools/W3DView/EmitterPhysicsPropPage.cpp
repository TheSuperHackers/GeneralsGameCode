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

// EmitterPhysicsPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "EmitterPhysicsPropPage.h"
#include "Utils.h"
#include "part_emt.h"
#include "VolumeRandomDialog.h"
#include "EmitterInstanceList.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterPhysicsPropPageClass property page

IMPLEMENT_DYNCREATE(EmitterPhysicsPropPageClass, CPropertyPage)

/////////////////////////////////////////////////////////////
//
//  EmitterPhysicsPropPageClass
//
/////////////////////////////////////////////////////////////
EmitterPhysicsPropPageClass::EmitterPhysicsPropPageClass (EmitterInstanceListClass *pemitter)
	: m_pEmitterList (NULL),
	  m_bValid (true),
	  m_Velocity (0, 0, 1),
	  m_Acceleration (0, 0, 0),
	  m_OutFactor (0),
	  m_InheritanceFactor (0),
	  m_Randomizer (NULL),
	  CPropertyPage(EmitterPhysicsPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(EmitterPhysicsPropPageClass)
	//}}AFX_DATA_INIT
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  ~EmitterPhysicsPropPageClass
//
/////////////////////////////////////////////////////////////
EmitterPhysicsPropPageClass::~EmitterPhysicsPropPageClass (void)
{
	SAFE_DELETE (m_Randomizer);
	return;
}


/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
/////////////////////////////////////////////////////////////
void
EmitterPhysicsPropPageClass::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterPhysicsPropPageClass)
	DDX_Control(pDX, IDC_OUT_FACTOR_SPIN, m_OutSpin);
	DDX_Control(pDX, IDC_INHERITANCE_FACTOR_SPIN, m_InheritanceSpin);
	DDX_Control(pDX, IDC_VELOCITY_Z_SPIN, m_VelocityZSpin);
	DDX_Control(pDX, IDC_VELOCITY_Y_SPIN, m_VelocityYSpin);
	DDX_Control(pDX, IDC_VELOCITY_X_SPIN, m_VelocityXSpin);
	DDX_Control(pDX, IDC_ACCELERATION_Z_SPIN, m_AccelZSpin);
	DDX_Control(pDX, IDC_ACCELERATION_Y_SPIN, m_AccelYSpin);
	DDX_Control(pDX, IDC_ACCELERATION_X_SPIN, m_AccelXSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EmitterPhysicsPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterPhysicsPropPageClass)
	ON_BN_CLICKED(IDC_SPECIFY_VELOCITY_RANDOM, OnSpecifyVelocityRandom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
EmitterPhysicsPropPageClass::Initialize (void)
{
	SAFE_DELETE (m_Randomizer);
	if (m_pEmitterList != NULL) {

		//
		// Get the emitter's settings
		//
		m_Velocity				= m_pEmitterList->Get_Velocity ();
		m_Acceleration			= m_pEmitterList->Get_Acceleration ();
		m_OutFactor				= m_pEmitterList->Get_Outward_Vel ();
		m_InheritanceFactor	= m_pEmitterList->Get_Vel_Inherit ();
		m_Randomizer			= m_pEmitterList->Get_Velocity_Random ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
/////////////////////////////////////////////////////////////
BOOL
EmitterPhysicsPropPageClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();

	//
	//	Setup the velocity controls
	//
	::Initialize_Spinner (m_OutSpin, m_OutFactor, -10000, 10000);
	::Initialize_Spinner (m_InheritanceSpin, m_InheritanceFactor, -10000, 10000);
	::Initialize_Spinner (m_VelocityXSpin, m_Velocity.X, -10000, 10000);
	::Initialize_Spinner (m_VelocityYSpin, m_Velocity.Y, -10000, 10000);
	::Initialize_Spinner (m_VelocityZSpin, m_Velocity.Z, -10000, 10000);

	//
	//	Setup the acceleration controls
	//
	::Initialize_Spinner (m_AccelXSpin, m_Acceleration.X, -10000, 10000);
	::Initialize_Spinner (m_AccelYSpin, m_Acceleration.Y, -10000, 10000);
	::Initialize_Spinner (m_AccelZSpin, m_Acceleration.Z, -10000, 10000);
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL
EmitterPhysicsPropPageClass::OnApply (void)
{
	//
	//	Read the velocity settings
	//
	m_Velocity.X = ::GetDlgItemFloat (m_hWnd, IDC_VELOCITY_X_EDIT);
	m_Velocity.Y = ::GetDlgItemFloat (m_hWnd, IDC_VELOCITY_Y_EDIT);
	m_Velocity.Z = ::GetDlgItemFloat (m_hWnd, IDC_VELOCITY_Z_EDIT);
	m_OutFactor = ::GetDlgItemFloat (m_hWnd, IDC_OUT_FACTOR_SPIN);
	m_InheritanceFactor = ::GetDlgItemFloat (m_hWnd, IDC_INHERITANCE_FACTOR_SPIN);

	//
	//	Read the acceleration settings
	//
	m_Acceleration.X = ::GetDlgItemFloat (m_hWnd, IDC_ACCELERATION_X_EDIT);
	m_Acceleration.Y = ::GetDlgItemFloat (m_hWnd, IDC_ACCELERATION_Y_EDIT);
	m_Acceleration.Z = ::GetDlgItemFloat (m_hWnd, IDC_ACCELERATION_Z_EDIT);

	// Allow the base class to process this message
	return CPropertyPage::OnApply ();
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL
EmitterPhysicsPropPageClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
)
{
	//
	//	Update the spinner control if necessary
	//
	NMHDR *pheader = (NMHDR *)lParam;
	if ((pheader != NULL) && (pheader->code == UDN_DELTAPOS)) {
		LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;
		::Update_Spinner_Buddy (pheader->hwndFrom, pupdown->iDelta);
		On_Setting_Changed (wParam);
	}

	// Allow the base class to process this message
	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  OnSpecifyCreationVolume
//
/////////////////////////////////////////////////////////////
void
EmitterPhysicsPropPageClass::OnSpecifyVelocityRandom (void)
{
	VolumeRandomDialogClass dialog (m_Randomizer, this);
	if (dialog.DoModal () == IDOK) {

		//
		//	Get the new randomizer from the dialog
		//
		SAFE_DELETE (m_Randomizer);
		m_Randomizer = dialog.Get_Randomizer ();
		if (m_Randomizer != NULL) {
			m_pEmitterList->Set_Velocity_Random (m_Randomizer->Clone ());
			SetModified ();
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnCommand
//
/////////////////////////////////////////////////////////////
BOOL
EmitterPhysicsPropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_VELOCITY_X_EDIT:
		case IDC_VELOCITY_Y_EDIT:
		case IDC_VELOCITY_Z_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);
				On_Setting_Changed (LOWORD (wParam));
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;

		case IDC_ACCELERATION_X_EDIT:
		case IDC_ACCELERATION_Y_EDIT:
		case IDC_ACCELERATION_Z_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);
				On_Setting_Changed (LOWORD (wParam));
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;

		case IDC_INHERITANCE_FACTOR_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);
				On_Setting_Changed (LOWORD (wParam));
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;

		case IDC_OUT_FACTOR_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);
				On_Setting_Changed (LOWORD (wParam));
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;
	}

	return CPropertyPage::OnCommand (wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  On_Setting_Changed
//
/////////////////////////////////////////////////////////////
void
EmitterPhysicsPropPageClass::On_Setting_Changed (UINT ctrl_id)
{
	switch (ctrl_id)
	{
		case IDC_VELOCITY_X_EDIT:
		case IDC_VELOCITY_Y_EDIT:
		case IDC_VELOCITY_Z_EDIT:
		case IDC_VELOCITY_X_SPIN:
		case IDC_VELOCITY_Y_SPIN:
		case IDC_VELOCITY_Z_SPIN:
		{
			Vector3 velocity;
			velocity.X = ::GetDlgItemFloat (m_hWnd, IDC_VELOCITY_X_EDIT);
			velocity.Y = ::GetDlgItemFloat (m_hWnd, IDC_VELOCITY_Y_EDIT);
			velocity.Z = ::GetDlgItemFloat (m_hWnd, IDC_VELOCITY_Z_EDIT);
			m_pEmitterList->Set_Velocity (velocity);
			SetModified ();
		}
		break;

		case IDC_ACCELERATION_X_EDIT:
		case IDC_ACCELERATION_Y_EDIT:
		case IDC_ACCELERATION_Z_EDIT:
		case IDC_ACCELERATION_X_SPIN:
		case IDC_ACCELERATION_Y_SPIN:
		case IDC_ACCELERATION_Z_SPIN:
		{
			Vector3 acceleration;
			acceleration.X = ::GetDlgItemFloat (m_hWnd, IDC_ACCELERATION_X_EDIT);
			acceleration.Y = ::GetDlgItemFloat (m_hWnd, IDC_ACCELERATION_Y_EDIT);
			acceleration.Z = ::GetDlgItemFloat (m_hWnd, IDC_ACCELERATION_Z_EDIT);
			m_pEmitterList->Set_Acceleration (acceleration);
			SetModified ();
		}
		break;

		case IDC_INHERITANCE_FACTOR_EDIT:
		case IDC_INHERITANCE_FACTOR_SPIN:
		{
			float value = ::GetDlgItemFloat (m_hWnd, IDC_INHERITANCE_FACTOR_EDIT);
			m_pEmitterList->Set_Vel_Inherit (value);
			SetModified ();
		}
		break;

		case IDC_OUT_FACTOR_EDIT:
		case IDC_OUT_FACTOR_SPIN:
		{
			float value = ::GetDlgItemFloat (m_hWnd, IDC_OUT_FACTOR_EDIT);
			m_pEmitterList->Set_Outward_Vel (value);
			SetModified ();
		}
		break;
	}

	return ;
}

