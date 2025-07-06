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

// SphereColorPropPage.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "SphereColorPropPage.h"
#include "OpacitySettingsDialog.h"
#include "ColorUtils.h"
#include "Utils.h"
#include "OpacityVectorDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(SphereColorPropPageClass, CPropertyPage)

/////////////////////////////////////////////////////////////
//
//	SphereColorPropPageClass
//
/////////////////////////////////////////////////////////////
SphereColorPropPageClass::SphereColorPropPageClass (SphereRenderObjClass *sphere)
	:	m_RenderObj (sphere),
		m_bValid (true),
		m_ColorBar (NULL),
		m_OpacityBar (NULL),
		m_EnableOpactiyVector (false),
		m_InvertVector (false),
		CPropertyPage(SphereColorPropPageClass::IDD)
{
	//{{AFX_DATA_INIT(SphereColorPropPageClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//	~SphereColorPropPageClass
//
/////////////////////////////////////////////////////////////
SphereColorPropPageClass::~SphereColorPropPageClass (void)
{
	return ;
}


/////////////////////////////////////////////////////////////
//
//	DoDataExchange
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::DoDataExchange (CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SphereColorPropPageClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(SphereColorPropPageClass, CPropertyPage)
	//{{AFX_MSG_MAP(SphereColorPropPageClass)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_OPACITY_VECTOR_CHECK, OnOpacityVectorCheck)
	ON_BN_CLICKED(IDC_INVERT_VECTOR_CHECK, OnInvertVectorCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////
//
//  Initialize
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::Initialize (void)
{
	m_ColorChannel.Reset ();
	m_OrigColorChannel.Reset ();
	m_AlphaChannel.Reset ();
	m_OrigAlphaChannel.Reset ();
	m_VectorChannel.Reset ();
	m_OrigVectorChannel.Reset ();	

	if (m_RenderObj != NULL) {

		m_ColorChannel			= m_RenderObj->Get_Color_Channel ();
		m_OrigColorChannel	= m_RenderObj->Get_Color_Channel ();

		m_AlphaChannel			= m_RenderObj->Get_Alpha_Channel ();
		m_OrigAlphaChannel	= m_RenderObj->Get_Alpha_Channel ();

		m_VectorChannel		= m_RenderObj->Get_Vector_Channel ();
		m_OrigVectorChannel	= m_RenderObj->Get_Vector_Channel ();

		m_EnableOpactiyVector	= (m_RenderObj->Get_Flags () & SphereRenderObjClass::USE_ALPHA_VECTOR) != 0;
		m_InvertVector				= (m_RenderObj->Get_Flags () & SphereRenderObjClass::USE_INVERSE_ALPHA) != 0;

		if (m_ColorChannel.Get_Key_Count () == 0) {
			m_ColorChannel.Add_Key (m_RenderObj->Get_Color (), 0);
			m_OrigColorChannel.Add_Key (m_RenderObj->Get_Color (), 0);			
		}

		if (m_AlphaChannel.Get_Key_Count () == 0) {
			m_AlphaChannel.Add_Key (m_RenderObj->Get_Alpha (), 0);
			m_OrigAlphaChannel.Add_Key (m_RenderObj->Get_Alpha(), 0);
		}

		if (m_VectorChannel.Get_Key_Count () == 0) {
			m_VectorChannel.Add_Key (m_RenderObj->Get_Vector (), 0);
			m_OrigVectorChannel.Add_Key (m_RenderObj->Get_Vector (), 0);
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
SphereColorPropPageClass::OnInitDialog (void)
{
	// Allow the base class to process this message
	CPropertyPage::OnInitDialog ();
	
	m_ColorBar		= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_COLOR_BAR));
	m_OpacityBar	= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_OPACITY_BAR));
	m_VectorBar		= ColorBarClass::Get_Color_Bar (::GetDlgItem (m_hWnd, IDC_VECTOR_BAR));

	//
	// Setup the color bars
	//
	m_ColorBar->Set_Range (0, 1);
	m_OpacityBar->Set_Range (0, 1);
	m_VectorBar->Set_Range (0, 1);

	//
	//	Set-up the color bar
	//
	int index;
	for (index = 0; index < m_OrigColorChannel.Get_Key_Count (); index ++) {
		m_ColorBar->Modify_Point (	index,
											m_OrigColorChannel[index].Get_Time (),
											m_OrigColorChannel[index].Get_Value ().X * 255,
											m_OrigColorChannel[index].Get_Value ().Y * 255,
											m_OrigColorChannel[index].Get_Value ().Z * 255);
	}

	//
	//	Set-up the opacity bar
	//
	for (index = 0; index < m_OrigAlphaChannel.Get_Key_Count (); index ++) {
		m_OpacityBar->Modify_Point (	index,
												m_OrigAlphaChannel[index].Get_Time (),
												m_OrigAlphaChannel[index].Get_Value () * 255,
												m_OrigAlphaChannel[index].Get_Value () * 255,
												m_OrigAlphaChannel[index].Get_Value () * 255);
	}

	//
	//	Set-up the vector bar
	//
	for (index = 0; index < m_OrigVectorChannel.Get_Key_Count (); index ++) {
		m_VectorBar->Modify_Point (index,
											m_OrigVectorChannel[index].Get_Time (),
											128,
											128,
											128);
		
		AlphaVectorStruct *data = new AlphaVectorStruct (m_OrigVectorChannel[index].Get_Value ());
		m_VectorBar->Set_User_Data (index, (ULONG)data);
	}

	//
	//	Ensure our initial 'current' values are up-to-date
	//
	Update_Colors ();
	Update_Opacities ();
	Update_Vectors ();

	//
	//	Ensure the disabled status of the dialog controls is correct
	//
	CheckDlgButton (IDC_OPACITY_VECTOR_CHECK, (m_RenderObj->Get_Flags () & SphereRenderObjClass::USE_ALPHA_VECTOR) != 0);
	CheckDlgButton (IDC_INVERT_VECTOR_CHECK, (m_RenderObj->Get_Flags () & SphereRenderObjClass::USE_INVERSE_ALPHA) != 0);	
	OnOpacityVectorCheck ();
	return TRUE;
}


/////////////////////////////////////////////////////////////
//
//  OnApply
//
/////////////////////////////////////////////////////////////
BOOL
SphereColorPropPageClass::OnApply (void)
{
	// Allow the base class to process this message
	return CPropertyPage::OnApply ();
}


/////////////////////////////////////////////////////////////
//
//  OnDestroy
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::OnDestroy (void)
{
	//
	//	Free the alpha vectors associated with the keys...
	//
	int count = m_VectorBar->Get_Point_Count ();
	for (int index = 0; index < count; index ++) {
		AlphaVectorStruct *data = (AlphaVectorStruct *)m_VectorBar->Get_User_Data (index);
		if (data != NULL) {
			delete data;
			m_VectorBar->Set_User_Data (index, 0L);
		}
	}
	
	CPropertyPage::OnDestroy();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnNotify
//
/////////////////////////////////////////////////////////////
BOOL
SphereColorPropPageClass::OnNotify
(
	WPARAM wParam,
	LPARAM lParam,
	LRESULT *pResult
) 
{
	CBR_NMHDR *color_bar_hdr = (CBR_NMHDR *)lParam;

	//
	//	Which control sent the notification?
	//
	switch (color_bar_hdr->hdr.idFrom)
	{
		case IDC_OPACITY_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {			
				
				//
				//	Allow the user to edit the keyframe
				//
				OpacitySettingsDialogClass dialog (color_bar_hdr->red / 255, this);
				if (dialog.DoModal () == IDOK) {
					m_OpacityBar->Modify_Point (	color_bar_hdr->key_index,
															color_bar_hdr->position,
															dialog.Get_Opacity () * 255,
															dialog.Get_Opacity () * 255,
															dialog.Get_Opacity () * 255);

					//
					// Update the object
					//
					Update_Opacities ();					
					SetModified ();					
				}
			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT))
			{				
				//
				// Update the object
				//
				Update_Opacities ();
				SetModified ();					
			}
		}
		break;

		case IDC_COLOR_BAR:
		{
			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {			
				
				//
				//	Allow the user to edit the keyframe
				//				
				int red		= (int)color_bar_hdr->red;
				int green	= (int)color_bar_hdr->green;
				int blue		= (int)color_bar_hdr->blue;
				if (Show_Color_Picker (&red, &green, &blue)) {
					m_ColorBar->Modify_Point (	color_bar_hdr->key_index,
														color_bar_hdr->position,
														red,
														green,
														blue);

					//
					// Update the object
					//
					Update_Colors ();					
					SetModified ();
				}

			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT))
			{
				//
				// Update the object
				//
				Update_Colors ();
				SetModified ();
			}
		}
		break;

		case IDC_VECTOR_BAR:
		{
			bool update = false;

			if (color_bar_hdr->hdr.code == CBRN_DBLCLK_POINT) {
				
				AlphaVectorStruct *data = (AlphaVectorStruct *)m_VectorBar->Get_User_Data (color_bar_hdr->key_index);
				if (data != NULL) {
					
					//
					//	Set-up the dialog so the user can edit this keyframe
					//
					OpacityVectorDialogClass dialog (this);
					dialog.Set_Render_Obj (m_RenderObj);
					dialog.Set_Vector (*data);
					dialog.Set_Key_Index (color_bar_hdr->key_index);

					//
					//	Show the dialog
					//
					if (dialog.DoModal () == IDOK) {
						(*data) = dialog.Get_Vector ();
					}
				}

			} else if ((color_bar_hdr->hdr.code == CBRN_MOVING_POINT) ||
						  (color_bar_hdr->hdr.code == CBRN_DELETED_POINT))
			{
				update = true;
			} else if (color_bar_hdr->hdr.code == CBRN_DEL_POINT) {
				AlphaVectorStruct *data = (AlphaVectorStruct *)m_VectorBar->Get_User_Data (color_bar_hdr->key_index);
				SAFE_DELETE (data);
				m_VectorBar->Set_User_Data (color_bar_hdr->key_index, 0L);
			} else if (color_bar_hdr->hdr.code == CBRN_INSERTED_POINT) {				
				AlphaVectorStruct *prev_data	= (AlphaVectorStruct *)m_VectorBar->Get_User_Data (color_bar_hdr->key_index - 1);
				AlphaVectorStruct *next_data	= (AlphaVectorStruct *)m_VectorBar->Get_User_Data (color_bar_hdr->key_index + 1);
				AlphaVectorStruct *new_data	= new AlphaVectorStruct;

				if (next_data == NULL) {
					(*new_data) = (*prev_data);
				} else {
					
					//
					//	Determine what the new data should be based on its position between
					// the prev and next keys.
					//
					float pos0 = 0;
					float pos1 = 0;
					float pos2 = 0;
					float	red = 0;
					float green = 0;
					float blue = 0;
					m_VectorBar->Get_Point (color_bar_hdr->key_index - 1, &pos0, &red, &green, &blue);
					m_VectorBar->Get_Point (color_bar_hdr->key_index, &pos1, &red, &green, &blue);
					m_VectorBar->Get_Point (color_bar_hdr->key_index + 1, &pos2, &red, &green, &blue);

					float percent = (pos1 - pos0) / (pos2 - pos0);
					new_data->intensity = prev_data->intensity + ((next_data->intensity - prev_data->intensity) * percent);
					::Slerp (new_data->angle,prev_data->angle, next_data->angle, percent);
				}

				//
				//	Associate this data with the new key
				//
				m_VectorBar->Set_User_Data (color_bar_hdr->key_index, (ULONG)new_data);
				update = true;
			}

			//
			// Update the object
			//
			if (update) {
				Update_Vectors ();				
				SetModified ();
			}
		}
		break;
	}
		
	return CPropertyPage::OnNotify (wParam, lParam, pResult);
}


