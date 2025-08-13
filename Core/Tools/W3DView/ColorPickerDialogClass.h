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

#if !defined(AFX_COLORPICKERDIALOGCLASS_H__C943A7F1_CAA3_11D2_8DDF_00104B6FD9E3__INCLUDED_)
#define AFX_COLORPICKERDIALOGCLASS_H__C943A7F1_CAA3_11D2_8DDF_00104B6FD9E3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ColorPickerDialogClass.h : header file
//

#include "resource.h"
#include "ColorUtils.h"


/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
class ColorBarClass;
class ColorPickerClass;

/////////////////////////////////////////////////////////////////////////////
//
// ColorPickerDialogClass dialog
//
class ColorPickerDialogClass : public CDialog
{
// Construction
public:
	ColorPickerDialogClass (int red, int green, int blue, CWnd* pParent = NULL, UINT res_id = ColorPickerDialogClass::IDD);

// Dialog Data
	//{{AFX_DATA(ColorPickerDialogClass)
	enum { IDD = IDD_COLOR_PICKER };
	CSpinButtonCtrl	m_BlueSpin;
	CSpinButtonCtrl	m_GreenSpin;
	CSpinButtonCtrl	m_RedSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ColorPickerDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ColorPickerDialogClass)
	virtual BOOL OnInitDialog();
	afx_msg void OnReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	public:

		/////////////////////////////////////////////////////////////
		//	Public methods
		/////////////////////////////////////////////////////////////
		int					Get_Red (void) const { return (int)m_CurrentRed; }
		int					Get_Green (void) const { return (int)m_CurrentGreen; }
		int					Get_Blue (void) const { return (int)m_CurrentBlue; }
		void					Set_Color (int r, int g, int b)
									{ Update_Color((float)r, (float)g, (float)b); }
		void					Set_Original_Color (int r, int g, int b);
		void					Create_Form (CWnd *parent);
		void					Set_Update_Callback (WWCTRL_COLORCALLBACK callme, void *arg)
									{ m_UpdateCallback = callme; m_CallArg = arg; }

	protected:

		/////////////////////////////////////////////////////////////
		//	Protected methods
		/////////////////////////////////////////////////////////////
		void					Update_Red_Bar (void);
		void					Update_Green_Bar (void);
		void					Update_Blue_Bar (void);
		void					Update_Current_Color_Bar (void);
		void					Update_Whiteness_Bar (void);
		void					Update_Color (float red, float green, float blue, DWORD flags = 0xFFFFFFFF);

	private:

		/////////////////////////////////////////////////////////////
		//	Private member data
		/////////////////////////////////////////////////////////////
		float	m_OrigRed;
		float	m_OrigGreen;
		float	m_OrigBlue;

		float	m_CurrentRed;
		float	m_CurrentGreen;
		float	m_CurrentBlue;

		ColorBarClass *	m_CurrentColorBar;
		ColorBarClass *	m_OrigColorBar;
		ColorBarClass *	m_RedColorBar;
		ColorBarClass *	m_GreenColorBar;
		ColorBarClass *	m_BlueColorBar;
		ColorBarClass *	m_WhitenessColorBar;
		ColorPickerClass *m_HuePicker;

		bool	m_bDeleteOnClose;

		// Callback function when color is updated.
		WWCTRL_COLORCALLBACK m_UpdateCallback;
		void *					m_CallArg;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORPICKERDIALOGCLASS_H__C943A7F1_CAA3_11D2_8DDF_00104B6FD9E3__INCLUDED_)
