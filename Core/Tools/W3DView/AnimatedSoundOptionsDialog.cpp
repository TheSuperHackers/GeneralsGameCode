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

// AnimatedSoundOptionsDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "Globals.h"
#include "AnimatedSoundOptionsDialog.h"
#include "ffactory.h"
#include "animatedsoundmgr.h"
#include "wwsaveload.h"
#include "definitionmgr.h"
#include "WWFILE.H"
#include "chunkio.h"
#include "wwdebug.h"
#include "RestrictedFileDialog.h"
#include "Utils.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// AnimatedSoundOptionsDialogClass dialog


AnimatedSoundOptionsDialogClass::AnimatedSoundOptionsDialogClass(CWnd* pParent /*=NULL*/)
	: CDialog(AnimatedSoundOptionsDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(AnimatedSoundOptionsDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void AnimatedSoundOptionsDialogClass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AnimatedSoundOptionsDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(AnimatedSoundOptionsDialogClass, CDialog)
	//{{AFX_MSG_MAP(AnimatedSoundOptionsDialogClass)
	ON_BN_CLICKED(IDC_SOUND_DEFINITION_LIBRARY_BROWSE_BUTTON, OnSoundDefinitionLibraryBrowseButton)
	ON_BN_CLICKED(IDC_SOUND_INI_BROWSE_BUTTON, OnSoundIniBrowseButton)
	ON_BN_CLICKED(IDC_SOUND_PATH_BROWSE_BUTTON, OnSoundPathBrowseButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
//	OnSoundDefinitionLibraryBrowseButton
//
/////////////////////////////////////////////////////////////////////////////
void
AnimatedSoundOptionsDialogClass::OnSoundDefinitionLibraryBrowseButton (void)
{
	CFileDialog dialog (	TRUE,
								".ddb",
								"20480.ddb",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"Definition Database Files(*.ddb)|*.ddb||",
								this);

	//
	//	Prompt the user
	//
	if (dialog.DoModal () == IDOK) {
		SetDlgItemText (IDC_SOUND_DEFINITION_LIBRARY_EDIT, dialog.GetPathName ());
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnSoundIniBrowseButton
//
/////////////////////////////////////////////////////////////////////////////
void
AnimatedSoundOptionsDialogClass::OnSoundIniBrowseButton (void) 
{
	CFileDialog dialog (	TRUE,
								".ini",
								"w3danimsound.ini",
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								"INI Files (*.ini)|*.ini||",
								this);

	//
	//	Prompt the user
	//
	if (dialog.DoModal () == IDOK) {
		SetDlgItemText (IDC_SOUND_INI_EDIT, dialog.GetPathName ());
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
AnimatedSoundOptionsDialogClass::OnOK (void)
{
	CDialog::OnOK ();

	//
	//	Get the user's response
	//
	CString sound_def_lib_path;
	CString sound_ini_path;
	CString sound_data_path;
	GetDlgItemText (IDC_SOUND_DEFINITION_LIBRARY_EDIT, sound_def_lib_path);
	GetDlgItemText (IDC_SOUND_INI_EDIT, sound_ini_path);
	GetDlgItemText (IDC_SOUND_FILE_PATH_EDIT, sound_data_path);

	//
	//	Store this information in the registry
	//
	theApp.WriteProfileString ("Config", "SoundDefLibPath", sound_def_lib_path);
	theApp.WriteProfileString ("Config", "AnimSoundINIPath", sound_ini_path);
	theApp.WriteProfileString ("Config", "AnimSoundDataPath", sound_data_path);

	Load_Animated_Sound_Settings ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
AnimatedSoundOptionsDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	StringClass sound_def_lib_path	= static_cast<const TCHAR*>(theApp.GetProfileString ("Config", "SoundDefLibPath"));
	StringClass sound_ini_path			= static_cast<const TCHAR*>(theApp.GetProfileString ("Config", "AnimSoundINIPath"));
	StringClass sound_data_path		= static_cast<const TCHAR*>(theApp.GetProfileString ("Config", "AnimSoundDataPath"));

	//
	//	Fill in the default values
	//
	SetDlgItemText (IDC_SOUND_DEFINITION_LIBRARY_EDIT, sound_def_lib_path);
	SetDlgItemText (IDC_SOUND_INI_EDIT, sound_ini_path);
	SetDlgItemText (IDC_SOUND_FILE_PATH_EDIT, sound_data_path);	

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//	Load_Animated_Sound_Settings
//
/////////////////////////////////////////////////////////////////////////////
void
AnimatedSoundOptionsDialogClass::Load_Animated_Sound_Settings (void)
{
	//
	//	Start fresh
	//
	DefinitionMgrClass::Free_Definitions ();

	//
	//	Get the data from the registry
	//
	StringClass sound_def_lib_path	= static_cast<const TCHAR*>(theApp.GetProfileString ("Config", "SoundDefLibPath"));
	StringClass sound_ini_path			= static_cast<const TCHAR*>(theApp.GetProfileString ("Config", "AnimSoundINIPath"));
	StringClass sound_data_path		= static_cast<const TCHAR*>(theApp.GetProfileString ("Config", "AnimSoundDataPath"));

	//
	//	Try to load the definitions into the definition mgr
	//
	FileClass *file = _TheFileFactory->Get_File (sound_def_lib_path);
	if (file != NULL) {
		file->Open (FileClass::READ);
		ChunkLoadClass cload (file);
		SaveLoadSystemClass::Load (cload);
		file->Close ();
		_TheFileFactory->Return_File (file);
	} else {
		WWDEBUG_SAY (("Failed to load file %s\n", sound_def_lib_path.str ()));
	}

	//
	//	Load the sound settings from the ini file
	//
	AnimatedSoundMgrClass::Shutdown ();
	AnimatedSoundMgrClass::Initialize (sound_ini_path);

	//
	//	Add a sub-directory to the file factory for audio use
	//
	_TheSimpleFileFactory->Append_Sub_Directory (sound_data_path);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
//	OnSoundPathBrowseButton
//
/////////////////////////////////////////////////////////////////////////////
void
AnimatedSoundOptionsDialogClass::OnSoundPathBrowseButton (void) 
{
	RestrictedFileDialogClass dialog (	TRUE,
													".wav",
													"test.wav",
													OFN_HIDEREADONLY | OFN_EXPLORER,
													"Directories|*.wav||",
													AfxGetMainWnd ());

	dialog.m_ofn.lpstrTitle = "Pick Sound Path";

	//
	//	Prompt the user
	//
	if (dialog.DoModal () == IDOK) {
		
		CString path = ::Strip_Filename_From_Path (dialog.GetPathName ());
		SetDlgItemText (IDC_SOUND_FILE_PATH_EDIT, path);
	}

	return ;
}
