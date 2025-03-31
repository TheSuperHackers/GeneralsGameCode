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

// GlobalLightOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Lib/BaseType.h"
#include "GlobalLightOptions.h"
#include "WorldBuilderDoc.h"
#include "Common/GlobalData.h"
#include "wbview3d.h"

/////////////////////////////////////////////////////////////////////////////
/// GlobalLightOptions dialog trivial construstor - Create does the real work.


GlobalLightOptions::GlobalLightOptions(CWnd* pParent /*=NULL*/)
	: CDialog(GlobalLightOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(GlobalLightOptions) 
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

/// Windows default stuff.
void GlobalLightOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GlobalLightOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


static void calcNewLight(Int lr, Int fb, Vector3 *newLight)
{
	newLight->Set(0,0,-1);
	Real yAngle = PI*(lr-90)/180;
	Real xAngle = PI*(fb-90)/180;
	Real zAngle = xAngle * WWMath::Sin(yAngle);
	xAngle *= WWMath::Cos(yAngle);
	newLight->Rotate_Y(yAngle);
	newLight->Rotate_X(xAngle);
	newLight->Rotate_Z(zAngle);
}

void GlobalLightOptions::updateEditFields(void) 
{ 
	m_updating = true;
	CString str;
	CWnd *pEdit;

	str.Format("%d",m_angleAzimuth[K_SUN]);
	pEdit = GetDlgItem(IDC_FB_EDIT);
	if (pEdit) pEdit->SetWindowText(str);
	str.Format("%d",m_angleElevation[K_SUN]);
	pEdit = GetDlgItem(IDC_LR_EDIT);
	if (pEdit) pEdit->SetWindowText(str);

	str.Format("%d",m_angleAzimuth[K_ACCENT1]);
	pEdit = GetDlgItem(IDC_FB_EDIT1);
	if (pEdit) pEdit->SetWindowText(str);
	str.Format("%d",m_angleElevation[K_ACCENT1]);
	pEdit = GetDlgItem(IDC_LR_EDIT1);
	if (pEdit) pEdit->SetWindowText(str);

	str.Format("%d",m_angleAzimuth[K_ACCENT2]);
	pEdit = GetDlgItem(IDC_FB_EDIT2);
	if (pEdit) pEdit->SetWindowText(str);
	str.Format("%d",m_angleElevation[K_ACCENT2]);
	pEdit = GetDlgItem(IDC_LR_EDIT2);
	if (pEdit) pEdit->SetWindowText(str);

	m_updating = false;
}

void GlobalLightOptions::showLightFeedback(Int lightIndex) 
{
	Vector3 light(0,0,0);
	light.X = sin(PI/2.0f+m_angleElevation[lightIndex]/180.0f*PI)*cos(m_angleAzimuth[lightIndex]/180.0f*PI);// -WWMath::Sin(PI*(m_angleLR[lightIndex]-90)/180);
	light.Y = sin(PI/2.0f+m_angleElevation[lightIndex]/180.0f*PI)*sin(m_angleAzimuth[lightIndex]/180.0f*PI);//-WWMath::Sin(PI*(m_angleFB[lightIndex]-90)/180);
	light.Z = cos (PI/2.0f+m_angleElevation[lightIndex]/180.0f*PI);

	WbView3d * pView = CWorldBuilderDoc::GetActive3DView();	
	if (pView) {
		Coord3D lightRay;
		lightRay.x=light.X;lightRay.y=light.Y;lightRay.z=light.Z;
		pView->doLightFeedback(true,lightRay,lightIndex);
	}
}

void GlobalLightOptions::applyAngle(Int lightIndex) 
{ 
	Vector3 light(0,0,0);
	light.X = sin(PI/2.0f+m_angleElevation[lightIndex]/180.0f*PI)*cos(m_angleAzimuth[lightIndex]/180.0f*PI);// -WWMath::Sin(PI*(m_angleLR[lightIndex]-90)/180);
	light.Y = sin(PI/2.0f+m_angleElevation[lightIndex]/180.0f*PI)*sin(m_angleAzimuth[lightIndex]/180.0f*PI);//-WWMath::Sin(PI*(m_angleFB[lightIndex]-90)/180);
	light.Z = cos (PI/2.0f+m_angleElevation[lightIndex]/180.0f*PI);

	CString str;
	str.Format("XYZ: %.2f, %.2f, %.2f", light.X, light.Y, light.Z);
	CWnd *pWnd = this->GetDlgItem(IDC_XYZ_STATIC);
	if (pWnd) {
		pWnd->SetWindowText(str);
	}
	GlobalData::TerrainLighting tl;
	if (m_lighting == K_OBJECTS || m_lighting == K_BOTH) {
		tl = TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][lightIndex];
	} else {
		tl = TheGlobalData->m_terrainLighting[TheGlobalData->m_timeOfDay][lightIndex];
	}
	tl.lightPos.x = light.X;
	tl.lightPos.y = light.Y;
	tl.lightPos.z = light.Z;

	WbView3d * pView = CWorldBuilderDoc::GetActive3DView();	
	if (pView) {
		pView->setLighting(&tl, m_lighting, lightIndex);
		Coord3D lightRay;
		lightRay.x=light.X;lightRay.y=light.Y;lightRay.z=light.Z;
		pView->doLightFeedback(true,lightRay,lightIndex);
	}
}

