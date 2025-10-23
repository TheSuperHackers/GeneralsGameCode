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
 *                     $Archive:: /Commando/Code/Tools/W3DView/TextureSettingsDialog.h                                                                                                                                                                                                                                                                                                                                 $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifdef WW3D_DX8

// Forward delcarations
class TextureClass;

#include "Resource.H"

/////////////////////////////////////////////////////////////////////////////
//
// TextureSettingsDialogClass
//
class TextureSettingsDialogClass : public CDialog
{
// Construction
public:
	TextureSettingsDialogClass (IndirectTextureClass *ptexture, IndirectTextureClass *poriginal_texture, CWnd *pParent = NULL);
	virtual ~TextureSettingsDialogClass (void);

// Dialog Data
	//{{AFX_DATA(TextureSettingsDialogClass)
	enum { IDD = IDD_TEXTURE_SETTINGS };
	CComboBox	m_TypeCombo;
	CSpinButtonCtrl	m_FrameRateSpin;
	CSpinButtonCtrl	m_FrameCountSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TextureSettingsDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TextureSettingsDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAnimationCheck();
	afx_msg void OnDestroy();
	afx_msg void OnBrowseButton();
	afx_msg void OnRestore();
	afx_msg void OnApply();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		////////////////////////////////////////////////////////////
		//
		//	Private methods
		//
		bool					Were_Settings_Modified (void) const { return m_bWereSettingsModified; }

	protected:

		////////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void					Fill_Controls (TextureClass *ptexture);
		void					Fill_Animation_Controls (TextureClass *ptexture);
		void					Load_Textures_Into_Combo (void);
		void					Load_Texture_Settings (void);
		void					Paint_Thumbnail (void);

	private:

		////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		IndirectTextureClass *	m_pTexture;
		IndirectTextureClass *	m_pOriginalTexture;
		TextureClass *				m_pStartingTexture;
		HBITMAP						m_hThumbnail;
		bool							m_bWereSettingsModified;
};

#endif //WW3D_DX8

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
