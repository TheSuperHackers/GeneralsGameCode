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

#if !defined(AFX_SCENELIGHTDIALOG_H__AF3CDE89_61EC_11D2_9FC7_00104B791122__INCLUDED_)
#define AFX_SCENELIGHTDIALOG_H__AF3CDE89_61EC_11D2_9FC7_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SceneLightDialog.h : header file
//

// Forward declarations
class Vector3;

/////////////////////////////////////////////////////////////////////////////
//
// CSceneLightDialog dialog
//
class CSceneLightDialog : public CDialog
{
// Construction
public:
	CSceneLightDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSceneLightDialog)
	enum { IDD = IDD_LIGHT_SCENE_DIALOG };
	CSpinButtonCtrl	m_StartAttenSpin;
	CSpinButtonCtrl	m_EndAttenSpin;
	CSpinButtonCtrl	m_DistanceSpin;
	CSliderCtrl	m_IntensitySlider;
	CSliderCtrl	m_blueSlider;
	CSliderCtrl	m_greenSlider;
	CSliderCtrl	m_redSlider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSceneLightDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSceneLightDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	virtual void OnCancel();
	afx_msg void OnGrayscaleCheck();
	afx_msg void OnChannelBothRadio();
	afx_msg void OnChannelDiffuseRadio();
	afx_msg void OnChannelSpecularRadio();
	afx_msg void OnAttenuationCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	protected:

		////////////////////////////////////////////////////////////////////
		//
		//	Protected methods
		//
		void				Set_Color_Control_State (const Vector3 &color);
		void				Update_Light (const Vector3 &color);
		void				Update_Distance (float distance);
		void				Update_Attenuation (void);
		void				Update_Attenuation_Controls (void);

	private:

		////////////////////////////////////////////////////////////////////
		//
		//	Private data types
		//
		typedef enum
		{
			DIFFUSE		= 2,
			SPECULAR		= 4,
			BOTH			= DIFFUSE | SPECULAR
		} CHANNEL;


		////////////////////////////////////////////////////////////////////
		//
		//	Private member data
		//
		int		m_InitialRedDiffuse;
		int		m_InitialGreenDiffuse;
		int		m_InitialBlueDiffuse;
		int		m_InitialRedSpecular;
		int		m_InitialGreenSpecular;
		int		m_InitialBlueSpecular;
		CHANNEL	m_CurrentChannel;

		float		m_InitialStartAtten;
		float		m_InitialEndAtten;
		float		m_InitialDistance;
		float		m_InitialIntensity;
		BOOL		m_InitialAttenOn;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCENELIGHTDIALOG_H__AF3CDE89_61EC_11D2_9FC7_00104B791122__INCLUDED_)
