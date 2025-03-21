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

// ImpassableOptions.cpp
// Author: John McDonald, April 2001

#include "StdAfx.h" 
#include "resource.h"

#include "ImpassableOptions.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "wbview3d.h"
#include "WorldBuilderDoc.h"

//-------------------------------------------------------------------------------------------------
ImpassableOptions::ImpassableOptions(CWnd* pParent, Real defaultSlope) : 
	CDialog(ImpassableOptions::IDD, pParent), 
	m_slopeToShow(defaultSlope),
	m_defaultSlopeToShow(defaultSlope)
{	
	// nada to do
	m_showImpassableAreas = TheTerrainRenderObject->getShowImpassableAreas();
	TheTerrainRenderObject->setShowImpassableAreas(TRUE);
}

//-------------------------------------------------------------------------------------------------
ImpassableOptions::~ImpassableOptions()
{
	TheTerrainRenderObject->setShowImpassableAreas(m_showImpassableAreas);
}

//-------------------------------------------------------------------------------------------------
Bool ImpassableOptions::ValidateSlope()
{
	if (m_slopeToShow < 0.0f) {
		m_slopeToShow = 0.0f;
		return FALSE;
	}

	if (m_slopeToShow >= 90.0f) {
		m_slopeToShow = 89.9f;
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
BOOL ImpassableOptions::OnInitDialog()
{
	CWnd* pWnd = GetDlgItem(IDC_ANGLE);
	if (!pWnd) {
		return FALSE;
	}

	AsciiString astr;
	astr.format("%.2f", m_slopeToShow);
	pWnd->SetWindowText(astr.str());

	return CDialog::OnInitDialog();
}

//-------------------------------------------------------------------------------------------------
void ImpassableOptions::OnAngleChange()
{
	CWnd* pWnd = GetDlgItem(IDC_ANGLE);
	if (!pWnd) {
		return;
	}

	CString cstr;
	pWnd->GetWindowText(cstr);
	char* buff = cstr.GetBuffer(0);

	m_slopeToShow = (Real)atof(buff);

	if (!ValidateSlope()) {
		AsciiString astr;
		astr.format("%.2f", m_slopeToShow);
		pWnd->SetWindowText(astr.str());
	}
	TheTerrainRenderObject->setViewImpassableAreaSlope(m_slopeToShow);
}

void ImpassableOptions::OnPreview()
{
	// update it.
	IRegion2D range = {0,0,0,0};
	WbView3d *pView = CWorldBuilderDoc::GetActive3DView();
	pView->updateHeightMapInView(TheTerrainRenderObject->getMap(), false, range);
}

BEGIN_MESSAGE_MAP(ImpassableOptions, CDialog)
	ON_EN_UPDATE(IDC_ANGLE, OnAngleChange)
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
END_MESSAGE_MAP()