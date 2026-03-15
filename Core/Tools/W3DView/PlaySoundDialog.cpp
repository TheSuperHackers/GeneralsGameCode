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

// PlaySoundDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "PlaySoundDialog.h"
#include "Utils.h"
#include "AudibleSound.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// PlaySoundDialogClass
//
/////////////////////////////////////////////////////////////////////////////
PlaySoundDialogClass::PlaySoundDialogClass(LPCTSTR filename, CWnd* pParent /*=nullptr*/)
	:	Filename (filename),
		SoundObj (nullptr),
		CDialog(PlaySoundDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(PlaySoundDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
PlaySoundDialogClass::DoDataExchange (CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PlaySoundDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(PlaySoundDialogClass, CDialog)
	//{{AFX_MSG_MAP(PlaySoundDialogClass)
	ON_BN_CLICKED(IDC_PLAY_SOUND_EFFECT, OnPlaySoundEffect)
	ON_BN_CLICKED(IDC_STOP_SOUND_EFFECT, OnStopSoundEffect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnPlaySoundEffect
//
/////////////////////////////////////////////////////////////////////////////
void
PlaySoundDialogClass::OnPlaySoundEffect (void)
{
	ASSERT (SoundObj != nullptr);
	if (SoundObj != nullptr) {
		SoundObj->Stop ();
		SoundObj->Play ();
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
/////////////////////////////////////////////////////////////////////////////
void
PlaySoundDialogClass::OnCancel (void)
{
	SoundObj->Stop ();
	REF_PTR_RELEASE (SoundObj);

	CDialog::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
PlaySoundDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Put the filename into the dialog
	//
	SetDlgItemText (IDC_FILENAME, Filename);

	//
	//	Create the sound effect so we can play it
	//
	SoundObj = WWAudioClass::Get_Instance ()->Create_Sound_Effect (Filename);
	if (SoundObj == nullptr) {
		CString message;
		message.Format ("Cannot find sound file: %s!", (LPCTSTR)Filename, MB_OK);
		MessageBox (message, "File Not Found", MB_ICONEXCLAMATION | MB_OK);
		EndDialog (IDCANCEL);
	} else {
		OnPlaySoundEffect ();
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnStopSoundEffect
//
/////////////////////////////////////////////////////////////////////////////
void
PlaySoundDialogClass::OnStopSoundEffect (void)
{
	ASSERT (SoundObj != nullptr);
	if (SoundObj != nullptr) {
		SoundObj->Stop ();
	}

	return ;
}

