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

// EmitterLineGroupPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "EmitterLineGroupPropPage.h"
#include "w3d_file.h"
#include "EmitterInstanceList.h"
#include "Utils.h"
#include "ColorBar.h"
#include "ParticleBlurTimeKeyDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterLineGroupPropPageClass property page

IMPLEMENT_DYNCREATE(EmitterLineGroupPropPageClass, CPropertyPage)

EmitterLineGroupPropPageClass::EmitterLineGroupPropPageClass() :
	CPropertyPage(EmitterLineGroupPropPageClass::IDD),
	m_pEmitterList(NULL),
	m_bValid(true),
	m_BlurTimeBar(NULL),
	m_Lifetime(0),
	m_MinBlurTime(0),
	m_MaxBlurTime(1)
{
	::memset (&m_BlurTimes, 0, sizeof (m_BlurTimes));

	//{{AFX_DATA_INIT(EmitterLineGroupPropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Initialize();
}

EmitterLineGroupPropPageClass::~EmitterLineGroupPropPageClass()
{
	// Free the blur time arrays
	SAFE_DELETE_ARRAY (m_BlurTimes.KeyTimes);
	SAFE_DELETE_ARRAY (m_BlurTimes.Values);
}

void EmitterLineGroupPropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterLineGroupPropPageClass)
	DDX_Control(pDX, IDC_BLUR_TIME_RANDOM_SPIN, m_BlurTimeRandomSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EmitterLineGroupPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterLineGroupPropPageClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
EmitterLineGroupPropPageClass::Initialize (void)
{
	SAFE_DELETE_ARRAY (m_BlurTimes.KeyTimes);
	SAFE_DELETE_ARRAY (m_BlurTimes.Values);	

	if (m_pEmitterList != NULL) {
		m_Lifetime = m_pEmitterList->Get_Lifetime ();
		m_pEmitterList->Get_Blur_Time_Keyframes (m_BlurTimes);

		//
		//	Determine what the min and max blur times are
		//
		m_MaxBlurTime = WWMath::Max(m_BlurTimes.Start,1.0f);
		m_MinBlurTime = WWMath::Min(m_BlurTimes.Start,0.0f);

		for (UINT index = 0; index < m_BlurTimes.NumKeyFrames; index ++) {
			if (m_BlurTimes.Values[index] > m_MaxBlurTime) {
				m_MaxBlurTime = m_BlurTimes.Values[index];
			}
			if (m_BlurTimes.Values[index] < m_MinBlurTime) {
				m_MinBlurTime = m_BlurTimes.Values[index];
			}
		}
	}

	return ;
}

/////////////////////////////////////////////////////////////////////////////
// EmitterLineGroupPropPageClass message handlers

BOOL EmitterLineGroupPropPageClass::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	//
	// Create the keyframe control
	//
	m_BlurTimeBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_BLUR_TIME_BAR));
	
	//
	// Setup the spinners
	//
	Initialize_Spinner (m_BlurTimeRandomSpin, m_BlurTimes.Rand, 0, 10000);

	//
	//	Reset the keyframe control
	//
	m_BlurTimeBar->Set_Range (0, 1);
	m_BlurTimeBar->Clear_Points ();
	m_BlurTimeBar->Modify_Point (0, 0, 0, 0, 0);
	m_BlurTimeBar->Set_Graph_Percent (0, Normalize_Blur_Time(m_BlurTimes.Start));

	//
	// Load the current set of frame keyframes into the control
	//
	for (UINT index = 0; index < m_BlurTimes.NumKeyFrames; index ++) {
		m_BlurTimeBar->Modify_Point (index + 1,
										m_BlurTimes.KeyTimes[index] / m_Lifetime,
										0,
										0,
										0);
		m_BlurTimeBar->Set_Graph_Percent (index + 1, Normalize_Blur_Time(m_BlurTimes.Values[index]));
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////
//
//  Update_Blur_Times
//
/////////////////////////////////////////////////////////////
void
EmitterLineGroupPropPageClass::Update_Blur_Times (void)
{
	float position = 0;
	float red = 0;
	float green = 0;
	float blue = 0;

	//
	//	Setup the initial or 'starting' size
	//
	m_BlurTimes.Start = Denormalize_Blur_Time(m_BlurTimeBar->Get_Graph_Percent (0));

	//
	// Free the current setting arrays
	//
	SAFE_DELETE_ARRAY (m_BlurTimes.KeyTimes);
	SAFE_DELETE_ARRAY (m_BlurTimes.Values);

	//
	//	Determine if we need to build the array of key frames or not
	//
	int count = m_BlurTimeBar->Get_Point_Count ();
	m_BlurTimes.NumKeyFrames = count - 1;
	if (count > 1) {
		m_BlurTimes.KeyTimes = new float[count - 1];
		m_BlurTimes.Values = new float[count - 1];

		//
		//	Get all the key frames and add them to our structure
		//	
		for (int index = 1; index < count; index ++) {
			m_BlurTimeBar->Get_Point (index, &position, &red, &green, &blue);			
			m_BlurTimes.KeyTimes[index - 1] = position * m_Lifetime;
			m_BlurTimes.Values[index - 1] = Denormalize_Blur_Time(m_BlurTimeBar->Get_Graph_Percent (index) );
		}
	}

	return ;
}

/////////////////////////////////////////////////////////////
//
//  On_Lifetime_Changed
//
/////////////////////////////////////////////////////////////
void
EmitterLineGroupPropPageClass::On_Lifetime_Changed (float lifetime)
{
	if (m_Lifetime != lifetime) {
		float conversion = lifetime / m_Lifetime;

		//
		//	Rescale the sizes
		//
		for (UINT index = 0; index < m_BlurTimes.NumKeyFrames; index ++) {
			m_BlurTimes.KeyTimes[index] *= conversion;
		}

		//
		//	Update the emitter
		//
		m_pEmitterList->Set_Blur_Time_Keyframes (m_BlurTimes);
		m_Lifetime = lifetime;
	}

	return ;
}

BOOL EmitterLineGroupPropPageClass::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (LOWORD (wParam))
	{
		case IDC_BLUR_TIME_RANDOM_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) 
			{
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);
				m_BlurTimes.Rand = ::GetDlgItemFloat (m_hWnd, IDC_BLUR_TIME_RANDOM_EDIT);
				m_pEmitterList->Set_Blur_Time_Keyframes (m_BlurTimes);
				SetModified ();
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}		
		break;
	}	
		
	return CPropertyPage::OnCommand(wParam, lParam);
}

