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
 *                     $Archive:: /VSS_Sync/W3DView/EmitterPropertySheet.cpp                                                                                                                                                                                                                                                                                                                                         $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "StdAfx.H"
#include "W3DView.H"
#include "EmitterPropertySheet.H"
#include "Part_Emt.H"
#include "Part_Ldr.H"
#include "AssetMgr.H"
#include "W3DViewDoc.H"
#include "Utils.H"
#include "DataTreeView.H"
#include "AssetInfo.H"
#include "texture.h"
#include "EmitterInstanceList.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EmitterPropertySheetClass

IMPLEMENT_DYNAMIC(EmitterPropertySheetClass, CPropertySheet)


/////////////////////////////////////////////////////////////
//
//  EmitterPropertySheetClass
//
EmitterPropertySheetClass::EmitterPropertySheetClass
(
	EmitterInstanceListClass *emitter_list,
	UINT nIDCaption,
	CWnd *pParentWnd
)
	:  m_pEmitterList (NULL),
	   CPropertySheet (nIDCaption, pParentWnd, 0)
{
	m_pEmitterList = emitter_list;
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  EmitterPropertySheetClass
//
EmitterPropertySheetClass::EmitterPropertySheetClass
(
	EmitterInstanceListClass *emitter_list,
	LPCTSTR pszCaption,
	CWnd *pParentWnd
)
	:  m_pEmitterList (NULL),
	   CPropertySheet (pszCaption, pParentWnd, 0)
{
	m_pEmitterList = emitter_list;
	Initialize ();
	return ;
}


/////////////////////////////////////////////////////////////
//
//  EmitterPropertySheetClass
//
EmitterPropertySheetClass::~EmitterPropertySheetClass (void)
{
	SAFE_DELETE (m_pEmitterList);
	return ;
}


BEGIN_MESSAGE_MAP(EmitterPropertySheetClass, CPropertySheet)
	//{{AFX_MSG_MAP(EmitterPropertySheetClass)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EmitterPropertySheetClass message handlers


/////////////////////////////////////////////////////////////
//
//  EmitterPropertySheetClass
//
LRESULT
EmitterPropertySheetClass::WindowProc
(
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (message)
	{
		// Is a control sending us an oldstyle notification?
		case WM_COMMAND:
		{
			// What control sent the notification?
			switch (LOWORD (wParam))
			{
				case IDCANCEL:
				{
					::GetCurrentDocument ()->Reload_Displayed_Object ();
				}
				break;

				case IDOK:
				{
					// If the apply button isn't enabled, then don't do the apply operation.
					if (::IsWindowEnabled (::GetDlgItem (m_hWnd, ID_APPLY_NOW)) == FALSE) {
						break;
					}
				}
				case ID_APPLY_NOW:
				{					
					// Did the user click the button?
					if (HIWORD (wParam) == BN_CLICKED) {
						LRESULT lresult = CPropertySheet::WindowProc (message, wParam, lParam);

						// If all the pages contain valid data, then update the emitter
						if (m_GeneralPage.Is_Data_Valid () &&
							 m_ParticlePage.Is_Data_Valid () &&
							 m_PhysicsPage.Is_Data_Valid () &&
							 m_ColorPage.Is_Data_Valid () &&
							 m_UserPage.Is_Data_Valid () &&
							 m_SizePage.Is_Data_Valid () &&
							 m_LinePage.Is_Data_Valid () && 
							 m_RotationPage.Is_Data_Valid () &&
							 m_FramePage.Is_Data_Valid () &&
							 m_LineGroupPage.Is_Data_Valid () ) 
						{
							// Update the current emitter to match the data
							Update_Emitter ();
						}
												
						return lresult;
					}
				}
				break;
			}
			break;
		}
		break;
	}

	// Allow the base class to process this message
	return CPropertySheet::WindowProc (message, wParam, lParam);
}


/////////////////////////////////////////////////////////////
//
//  Add_Emitter_To_Viewer
//
void
EmitterPropertySheetClass::Add_Emitter_To_Viewer (void)
{
	CW3DViewDoc *pdoc = ::GetCurrentDocument ();
	if ((pdoc != NULL) && (m_pEmitterList != NULL)) {
		
		//
		// Create a new prototype for this emitter and add it to the asset manager
		//
		ParticleEmitterDefClass *pdefinition		= new ParticleEmitterDefClass (*m_pEmitterList);
		ParticleEmitterPrototypeClass *pprototype	= new ParticleEmitterPrototypeClass (pdefinition);
		
		//
		// Update the asset manager with the new prototype
		//
		if (m_LastSavedName.GetLength () > 0) {
			WW3DAssetManager::Get_Instance()->Remove_Prototype (m_LastSavedName);
		}
		WW3DAssetManager::Get_Instance()->Add_Prototype (pprototype);
		
		//
		// Add this emitter to the data tree
		//
		CDataTreeView *pdata_tree = pdoc->GetDataTreeView ();
		pdata_tree->Refresh_Asset (m_pEmitterList->Get_Name (), m_LastSavedName, TypeEmitter);
		/*if (m_LastSavedName.GetLength () > 0) {
			pdata_tree->Refresh_Asset (m_pEmitterList->Get_Name (), m_LastSavedName, TypeEmitter);
		} else {
			pdata_tree->Add_Asset_To_Tree (m_pEmitterList->Get_Name (), TypeEmitter, true);
		}*/

		//
		// Display the emitter
		//
		pdoc->Reload_Displayed_Object ();
		m_LastSavedName = m_pEmitterList->Get_Name ();

		//
		// Regenerate the emitter pointer list
		//
		m_pEmitterList->Free_List ();
		pdoc->Build_Emitter_List (m_pEmitterList, m_pEmitterList->Get_Name ());		
	}

	return ;
}


/////////////////////////////////////////////////////////////
//
//  Update_Emitter
//
void
EmitterPropertySheetClass::Update_Emitter (void)
{
	//
	//	Update those pages that are dependant on the particle's
	// lifetime.
	//
	float lifetime = m_GeneralPage.Get_Lifetime ();
	m_ColorPage.On_Lifetime_Changed (lifetime);
	m_SizePage.On_Lifetime_Changed (lifetime);
	m_RotationPage.On_Lifetime_Changed (lifetime);
	m_FramePage.On_Lifetime_Changed (lifetime);
	m_LineGroupPage.On_Lifetime_Changed (lifetime);

	Add_Emitter_To_Viewer ();

	//
	// Create a new emitter
	//
	/*ParticleEmitterClass *pemitter = Create_Emitter ();
	Add_Emitter_To_Viewer (pemitter);

	//	
	// Use this emitter as the edited emitter from here on out
	//
	MEMBER_RELEASE (m_pEmitter);
	m_pEmitter = pemitter;*/

	// Pass the emitter along to the pages
	/*m_GeneralPage.Set_Emitter (m_pEmitterList);
	m_ParticlePage.Set_Emitter (m_pEmitterList);
	m_PhysicsPage.Set_Emitter (m_pEmitterList);
	m_ColorPage.Set_Emitter (m_pEmitterList);
	m_UserPage.Set_Emitter (m_pEmitterList);
	m_SizePage.Set_Emitter (m_pEmitterList);*/
	return ;
}


/////////////////////////////////////////////////////////////
//
//  Initialize
//
void
EmitterPropertySheetClass::Initialize (void)
{
	if (m_pEmitterList == NULL) {
		Create_New_Emitter ();		
	} else {
		m_LastSavedName = m_pEmitterList->Get_Name ();
	}

	// Pass the emitter along to the pages
	m_GeneralPage.Set_Emitter (m_pEmitterList);
	m_ParticlePage.Set_Emitter (m_pEmitterList);
	m_PhysicsPage.Set_Emitter (m_pEmitterList);
	m_ColorPage.Set_Emitter (m_pEmitterList);
	m_UserPage.Set_Emitter (m_pEmitterList);
	m_SizePage.Set_Emitter (m_pEmitterList);
	m_LinePage.Set_Emitter (m_pEmitterList);
	m_RotationPage.Set_Emitter (m_pEmitterList);
	m_FramePage.Set_Emitter (m_pEmitterList);
	m_LineGroupPage.Set_Emitter (m_pEmitterList);

	// Initialize the user page with data from the prototype
	/*if (m_pEmitter != NULL) {
		ParticleEmitterPrototypeClass *proto = NULL;
		proto = (ParticleEmitterPrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (m_pEmitter->Get_Name ());
		if (proto != NULL) {
			ParticleEmitterDefClass *definition = proto->Get_Definition ();
			m_UserPage.Set_Type (definition->Get_User_Type ());
			m_UserPage.Set_String (definition->Get_User_String ());
		}
	}*/

	// Add the pages to the sheet
	AddPage (&m_GeneralPage);
	AddPage (&m_ParticlePage);
	AddPage (&m_PhysicsPage);
	AddPage (&m_ColorPage);
	AddPage (&m_SizePage);
	AddPage (&m_UserPage);	
	AddPage (&m_LinePage);
	AddPage (&m_RotationPage);
	AddPage (&m_FramePage);
	AddPage (&m_LineGroupPage);

	m_GeneralPage.Set_Parent(this);

	return ;
}


/////////////////////////////////////////////////////////////
//
//  Create_Emitter
//
/*ParticleEmitterClass *
EmitterPropertySheetClass::Create_Emitter (void)
{
	//
	//	Read the particle settings
	//
	float rate				= m_ParticlePage.Get_Rate ();
	int burst				= m_ParticlePage.Get_Burst_Size ();
	float max_particles	= m_ParticlePage.Get_Max_Particles ();
	
	//
	//	Read the physics settings
	//
	Vector3 velocity		= m_PhysicsPage.Get_Velocity ();
	Vector3 acceleration	= m_PhysicsPage.Get_Acceleration ();
	float out_factor		= m_PhysicsPage.Get_Out_Factor ();
	float inherit_factor	= m_PhysicsPage.Get_Inheritance_Factor ();

	//
	//	Read the general settings
	//
	CString name			= m_GeneralPage.Get_Name ();
	CString texture_name	= m_GeneralPage.Get_Texture_Filename ();
	float lifetime			= m_GeneralPage.Get_Lifetime ();
	ShaderClass shader	= m_GeneralPage.Get_Shader ();

	//
	//	Read the keyframe settings
	//
	ParticlePropertyStruct<Vector3> colors;
	ParticlePropertyStruct<float> opacity;
	ParticlePropertyStruct<float> size;
	m_ColorPage.Get_Color_Keyframes (colors);
	m_ColorPage.Get_Opacity_Keyframes (opacity);
	m_SizePage.Get_Size_Keyframes (size);
	
	//
	//	Read the randomizers
	//
	Vector3Randomizer *creation_vol	= m_ParticlePage.Get_Creation_Volume ();
	Vector3Randomizer *vel_random		= m_PhysicsPage.Get_Velocity_Random ();

	//
	//	Load the texture
	//
	TextureClass *ptexture = NULL;
	if (texture_name.GetLength () > 0) {
		ptexture = WW3DAssetManager::Get_Instance()->Get_Texture (texture_name);
	}

	//
	//	Create the new particle emitter
	//
	ParticleEmitterClass *pemitter = new ParticleEmitterClass (rate,
																					burst,
																					creation_vol,
																					velocity,
																					vel_random,
																					out_factor,
																					inherit_factor,
																					colors,
																					opacity,
																					size,
																					acceleration,
																					lifetime,
																					ptexture,
																					shader,
																					max_particles);

	
	//
	//	Pass the name onto the emitter
	//
	pemitter->Set_Name (name);

	// Return the emitter
	return pemitter;
}*/


/////////////////////////////////////////////////////////////
//
//  Create_New_Emitter
//
void
EmitterPropertySheetClass::Create_New_Emitter (void)
{
	ParticlePropertyStruct<Vector3> color;
	color.Start = Vector3 (1, 1, 1);
	color.Rand.Set (0,0,0);
	color.NumKeyFrames = 0;
	color.KeyTimes = NULL;
	color.Values = NULL;

	ParticlePropertyStruct<float> opacity;
	opacity.Start = 1.0F;
	opacity.Rand = 0.0F;
	opacity.NumKeyFrames = 0;
	opacity.KeyTimes = NULL;
	opacity.Values = NULL;

	ParticlePropertyStruct<float> size;
	size.Start = 0.1F;
	size.Rand = 0.0F;
	size.NumKeyFrames = 0;
	size.KeyTimes = NULL;
	size.Values = NULL;

	ParticlePropertyStruct<float> rotation;
	rotation.Start = 0.0f;
	rotation.Rand = 0.0f;
	rotation.NumKeyFrames = 0;
	rotation.KeyTimes = NULL;
	rotation.Values = NULL;

	ParticlePropertyStruct<float> frames;
	frames.Start = 0.0f;
	frames.Rand = 0.0f;
	frames.NumKeyFrames = 0;
	frames.KeyTimes = NULL;
	frames.Values = NULL;

	ParticlePropertyStruct<float> blurtimes;
	blurtimes.Start = 0.0f;
	blurtimes.Rand = 0.0f;
	blurtimes.NumKeyFrames = 0;
	blurtimes.KeyTimes = NULL;
	blurtimes.Values = NULL;

	//
	//	Create the new emitter
	//
	ParticleEmitterClass *emitter = NULL;
	emitter = new ParticleEmitterClass (10,
													1,
													new Vector3SolidBoxRandomizer(Vector3(0.1F, 0.1F, 0.1F)),
													Vector3 (0, 0, 1),
													new Vector3SolidBoxRandomizer(Vector3(0, 0, 0.1F)),
													0,
													0,
													color,
													opacity,
													size,
													rotation,
													0.0f,
													frames,
													blurtimes,
													Vector3 (0, 0, 0),
													1.0F,
													0.0F,
													NULL,
													ShaderClass::_PresetAdditiveSpriteShader,
													0);

	//
	//	Create the new emitter list
	//
	m_pEmitterList = new EmitterInstanceListClass;
	m_pEmitterList->Add_Emitter (emitter);

	//
	//	Display the new emitter
	//
	::GetCurrentDocument ()->Display_Emitter (emitter);
	MEMBER_RELEASE (emitter);

	/*SAFE_DELETE_ARRAY (color.Values);
	SAFE_DELETE_ARRAY (color.KeyTimes);
	SAFE_DELETE_ARRAY (opacity.Values);
	SAFE_DELETE_ARRAY (opacity.KeyTimes);
	SAFE_DELETE_ARRAY (size.Values);
	SAFE_DELETE_ARRAY (size.KeyTimes);*/
	return ;
}


void
EmitterPropertySheetClass::Notify_Render_Mode_Changed(int new_mode)
{
	bool enable_line_page = (new_mode == W3D_EMITTER_RENDER_MODE_LINE);
	::Enable_Dialog_Controls(m_LinePage,enable_line_page);
}

