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

// RingGeneralPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "RingGeneralPropPage.h"
#include "Utils.h"
#include "assetmgr.h"
#include "texture.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(RingGeneralPropPageClass, CPropertyPage)


/////////////////////////////////////////////////////////////
//
//	RingGeneralPropPageClass
//
/////////////////////////////////////////////////////////////
RingGeneralPropPageClass::RingGeneralPropPageClass (RingRenderObjClass *ring)
	:	m_bValid (true),
		m_Lifetime (1.0F),
		m_RenderObj (ring),
		CPropertyPage(RingGeneralPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(RingGeneralPropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	~RingGeneralPropPageClass
//
/////////////////////////////////////////////////////////////
RingGeneralPropPageClass::~RingGeneralPropPageClass()
{
	return ;
}

void
RingGeneralPropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RingGeneralPropPageClass)
	DDX_Control(pDX, IDC_TEXTURE_TILE_SPIN, m_TextureTileSpin);
	DDX_Control(pDX, IDC_LIFETIME_SPIN, m_LifetimeSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(RingGeneralPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(RingGeneralPropPageClass)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowseButton)
	ON_EN_CHANGE(IDC_FILENAME_EDIT, OnChangeFilenameEdit)
	ON_EN_CHANGE(IDC_NAME_EDIT, OnChangeNameEdit)
	ON_EN_CHANGE(IDC_LIFETIME_EDIT, OnChangeLifetimeEdit)
	ON_CBN_SELCHANGE(IDC_SHADER_COMBO, OnSelchangeShaderCombo)
	ON_EN_CHANGE(IDC_TEXTURE_TILE_EDIT, OnChangeTextureTileEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::Initialize (void)
{
	if (m_RenderObj != NULL) {

		//
		// Get the object's texture
		//
		TextureClass *texture = m_RenderObj->Peek_Texture ();
		if (texture != NULL) {
			m_TextureFilename = texture->Get_Texture_Name ();
		}

		//
		//	Get the other misc data we care about
		//
		m_Lifetime	= m_RenderObj->Get_Animation_Duration ();
		m_Name		= m_RenderObj->Get_Name ();
		m_Shader		= m_RenderObj->Get_Shader ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  Add_Shader_To_Combo
//
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::Add_Shader_To_Combo
(
	ShaderClass &	shader,
	LPCTSTR			name
)
{
	int index = SendDlgItemMessage (IDC_SHADER_COMBO, CB_ADDSTRING, 0, (LPARAM)name);
	if (index != CB_ERR) {
		SendDlgItemMessage (IDC_SHADER_COMBO, CB_SETITEMDATA, (WPARAM)index, (LPARAM)&shader);

		//
		//	Is the blend mode of this shader the same as that of the
		//	object's shader.
		//
		if ((shader.Get_Alpha_Test () == m_Shader.Get_Alpha_Test ()) &&
			 (shader.Get_Dst_Blend_Func () == m_Shader.Get_Dst_Blend_Func ()) &&
			 (shader.Get_Src_Blend_Func () == m_Shader.Get_Src_Blend_Func ()))
		{
			SendDlgItemMessage (IDC_SHADER_COMBO, CB_SETCURSEL, (WPARAM)index);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
/////////////////////////////////////////////////////////////
BOOL
RingGeneralPropPageClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();

	//
	//	Add the known shaders to the combobox
	//
	Add_Shader_To_Combo (ShaderClass::_PresetAdditiveShader, "Additive");
	Add_Shader_To_Combo (ShaderClass::_PresetAlphaShader, "Alpha");
	Add_Shader_To_Combo (ShaderClass::_PresetOpaqueShader, "Opaque");
	Add_Shader_To_Combo (ShaderClass::_PresetMultiplicativeShader, "Multiplicative");

	CheckDlgButton (IDC_CAMERA_ALIGNED_CHECK, (m_RenderObj->Get_Flags () & RingRenderObjClass::USE_CAMERA_ALIGN) != 0);
	CheckDlgButton (IDC_LOOPING_CHECK, (m_RenderObj->Get_Flags () & RingRenderObjClass::USE_ANIMATION_LOOP) != 0);

	//
	// Fill the edit controls with the default values
	//
	SetDlgItemText (IDC_NAME_EDIT, m_Name);
	SetDlgItemText (IDC_FILENAME_EDIT, m_TextureFilename);

	//
	//	Initialize the texture tiling controls...
	//
	m_TextureTileSpin.SetRange (0, 8);
	m_TextureTileSpin.SetPos (m_RenderObj->Get_Texture_Tiling ());

	//
	// Initialize the lifetime control
	//
	::Initialize_Spinner (m_LifetimeSpin, m_Lifetime, 0, 1000);
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL
RingGeneralPropPageClass::OnApply (void)
{
	// Get the data from the dialog controls
	GetDlgItemText (IDC_NAME_EDIT, m_Name);
	GetDlgItemText (IDC_FILENAME_EDIT, m_TextureFilename);
	m_Lifetime = ::GetDlgItemFloat (m_hWnd, IDC_LIFETIME_EDIT);

	//
	//	Get the shader from the combobox
	//
	int index = SendDlgItemMessage (IDC_SHADER_COMBO, CB_GETCURSEL);
	if (index != CB_ERR) {
		ShaderClass *shader = (ShaderClass *)SendDlgItemMessage (IDC_SHADER_COMBO, CB_GETITEMDATA, (WPARAM)index);
		if (shader != NULL) {
			m_Shader = (*shader);
		}
	}

	// Check to make sure the user entered a valid name for the object
	BOOL retval = FALSE;
	if (m_Name.GetLength () == 0) {
		::MessageBox (m_hWnd, "Invalid ring name.  Please enter a new name.", "Invalid settings", MB_ICONEXCLAMATION | MB_OK);
		m_bValid = false;
	} else {

		//
		//	Create a texture and pass it onto the object
		//
		TextureClass *texture = NULL;
		if (m_TextureFilename.GetLength () > 0) {
			texture = WW3DAssetManager::Get_Instance ()->Get_Texture (::Get_Filename_From_Path (m_TextureFilename));
		}
		m_RenderObj->Set_Texture (texture);
		REF_PTR_RELEASE (texture);

		//
		//	Apply the changes to the ring
		//
		m_RenderObj->Set_Name (m_Name);
		m_RenderObj->Set_Shader (m_Shader);
		m_RenderObj->Set_Animation_Duration (m_Lifetime);
		m_RenderObj->Set_Flag (RingRenderObjClass::USE_CAMERA_ALIGN, IsDlgButtonChecked (IDC_CAMERA_ALIGNED_CHECK) != 0);
		m_RenderObj->Set_Flag (RingRenderObjClass::USE_ANIMATION_LOOP, IsDlgButtonChecked (IDC_LOOPING_CHECK) != 0);
		m_RenderObj->Set_Texture_Tiling (m_TextureTileSpin.GetPos ());

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
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::OnBrowseButton (void)
{
	CFileDialog dialog (	TRUE,
								".tga",
								NULL,
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Textures files (*.tga)|*.tga||",
								::AfxGetMainWnd ());

	// Ask the user what texture file they wish to load
	if (dialog.DoModal () == IDOK) {
		SetDlgItemText (IDC_FILENAME_EDIT, dialog.GetPathName ());
		SetModified ();
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnChangeFilenameEdit
//
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::OnChangeFilenameEdit (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnChangeNameEdit
//
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::OnChangeNameEdit (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL
RingGeneralPropPageClass::OnNotify
(
	WPARAM		wParam,
	LPARAM		lParam,
	LRESULT *	pResult
)
{
	//
	//	Update the spinner control if necessary
	//
	NMHDR *header = (NMHDR *)lParam;
	if ((header != NULL) && (header->code == UDN_DELTAPOS)) {
		LPNMUPDOWN updown = (LPNMUPDOWN)lParam;
		::Update_Spinner_Buddy (header->hwndFrom, updown->iDelta);
	}

	// Allow the base class to process this message
	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  OnChangeLifetimeEdit
//
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::OnChangeLifetimeEdit (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnSelchangeShaderCombo
//
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::OnSelchangeShaderCombo (void)
{
	SetModified ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnCommand
//
/////////////////////////////////////////////////////////////
BOOL
RingGeneralPropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_FILENAME_EDIT:
		case IDC_NAME_EDIT:
		case IDC_LIFETIME_EDIT:
		case IDC_TEXTURE_TILE_EDIT:
			if (HIWORD (lParam) == EN_CHANGE) {
				SetModified ();
			}
			break;

		case IDC_CAMERA_ALIGNED_CHECK:
		case IDC_LOOPING_CHECK:
			SetModified ();
			break;
	}

	return CPropertyPage::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  OnChangeTextureTileEdit
//
/////////////////////////////////////////////////////////////
void
RingGeneralPropPageClass::OnChangeTextureTileEdit (void)
{
	SetModified ();
	return ;
}

