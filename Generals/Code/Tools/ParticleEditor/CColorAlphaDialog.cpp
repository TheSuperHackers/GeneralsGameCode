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

#include "StdAfx.h"
#include "Resource.h"
#include "CColorAlphaDialog.h"

#include "GameClient/ParticleSys.h"

#include "CParticleEditorPage.h"
#include "EmissionTypePanels.h"
#include "ParticleEditorDialog.h"
#include "ParticleEditorExport.h"
#include "ParticleTypePanels.h"
#include "VelocityTypePanels.h"

#define		ARBITRARY_BUFF_SIZE		128

static const UINT colorControls[][2] =
{
	{IDC_PSEd_Color1,	IDC_PSEd_CF1_Frame},
	{IDC_PSEd_Color2,	IDC_PSEd_CF2_Frame},
	{IDC_PSEd_Color3,	IDC_PSEd_CF3_Frame},
	{IDC_PSEd_Color4,	IDC_PSEd_CF4_Frame},
	{IDC_PSEd_Color5,	IDC_PSEd_CF5_Frame},
	{IDC_PSEd_Color6,	IDC_PSEd_CF6_Frame},
	{IDC_PSEd_Color7,	IDC_PSEd_CF7_Frame},
	{IDC_PSEd_Color8,	IDC_PSEd_CF8_Frame}
};

static const UINT alphaControls[][3] =
{
	{IDC_PSEd_AF1_Min, IDC_PSEd_AF1_Max, IDC_PSEd_AF1_Frame},
	{IDC_PSEd_AF2_Min, IDC_PSEd_AF2_Max, IDC_PSEd_AF2_Frame},
	{IDC_PSEd_AF3_Min, IDC_PSEd_AF3_Max, IDC_PSEd_AF3_Frame},
	{IDC_PSEd_AF4_Min, IDC_PSEd_AF4_Max, IDC_PSEd_AF4_Frame},
	{IDC_PSEd_AF5_Min, IDC_PSEd_AF5_Max, IDC_PSEd_AF5_Frame},
	{IDC_PSEd_AF6_Min, IDC_PSEd_AF6_Max, IDC_PSEd_AF6_Frame},
	{IDC_PSEd_AF7_Min, IDC_PSEd_AF7_Max, IDC_PSEd_AF7_Frame},
	{IDC_PSEd_AF8_Min, IDC_PSEd_AF8_Max, IDC_PSEd_AF8_Frame}
};



CColorAlphaDialog::CColorAlphaDialog(UINT nIDTemplate, CWnd* pParentWnd) :
										CDialog(nIDTemplate, pParentWnd)
{}

void CColorAlphaDialog::InitPanel( void )
{
	CString custColor;
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color1", "0");
	m_customColors[0] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color2", "0");
	m_customColors[1] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color3", "0");
	m_customColors[2] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color4", "0");
	m_customColors[3] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color5", "0");
	m_customColors[4] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color6", "0");
	m_customColors[5] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color7", "0");
	m_customColors[6] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color8", "0");
	m_customColors[7] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color9", "0");
	m_customColors[8] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color10", "0");
	m_customColors[9] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color11", "0");
	m_customColors[10] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color12", "0");
	m_customColors[11] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color13", "0");
	m_customColors[12] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color14", "0");
	m_customColors[13] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color15", "0");
	m_customColors[14] = atol(custColor.GetBuffer(0));
	custColor = AfxGetApp()->GetProfileString("Custom Colors", "Color16", "0");
	m_customColors[15] = atol(custColor.GetBuffer(0));
}

void CColorAlphaDialog::performUpdate(IN Bool toUI)
{
	static char buff[ARBITRARY_BUFF_SIZE];
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	{	// update the colors
		for (int i = 0; i < MAX_KEYFRAMES; ++i) {
			// update the color swatch
			CButtonShowColor *pSwatch = (CButtonShowColor*) GetDlgItem(colorControls[i][0]);
			CWnd *pFrame = GetDlgItem(colorControls[i][1]);
			RGBColorKeyframe colorFrame;

			if (pSwatch && pFrame) {
				if (toUI) {
					pParent->getColorValueFromSystem(i, colorFrame);

					pSwatch->setColor(colorFrame.color);
					pSwatch->Invalidate(TRUE);

					sprintf(buff, "%u", colorFrame.frame);
					pFrame->SetWindowText(buff);
				} else {
					colorFrame.color = pSwatch->getColor();

					pFrame->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
					colorFrame.frame = atoi(buff);
					pParent->updateColorValueToSystem(i, colorFrame);
				}
			}
		}
	}

	{ // update the values
		for (int i = 0; i < MAX_KEYFRAMES; ++i) {
			ParticleSystemInfo::RandomKeyframe keyFrame;

			pParent->getAlphaRangeFromSystem(i, keyFrame);

			{	// Minimum first
				CWnd *pMin = GetDlgItem(alphaControls[i][0]);
				if (pMin) {
					if (toUI) {
						sprintf(buff, FORMAT_STRING, keyFrame.var.getMinimumValue());
						pMin->SetWindowText(buff);
					} else {
						pMin->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
						keyFrame.var.m_low = atof(buff);
					}
				}
			}

			{	// then maximum
				CWnd *pMax = GetDlgItem(alphaControls[i][1]);
				if (pMax) {
					if (toUI) {
						sprintf(buff, FORMAT_STRING, keyFrame.var.getMaximumValue());
						pMax->SetWindowText(buff);
					} else {
						pMax->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
						keyFrame.var.m_high = atof(buff);
					}
				}
			}

			{ // then the frame
				CWnd *pFrame = GetDlgItem(alphaControls[i][2]);
				if (pFrame) {
					if (toUI) {
						// Unsigned int
						sprintf(buff, "%u", keyFrame.frame);
						pFrame->SetWindowText(buff);
					} else {
						pFrame->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
						keyFrame.frame = (unsigned) atoi(buff);
					}
				}
			}

			if (toUI) {
				// We're all done.
			} else {
				pParent->updateAlphaRangeToSystem(i, keyFrame);
			}
		}
	}
}

