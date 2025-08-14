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

#ifndef __TEAMGENERIC_H__
#define __TEAMGENERIC_H__

class Dict;

class TeamGeneric : public CPropertyPage
{
	public:
		TeamGeneric();   // standard constructor

	// Dialog Data
		//{{AFX_DATA(TeamGeneric)
		enum { IDD = IDD_TeamGeneric };
		//}}AFX_DATA

		void setTeamDict(Dict *dict) { m_teamDict = dict; }
	protected:
		void _fillComboBoxesWithScripts();
		void _dictToScripts();

	protected:
		Dict *m_teamDict;


	protected: // Windows Functions
		virtual BOOL OnInitDialog();
		afx_msg void _scriptsToDict();
		afx_msg void OnScriptAdjust();
		DECLARE_MESSAGE_MAP()
};

#endif
