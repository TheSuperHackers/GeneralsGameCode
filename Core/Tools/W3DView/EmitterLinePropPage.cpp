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

// EmitterLinePropPage.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "EmitterLinePropPage.h"
#include "w3d_file.h"
#include "EmitterInstanceList.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterLinePropPageClass property page

IMPLEMENT_DYNCREATE(EmitterLinePropPageClass, CPropertyPage)

EmitterLinePropPageClass::EmitterLinePropPageClass() : 
	CPropertyPage(EmitterLinePropPageClass::IDD),
	m_pEmitterList(NULL),
	m_bValid(true),
	m_MappingMode(W3D_EMITTER_RENDER_MODE_TRI_PARTICLES),
	m_MergeIntersections(false),
	m_EndCaps(false),
	m_DisableSorting(false),
	m_SubdivisionLevel(0),
	m_NoiseAmplitude(0.0f),
	m_MergeAbortFactor(0.0f),
	m_TextureTileFactor(0.0f),
	m_UPerSec(0.0f),
	m_VPerSec(0.0f)
{
	//{{AFX_DATA_INIT(EmitterLinePropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

EmitterLinePropPageClass::~EmitterLinePropPageClass()
{
}

void EmitterLinePropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterLinePropPageClass)
	DDX_Control(pDX, IDC_MAPMODE_COMBO, m_MapModeCombo);
	DDX_Control(pDX, IDC_MERGE_ABORT_FACTOR_SPIN, m_MergeAbortFactorSpin);
	DDX_Control(pDX, IDC_VPERSEC_SPIN, m_VPerSecSpin);
	DDX_Control(pDX, IDC_UVTILING_SPIN, m_UVTilingSpin);
	DDX_Control(pDX, IDC_UPERSEC_SPIN, m_UPerSecSpin);
	DDX_Control(pDX, IDC_NOISE_AMPLITUDE_SPIN, m_NoiseAmplitudeSpin);
	DDX_Control(pDX, IDC_SUBDIVISION_LEVEL_SPIN, m_SubdivisionLevelSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EmitterLinePropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterLinePropPageClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EmitterLinePropPageClass message handlers


/////////////////////////////////////////////////////////////
//
//  Initialize
//
void
EmitterLinePropPageClass::Initialize (void)
{
	if (m_pEmitterList != NULL) {

		//
		// Read the settings from the emitter
		//
		m_MappingMode = m_pEmitterList->Get_Line_Texture_Mapping_Mode();
		m_MergeIntersections = !!m_pEmitterList->Is_Merge_Intersections();
		m_DisableSorting = !!m_pEmitterList->Is_Sorting_Disabled();
		m_EndCaps = !!m_pEmitterList->Are_End_Caps_Enabled();
		m_SubdivisionLevel = m_pEmitterList->Get_Subdivision_Level();
		m_NoiseAmplitude = m_pEmitterList->Get_Noise_Amplitude();
		m_MergeAbortFactor = m_pEmitterList->Get_Merge_Abort_Factor();
		m_TextureTileFactor = m_pEmitterList->Get_Texture_Tile_Factor();
		
		Vector2 uvrate = m_pEmitterList->Get_UV_Offset_Rate();
		m_UPerSec = uvrate.X;
		m_VPerSec = uvrate.Y;
	}

	return ;
}

BOOL EmitterLinePropPageClass::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// Set up the spinner ranges
	m_SubdivisionLevelSpin.SetRange(0,8);
	m_SubdivisionLevelSpin.SetPos(m_SubdivisionLevel);

	::Initialize_Spinner (m_NoiseAmplitudeSpin, m_NoiseAmplitude, -10000, 10000);
	::Initialize_Spinner (m_MergeAbortFactorSpin, m_MergeAbortFactor, -10000, 10000);
	::Initialize_Spinner (m_UVTilingSpin, m_TextureTileFactor, 0.0f,8.0f);
	::Initialize_Spinner (m_UPerSecSpin, m_UPerSec, 0.0f,32.0f);
	::Initialize_Spinner (m_VPerSecSpin, m_VPerSec, 0.0f,32.0f);

	// Set the combo box
	m_MapModeCombo.SetCurSel(m_MappingMode);

	// Set the checkboxes
	SendDlgItemMessage (IDC_MERGE_INTERSECTIONS_CHECK, BM_SETCHECK, (WPARAM)(m_MergeIntersections != 0));
	SendDlgItemMessage (IDC_END_CAPS_CHECK, BM_SETCHECK, (WPARAM)(m_EndCaps != 0));
	SendDlgItemMessage (IDC_DISABLE_SORTING_CHECK, BM_SETCHECK, (WPARAM)(m_DisableSorting != 0));

	bool enable = (m_pEmitterList->Get_Render_Mode() == W3D_EMITTER_RENDER_MODE_LINE);
	::Enable_Dialog_Controls(m_hWnd,enable);
	
	return TRUE;
}