static void SpitLights()
{
#ifdef DEBUG_LOGGING
	CString lightstrings[100];
	DEBUG_LOG(("GlobalLighting\n\n"));
	Int redA, greenA, blueA;
	Int redD, greenD, blueD;
	Real x, y, z;
	CString times[4];
	CString lights[3];
	times[0] = "Morning";
	times[1] = "Afternoon";
	times[2] = "Evening";
	times[3] = "Night";

	lights[0] = "";
	lights[1] = "2";
	lights[2] = "3";

	Int time=0;
	for (; time<4; time++) {
		for (Int light=0; light<3; light++) {
			redA = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].ambient.red*255;
			greenA = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].ambient.green*255;
			blueA = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].ambient.blue*255;

			redD = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].diffuse.red*255;
			greenD = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].diffuse.green*255;
			blueD = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].diffuse.blue*255;

			x = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].lightPos.x;
			y = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].lightPos.y;
			z = TheGlobalData->m_terrainLighting[time+TIME_OF_DAY_FIRST][light].lightPos.z;

			DEBUG_LOG(("TerrainLighting%sAmbient%s = R:%d G:%d B:%d\n", times[time], lights[light], redA, greenA, blueA));
			DEBUG_LOG(("TerrainLighting%sDiffuse%s = R:%d G:%d B:%d\n", times[time], lights[light], redD, greenD, blueD));
			DEBUG_LOG(("TerrainLighting%sLightPos%s = X:%0.2f Y:%0.2f Z:%0.2f\n", times[time], lights[light], x, y, z));

			redA = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].ambient.red*255;
			greenA = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].ambient.green*255;
			blueA = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].ambient.blue*255;

			redD = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].diffuse.red*255;
			greenD = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].diffuse.green*255;
			blueD = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].diffuse.blue*255;

			x = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].lightPos.x;
			y = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].lightPos.y;
			z = TheGlobalData->m_terrainObjectsLighting[time+TIME_OF_DAY_FIRST][light].lightPos.z;

			DEBUG_LOG(("TerrainObjectsLighting%sAmbient%s = R:%d G:%d B:%d\n", times[time], lights[light], redA, greenA, blueA));
			DEBUG_LOG(("TerrainObjectsLighting%sDiffuse%s = R:%d G:%d B:%d\n", times[time], lights[light], redD, greenD, blueD));
			DEBUG_LOG(("TerrainObjectsLighting%sLightPos%s = X:%0.2f Y:%0.2f Z:%0.2f\n", times[time], lights[light], x, y, z));

			DEBUG_LOG(("\n"));
		}
		DEBUG_LOG(("\n"));
	}

	DEBUG_LOG(("GlobalLighting Code\n\n"));
	for (time=0; time<4; time++) {
		for (Int light=0; light<3; light++) {
			Int theTime = time+TIME_OF_DAY_FIRST;
			GlobalData::TerrainLighting tl = TheGlobalData->m_terrainLighting[theTime][light];

			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].ambient.red = %0.4ff;\n", theTime, light, tl.ambient.red));
			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].ambient.green = %0.4ff;\n", theTime, light, tl.ambient.green));
			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].ambient.blue = %0.4ff;\n", theTime, light, tl.ambient.blue));

			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].diffuse.red = %0.4ff;\n", theTime, light, tl.diffuse.red));
			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].diffuse.green = %0.4ff;\n", theTime, light, tl.diffuse.green));
			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].diffuse.blue = %0.4ff;\n", theTime, light, tl.diffuse.blue));

			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].lightPos.x = %0.4ff;\n", theTime, light, tl.lightPos.x));
			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].lightPos.y = %0.4ff;\n", theTime, light, tl.lightPos.y));
			DEBUG_LOG(("TheGlobalData->m_terrainLighting[%d][%d].lightPos.z = %0.4ff;\n", theTime, light, tl.lightPos.z));
				
			tl = TheGlobalData->m_terrainObjectsLighting[theTime][light];

			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].ambient.red = %0.4ff;\n", theTime, light, tl.ambient.red));
			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].ambient.green = %0.4ff;\n", theTime, light, tl.ambient.green));
			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].ambient.blue = %0.4ff;\n", theTime, light, tl.ambient.blue));

			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].diffuse.red = %0.4ff;\n", theTime, light, tl.diffuse.red));
			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].diffuse.green = %0.4ff;\n", theTime, light, tl.diffuse.green));
			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].diffuse.blue = %0.4ff;\n", theTime, light, tl.diffuse.blue));

			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].lightPos.x = %0.4ff;\n", theTime, light, tl.lightPos.x));
			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].lightPos.y = %0.4ff;\n", theTime, light, tl.lightPos.y));
			DEBUG_LOG(("TheGlobalData->m_terrainObjectsLighting[%d][%d].lightPos.z = %0.4ff;\n", theTime, light, tl.lightPos.z));

			DEBUG_LOG(("\n"));
		}
		DEBUG_LOG(("\n"));
	}
