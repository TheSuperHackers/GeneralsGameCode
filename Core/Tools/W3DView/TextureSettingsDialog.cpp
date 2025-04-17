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
 *                     $Archive:: /Commando/Code/Tools/W3DView/TextureSettingsDialog.cpp                                                                                                                                                                                                                                                                                                                                 $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.H"
#include "Texture.H"
#include "W3DView.H"
#include "TextureSettingsDialog.H"
#include "Utils.H"
#include "AssetMgr.H"

/*#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif*/


/////////////////////////////////////////////////////////////////////////////
//
// Local data types
//
typedef enum
{
	TYPE_LOOP			= 0,
	TYPE_ONCE,
	TYPE_PING_PONG,	
	TYPE_MANUAL,
	TYPE_COUNT
} ANIM_TYPES;

#ifdef WW3D_DX8

/////////////////////////////////////////////////////////////////////////////
//
// TextureSettingsDialogClass
//
TextureSettingsDialogClass::TextureSettingsDialogClass
(
	IndirectTextureClass *ptexture,
	IndirectTextureClass *poriginal_texture,
	CWnd *pParent
)
	: m_pTexture (NULL),
	  m_pOriginalTexture (NULL),
	  m_pStartingTexture (NULL),
	  m_hThumbnail (NULL),
	  m_bWereSettingsModified (false),
	  CDialog(TextureSettingsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(TextureSettingsDialogClass)
	//}}AFX_DATA_INIT
	MEMBER_ADD (m_pTexture, ptexture);
	MEMBER_ADD (m_pOriginalTexture, poriginal_texture);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~TextureSettingsDialogClass
//
TextureSettingsDialogClass::~TextureSettingsDialogClass (void)
{
	SR_RELEASE (m_pTexture);
	SR_RELEASE (m_pOriginalTexture);
	SR_RELEASE (m_pStartingTexture);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
void
TextureSettingsDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TextureSettingsDialogClass)
	DDX_Control(pDX, IDC_TYPE_COMBO, m_TypeCombo);
	DDX_Control(pDX, IDC_FPS_SPIN, m_FrameRateSpin);
	DDX_Control(pDX, IDC_FRAME_COUNT_SPIN, m_FrameCountSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(TextureSettingsDialogClass, CDialog)
	//{{AFX_MSG_MAP(TextureSettingsDialogClass)
	ON_BN_CLICKED(IDC_ANIMATION_CHECK, OnAnimationCheck)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowseButton)
	ON_BN_CLICKED(IDC_RESTORE, OnRestore)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
BOOL
TextureSettingsDialogClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CDialog::OnInitDialog ();
	ASSERT (m_pTexture != NULL);
	ASSERT (m_pTexture->getClassID () == ID_INDIRECT_TEXTURE_CLASS);
	
	// Determine what the starting texture was so we can restore on cancel (if necessary)
	m_pStartingTexture = m_pTexture->Get_Texture ();
	//m_pStartingTexture->addReference ();

	// Set the range of the spin controls
	m_FrameCountSpin.SetRange (1, 10000);
	m_FrameRateSpin.SetRange (1, 10000);

	// Remove the border from around our child window
	HWND hchild_wnd = ::GetDlgItem (m_hWnd, IDC_TEXTURE_THUMBNAIL);
	LONG style = ::GetWindowLong (hchild_wnd, GWL_STYLE);
	::SetWindowLong (hchild_wnd, GWL_STYLE, style & (~WS_BORDER));

	// Enable or disable the 'restore' button based on whether or not we
	// have an original texture to switch to...
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_RESTORE), (m_pOriginalTexture != NULL));
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_APPLY), FALSE);

	// Fill the dialog controls with data from the texture
	Load_Texture_Settings ();
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_Texture_Settings
//
void
TextureSettingsDialogClass::Load_Texture_Settings (void)
{
	// Free the old thumbnail (if there was one)
	if (m_hThumbnail != NULL) {
		DeleteObject (m_hThumbnail);
		m_hThumbnail = NULL;
	}

	// Get the actual texture...
	TextureClass *ptexture = m_pTexture->Get_Texture ();

	// Refresh the preview window
	m_hThumbnail = Make_Bitmap_From_Texture (*ptexture, 96, 96);
	Paint_Thumbnail ();

	// Fill the controls using the texture pointer
	Fill_Controls (ptexture);
	Fill_Animation_Controls (ptexture);
	OnAnimationCheck ();

	// Release our hold on the texture
	SR_RELEASE (ptexture);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Controls
//
void
TextureSettingsDialogClass::Fill_Controls (TextureClass *ptexture)
{
	srTexture *psource = NULL;

	// What type of texture is this?
	switch (ptexture->getClassID ())
	{
		case srClass::ID_TEXTURE_FILE:
			psource = (srTextureFile *)ptexture;
			SetDlgItemText (IDC_FILENAME_EDIT, ptexture->getName ());
			break;

		case ID_MANUAL_ANIM_TEXTURE_INSTANCE_CLASS:
		case ID_TIME_ANIM_TEXTURE_INSTANCE_CLASS:
		case ID_RESIZEABLE_TEXTURE_INSTANCE_CLASS:
			SendDlgItemMessage (IDC_RESIZEABLE_CHECK, BM_SETCHECK, (WPARAM)TRUE);
			psource = ((ResizeableTextureInstanceClass *)ptexture)->Peek_Source();

			// Fill the 'filename' edit control
			if (psource != NULL && (psource->getClassID () == ID_FILE_LIST_TEXTURE_CLASS)) {
				FileListTextureClass *pfile_list = static_cast<FileListTextureClass *>(psource);
				SetDlgItemText (IDC_FILENAME_EDIT, pfile_list->Get_Filename (0));
			}
			
			break;

		default:
			ASSERT (0);
			break;
	}
	
	// Set the checkboxes
	ASSERT (psource != NULL);
	if (psource != NULL) {
		SendDlgItemMessage (IDC_MIPMAP_OFF_CHECK, BM_SETCHECK, (WPARAM)(psource->getMipmap () == srTextureIFace::MIPMAP_NONE));
		SendDlgItemMessage (IDC_ALPHA_CHECK, BM_SETCHECK, (WPARAM)(psource->isHintEnabled(srTextureIFace::HINT_ALPHA_BITMASK)));	
		SendDlgItemMessage (IDC_CLAMPU_CHECK, BM_SETCHECK, (WPARAM)(psource->Get_U_Addr_Mode() == TextureClass::TEXTURE_ADDRESS_CLAMP));
		SendDlgItemMessage (IDC_CLAMPV_CHECK, BM_SETCHECK, (WPARAM)(psource->Get_V_Addr_Mode() == TextureClass::TEXTURE_ADDRESS_CLAMP));
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Fill_Animation_Controls
//
void
TextureSettingsDialogClass::Fill_Animation_Controls (TextureClass *ptexture)
{
	bool banimated = false;
	int frame_count = 0;
	float frames_per_sec = 0;
	ANIM_TYPES type = TYPE_ONCE;

	// What type is the texture?
	switch (ptexture->getClassID ())
	{
		case ID_MANUAL_ANIM_TEXTURE_INSTANCE_CLASS:
		{
			ManualAnimTextureInstanceClass *anim_texture = (ManualAnimTextureInstanceClass *)ptexture;
			frame_count = anim_texture->Get_Num_Frames ();
			frames_per_sec = 15.0F;
			type = TYPE_MANUAL;
			banimated = true;
		}
		break;
		
		case ID_TIME_ANIM_TEXTURE_INSTANCE_CLASS:
		{
			// What mode is this animated texture using
			TimeAnimTextureInstanceClass *anim_texture = (TimeAnimTextureInstanceClass *)ptexture;
			switch (anim_texture->Get_Mode ())
			{
				case TimeAnimTextureInstanceClass::ONE_TIME:
					type = TYPE_ONCE;
					break;

				case TimeAnimTextureInstanceClass::LOOP:
					type = TYPE_LOOP;
					break;

				case TimeAnimTextureInstanceClass::PINGPONG:
					type = TYPE_PING_PONG;
					break;
			};

			// Get the texture's frame rate and count
			frame_count = anim_texture->Get_Num_Frames ();
			frames_per_sec = anim_texture->Get_Frame_Rate ();
			banimated = true;
		}
		break;		
	}

	// Check or uncheck the animation box depending on if it was an animated texture
	SendDlgItemMessage (IDC_ANIMATION_CHECK, BM_SETCHECK, (WPARAM)banimated);
	
	// Was this an animated texture?	
	if (banimated == true) {
		
		// Pass the frame count onto the control
		frame_count = (frame_count > 0) ? frame_count : 1;
		SetDlgItemInt (IDC_FRAME_COUNT_EDIT, frame_count);		
		m_FrameCountSpin.SetPos (frame_count);
		
		// Pass the frame rate onto the control
		frames_per_sec = (frames_per_sec > 0) ? frames_per_sec : 1;
		SetDlgItemInt (IDC_FPS_EDIT, (int)frames_per_sec);		
		m_FrameRateSpin.SetPos (frames_per_sec);

		// Select the type in the combobox
		m_TypeCombo.SetCurSel ((int)type);
	} else {
		m_TypeCombo.SetCurSel (0);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
void
TextureSettingsDialogClass::OnOK (void)
{	
	// Force the new settings to take effect
	OnApply ();

	// Allow the base class to process this message
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
void
TextureSettingsDialogClass::OnCancel (void) 
{
	// Reuse the starting texture
	m_pTexture->Set_Texture (m_pStartingTexture);

	// Allow the base class to process this message
	CDialog::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnAnimationCheck
//
void
TextureSettingsDialogClass::OnAnimationCheck (void)
{
	bool benable = (SendDlgItemMessage (IDC_ANIMATION_CHECK, BM_GETCHECK) == 1);

	// Enable or disable the controls based on the animation checkbox-state
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_FRAME_COUNT_EDIT), benable);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_FPS_EDIT), benable);
	::EnableWindow (m_TypeCombo, benable);
	::EnableWindow (m_FrameRateSpin, benable);
	::EnableWindow (m_FrameCountSpin, benable);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// WindowProc
//
LRESULT
TextureSettingsDialogClass::WindowProc
(
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	if (message == WM_PAINT) {
		Paint_Thumbnail ();
	} else if (message == WM_COMMAND) {
		
		// What control sent us this notification?
		switch (LOWORD (wParam))
		{
			case IDC_FRAME_COUNT_EDIT:
			case IDC_FPS_EDIT:
			case IDC_FILENAME_EDIT:
				if ((HIWORD (wParam) == EN_UPDATE) ||
					 (HIWORD (wParam) == EN_CHANGE)) {
					::EnableWindow (::GetDlgItem (m_hWnd, IDC_APPLY), TRUE);
				}
				break;

			case IDC_MIPMAP_OFF_CHECK:
			case IDC_CLAMPU_CHECK:
			case IDC_CLAMPV_CHECK:
			case IDC_ALPHA_CHECK:
			case IDC_RESIZEABLE_CHECK:
			case IDC_ANIMATION_CHECK:
				if (HIWORD (wParam) == BN_CLICKED) {
					::EnableWindow (::GetDlgItem (m_hWnd, IDC_APPLY), TRUE);
				}
				break;

			case IDC_TYPE_COMBO:
				if (HIWORD (wParam) == CBN_SELCHANGE) {
					::EnableWindow (::GetDlgItem (m_hWnd, IDC_APPLY), TRUE);
				}
				break;
		}
	}
	
	// Allow the base class to process this message
	return CDialog::WindowProc (message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDestroy
//
void
TextureSettingsDialogClass::OnDestroy (void)
{
	if (m_hThumbnail != NULL) {
		::DeleteObject (m_hThumbnail);
		m_hThumbnail = NULL;
	}

	// Allow the base class to process this message
	CDialog::OnDestroy ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnBrowseButton
//
void
TextureSettingsDialogClass::OnBrowseButton (void) 
{
	// Get the current filename to display
	CString filename;
	GetDlgItemText (IDC_FILENAME_EDIT, filename);

	CFileDialog dialog (TRUE,
							  ".tga",
							  filename,
							  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
							  "Targa files (*.tga)|*.tga||",
							  this);

	// Ask the user what Targa file they wish to load
	if (dialog.DoModal () == IDOK) {
		WW3D::Add_Search_Path (::Strip_Filename_From_Path (dialog.GetFileName ()));

		// Set the text of the filename combobox control
		SetDlgItemText (IDC_FILENAME_EDIT, dialog.GetFileName ());
		SendDlgItemMessage (IDC_FILENAME_EDIT, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

		// Enable the apply button
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_APPLY), TRUE);
	}

	return ;	
}


/////////////////////////////////////////////////////////////////////////////
//
// Paint_Thumbnail
//
void
TextureSettingsDialogClass::Paint_Thumbnail (void) 
{
	// Paint the thumbnail
	if (m_hThumbnail != NULL) {
		
		// Get the misc crap windows requries before we can
		// paint to the screen
		HWND hchild_wnd = ::GetDlgItem (m_hWnd, IDC_TEXTURE_THUMBNAIL);
		HDC hdc = ::GetDC (hchild_wnd);
		HDC hmem_dc = ::CreateCompatibleDC (NULL);
		HBITMAP hold_bmp = (HBITMAP)::SelectObject (hmem_dc, m_hThumbnail);

		// Paint the thumbnail onto the dialog
		CRect rect;
		::GetClientRect (hchild_wnd, &rect);
		::BitBlt (hdc,
					 rect.left + (rect.Width () >> 1) - 48,
					 rect.top + (rect.Height () >> 1) - 48,
					 96,
					 96,
					 hmem_dc,
					 0,
					 0,
					 SRCCOPY);

		// Release the misc windows crap
		::SelectObject (hmem_dc, hold_bmp);
		::ReleaseDC (hchild_wnd, hmem_dc);
		::DeleteDC (hmem_dc);
		::ValidateRect (hchild_wnd, NULL);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnRestore
//
void
TextureSettingsDialogClass::OnRestore (void)
{
	if (m_pOriginalTexture != NULL) {

		// Get the original texture
		TextureClass *pnew_texture = m_pOriginalTexture->Get_Texture ();
		m_pTexture->Set_Texture (pnew_texture);
		MEMBER_RELEASE (pnew_texture);

		// Reload the dialog control settings
		Load_Texture_Settings ();

		// Disable the apply button because we just did...
		::EnableWindow (::GetDlgItem (m_hWnd, IDC_APPLY), FALSE);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnApply
//
void
TextureSettingsDialogClass::OnApply (void)
{
	// Get the current texture name from the edit control
	CString texture_name;
	GetDlgItemText (IDC_FILENAME_EDIT, texture_name);

	// Determine some of the texture settings
	TextureClass *pnew_texture = new TimeAnimTextureInstanceClass;
	bool resizeable = (SendDlgItemMessage (IDC_RESIZEABLE_CHECK, BM_GETCHECK) == TRUE);
	int frame_count = GetDlgItemInt (IDC_FRAME_COUNT_EDIT);
	int frame_rate = GetDlgItemInt (IDC_FPS_EDIT);

	// Does the user want this to be an animated texture?
	if (SendDlgItemMessage (IDC_ANIMATION_CHECK, BM_GETCHECK) == 1) {
		
		// What type of animated texture is this?
		switch ((ANIM_TYPES)m_TypeCombo.GetCurSel ())
		{
			case TYPE_LOOP:					
				pnew_texture = new TimeAnimTextureInstanceClass (*(WW3DAssetManager::Get_Instance ()),
																				 texture_name,
																				 frame_count,
																				 (float)frame_rate,
																				 TimeAnimTextureInstanceClass::LOOP,
																				 TimeAnimTextureInstanceClass::FORWARD);
				break;

			case TYPE_ONCE:
				pnew_texture = new TimeAnimTextureInstanceClass (*(WW3DAssetManager::Get_Instance ()),
																				 texture_name,
																				 frame_count,
																				 (float)frame_rate,
																				 TimeAnimTextureInstanceClass::ONE_TIME,
																				 TimeAnimTextureInstanceClass::FORWARD);
				break;

			case TYPE_PING_PONG:
				pnew_texture = new TimeAnimTextureInstanceClass (*(WW3DAssetManager::Get_Instance ()),
																				 texture_name,
																				 frame_count,
																				 (float)frame_rate,
																				 TimeAnimTextureInstanceClass::PINGPONG,
																				 TimeAnimTextureInstanceClass::FORWARD);
				break;

			case TYPE_MANUAL:
				pnew_texture = new ManualAnimTextureInstanceClass (*(WW3DAssetManager::Get_Instance ()),
																					texture_name,
																					frame_count);
				break;
		}

	} else {

		// Should we allocate a resizeable texture or a normal one?
		/*if (resizeable) {
			pnew_texture = new ResizeableTextureInstanceClass (*(WW3DAssetManager::Get_Instance ()), texture_name);
		} else {*/
			// Normal texture, simply load it from file
			pnew_texture = WW3DAssetManager::Get_Instance ()->Get_Texture (texture_name);
		//}
	}

	ASSERT (pnew_texture != NULL);
	if (pnew_texture != NULL) {			
		
		// Turn mipmapping off if necessary
		if (SendDlgItemMessage (IDC_MIPMAP_OFF_CHECK, BM_GETCHECK) == 1) {
			::MipMapping_Off (pnew_texture);
		}

		// Compress the alpha channel to 1 bit if necessary
		if (SendDlgItemMessage (IDC_ALPHA_CHECK, BM_GETCHECK) == 1) {
			::Set_Alpha_Bitmap (pnew_texture);
		}
		
		// Clamp the UVs if necessary
		::Set_Clamping (pnew_texture,
							 (SendDlgItemMessage (IDC_CLAMPU_CHECK, BM_GETCHECK) == 1),
							 (SendDlgItemMessage (IDC_CLAMPV_CHECK, BM_GETCHECK) == 1));
		

		// Pass the new texture on, and free the old texture
		m_pTexture->Set_Texture (pnew_texture);

		// We now have a new starting texture... (used for cancel operations)
		SR_RELEASE (m_pStartingTexture);
		m_pStartingTexture = pnew_texture;

		// Reload any controls we might need to display changes...
		Load_Texture_Settings ();
		m_bWereSettingsModified = true;

	} else {
		MessageBox ("Unable to create the requested texture.", "Texture Error", MB_ICONERROR | MB_OK);
	}

	// Disable the apply button because we just did...
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_APPLY), FALSE);
	return ;
}

#endif //WW3D_DX8

