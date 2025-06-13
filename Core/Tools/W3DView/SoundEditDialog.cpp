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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/SoundEditDialog.cpp            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 10/16/00 11:48a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.h"
#include "SoundEditDialog.h"
#include "soundrobj.h"
#include "AudibleSound.h"
#include "Utils.h"
#include "Sound3D.h"
#include "PlaySoundDialog.h"
#include "W3DViewDoc.h"
#include "DataTreeView.h"
#include "assetmgr.h"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// SoundEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
SoundEditDialogClass::SoundEditDialogClass (CWnd *parent)
	:	SoundRObj (NULL),
		CDialog (SoundEditDialogClass::IDD, parent)
{
	//{{AFX_DATA_INIT(SoundEditDialogClass)
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// ~SoundEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
SoundEditDialogClass::~SoundEditDialogClass (void)
{
	REF_PTR_RELEASE (SoundRObj);
	return;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SoundEditDialogClass)
	DDX_Control(pDX, IDC_VOLUME_SLIDER, VolumeSlider);
	DDX_Control(pDX, IDC_PRIORITY_SLIDER, PrioritySlider);
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(SoundEditDialogClass, CDialog)
	//{{AFX_MSG_MAP(SoundEditDialogClass)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_2D_RADIO, On2DRadio)
	ON_BN_CLICKED(IDC_3D_RADIO, On3DRadio)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//
// SoundEditDialogClass
//
/////////////////////////////////////////////////////////////////////////////
#ifdef RTS_DEBUG
void SoundEditDialogClass::AssertValid() const
{
	CDialog::AssertValid();
}

void SoundEditDialogClass::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif //RTS_DEBUG



/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
SoundEditDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	//
	//	Create the reneder object if we don't already have one
	//
	if (SoundRObj == NULL) {
		SoundRObj = new SoundRenderObjClass;
	}		

	//
	//	Choose default settings
	//
	OldName						= SoundRObj->Get_Name ();
	bool stop_on_hide			= SoundRObj->Get_Flag (SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN);
	StringClass filename;
	float drop_off_radius	= 100;
	float max_vol_radius		= 10;
	float priority				= 0.5F;
	bool is_3d					= true;
	bool is_music				= false;
	float loop_count			= 1;
	float volume				= 1.0F;	

	//
	// Get the real settings from the sound object (if we have one)
	//
	AudibleSoundClass *sound = SoundRObj->Peek_Sound ();
	if (sound != NULL) {

		Sound3DClass *sound_3d = sound->As_Sound3DClass ();
		filename				= sound->Get_Filename ();
		drop_off_radius	= sound->Get_DropOff_Radius ();		
		priority				= sound->Peek_Priority ();
		is_3d					= (sound_3d != NULL);
		is_music				= (sound->Get_Type () == AudibleSoundClass::TYPE_MUSIC);
		loop_count			= sound->Get_Loop_Count ();
		volume				= sound->Get_Volume ();		

		if (sound_3d != NULL) {
			max_vol_radius	= sound_3d->Get_Max_Vol_Radius ();
		}
	}

	//
	// Fill the edit controls
	//
	SetDlgItemText (IDC_NAME_EDIT,		OldName);
	SetDlgItemText (IDC_FILENAME_EDIT,	filename);


	//
	// Check the appropriate controls
	//
	SendDlgItemMessage (IDC_INFINITE_LOOPS_CHECK,	BM_SETCHECK, (WPARAM)(loop_count == 0));
	SendDlgItemMessage (IDC_3D_RADIO,					BM_SETCHECK, (WPARAM)is_3d);
	SendDlgItemMessage (IDC_2D_RADIO,					BM_SETCHECK, (WPARAM)(is_3d == false));
	SendDlgItemMessage (IDC_MUSIC_RADIO,				BM_SETCHECK, (WPARAM)is_music);
	SendDlgItemMessage (IDC_SOUNDEFFECT_RADIO,		BM_SETCHECK, (WPARAM)(is_music == false));
	SendDlgItemMessage (IDC_STOP_WHEN_HIDDEN_CHECK,	BM_SETCHECK, (WPARAM)(stop_on_hide));

	//
	// Set up the sliders
	//
	VolumeSlider.SetRange (0, 100);
	PrioritySlider.SetRange (0, 100);
	VolumeSlider.SetPos (int(volume * 100.00F));
	PrioritySlider.SetPos (int(priority * 100.00F));

	//
	// Put the attenuation factors into the edit fields
	//
	::SetDlgItemFloat (m_hWnd, IDC_DROP_OFF_EDIT, drop_off_radius);
	::SetDlgItemFloat (m_hWnd, IDC_MAX_VOL_EDIT, max_vol_radius);
	::SetDlgItemFloat (m_hWnd, IDC_TRIGGER_RADIUS_EDIT, drop_off_radius);	

	//
	//	Make sure the appropriate controls are enabled/disabled
	//
	Update_Enable_State ();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::OnOK (void)
{
	CString name;
	GetDlgItemText (IDC_NAME_EDIT, name);

	//
	//	Early exit if the name isn't valid
	//
	if (name.GetLength () == 0) {
		::MessageBox (m_hWnd, "Invalid object name.  Please enter a new name.", "Invalid settings", MB_ICONEXCLAMATION | MB_OK);
		return ;
	}

	//
	//	Create a sound definition from the settings on the dialog
	//
	AudibleSoundClass *sound = Create_Sound_Object ();
	AudibleSoundDefinitionClass definition;
	definition.Initialize_From_Sound (sound);
	REF_PTR_RELEASE (sound);
	
	//
	//	Pass the sound definition onto the render object
	//
	SoundRObj->Set_Sound (&definition);

	//
	//	Pass the flags onto the render object
	//
	if (SendDlgItemMessage (IDC_STOP_WHEN_HIDDEN_CHECK, BM_GETCHECK) == 1) {
		SoundRObj->Set_Flags (SoundRenderObjClass::FLAG_STOP_WHEN_HIDDEN);
	} else {
		SoundRObj->Set_Flags (0);
	}
	
	//
	//	Name the render object
	//
	SoundRObj->Set_Name (name);

	//
	//	Add this sound object to the viewer
	//
	CW3DViewDoc *doc = ::GetCurrentDocument ();
	if (doc != NULL) {
		
		//
		// Create a new prototype for this emitter and add it to the asset manager
		//
		SoundRenderObjDefClass *definition			= new SoundRenderObjDefClass (*SoundRObj);
		SoundRenderObjPrototypeClass *prototype	= new SoundRenderObjPrototypeClass (definition);
		
		//
		// Update the asset manager with the new prototype
		//
		if (OldName.Get_Length () > 0) {
			WW3DAssetManager::Get_Instance()->Remove_Prototype (OldName);
		}
		WW3DAssetManager::Get_Instance()->Add_Prototype (prototype);
		
		//
		// Add the new object to the data tree
		//
		CDataTreeView *data_tree = doc->GetDataTreeView ();
		data_tree->Refresh_Asset (name, OldName, TypeSound);

		//
		// Display the emitter
		//
		doc->Reload_Displayed_Object ();
		OldName = name;
	}
		
	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Create_Sound_Object
//
/////////////////////////////////////////////////////////////////////////////
AudibleSoundClass *
SoundEditDialogClass::Create_Sound_Object (void)
{
	AudibleSoundClass *sound = NULL;

	//
	// Get the filename
	//
	CString filename;
	GetDlgItemText (IDC_FILENAME_EDIT, filename);
	filename = ::Get_Filename_From_Path (filename);

	//
	//	Try to create the sound either as a 3D object or a 2D object
	//
	bool is_3d = bool (SendDlgItemMessage (IDC_3D_RADIO, BM_GETCHECK) == 1);
	if (is_3d) {
		sound = WWAudioClass::Get_Instance ()->Create_3D_Sound (filename);
	} else {
		sound = WWAudioClass::Get_Instance ()->Create_Sound_Effect (filename);
	}

	if (sound != NULL) {

		//
		// Pass the new volume and priority onto the sound
		//
		float priority = ((float)PrioritySlider.GetPos ()) / 100.0F;
		float volume = ((float)VolumeSlider.GetPos ()) / 100.0F;
		sound->Set_Priority (priority);
		sound->Set_Volume (volume);

		//
		// Pass the loop count and type onto the sound
		//
		int loop_count = SendDlgItemMessage (IDC_INFINITE_LOOPS_CHECK, BM_GETCHECK) ? 0 : 1;
		sound->Set_Loop_Count (loop_count);
		
		bool is_music = bool (SendDlgItemMessage (IDC_MUSIC_RADIO, BM_GETCHECK) == 1);			
		sound->Set_Type (is_music ? AudibleSoundClass::TYPE_MUSIC : AudibleSoundClass::TYPE_SOUND_EFFECT);

		//
		// Pass the new drop off and maximum volume radii to the sound
		//
		float drop_off	= 0;
		float max_vol	= 0;
		if (is_3d) {
			drop_off	= ::GetDlgItemFloat (m_hWnd, IDC_DROP_OFF_EDIT);
			max_vol	= ::GetDlgItemFloat (m_hWnd, IDC_MAX_VOL_EDIT);
		} else {
			drop_off	= ::GetDlgItemFloat (m_hWnd, IDC_TRIGGER_RADIUS_EDIT);
			max_vol	= ::GetDlgItemFloat (m_hWnd, IDC_TRIGGER_RADIUS_EDIT);
		}

		sound->Set_DropOff_Radius (drop_off);

		//
		//	Only 3D sounds have a maximum volume radius
		//
		if (is_3d) {
			((Sound3DClass *)sound)->Set_Max_Vol_Radius (max_vol);
		}
	}
	
	return sound;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnBrowse
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::OnBrowse (void)
{
	//
	// Determine what filename and path to initially display in the dialog
	//
	CString default_filename;
	CString path;
	if (GetDlgItemText (IDC_FILENAME_EDIT, default_filename) > 0) {
		path = ::Strip_Filename_From_Path (default_filename);
	}

	CFileDialog dialog (TRUE,
							  ".wav",
							  default_filename,
							  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
							  "All Sound Files|*.wav;*.mp3|WAV File (*.wav)|*.wav|MP3 File (*.mp3)|*.mp3||",
							  this);

	//
	// Set the path, so it opens in the correct directory
	//
	dialog.m_ofn.lpstrInitialDir = path;

	//
	// Ask the user what new sound file to use
	//
	if (dialog.DoModal () == IDOK) {

		//
		// Put the selected filename into the dialog control
		//
		SetDlgItemText (IDC_FILENAME_EDIT, ::Get_Filename_From_Path (dialog.GetPathName ()));
	}
	
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// On2DRadio
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::On2DRadio (void)
{
	Update_Enable_State ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// On3DRadio
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::On3DRadio (void)
{
	Update_Enable_State ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// Update_Enable_State
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::Update_Enable_State (void)
{
	bool enable_3d = (SendDlgItemMessage (IDC_3D_RADIO, BM_GETCHECK) == 1);
		
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_MAX_VOL_EDIT), enable_3d);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_DROP_OFF_EDIT), enable_3d);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_TRIGGER_RADIUS_EDIT), !enable_3d);
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnPlay
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::OnPlay (void)
{
	//
	//	Get the current filename
	//
	CString filename;
	GetDlgItemText (IDC_FILENAME_EDIT, filename);

	//
	//	Show the 'play sound' dialog
	//
	PlaySoundDialogClass dialog (filename, this);
	dialog.DoModal ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnCancel
//
/////////////////////////////////////////////////////////////////////////////
void
SoundEditDialogClass::OnCancel (void)
{
	CDialog::OnCancel ();
	return ;
}