#endif
}

void GlobalLightOptions::OnResetLights()
{
	TheWritableGlobalData->m_terrainLighting[1][0].ambient.red = 0.50f;
	TheWritableGlobalData->m_terrainLighting[1][0].ambient.green = 0.39f;
	TheWritableGlobalData->m_terrainLighting[1][0].ambient.blue = 0.30f;
	TheWritableGlobalData->m_terrainLighting[1][0].diffuse.red = 0.90f;
	TheWritableGlobalData->m_terrainLighting[1][0].diffuse.green = 0.71f;
	TheWritableGlobalData->m_terrainLighting[1][0].diffuse.blue = 0.60f;
	TheWritableGlobalData->m_terrainLighting[1][0].lightPos.x = -0.96f;
	TheWritableGlobalData->m_terrainLighting[1][0].lightPos.y = 0.05f;
	TheWritableGlobalData->m_terrainLighting[1][0].lightPos.z = -0.29f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].ambient.red = 0.50f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].ambient.green = 0.40f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].ambient.blue = 0.30f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].diffuse.red = 0.90f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].diffuse.green = 0.70f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].diffuse.blue = 0.60f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].lightPos.x = -0.96f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].lightPos.y = 0.05f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][0].lightPos.z = -0.29f;

	TheWritableGlobalData->m_terrainLighting[1][1].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][1].lightPos.z = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][1].lightPos.z = -1.00f;

	TheWritableGlobalData->m_terrainLighting[1][2].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainLighting[1][2].lightPos.z = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[1][2].lightPos.z = -1.00f;


	TheWritableGlobalData->m_terrainLighting[2][0].ambient.red = 0.2196f;
	TheWritableGlobalData->m_terrainLighting[2][0].ambient.green = 0.2039f;
	TheWritableGlobalData->m_terrainLighting[2][0].ambient.blue = 0.1725f;
	TheWritableGlobalData->m_terrainLighting[2][0].diffuse.red = 1.0000f;
	TheWritableGlobalData->m_terrainLighting[2][0].diffuse.green = 1.0000f;
	TheWritableGlobalData->m_terrainLighting[2][0].diffuse.blue = 1.0000f;
	TheWritableGlobalData->m_terrainLighting[2][0].lightPos.x = -0.8100f;
	TheWritableGlobalData->m_terrainLighting[2][0].lightPos.y = 0.3800f;
	TheWritableGlobalData->m_terrainLighting[2][0].lightPos.z = -0.4500f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].ambient.red = 0.2196f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].ambient.green = 0.2039f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].ambient.blue = 0.1725f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].diffuse.red = 1.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].diffuse.green = 1.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].diffuse.blue = 1.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].lightPos.x = -0.8100f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].lightPos.y = 0.3800f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][0].lightPos.z = -0.4500f;

	TheWritableGlobalData->m_terrainLighting[2][1].ambient.red = 0.0000f;
	TheWritableGlobalData->m_terrainLighting[2][1].ambient.green = 0.0000f;
	TheWritableGlobalData->m_terrainLighting[2][1].ambient.blue = 0.0000f;
	TheWritableGlobalData->m_terrainLighting[2][1].diffuse.red = 0.2353f;
	TheWritableGlobalData->m_terrainLighting[2][1].diffuse.green = 0.2353f;
	TheWritableGlobalData->m_terrainLighting[2][1].diffuse.blue = 0.4706f;
	TheWritableGlobalData->m_terrainLighting[2][1].lightPos.x = 0.7900f;
	TheWritableGlobalData->m_terrainLighting[2][1].lightPos.y = 0.6200f;
	TheWritableGlobalData->m_terrainLighting[2][1].lightPos.z = 0.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].ambient.red = 0.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].ambient.green = 0.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].ambient.blue = 0.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].diffuse.red = 0.2353f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].diffuse.green = 0.2353f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].diffuse.blue = 0.3137f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].lightPos.x = 0.7900f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].lightPos.y = 0.6200f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][1].lightPos.z = 0.0000f;

	TheWritableGlobalData->m_terrainLighting[2][2].ambient.red = 0.0000f;
	TheWritableGlobalData->m_terrainLighting[2][2].ambient.green = 0.0000f;
	TheWritableGlobalData->m_terrainLighting[2][2].ambient.blue = 0.0000f;
	TheWritableGlobalData->m_terrainLighting[2][2].diffuse.red = 0.1176f;
	TheWritableGlobalData->m_terrainLighting[2][2].diffuse.green = 0.1176f;
	TheWritableGlobalData->m_terrainLighting[2][2].diffuse.blue = 0.0784f;
	TheWritableGlobalData->m_terrainLighting[2][2].lightPos.x = 0.8100f;
	TheWritableGlobalData->m_terrainLighting[2][2].lightPos.y = -0.4800f;
	TheWritableGlobalData->m_terrainLighting[2][2].lightPos.z = -0.3400f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].ambient.red = 0.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].ambient.green = 0.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].ambient.blue = 0.0000f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].diffuse.red = 0.1176f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].diffuse.green = 0.1176f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].diffuse.blue = 0.0784f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].lightPos.x = 0.8100f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].lightPos.y = -0.4800f;
	TheWritableGlobalData->m_terrainObjectsLighting[2][2].lightPos.z = -0.3400f;


	TheWritableGlobalData->m_terrainLighting[3][0].ambient.red = 0.25f;
	TheWritableGlobalData->m_terrainLighting[3][0].ambient.green = 0.23f;
	TheWritableGlobalData->m_terrainLighting[3][0].ambient.blue = 0.20f;
	TheWritableGlobalData->m_terrainLighting[3][0].diffuse.red = 0.60f;
	TheWritableGlobalData->m_terrainLighting[3][0].diffuse.green = 0.50f;
	TheWritableGlobalData->m_terrainLighting[3][0].diffuse.blue = 0.40f;
	TheWritableGlobalData->m_terrainLighting[3][0].lightPos.x = -1.00f;
	TheWritableGlobalData->m_terrainLighting[3][0].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][0].lightPos.z = -0.20f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].ambient.red = 0.25f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].ambient.green = 0.23f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].ambient.blue = 0.20f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].diffuse.red = 0.60f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].diffuse.green = 0.50f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].diffuse.blue = 0.40f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].lightPos.x = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][0].lightPos.z = -0.20f;

	TheWritableGlobalData->m_terrainLighting[3][1].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][1].lightPos.z = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][1].lightPos.z = -1.00f;

	TheWritableGlobalData->m_terrainLighting[3][2].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainLighting[3][2].lightPos.z = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[3][2].lightPos.z = -1.00f;


	TheWritableGlobalData->m_terrainLighting[4][0].ambient.red = 0.10f;
	TheWritableGlobalData->m_terrainLighting[4][0].ambient.green = 0.10f;
	TheWritableGlobalData->m_terrainLighting[4][0].ambient.blue = 0.15f;
	TheWritableGlobalData->m_terrainLighting[4][0].diffuse.red = 0.20f;
	TheWritableGlobalData->m_terrainLighting[4][0].diffuse.green = 0.20f;
	TheWritableGlobalData->m_terrainLighting[4][0].diffuse.blue = 0.30f;
	TheWritableGlobalData->m_terrainLighting[4][0].lightPos.x = -1.00f;
	TheWritableGlobalData->m_terrainLighting[4][0].lightPos.y = 1.00f;
	TheWritableGlobalData->m_terrainLighting[4][0].lightPos.z = -2.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].ambient.red = 0.10f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].ambient.green = 0.10f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].ambient.blue = 0.15f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].diffuse.red = 0.20f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].diffuse.green = 0.20f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].diffuse.blue = 0.30f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].lightPos.x = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].lightPos.y = 1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][0].lightPos.z = -2.00f;

	TheWritableGlobalData->m_terrainLighting[4][1].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][1].lightPos.z = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][1].lightPos.z = -1.00f;

	TheWritableGlobalData->m_terrainLighting[4][2].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainLighting[4][2].lightPos.z = -1.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].ambient.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].ambient.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].ambient.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].diffuse.red = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].diffuse.green = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].diffuse.blue = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].lightPos.x = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].lightPos.y = 0.00f;
	TheWritableGlobalData->m_terrainObjectsLighting[4][2].lightPos.z = -1.00f;

	WbView3d * pView = CWorldBuilderDoc::GetActive3DView();	
	if (pView) {
		pView->setLighting(&TheGlobalData->m_terrainLighting[TheGlobalData->m_timeOfDay][K_SUN], K_TERRAIN, K_SUN);
		pView->setLighting(&TheGlobalData->m_terrainLighting[TheGlobalData->m_timeOfDay][K_ACCENT1], K_TERRAIN, K_ACCENT1);
		pView->setLighting(&TheGlobalData->m_terrainLighting[TheGlobalData->m_timeOfDay][K_ACCENT2], K_TERRAIN, K_ACCENT2);

		pView->setLighting(&TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][K_SUN], K_OBJECTS, K_SUN);
		pView->setLighting(&TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][K_ACCENT1], K_OBJECTS, K_ACCENT1);
		pView->setLighting(&TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][K_ACCENT2], K_OBJECTS, K_ACCENT2);
	}
	stuffValuesIntoFields(K_SUN);
	stuffValuesIntoFields(K_ACCENT1);
	stuffValuesIntoFields(K_ACCENT2);
}

