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

#if !defined(AFX_EMITTERCOLORPROPPAGE_H__83A8B83F_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
#define AFX_EMITTERCOLORPROPPAGE_H__83A8B83F_BA3B_11D2_9FFA_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmitterColorPropPage.h : header file
//

#include "Resource.H"
#include "Vector3.H"
#include "ColorBar.H"
#include "Part_Emt.H"

// Forward declarations
class EmitterInstanceListClass;

/////////////////////////////////////////////////////////////////////////////
// EmitterColorPropPageClass dialog

class EmitterColorPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterColorPropPageClass)

// Construction
public:
	EmitterColorPropPageClass (EmitterInstanceListClass *pemitter_list = NULL);
	~EmitterColorPropPageClass ();

// Dialog Data
	//{{AFX_DATA(EmitterColorPropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_COLOR };
	CSpinButtonCtrl	m_OpacityRandomSpin;
	CSpinButtonCtrl	m_RedRandomSpin;
	CSpinButtonCtrl	m_GreenRandomSpin;
	CSpinButtonCtrl	m_BlueRandomSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterColorPropPageClass)
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
	//{{AFX_MSG(EmitterColorPropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnDeltaposRedRandomSpin(NMHDR* pNMHDR, LRESULT* pResult);
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

		void								Get_Color_Keyframes (ParticlePropertyStruct<Vector3> &colors)	{ colors = m_CurrentColors; }
		void								Get_Opacity_Keyframes (ParticlePropertyStruct<float> &opacity)	{ opacity = m_CurrentOpacities; }		
		/*const Vector3 &				Get_Start_Color (void) const { return m_StartColor; }
		const Vector3 &				Get_End_Color (void) const { return m_EndColor; }
		float								Get_Start_Opacity (void) const { return m_StartOpacity; }
		float								Get_End_Opacity (void) const { return m_EndOpacity; }
		float								Get_Fade_Time (void) const { return m_FadeTime; }*/

		void								On_Lifetime_Changed (float lifetime);

	protected:

		/////////////////////////////////////////////////////////
		//
		//	Protected methods
		//		
		void				Initialize (void);
		void				Update_Colors (void);
		void				Update_Opacities (void);

	private:

		/////////////////////////////////////////////////////////
		//
		//	Private member data
		//		
		EmitterInstanceListClass *	m_pEmitterList;
		bool								m_bValid;
		ColorBarClass *				m_ColorBar;
		ColorBarClass *				m_OpacityBar;
		ParticlePropertyStruct<Vector3>	m_OrigColors;
		ParticlePropertyStruct<float>		m_OrigOpacities;
		ParticlePropertyStruct<Vector3>	m_CurrentColors;
		ParticlePropertyStruct<float>		m_CurrentOpacities;
		float								m_Lifetime;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERCOLORPROPPAGE_H__83A8B83F_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
