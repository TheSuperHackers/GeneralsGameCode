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

// MeshMoldOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "MeshMoldOptions.h"
#include "Common/FileSystem.h"

/////////////////////////////////////////////////////////////////////////////
// MeshMoldOptions dialog

Real MeshMoldOptions::m_currentHeight=0;
Real MeshMoldOptions::m_currentScale=1.0f;
Int MeshMoldOptions::m_currentAngle=0;
MeshMoldOptions * MeshMoldOptions::m_staticThis=nullptr;
Bool MeshMoldOptions::m_doingPreview=false;
Bool MeshMoldOptions::m_raiseOnly=false;
Bool MeshMoldOptions::m_lowerOnly=false;

MeshMoldOptions::MeshMoldOptions(CWnd* pParent /*=nullptr*/)
{
	//{{AFX_DATA_INIT(MeshMoldOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MeshMoldOptions::DoDataExchange(CDataExchange* pDX)
{
	COptionsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MeshMoldOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

/// Dialog UI initialization.
/** Creates the slider controls, and sets the initial values for
width and feather in the ui controls. */
BOOL MeshMoldOptions::OnInitDialog()
{
	COptionsPanel::OnInitDialog();

	m_updating = true;
	m_anglePopup.SetupPopSliderButton(this, IDC_ANGLE_POPUP, this);
	m_scalePopup.SetupPopSliderButton(this, IDC_SCALE_POPUP, this);
	m_HeightPopup.SetupPopSliderButton(this, IDC_HEIGHT_POPUP, this);

	CWnd *pWnd = GetDlgItem(IDC_MESHMOLD_TREEVIEW);
	CRect rect(0,0,100,100);
	if (pWnd) pWnd->GetWindowRect(&rect);

	CButton *pRaise = (CButton *)GetDlgItem(IDC_RAISE_LOWER);
	if (pRaise) {
		pRaise->SetCheck(1);
	}

	ScreenToClient(&rect);
	rect.DeflateRect(2,2,2,2);
	m_moldTreeView.Create(TVS_HASLINES|TVS_LINESATROOT|TVS_HASBUTTONS|
		TVS_SHOWSELALWAYS|TVS_DISABLEDRAGDROP, rect, this, IDC_TERRAIN_TREEVIEW);
	m_moldTreeView.ShowWindow(SW_SHOW);

	{
		char				fileBuf[_MAX_PATH];
		Int					i;

		FilenameList filenameList;
		TheFileSystem->getFileListInDirectory(".\\data\\Editor\\Molds\\", "*.w3d", filenameList, FALSE);

		if (filenameList.size() > 0) {
			HTREEITEM child = nullptr;
			FilenameList::iterator it = filenameList.begin();
			do {
				AsciiString filename = *it;

				int len = filename.getLength();
				if (len<5) continue;
				strlcpy(fileBuf, filename.str(), ARRAY_SIZE(fileBuf));
				for (i=strlen(fileBuf)-1; i>0; i--) {
					if (fileBuf[i] == '.') {
						// strip off .w3d file extension.
						fileBuf[i] = 0;
					}
				}
				char *nameStart = fileBuf;
				for (i=0; i<strlen(fileBuf)-1; i++) {
					if (fileBuf[i] == '\\') {
						nameStart = fileBuf+i+1;
					}
				}

				TVINSERTSTRUCT ins;
				// not found, so add it.
				::memset(&ins, 0, sizeof(ins));
				ins.hParent = TVI_ROOT;
				ins.hInsertAfter = TVI_SORT;
				ins.item.mask = TVIF_PARAM|TVIF_TEXT;
				ins.item.lParam = -1;
				ins.item.pszText = nameStart;
				ins.item.cchTextMax = strlen(nameStart);
				child = m_moldTreeView.InsertItem(&ins);

				++it;
			} while (it != filenameList.end());
 			if (child) m_moldTreeView.SelectItem(child);
 		}
	}

	m_staticThis = this;
	m_updating = false;
	setAngle(m_currentAngle);
	setScale(m_currentScale);
	setHeight(m_currentHeight);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MeshMoldOptions::setHeight(Real height)
{
	char buffer[32];
	snprintf(buffer, ARRAY_SIZE(buffer), "%.2f", height);
	m_currentHeight = height;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
		if (pEdit) pEdit->SetWindowText(buffer);
	}
	MeshMoldTool::updateMeshLocation(false);
}

void MeshMoldOptions::setScale(Real scale)
{
	char buffer[12];
	snprintf(buffer, ARRAY_SIZE(buffer), "%d", (int)floor(scale*100));
	m_currentScale = scale;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SCALE_EDIT);
		if (pEdit) pEdit->SetWindowText(buffer);
	}
	MeshMoldTool::updateMeshLocation(false);
}

void MeshMoldOptions::setAngle(Int angle)
{
	char buffer[12];
	snprintf(buffer, ARRAY_SIZE(buffer), "%d", angle);
	m_currentAngle = angle;
	if (m_staticThis && !m_staticThis->m_updating) {
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_ANGLE_EDIT);
		if (pEdit) pEdit->SetWindowText(buffer);
	}
}


void MeshMoldOptions::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {
		case IDC_ANGLE_POPUP:
			*pMin = MIN_ANGLE;
			*pMax = MAX_ANGLE;
			*pInitial = m_currentAngle;
			*pLineSize = 1;
			break;

		case IDC_HEIGHT_POPUP:
			*pMin = MIN_HEIGHT;
			*pMax = MAX_HEIGHT;
			*pInitial = floor(m_currentHeight/MAP_HEIGHT_SCALE+0.5f);
			*pLineSize = 1;
			break;

		case IDC_SCALE_POPUP:
			*pMin = MIN_SCALE;
			*pMax = MAX_SCALE;
			*pInitial = floor(m_currentScale*100 + 0.5f);
			if (*pInitial > *pMax/2) *pMax = *pInitial*2;
			*pLineSize = 1;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Missing ID."));
			break;
	}
}

