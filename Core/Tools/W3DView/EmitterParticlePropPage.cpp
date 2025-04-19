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

// EmitterParticlePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "EmitterParticlePropPage.h"
#include "Part_Emt.H"
#include "Utils.H"
#include "Vector3RndCombo.H"
#include "VolumeRandomDialog.H"
#include "EmitterInstanceList.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// EmitterParticlePropPageClass property page

IMPLEMENT_DYNCREATE(EmitterParticlePropPageClass, CPropertyPage)

/////////////////////////////////////////////////////////////
//
//  EmitterParticlePropPageClass
//
EmitterParticlePropPageClass::EmitterParticlePropPageClass (EmitterInstanceListClass *pemitter)
	: m_pEmitterList (pemitter),
	  m_bValid (true),
	  m_Rate (0),
	  m_BurstSize (0),
	  m_MaxParticles (0),
	  m_Randomizer (NULL),
	  CPropertyPage(EmitterParticlePropPageClass::IDD)
{
	//{{AFX_DATA_INIT(EmitterParticlePropPageClass)
	//}}AFX_DATA_INIT
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  ~EmitterParticlePropPageClass
//
EmitterParticlePropPageClass::~EmitterParticlePropPageClass (void)
{
	SAFE_DELETE (m_Randomizer);
	return;
}


/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
EmitterParticlePropPageClass::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterParticlePropPageClass)
	DDX_Control(pDX, IDC_BURST_SIZE_SPIN, m_BurstSizeSpin);
	DDX_Control(pDX, IDC_EMISSION_RATE_SPIN, m_EmitionRateSpin);
	DDX_Control(pDX, IDC_MAX_PARTICLES_SPIN, m_MaxParticlesSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EmitterParticlePropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterParticlePropPageClass)
	ON_BN_CLICKED(IDC_SPECIFY_CREATION_VOLUME, OnSpecifyCreationVolume)
	ON_BN_CLICKED(IDC_MAX_PARTICLES_CHECK, OnMaxParticlesCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
void
EmitterParticlePropPageClass::Initialize (void)
{
	SAFE_DELETE (m_Randomizer);
	if (m_pEmitterList != NULL) {

		//
		// Read the settings from the emitter
		//
		m_Rate			= m_pEmitterList->Get_Emission_Rate ();		
		m_BurstSize		= m_pEmitterList->Get_Burst_Size ();
		m_MaxParticles	= m_pEmitterList->Get_Max_Emissions ();
		m_Randomizer	= m_pEmitterList->Get_Creation_Volume ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
EmitterParticlePropPageClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();

	//
	//	Setup the burst controls
	//
	m_BurstSizeSpin.SetRange (0, 10000);
	m_BurstSizeSpin.SetPos (m_BurstSize);
	::Initialize_Spinner (m_EmitionRateSpin, m_Rate, -10000, 10000);
	
	//
	//	Setup the max particles spin
	//
	m_MaxParticlesSpin.SetRange (0, 10000);
	m_MaxParticlesSpin.SetPos (m_MaxParticles);
	SendDlgItemMessage (IDC_MAX_PARTICLES_CHECK, BM_SETCHECK, WPARAM(m_MaxParticles != 0));
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_MAX_PARTICLES_EDIT), m_MaxParticles != 0);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_MAX_PARTICLES_SPIN), m_MaxParticles != 0);
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
BOOL
EmitterParticlePropPageClass::OnApply (void)
{
	//
	//	Get the data from the controls
	//
	m_Rate			= ::GetDlgItemFloat (m_hWnd, IDC_EMISSION_RATE_EDIT);
	m_BurstSize		= GetDlgItemInt (IDC_BURST_SIZE_EDIT);
	
	//
	//	Determine if we need to cap the particles or not
	//
	m_MaxParticles	= 0;
	if (SendDlgItemMessage (IDC_MAX_PARTICLES_CHECK, BM_GETCHECK)) {
		m_MaxParticles	= GetDlgItemInt (IDC_MAX_PARTICLES_EDIT);
	}
	
	//
	//	Apply the changes to the emitter
	//
	m_pEmitterList->Set_Emission_Rate (m_Rate);
	m_pEmitterList->Set_Burst_Size (m_BurstSize);
	m_pEmitterList->Set_Max_Emissions (m_MaxParticles);
	m_pEmitterList->Set_Creation_Volume (m_Randomizer->Clone ());

	// Allow the base class to process this message
	return CPropertyPage::OnApply ();
}


/////////////////////////////////////////////////////////////
//
//  WindowProc
//
LRESULT
EmitterParticlePropPageClass::WindowProc
(
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	/*switch (message)
	{
	}*/
	
	// Allow the base class to process this message
	return CPropertyPage::WindowProc(message, wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
BOOL
EmitterParticlePropPageClass::OnNotify
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
		SetModified ();
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
EmitterParticlePropPageClass::OnSpecifyCreationVolume (void) 
{
	VolumeRandomDialogClass dialog (m_Randomizer, this);
	if (dialog.DoModal () == IDOK) {
		
		//
		//	Get the new randomizer from the dialog
		//
		SAFE_DELETE (m_Randomizer);
		m_Randomizer = dialog.Get_Randomizer ();
		SetModified ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnCommand
//
/////////////////////////////////////////////////////////////
BOOL
EmitterParticlePropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_BURST_SIZE_EDIT:
		case IDC_EMISSION_RATE_EDIT:
		case IDC_MAX_PARTICLES_EDIT:
			if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
			break;
	}
	
	return CPropertyPage::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  OnMaxParticlesCheck
//
/////////////////////////////////////////////////////////////
void
EmitterParticlePropPageClass::OnMaxParticlesCheck (void)
{
	BOOL enable = SendDlgItemMessage (IDC_MAX_PARTICLES_CHECK, BM_GETCHECK);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_MAX_PARTICLES_EDIT), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_MAX_PARTICLES_SPIN), enable);

	SetModified ();
	return ;	
}