/////////////////////////////////////////////////////////////////////////////
// GlobalLightOptions message handlers

/// Dialog UI initialization.
/** Creates the slider controls, and sets the initial values for 
width and feather in the ui controls. */
BOOL GlobalLightOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	kUIRedIDs[0] = IDC_RD_EDIT;
	kUIRedIDs[1] = IDC_RD_EDIT1;
	kUIRedIDs[2] = IDC_RD_EDIT2;

	kUIGreenIDs[0] = IDC_GD_EDIT;
	kUIGreenIDs[1] = IDC_GD_EDIT1;
	kUIGreenIDs[2] = IDC_GD_EDIT2;

	kUIBlueIDs[0] = IDC_BD_EDIT;
	kUIBlueIDs[1] = IDC_BD_EDIT1;
	kUIBlueIDs[2] = IDC_BD_EDIT2;

	m_frontBackPopup.SetupPopSliderButton(this, IDC_FB_POPUP, this);
	m_leftRightPopup.SetupPopSliderButton(this, IDC_LR_POPUP, this);

	m_frontBackPopupAccent1.SetupPopSliderButton(this, IDC_FB_POPUP1, this);
	m_leftRightPopupAccent1.SetupPopSliderButton(this, IDC_LR_POPUP1, this);

	m_frontBackPopupAccent2.SetupPopSliderButton(this, IDC_FB_POPUP2, this);
	m_leftRightPopupAccent2.SetupPopSliderButton(this, IDC_LR_POPUP2, this);

	CButton *pButton = (CButton *)GetDlgItem(IDC_RADIO_EVERYTHING);
	pButton->SetCheck(1);
	m_lighting = K_BOTH;

	stuffValuesIntoFields(K_SUN);
	stuffValuesIntoFields(K_ACCENT1);
	stuffValuesIntoFields(K_ACCENT2);

	CRect rect;
	CWnd *item = GetDlgItem(IDC_PSEd_Color1);
	if (item) {
		item->GetWindowRect(&rect);
		ScreenToClient(&rect);
		DWORD style = item->GetStyle();
		m_colorButton.Create("", style, rect, this, IDC_PSEd_Color1);
		item->DestroyWindow();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



/** Displays the current values in the fields. */
void GlobalLightOptions::stuffValuesIntoFields(Int lightIndex) 
{
	const GlobalData::TerrainLighting *tl;
	if (m_lighting == K_OBJECTS || m_lighting == K_BOTH) {
		tl = &TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][lightIndex];
	} else {
		tl = &TheGlobalData->m_terrainLighting[TheGlobalData->m_timeOfDay][lightIndex];
	}
	Real azimuth = 90;
	Real elevation = 90;

	Vector3 light(tl->lightPos.x, tl->lightPos.y, tl->lightPos.z);
	light.Normalize();


	Real angleAzimuth = atan2(light.Y,light.X);//WWMath::Asin(light.Y);
	azimuth = angleAzimuth*180.0f/PI;//90-(angleFB/PI)*180;
	if (azimuth < 0) {
		azimuth += 360;
	}
 	Real angleElevation = acos(light.Z);//WWMath::Asin(light.X);
	elevation = (angleElevation-PI/2.0f)*180.0f/PI;//90-(angleLR/PI)*180;

	m_angleElevation[lightIndex] = elevation;
	m_angleAzimuth[lightIndex] = azimuth;

	updateEditFields();

	m_updating = true;
	CString str;
 	str.Format("XYZ: %.2f, %.2f, %.2f", light.X, light.Y, light.Z);
	CWnd *pWnd = this->GetDlgItem(IDC_XYZ_STATIC);
	if (pWnd && lightIndex==0) {
		pWnd->SetWindowText(str);
	}

	switch (lightIndex)
	{
		case K_SUN:
		default:
			PutInt(IDC_RA_EDIT, PercentToComponent(tl->ambient.red));
			PutInt(IDC_GA_EDIT, PercentToComponent(tl->ambient.green));
			PutInt(IDC_BA_EDIT, PercentToComponent(tl->ambient.blue));

			PutInt(IDC_RD_EDIT, PercentToComponent(tl->diffuse.red));
			PutInt(IDC_GD_EDIT, PercentToComponent(tl->diffuse.green));
			PutInt(IDC_BD_EDIT, PercentToComponent(tl->diffuse.blue));
			m_colorButton.setColor(tl->ambient);
			break;

		case K_ACCENT1:
			PutInt(IDC_RD_EDIT1, PercentToComponent(tl->diffuse.red));
			PutInt(IDC_GD_EDIT1, PercentToComponent(tl->diffuse.green));
			PutInt(IDC_BD_EDIT1, PercentToComponent(tl->diffuse.blue));
			break;

		case K_ACCENT2:
			PutInt(IDC_RD_EDIT2, PercentToComponent(tl->diffuse.red));
			PutInt(IDC_GD_EDIT2, PercentToComponent(tl->diffuse.green));
			PutInt(IDC_BD_EDIT2, PercentToComponent(tl->diffuse.blue));
			break;
	}

	m_updating = false;
	pWnd = GetDlgItem(IDC_TIME_OF_DAY_CAPTION);
	if (pWnd) {
		switch (TheGlobalData->m_timeOfDay) {
			default:
			case TIME_OF_DAY_MORNING: pWnd->SetWindowText("Time of day: Morning."); break;
			case TIME_OF_DAY_AFTERNOON: pWnd->SetWindowText("Time of day: Afternoon."); break;
			case TIME_OF_DAY_EVENING: pWnd->SetWindowText("Time of day: Evening."); break;
			case TIME_OF_DAY_NIGHT: pWnd->SetWindowText("Time of day: Night."); break;
		}
	}
	showLightFeedback(lightIndex);
}


