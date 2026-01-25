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

#pragma once

// EmitterLineGroupPropPage.h : header file
//

#include "part_emt.h"

// Forward declarations

class EmitterInstanceListClass;
class ColorBarClass;


/////////////////////////////////////////////////////////////////////////////
// EmitterLineGroupPropPageClass dialog

class EmitterLineGroupPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterLineGroupPropPageClass)

// Construction
public:
	EmitterLineGroupPropPageClass();
	~EmitterLineGroupPropPageClass();

// Dialog Data
	//{{AFX_DATA(EmitterLineGroupPropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_LINEGROUP };
	CSpinButtonCtrl	m_BlurTimeRandomSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterLineGroupPropPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(EmitterLineGroupPropPageClass)
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

	void								Get_Blur_Time_Keyframes (ParticlePropertyStruct<float> &blurtimes)	{ blurtimes = m_BlurTimes; }
	void								On_Lifetime_Changed (float lifetime);

	void								Initialize (void);
	void								Update_Blur_Times (void);

private:

	float								Normalize_Blur_Time(float blur);
	float								Normalize_Blur_Time(float blur,float min,float max);
	float								Denormalize_Blur_Time(float normalized_val);

	/////////////////////////////////////////////////////////
	//
	//	Private member data
	//
	EmitterInstanceListClass *			m_pEmitterList;
	bool										m_bValid;

	ColorBarClass *						m_BlurTimeBar;
	ParticlePropertyStruct<float>		m_BlurTimes;
	float										m_Lifetime;
	float										m_MinBlurTime;
	float										m_MaxBlurTime;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
