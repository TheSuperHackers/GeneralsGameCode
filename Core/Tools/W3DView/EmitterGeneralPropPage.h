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

#if !defined(AFX_EMITTERGENERALPROPPAGE_H__83A8B83C_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
#define AFX_EMITTERGENERALPROPPAGE_H__83A8B83C_BA3B_11D2_9FFA_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmitterGeneralPropPage.h : header file
//

#include "resource.h"
#include "shader.h"

// Forward delcarations
class EmitterInstanceListClass;
class EmitterPropertySheetClass;

/////////////////////////////////////////////////////////////////////////////
//
//	EmitterGeneralPropPageClass dialog
//
class EmitterGeneralPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterGeneralPropPageClass)

// Construction
public:
	EmitterGeneralPropPageClass (EmitterInstanceListClass *pemitter_list = NULL);
	~EmitterGeneralPropPageClass ();

// Dialog Data
	//{{AFX_DATA(EmitterGeneralPropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_GEN };
	CComboBox	m_RenderModeCombo;
	CSpinButtonCtrl	m_LifetimeSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterGeneralPropPageClass)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(EmitterGeneralPropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseButton();
	afx_msg void OnChangeFilenameEdit();
	afx_msg void OnChangeNameEdit();
	afx_msg void OnChangeParticleLifetimeEdit();
	afx_msg void OnSelchangeShaderCombo();
	afx_msg void OnParticleLifetimeCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		/////////////////////////////////////////////////////////
		//
		//	Public methods
		//

		//
		//	Inline accessors
		//
		EmitterInstanceListClass *	Get_Emitter (void) const { return m_pEmitterList; }
		void								Set_Emitter (EmitterInstanceListClass *pemitter_list) { m_pEmitterList = pemitter_list; Initialize (); }

		EmitterPropertySheetClass *Get_Parent (void) const { return m_Parent; }
		void								Set_Parent (EmitterPropertySheetClass * parent) { m_Parent = parent; }

		bool								Is_Data_Valid (void) const { return m_bValid; }

		const CString &				Get_Name (void) const					{ return m_EmitterName; }
		const CString &				Get_Texture_Filename (void) const	{ return m_TextureFilename; }
		float								Get_Lifetime (void) const				{ return m_Lifetime; }
		const ShaderClass &			Get_Shader (void) const					{ return m_Shader; }
		//void								Get_Shader (ShaderClass &shader);

	protected:

		/////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void								Initialize (void);
		void								Add_Shader_To_Combo (ShaderClass &shader, LPCTSTR name);

	private:

		/////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		EmitterInstanceListClass *	m_pEmitterList;
		EmitterPropertySheetClass *m_Parent;

		CString							m_EmitterName;
		CString							m_TextureFilename;
		ShaderClass						m_Shader;
		//int								m_ShaderType;
		float								m_Lifetime;
		bool								m_bValid;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERGENERALPROPPAGE_H__83A8B83C_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
