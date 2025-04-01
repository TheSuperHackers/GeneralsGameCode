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

// EmitterRotationPropPage.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "EmitterRotationPropPage.h"
#include "Utils.H"
#include "ParticleRotationKeyDialog.H"
#include "EmitterInstanceList.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterRotationPropPageClass property page

IMPLEMENT_DYNCREATE(EmitterRotationPropPageClass, CPropertyPage)


/////////////////////////////////////////////////////////////
//
//  EmitterRotationPropPageClass - constructor
//
/////////////////////////////////////////////////////////////
EmitterRotationPropPageClass::EmitterRotationPropPageClass() : 
	CPropertyPage(EmitterRotationPropPageClass::IDD),
	m_pEmitterList(NULL),
	m_bValid(true),
	m_RotationBar(NULL),
	m_Lifetime(0),
	m_MinRotation(0),
	m_MaxRotation(1),
	m_InitialOrientationRandom(0)

{
	::memset (&m_Rotations, 0, sizeof (m_Rotations));
	
	//{{AFX_DATA_INIT(EmitterRotationPropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Initialize();
}

/////////////////////////////////////////////////////////////
//
//  ~EmitterRotationPropPageClass - destructor
//
/////////////////////////////////////////////////////////////
EmitterRotationPropPageClass::~EmitterRotationPropPageClass()
{
	// Free the rotation arrays
	SAFE_DELETE_ARRAY (m_Rotations.KeyTimes);
	SAFE_DELETE_ARRAY (m_Rotations.Values);
}


/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
/////////////////////////////////////////////////////////////
void EmitterRotationPropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterRotationPropPageClass)
	DDX_Control(pDX, IDC_INITIAL_ORIENTATION_RANDOM_SPIN, m_InitialOrientationRandomSpin);
	DDX_Control(pDX, IDC_ROTATION_RANDOM_SPIN, m_RotationRandomSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EmitterRotationPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterRotationPropPageClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EmitterRotationPropPageClass message handlers


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void EmitterRotationPropPageClass::Initialize (void)
{
	SAFE_DELETE_ARRAY (m_Rotations.KeyTimes);
	SAFE_DELETE_ARRAY (m_Rotations.Values);	

	if (m_pEmitterList != NULL) {
		m_Lifetime = m_pEmitterList->Get_Lifetime ();
		m_pEmitterList->Get_Rotation_Keyframes (m_Rotations);
		m_InitialOrientationRandom = m_pEmitterList->Get_Initial_Orientation_Random();

		//
		//	Determine what the min and max rotations are
		//
		m_MaxRotation = WWMath::Max(m_Rotations.Start,1.0f);
		m_MinRotation = WWMath::Min(m_Rotations.Start,0.0f);

		for (UINT index = 0; index < m_Rotations.NumKeyFrames; index ++) {
			if (m_Rotations.Values[index] > m_MaxRotation) {
				m_MaxRotation = m_Rotations.Values[index];
			}
			if (m_Rotations.Values[index] < m_MinRotation) {
				m_MinRotation = m_Rotations.Values[index];
			}
		}
	}
}

/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
/////////////////////////////////////////////////////////////
BOOL 
EmitterRotationPropPageClass::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	//
	// Create the keyframe control
	//
	m_RotationBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_ROTATION_BAR));

	//
	// Setup the spinners
	//
	Initialize_Spinner (m_RotationRandomSpin, m_Rotations.Rand, 0, 10000);
	Initialize_Spinner (m_InitialOrientationRandomSpin, m_InitialOrientationRandom, 0, 10000);

	//
	//	Reset the color bars
	//
	m_RotationBar->Set_Range (0, 1);
	m_RotationBar->Clear_Points ();
	m_RotationBar->Modify_Point (0, 0, 0, 0, 0);
	m_RotationBar->Set_Graph_Percent (0, Normalize_Rotation(m_Rotations.Start));

	//
	// Load the current set of frame keyframes into the control
	//
	for (UINT index = 0; index < m_Rotations.NumKeyFrames; index ++) {
		m_RotationBar->Modify_Point (index + 1,
										m_Rotations.KeyTimes[index] / m_Lifetime,
										0,
										0,
										0);
		m_RotationBar->Set_Graph_Percent (index + 1, Normalize_Rotation(m_Rotations.Values[index]));
	}
	
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL EmitterRotationPropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	CBR_NMHDR *color_bar_hdr = (CBR_NMHDR *)lParam;

	//
	//	Update the spinner controls if necessary
	//
	NMHDR *pheader = (NMHDR *)lParam;
	if ((pheader != NULL) && (pheader->code == UDN_DELTAPOS)) {
		LPNMUPDOWN pupdown = (LPNMUPDOWN)lParam;		
		::Update_Spinner_Buddy (pheader->hwndFrom, pupdown->iDelta);
	}

	//
	//	Which control sent the notification?
	//
	switch (color_bar_hdr->hdr.idFrom)
	{
		case IDC_ROTATION_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {			
				
				//
				//	Allow the user to edit the keyframe
				//
				float rotation = Denormalize_Rotation(m_RotationBar->Get_Graph_Percent (color_bar_hdr->key_index));

				ParticleRotationKeyDialogClass dialog (rotation, this);
				if (dialog.DoModal () == IDOK) {
					rotation = dialog.Get_Rotation ();
					float norm_val = Normalize_Rotation(rotation);

					m_RotationBar->Set_Redraw (false);
					m_RotationBar->Set_Graph_Percent (color_bar_hdr->key_index, norm_val);
					
					//
					//	Determine if the user changed the 'max' or 'min' rotation
					//
					float new_max = WWMath::Max(rotation,1.0f);
					float new_min = WWMath::Min(rotation,0.0f);

					int count = m_RotationBar->Get_Point_Count ();
					for (int index = 0; index < count; index ++) {
						float tmp = Denormalize_Rotation(m_RotationBar->Get_Graph_Percent (index) );
						if (tmp > new_max) {
							new_max = tmp;
						}
						if (tmp < new_min) {
							new_min = tmp;
						}
					}

					//
					//	Renormalize the RotationBar key frame points if necessary
					//
					if ((new_max != m_MaxRotation) || (new_min != m_MinRotation)) {
						
						int count = m_RotationBar->Get_Point_Count ();
						for (int index = 0; index < count; index ++) {
							
							float rotation = Denormalize_Rotation(m_RotationBar->Get_Graph_Percent (index));
							float new_norm = Normalize_Rotation(rotation,new_min,new_max);

							m_RotationBar->Set_Graph_Percent (index, new_norm);
						}

						// Remember the new min and max
						m_MinRotation = new_min;
						m_MaxRotation = new_max;
					}
					m_RotationBar->Set_Redraw (true);

					//
					// Update the emitter
					//					
					Update_Rotations ();
					m_pEmitterList->Set_Rotation_Keyframes (m_Rotations, m_InitialOrientationRandom);
					SetModified ();
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT)) {			
				
				//
				// Update the emitter
				//
				Update_Rotations ();
				m_pEmitterList->Set_Rotation_Keyframes (m_Rotations, m_InitialOrientationRandom);
				SetModified ();					
			}
		}
		break;

		case IDC_ROTATION_RANDOM_SPIN:
		{
			// Update the emitter
			m_Rotations.Rand = ::GetDlgItemFloat (m_hWnd, IDC_ROTATION_RANDOM_EDIT);
			m_pEmitterList->Set_Rotation_Keyframes (m_Rotations, m_InitialOrientationRandom);
			SetModified ();
		}
		break;

		case IDC_INITIAL_ORIENTATION_RANDOM_SPIN:
		{
			// Update the emitter
			m_InitialOrientationRandom = ::GetDlgItemFloat (m_hWnd, IDC_INITIAL_ORIENTATION_RANDOM_EDIT);
			m_pEmitterList->Set_Rotation_Keyframes (m_Rotations, m_InitialOrientationRandom);
			SetModified ();
		}
		break;

	}
			
	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}

