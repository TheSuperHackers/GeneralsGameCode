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

#include "StdAfx.h"
#include "Common/STLTypedefs.h"
#include "GroveOptions.h"
#include "Common/ThingFactory.h"
#include "Common/ThingSort.h"
#include "Common/ThingTemplate.h"

#define ARBITRARY_BUFF_SIZE		128

/*extern*/ GroveOptions *TheGroveOptions = NULL;

void GroveOptions::makeMain(void)
{
	TheGroveOptions = this;
}

GroveOptions::GroveOptions(CWnd* pParent)
{

}

int GroveOptions::getNumTrees(void)
{
	CWnd* pWnd = GetDlgItem(IDC_Grove_NumberTrees);
	if (!pWnd) {
		return 0;
	}

	static char buff[ARBITRARY_BUFF_SIZE];
	pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
	return atoi(buff);
}


int GroveOptions::getNumType(int type)
{
	static char buff[ARBITRARY_BUFF_SIZE];

	if (type < 1 || type > 5) {
		return -1;
	}

	CWnd *pWnd;
	CComboBox* pBox;
	if (type == 1) {
		pWnd = GetDlgItem(IDC_Grove_Per1);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1);
	} else if (type == 2) {
		pWnd = GetDlgItem(IDC_Grove_Per2);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type2);
	} else if (type == 3) {
		pWnd = GetDlgItem(IDC_Grove_Per3);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type3);
	} else if (type == 4) {
		pWnd = GetDlgItem(IDC_Grove_Per4);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type4);
	} else if (type == 5) {
		pWnd = GetDlgItem(IDC_Grove_Per5);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type5);
	}

	if (pWnd && pBox) {
		if (pBox->GetCurSel() > 0) {
			pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
			return atoi(buff);
		}
		return 0;
	}
	return -1;
}

AsciiString GroveOptions::getTypeName(int type)
{
	if (type < 1 || type > 5) {
		return "";
	}

	CComboBox *pComboBox;
	if (type == 1) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1);
	} else if (type == 2) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type2);
	} else if (type == 3) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type3);
	} else if (type == 4) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type4);
	} else if (type == 5) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type5);
	}

	int curSel = pComboBox->GetCurSel();
	if (curSel < 0 || curSel > mVecDisplayNames.size()) {
		return "";
	}
	CString cstr;

	pComboBox->GetLBText(curSel, cstr);

	return cstr.GetBuffer(0);
}

int GroveOptions::getTotalTreePerc(void)
{
	static char buff[ARBITRARY_BUFF_SIZE];

	CWnd* pWnd = GetDlgItem(IDC_Grove_PerTotal);
	if (!pWnd) {
		return -1;
	}

	if (pWnd) {
		pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
		return atoi(buff);
	}
	return -1;
}

Bool GroveOptions::getCanPlaceInWater(void)
{
	CButton* pButt;

	pButt = (CButton*) GetDlgItem(IDC_Grove_AllowWaterPlacement);
	if (pButt) {
		return (pButt->GetState() != 0);
	}
	return false;
}

Bool GroveOptions::getCanPlaceOnCliffs(void)
{
	CButton* pButt;

	pButt = (CButton*) GetDlgItem(IDC_Grove_AllowCliffPlacement);
	if (pButt) {
		return (pButt->GetState() != 0);
	}
	return false;
}

BOOL GroveOptions::OnInitDialog()
{
	_buildTreeList();
	_setTreesToLists();
	_setDefaultRatios();
	_updateTreeWeights();
	_setDefaultNumTrees();
	_setDefaultPlacementAllowed();
	return true;
}

GroveOptions::~GroveOptions()
{
	TheGroveOptions = NULL;
}


