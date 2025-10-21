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

#if !defined(AFX_EMITTERFRAMEPROPPAGE_H__6F068804_EAD5_4A19_BE03_C9C7B534E6E8__INCLUDED_)
#define AFX_EMITTERFRAMEPROPPAGE_H__6F068804_EAD5_4A19_BE03_C9C7B534E6E8__INCLUDED_

#pragma once
// EmitterFramePropPage.h : header file
//

#include "ColorBar.h"
#include "part_emt.h"

class EmitterInstanceListClass;

/////////////////////////////////////////////////////////////////////////////
// EmitterFramePropPageClass dialog

class EmitterFramePropPageClass : public CPropertyPage
{
	DECLARE_DYNCREATE(EmitterFramePropPageClass)

// Construction
public:
	EmitterFramePropPageClass();
	~EmitterFramePropPageClass();

// Dialog Data
	//{{AFX_DATA(EmitterFramePropPageClass)
	enum { IDD = IDD_PROP_PAGE_EMITTER_FRAME };
	CComboBox	m_FrameLayoutCombo;
	CSpinButtonCtrl	m_FrameRandomSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(EmitterFramePropPageClass)
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
	//{{AFX_MSG(EmitterFramePropPageClass)
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

	void								Get_Frame_Keyframes (ParticlePropertyStruct<float> &frames)	{ frames = m_Frames; }
	void								On_Lifetime_Changed (float lifetime);

	void								Initialize (void);
	void								Update_Frames (void);

private:


	float								Normalize_Frame(float frame);
	float								Normalize_Frame(float frame,float min,float max);
	float								Denormalize_Frame(float normalized_val);

	/////////////////////////////////////////////////////////
	//
	//	Private member data
	//
	EmitterInstanceListClass *			m_pEmitterList;
	bool										m_bValid;

	ColorBarClass *						m_FrameBar;
	ParticlePropertyStruct<float>		m_Frames;
	float										m_Lifetime;
	float										m_MinFrame;
	float										m_MaxFrame;

};


inline float EmitterFramePropPageClass::Normalize_Frame(float frame)
{
	return (frame - m_MinFrame) / (m_MaxFrame - m_MinFrame);
}

inline float EmitterFramePropPageClass::Normalize_Frame(float frame,float min,float max)
{
	return (frame - min) / (max - min);
}

inline float EmitterFramePropPageClass::Denormalize_Frame(float normalized_val)
{
	return normalized_val * (m_MaxFrame - m_MinFrame) + m_MinFrame;
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMITTERFRAMEPROPPAGE_H__6F068804_EAD5_4A19_BE03_C9C7B534E6E8__INCLUDED_)
