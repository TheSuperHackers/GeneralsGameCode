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

// GammaDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "GammaDialog.h"
#include "dx8wrapper.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GammaDialogClass dialog


GammaDialogClass::GammaDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(GammaDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(GammaDialogClass)
	m_gamma = 0;
	//}}AFX_DATA_INIT
}


void GammaDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GammaDialogClass)
	DDX_Control(pDX, IDC_GAMMA_SLIDER, m_gammaslider);
	DDX_Slider(pDX, IDC_GAMMA_SLIDER, m_gamma);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GammaDialogClass, CDialog)
	//{{AFX_MSG_MAP(GammaDialogClass)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_GAMMA_SLIDER, OnReleasedcaptureGammaSlider)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GammaDialogClass message handlers

BOOL GammaDialogClass::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here	
	m_gamma=AfxGetApp()->GetProfileInt("Config","Gamma",10);
	if (m_gamma<10) m_gamma=10;
	if (m_gamma>30) m_gamma=30;
	m_gammaslider.SetRange(10,30);	
	m_gammaslider.SetPos(m_gamma);
	CString string;
	string.Format("%3.2f",m_gamma/10.0f);
	SetDlgItemText(IDC_GAMMA_DISPLAY,string);
	string.Format("Calibration instructions\n");
	string+="A. Set Gamma to 1.0 and Monitor Contrast and Brightness to maximum\n";
	string+="B. Adjust Monitor Brightness down so Bar 3 is barely visible\n";
	string+="C. Adjust Monitor Contrast as preferred but Bars 1,2,3,4 must be distinguishable from each other\n";
	string+="D. Set the Gamma using the Slider below so the gray box on the left matches it's checkered surroundings\n";
	string+="E. Press OK to save settings";
	SetDlgItemText(IDC_INSTRUCTIONS,string);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void GammaDialogClass::OnOK() 
{
	// TODO: Add extra validation here
	m_gamma=m_gammaslider.GetPos();
	if (m_gamma<10) m_gamma=10;
	if (m_gamma>30) m_gamma=30;	
	::AfxGetApp()->WriteProfileInt("Config","Gamma",m_gamma);
	DX8Wrapper::Set_Gamma(m_gamma/10.0f,0.0f,1.0f);
	
	CDialog::OnOK();
}

void GammaDialogClass::OnReleasedcaptureGammaSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here	
	m_gamma=m_gammaslider.GetPos();
	DX8Wrapper::Set_Gamma(m_gamma/10.0f,0.0f,1.0f);
	CString string;
	string.Format("%3.2f",m_gamma/10.0f);
	SetDlgItemText(IDC_GAMMA_DISPLAY,string);
	
	*pResult = 0;
}
