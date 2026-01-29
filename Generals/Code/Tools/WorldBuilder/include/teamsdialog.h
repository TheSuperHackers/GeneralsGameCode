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

// teamsdialog.h : header file
//

#include "GameLogic/SidesList.h"

/////////////////////////////////////////////////////////////////////////////
// CTeamsDialog dialog

class CTeamsDialog : public CDialog
{
// Construction
public:
	CTeamsDialog(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTeamsDialog)
	enum { IDD = IDD_TEAMS_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTeamsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTeamsDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnNewteam();
	afx_msg void OnDeleteteam();
	afx_msg void OnEditTemplate();
	afx_msg void OnSelchangePlayerList();
	afx_msg void OnClickTeamsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTeamsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCopyteam();
	afx_msg void OnSelectTeamMembers();
	afx_msg void OnMoveDownTeam();
	afx_msg void OnMoveUpTeam();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	Int								m_updating;
	SidesList						m_sides;
	Int								m_curTeam;

	enum
	{
		REBUILD_TEAMS = 0x01,
		REBUILD_OWNERS = 0x02,
		REBUILD_ALLIES = 0x04,

		REBUILD_NONE = 0x00,
		REBUILD_ALL = 0xff
	};

	void updateUI(Int whatToRebuild);
	void validateTeamOwners( void );
	Bool isValidTeamOwner( AsciiString ownerName );
	void doCorrectTeamOwnerDialog( TeamsInfo *ti );

private:
	void UpdateTeamsList(void);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
