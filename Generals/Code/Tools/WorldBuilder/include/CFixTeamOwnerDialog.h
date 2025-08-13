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

#include "Common/AsciiString.h"
class TeamsInfo;
class SidesList;

#pragma once

class CFixTeamOwnerDialog : public CDialog
{
	public:
		enum { IDD = IDD_CHANGE_TEAM_OWNER };
		CFixTeamOwnerDialog( TeamsInfo *ti, SidesList *sideList, UINT nIDTemplate = CFixTeamOwnerDialog::IDD,  CWnd* pParentWnd = NULL );
		AsciiString getSelectedOwner();
		Bool pickedValidTeam() { return m_pickedValidTeam; }

	protected:
		virtual BOOL OnInitDialog();
		afx_msg void OnOK();
		DECLARE_MESSAGE_MAP()

	protected:
		Bool m_pickedValidTeam;
		AsciiString m_selectedOwner;
		TeamsInfo *m_ti;
		SidesList *m_sl;
};
