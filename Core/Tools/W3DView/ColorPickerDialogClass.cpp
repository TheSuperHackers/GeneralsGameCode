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

// ColorPickerDialogClass.cpp : implementation file
//

#include "stdafx.h"
#include "colorpickerdialogclass.h"
#include "colorbar.h"
#include "colorpicker.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern HINSTANCE _hinstance;

/////////////////////////////////////////////////////////////////////////////
//
// Local constants
//
/////////////////////////////////////////////////////////////////////////////
const DWORD	UPDATE_COLOR_BARS		= 0x00000001;
const DWORD	UPDATE_WHITENESS		= 0x00000002;
const DWORD	UPDATE_HUE_PICKER		= 0x00000004;

/*class MyManageStateClass
{
	public:
		MyManageStateClass (void)	
		{
			m_hResHandle = ::AfxGetResourceHandle ();
			::AfxSetResourceHandle (_hinstance);
		}

		~MyManageStateClass (void)	{ ::AfxSetResourceHandle (m_hResHandle); }

	private:
		HINSTANCE m_hResHandle;
};*/

//#define MY_MANAGE_STATE()	MyManageStateClass _xmystate;

HWND
Create_Color_Picker_Form (HWND parent, int red, int green, int blue)
{
	//MY_MANAGE_STATE ()

	CWnd *parent_wnd = CWnd::FromHandle (parent);

	ColorPickerDialogClass *dialog = new ColorPickerDialogClass (red, green, blue, parent_wnd, IDD_COLOR_FORM);
	dialog->Create_Form (parent_wnd);

	//HINSTANCE old_handle = ::AfxGetResourceHandle ();
	//::AfxSetResourceHandle (_hinstance);
	return dialog->m_hWnd;
}

BOOL
Get_Form_Color (HWND form_wnd, int *red, int *green, int *blue)
{
	//MY_MANAGE_STATE ()

	BOOL retval = FALSE;

	ColorPickerDialogClass *dialog = (ColorPickerDialogClass *)::GetProp (form_wnd, "COLORPICKERDLGCLASS");
	if (dialog != NULL) {
		(*red)	= dialog->Get_Red ();
		(*green)	= dialog->Get_Green ();
		(*blue)	= dialog->Get_Blue ();
		retval = TRUE;
	}

	return retval;
}


BOOL
Set_Form_Color (HWND form_wnd, int red, int green, int blue)
{
	//MY_MANAGE_STATE ()

	BOOL retval = FALSE;

	ColorPickerDialogClass *dialog = (ColorPickerDialogClass *)::GetProp (form_wnd, "COLORPICKERDLGCLASS");
	if (dialog != NULL) {
		dialog->Set_Color (red, green, blue);
		retval = TRUE;
	}

	return retval;
}


BOOL
Set_Form_Original_Color (HWND form_wnd, int red, int green, int blue)
{
	//MY_MANAGE_STATE ()

	BOOL retval = FALSE;

	ColorPickerDialogClass *dialog = (ColorPickerDialogClass *)::GetProp (form_wnd, "COLORPICKERDLGCLASS");
	if (dialog != NULL) {
		dialog->Set_Original_Color (red, green, blue);
		retval = TRUE;
	}

	return retval;
}


BOOL
Show_Color_Picker (int *red, int *green, int *blue)
{
	//MY_MANAGE_STATE ()

	BOOL retval = FALSE;

	ColorPickerDialogClass dialog (*red, *green, *blue);
	if (dialog.DoModal () == IDOK) {
		(*red)	= dialog.Get_Red ();
		(*green)	= dialog.Get_Green ();
		(*blue)	= dialog.Get_Blue ();
		retval = TRUE;
	}

	return retval;
}


BOOL
Set_Update_Callback (HWND form_wnd, WWCTRL_COLORCALLBACK callback, void *arg)
{
	//MY_MANAGE_STATE()

	BOOL retval = FALSE;

	ColorPickerDialogClass *dialog = (ColorPickerDialogClass *)::GetProp (form_wnd, "COLORPICKERDLGCLASS");
	if (dialog != NULL) {
		dialog->Set_Update_Callback(callback, arg);
		retval = TRUE;
	}

	return retval;
}