void MeshMoldOptions::PopSliderChanged(const long sliderID, long theVal)
{
	CString str;
	CWnd *pEdit;
	switch (sliderID) {

		case IDC_ANGLE_POPUP:
			m_currentAngle = theVal;
			str.Format("%d",m_currentAngle);
			pEdit = m_staticThis->GetDlgItem(IDC_ANGLE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			break;

		case IDC_HEIGHT_POPUP:
			m_currentHeight = theVal*MAP_HEIGHT_SCALE;
			str.Format("%.2f",m_currentHeight);
			pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			MeshMoldTool::updateMeshLocation(false);
			break;

		case IDC_SCALE_POPUP:
			m_currentScale = theVal/100.0f;  //scale is in %
			str.Format("%d",theVal);
			pEdit = m_staticThis->GetDlgItem(IDC_SCALE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			MeshMoldTool::updateMeshLocation(false);
			break;

		default:
			DEBUG_CRASH(("Missing ID."));
			break;
	}
}

void MeshMoldOptions::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_ANGLE_POPUP:
			break;
		case IDC_HEIGHT_POPUP:
			break;
		case IDC_SCALE_POPUP:
			break;

		default:
			DEBUG_CRASH(("Missing ID."));
			break;
	}

}

BEGIN_MESSAGE_MAP(MeshMoldOptions, COptionsPanel)
	//{{AFX_MSG_MAP(MeshMoldOptions)
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
	ON_BN_CLICKED(IDC_APPLY_MESH, OnApplyMesh)
	ON_EN_CHANGE(IDC_SCALE_EDIT, OnChangeScaleEdit)
	ON_EN_CHANGE(IDC_HEIGHT_EDIT, OnChangeHeightEdit)
	ON_EN_CHANGE(IDC_ANGLE_EDIT, OnChangeAngleEdit)
	ON_BN_CLICKED(IDC_RAISE, OnRaise)
	ON_BN_CLICKED(IDC_RAISE_LOWER, OnRaiseLower)
	ON_BN_CLICKED(IDC_LOWER, OnLower)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MeshMoldOptions message handlers

void MeshMoldOptions::OnPreview()
{
	CButton *pButton = (CButton*)this->GetDlgItem(IDC_PREVIEW);
	m_doingPreview = (pButton->GetCheck() == 1);
	MeshMoldTool::updateMeshLocation(true);
}

void MeshMoldOptions::OnApplyMesh()
{
	MeshMoldTool::apply(CWorldBuilderDoc::GetActiveDoc());
}

void MeshMoldOptions::OnChangeScaleEdit()
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_SCALE_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int scale;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &scale)) {
				m_currentScale = scale/100.0f;
				MeshMoldTool::updateMeshLocation(false);
			}
			m_updating = false;
		}
}

void MeshMoldOptions::OnChangeHeightEdit()
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_HEIGHT_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Real height;
			m_updating = true;
			if (1==sscanf(buffer, "%f", &height)) {
				m_currentHeight = height;
				MeshMoldTool::updateMeshLocation(false);
			}
			m_updating = false;
		}
}

void MeshMoldOptions::OnChangeAngleEdit()
{
		if (m_updating) return;
		CWnd *pEdit = m_staticThis->GetDlgItem(IDC_ANGLE_EDIT);
		char buffer[_MAX_PATH];
		if (pEdit) {
			pEdit->GetWindowText(buffer, sizeof(buffer));
			Int angle;
			m_updating = true;
			if (1==sscanf(buffer, "%d", &angle)) {
				m_currentAngle = angle;
				MeshMoldTool::updateMeshLocation(false);
			}
			m_updating = false;
		}
}


BOOL MeshMoldOptions::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMTREEVIEW *pHdr = (NMTREEVIEW *)lParam;
	if (pHdr->hdr.hwndFrom == m_moldTreeView.m_hWnd) {

		if (pHdr->hdr.code == TVN_SELCHANGED) {
			char buffer[_MAX_PATH];
			HTREEITEM hItem = m_moldTreeView.GetSelectedItem();
			TVITEM item;
			::memset(&item, 0, sizeof(item));
			item.mask = TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
			item.hItem = hItem;
			item.pszText = buffer;
			item.cchTextMax = sizeof(buffer)-2;
			m_moldTreeView.GetItem(&item);
			m_meshModelName = AsciiString(buffer);
			if (m_doingPreview) {
				MeshMoldTool::updateMeshLocation(false);
			}
		}
	}
	return COptionsPanel::OnNotify(wParam, lParam, pResult);
}


void MeshMoldOptions::OnRaise()
{
	m_raiseOnly = true;
	m_lowerOnly = false;
	if (m_doingPreview) {
		MeshMoldTool::updateMeshLocation(false);
	}
}

void MeshMoldOptions::OnRaiseLower()
{
	m_raiseOnly = false;
	m_lowerOnly = false;
	if (m_doingPreview) {
		MeshMoldTool::updateMeshLocation(false);
	}
}

void MeshMoldOptions::OnLower()
{
	m_raiseOnly = false;
	m_lowerOnly = true;
	if (m_doingPreview) {
		MeshMoldTool::updateMeshLocation(false);
	}
}
