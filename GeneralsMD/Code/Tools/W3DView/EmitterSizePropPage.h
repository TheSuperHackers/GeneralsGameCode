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

#if !defined(AFX_EMITTERSIZEPROPPAGE_H__0CD2CBA3_1219_11D3_A034_00104B791122__INCLUDED_)
#define AFX_EMITTERSIZEPROPPAGE_H__0CD2CBA3_1219_11D3_A034_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmitterSizePropPage.h : header file
//

#include "ColorBar.H"
#include "Part_Emt.H"

// Forward delcarations
class EmitterInstanceListClass;

/////////////////////////////////////////////////////////////////////////////
//
// EmitterSizePropPageClass dialog
//
/////////////////////////////////////////////////////////////////////////////
class EmitterSizePropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterSizePropPageClass)

// Construction
public:
	EmitterSizePropPageClass(EmitterInstanceListClass *pemitter = NULL);
	~EmitterSizePropPageClass();

// Dialog Data
	//{{AFX_DATA(EmitterSizePropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_SIZE };
	CSpinButtonCtrl	m_SizeRandomSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterSizePropPageClass)
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
	//{{AFX_MSG(EmitterSizePropPageClass)
	virtual BOOL OnInitDialog();
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
		bool								Is_Data_Valid (void) const { return m_bValid; }

		void								Get_Size_Keyframes (ParticlePropertyStruct<float> &sizes)		{ sizes = m_CurrentSizes; }

		void								On_Lifetime_Changed (float lifetime);

	protected:

		/////////////////////////////////////////////////////////
		//
		//	Protected methods
		//		
		void				Initialize (void);
		void				Update_Sizes (void);

	private:

		/////////////////////////////////////////////////////////
		//
		//	Private member data
		//		
		EmitterInstanceListClass *	m_pEmitterList;
		bool								m_bValid;
		ColorBarClass *				m_SizeBar;
		ParticlePropertyStruct<float>		m_OrigSizes;
		ParticlePropertyStruct<float>		m_CurrentSizes;
		float								m_Lifetime;
		float								m_MaxSize;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERSIZEPROPPAGE_H__0CD2CBA3_1219_11D3_A034_00104B791122__INCLUDED_)