/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */

BOOL GlobalLightOptions::GetInt(Int ctrlID, Int *rVal) 
{
	CWnd *pEdit = GetDlgItem(ctrlID);
	char buffer[_MAX_PATH];
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		Int val;
		if (1==sscanf(buffer, "%d", &val)) {
			*rVal = val;
			return true;
		}
	}
	return false;
}

void GlobalLightOptions::PutInt(Int ctrlID, Int val) 
{
	CString str;
	CWnd *pEdit = GetDlgItem(ctrlID);
	if (pEdit) {
		str.Format("%d", val);
		pEdit->SetWindowText(str);
	}
}

void GlobalLightOptions::OnChangeFrontBackEdit() 
{
	if (m_updating) return;
	GetInt(IDC_FB_EDIT, &m_angleAzimuth[K_SUN]);
	GetInt(IDC_FB_EDIT1, &m_angleAzimuth[K_ACCENT1]);
	GetInt(IDC_FB_EDIT2, &m_angleAzimuth[K_ACCENT2]);
	m_updating = true;
	applyAngle(K_SUN);
	applyAngle(K_ACCENT1);
	applyAngle(K_ACCENT2);
	m_updating = false;
}

/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the angles. */
void GlobalLightOptions::OnChangeLeftRightEdit() 
{
	if (m_updating) return;
	GetInt(IDC_LR_EDIT, &m_angleElevation[K_SUN]);
	GetInt(IDC_LR_EDIT1, &m_angleElevation[K_ACCENT1]);
	GetInt(IDC_LR_EDIT2, &m_angleElevation[K_ACCENT2]);
	m_updating = true;
	applyAngle(K_SUN);
	applyAngle(K_ACCENT1);
	applyAngle(K_ACCENT2);
	m_updating = false;
}

