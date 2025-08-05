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

#if !defined(AFX_RINGSIZEPROPPAGE_H__E86BBE8A_F527_11D3_A08F_00104B791122__INCLUDED_)
#define AFX_RINGSIZEPROPPAGE_H__E86BBE8A_F527_11D3_A08F_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RingSizePropPage.h : header file
//

#include "resource.h"
#include "ringobj.h"
#include "ColorBar.h"

/////////////////////////////////////////////////////////////////////////////
//
// RingSizePropPageClass
//
/////////////////////////////////////////////////////////////////////////////
class RingSizePropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(RingSizePropPageClass)

// Construction
public:
	RingSizePropPageClass(RingRenderObjClass *ring = NULL);
	~RingSizePropPageClass();

// Dialog Data
	//{{AFX_DATA(RingSizePropPageClass)
	enum { IDD = IDD_PROP_PAGE_RING_SCALE };
	CSpinButtonCtrl	m_InnerSizeXSpin;
	CSpinButtonCtrl	m_InnerSizeYSpin;
	CSpinButtonCtrl	m_OuterSizeXSpin;
	CSpinButtonCtrl	m_OuterSizeYSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(RingSizePropPageClass)
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
	//{{AFX_MSG(RingSizePropPageClass)
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

	RingRenderObjClass *		Get_Ring (void) const					{ return m_RenderObj; }
	void							Set_Ring (RingRenderObjClass *ring)	{ m_RenderObj = ring; Initialize (); }
	bool							Is_Data_Valid (void) const				{ return m_bValid; }

protected:

	/////////////////////////////////////////////////////////
	//	Protected methods
	/////////////////////////////////////////////////////////
	void				Initialize (void);
	void				Update_Inner_Scale_Array (void);
	void				Update_Outer_Scale_Array (void);

private:

	/////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////
	RingRenderObjClass *				m_RenderObj;
	bool									m_bValid;
	ColorBarClass *					m_InnerScaleXBar;
	ColorBarClass *					m_InnerScaleYBar;
	ColorBarClass *					m_OuterScaleXBar;
	ColorBarClass *					m_OuterScaleYBar;
	Vector2								m_InnerSize;
	Vector2								m_OuterSize;

	RingScaleChannelClass			m_InnerScaleChannel;
	RingScaleChannelClass			m_OrigInnerScaleChannel;

	RingScaleChannelClass			m_OuterScaleChannel;
	RingScaleChannelClass			m_OrigOuterScaleChannel;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RINGSIZEPROPPAGE_H__E86BBE8A_F527_11D3_A08F_00104B791122__INCLUDED_)
