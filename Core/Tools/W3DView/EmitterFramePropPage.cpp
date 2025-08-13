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

// EmitterFramePropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "EmitterFramePropPage.h"
#include "Utils.h"
#include "ParticleFrameKeyDialog.h"
#include "EmitterInstanceList.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterFramePropPageClass property page

IMPLEMENT_DYNCREATE(EmitterFramePropPageClass, CPropertyPage)

/////////////////////////////////////////////////////////////
//
//  EmitterFramePropPageClass - constructor
//
/////////////////////////////////////////////////////////////
EmitterFramePropPageClass::EmitterFramePropPageClass() :
	CPropertyPage(EmitterFramePropPageClass::IDD),
	m_pEmitterList(NULL),
	m_bValid(true),
	m_FrameBar(NULL),
	m_Lifetime(0),
	m_MinFrame(0),
	m_MaxFrame(1)
{
	::memset (&m_Frames, 0, sizeof (m_Frames));

	//{{AFX_DATA_INIT(EmitterFramePropPageClass)
	//}}AFX_DATA_INIT

	Initialize();
}

/////////////////////////////////////////////////////////////
//
//  ~EmitterFramePropPageClass - destructor
//
/////////////////////////////////////////////////////////////
EmitterFramePropPageClass::~EmitterFramePropPageClass()
{
	// Free the frame arrays
	SAFE_DELETE_ARRAY (m_Frames.KeyTimes);
	SAFE_DELETE_ARRAY (m_Frames.Values);
}

/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
/////////////////////////////////////////////////////////////
void EmitterFramePropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterFramePropPageClass)
	DDX_Control(pDX, IDC_FRAME_LAYOUT_COMBO, m_FrameLayoutCombo);
	DDX_Control(pDX, IDC_FRAME_RANDOM_SPIN, m_FrameRandomSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EmitterFramePropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterFramePropPageClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
EmitterFramePropPageClass::Initialize (void)
{
	SAFE_DELETE_ARRAY (m_Frames.KeyTimes);
	SAFE_DELETE_ARRAY (m_Frames.Values);

	if (m_pEmitterList != NULL) {
		m_Lifetime = m_pEmitterList->Get_Lifetime ();
		m_pEmitterList->Get_Frame_Keyframes (m_Frames);

		//
		//	Determine what the min and max rotations are
		//
		m_MaxFrame = WWMath::Max(m_Frames.Start,1.0f);
		m_MinFrame = WWMath::Min(m_Frames.Start,0.0f);

		for (UINT index = 0; index < m_Frames.NumKeyFrames; index ++) {
			if (m_Frames.Values[index] > m_MaxFrame) {
				m_MaxFrame = m_Frames.Values[index];
			}
			if (m_Frames.Values[index] < m_MinFrame) {
				m_MinFrame = m_Frames.Values[index];
			}
		}
	}

	return ;
}



/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
/////////////////////////////////////////////////////////////
BOOL
EmitterFramePropPageClass::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//
	// Set the frame layout combo box
	//
	int mode = m_pEmitterList->Get_Frame_Mode();
	switch (mode) {
		case W3D_EMITTER_FRAME_MODE_1x1: m_FrameLayoutCombo.SetCurSel(0);	break;
		case W3D_EMITTER_FRAME_MODE_2x2: m_FrameLayoutCombo.SetCurSel(1); break;
		case W3D_EMITTER_FRAME_MODE_4x4: m_FrameLayoutCombo.SetCurSel(2); break;
		case W3D_EMITTER_FRAME_MODE_8x8: m_FrameLayoutCombo.SetCurSel(3); break;
		case W3D_EMITTER_FRAME_MODE_16x16: m_FrameLayoutCombo.SetCurSel(4); break;
		default:
			m_FrameLayoutCombo.SetCurSel(4);
			break;
	}

	//
	// Create the keyframe control
	//
	m_FrameBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_FRAME_BAR));

	//
	// Setup the spinners
	//
	Initialize_Spinner (m_FrameRandomSpin, m_Frames.Rand, 0, 10000);

	//
	//	Reset the keyframe control
	//
	m_FrameBar->Set_Range (0, 1);
	m_FrameBar->Clear_Points ();
	m_FrameBar->Modify_Point (0, 0, 0, 0, 0);
	m_FrameBar->Set_Graph_Percent (0, Normalize_Frame(m_Frames.Start));

	//
	// Load the current set of frame keyframes into the control
	//
	for (UINT index = 0; index < m_Frames.NumKeyFrames; index ++) {
		m_FrameBar->Modify_Point (index + 1,
										m_Frames.KeyTimes[index] / m_Lifetime,
										0,
										0,
										0);
		m_FrameBar->Set_Graph_Percent (index + 1, Normalize_Frame(m_Frames.Values[index]));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL EmitterFramePropPageClass::OnApply()
{
	return CPropertyPage::OnApply();
}



/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL EmitterFramePropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
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
		case IDC_FRAME_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {

				//
				//	Allow the user to edit the keyframe
				//
				float frame = Denormalize_Frame(m_FrameBar->Get_Graph_Percent (color_bar_hdr->key_index));

				ParticleFrameKeyDialogClass dialog (frame, this);
				if (dialog.DoModal () == IDOK) {
					frame = dialog.Get_Frame ();
					float norm_val = Normalize_Frame(frame);

					m_FrameBar->Set_Redraw (false);
					m_FrameBar->Set_Graph_Percent (color_bar_hdr->key_index, norm_val);

					//
					//	Determine if the user changed the 'max' or 'min' frame
					//
					float new_max = WWMath::Max(frame,1.0f);
					float new_min = WWMath::Min(frame,0.0f);

					int count = m_FrameBar->Get_Point_Count ();
					for (int index = 0; index < count; index ++) {
						float tmp = Denormalize_Frame(m_FrameBar->Get_Graph_Percent (index) );
						if (tmp > new_max) {
							new_max = tmp;
						}
						if (tmp < new_min) {
							new_min = tmp;
						}
					}

					//
					//	Renormalize the FrameBar key frame points if necessary
					//
					if ((new_max != m_MaxFrame) || (new_min != m_MinFrame)) {

						int count = m_FrameBar->Get_Point_Count ();
						for (int index = 0; index < count; index ++) {

							float frame = Denormalize_Frame(m_FrameBar->Get_Graph_Percent (index));
							float new_norm = Normalize_Frame(frame,new_min,new_max);

							m_FrameBar->Set_Graph_Percent (index, new_norm);
						}

						// Remember the new min and max
						m_MinFrame = new_min;
						m_MaxFrame = new_max;
					}
					m_FrameBar->Set_Redraw (true);

					//
					// Update the emitter
					//
					Update_Frames ();
					m_pEmitterList->Set_Frame_Keyframes (m_Frames);
					SetModified ();
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT)) {

				//
				// Update the emitter
				//
				Update_Frames ();
				m_pEmitterList->Set_Frame_Keyframes (m_Frames);
				SetModified ();
			}
		}
		break;

		case IDC_FRAME_RANDOM_SPIN:
		{
			// Update the emitter
			m_Frames.Rand = ::GetDlgItemFloat (m_hWnd, IDC_FRAME_RANDOM_EDIT);
			m_pEmitterList->Set_Frame_Keyframes (m_Frames);
			SetModified ();
		}
		break;
	}
	return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