BOOL EmitterLineGroupPropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
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
		case IDC_BLUR_TIME_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {			
				
				//
				//	Allow the user to edit the keyframe
				//
				float blur_time = Denormalize_Blur_Time(m_BlurTimeBar->Get_Graph_Percent (color_bar_hdr->key_index));

				ParticleBlurTimeKeyDialogClass dialog (blur_time, this);
				if (dialog.DoModal () == IDOK) {
					blur_time = dialog.Get_Blur_Time ();
					float norm_val = Normalize_Blur_Time(blur_time);

					m_BlurTimeBar->Set_Redraw (false);
					m_BlurTimeBar->Set_Graph_Percent (color_bar_hdr->key_index, norm_val);
					
					//
					//	Determine if the user changed the 'max' or 'min' frame
					//
					float new_max = WWMath::Max(blur_time,1.0f);
					float new_min = WWMath::Min(blur_time,0.0f);

					int count = m_BlurTimeBar->Get_Point_Count ();
					for (int index = 0; index < count; index ++) {
						float tmp = Denormalize_Blur_Time(m_BlurTimeBar->Get_Graph_Percent (index) );
						if (tmp > new_max) {
							new_max = tmp;
						}
						if (tmp < new_min) {
							new_min = tmp;
						}
					}

					//
					//	Renormalize the BlurTimeBar key frame points if necessary
					//
					if ((new_max != m_MaxBlurTime) || (new_min != m_MinBlurTime)) {
						
						int count = m_BlurTimeBar->Get_Point_Count ();
						for (int index = 0; index < count; index ++) {
							
							float frame = Denormalize_Blur_Time(m_BlurTimeBar->Get_Graph_Percent (index));
							float new_norm = Normalize_Blur_Time(frame,new_min,new_max);

							m_BlurTimeBar->Set_Graph_Percent (index, new_norm);
						}

						// Remember the new min and max
						m_MinBlurTime = new_min;
						m_MaxBlurTime = new_max;
					}
					m_BlurTimeBar->Set_Redraw (true);

					//
					// Update the emitter
					//					
					Update_Blur_Times ();
					m_pEmitterList->Set_Blur_Time_Keyframes (m_BlurTimes);
					SetModified ();
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT)) {			
				
				//
				// Update the emitter
				//
				Update_Blur_Times ();
				m_pEmitterList->Set_Blur_Time_Keyframes (m_BlurTimes);
				SetModified ();					
			}
		}
		break;

		case IDC_BLUR_TIME_RANDOM_SPIN:
		{
			// Update the emitter
			m_BlurTimes.Rand = ::GetDlgItemFloat (m_hWnd, IDC_BLUR_TIME_RANDOM_EDIT);
			m_pEmitterList->Set_Blur_Time_Keyframes (m_BlurTimes);
			SetModified ();
		}
		break;
	}
	
	return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

float EmitterLineGroupPropPageClass::Normalize_Blur_Time(float blur)
{
	return (blur - m_MinBlurTime) / (m_MaxBlurTime - m_MinBlurTime);
}

float EmitterLineGroupPropPageClass::Normalize_Blur_Time(float blur,float min,float max)
{
	return (blur - min) / (max - min);
}

float EmitterLineGroupPropPageClass::Denormalize_Blur_Time(float normalized_val)
{
	return normalized_val * (m_MaxBlurTime - m_MinBlurTime) + m_MinBlurTime;
}
