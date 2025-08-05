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

// ScorchOptions.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "ScorchOptions.h"
#include "CUndoable.h"
#include "wbview3d.h"
#include "Common/WellKnownKeys.h"
#include "WorldBuilderDoc.h"

#define DEFAULT_SCORCHMARK_RADIUS 20

Scorches ScorchOptions::m_scorchtype = SCORCH_1;
Real ScorchOptions::m_scorchsize = DEFAULT_SCORCHMARK_RADIUS;
ScorchOptions *ScorchOptions::m_staticThis = NULL;

/////////////////////////////////////////////////////////////////////////////
// ScorchOptions dialog


ScorchOptions::ScorchOptions(CWnd* pParent /*=NULL*/)
{
	//{{AFX_DATA_INIT(ScorchOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ScorchOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScorchOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ScorchOptions, CDialog)
	//{{AFX_MSG_MAP(ScorchOptions)
	ON_CBN_SELENDOK(IDC_SCORCHTYPE, OnChangeScorchtype)
	ON_EN_CHANGE(IDC_SIZE_EDIT, OnChangeSizeEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

MapObject *ScorchOptions::getSingleSelectedScorch(void)
{
	MapObject *theMapObj = NULL;
//	Bool found = false;
	Int selCount=0;
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->isSelected()) {
			if (pMapObj->isScorch()) {
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

void ScorchOptions::updateTheUI(void)
{
	m_updating = true;
	MapObject *theMapObj = getSingleSelectedScorch();
	CString str;
	CWnd *pEdit;
	if (theMapObj) {
		m_scorchtype = (Scorches) theMapObj->getProperties()->getInt(TheKey_scorchType);
		m_scorchsize = theMapObj->getProperties()->getReal(TheKey_objectRadius);
	}
	CComboBox *scorch = (CComboBox*)GetDlgItem(IDC_SCORCHTYPE);

	scorch->SetCurSel((int)m_scorchtype);
	str.Format("%f",m_scorchsize);
	pEdit = GetDlgItem(IDC_SIZE_EDIT);
	if (pEdit)
		pEdit->SetWindowText(str);
	m_updating = false;
}

void ScorchOptions::update(void)
{
	if (m_staticThis) {
		m_staticThis->updateTheUI();
	}
}

/////////////////////////////////////////////////////////////////////////////
// ScorchOptions message handlers

BOOL ScorchOptions::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_staticThis = this;


	m_radiusPopup.SetupPopSliderButton(this, IDC_SIZE_POPUP, this);

	CString str;
	CComboBox *scorch = (CComboBox*)GetDlgItem(IDC_SCORCHTYPE);
/* Use the values in the .rc file.  jba.
	scorch->ResetContent();
	for (Int i = 0; i < SCORCH_COUNT; i++)
	{
		str.Format("Scorch %d", i);
		scorch->InsertString(-1, str);
	}
*/
	scorch->SetCurSel(0);

	update();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ScorchOptions::OnChangeScorchtype()
{
	if (m_updating)
		return;
	CComboBox *scorch = (CComboBox*)GetDlgItem(IDC_SCORCHTYPE);
	int curSel = scorch->GetCurSel();
	m_scorchtype = (Scorches) curSel;
	changeScorch();
}

void ScorchOptions::OnChangeSizeEdit()
{
	if (m_updating)
		return;
	CWnd* edit = GetDlgItem(IDC_SIZE_EDIT);
	CString cstr;
	edit->GetWindowText(cstr);
	if (!cstr.IsEmpty()) {
		m_scorchsize = atof(cstr.GetBuffer(0));
	}
	changeSize();
}


void ScorchOptions::GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial)
{
	switch (sliderID) {
		case IDC_SIZE_POPUP:
			*pMin = 0;
			*pMax = 256;
			*pInitial = m_scorchsize;
			*pLineSize = 1;
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
}

void ScorchOptions::PopSliderChanged(const long sliderID, long theVal)
{
	CString str;
	CWnd *pEdit;
	m_updating = true;
	switch (sliderID) {
		case IDC_SIZE_POPUP:
			m_scorchsize = theVal;
			str.Format("%f",m_scorchsize);
			pEdit = m_staticThis->GetDlgItem(IDC_SIZE_EDIT);
			if (pEdit) pEdit->SetWindowText(str);
			break;


		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch
	m_updating = false;
}

void ScorchOptions::PopSliderFinished(const long sliderID, long theVal)
{
	switch (sliderID) {
		case IDC_SIZE_POPUP:
			changeSize();
			break;

		default:
			// uh-oh!
			DEBUG_CRASH(("Slider message from unknown control"));
			break;
	}	// switch

}

void ScorchOptions::changeScorch(void)
{
	getAllSelectedDicts();

	Dict newDict;
	newDict.setInt(TheKey_scorchType, (Int)m_scorchtype);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.

	WbView3d *pView = CWorldBuilderDoc::GetActive3DView();
	pView->Invalidate();
}

void ScorchOptions::changeSize(void)
{
	getAllSelectedDicts();

	Dict newDict;
	newDict.setReal(TheKey_objectRadius, m_scorchsize);
	DictItemUndoable *pUndo = new DictItemUndoable(getAllSelectedDictsData(), newDict, newDict.getNthKey(0), m_allSelectedDicts.size());
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.

	WbView3d *pView = CWorldBuilderDoc::GetActive3DView();
	pView->Invalidate();
}

void ScorchOptions::getAllSelectedDicts(void)
{
	m_allSelectedDicts.clear();

	for (MapObject *pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (!pMapObj->isSelected() || !pMapObj->isScorch()) {
			continue;
		}
		m_allSelectedDicts.push_back(pMapObj->getProperties());
	}
}

Dict** ScorchOptions::getAllSelectedDictsData()
{
#if defined(USING_STLPORT) || __cplusplus < 201103L
	return !m_allSelectedDicts.empty() ? &m_allSelectedDicts.front() : NULL;
#else
	return m_allSelectedDicts.data();
#endif
}
