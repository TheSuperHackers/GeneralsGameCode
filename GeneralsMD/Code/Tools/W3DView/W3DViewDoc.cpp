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

// W3DViewDoc.cpp : implementation of the CW3DViewDoc class
//

#include "stdafx.h"

#include "w3dview.h"
#include "w3dviewdoc.h"
#include "ffactory.h"
#include "globals.h"
#include "viewerassetmgr.h"
#include "globals.h"
#include "rendobj.h"
#include "graphicview.h"
#include "datatreeview.h"
#include "mainfrm.h"
#include "distlod.h"
#include "light.h"
#include "camera.h"
#include "w3d_file.h"
#include "wwfile.h"
#include "bmp2d.h"
#include "part_emt.h"
#include "part_ldr.h"
#include "utils.h"
#include "w3derr.h"
#include "chunkio.h"
#include "assetinfo.h"
#include "meshmdl.h"
#include "agg_def.h"
#include "hlod.h"
#include "restrictedfiledialog.h"
#include "viewerscene.h"
#include "ini.h"
#include "ww3d.h"
#include "emitterinstancelist.h"
#include "mesh.h"
#include "screencursor.h"
#include "sphereobj.h"
#include "ringobj.h"
#include "textfile.h"
#include "hmorphanim.h"
#include "mmsystem.h"
#include "soundrobj.h"
#include "dazzle.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CW3DViewDoc

IMPLEMENT_DYNCREATE(CW3DViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CW3DViewDoc, CDocument)
	//{{AFX_MSG_MAP(CW3DViewDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CW3DViewDoc construction/destruction


///////////////////////////////////////////////////////////////
//
//  CW3DViewDoc
//
CW3DViewDoc::CW3DViewDoc (void)
    : m_pCScene (NULL),
      m_pC2DScene (NULL),
		m_pCursorScene (NULL),
      m_pCBackObjectScene (NULL),
		m_pDazzleLayer (NULL),
      m_pCBackObjectCamera (NULL),  
      m_pCBackgroundObject (NULL),
      m_pC2DCamera (NULL),
      m_pCSceneLight (NULL),
      m_pCRenderObj (NULL),
      m_pCAnimation (NULL),
		m_pCAnimCombo (NULL),
		m_pCBackgroundBMP (NULL),		
      m_CurrentFrame (0),
      m_bAnimBlend (TRUE),
		m_bAnimateCamera (false),
		m_bAutoCameraReset (true),
		m_bOneTimeReset (true),
		m_pCursor (NULL),
      m_backgroundColor (0.5F, 0.5F, 0.5F),
		m_ManualFOV (false),
		m_ManualClipPlanes (false),
		m_IsInitialized (false),
		m_bFogEnabled(false),
		m_nChannelQnBytes(2),
		m_bCompress_channel_Q(false)
{
	// Read the camera animation settings from the registry
	m_bAnimateCamera = ((BOOL)theApp.GetProfileInt ("Config", "AnimateCamera", 0)) == TRUE;
	m_bAutoCameraReset = ((BOOL)theApp.GetProfileInt ("Config", "ResetCamera", 1)) == TRUE;
	return ;
}

///////////////////////////////////////////////////////////////
//
//  ~CW3DViewDoc
//
///////////////////////////////////////////////////////////////
CW3DViewDoc::~CW3DViewDoc (void)
{
    CleanupResources ();
	 MEMBER_RELEASE (m_pCursor);
    return ;
}


///////////////////////////////////////////////////////////////
//
//  CleanupResources
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::CleanupResources (void)
{
    if (m_pC2DScene)
    {
		  if (m_pCBackgroundBMP)
        {
            // Remove the background BMP from the scene
				m_pCBackgroundBMP->Remove ();
        }

        // Release the 2D scene we allocated to display background BMPs
        m_pC2DScene->Release_Ref ();
        m_pC2DScene = NULL;
    }

    if (m_pCBackObjectScene)
    {
        if (m_pCBackgroundObject)
        {
            // Remove the background BMP from the scene
				m_pCBackgroundObject->Remove ();
        }

        // Release the scene we allocated to display background objects
        m_pCBackObjectScene->Release_Ref ();
        m_pCBackObjectScene = NULL;
    }

	if (m_pCursor != NULL) {
		m_pCursor->Remove ();		
	}
	MEMBER_RELEASE (m_pCursorScene);

    if (m_pCScene)
    {
        if (m_pCRenderObj)
        {
            // Remove the currently displayed object from the scene
				Remove_Object_From_Scene (m_pCRenderObj);
        }

        if (m_pCSceneLight)
        {
            // Remove the light from the scene
				Remove_Object_From_Scene (m_pCSceneLight);
        }

		  // Get rid of the lined up objects.
		  m_pCScene->Clear_Lineup();

        // Release the scene object we allocated earlier
        m_pCScene->Release_Ref ();
        m_pCScene = NULL;
    }

	 // Was there a dazzle layer?
	 if (m_pDazzleLayer) {
		 delete m_pDazzleLayer;
		 m_pDazzleLayer = NULL;
	 }

    // Was there a valid scene object?
    if (m_pCBackObjectScene)
    {
        // Free the scene object
        m_pCBackObjectScene->Release_Ref ();
        m_pCBackObjectScene = NULL;
    }

    // Was there a valid 2D camera?
    if (m_pC2DCamera)
    {
        // Free the camera object
        m_pC2DCamera->Release_Ref ();
        m_pC2DCamera = NULL;
    }

    // Was there a valid background camera?
    if (m_pCBackObjectCamera)
    {
        // Free the camera object
        m_pCBackObjectCamera->Release_Ref ();
        m_pCBackObjectCamera = NULL;
    }

    // Was there a valid background BMP?
    if (m_pCBackgroundBMP)
    {
        m_pCBackgroundBMP->Release_Ref ();
        m_pCBackgroundBMP = NULL;
    }

    // Was there a valid scene light?
    if (m_pCSceneLight)
    {
        m_pCSceneLight->Release_Ref ();
        m_pCSceneLight = NULL;
    }

    // Was there a valid display object?
    if (m_pCRenderObj)
    {
        // Free the currently displayed object
			SAFE_DELETE (m_pCAnimCombo);
			MEMBER_RELEASE (m_pCAnimation);
			MEMBER_RELEASE (m_pCRenderObj);
    }

    return ;
}

///////////////////////////////////////////////////////////////
//
//  OnNewDocument
//
///////////////////////////////////////////////////////////////
BOOL
CW3DViewDoc::OnNewDocument (void)
{
	if (!CDocument::OnNewDocument())
		return FALSE;
		
	_TheAssetMgr->Start_Tracking_Textures ();
	m_LoadList.Delete_All ();

	 m_bOneTimeReset = true;
    if (m_pCScene && m_pCRenderObj)
    {
        // Remove the currently displayed object from the scene
		  Remove_Object_From_Scene (m_pCRenderObj);
    }

	 if (m_pCScene)
	 {
		 // Remove all objects from the lineup.
		 m_pCScene->Clear_Lineup();

		 // Update the fog color.
		 m_pCScene->Set_Fog_Color(m_backgroundColor);
	 }

    if (m_pCRenderObj)
    {
			// Free the currently displayed object
			SAFE_DELETE (m_pCAnimCombo);
			MEMBER_RELEASE (m_pCAnimation);
			MEMBER_RELEASE (m_pCRenderObj);
    }

    CDataTreeView *pCDataTreeView = GetDataTreeView ();
    if (pCDataTreeView)
    {
        // Delete everything from the tree
        pCDataTreeView->GetTreeCtrl ().DeleteAllItems ();

        // Recreate the root nodes
        pCDataTreeView->CreateRootNodes ();
    }

    // Remove everything from the scene and the asset manager
	 CleanupResources ();
	 //WW3DAssetManager::Get_Instance ()->Release_All_Textures ();
    WW3DAssetManager::Get_Instance ()->Free_Assets ();
	 WW3DAssetManager::Get_Instance ()->Load_Procedural_Textures();

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// CW3DViewDoc
//
/////////////////////////////////////////////////////////////////////////////
void
CW3DViewDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}

	return ;
}

/////////////////////////////////////////////////////////////////////////////
// CW3DViewDoc diagnostics

#ifdef _DEBUG
void CW3DViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CW3DViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