BOOL EmitterLinePropPageClass::OnApply() 
{
	//
	//	Get the data from the controls
	//
	m_SubdivisionLevel = GetDlgItemInt (IDC_SUBDIVISION_LEVEL_EDIT);
	m_NoiseAmplitude = ::GetDlgItemFloat (m_hWnd, IDC_NOISE_AMPLITUDE_EDIT);
	m_MergeAbortFactor = ::GetDlgItemFloat (m_hWnd, IDC_MERGE_ABORT_FACTOR_EDIT);
	m_TextureTileFactor = ::GetDlgItemFloat (m_hWnd, IDC_UVTILING_EDIT);
	m_UPerSec = ::GetDlgItemFloat (m_hWnd, IDC_UPERSEC_EDIT);
	m_VPerSec = ::GetDlgItemFloat (m_hWnd, IDC_VPERSEC_EDIT);
	
	m_MappingMode = SendDlgItemMessage (IDC_MAPMODE_COMBO, CB_GETCURSEL);
	m_MergeIntersections = !!SendDlgItemMessage (IDC_MERGE_INTERSECTIONS_CHECK, BM_GETCHECK);
	m_EndCaps = !!SendDlgItemMessage (IDC_END_CAPS_CHECK, BM_GETCHECK);
	m_DisableSorting = !!SendDlgItemMessage (IDC_DISABLE_SORTING_CHECK, BM_GETCHECK);

	//
	//	Apply the changes to the emitter
	//
	m_pEmitterList->Set_Subdivision_Level (m_SubdivisionLevel);
	m_pEmitterList->Set_Noise_Amplitude (m_NoiseAmplitude);
	m_pEmitterList->Set_Merge_Abort_Factor (m_MergeAbortFactor);
	m_pEmitterList->Set_Texture_Tile_Factor (m_TextureTileFactor);
	m_pEmitterList->Set_UV_Offset_Rate (Vector2(m_UPerSec,m_VPerSec));

	m_pEmitterList->Set_Line_Texture_Mapping_Mode (m_MappingMode);
	m_pEmitterList->Set_Merge_Intersections (m_MergeIntersections);
	m_pEmitterList->Set_Disable_Sorting(m_DisableSorting);
	m_pEmitterList->Set_End_Caps(m_EndCaps);


	// Allow the base class to process this message
	return CPropertyPage::OnApply();
}


BOOL EmitterLinePropPageClass::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (LOWORD (wParam))
	{
		// Check if any of the edit boxes were modified, 
		case IDC_SUBDIVISION_LEVEL_EDIT:
		case IDC_NOISE_AMPLITUDE_EDIT:
		case IDC_MERGE_ABORT_FACTOR_EDIT:
		case IDC_UVTILING_EDIT:
		case IDC_UPERSEC_EDIT:
		case IDC_VPERSEC_EDIT:
			if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
			break;

		case IDC_MERGE_INTERSECTIONS_CHECK:
		case IDC_END_CAPS_CHECK:
		case IDC_DISABLE_SORTING_CHECK:
			if (HIWORD (wParam) == BN_CLICKED) {
				SetModified ();
			}
			break;

		case IDC_MAPMODE_COMBO:
			if (HIWORD (wParam) == CBN_SELCHANGE) {
				SetModified ();
			}
			break;
	
	}
	
	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL EmitterLinePropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
