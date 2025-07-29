/*
**	Command & Conquer Generals Zero Hour(tm)
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

// ShadowOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "Lib/BaseType.h"
#include "rendobj.h"
#include "Common/GlobalData.h"
#include "ShadowOptions.h"
#include "W3DDevice/GameClient/W3DShadow.h"

/////////////////////////////////////////////////////////////////////////////
// ShadowOptions dialog


ShadowOptions::ShadowOptions(CWnd* pParent /*=NULL*/)
	: CDialog(ShadowOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(ShadowOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ShadowOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ShadowOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ShadowOptions, CDialog)
	//{{AFX_MSG_MAP(ShadowOptions)
	ON_EN_CHANGE(IDC_ALPHA_EDIT, OnChangeAlphaEdit)
	ON_EN_CHANGE(IDC_BA_EDIT, OnChangeBaEdit)
	ON_EN_CHANGE(IDC_GA_EDIT, OnChangeGaEdit)
	ON_EN_CHANGE(IDC_RA_EDIT, OnChangeRaEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ShadowOptions message handlers

void ShadowOptions::setShadowColor(void) 
{
	Int r, g, b, shift;

	shift = (1.0-m_intensity)*255;
	if (shift>255) shift = 255;
	if (shift<0) shift = 0;

	r = m_intensity*m_red*255 + shift;
	if (r>255) r = 255;
	if (r<0) r = 0;

	g = m_intensity*m_green*255 + shift;
	if (g>255) g = 255;
	if (g<0) g = 0;

	b = m_intensity*m_blue*255 + shift;
	if (b>255) b = 255;
	if (b<0) b = 0;

	UnsignedInt clr = (255<<24) + (r<<16) + (g<<8) + b;
	DEBUG_LOG(("Setting shadows to %x", clr));
	TheW3DShadowManager->setShadowColor(clr);
}

void ShadowOptions::OnChangeAlphaEdit() 
{
	CWnd *pEdit = GetDlgItem(IDC_ALPHA_EDIT);
	Real clr;
	char buffer[_MAX_PATH];
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &clr)) {
			m_intensity = clr;
			setShadowColor();
		}
	}
}

void ShadowOptions::OnChangeBaEdit() 
{
	CWnd *pEdit = GetDlgItem(IDC_BA_EDIT);
	Real clr;
	char buffer[_MAX_PATH];
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &clr)) {
			m_blue = clr;
			setShadowColor();
		}
	}
}

void ShadowOptions::OnChangeGaEdit() 
{
	CWnd *pEdit = GetDlgItem(IDC_GA_EDIT);
	Real clr;
	char buffer[_MAX_PATH];
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &clr)) {
			m_green = clr;
			setShadowColor();
		}
	}
}

void ShadowOptions::OnChangeRaEdit() 
{
	CWnd *pEdit = GetDlgItem(IDC_RA_EDIT);
	Real clr;
	char buffer[_MAX_PATH];
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &clr)) {
			m_red = clr;
			setShadowColor();
		}
	}
}

BOOL ShadowOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UnsignedInt clr = TheW3DShadowManager->getShadowColor();
	m_red = ((clr>>16)&0x00FF)/255.0f;
	m_green = ((clr>>8)&0x00FF)/255.0f;
	m_blue = ((clr)&0x00FF)/255.0f;

	m_intensity = m_red;
	if (m_green<m_red) m_intensity = m_green;
	if (m_blue < m_intensity) m_intensity = m_blue;
	m_intensity = 1.0f - m_intensity;
	if (m_intensity < (1/256.0f)) {
		m_intensity = 0;
		m_red = m_green = m_blue = 0;
	} else {
		m_red -= (1.0-m_intensity);
		m_red /= m_intensity;
		m_green -= (1.0-m_intensity);
		m_green /= m_intensity;
		m_blue -= (1.0-m_intensity);
		m_blue /= m_intensity;
	}

	CWnd *pEdit = GetDlgItem(IDC_RA_EDIT);
	CString text;
	if (pEdit) {
		text.Format("%.2f", m_red);
		pEdit->SetWindowText(text);
	}
	pEdit = GetDlgItem(IDC_BA_EDIT);
	if (pEdit) {
		text.Format("%.2f", m_blue);
		pEdit->SetWindowText(text);
	}
	pEdit = GetDlgItem(IDC_GA_EDIT);
	if (pEdit) {
		text.Format("%.2f", m_green);
		pEdit->SetWindowText(text);
	}
	pEdit = GetDlgItem(IDC_ALPHA_EDIT);
	if (pEdit) {
		text.Format("%.2f", m_intensity);
		pEdit->SetWindowText(text);
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
