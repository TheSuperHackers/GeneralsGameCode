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

// CameraOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "WorldBuilder.h"
#include "CameraOptions.h"
#include "wbview3d.h"
#include "WorldBuilderDoc.h"


#include "WaypointOptions.h" //WST 10/7/2002
#include "CUndoable.h" //WST 10/7/2002

/////////////////////////////////////////////////////////////////////////////
// CameraOptions dialog


CameraOptions::CameraOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CameraOptions::IDD, pParent)
{
	m_updating = false;
	//{{AFX_DATA_INIT(CameraOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CameraOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CameraOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CameraOptions, CDialog)
	//{{AFX_MSG_MAP(CameraOptions)
	ON_BN_CLICKED(IDC_CameraReset, OnCameraReset)
	ON_BN_CLICKED(IDC_DROP_WAYPOINT_BUTTON, OnDropWaypointButton)
	ON_BN_CLICKED(IDC_CENTER_ON_SELECTED, OnCenterOnSelectedButton)
	ON_WM_MOVE()
	ON_EN_CHANGE(IDC_PITCH_EDIT, OnChangePitchEdit)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CameraOptions message handlers

void CameraOptions::OnCameraReset()
{
	WbView3d * p3View = CWorldBuilderDoc::GetActive3DView();
	if (p3View)
	{
		p3View->setDefaultCamera();
		update();
	}
}

//WST 10/7/2002 - Drop waypoint button for Adam Isgreen -----------
void CameraOptions::OnDropWaypointButton()
{
	//The following code is taken from waypointTool.cpp. On mouse down.
	WbView3d		* p3View = CWorldBuilderDoc::GetActive3DView();
	Coord3D			docPt;
	CWorldBuilderDoc *pDoc	= CWorldBuilderDoc::GetActiveDoc();

	Vector3 camTarget = p3View->getCameraTarget();
	docPt.x = camTarget.X;
	docPt.y = camTarget.Y;
	docPt.z = MAGIC_GROUND_Z;

	//
	// MBL CNC3 INCURSION 10.29.2002 - Fix compile error w/ 10-15-2002 Drop
	//
	// MapObject *pNew = new MapObject(docPt, "*Waypoints/Waypoint", 0, 0, NULL, NULL );
	MapObject *pNew = newInstance(MapObject)(docPt, "*Waypoints/Waypoint", 0, 0, NULL, NULL );

	Int id = pDoc->getNextWaypointID();
	AsciiString name = WaypointOptions::GenerateUniqueName(id);
	pNew->setSelected(true);
	pNew->setIsWaypoint();
	pNew->setWaypointID(id);
	pNew->setWaypointName(name);
	pNew->getProperties()->setAsciiString(TheKey_originalOwner, "team");
	AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	pNew = NULL; // undoable owns it now.
}

//WST 11/25/2002 - New Center Camera button for Designers -----------
void CameraOptions::OnCenterOnSelectedButton()
{
	// Center camera on the selected map object

	int count = 0;
	const Coord3D *objectPosition;

	MapObject *mapObject = MapObject::getFirstMapObject();
	while (mapObject) {
		if (mapObject->isSelected()) {
			objectPosition = mapObject->getLocation();
			count++;
		}
		mapObject = mapObject->getNext();
	}

	if (count==1)	{
		// Only center if there is only one object selected on map
		WbView3d * p3View = CWorldBuilderDoc::GetActive3DView();
		if (p3View)	{
			p3View->setCenterInView(objectPosition->x/MAP_XY_FACTOR,objectPosition->y/MAP_XY_FACTOR);
		}
	}
}


void CameraOptions::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(CAMERA_OPTIONS_PANEL_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(CAMERA_OPTIONS_PANEL_SECTION, "Left", frameRect.left);
	}

}

void CameraOptions::putInt(Int ctrlID, Int val)
{
	CString str;
	CWnd *pEdit = GetDlgItem(ctrlID);
	if (pEdit) {
		str.Format("%d", val);
		pEdit->SetWindowText(str);
	}
}

void CameraOptions::putReal(Int ctrlID, Real val)
{
	CString str;
	CWnd *pEdit = GetDlgItem(ctrlID);
	if (pEdit) {
		str.Format("%g", val);
		pEdit->SetWindowText(str);
	}
}

void CameraOptions::putAsciiString(Int ctrlID, AsciiString val)
{
	CString str;
	CWnd *pEdit = GetDlgItem(ctrlID);
	if (pEdit) {
		str.Format("%s", val.str());
		pEdit->SetWindowText(str);
	}
}

BOOL CameraOptions::getReal(Int ctrlID, Real *rVal)
{
	CWnd *pEdit = GetDlgItem(ctrlID);
	char buffer[_MAX_PATH];
	if (pEdit) {
		pEdit->GetWindowText(buffer, sizeof(buffer));
		Real val;
		if (1==sscanf(buffer, "%f", &val)) {
			*rVal = val;
			return true;
		}
	}
	return false;
}

void CameraOptions::stuffValuesIntoFields( void )
{
	WbView3d * p3View = CWorldBuilderDoc::GetActive3DView();
	if (p3View)
	{
		m_updating = true;
		putReal(IDC_PITCH_EDIT, p3View->getCameraPitch());
		m_updating = false;

		Real height = p3View->getHeightAboveGround();
		//WST 10/11/2002 This is inaccurate Real zoom = height/TheGlobalData->m_maxCameraHeight;
		Real zoom = p3View->getCurrentZoom(); //WST 10.11.2002

		putReal(IDC_ZOOMTEXT, zoom);
		putReal(IDC_HEIGHTTEXT, height);

		Vector3 source = p3View->getCameraSource();
		Vector3 target = p3View->getCameraTarget();
//		Real angle = p3View->getCameraAngle();

		AsciiString s;
		s.format("(%g, %g)", target.X, target.Y);
		putAsciiString(IDC_POSTEXT, s);
		s.format("(%g, %g)", target.X - 1.0f*(source.X-target.X), target.Y - 1.0f*(source.Y-target.Y));
		putAsciiString(IDC_TARGETTEXT, s);
	}
}

void CameraOptions::update( void )
{
	stuffValuesIntoFields();
}

void CameraOptions::applyCameraPitch( Real pitch )
{
	WbView3d * p3View = CWorldBuilderDoc::GetActive3DView();
	if (p3View)
	{
		p3View->setCameraPitch(pitch);
	}
}



void CameraOptions::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {

		case IDC_PITCH_POPUP:
			*pMin = 0;
			*pMax = 100;
			*pInitial = 100;
			*pLineSize = 1;
			{
				WbView3d * p3View = CWorldBuilderDoc::GetActive3DView();
				if (p3View)
				{
					*pInitial = (Int)(100.0f * p3View->getCameraPitch());
				}
			}
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}
}

void CameraOptions::PopSliderChanged(const long sliderID, long theVal)
{
	switch (sliderID) {

		case IDC_PITCH_POPUP:
			putReal(IDC_PITCH_EDIT, ((Real)theVal)*0.01f);
			break;


		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}
}

void CameraOptions::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_PITCH_POPUP:
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}

}


BOOL CameraOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pitchPopup.SetupPopSliderButton(this, IDC_PITCH_POPUP, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CameraOptions::OnChangePitchEdit()
{
	if (m_updating)
		return;

	Real pitch;
	if (getReal(IDC_PITCH_EDIT, &pitch))
	{
		m_updating = true;
		applyCameraPitch(pitch);
		m_updating = false;
	}
}

void CameraOptions::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	update();
}