void GroveOptions::_setTreesToLists(void)
{
	CString str;
	for (VecPairNameDisplayNameIt it = mVecDisplayNames.begin(); it != mVecDisplayNames.end(); it++) {
		// TODO: If/when Models get Display strings, we need to replace the
		// current (str = ...) line with the commented one JKMCD
		str = it->first.str();
		//str = GetDisplayNameFromPair(it).str();

		CComboBox* pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1);
		if (pComboBox) {
			pComboBox->AddString(str);
		}

		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type2);
		if (pComboBox) {
			pComboBox->AddString(str);
		}

		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type3);
		if (pComboBox) {
			pComboBox->AddString(str);
		}

		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type4);
		if (pComboBox) {
			pComboBox->AddString(str);
		}

		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type5);
		if (pComboBox) {
			pComboBox->AddString(str);
		}
	}

	int selValue;
	str = "";
	CComboBox* pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1);
	if (pComboBox) {
		pComboBox->AddString(str);
		selValue = AfxGetApp()->GetProfileInt("GroveOptions", "TreeType1", 0);
		pComboBox->SetCurSel(selValue % mVecDisplayNames.size());
	}

	pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type2);
	if (pComboBox) {
		pComboBox->AddString(str);
		selValue = AfxGetApp()->GetProfileInt("GroveOptions", "TreeType2", 0);
		pComboBox->SetCurSel(selValue % mVecDisplayNames.size());
	}

	pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type3);
	if (pComboBox) {
		pComboBox->AddString(str);
		selValue = AfxGetApp()->GetProfileInt("GroveOptions", "TreeType3", 0);
		pComboBox->SetCurSel(selValue % mVecDisplayNames.size());
	}

	pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type4);
	if (pComboBox) {
		pComboBox->AddString(str);
		selValue = AfxGetApp()->GetProfileInt("GroveOptions", "TreeType4", 0);
		pComboBox->SetCurSel(selValue % mVecDisplayNames.size());
	}

	pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type5);
	if (pComboBox) {
		pComboBox->AddString(str);
		selValue = AfxGetApp()->GetProfileInt("GroveOptions", "TreeType5", 0);
		pComboBox->SetCurSel(selValue % mVecDisplayNames.size());
	}
}

void GroveOptions::_buildTreeList(void)
{
	const ThingTemplate* pTemplate;
	for (pTemplate = TheThingFactory->firstTemplate(); pTemplate; pTemplate = pTemplate->friend_getNextTemplate()) {
		if (pTemplate->getEditorSorting() == ES_SHRUBBERY) {
			PairNameDisplayName currentName;
			currentName.first = pTemplate->getName();
			currentName.second = pTemplate->getDisplayName();
			mVecDisplayNames.push_back(currentName);
		}
	}
}

void GroveOptions::_setDefaultRatios(void)
{
	static char buff[ARBITRARY_BUFF_SIZE];
	int defaultRatio;

	CWnd* pWnd = GetDlgItem(IDC_Grove_Per1);
	if (pWnd) {
		defaultRatio = AfxGetApp()->GetProfileInt("GroveOptions", "DefaultRatio1", 0);
		sprintf(buff, "%d", defaultRatio);
		pWnd->SetWindowText(buff);
	}

	pWnd = GetDlgItem(IDC_Grove_Per2);
	if (pWnd) {
		defaultRatio = AfxGetApp()->GetProfileInt("GroveOptions", "DefaultRatio2", 0);
		sprintf(buff, "%d", defaultRatio);
		pWnd->SetWindowText(buff);
	}

	pWnd = GetDlgItem(IDC_Grove_Per3);
	if (pWnd) {
		defaultRatio = AfxGetApp()->GetProfileInt("GroveOptions", "DefaultRatio3", 0);
		sprintf(buff, "%d", defaultRatio);
		pWnd->SetWindowText(buff);
	}

	pWnd = GetDlgItem(IDC_Grove_Per4);
	if (pWnd) {
		defaultRatio = AfxGetApp()->GetProfileInt("GroveOptions", "DefaultRatio4", 0);
		sprintf(buff, "%d", defaultRatio);
		pWnd->SetWindowText(buff);
	}

	pWnd = GetDlgItem(IDC_Grove_Per5);
	if (pWnd) {
		defaultRatio = AfxGetApp()->GetProfileInt("GroveOptions", "DefaultRatio5", 0);
		sprintf(buff, "%d", defaultRatio);
		pWnd->SetWindowText(buff);
	}
}

void GroveOptions::_setDefaultNumTrees(void)
{
	CWnd* pWnd = GetDlgItem(IDC_Grove_NumberTrees);
	if (!pWnd) {
		return;
	}

	int defaultNumTrees = AfxGetApp()->GetProfileInt("GroveOptions", "NumberofTrees", 10);
	static char buff[ARBITRARY_BUFF_SIZE];
	sprintf(buff, "%d", defaultNumTrees);

	pWnd->SetWindowText(buff);
}

void GroveOptions::_setDefaultPlacementAllowed(void)
{
	CButton* pButt;
	int state;

	pButt = (CButton*) GetDlgItem(IDC_Grove_AllowCliffPlacement);
	if (pButt) {
		state = AfxGetApp()->GetProfileInt("GroveOptions", "AllowCliffPlace", 1);
		pButt->SetCheck(state);
	}

	pButt = (CButton*) GetDlgItem(IDC_Grove_AllowWaterPlacement);
	if (pButt) {
		state = AfxGetApp()->GetProfileInt("GroveOptions", "AllowWaterPlace", 1);
		pButt->SetCheck(state);
	}
}


