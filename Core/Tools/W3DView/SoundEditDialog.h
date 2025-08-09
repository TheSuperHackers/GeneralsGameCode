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

#if !defined(AFX_SOUNDEDITDIALOG_H__376FEA4C_0A53_44FB_A6B5_DF0E96CF1DEF__INCLUDED_)
#define AFX_SOUNDEDITDIALOG_H__376FEA4C_0A53_44FB_A6B5_DF0E96CF1DEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "wwstring.h"
#include "resource.h"
#include "soundrobj.h"

/////////////////////////////////////////////////////////////////////
//	Forward declarations
/////////////////////////////////////////////////////////////////////
class AudibleSoundClass;


/////////////////////////////////////////////////////////////////////
//
//	SoundEditDialogClass
//
/////////////////////////////////////////////////////////////////////
class SoundEditDialogClass : public CDialog
{
public:
	SoundEditDialogClass (CWnd *parent);
	virtual ~SoundEditDialogClass (void);

// Form Data
public:
	//{{AFX_DATA(SoundEditDialogClass)
	enum { IDD = IDD_SOUND_EDIT };
	CSliderCtrl	VolumeSlider;
	CSliderCtrl	PrioritySlider;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SoundEditDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(SoundEditDialogClass)
	afx_msg void OnBrowse();
	afx_msg void On2DRadio();
	afx_msg void On3DRadio();
	afx_msg void OnPlay();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	///////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////
	void							Set_Sound (SoundRenderObjClass *sound)	{ REF_PTR_SET (SoundRObj, sound); }
	SoundRenderObjClass *	Get_Sound (void) const						{ if (SoundRObj != NULL) SoundRObj->Add_Ref (); return SoundRObj; }

protected:

	///////////////////////////////////////////////////////
	//	Protected methods
	///////////////////////////////////////////////////////
	void							Update_Enable_State (void);
	AudibleSoundClass *		Create_Sound_Object (void);

private:

	///////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////
	SoundRenderObjClass *	SoundRObj;
	StringClass					OldName;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDEDITDIALOG_H__376FEA4C_0A53_44FB_A6B5_DF0E96CF1DEF__INCLUDED_)
