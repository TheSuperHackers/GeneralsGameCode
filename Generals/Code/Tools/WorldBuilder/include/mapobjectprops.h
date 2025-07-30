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

#if !defined(AFX_MAPOBJECTPROPS_H__44517B9E_12AB_4E2C_B49B_D6BB65C59649__INCLUDED_)
#define AFX_MAPOBJECTPROPS_H__44517B9E_12AB_4E2C_B49B_D6BB65C59649__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// mapobjectprops.h : header file
//

#include "OptionsPanel.h"
#include "Common/Dict.h"
#include "WBPopupSlider.h"

class MapObject;
class ModifyObjectUndoable;

/////////////////////////////////////////////////////////////////////////////
// External Defines
extern const char* NEUTRAL_TEAM_UI_STR;
extern const char* NEUTRAL_TEAM_INTERNAL_STR;


/////////////////////////////////////////////////////////////////////////////
// MapObjectProps dialog

class MapObjectProps : public COptionsPanel, public PopupSliderOwner
{
// Construction
public:
	MapObjectProps(Dict* dictToEdit = NULL, const char* title = NULL, CWnd* pParent = NULL);   // standard constructor
	~MapObjectProps();
	void makeMain();

// Dialog Data
	//{{AFX_DATA(MapObjectProps)
	enum { IDD = IDD_MAPOBJECT_PROPS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MapObjectProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void getAllSelectedDicts(void);
	Dict** getAllSelectedDictsData();

	static MapObjectProps *TheMapObjectProps; 

	Dict* m_dictToEdit;
	std::vector<Dict*> m_allSelectedDicts;
	const char* m_title;
	MapObject *m_selectedObject;

	void updateTheUI(void);
	void enableButtons();
	int getSel();
	WBPopupSliderButton m_heightSlider;
	WBPopupSliderButton m_angleSlider;
	Real m_angle;
	Real m_height;
	ModifyObjectUndoable *m_posUndoable;

	// Generated message map functions
	//{{AFX_MSG(MapObjectProps)
	afx_msg void OnSelchangeProperties();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditprop();
	afx_msg void OnNewprop();
	afx_msg void OnRemoveprop();
	afx_msg void OnDblclkProperties();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void _TeamToDict(void);
	afx_msg void _NameToDict(void);
	afx_msg void _HealthToDict(void);
	afx_msg void _EnabledToDict(void);
	afx_msg void _ScriptToDict(void);
	afx_msg void _IndestructibleToDict(void);
	afx_msg void _UnsellableToDict(void);
	afx_msg void _TargetableToDict();
	afx_msg void _PoweredToDict(void);
	afx_msg void _AggressivenessToDict(void);
	afx_msg void _VisibilityToDict(void);
	afx_msg void _VeterancyToDict(void);
	afx_msg void _ShroudClearingDistanceToDict(void);
	afx_msg void _RecruitableAIToDict(void);
	afx_msg void _SelectableToDict(void);
	afx_msg void _WeatherToDict(void);
	afx_msg void _TimeToDict(void);
	afx_msg void _PrebuiltUpgradesToDict(void);
	afx_msg void SetZOffset(void);
	afx_msg void SetAngle(void);
	afx_msg void _HPsToDict();
	afx_msg void _StoppingDistanceToDict(void);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void _DictToTeam(void);
	void _DictToName(void);
	void _DictToHealth(void);
	void _DictToHPs(void);
	void _DictToEnabled(void);
	void _DictToScript(void);
	void _DictToDestructible(void);
	void _DictToUnsellable(void);
	void _DictToTargetable(void);
	
	void _DictToPowered(void);
	void _DictToAggressiveness(void);
	void _DictToVisibilityRange(void);
	void _DictToVeterancy(void);
	void _DictToShroudClearingDistance(void);
	void _DictToRecruitableAI();
	void _DictToSelectable(void);
	void _DictToWeather(void);
	void _DictToTime(void);
	void ShowZOffset(MapObject* pMapObj);
	void ShowAngle(MapObject* pMapObj);
	void _DictToStoppingDistance(void);
	void _DictToPrebuiltUpgrades(void);

public:
	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial);
	virtual void PopSliderChanged(const long sliderID, long theVal);
	virtual void PopSliderFinished(const long sliderID, long theVal);

	static MapObject *getSingleSelectedMapObject(void);
	static void update(void);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPOBJECTPROPS_H__44517B9E_12AB_4E2C_B49B_D6BB65C59649__INCLUDED_)