void GroveOptions::_updateTreeWeights(void)
{
	static char buff[ARBITRARY_BUFF_SIZE];
	int val = 0;
	int ratio;
	CWnd* pWnd = GetDlgItem(IDC_Grove_Per1);
	if (pWnd) {
		pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
		ratio = atoi(buff);
		AfxGetApp()->WriteProfileInt("GroveOptions", "DefaultRatio1", ratio);
		val += ratio;
	}

	pWnd = GetDlgItem(IDC_Grove_Per2);
	if (pWnd) {
		pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
		ratio = atoi(buff);
		AfxGetApp()->WriteProfileInt("GroveOptions", "DefaultRatio2", ratio);
		val += ratio;
	}

	pWnd = GetDlgItem(IDC_Grove_Per3);
	if (pWnd) {
		pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
		ratio = atoi(buff);
		AfxGetApp()->WriteProfileInt("GroveOptions", "DefaultRatio3", ratio);
		val += ratio;
	}

	pWnd = GetDlgItem(IDC_Grove_Per4);
	if (pWnd) {
		pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
		ratio = atoi(buff);
		AfxGetApp()->WriteProfileInt("GroveOptions", "DefaultRatio4", ratio);
		val += ratio;
	}

	pWnd = GetDlgItem(IDC_Grove_Per5);
	if (pWnd) {
		pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
		ratio = atoi(buff);
		AfxGetApp()->WriteProfileInt("GroveOptions", "DefaultRatio5", ratio);
		val += ratio;
	}

	pWnd = GetDlgItem(IDC_Grove_PerTotal);
	if (pWnd) {
		sprintf(buff, "%d", val);
		pWnd->SetWindowText(buff);
	}
}

void GroveOptions::_updateTreeCount(void)
{
	static char buff[ARBITRARY_BUFF_SIZE];
	CWnd* pWnd = GetDlgItem(IDC_Grove_NumberTrees);
	if (pWnd) {
		pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
		int val = atoi(buff);
		AfxGetApp()->WriteProfileInt("GroveOptions", "NumberofTrees", val);
	}
}

void GroveOptions::_updateGroveMakeup(void)
{
	for (int type = 1; type <= 5; ++type) {
		CComboBox *pComboBox;
		if (type == 1) {
			pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1);
		} else if (type == 2) {
			pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type2);
		} else if (type == 3) {
			pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type3);
		} else if (type == 4) {
			pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type4);
		} else if (type == 5) {
			pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type5);
		}

		if (!pComboBox) {
			continue;
		}

		int curSel = pComboBox->GetCurSel();
		if (type == 1) {
			AfxGetApp()->WriteProfileInt("GroveOptions", "TreeType1", curSel);
		} else if (type == 2) {
			AfxGetApp()->WriteProfileInt("GroveOptions", "TreeType2", curSel);
		} else if (type == 3) {
			AfxGetApp()->WriteProfileInt("GroveOptions", "TreeType3", curSel);
		} else if (type == 4) {
			AfxGetApp()->WriteProfileInt("GroveOptions", "TreeType4", curSel);
		} else if (type == 5) {
			AfxGetApp()->WriteProfileInt("GroveOptions", "TreeType5", curSel);
		}
	}
}

void GroveOptions::_updatePlacementAllowed(void)
{
	// huh huh huh-huh
	CButton* pButt;

	pButt = (CButton*) GetDlgItem(IDC_Grove_AllowCliffPlacement);
	if (pButt) {
		AfxGetApp()->WriteProfileInt("GroveOptions", "AllowCliffPlace", pButt->GetCheck());
	}

	pButt = (CButton*) GetDlgItem(IDC_Grove_AllowWaterPlacement);
	if (pButt) {
		AfxGetApp()->WriteProfileInt("GroveOptions", "AllowWaterPlace", pButt->GetCheck());
	}


}

void GroveOptions::OnOK()
{

}

void GroveOptions::OnClose()
{


}

UnicodeString GetDisplayNameFromPair(const PairNameDisplayName* pNamePair)
{
	if (!pNamePair) {
		return UnicodeString::TheEmptyString;
	}

	if (!pNamePair->second.isEmpty()) {
		return pNamePair->second;
	}

	// The unicode portion of the pair was empty. We need to use the Ascii version instead.
	UnicodeString retStr;
	retStr.translate(pNamePair->first);
	return retStr;
}

BEGIN_MESSAGE_MAP(GroveOptions, CDialog)
	ON_EN_KILLFOCUS(IDC_Grove_Per1, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per2, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per3, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per4, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per5, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_NumberTrees, _updateTreeCount)
	ON_CBN_SELENDOK(IDC_Grove_Type1, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type2, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type3, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type4, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type5, _updateGroveMakeup)
	ON_BN_CLICKED(IDC_Grove_AllowCliffPlacement, _updatePlacementAllowed)
	ON_BN_CLICKED(IDC_Grove_AllowWaterPlacement, _updatePlacementAllowed)
END_MESSAGE_MAP()
