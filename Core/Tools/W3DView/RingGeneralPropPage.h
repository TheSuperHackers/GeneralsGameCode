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

#if !defined(AFX_RINGGENERALPROPPAGE_H__E86BBE83_F527_11D3_A08F_00104B791122__INCLUDED_)
#define AFX_RINGGENERALPROPPAGE_H__E86BBE83_F527_11D3_A08F_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RingGeneralPropPage.h : header file
//

#include "resource.h"
#include "ringobj.h"
#include "shader.h"

/////////////////////////////////////////////////////////////////////////////
//
// RingGeneralPropPageClass
//
/////////////////////////////////////////////////////////////////////////////
class RingGeneralPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(RingGeneralPropPageClass)

// Construction
public:
	RingGeneralPropPageClass (RingRenderObjClass *ring = NULL);
	~RingGeneralPropPageClass ();

// Dialog Data
	//{{AFX_DATA(RingGeneralPropPageClass)
	enum { IDD = IDD_PROP_PAGE_RING_GEN };
	CSpinButtonCtrl	m_TextureTileSpin;
	CSpinButtonCtrl	m_LifetimeSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(RingGeneralPropPageClass)
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
	//{{AFX_MSG(RingGeneralPropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseButton();
	afx_msg void OnChangeFilenameEdit();
	afx_msg void OnChangeNameEdit();
	afx_msg void OnChangeLifetimeEdit();
	afx_msg void OnSelchangeShaderCombo();
	afx_msg void OnChangeTextureTileEdit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:

	/////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////

	//
	//	Inline accessors
	//
	RingRenderObjClass *	Get_Ring (void) const					{ return m_RenderObj; }
	void						Set_Ring (RingRenderObjClass *ring)	{ m_RenderObj = ring; Initialize (); }
	bool						Is_Data_Valid (void) const				{ return m_bValid; }

	const CString &		Get_Name (void) const					{ return m_Name; }
	const CString &		Get_Texture_Filename (void) const	{ return m_TextureFilename; }
	float						Get_Lifetime (void) const				{ return m_Lifetime; }
	const ShaderClass &	Get_Shader (void) const					{ return m_Shader; }

protected:

	/////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////
	void						Initialize (void);
	void						Add_Shader_To_Combo (ShaderClass &shader, LPCTSTR name);

private:

	/////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////
	RingRenderObjClass *		m_RenderObj;
	CString						m_Name;
	CString						m_TextureFilename;
	ShaderClass					m_Shader;
	float							m_Lifetime;
	bool							m_bValid;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RINGGENERALPROPPAGE_H__E86BBE83_F527_11D3_A08F_00104B791122__INCLUDED_)
