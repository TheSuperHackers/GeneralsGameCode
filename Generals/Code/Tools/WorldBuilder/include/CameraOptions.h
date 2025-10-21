/*
**	Command & Conquer Generals(tm)
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

#if !defined(AFX_CAMERAOPTIONS_H__4EF4F775_1290_47AE_817F_9340BA3A898C__INCLUDED_)
#define AFX_CAMERAOPTIONS_H__4EF4F775_1290_47AE_817F_9340BA3A898C__INCLUDED_

#pragma once
// CameraOptions.h : header file
//

#define  CAMERA_OPTIONS_PANEL_SECTION "CameraOptionsWindow"
#include "WBPopupSlider.h"

/////////////////////////////////////////////////////////////////////////////
// CameraOptions dialog

class CameraOptions : public CDialog, public PopupSliderOwner
{
// Construction
public:
	CameraOptions(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CameraOptions)
	enum { IDD = IDD_CAMERA_OPTIONS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CameraOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CameraOptions)
	afx_msg void OnCameraReset();
	afx_msg void OnMove(int x, int y);
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePitchEdit();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	void	putInt( Int ctrlID, Int val );
	void	putReal( Int ctrlID, Real val );
	void	putAsciiString( Int ctrlID, AsciiString val );
	BOOL	getReal( Int ctrlID, Real *rVal );
	void	stuffValuesIntoFields( void );
	void	applyCameraPitch( Real pitch );

	WBPopupSliderButton m_pitchPopup;
	Bool	m_updating;

public: // popup slider interface.

	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);

public:
	void update( void );

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMERAOPTIONS_H__4EF4F775_1290_47AE_817F_9340BA3A898C__INCLUDED_)
