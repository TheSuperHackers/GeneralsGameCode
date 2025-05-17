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

#if !defined(AFX_SPHERESIZEPROPPAGE_H__E86BBE8A_F527_11D3_A08F_00104B791122__INCLUDED_)
#define AFX_SPHERESIZEPROPPAGE_H__E86BBE8A_F527_11D3_A08F_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SphereSizePropPage.h : header file
//

#include "resource.h"
#include "sphereobj.h"
#include "colorbar.h"

/////////////////////////////////////////////////////////////////////////////
//
// SphereSizePropPageClass
//
/////////////////////////////////////////////////////////////////////////////
class SphereSizePropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(SphereSizePropPageClass)

// Construction
public:
	SphereSizePropPageClass(SphereRenderObjClass *sphere = NULL);
	~SphereSizePropPageClass();

// Dialog Data
	//{{AFX_DATA(SphereSizePropPageClass)
	enum { IDD = IDD_PROP_PAGE_SPHERE_SCALE };
	CSpinButtonCtrl	m_SizeZSpin;
	CSpinButtonCtrl	m_SizeYSpin;
	CSpinButtonCtrl	m_SizeXSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(SphereSizePropPageClass)
	public:
	virtual BOOL OnApply();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(SphereSizePropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:

	/////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////

	//
	//	Inline accessors
	//
	
	SphereRenderObjClass *		Get_Sphere (void) const							{ return m_RenderObj; }
	void								Set_Sphere (SphereRenderObjClass *sphere)	{ m_RenderObj = sphere; Initialize (); }
	bool								Is_Data_Valid (void) const						{ return m_bValid; }

protected:

	/////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////
	void				Initialize (void);
	void				Update_Scale_Array (void);

private:

	/////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////
	SphereRenderObjClass *			m_RenderObj;
	bool									m_bValid;
	ColorBarClass *					m_ScaleXBar;
	ColorBarClass *					m_ScaleYBar;
	ColorBarClass *					m_ScaleZBar;
	Vector3								m_Size;

	SphereScaleChannelClass			m_ScaleChannel;
	SphereScaleChannelClass			m_OrigScaleChannel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPHERESIZEPROPPAGE_H__E86BBE8A_F527_11D3_A08F_00104B791122__INCLUDED_)
