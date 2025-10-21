/*
**	Command & Conquer Renegade(tm)
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

#if !defined(AFX_ANIMATEDSOUNDOPTIONSDIALOG_H__5FD0B3D4_3975_4315_B412_44586622684B__INCLUDED_)
#define AFX_ANIMATEDSOUNDOPTIONSDIALOG_H__5FD0B3D4_3975_4315_B412_44586622684B__INCLUDED_

#pragma once
// AnimatedSoundOptionsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AnimatedSoundOptionsDialogClass dialog

class AnimatedSoundOptionsDialogClass : public CDialog
{
// Construction
public:
	AnimatedSoundOptionsDialogClass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AnimatedSoundOptionsDialogClass)
	enum { IDD = IDD_ANIMATED_SOUND_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AnimatedSoundOptionsDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AnimatedSoundOptionsDialogClass)
	afx_msg void OnSoundDefinitionLibraryBrowseButton();
	afx_msg void OnSoundIniBrowseButton();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSoundPathBrowseButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	static void			Load_Animated_Sound_Settings (void);

private:

	///////////////////////////////////////////////////////////////////
	//	Private methods
	///////////////////////////////////////////////////////////////////
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMATEDSOUNDOPTIONSDIALOG_H__5FD0B3D4_3975_4315_B412_44586622684B__INCLUDED_)
