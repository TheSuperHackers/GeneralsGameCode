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

// EmitterGeneralPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "EmitterGeneralPropPage.h"
#include "EmitterPropertySheet.h"
#include "part_emt.h"
#include "Utils.h"
#include "texture.h"
#include "shader.h"
#include "EmitterInstanceList.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterGeneralPropPageClass property page

IMPLEMENT_DYNCREATE(EmitterGeneralPropPageClass, CPropertyPage)


/////////////////////////////////////////////////////////////
//
//  EmitterGeneralPropPageClass
//
EmitterGeneralPropPageClass::EmitterGeneralPropPageClass (EmitterInstanceListClass *pemitter)
	: m_pEmitterList (nullptr),
	  m_Parent (nullptr),
	  m_bValid (true),
	  m_Lifetime (0),
	  CPropertyPage(EmitterGeneralPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(EmitterGeneralPropPageClass)
	//}}AFX_DATA_INIT
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  ~EmitterGeneralPropPageClass
//
EmitterGeneralPropPageClass::~EmitterGeneralPropPageClass (void)
{
	return;
}


/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
EmitterGeneralPropPageClass::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterGeneralPropPageClass)
	DDX_Control(pDX, IDC_RENDER_MODE_COMBO, m_RenderModeCombo);
	DDX_Control(pDX, IDC_PARTICLE_LIFETIME_SPIN, m_LifetimeSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(EmitterGeneralPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterGeneralPropPageClass)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowseButton)
	ON_EN_CHANGE(IDC_FILENAME_EDIT, OnChangeFilenameEdit)
	ON_EN_CHANGE(IDC_NAME_EDIT, OnChangeNameEdit)
	ON_EN_CHANGE(IDC_PARTICLE_LIFETIME_EDIT, OnChangeParticleLifetimeEdit)
	ON_CBN_SELCHANGE(IDC_SHADER_COMBO, OnSelchangeShaderCombo)
	ON_BN_CLICKED(IDC_PARTICLE_LIFETIME_CHECK, OnParticleLifetimeCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
void
EmitterGeneralPropPageClass::Initialize (void)
{
	if (m_pEmitterList != nullptr) {

		//
		// Get the emitter's texture
		//
		m_TextureFilename = m_pEmitterList->Get_Texture_Filename ();

		m_Lifetime		= m_pEmitterList->Get_Lifetime ();
		m_EmitterName	= m_pEmitterList->Get_Name ();
		m_pEmitterList->Get_Shader (m_Shader);
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  Add_Shader_To_Combo
//
void
EmitterGeneralPropPageClass::Add_Shader_To_Combo
(
	ShaderClass &shader,
	LPCTSTR name
)
{
	int index = SendDlgItemMessage (IDC_SHADER_COMBO, CB_ADDSTRING, 0, (LPARAM)name);
	if (index != CB_ERR) {
		SendDlgItemMessage (IDC_SHADER_COMBO, CB_SETITEMDATA, (WPARAM)index, (LPARAM)&shader);

		//
		//	Is the blend mode of this shader the same as that of the
		//	particle emitter's shader.
		//
		if ((shader.Get_Alpha_Test () == m_Shader.Get_Alpha_Test ()) &&
			 (shader.Get_Dst_Blend_Func () == m_Shader.Get_Dst_Blend_Func ()) &&
			 (shader.Get_Src_Blend_Func () == m_Shader.Get_Src_Blend_Func ())) {
			SendDlgItemMessage (IDC_SHADER_COMBO, CB_SETCURSEL, (WPARAM)index);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
EmitterGeneralPropPageClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();

	//
	//	Add the known shaders to the combobox
	//
	Add_Shader_To_Combo (ShaderClass::_PresetAdditiveSpriteShader, "Additive");
	Add_Shader_To_Combo (ShaderClass::_PresetAlphaSpriteShader, "Alpha");
	Add_Shader_To_Combo (ShaderClass::_PresetATestSpriteShader, "Alpha-Test");
	Add_Shader_To_Combo (ShaderClass::_PresetATestBlendSpriteShader, "Alpha-Test-Blend");
	Add_Shader_To_Combo (ShaderClass::_PresetScreenSpriteShader, "Screen");
	Add_Shader_To_Combo (ShaderClass::_PresetMultiplicativeSpriteShader, "Multiplicative");
	Add_Shader_To_Combo (ShaderClass::_PresetOpaqueSpriteShader, "Opaque");

	//
	// Fill the edit controls with the default values
	//
	SetDlgItemText (IDC_NAME_EDIT, m_EmitterName);
	SetDlgItemText (IDC_FILENAME_EDIT, m_TextureFilename);

	//
	// Initialize the lifetime control
	//
	SendDlgItemMessage (IDC_PARTICLE_LIFETIME_CHECK, BM_SETCHECK, (WPARAM)(m_Lifetime < 100));
	if (m_Lifetime > 100) {
		m_Lifetime = 0;
	}
	::Initialize_Spinner (m_LifetimeSpin, m_Lifetime, 0, 1000);

	OnParticleLifetimeCheck ();

	//
	// Initialize the render mode combo
	//
	m_RenderModeCombo.SetCurSel(m_pEmitterList->Get_Render_Mode());

	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
BOOL
EmitterGeneralPropPageClass::OnApply (void)
{
	// Get the data from the dialog controls
	GetDlgItemText (IDC_NAME_EDIT, m_EmitterName);
	GetDlgItemText (IDC_FILENAME_EDIT, m_TextureFilename);
	m_Lifetime = ::GetDlgItemFloat (m_hWnd, IDC_PARTICLE_LIFETIME_EDIT);
	if (SendDlgItemMessage (IDC_PARTICLE_LIFETIME_CHECK, BM_GETCHECK) == 0) {
		m_Lifetime = 5000000.0F;
	}

	//
	//	Get the shader from the combobox
	//
	int index = SendDlgItemMessage (IDC_SHADER_COMBO, CB_GETCURSEL);
	if (index != CB_ERR) {
		ShaderClass *shader = (ShaderClass *)SendDlgItemMessage (IDC_SHADER_COMBO, CB_GETITEMDATA, (WPARAM)index);
		if (shader != nullptr) {
			m_Shader = (*shader);
		}
	}

	// Check to make sure the user entered a valid name for the emitter.
	BOOL retval = FALSE;
	if (m_EmitterName.GetLength () == 0) {
		::MessageBox (m_hWnd, "Invalid emitter name.  Please enter a new name.", "Invalid settings", MB_ICONEXCLAMATION | MB_OK);
		m_bValid = false;
	} else {

		//
		//	Apply the changes to the emitter
		//
		m_pEmitterList->Set_Lifetime (m_Lifetime);
		m_pEmitterList->Set_Texture_Filename (m_TextureFilename);
		m_pEmitterList->Set_Name (m_EmitterName);
		m_pEmitterList->Set_Shader (m_Shader);
		m_pEmitterList->Set_Render_Mode (m_RenderModeCombo.GetCurSel());

		// Allow the base class to process this message
		retval = CPropertyPage::OnApply ();
		m_bValid = true;
	}

	// Return the TRUE/FALSE result code
	return retval;
}


/////////////////////////////////////////////////////////////
//
//  OnBrowseButton
//
void
EmitterGeneralPropPageClass::OnBrowseButton (void)
{
	CFileDialog openFileDialog (TRUE,
										 ".tga",
										 nullptr,
										 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
										 "Textures files (*.tga)|*.tga||",
										 ::AfxGetMainWnd ());

	// Ask the user what texture file they wish to load
	if (openFileDialog.DoModal () == IDOK) {
		SetDlgItemText (IDC_FILENAME_EDIT, openFileDialog.GetPathName ());
		SetModified ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnChangeFilenameEdit
//
void
EmitterGeneralPropPageClass::OnChangeFilenameEdit (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnChangeNameEdit
//
void
EmitterGeneralPropPageClass::OnChangeNameEdit (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
BOOL
EmitterGeneralPropPageClass::OnNotify
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
	if ((pheader != nullptr) && (pheader->code == UDN_DELTAPOS)) {
		LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;
		::Update_Spinner_Buddy (pheader->hwndFrom, pupdown->iDelta);
	}

	// Allow the base class to process this message
	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  OnChangeParticleLifetimeEdit
//
void
EmitterGeneralPropPageClass::OnChangeParticleLifetimeEdit (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnSelchangeShaderCombo
//
void
EmitterGeneralPropPageClass::OnSelchangeShaderCombo (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnCommand
//
BOOL
EmitterGeneralPropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam)) {
		case IDC_FILENAME_EDIT:
		case IDC_NAME_EDIT:
		case IDC_PARTICLE_LIFETIME_EDIT:
			if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
			break;

		case IDC_PARTICLE_LIFETIME_CHECK:
			if (HIWORD (wParam) == BN_CLICKED) {
				SetModified ();
			}
			break;

		case IDC_RENDER_MODE_COMBO:
			if (HIWORD (wParam) == CBN_SELCHANGE) {
				SetModified ();
				if (m_Parent != nullptr) {
					int cur_mode = ::SendMessage ((HWND)lParam, CB_GETCURSEL, 0, 0);
					m_Parent->Notify_Render_Mode_Changed(cur_mode);
				}
			}
			break;

	}

	return CPropertyPage::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  OnParticleLifetimeCheck
//
/////////////////////////////////////////////////////////////
void
EmitterGeneralPropPageClass::OnParticleLifetimeCheck (void)
{
	bool enable = (SendDlgItemMessage (IDC_PARTICLE_LIFETIME_CHECK, BM_GETCHECK) == 1);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PARTICLE_LIFETIME_EDIT), enable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_PARTICLE_LIFETIME_SPIN), enable);

	if (enable == false) {
		m_Lifetime = 0;
		::SetDlgItemFloat (m_hWnd, IDC_PARTICLE_LIFETIME_EDIT, 0);
	}

	SetModified ();
	return ;
}