/// Handles width edit ui messages.
/** Gets the new edit control text, converts it to an int, then updates
		the slider and brush tool. */
void GlobalLightOptions::applyColor(Int lightIndex)
{
	Int clr;
	GlobalData::TerrainLighting tl;
	if (m_lighting == K_OBJECTS || m_lighting == K_BOTH) {
		tl = TheGlobalData->m_terrainObjectsLighting[TheGlobalData->m_timeOfDay][lightIndex];
	} else {
		tl = TheGlobalData->m_terrainLighting[TheGlobalData->m_timeOfDay][lightIndex];
	}
	if (lightIndex == K_SUN) {
		tl.ambient.red		= TheGlobalData->m_terrainAmbient[K_SUN].red;
		tl.ambient.green	= TheGlobalData->m_terrainAmbient[K_SUN].green;
		tl.ambient.blue		= TheGlobalData->m_terrainAmbient[K_SUN].blue;

		if (GetInt(IDC_RA_EDIT, &clr))
			tl.ambient.red = ComponentToPercent(clr);
		if (GetInt(IDC_GA_EDIT, &clr))
			tl.ambient.green = ComponentToPercent(clr);
		if (GetInt(IDC_BA_EDIT, &clr))
			tl.ambient.blue = ComponentToPercent(clr);
	} else {
		tl.ambient.red		= 0;
		tl.ambient.green	= 0;
		tl.ambient.blue		= 0;
	}

	tl.diffuse.red		= TheGlobalData->m_terrainDiffuse[lightIndex].red;
	tl.diffuse.green	= TheGlobalData->m_terrainDiffuse[lightIndex].green;
	tl.diffuse.blue		= TheGlobalData->m_terrainDiffuse[lightIndex].blue;

	if (GetInt(kUIRedIDs[lightIndex], &clr))
		tl.diffuse.red = ComponentToPercent(clr);
	if (GetInt(kUIGreenIDs[lightIndex], &clr))
		tl.diffuse.green = ComponentToPercent(clr);
	if (GetInt(kUIBlueIDs[lightIndex], &clr))
		tl.diffuse.blue = ComponentToPercent(clr);
	WbView3d * pView = CWorldBuilderDoc::GetActive3DView();	
	if (pView) {
		pView->setLighting(&tl, m_lighting, lightIndex);
	}
}

