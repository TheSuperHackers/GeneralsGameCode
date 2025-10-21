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

#if !defined(AFX_RINGCOLORPROPPAGE_H__E86BBE86_F527_11D3_A08F_00104B791122__INCLUDED_)
#define AFX_RINGCOLORPROPPAGE_H__E86BBE86_F527_11D3_A08F_00104B791122__INCLUDED_

#pragma once
// RingColorPropPage.h : header file
//

#include "resource.h"
#include "ringobj.h"
#include "ColorBar.h"

/////////////////////////////////////////////////////////////////////////////
//
// RingColorPropPageClass
//
/////////////////////////////////////////////////////////////////////////////
class RingColorPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(RingColorPropPageClass)

// Construction
public:
	RingColorPropPageClass (RingRenderObjClass *ring = NULL);
	~RingColorPropPageClass ();

// Dialog Data
	//{{AFX_DATA(RingColorPropPageClass)
	enum { IDD = IDD_PROP_PAGE_RING_COLOR };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(RingColorPropPageClass)
	public:
	virtual BOOL OnApply();
	virtual void OnCancel();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(RingColorPropPageClass)
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

	RingRenderObjClass *		Get_Ring (void) const							{ return m_RenderObj; }
	void							Set_Ring (RingRenderObjClass *ring)	{ m_RenderObj = ring; Initialize (); }
	bool							Is_Data_Valid (void) const						{ return m_bValid; }

protected:

	/////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////
	void				Initialize (void);
	void				Update_Colors (void);
	void				Update_Opacities (void);

private:

	/////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////
	RingRenderObjClass *		m_RenderObj;
	bool							m_bValid;
	ColorBarClass *			m_ColorBar;
	ColorBarClass *			m_OpacityBar;

	RingColorChannelClass	m_ColorChannel;
	RingColorChannelClass	m_OrigColorChannel;
	RingAlphaChannelClass	m_AlphaChannel;
	RingAlphaChannelClass	m_OrigAlphaChannel;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RINGCOLORPROPPAGE_H__E86BBE86_F527_11D3_A08F_00104B791122__INCLUDED_)

