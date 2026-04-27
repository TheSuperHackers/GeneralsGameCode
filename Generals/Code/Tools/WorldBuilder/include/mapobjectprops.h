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

#pragma once

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
	MapObjectProps(Dict* dictToEdit = nullptr, const char* title = nullptr, CWnd* pParent = nullptr);   // standard constructor
	virtual ~MapObjectProps() override;
	void makeMain();

// Dialog Data
	//{{AFX_DATA(MapObjectProps)
	enum { IDD = IDD_MAPOBJECT_PROPS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MapObjectProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void getAllSelectedDicts();
	Dict** getAllSelectedDictsData();

	static MapObjectProps *TheMapObjectProps;

	Dict* m_dictToEdit;
	std::vector<Dict*> m_allSelectedDicts;
	const char* m_title;
	MapObject *m_selectedObject;

	void updateTheUI();
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
	virtual BOOL OnInitDialog() override;
	afx_msg void OnEditprop();
	afx_msg void OnNewprop();
	afx_msg void OnRemoveprop();
	afx_msg void OnDblclkProperties();
	virtual void OnOK() override;
	virtual void OnCancel() override;
	afx_msg void _TeamToDict();
	afx_msg void _NameToDict();
	afx_msg void _HealthToDict();
	afx_msg void _EnabledToDict();
	afx_msg void _ScriptToDict();
	afx_msg void _IndestructibleToDict();
	afx_msg void _UnsellableToDict();
	afx_msg void _TargetableToDict();
	afx_msg void _PoweredToDict();
	afx_msg void _AggressivenessToDict();
	afx_msg void _VisibilityToDict();
	afx_msg void _VeterancyToDict();
	afx_msg void _ShroudClearingDistanceToDict();
	afx_msg void _RecruitableAIToDict();
	afx_msg void _SelectableToDict();
	afx_msg void _WeatherToDict();
	afx_msg void _TimeToDict();
	afx_msg void _PrebuiltUpgradesToDict();
	afx_msg void SetZOffset();
	afx_msg void SetAngle();
	afx_msg void _HPsToDict();
	afx_msg void _StoppingDistanceToDict();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void _DictToTeam();
	void _DictToName();
	void _DictToHealth();
	void _DictToHPs();
	void _DictToEnabled();
	void _DictToScript();
	void _DictToDestructible();
	void _DictToUnsellable();
	void _DictToTargetable();

	void _DictToPowered();
	void _DictToAggressiveness();
	void _DictToVisibilityRange();
	void _DictToVeterancy();
	void _DictToShroudClearingDistance();
	void _DictToRecruitableAI();
	void _DictToSelectable();
	void _DictToWeather();
	void _DictToTime();
	void ShowZOffset(MapObject* pMapObj);
	void ShowAngle(MapObject* pMapObj);
	void _DictToStoppingDistance();
	void _DictToPrebuiltUpgrades();

public:
	virtual void GetPopSliderInfo(const long sliderID, long *pMin, long *pMax, long *pLineSize, long *pInitial) override;
	virtual void PopSliderChanged(const long sliderID, long theVal) override;
	virtual void PopSliderFinished(const long sliderID, long theVal) override;

	static MapObject *getSingleSelectedMapObject();
	static void update();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
