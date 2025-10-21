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

#if !defined(AFX_DEVICESELECTIONDIALOG_H__49E2DFF9_5D3F_11D2_9FC6_00104B791122__INCLUDED_)
#define AFX_DEVICESELECTIONDIALOG_H__49E2DFF9_5D3F_11D2_9FC6_00104B791122__INCLUDED_

#pragma once
// DeviceSelectionDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDeviceSelectionDialog dialog

class CDeviceSelectionDialog : public CDialog
{
// Construction
public:
	CDeviceSelectionDialog(BOOL bLookupCachedInfo = TRUE, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeviceSelectionDialog)
	enum { IDD = IDD_RENDER_DEVICE_SELECTOR };
	CComboBox	m_deviceListComboBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeviceSelectionDialog)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeviceSelectionDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeRenderDeviceCombo();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    public:

        // Return the selected device index
        int GetDeviceIndex () const
            { return m_iDeviceIndex; }

        // Return the selected bits per pixel
        int GetBitsPerPixel () const
            { return m_iBitsPerPixel; }

        const CString &GetDriverName () const
            { return m_DriverName; }

    protected:
        void UpdateDeviceDescription (void);

    private:
        BOOL		m_bLookupCachedInfo;
        int			m_iDeviceIndex;
        int			m_iBitsPerPixel;
		  CString	m_DriverName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEVICESELECTIONDIALOG_H__49E2DFF9_5D3F_11D2_9FC6_00104B791122__INCLUDED_)
