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

// LightOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "LightOptions.h"
#include "WorldBuilderView.h"
#include "WorldBuilderDoc.h"
#include "wbview3d.h"
#include "Common/WellKnownKeys.h"

LightOptions *LightOptions::m_staticThis = NULL;
/////////////////////////////////////////////////////////////////////////////
/// LightOptions dialog trivial construstor - Create does the real work.


LightOptions::LightOptions(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(LightOptions) 
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/// Windows default stuff.
void LightOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LightOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

MapObject *LightOptions::getSingleSelectedLight(void)
{
	MapObject *pMapObj; 
	MapObject *theMapObj = NULL; 
//	Bool found = false;
	Int selCount=0;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isSelected()) {
			if (pMapObj->isLight()) {
				theMapObj = pMapObj;
			}
			selCount++;
		}
	}
	if (selCount==1 && theMapObj) {
		return theMapObj;
	}
	return(NULL);
}


void LightOptions::updateTheUI(void) 
{
	MapObject *theMapObj = getSingleSelectedLight(); 
	if (!theMapObj) return;
	Dict *props = theMapObj->getProperties();

	CString str;

	Real lightHeightAboveTerrain, lightInnerRadius, lightOuterRadius;
	RGBColor lightAmbientColor, lightDiffuseColor;

	lightHeightAboveTerrain = props->getReal(TheKey_lightHeightAboveTerrain);
	lightInnerRadius = props->getReal(TheKey_lightInnerRadius);
	lightOuterRadius = props->getReal(TheKey_lightOuterRadius);
	lightAmbientColor.setFromInt(props->getInt(TheKey_lightAmbientColor));
	lightDiffuseColor.setFromInt(props->getInt(TheKey_lightDiffuseColor));

	CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RA_EDIT);
	if (pEdit) {
		str.Format("%.2f", lightAmbientColor.red);
		pEdit->SetWindowText(str);
	}
	pEdit = m_staticThis->GetDlgItem(IDC_GA_EDIT);
	if (pEdit) {
		str.Format("%.2f", lightAmbientColor.green);
		pEdit->SetWindowText(str);
	}
	pEdit = m_staticThis->GetDlgItem(IDC_BA_EDIT);
	if (pEdit) {
		str.Format("%.2f", lightAmbientColor.blue);
		pEdit->SetWindowText(str);
	}
	pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
	if (pEdit) {
		str.Format("%.2f", lightHeightAboveTerrain);
		pEdit->SetWindowText(str);
	}
	pEdit = m_staticThis->GetDlgItem(IDC_RADIUS_EDIT);
	if (pEdit) {
		str.Format("%.2f", lightOuterRadius);
		pEdit->SetWindowText(str);
	}
}

void LightOptions::update(void) 
{
	if (m_staticThis) {
		m_staticThis->updateTheUI();
	}
}

/////////////////////////////////////////////////////////////////////////////
// LightOptions message handlers

/// Dialog UI initialization.
/** Creates the slider controls, and sets the initial values for 
width and feather in the ui controls. */
BOOL LightOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_updating = true;

	m_staticThis = this;
	m_updating = false;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BEGIN_MESSAGE_MAP(LightOptions, COptionsPanel)
	//{{AFX_MSG_MAP(LightOptions)
	ON_EN_CHANGE(IDC_RA_EDIT, OnChangeLightEdit)
	ON_EN_CHANGE(IDC_GA_EDIT, OnChangeLightEdit)
	ON_EN_CHANGE(IDC_BA_EDIT, OnChangeLightEdit)
	ON_EN_CHANGE(IDC_HEIGHT_EDIT, OnChangeLightEdit)
	ON_EN_CHANGE(IDC_RADIUS_EDIT, OnChangeLightEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void LightOptions::OnChangeLightEdit() 
{
	MapObject *theMapObj = getSingleSelectedLight(); 
	if (!theMapObj) return;
	if (!theMapObj->isLight()) return;

	Real lightHeightAboveTerrain, lightInnerRadius, lightOuterRadius;
	RGBColor lightAmbientColor, lightDiffuseColor;

	Dict *props = theMapObj->getProperties();
	lightHeightAboveTerrain = props->getReal(TheKey_lightHeightAboveTerrain);
	lightInnerRadius = props->getReal(TheKey_lightInnerRadius);
	lightOuterRadius = props->getReal(TheKey_lightOuterRadius);
	lightAmbientColor.setFromInt(props->getInt(TheKey_lightAmbientColor));
	lightDiffuseColor.setFromInt(props->getInt(TheKey_lightDiffuseColor));

	Real clr;
	char buffer[_MAX_PATH];

	CWnd *pEdit = m_staticThis->GetDlgItem(IDC_RA_EDIT);
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &clr)) lightAmbientColor.red = clr;
	}
	pEdit = m_staticThis->GetDlgItem(IDC_GA_EDIT);
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &clr)) lightAmbientColor.green = clr;
	}
	pEdit = m_staticThis->GetDlgItem(IDC_BA_EDIT);
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &clr)) lightAmbientColor.blue = clr;
	}
	pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
	Real r;
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &r)) lightHeightAboveTerrain = r;
	}
	pEdit = m_staticThis->GetDlgItem(IDC_RADIUS_EDIT);
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		if (1==sscanf(buffer, "%f", &r)) lightOuterRadius = r;
	}
	lightDiffuseColor.red = 0;
	lightDiffuseColor.green = 0;
	lightDiffuseColor.blue = 0;
	lightInnerRadius = lightOuterRadius/2;

	props->setReal(TheKey_lightHeightAboveTerrain, lightHeightAboveTerrain);
	props->setReal(TheKey_lightInnerRadius, lightInnerRadius);
	props->setReal(TheKey_lightOuterRadius, lightOuterRadius);
	props->setInt(TheKey_lightAmbientColor, lightAmbientColor.getAsInt());
	props->setInt(TheKey_lightDiffuseColor, lightDiffuseColor.getAsInt());

	WbView3d * pView = CWorldBuilderDoc::GetActive3DView();	
	if (pView) {
		pView->invalObjectInView(theMapObj);
	}
}