///////////////////////////////////////////////////////////////
//
//  InitScene
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::InitScene (void)
{
	if (m_pCScene == NULL) {

		//
		//	Make sure the emitters don't remove themselves from the scene
		// when they are finished emitting...
		//
		ParticleEmitterClass::Set_Default_Remove_On_Complete (false);

		m_pCScene = new ViewerSceneClass;
		ASSERT (m_pCScene);
		if (m_pCScene != NULL) {

			// Set some default ambient lighting
			m_pCScene->Set_Ambient_Light (Vector3 (0.5F, 0.5F, 0.5F));

			// Set up the correct fog color.
			m_pCScene->Set_Fog_Color(GetBackgroundColor());

			// Create a new scene light
			m_pCSceneLight = new LightClass;
			ASSERT (m_pCSceneLight);

			if (m_pCSceneLight != NULL) {

				// Create some default light settings
				m_pCSceneLight->Set_Position (Vector3 (0, 5000, 3000));
				m_pCSceneLight->Set_Intensity (1.0F);
				m_pCSceneLight->Set_Force_Visible(true);
				m_pCSceneLight->Set_Flag (LightClass::NEAR_ATTENUATION, false);
				m_pCSceneLight->Set_Far_Attenuation_Range (1000000, 1000000);
				m_pCSceneLight->Set_Ambient(Vector3(0,0,0));
				m_pCSceneLight->Set_Diffuse (Vector3(1, 1, 1));
				m_pCSceneLight->Set_Specular (Vector3(1, 1, 1));

				// Add this light to the scene
				m_pCScene->Add_Render_Object (m_pCSceneLight);
			}
		}

		// Instantiate a new 2D scene
		m_pC2DScene = new SimpleSceneClass;
		ASSERT (m_pC2DScene);

		// Instantiate a new 2D cursor scene
		m_pCursorScene = new SimpleSceneClass;
		ASSERT (m_pCursorScene);

		Create_Cursor ();
		m_pCursorScene->Add_Render_Object (m_pCursor);


		m_pCBackObjectScene = new SimpleSceneClass;

		// Were we successful in instantiating the scene object?
		ASSERT (m_pCBackObjectScene);
		if (m_pCBackObjectScene) {

			// Set the default ambient light for the background object
			m_pCBackObjectScene->Set_Ambient_Light (Vector3 (0.5F, 0.5F, 0.5F));
		}

			
		// Create a new instance of the camera class to use
		// when rendering the background object
		m_pCBackObjectCamera = new CameraClass ();

		// Were we successful in creating the new instance?
		ASSERT (m_pCBackObjectCamera);
		if (m_pCBackObjectCamera) {

			// Set the default values for the new camera
			m_pCBackObjectCamera->Set_View_Plane (Vector2 (-1.00F, -1.00F), Vector2 (1.00F, 1.00F));
			m_pCBackObjectCamera->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));
			m_pCBackObjectCamera->Set_Clip_Planes (0.1F, 10.0F);
		}        

		// Create a new instance of the camera class to use
		// when rendering the background BMP
		m_pC2DCamera = new CameraClass ();

		// Were we successful in creating the new instance?
		ASSERT (m_pC2DCamera);		
		if (m_pC2DCamera) {

			// Set the default values for the new camera
			m_pC2DCamera->Set_View_Plane (Vector2 (-1.00F, -1.00F), Vector2 (1.00F, 1.00F));
			m_pC2DCamera->Set_Position (Vector3 (0.00F, 0.00F, 1.00F));
			m_pC2DCamera->Set_Clip_Planes (0.1F, 10.0F);
		}

		//
		// Read the texture paths from the registry
		//
		CString path1 = theApp.GetProfileString ("Config", "TexturePath1", "");
		CString path2 = theApp.GetProfileString ("Config", "TexturePath2", "");
		Set_Texture_Path1 (path1);
		Set_Texture_Path2 (path2);	

		// Construct a dazzle layer object 
		m_pDazzleLayer  = new DazzleLayerClass();
		DazzleRenderObjClass::Set_Current_Dazzle_Layer(m_pDazzleLayer);

		// Enable fog if appropriate.
		if (IsFogEnabled()) {
			m_pCScene->Set_Fog_Enable(true);
		}
	}

	Load_Camera_Settings ();
	m_IsInitialized = true;
	return ;
}


///////////////////////////////////////////////////////////////
//
//  OnOpenDocument
//
///////////////////////////////////////////////////////////////
BOOL
CW3DViewDoc::OnOpenDocument (LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	//
	//	Don't allow repaints while the load is going on
	//
	CGraphicView *current_view = ::Get_Graphic_View ();
	if (current_view != NULL) {
		current_view->Allow_Update (false);
	}

	//
	// Load the assets from this file into application
	//
	LoadAssetsFromFile (lpszPathName);

	//
	// Re-load the data list to include all new assets
	//
	CDataTreeView *data_view = GetDataTreeView ();
	if (data_view != NULL) {		
		data_view->LoadAssetsIntoTree ();
	}

	//
	//	Turn repainting back on...
	//
	if (current_view != NULL) {
		current_view->Allow_Update (true);
	}

	return TRUE;
}