/////////////////////////////////////////////////////////////
//
//  Update_Rotations
//
/////////////////////////////////////////////////////////////
void
EmitterRotationPropPageClass::Update_Rotations (void)
{
	float position = 0;
	float red = 0;
	float green = 0;
	float blue = 0;

	//
	//	Setup the initial or 'starting' size
	//
	m_Rotations.Start = Denormalize_Rotation(m_RotationBar->Get_Graph_Percent (0));

	//
	// Free the current setting arrays
	//
	SAFE_DELETE_ARRAY (m_Rotations.KeyTimes);
	SAFE_DELETE_ARRAY (m_Rotations.Values);

	//
	//	Determine if we need to build the array of key frames or not
	//
	int count = m_RotationBar->Get_Point_Count ();
	m_Rotations.NumKeyFrames = count - 1;
	if (count > 1) {
		m_Rotations.KeyTimes = new float[count - 1];
		m_Rotations.Values = new float[count - 1];

		//
		//	Get all the rotation key frames and add them to our structure
		//	
		for (int index = 1; index < count; index ++) {
			m_RotationBar->Get_Point (index, &position, &red, &green, &blue);			
			m_Rotations.KeyTimes[index - 1] = position * m_Lifetime;
			m_Rotations.Values[index - 1] = Denormalize_Rotation(m_RotationBar->Get_Graph_Percent (index) );
		}
	}

	return ;
}

/////////////////////////////////////////////////////////////
//
//  OnCommand
//
/////////////////////////////////////////////////////////////
BOOL 
EmitterRotationPropPageClass::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (LOWORD (wParam))
	{
		case IDC_ROTATION_RANDOM_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				m_Rotations.Rand = ::GetDlgItemFloat (m_hWnd, IDC_ROTATION_RANDOM_EDIT);
				m_pEmitterList->Set_Rotation_Keyframes (m_Rotations, m_InitialOrientationRandom);
				SetModified ();
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;

		case IDC_INITIAL_ORIENTATION_RANDOM_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				m_InitialOrientationRandom = ::GetDlgItemFloat (m_hWnd, IDC_INITIAL_ORIENTATION_RANDOM_EDIT);
				m_pEmitterList->Set_Rotation_Keyframes (m_Rotations, m_InitialOrientationRandom);
				SetModified ();
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;

	}
	
	return CPropertyPage::OnCommand(wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  On_Lifetime_Changed
//
/////////////////////////////////////////////////////////////
void
EmitterRotationPropPageClass::On_Lifetime_Changed (float lifetime)
{
	if (m_Lifetime != lifetime) {
		float conversion = lifetime / m_Lifetime;

		//
		//	Rescale the sizes
		//
		for (UINT index = 0; index < m_Rotations.NumKeyFrames; index ++) {
			m_Rotations.KeyTimes[index] *= conversion;
		}

		//
		//	Update the emitter
		//
		m_pEmitterList->Set_Rotation_Keyframes (m_Rotations, m_InitialOrientationRandom);
		m_Lifetime = lifetime;
	}

	return ;
}