void CColorAlphaDialog::OnParticleSystemEdit()
{
	DebugWindowDialog *pParent = (DebugWindowDialog*) GetParent();
	if (!pParent) {
		return;
	}

	pParent->signalParticleSystemEdit();
}

BOOL CColorAlphaDialog::OnInitDialog()
{
	// replace all the buttons with our buttons.
	for (int i = 0; i < MAX_KEYFRAMES; ++i) {
		CRect rect;
		CWnd *item = GetDlgItem(colorControls[i][0]);
		if (!item) {
			continue;
		}

		item->GetWindowRect(&rect);
		ScreenToClient(&rect);
		DWORD style = item->GetStyle();
		m_colorButton[i].Create("", style, rect, this, colorControls[i][0]);

		item->DestroyWindow();
	}

	return CDialog::OnInitDialog();
}

#define ONCOLORDLG(x) void CColorAlphaDialog::OnColor##x() { onColorPress( x ); }
ONCOLORDLG(1)
ONCOLORDLG(2)
ONCOLORDLG(3)
ONCOLORDLG(4)
ONCOLORDLG(5)
ONCOLORDLG(6)
ONCOLORDLG(7)
ONCOLORDLG(8)
#undef ONCOLORDLG

void CColorAlphaDialog::onColorPress(Int colorPressed)
{
	CColorDialog dlg(CButtonShowColor::RGBtoBGR(m_colorButton[colorPressed - 1].getColor().getAsInt()));
	dlg.m_cc.Flags |= (CC_FULLOPEN | CC_ANYCOLOR);
	dlg.m_cc.lpCustColors = m_customColors;
	if (dlg.DoModal() == IDOK) {

		m_colorButton[colorPressed - 1].setColor(CButtonShowColor::BGRtoRGB(dlg.GetColor()));
		static char buff[ARBITRARY_BUFF_SIZE];

		ltoa(m_customColors[0], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color1", buff);
		ltoa(m_customColors[1], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color2", buff);
		ltoa(m_customColors[2], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color3", buff);
		ltoa(m_customColors[3], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color4", buff);
		ltoa(m_customColors[4], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color5", buff);
		ltoa(m_customColors[5], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color6", buff);
		ltoa(m_customColors[6], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color7", buff);
		ltoa(m_customColors[7], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color8", buff);
		ltoa(m_customColors[8], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color9", buff);
		ltoa(m_customColors[9], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color10", buff);
		ltoa(m_customColors[10], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color11", buff);
		ltoa(m_customColors[11], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color12", buff);
		ltoa(m_customColors[12], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color13", buff);
		ltoa(m_customColors[13], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color14", buff);
		ltoa(m_customColors[14], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color15", buff);
		ltoa(m_customColors[15], buff, 10);
		AfxGetApp()->WriteProfileString("Custom Colors", "Color16", buff);



		OnParticleSystemEdit();
	}
}

BEGIN_MESSAGE_MAP(CColorAlphaDialog, CDialog)
	ON_BN_CLICKED(IDC_PSEd_Color1, OnColor1)
	ON_BN_CLICKED(IDC_PSEd_Color2, OnColor2)
	ON_BN_CLICKED(IDC_PSEd_Color3, OnColor3)
	ON_BN_CLICKED(IDC_PSEd_Color4, OnColor4)
	ON_BN_CLICKED(IDC_PSEd_Color5, OnColor5)
	ON_BN_CLICKED(IDC_PSEd_Color6, OnColor6)
	ON_BN_CLICKED(IDC_PSEd_Color7, OnColor7)
	ON_BN_CLICKED(IDC_PSEd_Color8, OnColor8)

	ON_EN_KILLFOCUS(IDC_PSEd_CF1_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CF2_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CF3_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CF4_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CF5_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CF6_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CF7_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_CF8_Frame, OnParticleSystemEdit)

	ON_EN_KILLFOCUS(IDC_PSEd_AF1_Min, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF2_Min, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF3_Min, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF4_Min, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF5_Min, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF6_Min, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF7_Min, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF8_Min, OnParticleSystemEdit)

	ON_EN_KILLFOCUS(IDC_PSEd_AF1_Max, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF2_Max, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF3_Max, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF4_Max, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF5_Max, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF6_Max, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF7_Max, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF8_Max, OnParticleSystemEdit)

	ON_EN_KILLFOCUS(IDC_PSEd_AF1_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF2_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF3_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF4_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF5_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF6_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF7_Frame, OnParticleSystemEdit)
	ON_EN_KILLFOCUS(IDC_PSEd_AF8_Frame, OnParticleSystemEdit)
END_MESSAGE_MAP()
