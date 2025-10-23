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
	TeamObjectProperties(Dict* dictToEdit = NULL);
	~TeamObjectProperties();

// Dialog Data
	//{{AFX_DATA(MapObjectProps)
	enum { IDD = IDD_TeamObjectProperties };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TeamObjectProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	Dict* m_dictToEdit;

#if 0 // Keys not implemented yet.  jba. [3/26/2003]//
	void updateTheUI(void);

	// Generated message map functions
	//{{AFX_MSG(TeamObjectProperties)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void _HealthToDict(void);
	afx_msg void _EnabledToDict(void);
	afx_msg void _IndestructibleToDict(void);
	afx_msg void _UnsellableToDict(void);
	afx_msg void _PoweredToDict(void);
	afx_msg void _AggressivenessToDict(void);
	afx_msg void _VisibilityToDict(void);
	afx_msg void _VeterancyToDict(void);
	afx_msg void _ShroudClearingDistanceToDict(void);
	afx_msg void _RecruitableAIToDict(void);
	afx_msg void _SelectableToDict(void);
	afx_msg void _WeatherToDict(void);
	afx_msg void _TimeToDict(void);
	afx_msg void _HPsToDict();
	afx_msg void _StoppingDistanceToDict(void);
	afx_msg void _UpdateTeamMembers(void);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void _DictToHealth(void);
	void _DictToHPs(void);
	void _DictToEnabled(void);
	void _DictToDestructible(void);
	void _DictToUnsellable(void);
	void _DictToPowered(void);
	void _DictToAggressiveness(void);
	void _DictToVisibilityRange(void);
	void _DictToVeterancy(void);
	void _DictToShroudClearingDistance(void);
	void _DictToRecruitableAI();
	void _DictToSelectable(void);
	void _DictToWeather(void);
	void _DictToTime(void);
	void _DictToStoppingDistance(void);
	void _PropertiesToDict(void);
#endif
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
