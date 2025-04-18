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

// EmitterSizePropPage.cpp : implementation file
//

#include "Stdafx.H"
#include "W3dview.H"
#include "EmitterSizePropPage.H"
#include "Utils.H"
#include "ParticleSizeDialog.H"
#include "EmitterInstanceList.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterSizePropPageClass property page

IMPLEMENT_DYNCREATE(EmitterSizePropPageClass, CPropertyPage)


/////////////////////////////////////////////////////////////
//
//  EmitterSizePropPageClass
//
/////////////////////////////////////////////////////////////
EmitterSizePropPageClass::EmitterSizePropPageClass (EmitterInstanceListClass *pemitter)
	:	m_pEmitterList (NULL),
		m_bValid (true),
		m_SizeBar (NULL),
		m_Lifetime (0),
		CPropertyPage(EmitterSizePropPageClass::IDD)
{
	::memset (&m_OrigSizes, 0, sizeof (m_OrigSizes));
	::memset (&m_CurrentSizes, 0, sizeof (m_CurrentSizes));

	//{{AFX_DATA_INIT(EmitterSizePropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  ~EmitterSizePropPageClass
//
/////////////////////////////////////////////////////////////
EmitterSizePropPageClass::~EmitterSizePropPageClass (void)
{
	// Free the original setting arrays
	SAFE_DELETE_ARRAY (m_OrigSizes.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigSizes.Values);
	
	// Free the current setting arrays
	SAFE_DELETE_ARRAY (m_CurrentSizes.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentSizes.Values);
	return ;
}


/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
/////////////////////////////////////////////////////////////
void
EmitterSizePropPageClass::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EmitterSizePropPageClass)
	DDX_Control(pDX, IDC_SIZE_RANDOM_SPIN, m_SizeRandomSpin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EmitterSizePropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(EmitterSizePropPageClass)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
EmitterSizePropPageClass::Initialize (void)
{
	SAFE_DELETE_ARRAY (m_OrigSizes.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigSizes.Values);	
	SAFE_DELETE_ARRAY (m_CurrentSizes.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentSizes.Values);

	if (m_pEmitterList != NULL) {
		m_Lifetime = m_pEmitterList->Get_Lifetime ();
		m_pEmitterList->Get_Size_Keyframes (m_OrigSizes);
		m_pEmitterList->Get_Size_Keyframes (m_CurrentSizes);

		//
		//	Determine what the largest size is
		//
		m_MaxSize = m_OrigSizes.Start;
		for (UINT index = 0; index < m_OrigSizes.NumKeyFrames; index ++) {
			if (m_OrigSizes.Values[index] > m_MaxSize) {
				m_MaxSize = m_OrigSizes.Values[index];
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
EmitterSizePropPageClass::OnInitDialog (void) 
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();
	
	m_SizeBar = ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_SIZE_BAR));
	m_SizeBar->Set_Range (0, 1);

	//
	// Setup the spinners
	//
	Initialize_Spinner (m_SizeRandomSpin, m_OrigSizes.Rand, 0, 10000);

	//
	//	Reset the color bars
	//
	m_SizeBar->Set_Range (0, 1);
	m_SizeBar->Clear_Points ();
	m_SizeBar->Modify_Point (0, 0, 0, 0, 0);
	m_SizeBar->Set_Graph_Percent (0, m_OrigSizes.Start / m_MaxSize);

	//
	//	Set-up the color bar
	//
	for (UINT index = 0; index < m_OrigSizes.NumKeyFrames; index ++) {
		m_SizeBar->Modify_Point (index + 1,
										m_OrigSizes.KeyTimes[index] / m_Lifetime,
										0,
										0,
										0);
		m_SizeBar->Set_Graph_Percent (index + 1, m_OrigSizes.Values[index] / m_MaxSize);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL
EmitterSizePropPageClass::OnApply (void)
{
	/*SAFE_DELETE_ARRAY (m_OrigSizes.KeyTimes);
	SAFE_DELETE_ARRAY (m_OrigSizes.Values);

	//
	//	Make the current values from the emitter the default values.
	//
	m_pEmitterList->Get_Size_Key_Frames (m_OrigSizes);*/

	// Allow the base class to process this message
	return CPropertyPage::OnApply ();
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL
EmitterSizePropPageClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
)
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
		case IDC_SIZE_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {			
				
				//
				//	Allow the user to edit the keyframe
				//
				float size = m_SizeBar->Get_Graph_Percent (color_bar_hdr->key_index) * m_MaxSize;
				ParticleSizeDialogClass dialog (size, this);
				if (dialog.DoModal () == IDOK) {
					size = dialog.Get_Size ();

					m_SizeBar->Set_Redraw (false);
					m_SizeBar->Set_Graph_Percent (color_bar_hdr->key_index, size / m_MaxSize);
					
					//
					//	Determine if the user changed the 'max' size
					//
					float new_max = size;
					int count = m_SizeBar->Get_Point_Count ();
					for (int index = 0; index < count; index ++) {
						float size = m_SizeBar->Get_Graph_Percent (index) * m_MaxSize;
						if (size > new_max) {
							new_max = size;
						}
					}

					//
					//	Rescale the key frame points if necessary
					//
					if (new_max != m_MaxSize) {
						int count = m_SizeBar->Get_Point_Count ();
						for (int index = 0; index < count; index ++) {
							float percent = m_SizeBar->Get_Graph_Percent (index);
							m_SizeBar->Set_Graph_Percent (index, (percent * m_MaxSize) / new_max);
						}

						// Remember the new size maximum
						m_MaxSize = new_max;
					}
					m_SizeBar->Set_Redraw (true);

					//
					// Update the emitter
					//					
					Update_Sizes ();
					m_pEmitterList->Set_Size_Keyframes (m_CurrentSizes);
					SetModified ();
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT)) {			
				
				//
				// Update the emitter
				//
				Update_Sizes ();
				m_pEmitterList->Set_Size_Keyframes (m_CurrentSizes);
				SetModified ();					
			}
		}
		break;

		case IDC_SIZE_RANDOM_SPIN:
		{
			// Update the emitter
			m_CurrentSizes.Rand = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_RANDOM_EDIT);
			m_pEmitterList->Set_Size_Keyframes (m_CurrentSizes);
			SetModified ();
		}
		break;
	}
			
	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  Update_Sizes
//
/////////////////////////////////////////////////////////////
void
EmitterSizePropPageClass::Update_Sizes (void)
{
	float position = 0;
	float red = 0;
	float green = 0;
	float blue = 0;

	//
	//	Setup the initial or 'starting' size
	//
	m_CurrentSizes.Start = m_SizeBar->Get_Graph_Percent (0) * m_MaxSize;

	//
	// Free the current setting arrays
	//
	SAFE_DELETE_ARRAY (m_CurrentSizes.KeyTimes);
	SAFE_DELETE_ARRAY (m_CurrentSizes.Values);

	//
	//	Determine if we need to build the array of key frames or not
	//
	int count = m_SizeBar->Get_Point_Count ();
	m_CurrentSizes.NumKeyFrames = count - 1;
	if (count > 1) {
		m_CurrentSizes.KeyTimes = new float[count - 1];
		m_CurrentSizes.Values = new float[count - 1];

		//
		//	Get all the size key frames and add them to our structure
		//	
		for (int index = 1; index < count; index ++) {
			m_SizeBar->Get_Point (index, &position, &red, &green, &blue);			
			m_CurrentSizes.KeyTimes[index - 1] = position * m_Lifetime;
			m_CurrentSizes.Values[index - 1] = m_SizeBar->Get_Graph_Percent (index) * m_MaxSize;
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
EmitterSizePropPageClass::OnCommand
(
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (LOWORD (wParam))
	{
		case IDC_SIZE_RANDOM_EDIT:
		{
			// Update the emitter
			if ((HIWORD (wParam) == EN_KILLFOCUS) &&
				 SendDlgItemMessage (LOWORD (wParam), EM_GETMODIFY)) {
				SendDlgItemMessage (LOWORD (wParam), EM_SETMODIFY, (WPARAM)0);

				m_CurrentSizes.Rand = ::GetDlgItemFloat (m_hWnd, IDC_SIZE_RANDOM_EDIT);
				m_pEmitterList->Set_Size_Keyframes (m_CurrentSizes);
				SetModified ();
			} else if (HIWORD (wParam) == EN_CHANGE) {
				SetModified ();
			}
		}
		break;
	}

	return CPropertyPage::OnCommand (wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  On_Lifetime_Changed
//
/////////////////////////////////////////////////////////////
void
EmitterSizePropPageClass::On_Lifetime_Changed (float lifetime)
{
	if (m_Lifetime != lifetime) {
		float conversion = lifetime / m_Lifetime;

		//
		//	Rescale the sizes
		//
		for (UINT index = 0; index < m_CurrentSizes.NumKeyFrames; index ++) {
			m_CurrentSizes.KeyTimes[index] *= conversion;
		}

		//
		//	Update the emitter
		//
		m_pEmitterList->Set_Size_Keyframes (m_CurrentSizes);
		m_Lifetime = lifetime;
	}

	return ;
}