/////////////////////////////////////////////////////////////////////////////
//
// ColorPickerDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ColorPickerDialogClass::ColorPickerDialogClass
(
	int red,
	int green,
	int blue,
	CWnd *pParent,
	UINT res_id
)
	:	m_OrigRed ((float)red),
		m_OrigGreen ((float)green),
		m_OrigBlue ((float)blue),
		m_CurrentRed ((float)red),
		m_CurrentGreen ((float)green),
		m_CurrentBlue ((float)blue),
		m_CurrentColorBar (NULL),
		m_OrigColorBar (NULL),
		m_RedColorBar (NULL),
		m_GreenColorBar (NULL),
		m_BlueColorBar (NULL),
		m_WhitenessColorBar (NULL),
		m_HuePicker (NULL),
		m_bDeleteOnClose (false),
		m_UpdateCallback(NULL),
		CDialog(res_id, pParent)
{
	//{{AFX_DATA_INIT(ColorPickerDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create_Form
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::Create_Form (CWnd *parent)
{
	Create (IDD_COLOR_FORM, parent);
	SetProp (m_hWnd, "COLORPICKERDLGCLASS", (HANDLE)this);
	m_bDeleteOnClose = true;
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ColorPickerDialogClass)
	DDX_Control(pDX, IDC_BLUE_SPIN, m_BlueSpin);
	DDX_Control(pDX, IDC_GREEN_SPIN, m_GreenSpin);
	DDX_Control(pDX, IDC_RED_SPIN, m_RedSpin);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ColorPickerDialogClass, CDialog)
	//{{AFX_MSG_MAP(ColorPickerDialogClass)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ColorPickerDialogClass::OnInitDialog (void) 
{
	CDialog::OnInitDialog ();

	//
	//	Setup the spin controls
	//
	m_BlueSpin.SetRange (0, 255);
	m_GreenSpin.SetRange (0, 255);
	m_RedSpin.SetRange (0, 255);
	m_BlueSpin.SetPos ((int)m_CurrentBlue);
	m_GreenSpin.SetPos ((int)m_CurrentGreen);
	m_RedSpin.SetPos ((int)m_CurrentRed);

	//
	//	Get control of all the color bars on the dialog
	//
	m_CurrentColorBar		= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_CURRENT_COLOR_BAR));
	m_OrigColorBar			= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_ORIG_COLOR_BAR));
	m_RedColorBar			= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_RED_BAR));
	m_GreenColorBar		= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_GREEN_BAR));
	m_BlueColorBar			= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_BLUE_BAR));
	m_WhitenessColorBar	= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_WHITENESS_BAR));	
	m_HuePicker				= ColorPickerClass::Get_Color_Picker (::GetDlgItem (m_hWnd, IDC_HUE_PICKER));	
	
	// Setup the original color bar
	m_OrigColorBar->Modify_Point (0, 0, m_OrigRed, m_OrigGreen, m_OrigBlue);
	m_HuePicker->Select_Color ((int)m_OrigRed, (int)m_OrigGreen, (int)m_OrigBlue);
	//m_WhitenessColorBar->Modify_Point (0, 0, m_OrigRed, m_OrigGreen, m_OrigBlue);

	m_RedColorBar->Set_Range (0, 255);
	m_GreenColorBar->Set_Range (0, 255);
	m_BlueColorBar->Set_Range (0, 255);
	m_WhitenessColorBar->Set_Range (0, 255);

	m_WhitenessColorBar->Insert_Point (1, 255, 255, 255, 255);
	
	//
	//	Setup the red/green/blue color bars
	//
	//m_RedColorBar->Insert_Point (1, 1, 255, 0, 0);
	//m_GreenColorBar->Insert_Point (1, 1, 0, 255, 0);
	//m_BlueColorBar->Insert_Point (1, 1, 0, 0, 255);

	//
	//	Update the remaining color bars to reflect the initial color
	//
	Update_Red_Bar ();
	Update_Green_Bar ();
	Update_Blue_Bar ();
	Update_Current_Color_Bar ();
	Update_Whiteness_Bar ();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Red_Bar
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::Update_Red_Bar (void)
{
	m_RedColorBar->Set_Selection_Pos (m_CurrentRed);
	m_RedColorBar->Modify_Point (0, 0, 0, (float)m_CurrentGreen, (float)m_CurrentBlue);
	m_RedColorBar->Modify_Point (1, 255, 255, (float)m_CurrentGreen, (float)m_CurrentBlue);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Green_Bar
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::Update_Green_Bar (void)
{
	m_GreenColorBar->Set_Selection_Pos (m_CurrentGreen);
	m_GreenColorBar->Modify_Point (0, 0, m_CurrentRed, 0, m_CurrentBlue);
	m_GreenColorBar->Modify_Point (1, 255, m_CurrentRed, 255, m_CurrentBlue);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Blue_Bar
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::Update_Blue_Bar (void)
{
	m_BlueColorBar->Set_Selection_Pos (m_CurrentBlue);
	m_BlueColorBar->Modify_Point (0, 0, m_CurrentRed, m_CurrentGreen, 0);
	m_BlueColorBar->Modify_Point (1, 255, m_CurrentRed, m_CurrentGreen, 255);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Current_Color_Bar
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::Update_Current_Color_Bar (void)
{
	m_CurrentColorBar->Modify_Point (0, 0, m_CurrentRed, m_CurrentGreen, m_CurrentBlue);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Whiteness_Bar
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::Update_Whiteness_Bar (void)
{
	int red = 0;
	int green = 0;
	int blue = 0;
	m_HuePicker->Get_Current_Color (&red, &green, &blue);

	//
	//	Given the current color, determine the 'whiteness' and update
	//
	float whiteness = min (m_CurrentRed, m_CurrentGreen);	
	whiteness = min (whiteness, m_CurrentBlue);
	float percent = whiteness / 255;
	m_WhitenessColorBar->Set_Selection_Pos (whiteness);

	m_WhitenessColorBar->Modify_Point (0, 0, (float)red, (float)green, (float)blue);

	// Can we extrapolate the starting color from the whiteness factor?
	/*if (percent == 1) {
		m_WhitenessColorBar->Modify_Point (0, 0, 0, 0, 0);
	} else {
		
		//
		//	Extrapolate the starting color
		//
		float start_red = (m_CurrentRed - whiteness) / (1 - percent);
		float start_green = (m_CurrentGreen - whiteness) / (1 - percent);
		float start_blue = (m_CurrentBlue - whiteness) / (1 - percent);
		m_WhitenessColorBar->Modify_Point (0, 0, start_red, start_green, start_blue);
	}*/

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnNotify
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ColorPickerDialogClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
)
{
	CBR_NMHDR *color_bar_hdr = (CBR_NMHDR *)lParam;
	switch (color_bar_hdr->hdr.idFrom)
	{
		case IDC_RED_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_SEL_CHANGED) {
				float red = color_bar_hdr->red;
				float green = m_CurrentGreen;
				float blue = m_CurrentBlue;
				Update_Color (red, green, blue);
			}
		}
		break;

		case IDC_GREEN_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_SEL_CHANGED) {
				float red = m_CurrentRed;
				float green = color_bar_hdr->green;
				float blue = m_CurrentBlue;
				Update_Color (red, green, blue);
			}
		}
		break;

		case IDC_BLUE_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_SEL_CHANGED) {
				float red = m_CurrentRed;
				float green = m_CurrentGreen;
				float blue = color_bar_hdr->blue;
				Update_Color (red, green, blue);
			}
		}
		break;

		case IDC_WHITENESS_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_SEL_CHANGED) {
				float red = color_bar_hdr->red;
				float green = color_bar_hdr->green;
				float blue = color_bar_hdr->blue;
				Update_Color (red, green, blue, UPDATE_COLOR_BARS);
			}
		}
		break;

		case IDC_HUE_PICKER:
		{
			CP_NMHDR *picker_hdr = (CP_NMHDR *)lParam;
			if (picker_hdr->hdr.code == CPN_COLORCHANGE) {
				float red = picker_hdr->red;
				float green = picker_hdr->green;
				float blue = picker_hdr->blue;
				float whiteness = m_WhitenessColorBar->Get_Selection_Pos () / 255;
				red = red + ((255 - red) * whiteness);
				green = green + ((255 - green) * whiteness);
				blue = blue + ((255 - blue) * whiteness);
				Update_Color (red, green, blue, UPDATE_COLOR_BARS | UPDATE_WHITENESS);
			}			
		}
		break;

		case IDC_RED_SPIN:
		case IDC_GREEN_SPIN:
		case IDC_BLUE_SPIN:
		{
			if (color_bar_hdr->hdr.code == UDN_DELTAPOS) {
				float red	= (float)m_RedSpin.GetPos ();
				float green	= (float)m_GreenSpin.GetPos ();
				float blue	= (float)m_BlueSpin.GetPos ();
				Update_Color (red, green, blue, UPDATE_COLOR_BARS | UPDATE_WHITENESS | UPDATE_HUE_PICKER);
			}
		}
		break;
	}
	
	return CDialog::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Color
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::Update_Color
(
	float red,
	float green,
	float blue,
	DWORD flags
)
{
	/*bool update_red = m_CurrentRed != red;
	bool update_green = m_CurrentGreen != green;
	bool update_blue = m_CurrentBlue != blue;*/

	m_CurrentRed = red;
	m_CurrentGreen = green;
	m_CurrentBlue = blue;

	int int_blue = (int)blue;
	int int_green = (int)green;
	int int_red = (int)red;
	if (int_blue != m_BlueSpin.GetPos ()) {
		m_BlueSpin.SetPos (int_blue);
	}
	if (int_green != m_GreenSpin.GetPos ()) {
		m_GreenSpin.SetPos (int_green);
	}
	if (int_red != m_RedSpin.GetPos ()) {
		m_RedSpin.SetPos (int_red);
	}
	
	//	Hack to get the edit controls to update in a timely fashion
	::UpdateWindow (::GetDlgItem (m_hWnd, IDC_RED_EDIT));
	::UpdateWindow (::GetDlgItem (m_hWnd, IDC_GREEN_EDIT));
	::UpdateWindow (::GetDlgItem (m_hWnd, IDC_BLUE_EDIT));

	//
	//	Update the red, green and blue color bars
	//
	if (flags & UPDATE_COLOR_BARS) {
		Update_Red_Bar ();
		Update_Green_Bar ();
		Update_Blue_Bar ();
	}	

	//
	//	Update the hue picker
	//
	if (flags & UPDATE_HUE_PICKER) {
		m_HuePicker->Select_Color ((int)red, (int)green, (int)blue);
	}

	//
	//	Update the whiteness color bar
	//
	if (flags & UPDATE_WHITENESS) {
		Update_Whiteness_Bar ();
	}

	Update_Current_Color_Bar ();	

	// If a callback is registered, call it.
	if (m_UpdateCallback)
		m_UpdateCallback((int)red, (int)green, (int)blue, m_CallArg);

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Set_Original_Color
//
/////////////////////////////////////////////////////////////////////////////
void ColorPickerDialogClass::Set_Original_Color (int r, int g, int b)
{
	m_OrigRed	= (float)r;
	m_OrigGreen	= (float)g;
	m_OrigBlue	= (float)b;
	m_OrigColorBar->Modify_Point (0, 0, m_OrigRed, m_OrigGreen, m_OrigBlue);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnReset
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::OnReset (void)
{
	Update_Color (m_OrigRed,
					  m_OrigGreen,
					  m_OrigBlue,
					  UPDATE_COLOR_BARS| UPDATE_WHITENESS | UPDATE_HUE_PICKER);
	return ;
}

LRESULT ColorPickerDialogClass::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	return CDialog::WindowProc(message, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ColorPickerDialogClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_RED_EDIT:
		case IDC_GREEN_EDIT:
		case IDC_BLUE_EDIT:
		{
			if (HIWORD (wParam) == EN_KILLFOCUS) {
				float red	= (float)GetDlgItemInt (IDC_RED_EDIT);
				float green = (float)GetDlgItemInt (IDC_GREEN_EDIT);
				float blue	= (float)GetDlgItemInt (IDC_BLUE_EDIT);
				Update_Color (red, green, blue, UPDATE_COLOR_BARS| UPDATE_WHITENESS | UPDATE_HUE_PICKER);
			}
		}
		break;
	}
	
	return CDialog::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////////
//
// PostNcDestroy
//
/////////////////////////////////////////////////////////////////////////////
void
ColorPickerDialogClass::PostNcDestroy (void)
{	
	CDialog::PostNcDestroy();

	if (m_bDeleteOnClose) {
		delete this;
	}

	return ;
}

