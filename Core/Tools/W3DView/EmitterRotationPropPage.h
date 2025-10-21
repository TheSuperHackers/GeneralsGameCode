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

#if !defined(AFX_EMITTERROTATIONPROPPAGE_H__35F8A2F6_C91C_4750_BF06_100433C1ECBB__INCLUDED_)
#define AFX_EMITTERROTATIONPROPPAGE_H__35F8A2F6_C91C_4750_BF06_100433C1ECBB__INCLUDED_

#pragma once
// EmitterRotationPropPage.h : header file
//

#include "ColorBar.h"
#include "part_emt.h"

// Forward delcarations
class EmitterInstanceListClass;


/////////////////////////////////////////////////////////////////////////////
// EmitterRotationPropPageClass dialog

class EmitterRotationPropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterRotationPropPageClass)

// Construction
public:
	EmitterRotationPropPageClass();
	~EmitterRotationPropPageClass();

// Dialog Data
	//{{AFX_DATA(EmitterRotationPropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_ROTATION };
	CSpinButtonCtrl	m_InitialOrientationRandomSpin;
	CSpinButtonCtrl	m_RotationRandomSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterRotationPropPageClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(EmitterRotationPropPageClass)
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

	void								Get_Rotation_Keyframes (ParticlePropertyStruct<float> &rotations)		{ rotations = m_Rotations; }
	void								On_Lifetime_Changed (float lifetime);

protected:

	/////////////////////////////////////////////////////////
	//
	//	Protected methods
	//
	void								Initialize (void);
	void								Update_Rotations (void);

private:

	float								Normalize_Rotation(float rot);
	float								Normalize_Rotation(float rot,float min,float max);
	float								Denormalize_Rotation(float normalized_val);

	/////////////////////////////////////////////////////////
	//
	//	Private member data
	//
	EmitterInstanceListClass *			m_pEmitterList;
	bool										m_bValid;
	ColorBarClass *						m_RotationBar;

	ParticlePropertyStruct<float>		m_Rotations;

	float										m_Lifetime;
	float										m_MinRotation;
	float										m_MaxRotation;
	float										m_InitialOrientationRandom;

};

inline float EmitterRotationPropPageClass::Normalize_Rotation(float rot)
{
	return (rot - m_MinRotation) / (m_MaxRotation - m_MinRotation);
}

inline float EmitterRotationPropPageClass::Normalize_Rotation(float rot,float min,float max)
{
	return (rot - min) / (max - min);
}

inline float EmitterRotationPropPageClass::Denormalize_Rotation(float normalized_val)
{
	return normalized_val * (m_MaxRotation - m_MinRotation) + m_MinRotation;
}




//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERROTATIONPROPPAGE_H__35F8A2F6_C91C_4750_BF06_100433C1ECBB__INCLUDED_)