void GlobalLightOptions::OnChangeColorEdit() 
{
	if (m_updating) return;
	applyColor(K_SUN);
	applyColor(K_ACCENT1);
	applyColor(K_ACCENT2);

}


void GlobalLightOptions::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {

		case IDC_FB_POPUP:
			*pMin = 0;
			*pMax = 359;
			*pInitial = m_angleAzimuth[K_SUN];
			*pLineSize = 1;
			break;
		case IDC_FB_POPUP1:
			*pMin = 0;
			*pMax = 359;
			*pInitial = m_angleAzimuth[K_ACCENT1];
			*pLineSize = 1;
			break;
		case IDC_FB_POPUP2:
			*pMin = 0;
			*pMax = 359;
			*pInitial = m_angleAzimuth[K_ACCENT2];
			*pLineSize = 1;
			break;

		case IDC_LR_POPUP:
			*pMin = 0;
			*pMax = 90;
			*pInitial = m_angleElevation[K_SUN];
			*pLineSize = 1;
			break;
		case IDC_LR_POPUP1:
			*pMin = 0;
			*pMax = 90;
			*pInitial = m_angleElevation[K_ACCENT1];
			*pLineSize = 1;
			break;
		case IDC_LR_POPUP2:
			*pMin = 0;
			*pMax = 90;
			*pInitial = m_angleElevation[K_ACCENT2];
			*pLineSize = 1;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void GlobalLightOptions::PopSliderChanged(const long sliderID, long theVal)
{
	switch (sliderID) {

		case IDC_FB_POPUP:
			m_angleAzimuth[K_SUN] = theVal;
			PutInt(IDC_FB_EDIT, m_angleAzimuth[K_SUN]);
			applyAngle(0);
			break;

		case IDC_FB_POPUP1:
			m_angleAzimuth[K_ACCENT1] = theVal;
			PutInt(IDC_FB_EDIT1, m_angleAzimuth[K_ACCENT1]);
			applyAngle(1);
			break;

		case IDC_FB_POPUP2:
			m_angleAzimuth[K_ACCENT2] = theVal;
			PutInt(IDC_FB_EDIT2, m_angleAzimuth[K_ACCENT2]);
			applyAngle(2);
			break;

		case IDC_LR_POPUP:
			m_angleElevation[K_SUN] = theVal;
			PutInt(IDC_LR_EDIT, m_angleElevation[K_SUN]);
			applyAngle(0);
			break;

		case IDC_LR_POPUP1:
			m_angleElevation[K_ACCENT1] = theVal;
			PutInt(IDC_LR_EDIT1, m_angleElevation[K_ACCENT1]);
			applyAngle(1);
			break;

		case IDC_LR_POPUP2:
			m_angleElevation[K_ACCENT2] = theVal;
			PutInt(IDC_LR_EDIT2, m_angleElevation[K_ACCENT2]);
			applyAngle(2);
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void GlobalLightOptions::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_FB_POPUP:
			break;
		case IDC_LR_POPUP:
			break;
		case IDC_FB_POPUP1:
			break;
		case IDC_LR_POPUP1:
			break;
		case IDC_FB_POPUP2:
			break;
		case IDC_LR_POPUP2:
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch

}


BEGIN_MESSAGE_MAP(GlobalLightOptions, CDialog)
	//{{AFX_MSG_MAP(GlobalLightOptions)
	ON_WM_MOVE()
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_EN_CHANGE(IDC_RA_EDIT, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_BA_EDIT, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_GA_EDIT, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_FB_EDIT, OnChangeFrontBackEdit)
	ON_EN_CHANGE(IDC_FB_EDIT1, OnChangeFrontBackEdit)
	ON_EN_CHANGE(IDC_FB_EDIT2, OnChangeFrontBackEdit)
	ON_EN_CHANGE(IDC_LR_EDIT, OnChangeLeftRightEdit)
	ON_EN_CHANGE(IDC_LR_EDIT1, OnChangeLeftRightEdit)
	ON_EN_CHANGE(IDC_LR_EDIT2, OnChangeLeftRightEdit)
	ON_EN_CHANGE(IDC_RD_EDIT, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_GD_EDIT, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_BD_EDIT, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_RD_EDIT1, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_GD_EDIT1, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_BD_EDIT1, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_RD_EDIT2, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_GD_EDIT2, OnChangeColorEdit)
	ON_EN_CHANGE(IDC_BD_EDIT2, OnChangeColorEdit)
	ON_BN_CLICKED(IDC_RADIO_EVERYTHING, OnRadioEverything)
	ON_BN_CLICKED(IDC_RADIO_OBJECTS, OnRadioObjects)
	ON_BN_CLICKED(IDC_RADIO_TERRAIN, OnRadioTerrain)
	ON_BN_CLICKED(IDC_PSEd_Color1, OnColorPress)
	ON_BN_CLICKED(IDC_GlobalLightingReset, OnResetLights)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void GlobalLightOptions::OnRadioEverything() 
{
	m_lighting = K_BOTH;
	stuffValuesIntoFields(K_SUN);
	stuffValuesIntoFields(K_ACCENT1);
	stuffValuesIntoFields(K_ACCENT2);
}

void GlobalLightOptions::OnRadioObjects() 
{
	m_lighting = K_OBJECTS;
	stuffValuesIntoFields(K_SUN);
	stuffValuesIntoFields(K_ACCENT1);
	stuffValuesIntoFields(K_ACCENT2);
}

void GlobalLightOptions::OnRadioTerrain() 
{
	m_lighting = K_TERRAIN;
	stuffValuesIntoFields(K_SUN);
	stuffValuesIntoFields(K_ACCENT1);
	stuffValuesIntoFields(K_ACCENT2);
}

void GlobalLightOptions::OnColorPress()
{
	CColorDialog dlg;
	if (dlg.DoModal() == IDOK) {
		m_colorButton.setColor(CButtonShowColor::BGRtoRGB(dlg.GetColor()));
		RGBColor color = m_colorButton.getColor();
		PutInt(IDC_RA_EDIT, PercentToComponent(color.red));
		PutInt(IDC_GA_EDIT, PercentToComponent(color.green));
		PutInt(IDC_BA_EDIT, PercentToComponent(color.blue));
		applyColor(K_SUN);
	}
}

void GlobalLightOptions::OnClose()
{
	ShowWindow(SW_HIDE);

	WbView3d * pView = CWorldBuilderDoc::GetActive3DView();	
	if (pView) {
		Coord3D lightRay; 
		lightRay.x=0.0f;lightRay.y=0.0f;lightRay.z=-1.0f;	//default light above terrain.
		pView->doLightFeedback(false,lightRay,0);	//turn off the light direction indicator
	}
};

void GlobalLightOptions::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	stuffValuesIntoFields(K_SUN);
	stuffValuesIntoFields(K_ACCENT1);
	stuffValuesIntoFields(K_ACCENT2);
	if (!bShow) {
		WbView3d * pView = CWorldBuilderDoc::GetActive3DView();
		if (pView) {
			Coord3D lightRay;
			lightRay.x=0.0f;lightRay.y=0.0f;lightRay.z=-1.0f;	//default light above terrain.
			pView->doLightFeedback(false,lightRay,0);	//turn off the light direction indicator
			pView->doLightFeedback(false,lightRay,1);	//turn off the light direction indicator
			pView->doLightFeedback(false,lightRay,2);	//turn off the light direction indicator
		}
	}
#if 0
	SpitLights();
#endif
};

void GlobalLightOptions::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(GLOBALLIGHT_OPTIONS_PANEL_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(GLOBALLIGHT_OPTIONS_PANEL_SECTION, "Left", frameRect.left);
	}
	
}
