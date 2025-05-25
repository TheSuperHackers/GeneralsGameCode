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
#define TREES_PER_SET 11

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

	if (type < 1 || type > 11) {
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
	} else if (type == 6) {
		pWnd = GetDlgItem(IDC_Grove_Per6);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type6);
	} else if (type == 7) {
		pWnd = GetDlgItem(IDC_Grove_Per7);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type7);
	} else if (type == 8) {
		pWnd = GetDlgItem(IDC_Grove_Per8);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type8);
	} else if (type == 9) {
		pWnd = GetDlgItem(IDC_Grove_Per9);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type9);
	} else if (type == 10) {
		pWnd = GetDlgItem(IDC_Grove_Per10);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type10);
	} else if (type == 11) {
		pWnd = GetDlgItem(IDC_Grove_Per11);
		pBox = (CComboBox*) GetDlgItem(IDC_Grove_Type11);
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
	if (type < 1 || type > 11) {
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
	} else if (type == 6) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type6);
	} else if (type == 7) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type7);
	} else if (type == 8) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type8);
	} else if (type == 9) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type9);
	} else if (type == 10) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type10);
	} else if (type == 11) {
		pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type11);
	}

	int curSel = pComboBox->GetCurSel();
	if (curSel < 0 || (type == 11 && curSel >= mVecDisplayNames_PropsOnly.size()) || (type != 11 && curSel >= mVecDisplayNames.size())) {
		return "";
	}

	CString cstr;

	// Retrieve the correct list based on the type
	if (type == 11) {
		pComboBox->GetLBText(curSel, cstr);
		return cstr.GetBuffer(0); // Use mVecDisplayNames_PropsOnly
	} else {
		pComboBox->GetLBText(curSel, cstr);
		return cstr.GetBuffer(0); // Use mVecDisplayNames
	}
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
	_buildTreeListProps();
	_setTreesToLists();
	_setDefaultRatios();
	_updateTreeWeights();
	_setDefaultNumTrees();
	_setDefaultPlacementAllowed();
	return true;
}

void GroveOptions::OnMove(int x, int y)
{
  /**
   * Adriane [Deathscythe] -- Bug fix
   * This is required to save the top and left position values.
   * The handler is defined in COptionsPanel and must be called explicitly.
   */
	COptionsPanel::OnMove(x, y); // forward to base 
}