/////////////////////////////////////////////////////////////
//
//  Update_Rotations
//
/////////////////////////////////////////////////////////////
void
EmitterFramePropPageClass::Update_Frames (void)
{
	float position = 0;
	float red = 0;
	float green = 0;
	float blue = 0;

	//
	//	Setup the initial or 'starting' size
	//
	m_Frames.Start = Denormalize_Frame(m_FrameBar->Get_Graph_Percent (0));

	//
	// Free the current setting arrays
	//
	SAFE_DELETE_ARRAY (m_Frames.KeyTimes);
	SAFE_DELETE_ARRAY (m_Frames.Values);

	//
	//	Determine if we need to build the array of key frames or not
	//
	int count = m_FrameBar->Get_Point_Count ();
	m_Frames.NumKeyFrames = count - 1;
	if (count > 1) {
		m_Frames.KeyTimes = new float[count - 1];
		m_Frames.Values = new float[count - 1];

		//
		//	Get all the key frames and add them to our structure
		//
		for (int index = 1; index < count; index ++) {
			m_FrameBar->Get_Point (index, &position, &red, &green, &blue);
			m_Frames.KeyTimes[index - 1] = position * m_Lifetime;
			m_Frames.Values[index - 1] = Denormalize_Frame(m_FrameBar->Get_Graph_Percent (index) );
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
EmitterFramePropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD (wParam))
	{
		case IDC_FRAME_RANDOM_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY))
			{
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);
				m_Frames.Rand = ::GetDlgItemFloat (m_hWnd, IDC_FRAME_RANDOM_EDIT);
				m_pEmitterList->Set_Frame_Keyframes (m_Frames);
				SetModified ();
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}

		case IDC_FRAME_LAYOUT_COMBO:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				m_pEmitterList->Set_Frame_Mode(m_FrameLayoutCombo.GetCurSel());
				SetModified();
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
EmitterFramePropPageClass::On_Lifetime_Changed (float lifetime)
{
	if (m_Lifetime != lifetime) {
		float conversion = lifetime / m_Lifetime;

		//
		//	Rescale the sizes
		//
		for (UINT index = 0; index < m_Frames.NumKeyFrames; index ++) {
			m_Frames.KeyTimes[index] *= conversion;
		}

		//
		//	Update the emitter
		//
		m_pEmitterList->Set_Frame_Keyframes (m_Frames);
		m_Lifetime = lifetime;
	}

	return ;
}