///////////////////////////////////////////////////////////////
//
//  LoadAssetsFromFile
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::LoadAssetsFromFile (LPCTSTR lpszPathName) 
{
	if (m_pCScene == NULL) {
		InitScene ();
	}

	//
	//	Remember the last path we opened
	//
	m_LastPath = ::Strip_Filename_From_Path (lpszPathName);

	//
	//	Add this path to the load list
	//
	m_LoadList.Add (lpszPathName);
	
	//
	//	Don't allow repaints while the load is going on
	//
	CGraphicView *current_view = ::Get_Graphic_View ();
	if (current_view != NULL) {
		current_view->Allow_Update (false);
	}

	//
	// HACK HACK -- Force the current directory to be the directory
	//              the file is located in.
	//
	if (::strrchr (lpszPathName, '\\')) {
		CString stringTemp = lpszPathName;
		stringTemp = stringTemp.Left ((long)::strrchr (lpszPathName, '\\') - (long)lpszPathName);
		::SetCurrentDirectory (stringTemp);
		_TheSimpleFileFactory->Append_Sub_Directory(stringTemp);
	}

	LPCTSTR extension = ::strrchr (lpszPathName, '.');
	if (::lstrcmpi (extension, ".tga") == 0 || ::lstrcmpi (extension, ".dds") == 0) {
		
		// Load the texture file into the asset manager
		TextureClass *ptexture = WW3DAssetManager::Get_Instance()->Get_Texture (::Get_Filename_From_Path (lpszPathName));
		if (ptexture != NULL) {
			ptexture->Release_Ref();
		}

	} else {
		WW3DAssetManager::Get_Instance()->Load_3D_Assets (::Get_Filename_From_Path (lpszPathName));
	}

	//
	//	Turn repainting back on...
	//
	if (current_view != NULL) {
		current_view->Allow_Update (true);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Reload_Displayed_Object
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Reload_Displayed_Object (void)
{
	GetDataTreeView ()->Display_Asset ();
	//SAFE_ADD_REF (m_pCRenderObj);
	//DisplayObject (m_pCRenderObj, false, false);
	//SAFE_RELEASE_REF (m_pCRenderObj);
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Display_Emitter
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Display_Emitter
(
	ParticleEmitterClass *pemitter,
	bool use_global_reset_flag,
	bool allow_reset
)
{
	ASSERT (m_pCScene);

	// Data OK?
	if (m_pCScene != NULL) {

		// Lose the animation
		SAFE_DELETE (m_pCAnimCombo);
		MEMBER_RELEASE (m_pCAnimation);

			if (m_pCRenderObj != NULL) {
				
				// Remove this object from the scene
				Remove_Object_From_Scene (m_pCRenderObj);
				m_pCRenderObj->Release_Ref ();
				m_pCRenderObj = NULL;
			}
			m_pCScene->Clear_Lineup();

		// Do we have a new emitter to display?
		if (pemitter != NULL) {

			// Add the emitter to the scene
			pemitter->Set_Transform (Matrix3D (1));			
			MEMBER_ADD (m_pCRenderObj, pemitter);
			m_pCScene->Add_Render_Object (m_pCRenderObj);
			pemitter->Start ();

			CGraphicView *pCGraphicView = GetGraphicView ();
			if (pCGraphicView) {

				// Try to find a good view for the emitter
				if ((use_global_reset_flag && m_bAutoCameraReset) ||
					 ((use_global_reset_flag == false) && allow_reset) ||
					 m_bOneTimeReset) {
					pCGraphicView->Reset_Camera_To_Display_Emitter (*pemitter);
					m_bOneTimeReset = false;
				}
			}
		}
	}
	
	return ;
}


///////////////////////////////////////////////////////////////
//
//  DisplayObject
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::DisplayObject
(
	RenderObjClass *pCModel,
	bool use_global_reset_flag,
	bool allow_reset,
	bool add_ghost
)
{
    ASSERT (m_pCScene);

    // Data OK?
    if (m_pCScene)
    {
        // Lose the animation
		  SAFE_DELETE (m_pCAnimCombo);
		  MEMBER_RELEASE (m_pCAnimation);

        // Do we have an old object to remove from the scene?
		  if (add_ghost == false) {
			  if (m_pCRenderObj)
			  {
					// Remove this object from the scene
					Remove_Object_From_Scene (m_pCRenderObj);
					m_pCRenderObj->Release_Ref ();
					m_pCRenderObj = NULL;
			  }
		  }
		  m_pCScene->Clear_Lineup();

        // Do we have a new object to display?
        if (pCModel && (add_ghost == false))
        {
            // Reset the animation for this object
            pCModel->Set_Animation ();

            m_pCRenderObj = pCModel;
            m_pCRenderObj->Add_Ref ();
            m_pCRenderObj->Set_Transform (Matrix3D (1));

            // Add this object to the scene
				if (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_BITMAP2D) {
					m_pC2DScene->Add_Render_Object (m_pCRenderObj);
				} else {
					m_pCScene->Add_Render_Object (m_pCRenderObj);
				}

				// Reset the current lod to be the lowest possible LOD...
				if ((m_pCScene->Are_LODs_Switching ()) &&
					 (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_HLOD)) {
					((HLodClass *)m_pCRenderObj)->Set_LOD_Level (0);
				}

            CGraphicView *pCGraphicView = GetGraphicView ();
            if (pCGraphicView)
            {
                // Reset the camera to so the user can see
                // the whole object
                if ((use_global_reset_flag && m_bAutoCameraReset) ||
					     ((use_global_reset_flag == false) && allow_reset) ||
						  m_bOneTimeReset) {
						pCGraphicView->Reset_Camera_To_Display_Object (*m_pCRenderObj);
						m_bOneTimeReset = false;
					 }
            }
        }
		  else if (pCModel) {
            // Reset the animation for this object
            pCModel->Set_Animation ();

				RenderObjClass *m_pCRenderObj;

            m_pCRenderObj = pCModel;
            m_pCRenderObj->Add_Ref ();
            m_pCRenderObj->Set_Transform (Matrix3D (1));

            // Add this object to the scene
				if (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_BITMAP2D) {
					m_pC2DScene->Add_Render_Object (m_pCRenderObj);
				} else {
					m_pCScene->Clear_Lineup();
					m_pCScene->Add_Render_Object (m_pCRenderObj);
				}

				// Reset the current lod to be the lowest possible LOD...
				if ((m_pCScene->Are_LODs_Switching ()) &&
					 (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_HLOD)) {
					((HLodClass *)m_pCRenderObj)->Set_LOD_Level (0);
				}

            CGraphicView *pCGraphicView = GetGraphicView ();
            if (pCGraphicView)
            {
                // Reset the camera to so the user can see
                // the whole object
                if ((use_global_reset_flag && m_bAutoCameraReset) ||
					     ((use_global_reset_flag == false) && allow_reset) ||
						  m_bOneTimeReset) {
						pCGraphicView->Reset_Camera_To_Display_Object (*m_pCRenderObj);
						m_bOneTimeReset = false;
					 }
            }
		  }
    }

    return ;
}


///////////////////////////////////////////////////////////////
//
//  ResetAnimation
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::ResetAnimation (void)
{
	if (m_pCAnimation != NULL) {
		
		//
		// Reset the frame counter
		//
		m_CurrentFrame = 0;
		m_animTime = 0.00F;

		float frame_rate = m_pCAnimation->Get_Frame_Rate ();
		float anim_speed = ::Get_Graphic_View ()->GetAnimationSpeed ();

		//
		// Update the status bar on the main window
		//
		((CMainFrame *)::AfxGetMainWnd ())->UpdateFrameCount (	0,
																					m_pCAnimation->Get_Num_Frames () - 1,
																					frame_rate * anim_speed);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  StepAnimation
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::StepAnimation (int iFrameInc)
{
	if (m_pCRenderObj && m_pCAnimation) {
		int iTotalFrames = m_pCAnimation->Get_Num_Frames ();

		// Increment the frame
		m_CurrentFrame += iFrameInc;

		//
		// Wrap the animation
		//
		if (m_CurrentFrame >= iTotalFrames) {
			m_CurrentFrame = 0;
			m_animTime = 0.00F;
		}
		else if (m_CurrentFrame < 0) {
			m_CurrentFrame = iTotalFrames-1;
		}

		//
		// Update the status bar on the main window
		//
		float frame_rate = m_pCAnimation->Get_Frame_Rate ();
		float anim_speed = ::Get_Graphic_View ()->GetAnimationSpeed ();
		((CMainFrame *)::AfxGetMainWnd ())->UpdateFrameCount (m_CurrentFrame, iTotalFrames - 1, frame_rate * anim_speed);

		//
		// Update the animation frame
		//
		if (m_pCAnimCombo) {
			for (int i = 0; i < m_pCAnimCombo->Get_Num_Anims(); i++)
			{
			m_pCAnimCombo->Set_Frame(i, m_CurrentFrame);
			}

			m_pCRenderObj->Set_Animation (m_pCAnimCombo);
		} else {
			m_pCRenderObj->Set_Animation (m_pCAnimation, m_CurrentFrame);
		}

		Update_Camera ();
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  PlayAnimation
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::PlayAnimation
(
    RenderObjClass *pCModel,
    LPCTSTR pszAnimationName,
	 bool use_global_reset_flag,
	 bool allow_reset
)
{
    ASSERT (m_pCScene);
    ASSERT (pCModel);
    ASSERT (pszAnimationName);

    // Data OK?
    if (m_pCScene &&
        pCModel &&
        pszAnimationName)
    {
        // Display this hierarchy on the screen
        DisplayObject (pCModel);

        // Get an instance of the animation object
		  SAFE_DELETE (m_pCAnimCombo);
		  MEMBER_RELEASE (m_pCAnimation);
        m_pCAnimation = WW3DAssetManager::Get_Instance()->Get_HAnim (pszAnimationName);       
        ASSERT (m_pCAnimation);

        // Reset the frame counter
        m_CurrentFrame = 0;
        m_animTime = 0.00F;

        if (m_pCRenderObj)
        {
            // Update the animation frame
            m_pCRenderObj->Set_Animation (m_pCAnimation, 0);

            CGraphicView *pCGraphicView = GetGraphicView ();
            if (pCGraphicView)
            {
                // Reset the camera to so the user can see
                // the whole object
                if ((use_global_reset_flag && m_bAutoCameraReset) ||
					     ((use_global_reset_flag == false) && allow_reset) ||
						  m_bOneTimeReset) {
						pCGraphicView->Reset_Camera_To_Display_Object (*m_pCRenderObj);
						m_bOneTimeReset = false;
					 }

                AfxGetMainWnd ()->PostMessage (WM_COMMAND, MAKELPARAM (IDM_ANI_START, 0));
            }
        }

		  Update_Camera ();
		  Play_Animation_Sound ();
    }
        
    return ;
}


///////////////////////////////////////////////////////////////
//
//	Play_Animation_Sound
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Play_Animation_Sound (void)
{
	if (m_pCAnimation != NULL) {	  
	  CString animation_name = m_pCAnimation->Get_Name ();

	  //
	  // Play a sound with the animation
	  //
	  const char *separator = ::strchr (animation_name , '.');
	  if (separator != NULL) {
		  CString sound_filename = separator + 1;
		  sound_filename += ".wav";
		  ::PlaySound (NULL, NULL, SND_PURGE);
		  ::PlaySound (sound_filename, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//	PlayAnimation
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::PlayAnimation
(
    RenderObjClass *pCModel,
    HAnimComboClass *pCAnimCombo,
	 bool use_global_reset_flag,
	 bool allow_reset
)
{
    ASSERT (m_pCScene);
    ASSERT (pCModel);
    ASSERT (pCAnimCombo);

    // Data OK?
    if (m_pCScene &&
        pCModel &&
        pCAnimCombo)
    {
        // Display this hierarchy on the screen
        DisplayObject (pCModel);

        // Get an instance of the animation object
		  SAFE_DELETE (m_pCAnimCombo);
		  MEMBER_RELEASE (m_pCAnimation);
		  m_pCAnimCombo = pCAnimCombo;
		  m_pCAnimation = m_pCAnimCombo->Get_Motion(0);	// ref added by get_motion
        ASSERT (m_pCAnimation);

		  // It will be assumed that every animation in the m_pCAnimCombo
		  // has the same number of frames and has the same framerate as
		  // the first animation in the combo (m_pCAnimation).

        // Reset the frame counter
        m_CurrentFrame = 0;
        m_animTime = 0.00F;

        if (m_pCRenderObj)
        {
            // Update the animation frame
			  for (int i = 0; i < m_pCAnimCombo->Get_Num_Anims(); i++)
				  m_pCAnimCombo->Set_Frame(i, 0.0f);
            m_pCRenderObj->Set_Animation(m_pCAnimCombo);

            CGraphicView *pCGraphicView = GetGraphicView ();
            if (pCGraphicView)
            {
                // Reset the camera to so the user can see
                // the whole object
                if ((use_global_reset_flag && m_bAutoCameraReset) ||
					     ((use_global_reset_flag == false) && allow_reset) ||
						  m_bOneTimeReset) {
						pCGraphicView->Reset_Camera_To_Display_Object (*m_pCRenderObj);
						m_bOneTimeReset = false;
					 }

                AfxGetMainWnd ()->PostMessage (WM_COMMAND, MAKELPARAM (IDM_ANI_START, 0));
            }
        }

		  Update_Camera ();
		  Play_Animation_Sound ();
    }
        
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Get_Camera_Transform
//
////////////////////////////////////////////////////////////////////////////
bool
Get_Camera_Transform (RenderObjClass *render_obj, Matrix3D &tm) 
{
	bool retval = false;

	if (render_obj != NULL) {
		for (int index = 0; (index < render_obj->Get_Num_Sub_Objects ()) && !retval; index ++) {
			RenderObjClass *psub_obj = render_obj->Get_Sub_Object (index);
			if (psub_obj != NULL) {
				retval = Get_Camera_Transform (psub_obj, tm);
			}
			MEMBER_RELEASE (psub_obj);
		}

		if (!retval) {
			int index = render_obj->Get_Bone_Index ("CAMERA");
			if (index > 0) {
				tm = render_obj->Get_Bone_Transform (index);
				retval = true;
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Update_Camera
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Update_Camera (void)
{
	// Should we update the camera's position as well?
	if (m_bAnimateCamera && m_pCRenderObj != NULL) {			

		Matrix3D transform (1);
		if (Get_Camera_Transform (m_pCRenderObj, transform)) {

			// Convert the bone's transform into a camera transform
			//Matrix3D	transform = m_pCRenderObj->Get_Bone_Transform (index);												
			Matrix3D cam_transform (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
#ifdef ALLOW_TEMPORARIES
			Matrix3D new_transform = transform * cam_transform;
#else
			Matrix3D new_transform;
			new_transform.mul(transform, cam_transform);
#endif

			// Pass the new transform onto the camera
			CameraClass *pcamera = GetGraphicView()->GetCamera ();
			pcamera->Set_Transform (new_transform);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  UpdateFrame
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::UpdateFrame (float relativeTimeSlice)
{
	if (m_pCRenderObj && m_pCAnimation) {

		int total_frames = m_pCAnimation->Get_Num_Frames ();
		float frame_rate = m_pCAnimation->Get_Frame_Rate ();
		float loop_time = ((float)(total_frames-1)) / frame_rate;

		//
		// Update the total elapsed animation time
		//
		m_animTime += relativeTimeSlice;        		  
		if (m_animTime > loop_time) {
			m_animTime -= loop_time;
		}

		// Adjust the current frame counter
		m_CurrentFrame = (frame_rate * m_animTime);

		//
		// Update the status bar on the main window
		//
		float anim_speed = ::Get_Graphic_View ()->GetAnimationSpeed ();
		((CMainFrame *)::AfxGetMainWnd ())->UpdateFrameCount (m_CurrentFrame, total_frames - 1, frame_rate * anim_speed);

		if (m_pCAnimCombo) {

			//
			// Update the animation frame for all anims in the combo.
			//
			for (int i = 0; i < m_pCAnimCombo->Get_Num_Anims(); i++) {
				m_pCAnimCombo->Set_Frame(i, m_CurrentFrame);
			}

			m_pCRenderObj->Set_Animation (m_pCAnimCombo);
		} else if (m_bAnimBlend) {
			m_pCRenderObj->Set_Animation (m_pCAnimation, m_CurrentFrame);
		} else {
			m_pCRenderObj->Set_Animation (m_pCAnimation, (int)m_CurrentFrame);
		}

		Update_Camera ();
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  GetDataTreeView
//
///////////////////////////////////////////////////////////////
CDataTreeView *
CW3DViewDoc::GetDataTreeView (void)
{
    CDataTreeView *pCDataTreeView = NULL;

    // Get a pointer to the main window
    CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
    if (pCMainWnd)
    {
        // Get the pane from the main window
        pCDataTreeView = (CDataTreeView *)pCMainWnd->GetPane (0, 0);
    }

    // Return a pointer to the tree view
    return pCDataTreeView;
}


///////////////////////////////////////////////////////////////
//
//  GetGraphicView
//
///////////////////////////////////////////////////////////////
CGraphicView *
CW3DViewDoc::GetGraphicView (void)
{
    CGraphicView *pCGrephicView = NULL;

    // Get a pointer to the main window
    CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
    if (pCMainWnd)
    {
        // Get the pane from the main window
        pCGrephicView = (CGraphicView *)pCMainWnd->GetPane (0, 1);
    }

    // Return a pointer to the graphic view
    return pCGrephicView;
}


///////////////////////////////////////////////////////////////
//
//  GenerateLOD
//
///////////////////////////////////////////////////////////////
HLodPrototypeClass *
CW3DViewDoc::GenerateLOD
(
	LPCTSTR pszLODBaseName,
	LOD_NAMING_TYPE type
)
{
	// Assume failure
	HLodPrototypeClass *plod_prototype = NULL;

	// Get an iterator from the asset manager that we can
	// use to enumerate the currently loaded assets
	RenderObjIterator *pObjEnum = WW3DAssetManager::Get_Instance()->Create_Render_Obj_Iterator ();    
	ASSERT (pObjEnum);
	if (pObjEnum)
	{
		int lod_count = 0;
		int iStartingIndex = 0xFFFF;
		char starting_char = 'Z';

		// Loop through all the assets in the manager looking for hierarchies that match the
		// naming convention
		for (pObjEnum->First ();
			  pObjEnum->Is_Done () == FALSE;
			  pObjEnum->Next ())	{
			LPCTSTR pszItemName = pObjEnum->Current_Item_Name ();

			// Is this a hierarchy?
			if (WW3DAssetManager::Get_Instance()->Render_Obj_Exists (pszItemName) &&
				 (pObjEnum->Current_Item_Class_ID () == RenderObjClass::CLASSID_HLOD)) {

				if (Is_Model_Part_of_LOD (pszItemName, pszLODBaseName, type)) {
					lod_count ++;
					if (type == TYPE_COMMANDO) {
						iStartingIndex = min (iStartingIndex, ::atoi (&pszItemName[::lstrlen (pszItemName)-1]));
					} else {
						starting_char = min (starting_char, (char)::toupper (pszItemName[::lstrlen (pszItemName)-1]));
					}
				}
			}
		}

		// Create an array of LOD models
		RenderObjClass **plod_array = new RenderObjClass *[lod_count];
		ASSERT (plod_array != NULL);
		if (plod_array != NULL) {

			// Loop through all the levels-of-detail and add them to our array
			for (int lod_index = 0; lod_index < lod_count; lod_index ++) {
				CString lod_name;

				if (type == TYPE_COMMANDO) {
					lod_name.Format ("%sL%d", pszLODBaseName, iStartingIndex + lod_index);
				} else {
					lod_name.Format ("%s%c", pszLODBaseName, starting_char++);
				}

				// Add this lod to the array in reverse order (LOD 0 is the lowest-level LOD)
				plod_array[lod_count - (lod_index+1)] = WW3DAssetManager::Get_Instance ()->Create_Render_Obj (lod_name);
			}

			// Create a new HLod prototype from the provided lod array
			HLodClass *pnew_lod = new HLodClass (pszLODBaseName, plod_array, lod_count);
			HLodDefClass *pdefinition = new HLodDefClass (*pnew_lod);
			plod_prototype = new HLodPrototypeClass (pdefinition);

			MEMBER_RELEASE (pnew_lod);

			// Loop through all the LOD definitions and free their names
			for (lod_index = 0; lod_index < lod_count; lod_index ++) {
				MEMBER_RELEASE (plod_array[lod_index]);
			}

			// Free the LOD definition array
			SAFE_DELETE_ARRAY (plod_array);
		}

		SAFE_DELETE (pObjEnum);
	}

	// Return the LOD prototype to the caller
	return plod_prototype;
}


///////////////////////////////////////////////////////////////
//
//  SetBackgroundBMP
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::SetBackgroundBMP (LPCTSTR pszBackgroundBMP)
{
    ASSERT (m_pC2DScene);
    if (m_pC2DScene)
    {
		  // Remove the old background BMP if there was one.
        if (m_pCBackgroundBMP)
        {
            // Remove the background BMP from the scene
            // and release its pointer
				m_pCBackgroundBMP->Remove ();
            m_pCBackgroundBMP->Release_Ref ();
            m_pCBackgroundBMP = NULL;
        }

        // Is this a new background BMP?
        if (pszBackgroundBMP &&
            (m_stringBackgroundBMP.CompareNoCase (pszBackgroundBMP) != 0))
        {
				// Create a new instance of the BMP object to use
            m_pCBackgroundBMP = new Bitmap2DObjClass (pszBackgroundBMP, 0.5F, 0.5F, TRUE, FALSE);
				        
            // Were we successful in creating the bitmap object?
            ASSERT (m_pCBackgroundBMP);
            if (m_pCBackgroundBMP)
            {
                // Add the object to the scene
					 m_pC2DScene->Add_Render_Object (m_pCBackgroundBMP);
            }
        }

        // Remember what our current background BMP is
        m_stringBackgroundBMP = pszBackgroundBMP;
    }

    return ;
}


///////////////////////////////////////////////////////////////
//
//  LoadSettings
//
///////////////////////////////////////////////////////////////
BOOL
CW3DViewDoc::LoadSettings (LPCTSTR filename)
{
	// Assume failure
	BOOL bReturn = FALSE;

	// Params OK?
	ASSERT (filename != NULL);
	if (filename != NULL) {

		// Open the INI file
		FileClass * pini_file = _TheFileFactory->Get_File (filename);
		INIClass ini_obj (*pini_file);			
		
		CGraphicView *graphic_view = GetGraphicView ();
		Vector3 color;

		//
		// Ambient light color
		//
		if (ini_obj.Is_Present ("Settings", "AmbientLightR") &&
			 ini_obj.Is_Present ("Settings", "AmbientLightG") &&
			 ini_obj.Is_Present ("Settings", "AmbientLightB")) {
			
			// Read the settings from,the INI file
			color.X = ini_obj.Get_Float ("Settings", "AmbientLightR");
			color.Y = ini_obj.Get_Float ("Settings", "AmbientLightG");
			color.Z = ini_obj.Get_Float ("Settings", "AmbientLightB");

			// Pass the ambient light color onto the scene
			m_pCScene->Set_Ambient_Light (color);
		}

		//
		// Scene light color
		//
		if (ini_obj.Is_Present ("Settings", "SceneLightR") &&
			 ini_obj.Is_Present ("Settings", "SceneLightG") &&
			 ini_obj.Is_Present ("Settings", "SceneLightB")) {
			
			// Read the settings from,the INI file
			color.X = ini_obj.Get_Float ("Settings", "SceneLightR");
			color.Y = ini_obj.Get_Float ("Settings", "SceneLightG");
			color.Z = ini_obj.Get_Float ("Settings", "SceneLightB");

			// Pass the scene light color onto the light
         m_pCSceneLight->Set_Diffuse (color);
         m_pCSceneLight->Set_Specular (color);
		}

		//
		// Scene light orientation
		//
		if (ini_obj.Is_Present ("Settings", "SceneLightX") &&
			 ini_obj.Is_Present ("Settings", "SceneLightY") &&
			 ini_obj.Is_Present ("Settings", "SceneLightZ") &&
			 ini_obj.Is_Present ("Settings", "SceneLightW")) {
			
			// Read the settings from,the INI file
			Quaternion orientation;
			orientation.X = ini_obj.Get_Float ("Settings", "SceneLightX");
			orientation.Y = ini_obj.Get_Float ("Settings", "SceneLightY");
			orientation.Z = ini_obj.Get_Float ("Settings", "SceneLightZ");
			orientation.W = ini_obj.Get_Float ("Settings", "SceneLightW");

			// Get the light's transform and inverse transform			
			Vector3 obj_pos = graphic_view->Get_Object_Center ();
			Vector3 light_pos = m_pCSceneLight->Get_Position ();
			float distance = (light_pos - obj_pos).Length ();

			// Move the light to the object's center, perform the rotation, and
			Matrix3D light_tm (1);
			light_tm.Set_Translation (obj_pos);

#ifdef ALLOW_TEMPORARIES
			Matrix3D::Multiply (light_tm, Build_Matrix3D (orientation), &light_tm);
#else
			Matrix3D orientation_matrix;
			Matrix3D::Multiply (light_tm, Build_Matrix3D (orientation, orientation_matrix), &light_tm);
#endif
			light_tm.Translate (Vector3 (0, 0, distance));
			
			// Pass the new transform onto the light
			RenderObjClass *pLightMesh = graphic_view->Get_Light_Mesh ();
			pLightMesh->Set_Transform (light_tm);
			m_pCSceneLight->Set_Transform (light_tm);
		}

		//
		// Scene light distance, intensity, and attenuation
		//
		if (ini_obj.Is_Present ("Settings", "SceneLightDistance") &&
			 ini_obj.Is_Present ("Settings", "SceneLightIntensity") &&
			 ini_obj.Is_Present ("Settings", "SceneLightAttenStart") &&
			 ini_obj.Is_Present ("Settings", "SceneLightAttenEnd") &&
			 ini_obj.Is_Present ("Settings", "SceneLightAttenOn")) {

			// Read the settings from,the INI file
			float distance = ini_obj.Get_Float ("Settings", "SceneLightDistance");
			float intensity = ini_obj.Get_Float ("Settings", "SceneLightIntensity");
			float start = ini_obj.Get_Float ("Settings", "SceneLightAttenStart");
			float end = ini_obj.Get_Float ("Settings", "SceneLightAttenEnd");
			int atten_on = ini_obj.Get_Int ("Settings", "SceneLightAttenOn");
			
			// Pass the intensity and attenuation settings onto the light
			m_pCSceneLight->Set_Intensity (intensity);
			m_pCSceneLight->Set_Far_Attenuation_Range (start, end);
			m_pCSceneLight->Set_Flag (LightClass::FAR_ATTENUATION, (atten_on == 1));

			// Get the position of the light and the displayed object
			Vector3 obj_pos = graphic_view->Get_Object_Center ();
			Vector3 light_pos = m_pCSceneLight->Get_Position ();			
			Vector3 new_pos = (light_pos - obj_pos);
			new_pos.Normalize ();
			new_pos = new_pos * distance;

			// Pass the new position onto the light
			RenderObjClass *pLightMesh = graphic_view->Get_Light_Mesh ();
			pLightMesh->Set_Position (new_pos);
			m_pCSceneLight->Set_Position (new_pos);			
		}

		//
		// Background color
		//
		if (ini_obj.Is_Present ("Settings", "BackgroundR") &&
			 ini_obj.Is_Present ("Settings", "BackgroundG") &&
			 ini_obj.Is_Present ("Settings", "BackgroundB")) {
			
			// Read the settings from,the INI file
			color.X = ini_obj.Get_Float ("Settings", "BackgroundR");
			color.Y = ini_obj.Get_Float ("Settings", "BackgroundG");
			color.Z = ini_obj.Get_Float ("Settings", "BackgroundB");

			// Record the background color for later use
			SetBackgroundColor (color);
		} 

		//
		// Background bitmap
		//
		if (ini_obj.Is_Present ("Settings", "BackgroundBMP")) {
			
			// Load this background BMP
			TCHAR bmp_filename[MAX_PATH];
			ini_obj.Get_String ("Settings", "BackgroundBMP", "", bmp_filename, sizeof (bmp_filename));
			SetBackgroundBMP (bmp_filename);
		}

		//
		// Fog Settings
		//
		if (ini_obj.Is_Present ("Settings", "FogEnabled")) {
			bool enable = ini_obj.Get_Bool ("Settings", "FogEnabled");
			EnableFog(enable);
		}

      // Close the INI file
		_TheFileFactory->Return_File (pini_file);
	}

	// Return the TRUE/FALSE result code
	return bReturn;
}


///////////////////////////////////////////////////////////////
//
//  SaveSettings
//
///////////////////////////////////////////////////////////////
BOOL
CW3DViewDoc::SaveSettings
(
    LPCTSTR pszFilename,
    DWORD dwSettingsMask
)
{
    // Assume failure
    BOOL bReturn = FALSE;
    ASSERT (pszFilename);
    ASSERT (dwSettingsMask != 0L);
    ASSERT (m_pCScene);    

    // Is the filename OK?
    HANDLE hFile = ::CreateFile (pszFilename,
                                 0,
                                 0,
                                 NULL,
                                 OPEN_ALWAYS,
                                 0L,
                                 NULL);

    ASSERT (hFile != NULL);
    if (hFile == NULL)
    {
        // Invalid file, let the user know
        ::AfxGetMainWnd ()->MessageBox ("Unable to open file for writing.  Please select another filename.", "File Error", MB_ICONERROR | MB_OK);
    }
    else if (pszFilename &&
             (dwSettingsMask != 0L) &&
             (m_pCScene != NULL))
    {
        CString stringCompleteFilename = pszFilename;

        // Does this filename contain a path?
        if (::strrchr (pszFilename, '\\') == NULL)
        {
            // Add the current directories path to the filename
            TCHAR szPath[MAX_PATH] = { 0 };
            ::GetCurrentDirectory (sizeof (szPath), szPath);

            if (szPath[::lstrlen (szPath)-1] != '\\')
            {
                // Ensure the path is directory delimited
                ::strcat (szPath, "\\");
            }

            // Prepend the filename with its new path
            stringCompleteFilename = CString (szPath) + stringCompleteFilename;
        }

        // Should we save light settings?
        if (dwSettingsMask & SAVE_SETTINGS_LIGHT)
        {
            Vector3 colorSettings = m_pCScene->Get_Ambient_Light ();
        
            // Write the 'Red' component out to the file
            CString stringValue;
            stringValue.Format ("%f", colorSettings.X);
            ::WritePrivateProfileString ("Settings",
                                         "AmbientLightR",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the 'Green' component out to the file
            stringValue.Format ("%f", colorSettings.Y);
            ::WritePrivateProfileString ("Settings",
                                         "AmbientLightG",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the 'Blue' component out to the file
            stringValue.Format ("%f", colorSettings.Z);
            ::WritePrivateProfileString ("Settings",
                                         "AmbientLightB",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

             m_pCSceneLight->Get_Diffuse (&colorSettings);
        
            // Write the 'Red' component out to the file
            stringValue.Format ("%f", colorSettings.X);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightR",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the 'Green' component out to the file
            stringValue.Format ("%f", colorSettings.Y);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightG",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the 'Blue' component out to the file
            stringValue.Format ("%f", colorSettings.Z);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightB",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

				Matrix3D transform = m_pCSceneLight->Get_Transform ();
				Quaternion orientation = ::Build_Quaternion (transform);
        
            // Write the x-position out to the file
            stringValue.Format ("%f", orientation.X);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightX",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the y-position out to the file
            stringValue.Format ("%f", orientation.Y);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightY",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the z-position out to the file
            stringValue.Format ("%f", orientation.Z);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightZ",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the w-position out to the file
            stringValue.Format ("%f", orientation.W);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightW",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

				// Get the light's transform and inverse transform
				CGraphicView *pCGraphicView = GetGraphicView ();
				Vector3 obj_pos = pCGraphicView->Get_Object_Center ();
				Vector3 light_pos = m_pCSceneLight->Get_Position ();
				float distance = (light_pos - obj_pos).Length ();
        
            // Write the distance out to the file
            stringValue.Format ("%f", distance);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightDistance",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the intensity out to the file
				float intensity = m_pCSceneLight->Get_Intensity ();
            stringValue.Format ("%f", intensity);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightIntensity",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the start attenuation out to the file
				double start = 0;
				double end = 0;
				m_pCSceneLight->Get_Far_Attenuation_Range (start, end);
            stringValue.Format ("%f", start);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightAttenStart",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the end attenuation out to the file
            stringValue.Format ("%f", end);
            ::WritePrivateProfileString ("Settings",
                                         "SceneLightAttenEnd",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the end attenuation out to the file
				BOOL atten_on = m_pCSceneLight->Get_Flag (LightClass::FAR_ATTENUATION);
				stringValue.Format ("%d", atten_on);
            ::WritePrivateProfileString ("Settings",
													  "SceneLightAttenOn",
													  stringValue,
													  (LPCTSTR)stringCompleteFilename);
        }

        // Should we save background settings?
        if (dwSettingsMask & SAVE_SETTINGS_BACK)
        {
            // Write the 'Red' component out to the file
            CString stringValue;
            stringValue.Format ("%f", m_backgroundColor.X);
            ::WritePrivateProfileString ("Settings",
                                         "BackgroundR",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the 'Green' component out to the file
            stringValue.Format ("%f", m_backgroundColor.Y);
            ::WritePrivateProfileString ("Settings",
                                         "BackgroundG",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the 'Blue' component out to the file
            stringValue.Format ("%f", m_backgroundColor.Z);
            ::WritePrivateProfileString ("Settings",
                                         "BackgroundB",
                                         stringValue,
                                         (LPCTSTR)stringCompleteFilename);

            // Write the BMP filename out to the profile
            ::WritePrivateProfileString ("Settings",
                                         "BackgroundBMP",
                                         (LPCTSTR)m_stringBackgroundBMP,
                                         (LPCTSTR)stringCompleteFilename);

				// Write the fog settings out to the file.
				::WritePrivateProfileString("Settings",
													 "FogEnabled",
													 IsFogEnabled() ? "true" : "false",
													 (LPCTSTR)stringCompleteFilename);
        }

        // Should we save camera settings?
        if (dwSettingsMask & SAVE_SETTINGS_CAMERA)
        {
        }

        // Success!
        bReturn = TRUE;
    }
    
    // Return the TRUE/FALSE result code
    return bReturn;
}


///////////////////////////////////////////////////////////////
//
//  Save_Selected_LOD
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Selected_LOD (void)
{
	// Assume failure
	bool retval = false;

	// Is this an emitter?
	if ((m_pCRenderObj != NULL) &&
		 m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_HLOD) {
		
		// Build the default filename from the name of the LOD
		CString default_filename = GetDataTreeView ()->GetCurrentSelectionName ();
		default_filename += ".w3d";

		RestrictedFileDialogClass dialog (FALSE,
												  ".w3d",
												  (LPCTSTR)default_filename,
												  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
												  "Westwood 3D Files (*.w3d)|*.~xyzabc||",
												  AfxGetMainWnd ());

		// Ask the user what filename they wish to save as.
		dialog.m_ofn.lpstrTitle = "Export LOD";
		if (dialog.DoModal () == IDOK) {
			
			// Save the LOD to the requested file
			retval = Save_Current_LOD (dialog.GetPathName ());
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Current_LOD
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Current_LOD (const CString &filename)
{
	// Assume failure
	bool retval = false;

	// Get the prototype for this aggregate
	HLodPrototypeClass *proto = NULL;
	proto = (HLodPrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (m_pCRenderObj->Get_Name ());
	ASSERT (proto != NULL);
	if (proto != NULL) {

		// Get the definition from the prototype
		HLodDefClass *pdefinition = proto->Get_Definition ();
		ASSERT (pdefinition != NULL);
		if (pdefinition != NULL) {

			// Get a file object for the new file
			FileClass *pfile = _TheFileFactory->Get_File (filename);
			if (pfile) {

				// Open the file for writing
				pfile->Open (FileClass::WRITE);

				// Save the definition to the file
				ChunkSaveClass save_chunk (pfile);
				retval = (pdefinition->Save (save_chunk) == WW3D_ERROR_OK);

				// Close the file
				pfile->Close ();
				_TheFileFactory->Return_File (pfile);
			}
		}
	}

	// Return the true/false result code
	return retval;


}


///////////////////////////////////////////////////////////////
//
//  SetBackgroundObject
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::SetBackgroundObject (LPCTSTR pszBackgroundObjectName)
{
    // State valid?
    ASSERT (m_pCBackObjectScene);
    ASSERT (m_pCBackObjectCamera);
    if (m_pCBackObjectScene && m_pCBackObjectCamera)
    {
        // Did we have an old background object we needed to remove?
        if (m_pCBackgroundObject)
        {
            // Remove the object from the scene
				m_pCBackgroundObject->Remove ();

            // Free the object
            m_pCBackgroundObject->Release_Ref ();
            m_pCBackgroundObject = NULL;            
        }

        if (pszBackgroundObjectName)
        {            
            // Create a new instance of the render object to use as the background
            m_pCBackgroundObject = WW3DAssetManager::Get_Instance()->Create_Render_Obj (pszBackgroundObjectName);
            
            ASSERT (m_pCBackgroundObject);
            if (m_pCBackgroundObject)
            {
                // Place the object at world center
                m_pCBackgroundObject->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));
                
                // Calculate the depth the camera should be at by the objects bouding sphere
                float cameraDepth = m_pCBackgroundObject->Get_Bounding_Sphere ().Radius * 4.0F;
                
                CGraphicView *pCGraphicView = GetGraphicView ();
                if (pCGraphicView)
                {
                    // Set the camera's rotation
                    m_pCBackObjectCamera->Set_Transform (pCGraphicView->GetCamera ()->Get_Transform ());
                }

                // Set the camera's position and depth                
                m_pCBackObjectCamera->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));
                //m_pCBackObjectCamera->Set_Depth (cameraDepth);
                m_pCBackObjectCamera->Set_Clip_Planes (1, cameraDepth);
                
                // Add the background object to the scene
					 m_pCBackObjectScene->Add_Render_Object (m_pCBackgroundObject);
            }
        }

        // Remember this for later...
        m_stringBackgroundObject = pszBackgroundObjectName;
    }

    return ;
}


///////////////////////////////////////////////////////////////
//
//  Remove_Object_From_Scene
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Remove_Object_From_Scene (RenderObjClass *prender_obj)
{
	// If the render object is NULL, then remove the current render object
	if (prender_obj == NULL) {
		prender_obj = m_pCRenderObj;
	}

	// Recursively remove objects from the scene (to make sure we get all particle buffers)
	//for (int index = 0; index < prender_obj->Get_Num_Sub_Objects (); index ++) {
	while (prender_obj->Get_Num_Sub_Objects () > 0) {
		RenderObjClass *psub_obj = prender_obj->Get_Sub_Object (0);
		if (psub_obj != NULL) {
			Remove_Object_From_Scene (psub_obj);
		}
		MEMBER_RELEASE (psub_obj);
	}

	// If this is an emitter, then remove its buffer
	if ((prender_obj != NULL) &&
		 prender_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
		
		// Attempt to remove this emitter's buffer
		((ParticleEmitterClass *)prender_obj)->Stop ();
		((ParticleEmitterClass *)prender_obj)->Remove_Buffer_From_Scene ();
		((ParticleEmitterClass *)prender_obj)->Buffer_Scene_Not_Needed ();
	}

	// Remove the render object from the scene (if we have a valid scene)
	if (m_pCScene != NULL) {
		prender_obj->Remove ();
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Save_Selected_Primitive
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Selected_Primitive (void)
{
	// Assume failure
	bool retval = false;

	// Is this an emitter?
	if ((m_pCRenderObj != NULL) &&
		 (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_SPHERE ||
		  m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_RING)) {
		
		// Build the default filename from the name of the emitter
		CString default_filename = GetDataTreeView ()->GetCurrentSelectionName ();
		default_filename += ".w3d";

		RestrictedFileDialogClass dialog (FALSE,
								  ".w3d",
								  (LPCTSTR)default_filename,
								  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								  "Westwood 3D Files (*.w3d)|*.~xyzabc||",
								  AfxGetMainWnd ());

		if (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_SPHERE) {
			dialog.m_ofn.lpstrTitle = "Export Sphere";
		} else {
			dialog.m_ofn.lpstrTitle = "Export Ring";
		}

		// Ask the user what filename they wish to save as.		
		if (dialog.DoModal () == IDOK) {
			
			// Save the emitter to the requested file
			if (m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_SPHERE) {
				retval = Save_Current_Sphere (dialog.GetPathName ());
			} else {
				retval = Save_Current_Ring (dialog.GetPathName ());
			}
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Current_Sphere
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Current_Sphere (const CString &filename)
{
	// Assume failure
	bool retval = false;

	//
	// Get the prototype for this object
	//
	SpherePrototypeClass *proto = NULL;
	proto = (SpherePrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (m_pCRenderObj->Get_Name ());
	ASSERT (proto != NULL);
	if (proto != NULL) {

		//
		// Get a file object for the new file
		//
		FileClass *file = _TheFileFactory->Get_File (filename);
		if (file) {

			// Open the file for writing
			file->Open (FileClass::WRITE);

			// Save the definition to the file
			ChunkSaveClass csave (file);
			proto->Save (csave);
			retval = true;

			// Close the file
			file->Close ();
			_TheFileFactory->Return_File (file);
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Current_Ring
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Current_Ring (const CString &filename)
{
	// Assume failure
	bool retval = false;

	//
	// Get the prototype for this object
	//
	RingPrototypeClass *proto = NULL;
	proto = (RingPrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (m_pCRenderObj->Get_Name ());
	ASSERT (proto != NULL);
	if (proto != NULL) {

		//
		// Get a file object for the new file
		//
		FileClass *file = _TheFileFactory->Get_File (filename);
		if (file) {

			// Open the file for writing
			file->Open (FileClass::WRITE);

			// Save the definition to the file
			ChunkSaveClass csave (file);
			proto->Save (csave);
			retval = true;

			// Close the file
			file->Close ();
			_TheFileFactory->Return_File (file);
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Selected_Emitter
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Selected_Emitter (void)
{
	// Assume failure
	bool retval = false;

	// Is this an emitter?
	if ((m_pCRenderObj != NULL) &&
		 m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
		
		// Build the default filename from the name of the emitter
		CString default_filename = GetDataTreeView ()->GetCurrentSelectionName ();
		default_filename += ".w3d";

		RestrictedFileDialogClass dialog (FALSE,
								  ".w3d",
								  (LPCTSTR)default_filename,
								  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								  "Westwood 3D Files (*.w3d)|*.~xyzabc||",
								  AfxGetMainWnd ());

		// Ask the user what filename they wish to save as.
		dialog.m_ofn.lpstrTitle = "Export Emitter";
		if (dialog.DoModal () == IDOK) {
			
			// Save the emitter to the requested file
			retval = Save_Current_Emitter (dialog.GetPathName ());
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Current_Emitter
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Current_Emitter (const CString &filename)
{
	// Assume failure
	bool retval = false;
	// Get the prototype for this aggregate
	ParticleEmitterPrototypeClass *proto = NULL;
	proto = (ParticleEmitterPrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (m_pCRenderObj->Get_Name ());
	ASSERT (proto != NULL);
	if (proto != NULL) {

		// Get the definition from the prototype
		ParticleEmitterDefClass *pdefinition = proto->Get_Definition ();
		ASSERT (pdefinition != NULL);
		if (pdefinition != NULL) {

			// Get a file object for the new file
			FileClass *pfile = _TheFileFactory->Get_File (filename);
			if (pfile) {

				// Open the file for writing
				pfile->Open (FileClass::WRITE);

				// Save the definition to the file
				ChunkSaveClass save_chunk (pfile);
				retval = (pdefinition->Save_W3D (save_chunk) == WW3D_ERROR_OK);

				// Close the file
				pfile->Close ();
				_TheFileFactory->Return_File (pfile);
			}
		}
	}
	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Selected_Sound_Object
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Selected_Sound_Object (void)
{
	bool retval = false;

	//
	// Is this a sound render object?
	//
	if ((m_pCRenderObj != NULL) &&
		 m_pCRenderObj->Class_ID () == RenderObjClass::CLASSID_SOUND)
	{		
		//
		// Build the default filename from the name of the emitter
		//
		CString default_filename = GetDataTreeView ()->GetCurrentSelectionName ();
		default_filename += ".w3d";

		RestrictedFileDialogClass dialog (FALSE,
								  ".w3d",
								  (LPCTSTR)default_filename,
								  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								  "Westwood 3D Files (*.w3d)|*.~xyzabc||",
								  AfxGetMainWnd ());

		//
		// Ask the user what filename they wish to save as.
		//
		dialog.m_ofn.lpstrTitle = "Export Sound Object";
		if (dialog.DoModal () == IDOK) {
			
			//
			// Save the sound object to the requested file
			//
			retval = Save_Current_Sound_Object (dialog.GetPathName ());
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Current_Sound_Object
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Current_Sound_Object (const CString &filename)
{
	bool retval = false;

	//
	// Get the prototype for this sound object
	//
	SoundRenderObjPrototypeClass *proto = NULL;
	proto = (SoundRenderObjPrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (m_pCRenderObj->Get_Name ());
	
	ASSERT (proto != NULL);
	if (proto != NULL) {

		//
		// Get the definition from the prototype
		//
		SoundRenderObjDefClass *definition = proto->Peek_Definition ();
		ASSERT (definition != NULL);
		if (definition != NULL) {

			//
			// Get a file object for the new file
			//
			FileClass *file = _TheFileFactory->Get_File (filename);
			if (file) {

				//
				// Open the file for writing
				//
				file->Open (FileClass::WRITE);

				//
				// Save the definition to the file
				//
				ChunkSaveClass csave (file);
				retval = (definition->Save_W3D (csave) == WW3D_ERROR_OK);

				//
				// Close the file
				//
				file->Close ();
				_TheFileFactory->Return_File (file);
			}
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Auto_Assign_Bones
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Auto_Assign_Bones (void)
{
	if (m_pCRenderObj != NULL) {
		bool bupdate_prototype = false;

		// Loop through all the bones in this render object
		int bone_count = m_pCRenderObj->Get_Num_Bones ();
		for (int index = 0; index < bone_count; index ++) {
			const char *pbone_name = m_pCRenderObj->Get_Bone_Name (index);
			
			// Attempt to find a render object with the same name as this bone
			if (WW3DAssetManager::Get_Instance ()->Render_Obj_Exists (pbone_name)) {
				
				// Add this render object to the bone
				RenderObjClass *prender_obj = WW3DAssetManager::Get_Instance ()->Create_Render_Obj (pbone_name);
				m_pCRenderObj->Add_Sub_Object_To_Bone (prender_obj, index);
				MEMBER_RELEASE (prender_obj);
				bupdate_prototype = true;
			}
		}

		if (bupdate_prototype) {
			Update_Aggregate_Prototype (*m_pCRenderObj);
		}
	}
		
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Save_Selected_Aggregate
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Selected_Aggregate (void)
{
	// Assume failure
	bool retval = false;

	// Do we have a valid render object?
	if (m_pCRenderObj != NULL) {

		// Build the default filename from the name of render object
		CString default_filename = GetDataTreeView ()->GetCurrentSelectionName ();
		default_filename += ".w3d";

		RestrictedFileDialogClass dialog (FALSE,
								  ".w3d",
								  (LPCTSTR)default_filename,
								  OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
								  "Westwood 3D Files (*.w3d)|*.~xyzabc||",
								  AfxGetMainWnd ());

		// Ask the user what filename they wish to save as.
		dialog.m_ofn.lpstrTitle = "Export Aggregate";
		if (dialog.DoModal () == IDOK) {
			
			// Save the aggregate to the requested file
			retval = Save_Current_Aggregate (dialog.GetPathName ());
		}
	}

	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Save_Current_Aggregate
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Save_Current_Aggregate (const CString &filename)
{
	// Assume failure
	bool retval = false;
	// Get the prototype for this aggregate
	AggregatePrototypeClass *proto = NULL;
	proto = (AggregatePrototypeClass *)WW3DAssetManager::Get_Instance ()->Find_Prototype (m_pCRenderObj->Get_Name ());
	ASSERT (proto != NULL);
	if (proto != NULL) {

		// Get the definition from the prototype
		AggregateDefClass *pdefinition = proto->Get_Definition ();
		ASSERT (pdefinition != NULL);
		if (pdefinition != NULL) {

			// Get a file object for the new file
			FileClass *pfile = _TheFileFactory->Get_File (filename);
			if (pfile) {

				// Open the file for writing
				pfile->Open (FileClass::WRITE);

				// Save the definition to the file
				ChunkSaveClass save_chunk (pfile);
				retval = (pdefinition->Save_W3D (save_chunk) == WW3D_ERROR_OK);

				// Close the file
				pfile->Close ();
				_TheFileFactory->Return_File (pfile);
			}
		}
	}
	// Return the true/false result code
	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Update_Aggregate_Prototype
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Update_Aggregate_Prototype (RenderObjClass &render_obj)
{
	// Build a definition object from the render object
	AggregateDefClass *pdefinition = new AggregateDefClass (render_obj);
	AggregatePrototypeClass *pprototype = new AggregatePrototypeClass (pdefinition);

	// Add this prototype to the asset manager
	WW3DAssetManager::Get_Instance ()->Remove_Prototype (pdefinition->Get_Name ());
	WW3DAssetManager::Get_Instance ()->Add_Prototype (pprototype);
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Update_LOD_Prototype
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Update_LOD_Prototype (HLodClass &hlod)
{
	// Build a definition object from the render object
	HLodDefClass *pdefinition = new HLodDefClass (hlod);
	HLodPrototypeClass *pprototype = new HLodPrototypeClass (pdefinition);

	// Add this prototype to the asset manager
	WW3DAssetManager::Get_Instance ()->Remove_Prototype (pdefinition->Get_Name ());
	WW3DAssetManager::Get_Instance ()->Add_Prototype (pprototype);
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Animate_Camera
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Animate_Camera (bool banimate)
{
	m_bAnimateCamera = banimate;
	
	// Restore the camera if we are done animating it
	if (m_bAnimateCamera == false) {
		::AfxGetMainWnd ()->SendMessage (WM_COMMAND, MAKEWPARAM (IDM_CAMERA_RESET, 0));
	}
	
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Make_Movie
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Make_Movie (void)
{
	// Hide the mouse cursor when we're making a movie.
	bool restore_cursor = Is_Cursor_Shown();
	Show_Cursor(false);
	int i;

	CGraphicView *graphic_view= GetGraphicView ();
	if (m_pCRenderObj && m_pCAnimation) {

		// Get the directory where this executable was run from
		TCHAR filename[MAX_PATH];
		::GetModuleFileName (NULL, filename, sizeof (filename));

		// Strip the filename from the path
		LPTSTR ppath = ::strrchr (filename, '\\');
		if (ppath != NULL) {
			ppath[0] = 0;
		}
		::SetCurrentDirectory (filename);

		// Rewind the animation
		if (m_pCAnimCombo)
		{
			for (i = 0; i < m_pCAnimCombo->Get_Num_Anims(); i++)
				m_pCAnimCombo->Set_Frame(i, 0.0f);

			m_pCRenderObj->Set_Animation (m_pCAnimCombo);
		}
		else
			m_pCRenderObj->Set_Animation (m_pCAnimation, (int)0);
		graphic_view->RepaintView (FALSE);

		// Begin our movie
		WW3D::Pause_Movie (true);
		WW3D::Start_Movie_Capture ("Grab", 30);
		WW3D::Pause_Movie (true);
		
		float frames = m_pCAnimation->Get_Num_Frames ();
		float frame_inc = m_pCAnimation->Get_Frame_Rate () / 30.0F;
		DWORD ticks = 1000 / 30;

		// Loop through all the frames of animation
		for (float frame = 0; frame <= (frames - 1.0F); frame += frame_inc) {

			if (m_pCAnimCombo)
			{
				for (i = 0; i < m_pCAnimCombo->Get_Num_Anims(); i++)
					m_pCAnimCombo->Set_Frame(i, frame);
				m_pCRenderObj->Set_Animation (m_pCAnimCombo);
			}
			else
				m_pCRenderObj->Set_Animation (m_pCAnimation, frame);

			// Should we be updating the camera?
			if (m_bAnimateCamera) {
				int index = m_pCRenderObj->Get_Bone_Index ("CAMERA");
				if (index != -1) {
					
					// Convert the bone's transform into a camera transform
					Matrix3D	transform = m_pCRenderObj->Get_Bone_Transform (index);												
					Matrix3D cam_transform (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
#ifdef ALLOW_TEMPORARIES
					Matrix3D new_transform = transform * cam_transform;
#else
					Matrix3D new_transform;
					new_transform.mul(transform, cam_transform);
#endif

					// Pass the new transform onto the camera
					CameraClass *pcamera = GetGraphicView()->GetCamera ();
					pcamera->Set_Transform (new_transform);
				}
			}	
			
			graphic_view->RepaintView (FALSE, ticks);
			graphic_view->RepaintView (FALSE, 1);
			WW3D::Update_Movie_Capture ();

			if (::GetAsyncKeyState (VK_ESCAPE) < 0) {
				break;
			}
		}

		// Stop capturing the movie data
		WW3D::Stop_Movie_Capture ();
	}

	// Restore the mouse cursor to its previous visibility state.
	Show_Cursor(restore_cursor);

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Build_Emitter_List
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Build_Emitter_List
(
	EmitterInstanceListClass *emitter_list,
	LPCTSTR emitter_name,
	RenderObjClass *render_obj
)
{
	//
	// If the render object is NULL, then start from the current render object
	//
	if (render_obj == NULL) {
		render_obj = m_pCRenderObj;
	}

	//
	// Recursively walk through the objects in the scene
	//
	for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
		RenderObjClass *psub_obj = render_obj->Get_Sub_Object (index);
		if (psub_obj != NULL) {
			Build_Emitter_List (emitter_list, emitter_name, psub_obj);
		}
		MEMBER_RELEASE (psub_obj);
	}
	
	//
	// Is this the emitter we are requesting?
	//
	if ((render_obj != NULL) &&
		 (render_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) &&
		 (::lstrcmpi (emitter_name, render_obj->Get_Name ()) == 0)) {
		
		emitter_list->Add_Emitter ((ParticleEmitterClass *)render_obj);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Show_Cursor
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Show_Cursor (bool onoff)
{
	if (m_pCursor == NULL) {
		Create_Cursor ();
	}

	m_pCursor->Set_Hidden (!onoff);
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Is_Cursor_Shown
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Is_Cursor_Shown (void) const
{
	return m_pCursor != NULL && m_pCursor->Is_Not_Hidden_At_All ();
}


///////////////////////////////////////////////////////////////
//
//  Set_Cursor
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Set_Cursor (LPCTSTR resource_name)
{	
	m_pCursor->Set_Texture (::Load_RC_Texture (resource_name));
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Create_Cursor
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Create_Cursor (void)
{
	if (m_pCursor == NULL) {
		m_pCursor = new ScreenCursorClass;
		m_pCursor->Set_Window (GetGraphicView ()->m_hWnd);
		m_pCursor->Set_Texture (::Load_RC_Texture ("cursor.tga"));
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Count_Particles
//
///////////////////////////////////////////////////////////////
int
CW3DViewDoc::Count_Particles (RenderObjClass *render_obj)
{
	int count = 0;

	//
	// If the render object is NULL, then start from the current render object
	//
	if (render_obj == NULL) {
		render_obj = m_pCRenderObj;
	}

	//
	// Recursively walk through the subobjects
	//
	if (render_obj != NULL) {
		for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
			RenderObjClass *psub_obj = render_obj->Get_Sub_Object (index);
			if (psub_obj != NULL) {
				count += Count_Particles (psub_obj);
			}
			MEMBER_RELEASE (psub_obj);
		}


		// If this is an emitter, then remove its buffer
		if (render_obj->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {
			
			//
			//	Add the number of particles in the buffer to the total
			//
			ParticleEmitterClass *emitter = static_cast<ParticleEmitterClass *> (render_obj);
			ParticleBufferClass *buffer = emitter->Peek_Buffer ();
			if (buffer != NULL) {
				count += buffer->Get_Particle_Count ();
			}		
		}
	}

	return count;
}


///////////////////////////////////////////////////////////////
//
//  Update_Particle_Count
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Update_Particle_Count (void)
{
	int particles = Count_Particles ();
	((CMainFrame *)::AfxGetMainWnd ())->Update_Particle_Count (particles);
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Switch_LOD
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Switch_LOD (int increment, RenderObjClass *render_obj)
{
	//
	// If the render object is NULL, then start from the current render object
	//
	if (render_obj == NULL) {
		render_obj = m_pCRenderObj;
	}

	//
	// Recursively walk through the subobjects
	//
	if (render_obj != NULL) {
		for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
			RenderObjClass *psub_obj = render_obj->Get_Sub_Object (index);
			if (psub_obj != NULL) {
				Switch_LOD (increment, psub_obj);
			}
			MEMBER_RELEASE (psub_obj);
		}

		//
		// If this is an HLOD then switch its LOD
		//
		if (render_obj->Class_ID () == RenderObjClass::CLASSID_HLOD) {
			int current_lod = ((HLodClass *)render_obj)->Get_LOD_Level ();
			((HLodClass *)render_obj)->Set_LOD_Level (current_lod + increment);
		}
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Toggle_Alternate_Materials
//
///////////////////////////////////////////////////////////////
void 
CW3DViewDoc::Toggle_Alternate_Materials(RenderObjClass * render_obj)
{
	// 
	// If the render object is NULL, start from the current render object
	//
	if (render_obj == NULL) {
		render_obj = m_pCRenderObj;
	}

	if (render_obj != NULL) {

		// 
		// If this is a mesh, toggle the materials
		//
		if (render_obj->Class_ID() == RenderObjClass::CLASSID_MESH) {
			MeshModelClass * mdl = ((MeshClass *)render_obj)->Get_Model();
			mdl->Enable_Alternate_Material_Description(!mdl->Is_Alternate_Material_Description_Enabled());
		}

		// 
		// Recurse into any children
		//
		for (int index = 0; index < render_obj->Get_Num_Sub_Objects(); index++) {
			RenderObjClass * sub_obj = render_obj->Get_Sub_Object(index);
			Toggle_Alternate_Materials(sub_obj);
		}
	}
		
	return;
}


///////////////////////////////////////////////////////////////
//
//  Lookup_Path
//
///////////////////////////////////////////////////////////////
bool
CW3DViewDoc::Lookup_Path (LPCTSTR asset_name, CString &path) 
{
	bool retval = false;

	//
	//	Loop over every file we've loaded...
	//
	int counter = m_LoadList.Count ();
	while ((counter --) && !retval) {
		
		//
		//	Does this path contain the W3D this asset came from?
		//
		CString curr_path		= m_LoadList[counter];
		CString curr_asset	= ::Asset_Name_From_Filename (curr_path);
		if (::lstrcmpi (asset_name, curr_asset) == 0) {
			path = curr_path;
			retval = true;
		}
	}

	return retval;
}


///////////////////////////////////////////////////////////////
//
//  Copy_Assets_To_Dir
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Copy_Assets_To_Dir (LPCTSTR directory) 
{	
	CDataTreeView *data_tree = GetDataTreeView ();
	SANITY_CHECK ((m_pCRenderObj != NULL && data_tree != NULL)) {
		return ;
	}

	//
	//	Attempt to locate the file this asset came from
	//
	LPCTSTR asset_name = data_tree->GetCurrentSelectionName ();
	CString filename;
	if (Lookup_Path (asset_name, filename)) {
		
		CString src_path	= ::Strip_Filename_From_Path (filename);
		CString dest_path	= directory;
		::Delimit_Path (src_path);
		::Delimit_Path (dest_path);

		//
		// Get a list of dependent files from the render object
		//
		DynamicVectorClass<StringClass> dependency_list;
		m_pCRenderObj->Build_Dependency_List (dependency_list);		

		//
		//	Loop over the list of dependent files and copy them
		// each to the destination directory
		//
		DynamicVectorClass<CString> copy_failure_list;
		int counter = dependency_list.Count ();
		while (counter --) {

			//
			//	Determine the source and destination filenames
			//
			StringClass filename		= dependency_list[counter];
			CString src_filename		= src_path + CString (filename);
			CString dest_filename	= dest_path + CString (filename);

			//
			//	Copy the file
			//
			if (::Copy_File (src_filename, dest_filename, true) == false) {
				copy_failure_list.Add (src_filename);
			}
		}

		//
		//	Let the user know if something failed to copy
		//
		if (copy_failure_list.Count () > 0) {
			CString message = "Unable to copy the following files:\r\n\r\n";

			counter = copy_failure_list.Count ();
			while (counter --) {
				message += copy_failure_list[counter];
				message += "\r\n";
			}

			::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, "Copy Failure", MB_ICONERROR | MB_OK);
		}

	} else {

		//
		//	Let the user know if we were unable to find the asset's W3D file
		//
		CString message;
		message.Format ("Unable to find file for asset: %s.", asset_name);
		::MessageBox (::AfxGetMainWnd ()->m_hWnd, message, "File Not Found", MB_ICONEXCLAMATION | MB_OK);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Set_Texture_Path1
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Set_Texture_Path1 (LPCTSTR path)
{		
	if (m_TexturePath1.CompareNoCase (path) != 0) {

		//
		//	Pass the new search path onto the File Factory
		//
		if (::lstrlen (path) > 0) {
			_TheSimpleFileFactory->Append_Sub_Directory(path);
		}

		m_TexturePath1 = path;
		theApp.WriteProfileString ("Config", "TexturePath1", m_TexturePath1);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Set_Texture_Path2
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Set_Texture_Path2 (LPCTSTR path)
{	
	if (m_TexturePath2.CompareNoCase (path) != 0) {
		
		//
		//	Pass the new search path onto Surrender
		//
		if (::lstrlen (path) > 0) {
			_TheSimpleFileFactory->Append_Sub_Directory(path);
		}
		m_TexturePath2 = path;
		theApp.WriteProfileString ("Config", "TexturePath2", m_TexturePath2);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Import_Facial_Animation
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Import_Facial_Animation (const CString &heirarchy_name, const CString &filename)
{
	//
	// Crate a new text file object that can be used to import the animation
	//
	TextFileClass *anim_desc_file = new TextFileClass (filename);
	if (anim_desc_file->Open () == (int)true) {

		//
		// Create the new animation and import its data from the text file
		//
		HMorphAnimClass *new_anim = new HMorphAnimClass;
		new_anim->Import (heirarchy_name, *anim_desc_file);
		
		//
		//	Give the new animation a name
		//
		CString new_name;
		CString animation_name = ::Asset_Name_From_Filename (filename);
		animation_name.MakeUpper ();
		new_name.Format ("%s.%s", (LPCTSTR)heirarchy_name, (LPCTSTR)animation_name);
		new_anim->Set_Name (new_name);

		//
		// Add this animation to the asset manager
		//
		WW3DAssetManager::Get_Instance ()->Add_Anim (new_anim);

		//
		// Save this animation to disk
		//
		CString directory = ::Strip_Filename_From_Path (filename);
		::Delimit_Path (directory);
		CString path = directory + animation_name + CString (".w3d");
		RawFileClass *animation_file = new RawFileClass (path);
		if (	animation_file->Create () == (int)true &&
				animation_file->Open (FileClass::WRITE) == (int)true)
		{
			ChunkSaveClass csave (animation_file);
			new_anim->Save_W3D (csave);
			animation_file->Close ();
		}
		SAFE_DELETE (animation_file);

		//
		// Cleanup
		//
		anim_desc_file->Close ();
		MEMBER_RELEASE (new_anim);
		SAFE_DELETE (anim_desc_file);
	}

	return ;
}


///////////////////////////////////////////////////////////////
//
//  Get_Current_HTree
//
///////////////////////////////////////////////////////////////
const HTreeClass *
CW3DViewDoc::Get_Current_HTree (void) const
{
	const HTreeClass *htree = NULL;

	if (m_pCRenderObj != NULL) {
		htree = m_pCRenderObj->Get_HTree ();
	}

	return htree;
}


///////////////////////////////////////////////////////////////
//
//  Save_Camera_Settings
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Save_Camera_Settings (void)
{
	theApp.WriteProfileInt ("Config", "UseManualFOV", m_ManualFOV);
	theApp.WriteProfileInt ("Config", "UseManualClipPlanes", m_ManualClipPlanes);	

	CGraphicView *graphic_view	= ::Get_Graphic_View ();
	CameraClass *camera			= graphic_view->GetCamera ();
	if (camera != NULL) {
		
		double hfov = camera->Get_Horizontal_FOV ();
		double vfov = camera->Get_Vertical_FOV ();

		float znear = 0;
		float zfar = 0;
		camera->Get_Clip_Planes (znear, zfar);

		CString hfov_string;
		CString vfov_string;
		CString znear_string;
		CString zfar_string;
		hfov_string.Format ("%f", hfov);
		vfov_string.Format ("%f", vfov);
		znear_string.Format ("%f", znear);
		zfar_string.Format ("%f", zfar);

		theApp.WriteProfileString ("Config", "hfov", hfov_string);
		theApp.WriteProfileString ("Config", "vfov", vfov_string);
		theApp.WriteProfileString ("Config", "znear", znear_string);
		theApp.WriteProfileString ("Config", "zfar", zfar_string);		
	}
	
	return ;
}


///////////////////////////////////////////////////////////////
//
//  Load_Camera_Settings
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Load_Camera_Settings (void)
{
	m_ManualFOV				= (theApp.GetProfileInt ("Config", "UseManualFOV", 0) == TRUE);
	m_ManualClipPlanes	= (theApp.GetProfileInt ("Config", "UseManualClipPlanes", 0) == TRUE);

	CGraphicView *graphic_view	= GetGraphicView ();
	if (graphic_view != NULL) {
		CameraClass *camera = graphic_view->GetCamera ();
		if (camera != NULL) {

			//
			// Should we load the FOV settings from the registry?
			//
			if (m_ManualFOV) {

				CString hfov_string = theApp.GetProfileString ("Config", "hfov", "0");
				CString vfov_string = theApp.GetProfileString ("Config", "vfov", "0");

				double hfov = ::atof (hfov_string);
				double vfov = ::atof (vfov_string);
				
				camera->Set_View_Plane (hfov, vfov);
			}

			//
			//	Should we load the clip planes from the registry?
			//
			if (m_ManualClipPlanes) {

				CString znear_string	= theApp.GetProfileString ("Config", "znear", "0.1F");
				CString zfar_string	= theApp.GetProfileString ("Config", "zfar", "100.0F");

				float znear	= ::atof (znear_string);
				float zfar		= ::atof (zfar_string);

				camera->Set_Clip_Planes (znear, zfar);
				
				if (m_pCScene != NULL) {
					m_pCScene->Set_Fog_Range (znear, zfar);
					m_pCScene->Recalculate_Fog_Planes();
				}
			}
		}
	}

	return ;
}

///////////////////////////////////////////////////////////////
//
//  Render_Dazzles
//
///////////////////////////////////////////////////////////////
void
CW3DViewDoc::Render_Dazzles (CameraClass * camera)
{
	if (m_pDazzleLayer != NULL) {
		m_pDazzleLayer->Render(camera);
	}
}


void
CW3DViewDoc::SetBackgroundColor (const Vector3 &backgroundColor)
{
	m_backgroundColor = backgroundColor;

	// If this assert fires, then we will be getting the background color
	// and the fog color out of sync. That would look funky.
	ASSERT(m_pCScene);
	if (m_pCScene)
		m_pCScene->Set_Fog_Color(backgroundColor);
}


void
CW3DViewDoc::EnableFog (bool enable)
{
	if (m_pCScene)
	{
		m_pCScene->Set_Fog_Enable(enable);
		m_bFogEnabled = enable;
	}
}
