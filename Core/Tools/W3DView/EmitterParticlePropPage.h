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

#if !defined(AFX_EMITTERPARTICLEPROPPAGE_H__83A8B83D_BA3B_11D2_9FFA_00104B791122__INCLUDED_)
#define AFX_EMITTERPARTICLEPROPPAGE_H__83A8B83D_BA3B_11D2_9FFA_00104B791122__INCLUDED_

#pragma once
// EmitterParticlePropPage.h : header file
//

#include "resource.h"
#include "vector3.h"
#include "v3_rnd.h"

// Forward delcarations
class EmitterInstanceListClass;
//class Vector3Randomizer;

/////////////////////////////////////////////////////////////////////////////
// EmitterParticlePropPageClass dialog

class EmitterParticlePropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterParticlePropPageClass)

// Construction
public:
	EmitterParticlePropPageClass (EmitterInstanceListClass *pemitter_list = NULL);
	~EmitterParticlePropPageClass ();

// Dialog Data
	//{{AFX_DATA(EmitterParticlePropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_PARTICLE };
	CSpinButtonCtrl	m_BurstSizeSpin;
	CSpinButtonCtrl	m_EmitionRateSpin;
	CSpinButtonCtrl	m_MaxParticlesSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterParticlePropPageClass)
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(EmitterParticlePropPageClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnSpecifyCreationVolume();
	afx_msg void OnMaxParticlesCheck();
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

		float								Get_Rate (void) const				{ return m_Rate; }
		int								Get_Burst_Size (void) const		{ return m_BurstSize; }
		int								Get_Max_Particles (void) const	{ return m_MaxParticles; }
		Vector3Randomizer *			Get_Creation_Volume (void) const	{ return m_Randomizer->Clone (); }

	protected:

		/////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void								Initialize (void);

	private:

		/////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		EmitterInstanceListClass *	m_pEmitterList;
		bool								m_bValid;
		float								m_Rate;
		int								m_BurstSize;
		int								m_MaxParticles;
		Vector3Randomizer *			m_Randomizer;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERPARTICLEPROPPAGE_H__83A8B83D_BA3B_11D2_9FFA_00104B791122__INCLUDED_)

