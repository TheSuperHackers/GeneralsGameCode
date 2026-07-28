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

// TeamObjectProperties.h
// Mike Lytle
// January, 2003
// (c) Electronic Arts 2003

#pragma once

#include "resource.h"

// Forward declarations.
class Dict;


// External Defines

class TeamObjectProperties : public CPropertyPage
{
// Construction
public:
	TeamObjectProperties(Dict* dictToEdit = nullptr);
	virtual ~TeamObjectProperties() override;

// Dialog Data
	//{{AFX_DATA(MapObjectProps)
	enum { IDD = IDD_TeamObjectProperties };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TeamObjectProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	//}}AFX_VIRTUAL

// Implementation
protected:
	Dict* m_dictToEdit;

#if 0 // Keys not implemented yet.  jba. [3/26/2003]//
	void updateTheUI();

	// Generated message map functions
	//{{AFX_MSG(TeamObjectProperties)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void _HealthToDict();
	afx_msg void _EnabledToDict();
	afx_msg void _IndestructibleToDict();
	afx_msg void _UnsellableToDict();
	afx_msg void _PoweredToDict();
	afx_msg void _AggressivenessToDict();
	afx_msg void _VisibilityToDict();
	afx_msg void _VeterancyToDict();
	afx_msg void _ShroudClearingDistanceToDict();
	afx_msg void _RecruitableAIToDict();
	afx_msg void _SelectableToDict();
	afx_msg void _WeatherToDict();
	afx_msg void _TimeToDict();
	afx_msg void _HPsToDict();
	afx_msg void _StoppingDistanceToDict();
	afx_msg void _UpdateTeamMembers();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void _DictToHealth();
	void _DictToHPs();
	void _DictToEnabled();
	void _DictToDestructible();
	void _DictToUnsellable();
	void _DictToPowered();
	void _DictToAggressiveness();
	void _DictToVisibilityRange();
	void _DictToVeterancy();
	void _DictToShroudClearingDistance();
	void _DictToRecruitableAI();
	void _DictToSelectable();
	void _DictToWeather();
	void _DictToTime();
	void _DictToStoppingDistance();
	void _PropertiesToDict();
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
