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

#define MAX_SETS 10
#define TREES_PER_SET 5

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


void GroveOptions::_setTreesToLists()
{
	CString str;

	// Fill all 5 tree type combo boxes with model display names
	for (VecPairNameDisplayNameIt it = mVecDisplayNames.begin(); it != mVecDisplayNames.end(); ++it) {
		str = it->first.str();

		for (int i = 0; i < TREES_PER_SET; ++i) {
			CComboBox* pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1 + i * 2);
			if (pComboBox) {
				pComboBox->AddString(str);
			}
		}
	}

	// Add a blank entry at the end
	str = "";
	for (int i = 0; i < TREES_PER_SET; ++i) {
		CComboBox* pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1 + i * 2);
		if (pComboBox) {
			pComboBox->AddString(str);
		}
	}

	// Fill Set Name combo box
	CComboBox* pSetNameBox = (CComboBox*) GetDlgItem(IDC_Grove_SetName);
	if (pSetNameBox) {
		pSetNameBox->ResetContent();

		CString setNameKey, setName;
		for (int i = 0; i < MAX_SETS; ++i) {
			setNameKey.Format("SetName%d", i);
			setName = AfxGetApp()->GetProfileString("GroveOptions", setNameKey, CString("Set ") + (char)('A' + i));
			pSetNameBox->AddString(setName);
		}

		int selIndex = AfxGetApp()->GetProfileInt("GroveOptions", "SetNameIndex", 0);
		pSetNameBox->SetCurSel(selIndex);
		_loadSet(selIndex);
	}
}


/**
 * Adriane [Deatscythe]
 * I got bored talking with chatGPT -- i had to abandon this feature for now
 */
void GroveOptions::OnSaveSetName()
{
// 	DEBUG_LOG(("test\n"));
// 	CComboBox* pSetNameBox = (CComboBox*) GetDlgItem(IDC_Grove_SetName);
// 	if (!pSetNameBox) return;

// 	int selIndex = pSetNameBox->GetCurSel();
// 	if (selIndex == CB_ERR) return;

// 	CString selText;
// 	pSetNameBox->GetWindowText(selText); 
// 	DEBUG_LOG(("testX %s\n", selText));

// 	CString setNameKey;
// 	setNameKey.Format("SetName%d", selIndex);
// 	AfxGetApp()->WriteProfileString("GroveOptions", setNameKey, selText);
}



// void GroveOptions::OnOK() 
// {
// 	CComboBox* pSetNameBox = (CComboBox*) GetDlgItem(IDC_Grove_SetName);
// 	if (pSetNameBox) {
// 		int selIndex = pSetNameBox->GetCurSel();
// 		if (selIndex != CB_ERR) {
// 			CString selText;
// 			pSetNameBox->GetWindowText(selText);
// 			AfxGetApp()->WriteProfileString("GroveOptions", "SetNameText", selText);
	
// 			CString setNameKey;
// 			setNameKey.Format("SetName%d", selIndex);
// 			AfxGetApp()->WriteProfileString("GroveOptions", setNameKey, selText);
// 		}
// 	}
// }

void GroveOptions::OnSelchangeGroveSetName()
{
	CComboBox* pSetNameBox = (CComboBox*) GetDlgItem(IDC_Grove_SetName);
	if (!pSetNameBox) return;

	int selIndex = pSetNameBox->GetCurSel();
	if (selIndex == CB_ERR) return;

	// CString selText;
	// pSetNameBox->GetWindowText(selText); // <- use GetWindowText instead of GetLBText

	// Save to profile
	AfxGetApp()->WriteProfileInt("GroveOptions", "SetNameIndex", selIndex);
	// AfxGetApp()->WriteProfileString("GroveOptions", "SetNameText", selText);

	// CString setNameKey;
	// setNameKey.Format("SetName%d", selIndex);
	// AfxGetApp()->WriteProfileString("GroveOptions", setNameKey, selText);

	_loadSet(selIndex);
}


void GroveOptions::_loadSet(int setIndex)
{
	for (int i = 0; i < TREES_PER_SET; ++i) {
		CString key;
		key.Format("TreeTypeSet%d_%d", setIndex, i + 1);
		int treeIndex = AfxGetApp()->GetProfileInt("GroveOptions", key, 0);

		CComboBox* pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1 + i * 2);
		if (pComboBox) {
			pComboBox->SetCurSel(treeIndex % (mVecDisplayNames.size() + 1));
		}
	}
}

void GroveOptions::_updateGroveMakeup(void)
{
	// Save current Set Name selection
	CComboBox* pSetNameBox = (CComboBox*)GetDlgItem(IDC_Grove_SetName);
	int setIndex = 0;
	if (pSetNameBox) {
		setIndex = pSetNameBox->GetCurSel();
		AfxGetApp()->WriteProfileInt("GroveOptions", "SetNameIndex", setIndex);
	}

	// Save current tree type selections for this set
	for (int i = 0; i < TREES_PER_SET; ++i) {
		CComboBox* pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1 + i * 2);
		if (pComboBox) {
			int curSel = pComboBox->GetCurSel();
			CString key;
			key.Format("TreeTypeSet%d_%d", setIndex, i + 1);
			AfxGetApp()->WriteProfileInt("GroveOptions", key, curSel);
		}
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
	ON_CBN_SELCHANGE(IDC_Grove_SetName, OnSelchangeGroveSetName)
	ON_BN_CLICKED(IDC_Grove_SaveSet, OnSaveSetName)
END_MESSAGE_MAP()