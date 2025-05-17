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

// ResolutionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "ResolutionDialog.h"
#include "WW3D.H"
#include "Globals.h"
#include "GraphicView.h"
#include "Utils.h"
#include "rddesc.h"



#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// ResolutionDialogClass
//
/////////////////////////////////////////////////////////////////////////////
ResolutionDialogClass::ResolutionDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(ResolutionDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(ResolutionDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
ResolutionDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ResolutionDialogClass)
	DDX_Control(pDX, IDC_RESOLUTION_LIST_CTRL, m_ListCtrl);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(ResolutionDialogClass, CDialog)
	//{{AFX_MSG_MAP(ResolutionDialogClass)
	ON_NOTIFY(NM_DBLCLK, IDC_RESOLUTION_LIST_CTRL, OnDblclkResolutionListCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
ResolutionDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Configure the list control
	//
	m_ListCtrl.SetExtendedStyle (LVS_EX_FULLROWSELECT);
	m_ListCtrl.InsertColumn (0, "Resolution");
	m_ListCtrl.InsertColumn (1, "Bit Depth");

	//
	//	Size the columns
	//
	CRect rect;
	m_ListCtrl.GetClientRect (&rect);
	m_ListCtrl.SetColumnWidth (0, (rect.Width () >> 1) - 4);
	m_ListCtrl.SetColumnWidth (1, (rect.Width () >> 1) - 4);

	//
	//	Get information about the current render device
	//
	const RenderDeviceDescClass &device_info = WW3D::Get_Render_Device_Desc ();
	const DynamicVectorClass<ResolutionDescClass> &res_list = device_info.Enumerate_Resolutions ();

	//
	//	Get the current resolution
	//
	int curr_width			= 0;
	int curr_height		= 0;
	int curr_bpp			= 0;
	bool curr_windowed	= false;
	WW3D::Get_Device_Resolution (curr_width, curr_height, curr_bpp, curr_windowed);
	SendDlgItemMessage (IDC_FULLSCREEN_CHECK, BM_SETCHECK, (WPARAM)(curr_windowed == false));

	//
	//	Loop over all the resolutions available to us
	//
	bool found = false;
	int index = res_list.Count ();
	while (index --) {

		int width	= res_list[index].Width;
		int height	= res_list[index].Height;
		int bpp		= res_list[index].BitDepth;
		
		//
		//	Format description strings for this resolution
		//
		CString resolution_string;
		CString bit_depth_string;
		resolution_string.Format ("%d x %d", width, height);
		bit_depth_string.Format ("%d bpp (%d colors)", bpp, 1 << bpp);

		//
		//	Add this resolution to the list ctrl
		//
		int list_index = m_ListCtrl.InsertItem (0, resolution_string, 0);
		if (list_index >= 0) {
			m_ListCtrl.SetItemText (list_index, 1, bit_depth_string);
			m_ListCtrl.SetItemData (list_index, index);

			//
			//	Is this the current resolution?  If so,
			// select this entry as the default...
			//
			if (	found == false &&
					curr_width == width &&
					curr_height == height &&
					curr_bpp == bpp)
			{
				m_ListCtrl.SetItemState (list_index, LVIS_SELECTED, LVIS_SELECTED);
				found = true;
			}
		}
	}
	
	//
	//	Select the first entry by default (if necessary)
	//
	if (found == false) {
		m_ListCtrl.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
ResolutionDialogClass::OnOK (void)
{
	CDialog::OnOK ();

	//
	//	Get the current selection
	//
	int list_index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (list_index >= 0) {

		//
		//	Index into the resolution list for this device...
		//
		int index = m_ListCtrl.GetItemData (list_index);
		if (index >= 0) {

			//
			//	Lookup the selected resolution information
			//
			const RenderDeviceDescClass &device_info = WW3D::Get_Render_Device_Desc ();
			const DynamicVectorClass<ResolutionDescClass> &res_list = device_info.Enumerate_Resolutions ();
			g_iWidth				= res_list[index].Width;
			g_iHeight			= res_list[index].Height;
			g_iBitsPerPixel	= res_list[index].BitDepth;
			
			//
			// Cache this information in the registry
			//
			theApp.WriteProfileInt ("Config", "DeviceWidth", g_iWidth);
			theApp.WriteProfileInt ("Config", "DeviceHeight", g_iHeight);
			theApp.WriteProfileInt ("Config", "DeviceBitsPerPix", g_iBitsPerPixel);
					
			//
			//	Reset the display
			//
			bool fullscreen = (SendDlgItemMessage (IDC_FULLSCREEN_CHECK, BM_GETCHECK) == 1);
			::Get_Graphic_View ()->Set_Fullscreen (fullscreen);
		}
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnDblclkResolutionListCtrl
//
/////////////////////////////////////////////////////////////////////////////
void
ResolutionDialogClass::OnDblclkResolutionListCtrl
(
	NMHDR *		pNMHDR,
	LRESULT *	pResult
)
{
	int list_index = m_ListCtrl.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);
	if (list_index >= 0) {
		OnOK ();
	}

	(*pResult) = 0;
	return ;
}