/////////////////////////////////////////////////////////////
//
//  OnCancel
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::OnCancel (void)
{
	//
	//	Reset the object to its original state
	//
	m_RenderObj->Set_Color_Channel (m_OrigColorChannel);
	m_RenderObj->Set_Alpha_Channel (m_OrigAlphaChannel);
	m_RenderObj->Set_Vector_Channel (m_OrigVectorChannel);

	m_RenderObj->Set_Flag (SphereRenderObjClass::USE_ALPHA_VECTOR, m_EnableOpactiyVector);
	m_RenderObj->Set_Flag (SphereRenderObjClass::USE_INVERSE_ALPHA, m_InvertVector);

	CPropertyPage::OnCancel ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Opacities
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::Update_Opacities (void)
{
	m_AlphaChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;
	
	//
	//	Build the channel
	//	
	int count = m_OpacityBar->Get_Point_Count ();
	for (int index = 0; index < count; index ++) {
		m_OpacityBar->Get_Point (index, &position, &red, &green, &blue);
		m_AlphaChannel.Add_Key (red / 255, position);
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Alpha_Channel (m_AlphaChannel);
	m_RenderObj->Restart_Animation ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Colors
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::Update_Colors (void)
{		
	m_ColorChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;	
		
	//
	//	Build the channel
	//		
	int count = m_ColorBar->Get_Point_Count ();
	for (int index = 0; index < count; index ++) {
		m_ColorBar->Get_Point (index, &position, &red, &green, &blue);
		m_ColorChannel.Add_Key (Vector3 (red / 255, green / 255, blue / 255), position);
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Color_Channel (m_ColorChannel);
	m_RenderObj->Restart_Animation ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Vectors
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::Update_Vectors (void)
{
	m_VectorChannel.Reset ();

	float position	= 0;
	float red		= 0;
	float green		= 0;
	float blue		= 0;

	//
	//	Build the channel
	//	
	int count = m_VectorBar->Get_Point_Count ();
	for (int index = 0; index < count; index ++) {
		m_VectorBar->Get_Point (index, &position, &red, &green, &blue);
			
		AlphaVectorStruct *data = (AlphaVectorStruct *)m_VectorBar->Get_User_Data (index);
		if (data != NULL) {
			m_VectorChannel.Add_Key (*data, position);
		}
	}

	//
	//	Update the object
	//
	m_RenderObj->Set_Vector_Channel (m_VectorChannel);
	m_RenderObj->Restart_Animation ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  OnOpacityVectorCheck
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::OnOpacityVectorCheck (void)
{
	bool is_checked = (IsDlgButtonChecked (IDC_OPACITY_VECTOR_CHECK) == 1);

	//
	//	Enable/disable the alpha-vector timeline
	//
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_VECTOR_BAR), is_checked);
	::EnableWindow (::GetDlgItem (m_hWnd, IDC_INVERT_VECTOR_CHECK), is_checked);

	//
	//	Update the render object
	//
	m_RenderObj->Set_Flag (SphereRenderObjClass::USE_ALPHA_VECTOR, is_checked);
	SetModified ();
	return ;
}

/////////////////////////////////////////////////////////////
//
//  OnInvertVectorCheck
//
/////////////////////////////////////////////////////////////
void
SphereColorPropPageClass::OnInvertVectorCheck (void)
{
	bool is_checked = (IsDlgButtonChecked (IDC_INVERT_VECTOR_CHECK) == 1);

	//
	//	Invert the effect
	//
	m_RenderObj->Set_Flag (SphereRenderObjClass::USE_INVERSE_ALPHA, is_checked);

	//
	//	Update the render object
	//
	m_RenderObj->Set_Flag (SphereRenderObjClass::USE_INVERSE_ALPHA, is_checked);	
	SetModified ();
	return ;
}
