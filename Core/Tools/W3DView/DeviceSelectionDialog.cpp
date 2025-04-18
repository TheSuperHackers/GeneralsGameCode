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

// DeviceSelectionDialog.cpp : implementation file
//

#include "StdAfx.H"

#include "W3DView.h"
#include "DeviceSelectionDialog.h"
#include "WW3D.H"
#include "Resource.H"
#include "Globals.h"
#include "W3DView.H"
#include "Utils.H"
#include "rddesc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeviceSelectionDialog dialog

////////////////////////////////////////////////////////////////////
//
//  CDeviceSelectionDialog
//
CDeviceSelectionDialog::CDeviceSelectionDialog
(
    BOOL bLookupCachedInfo,
    CWnd* pParent /*=NULL*/
)
	: m_iDeviceIndex (1),
      m_iBitsPerPixel (16),
      m_bLookupCachedInfo (bLookupCachedInfo),
      CDialog(CDeviceSelectionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeviceSelectionDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    return ;
}

////////////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CDeviceSelectionDialog::DoDataExchange (CDataExchange* pDX)
{
    // Allow the base class to process this message
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeviceSelectionDialog)
	DDX_Control(pDX, IDC_RENDER_DEVICE_COMBO, m_deviceListComboBox);
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CDeviceSelectionDialog, CDialog)
	//{{AFX_MSG_MAP(CDeviceSelectionDialog)
	ON_CBN_SELCHANGE(IDC_RENDER_DEVICE_COMBO, OnSelchangeRenderDeviceCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeviceSelectionDialog message handlers


////////////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CDeviceSelectionDialog::OnInitDialog (void)
{
	CDialog::OnInitDialog();
	
	//
	// Loop through all the devices and add them to the combobox
	//
	int device_count = WW3D::Get_Render_Device_Count ();	
	int selected_index = 0;
	for (int index = 0; index < device_count; index ++) {
		
		//
		// Add this device to the combobox
		//
		const char *name = WW3D::Get_Render_Device_Name(index);
		int combo_index = m_deviceListComboBox.InsertString (index, name);
		if (m_DriverName.CompareNoCase (name) == 0) {
			selected_index = combo_index;			
		}

		// Associate the index of this device with the item we just inserted
		m_deviceListComboBox.SetItemData (combo_index, index);
	}

	// Check the '16bpp' radio by default
	SendDlgItemMessage (IDC_COLORDEPTH_16, BM_SETCHECK, (WPARAM)TRUE);

	// Force the first entry in the combobox to be selected.
	//m_deviceListComboBox.SetCurSel (0);
	m_deviceListComboBox.SetCurSel (selected_index);

	// Update the static controls on the dialog to reflect the device
	UpdateDeviceDescription ();
	return TRUE;
}


////////////////////////////////////////////////////////////////////
//
//  OnSelchangeRenderDeviceCombo
//
void
CDeviceSelectionDialog::OnSelchangeRenderDeviceCombo (void) 
{
	int index = m_deviceListComboBox.GetCurSel ();
	if (index != CB_ERR) {
		//WW3D::Set_Render_Device ();

		// Update the static controls with the information from the device
		UpdateDeviceDescription ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//  UpdateDeviceDescription
//
void
CDeviceSelectionDialog::UpdateDeviceDescription (void)
{
	const RenderDeviceDescClass &device_desc = WW3D::Get_Render_Device_Desc ();

	//
	// Reload the static text controls on the dialog
	//	
	SetDlgItemText (IDC_DRIVER_NAME, m_DriverName);
	SetDlgItemText (IDC_DEVICE_NAME_STATIC, device_desc.Get_Device_Name());
	SetDlgItemText (IDC_DEVICE_VENDOR_STATIC, device_desc.Get_Device_Vendor());
	SetDlgItemText (IDC_DEVICE_PLATFORM_STATIC, device_desc.Get_Device_Platform());
	SetDlgItemText (IDC_DRIVER_NAME_STATIC, device_desc.Get_Driver_Name());
	SetDlgItemText (IDC_DRIVER_VENDOR_STATIC, device_desc.Get_Driver_Vendor());
	SetDlgItemText (IDC_DRIVER_VERSION_STATIC, device_desc.Get_Driver_Version());
	SetDlgItemText (IDC_HARDWARE_NAME_STATIC, device_desc.Get_Hardware_Name());
	SetDlgItemText (IDC_HARDWARE_VENDOR_STATIC, device_desc.Get_Hardware_Vendor());
	SetDlgItemText (IDC_HARDWARE_CHIPSET_STATIC, device_desc.Get_Hardware_Chipset());
	return ;
}


////////////////////////////////////////////////////////////////////
//
//  OnOK
//
void
CDeviceSelectionDialog::OnOK (void) 
{
	// Ask the combobox for its current selection 
	m_iDeviceIndex = m_deviceListComboBox.GetItemData (m_deviceListComboBox.GetCurSel ());
	m_iBitsPerPixel = (SendDlgItemMessage (IDC_COLORDEPTH_16, BM_GETCHECK) == TRUE) ? 16 : 24;

	// Get the device name of the currently selected device
	CString stringDeviceName;
	m_deviceListComboBox.GetLBText (m_deviceListComboBox.GetCurSel (), stringDeviceName);
	m_DriverName = stringDeviceName;

	// Cache this information in the registry
	theApp.WriteProfileString ("Config", "DeviceName", stringDeviceName);
	theApp.WriteProfileInt ("Config", "DeviceBitsPerPix", m_iBitsPerPixel);

	// Allow the base class to process this message
	CDialog::OnOK();
	return ;
}


////////////////////////////////////////////////////////////////////
//
//  DoModal
//
int
CDeviceSelectionDialog::DoModal (void)
{
	BOOL bFoundDevice = FALSE;
	int iReturn = IDOK;

	// Get the name of the last used device driver from the registry
	m_DriverName = theApp.GetProfileString ("Config", "DeviceName");
	if (m_bLookupCachedInfo &&
		 (m_DriverName.GetLength () > 0) &&
		 !(::GetKeyState (VK_SHIFT) & 0xF000)) {

		//
		// Loop through all the devices and see if we can find the right one
		//
		int device_count = WW3D::Get_Render_Device_Count ();	
		for (int index = 0; (index < device_count) && !bFoundDevice; index ++) {
			
			//
			// Is this the device we are looking for?
			//
			const char *name = WW3D::Get_Render_Device_Name (index);
			if (m_DriverName.CompareNoCase (name) == 0) {

				//
				// Set the internal device information to simulate 'showing' the dialog
				//
				m_iDeviceIndex = index;
				m_iBitsPerPixel = theApp.GetProfileInt ("Config", "DeviceBitsPerPix", 16);

				// Found it!
				bFoundDevice = TRUE;
			}
		}
	}

	// Show the dialog and allow the user to select the device
	if (bFoundDevice == FALSE) {        
		iReturn = CDialog::DoModal ();
	}
	
	// Return the integer return code
	return iReturn;
}
