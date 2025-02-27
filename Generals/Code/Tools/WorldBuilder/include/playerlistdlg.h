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

// playerlistdlg.h : header file
//

#include "GameLogic/SidesList.h"
#include "CButtonShowColor.h"

/////////////////////////////////////////////////////////////////////////////
// PlayerListDlg dialog

class PlayerListDlg : public CDialog
{
// Construction
public:
	PlayerListDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PlayerListDlg)
	enum { IDD = IDD_PLAYERLIST };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PlayerListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	Int					m_updating;
	SidesList			m_sides;
	Int					m_curPlayerIdx;
	CButtonShowColor	m_colorButton;

	void updateTheUI(void);
	void PopulateColorComboBox(void);
	void SelectColor(RGBColor rgb);

	// Generated message map functions
	//{{AFX_MSG(PlayerListDlg)
	afx_msg void OnNewplayer();
	afx_msg void OnEditplayer();
	afx_msg void OnRemoveplayer();
	afx_msg void OnSelchangePlayers();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkPlayers();
	afx_msg void OnSelchangeAllieslist();
	afx_msg void OnSelchangeEnemieslist();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPlayeriscomputer();
	afx_msg void OnEditchangePlayerfaction();
	afx_msg void OnChangePlayername();
	afx_msg void OnChangePlayerdisplayname();
	afx_msg void OnColorPress();
	afx_msg void OnSelectPlayerColor();
	afx_msg void OnAddskirmishplayers();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