GroveOptions::~GroveOptions()
{
	TheGroveOptions = NULL;
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


void GroveOptions::_setTreesToLists()
{
	CString str;
	
	const int treeTypeComboIDs[TREES_PER_SET] = {
		IDC_Grove_Type1,
		IDC_Grove_Type2,
		IDC_Grove_Type3,
		IDC_Grove_Type4,
		IDC_Grove_Type5,
		IDC_Grove_Type6,
		IDC_Grove_Type7,
		IDC_Grove_Type8,
		IDC_Grove_Type9,
		IDC_Grove_Type10,
		IDC_Grove_Type11,
	};

	// Fill all 5 tree type combo boxes with model display names
	for (VecPairNameDisplayNameIt it = mVecDisplayNames.begin(); it != mVecDisplayNames.end(); ++it) {
		str = it->first.str();

		for (int i = 0; i < TREES_PER_SET; ++i) {
			CComboBox* pComboBox = (CComboBox*) GetDlgItem(treeTypeComboIDs[i]);
			if (pComboBox) {
				pComboBox->AddString(str);
			}
		}
	}

	// Add a blank entry at the end for each combo box
	str = "";
	for (int i = 0; i < TREES_PER_SET; ++i) {
		CComboBox* pComboBox = (CComboBox*) GetDlgItem(treeTypeComboIDs[i]);
		if (pComboBox) {
			pComboBox->AddString(str);
		}
	}

	// Fill IDC_Grove_Type11 with model display names from mVecDisplayNames_PropsOnly
	CComboBox* pComboBox11 = (CComboBox*) GetDlgItem(IDC_Grove_Type11);
	if (pComboBox11) {
		pComboBox11->ResetContent(); // Clear existing content
		for (VecPairNameDisplayNameIt it = mVecDisplayNames_PropsOnly.begin(); it != mVecDisplayNames_PropsOnly.end(); ++it) {
			str = it->first.str();
			pComboBox11->AddString(str);
		}
		pComboBox11->AddString(""); // Add a blank entry at the end for consistency
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

void GroveOptions::_loadSet(int setIndex)
{
	// Read all tree indices in one line
	CString key;
	key.Format("TreeTypeSet%d", setIndex);
	CString line = AfxGetApp()->GetProfileString("GroveOptions", key, "");

	int indices[TREES_PER_SET] = { 0 };
	int count = 0;

	int start = 0;
	while (count < TREES_PER_SET) {
		int comma = line.Find(',', start);
		CString token = (comma == -1) ? line.Mid(start) : line.Mid(start, comma - start);
		indices[count] = atoi(token);
		if (comma == -1) break;
		start = comma + 1;
		count++;
	}

	// Apply to tree type combo boxes
	for (int i = 0; i < TREES_PER_SET; ++i) {
		CComboBox* pComboBox = (CComboBox*) GetDlgItem(IDC_Grove_Type1 + i * 2);
		if (pComboBox) {
			int maxIndex = (i == 10) ? mVecDisplayNames_PropsOnly.size() : mVecDisplayNames.size();
			pComboBox->SetCurSel(indices[i] % (maxIndex + 1));
		}
	}
}

void GroveOptions::_updateGroveMakeup()
{
	// Save current Set Name selection
	CComboBox* pSetNameBox = (CComboBox*)GetDlgItem(IDC_Grove_SetName);
	int setIndex = 0;
	if (pSetNameBox) {
		setIndex = pSetNameBox->GetCurSel();
		AfxGetApp()->WriteProfileInt("GroveOptions", "SetNameIndex", setIndex);
	}

	const int treeTypeComboIDs[TREES_PER_SET] = {
		IDC_Grove_Type1,
		IDC_Grove_Type2,
		IDC_Grove_Type3,
		IDC_Grove_Type4,
		IDC_Grove_Type5,
		IDC_Grove_Type6,
		IDC_Grove_Type7,
		IDC_Grove_Type8,
		IDC_Grove_Type9,
		IDC_Grove_Type10,
		IDC_Grove_Type11
	};

	CString saveLine;
	for (int i = 0; i < TREES_PER_SET; ++i) {
		CComboBox* pComboBox = (CComboBox*) GetDlgItem(treeTypeComboIDs[i]);
		int curSel = (pComboBox ? pComboBox->GetCurSel() : 0);

		CString part;
		part.Format("%d,", curSel);
		saveLine += part;
	}


	saveLine.TrimRight(','); // Remove trailing comma

	CString key;
	key.Format("TreeTypeSet%d", setIndex);
	AfxGetApp()->WriteProfileString("GroveOptions", key, saveLine);

	DEBUG_LOG(("Saved TreeTypeSet%d = %s\n", setIndex, (LPCSTR)saveLine));
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

void GroveOptions::_buildTreeListProps(void)
{
	const ThingTemplate* pTemplate;
	for (pTemplate = TheThingFactory->firstTemplate(); pTemplate; pTemplate = pTemplate->friend_getNextTemplate()) {
		if (pTemplate->getEditorSorting() == ES_MISC_MAN_MADE || pTemplate->getEditorSorting() == ES_MISC_NATURAL) {
			PairNameDisplayName currentName;
			currentName.first = pTemplate->getName();
			currentName.second = pTemplate->getDisplayName();
			mVecDisplayNames_PropsOnly.push_back(currentName);
		}
	}
}


void GroveOptions::_setDefaultRatios(void)
{
    static char buff[ARBITRARY_BUFF_SIZE];
    int defaultRatio;

	static const int ids[11] = {
		IDC_Grove_Per1, IDC_Grove_Per2, IDC_Grove_Per3, IDC_Grove_Per4, IDC_Grove_Per5,
		IDC_Grove_Per6, IDC_Grove_Per7, IDC_Grove_Per8, IDC_Grove_Per9, IDC_Grove_Per10, IDC_Grove_Per11
	};

	for (int i = 0; i < 11; ++i)
	{
		CString key;
		key.Format("DefaultRatio%d", i + 1);

		CWnd* pWnd = GetDlgItem(ids[i]);
		if (pWnd)
		{
			defaultRatio = AfxGetApp()->GetProfileInt("GroveOptions", key, 0);
			sprintf(buff, "%d", defaultRatio);
			pWnd->SetWindowText(buff);
		}
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

    static const int ids[11] = {
        IDC_Grove_Per1, IDC_Grove_Per2, IDC_Grove_Per3, IDC_Grove_Per4, IDC_Grove_Per5,
        IDC_Grove_Per6, IDC_Grove_Per7, IDC_Grove_Per8, IDC_Grove_Per9, IDC_Grove_Per10, IDC_Grove_Per11
    };

    for (int i = 0; i < 11; ++i)
    {
        CWnd* pWnd = GetDlgItem(ids[i]);
        if (pWnd)
        {
            pWnd->GetWindowText(buff, ARBITRARY_BUFF_SIZE - 1);
            ratio = atoi(buff);
            CString key;
            key.Format("DefaultRatio%d", i + 1);
            AfxGetApp()->WriteProfileInt("GroveOptions", key, ratio);
            val += ratio;
        }
    }

    CWnd* pWndTotal = GetDlgItem(IDC_Grove_PerTotal);
    if (pWndTotal)
    {
        sprintf(buff, "%d", val);
        pWndTotal->SetWindowText(buff);
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
	ON_WM_MOVE()
	ON_EN_KILLFOCUS(IDC_Grove_Per1, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per2, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per3, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per4, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per5, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per6, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per7, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per8, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per9, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per10, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_Per11, _updateTreeWeights)
	ON_EN_KILLFOCUS(IDC_Grove_NumberTrees, _updateTreeCount)
	ON_CBN_SELENDOK(IDC_Grove_Type1, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type2, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type3, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type4, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type5, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type6, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type7, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type8, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type9, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type10, _updateGroveMakeup)
	ON_CBN_SELENDOK(IDC_Grove_Type11, _updateGroveMakeup)
	ON_BN_CLICKED(IDC_Grove_AllowCliffPlacement, _updatePlacementAllowed)
	ON_BN_CLICKED(IDC_Grove_AllowWaterPlacement, _updatePlacementAllowed)
	ON_CBN_SELCHANGE(IDC_Grove_SetName, OnSelchangeGroveSetName)
	ON_BN_CLICKED(IDC_Grove_SaveSet, OnSaveSetName)
END_MESSAGE_MAP()