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

#if !defined(AFX_EMITTERPHYSICSPROPPAGE_H__83A8B83E_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
#define AFX_EMITTERPHYSICSPROPPAGE_H__83A8B83E_BA3B_11D2_9FFA_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmitterPhysicsPropPage.h : header file
//

#include "Resource.H"
#include "Vector3.H"

// Forward delcarations
class EmitterInstanceListClass;
class Vector3Randomizer;

/////////////////////////////////////////////////////////////////////////////
// EmitterPhysicsPropPageClass dialog

class EmitterPhysicsPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterPhysicsPropPageClass)

// Construction
public:
	EmitterPhysicsPropPageClass (EmitterInstanceListClass *pemitter_list = NULL);
	~EmitterPhysicsPropPageClass ();

// Dialog Data
	//{{AFX_DATA(EmitterPhysicsPropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_PHYSICS };
	CSpinButtonCtrl	m_OutSpin;
	CSpinButtonCtrl	m_InheritanceSpin;
	CSpinButtonCtrl	m_VelocityZSpin;
	CSpinButtonCtrl	m_VelocityYSpin;
	CSpinButtonCtrl	m_VelocityXSpin;
	CSpinButtonCtrl	m_AccelZSpin;
	CSpinButtonCtrl	m_AccelYSpin;
	CSpinButtonCtrl	m_AccelXSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterPhysicsPropPageClass)
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
	//{{AFX_MSG(EmitterPhysicsPropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnSpecifyVelocityRandom();
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

		Vector3Randomizer *			Get_Velocity_Random (void) const	{ return m_Randomizer; }
		const Vector3 &				Get_Velocity (void) const			{ return m_Velocity; }
		const Vector3 &				Get_Acceleration (void) const		{ return m_Acceleration; }
		float								Get_Out_Factor (void) const		{ return m_OutFactor; }
		float								Get_Inheritance_Factor (void) const	{ return m_InheritanceFactor; }

	protected:

		/////////////////////////////////////////////////////////
		//
		//	Protected methods
		//		
		void								Initialize (void);
		void								On_Setting_Changed (UINT ctrl_id);
	
	private:

		/////////////////////////////////////////////////////////
		//
		//	Private member data
		//		
		EmitterInstanceListClass *	m_pEmitterList;
		bool								m_bValid;

		Vector3							m_Velocity;
		Vector3							m_Acceleration;
		float								m_OutFactor;
		float								m_InheritanceFactor;
		Vector3Randomizer *			m_Randomizer;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERPHYSICSPROPPAGE_H__83A8B83E_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
