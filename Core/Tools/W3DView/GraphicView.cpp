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

// GraphicView.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "GraphicView.h"
#include "ww3d.h"
#include "Globals.h"
#include "W3DViewDoc.h"
#include <process.h>
#include "quat.h"
#include "MainFrm.h"
#include "Utils.h"
#include "mmsystem.h"
#include "light.h"
#include "ViewerAssetMgr.h"
#include "rcfile.h"
#include "part_emt.h"
#include "part_buf.h"
#include "hlod.h"
#include "ViewerScene.h"
#include "ScreenCursor.h"
#include "mesh.h"
#include "coltest.h"
#include "MPU.H"
#include "dazzle.h"
#include "SoundScene.h"
#include "WWAudio.h"
#include "metalmap.h"
#include "dx8wrapper.h"
#include "matrix3.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////
//  Local Prototypes
/////////////////////////////////////////////////////////////////////////
void CALLBACK fnTimerCallback (UINT, UINT, DWORD, DWORD, DWORD);


IMPLEMENT_DYNCREATE(CGraphicView, CView)


////////////////////////////////////////////////////////////////////////////
//
//  CGraphicView
//
////////////////////////////////////////////////////////////////////////////
CGraphicView::CGraphicView (void)
    : m_bInitialized (FALSE),
      m_pCamera (NULL),
      m_TimerID (0),
      m_bMouseDown (FALSE),
      m_bRMouseDown (FALSE),
      m_bActive (TRUE),
      m_animationSpeed (1.0F),
      m_dwLastFrameUpdate (0),
		m_iWindowed (1),
      m_animationState (AnimInvalid),
      m_objectRotation (NoRotation),
		m_LightRotation (NoRotation),
		m_bLightMeshInScene (false),
		m_pLightMesh (NULL),
		m_ParticleCountUpdate (0),
		m_CameraBonePosX (false),
		m_UpdateCounter (0),
      m_allowedCameraRotation (FreeRotation),
		m_ObjectCenter (0.0f, 0.0f, 0.0f)
{
    // Get the windowed mode from the registry
    CString string_windowed = theApp.GetProfileString ("Config", "Windowed", "1");
	 m_iWindowed = ::atoi ((LPCTSTR)string_windowed);
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  ~CGraphicView
//
////////////////////////////////////////////////////////////////////////////
CGraphicView::~CGraphicView ()
{
	return ;
}


BEGIN_MESSAGE_MAP(CGraphicView, CView)
	//{{AFX_MSG_MAP(CGraphicView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



////////////////////////////////////////////////////////////////////////////
//
//  OnDraw
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnDraw (CDC* pDC)
{
	// Get the document to display
    CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();

    // Are we in a valid state?
    if (!pDC->IsPrinting ())
    {
    }

    return ;
}


#ifdef RTS_DEBUG
void CGraphicView::AssertValid() const
{
	CView::AssertValid();
}

void CGraphicView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //RTS_DEBUG



////////////////////////////////////////////////////////////////////////////
//
//  PreCreateWindow
//
////////////////////////////////////////////////////////////////////////////
int
CGraphicView::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	// Allow the base class to process this message
    if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

    m_dwLastFrameUpdate = timeGetTime ();//::GetTickCount ();
	return 0;
}


////////////////////////////////////////////////////////////////////////////
//
//  InitializeGraphicView
//
////////////////////////////////////////////////////////////////////////////
BOOL
CGraphicView::InitializeGraphicView (void)
{
	// Assume failure
	BOOL bReturn = FALSE;
	if (g_iDeviceIndex < 0) {
		return FALSE;
	}

	m_bInitialized = FALSE;

	// Initialize the rendering engine with the information from
	// this window.
	RECT rect;
	GetClientRect (&rect);

	int cx = rect.right-rect.left;
	int cy = rect.bottom-rect.top;
	if (m_iWindowed == 0) {
		cx = g_iWidth;
		cy = g_iHeight;
		((CW3DViewDoc *)GetDocument())->Show_Cursor (true);
	} else {
		((CW3DViewDoc *)GetDocument())->Show_Cursor (false);
	}

	bReturn = (WW3D::Set_Render_Device (g_iDeviceIndex,
													cx,
													cy,
													g_iBitsPerPixel,
													m_iWindowed) == WW3D_ERROR_OK);

    ASSERT (bReturn);
    if (bReturn && (m_pCamera == NULL))
    {
        // Instantiate a new camera class
	    m_pCamera = new CameraClass ();
        bReturn = (m_pCamera != NULL);

        // Were we successful in creating a camera?
        ASSERT (m_pCamera);
        if (m_pCamera)
        {
            // Create a transformation matrix
            Matrix3D transform (1);
	        transform.Translate (Vector3 (0.0F, 0.0F, 35.0F));

	        // Point the camera in this direction (I think)
            m_pCamera->Set_Transform (transform);
        }

		  //
		  //	Attach the 'listener' to the camera
		  //
		  WWAudioClass::Get_Instance ()->Get_Sound_Scene ()->Attach_Listener_To_Obj (m_pCamera);
    }

	Reset_FOV ();

	 if (m_pLightMesh == NULL)
	 {
		ResourceFileClass light_mesh_file (NULL, "Light.w3d");
		WW3DAssetManager::Get_Instance()->Load_3D_Assets (light_mesh_file);

		m_pLightMesh = WW3DAssetManager::Get_Instance()->Create_Render_Obj ("LIGHT");
		ASSERT (m_pLightMesh != NULL);
		m_bLightMeshInScene = false;
	 }


    // Remember whether or not we are initialized
    m_bInitialized = bReturn;

    if (m_bInitialized && (m_TimerID == 0))
    {
		// Kick off a timer that we can use to update
		// the display (kinda like a game loop iterator)
		TIMECAPS caps = { 0 };
		::timeGetDevCaps (&caps, sizeof (TIMECAPS));
		UINT freq = max (caps.wPeriodMin, 16U);
		m_TimerID = (UINT)::timeSetEvent (freq,
													 freq,
													 fnTimerCallback,
													 (DWORD)m_hWnd,
													 TIME_PERIODIC);
    }

	// Return the TRUE/FALSE result code
	return bReturn;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnSize
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnSize
(
    UINT nType,
    int cx,
    int cy
)
{
	// Allow the base class to process this message
    CView::OnSize (nType, cx, cy);

	if (m_bInitialized) {

		if (m_iWindowed == 0) {
			cx = g_iWidth;
			cy = g_iHeight;
		}

		// Change the resolution of the rendering device to
		// match that of the view's current dimensions
		if (m_iWindowed == 1) {
			WW3D::Set_Device_Resolution (cx, cy, g_iBitsPerPixel, m_iWindowed);
		}

		// Force a repaint of the screen
		Reset_FOV ();
		RepaintView ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnDestroy
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnDestroy (void)
{
	// Allow the base class to process this message
	CView::OnDestroy ();

	//
	//	Remove the listener from the camera
	//
	WWAudioClass::Get_Instance ()->Get_Sound_Scene ()->Attach_Listener_To_Obj (NULL);

	//
	// Free the camera object
	//
	MEMBER_RELEASE (m_pCamera);
	MEMBER_RELEASE (m_pLightMesh);

	// Is there an update thread running?
	if (m_TimerID == 0) {

		// Stop the timer
		::timeKillEvent ((UINT)m_TimerID);
		m_TimerID = 0;
	}

	// Cache this information in the registry
	TCHAR temp_string[10];
	::itoa (m_iWindowed, temp_string, 10);
	theApp.WriteProfileString ("Config", "Windowed", temp_string);

	// We are no longer initialized
	m_bInitialized = FALSE;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnInitialUpdate
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnInitialUpdate (void)
{
	// Allow the base class to process this message
    CView::OnInitialUpdate ();

	CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();
	if (doc)
	{
		// Ask the document to initialize the scene (if it hasn't
		// already done so)
		doc->InitScene ();
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Lowest_LOD
//
////////////////////////////////////////////////////////////////////////////
void
Set_Lowest_LOD (RenderObjClass *render_obj)
{
	if (render_obj != NULL) {
		for (int index = 0; index < render_obj->Get_Num_Sub_Objects (); index ++) {
			RenderObjClass *psub_obj = render_obj->Get_Sub_Object (index);
			if (psub_obj != NULL) {
				Set_Lowest_LOD (psub_obj);
			}
			MEMBER_RELEASE (psub_obj);
		}

		//
		// Switcht this LOD to its lowest level
		//
		if (render_obj->Class_ID () == RenderObjClass::CLASSID_HLOD) {
			((HLodClass *)render_obj)->Set_LOD_Level (0);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Allow_Update
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Allow_Update (bool onoff)
{
	if (onoff) {
		m_UpdateCounter --;
	} else {
		m_UpdateCounter ++;
	}

	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  RepaintView
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::RepaintView
(
	BOOL bUpdateAnimation,
	DWORD ticks_to_use
)
{
	//
	//	Simple check to avoid re-entrance
	//
	static bool _already_painting = false;
	if (_already_painting) return;
	_already_painting = true;

	 //
	 // Are we in a valid state?
	 //
	 CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument();
	 if (doc->Is_Initialized () && doc->GetScene () && m_UpdateCounter == 0) {

		// Only update the frame if the animation is
		// supposed to be playing
		int cur_ticks = timeGetTime();
		int ticks_elapsed = cur_ticks - m_dwLastFrameUpdate;
		m_dwLastFrameUpdate = cur_ticks;

		// Update the W3D frame times according to our elapsed tick count
		if (ticks_to_use == 0) {
			WW3D::Sync (WW3D::Get_Sync_Time() + (ticks_elapsed * m_animationSpeed));
		} else {
			WW3D::Sync (WW3D::Get_Sync_Time() + ticks_to_use);
		}

		// Do we need to update the current animation?
		if ((m_animationState == AnimPlaying) &&
			bUpdateAnimation)
		{
			float animationSpeed = ((float)ticks_elapsed) / 1000.00F;
			animationSpeed = (animationSpeed * m_animationSpeed);
			doc->UpdateFrame (animationSpeed);
		}

		// Perform the object rotation if necessary
		if ((m_objectRotation != NoRotation) &&
			(bUpdateAnimation == TRUE))
		{
			Rotate_Object ();
		}

		// Perform the light rotation if necessary
		if ((m_LightRotation != NoRotation) &&
			(bUpdateAnimation == TRUE))
		{
			Rotate_Light ();
		}

		// Reset the current lod to be the lowest possible LOD...
		RenderObjClass *prender_obj = doc->GetDisplayedObject ();
		if ((prender_obj != NULL) &&
			 (doc->GetScene ()->Are_LODs_Switching ()))
		{
			Set_Lowest_LOD (prender_obj);
		}

		// Update the metal map
		// assuming object is at origin
		MetalMapManagerClass *metal=_TheAssetMgr->Peek_Metal_Map_Manager();
		if (metal)
		{
			LightClass *pscene_light = doc->GetSceneLight();
			Vector3 ambient,diffuse,l,v;
			ambient=doc->GetScene()->Get_Ambient_Light();
			pscene_light->Get_Diffuse(&diffuse);
			l=pscene_light->Get_Position();
			l.Normalize();
			v=m_pCamera->Get_Position();
			v.Normalize();
			metal->Update_Lighting(ambient,diffuse,l,v);
			metal->Update_Textures();
		}

		//
		//	Render the background BMP
		//
		WW3D::Begin_Render (TRUE, TRUE, doc->GetBackgroundColor ());
		WW3D::Render (doc->Get2DScene (), doc->Get2DCamera (), FALSE, FALSE);

		//
		// Render the background scene
		//
		if (doc->GetBackgroundObjectName ().GetLength () > 0) {
			WW3D::Render (doc->GetBackObjectScene (), doc->GetBackObjectCamera (), FALSE, FALSE);
		}

		//
		// Render the main scene
		//
		DWORD pt_high = 0L;

		// Wait for all previous rendering to complete before starting benchmark.
		DWORD profile_time = ::Get_CPU_Clock (pt_high);

		WW3D::Render (doc->GetScene (), m_pCamera, FALSE, FALSE);

		// Wait for all rendering to complete before stopping benchmark.
		DWORD milliseconds = (::Get_CPU_Clock (pt_high) - profile_time) / 1000;

		//
		// Render the cursor
		//
		WW3D::Render (doc->GetCursorScene (), doc->Get2DCamera (), FALSE, FALSE);

		// Render the dazzles
		doc->Render_Dazzles(m_pCamera);

		// Finish out the rendering process
		WW3D::End_Render ();

		//
		//	Let the audio class think
		//
		WWAudioClass::Get_Instance ()->On_Frame_Update (WW3D::Get_Frame_Time());

		//
		//	Update the count of particles and polys in the status bar
		//
		if ((cur_ticks - m_ParticleCountUpdate > 250)) {
			m_ParticleCountUpdate = cur_ticks;
			doc->Update_Particle_Count ();

			int polys = (prender_obj != NULL) ? prender_obj->Get_Num_Polys () : 0;
			((CMainFrame *)::AfxGetMainWnd ())->UpdatePolygonCount (polys);
		}

		//
		//	Update the frame time in the status bar
		//
		((CMainFrame *)::AfxGetMainWnd ())->Update_Frame_Time (milliseconds);
	}

	_already_painting = false;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  UpdateDisplay
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::UpdateDisplay (void)
{
	// Get the document to display
    CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();

    // Are we in a valid state?
    /*if (m_bInitialized && doc->GetScene ())
    {
        RenderObjClass *pCRenderObj = doc->GetDisplayedObject ();
        if (pCRenderObj)
        {
            Matrix3D transform = pCRenderObj->Get_Transform ();
            transform.Rotate_X (0.05F);
            transform.Rotate_Y (0.05F);
            transform.Rotate_Z (0.05F);

            pCRenderObj->Set_Transform (transform);
        }

		// Render the current view inside the frame
        WW3D::Begin_Render (TRUE, TRUE, Vector3 (0.2,0.4,0.6));
		WW3D::Render (doc->GetScene (), m_pCamera, FALSE, FALSE);
		WW3D::End_Render ();
    } */

    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  WindowProc
//
////////////////////////////////////////////////////////////////////////////
LRESULT
CGraphicView::WindowProc
(
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
	// Is this the repaint message we are expecting?
	if (message == WM_USER+101) {

		//
		//	Force the repaint...
		//
		RepaintView ();
		RemoveProp (m_hWnd, "WaitingToProcess");

	} else if (message == WM_PAINT) {

		// If we are in fullscreen mode, then erase the window background
		if (m_iWindowed == 0) {

			// Get the client rectangle of the window
			RECT rect;
			GetClientRect (&rect);

			// Get the window's DC
			HDC hDC = ::GetDC (m_hWnd);
			if (hDC) {

				// Erase the background
				::FillRect (hDC, &rect, (HBRUSH)(COLOR_WINDOW + 1));
				::ReleaseDC (m_hWnd, hDC);
			}
		}

		RepaintView (FALSE);
		ValidateRect (NULL);
		return 0;

	} else if (message == WM_KEYDOWN) {

		if ((wParam == VK_CONTROL) && (m_bLightMeshInScene == false)) {
			CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();
			m_pLightMesh->Add (doc->GetScene ());
			m_bLightMeshInScene = true;
		}

	} else if (message == WM_KEYUP) {

		if ((wParam == VK_CONTROL) && (m_bLightMeshInScene == true)) {
			CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();
			m_pLightMesh->Remove ();
			m_bLightMeshInScene = false;
		}
	}

	// Allow the base class to process this message
	return CView::WindowProc(message, wParam, lParam);
}


////////////////////////////////////////////////////////////////////////////
//
//  fnTimerCallback
//
////////////////////////////////////////////////////////////////////////////
void CALLBACK
fnTimerCallback
(
	UINT uID,
	UINT uMsg,
	DWORD dwUser,
	DWORD dw1,
	DWORD dw2
)
{
	HWND hwnd = (HWND)dwUser;
	if (hwnd != NULL) {

		// Send this event off to the view to process (hackish, but fine for now)
		if ((GetProp (hwnd, "WaitingToProcess") == NULL) &&
			 (GetProp (hwnd, "Inactive") == NULL)) {

			SetProp (hwnd, "WaitingToProcess", (HANDLE)1);

			// Send the message to the view so it will be in the
			// same thread (Surrender doesn't seem to be thread-safe)
			::PostMessage (hwnd, WM_USER + 101, 0, 0L);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLButtonDown
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnLButtonDown
(
    UINT nFlags,
    CPoint point
)
{
	// Capture all mouse messages
	SetCapture ();

	// Mouse button is down
	m_bMouseDown = TRUE;
	m_lastPoint = point;

	if (m_bRMouseDown) {
		::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_CURSOR_GRAB)));
		((CW3DViewDoc *)GetDocument())->Set_Cursor ("grab.tga");
	} else {
		((CW3DViewDoc *)GetDocument())->Set_Cursor ("orbit.tga");
	}

	CView::OnLButtonDown (nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnLButtonUp
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnLButtonUp
(
    UINT nFlags,
    CPoint point
)
{
    if (!m_bRMouseDown)
    {
        // Release the mouse capture
        ReleaseCapture ();
    }

    // Mouse button is up
    m_bMouseDown = FALSE;

    if (m_bRMouseDown == TRUE)
    {
        ::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_CURSOR_ZOOM)));
		  ((CW3DViewDoc *)GetDocument())->Set_Cursor ("zoom.tga");
    }
    else
    {
        ::SetCursor (::LoadCursor (NULL, MAKEINTRESOURCE (IDC_ARROW)));
		  ((CW3DViewDoc *)GetDocument())->Set_Cursor ("cursor.tga");
    }

	// Allow the base class to process this message
    CView::OnLButtonUp (nFlags, point);
    return ;
}

float minZoomAdjust = 0.0F;
Vector3 sphereCenter;
Quaternion rotation;


////////////////////////////////////////////////////////////////////////////
//
//  OnMouseMove
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnMouseMove
(
    UINT nFlags,
    CPoint point
)
{
	int iDeltaX = m_lastPoint.x-point.x;
	int iDeltaY = m_lastPoint.y-point.y;

	// Get the document to display
	CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();

	if (!(nFlags & MK_CONTROL) && m_bLightMeshInScene) {
		m_pLightMesh->Remove ();
		m_bLightMeshInScene = false;
	} else if ((nFlags & MK_CONTROL) && (m_bLightMeshInScene == false)) {
		m_pLightMesh->Add (doc->GetScene ());
		m_bLightMeshInScene = true;
	}

	// Is the mouse button down?
	if (m_bMouseDown && m_bRMouseDown)
	{
		// Get the transformation matrix for the camera and its inverse
		Matrix3D transform = m_pCamera->Get_Transform ();

		RECT rect;
		GetClientRect (&rect);

		float midPointX = float(rect.right >> 1);
		float midPointY = float(rect.bottom >> 1);

		float lastPointX = ((float)m_lastPoint.x - midPointX) / midPointX;
		float lastPointY = (midPointY - (float)m_lastPoint.y) / midPointY;

		float pointX = ((float)point.x - midPointX) / midPointX;
		float pointY = (midPointY - (float)point.y) / midPointY;


		Vector3 cameraPan = Vector3(-1.00F*m_CameraDistance*(pointX - lastPointX), -1.00F*m_CameraDistance*(pointY - lastPointY), 0.00F);

		transform.Translate (cameraPan);

		Matrix3x3 view = Build_Matrix3 (rotation);
		Vector3 move = view * cameraPan;
		sphereCenter += move;

		// Move the camera back to get a good view of the object
		m_pCamera->Set_Transform (transform);

		m_lastPoint = point;
	}
	// Is the mouse button down?
	else if ((nFlags & MK_CONTROL) && m_bMouseDown)
	{
		LightClass *pSceneLight = doc->GetSceneLight ();
		if ((pSceneLight != NULL) && (m_pLightMesh != NULL))
		{
			RECT rect;
			GetClientRect (&rect);

			Vector3 point_in_view;
			Vector3 lastpoint_in_view;

			float midPointX = float(rect.right >> 1);
			float midPointY = float(rect.bottom >> 1);

			float lastPointX = ((float)m_lastPoint.x - midPointX) / midPointX;
			float lastPointY = (midPointY - (float)m_lastPoint.y) / midPointY;

			float pointX = ((float)point.x - midPointX) / midPointX;
			float pointY = (midPointY - (float)point.y) / midPointY;

			Quaternion mouse_motion = Inverse(::Trackball(lastPointX, lastPointY, pointX, pointY, 0.8F));
			Quaternion light_orientation;
			Quaternion camera = Build_Quaternion(m_pCamera->Get_Transform());
			Quaternion cur_light = Build_Quaternion(pSceneLight->Get_Transform());

			light_orientation = camera;
			light_orientation = light_orientation * mouse_motion;
			light_orientation = light_orientation * Inverse(camera);
			light_orientation = light_orientation * cur_light;
			light_orientation.Normalize();

			Vector3 to_center;
			Matrix3D matrix = pSceneLight->Get_Transform();
			Matrix3D::Inverse_Transform_Vector(matrix,sphereCenter,&to_center);

			Matrix3D light_tm(light_orientation, sphereCenter);
			light_tm.Translate(-to_center);

			m_pLightMesh->Set_Transform(light_tm);
			pSceneLight->Set_Transform(light_tm);
		}

		m_lastPoint = point;
	}
	// Is the mouse button down?
	else if ((nFlags & MK_CONTROL) && m_bRMouseDown)
	{
		// Get the currently displayed object
		CW3DViewDoc *doc= (CW3DViewDoc *)GetDocument();
		LightClass *pscene_light = doc->GetSceneLight ();
		RenderObjClass *prender_obj = doc->GetDisplayedObject ();
		if ((pscene_light != NULL) && (prender_obj != NULL)) {

			// Calculate a light adjustment factor
			CRect rect;
			GetClientRect (&rect);
			float deltay = (float(iDeltaY))/(float(rect.bottom - rect.top));
			float adjustment = deltay * (m_ViewedSphere.Radius * 3.0F);

			// Determine the light's new position based on this factor
			Matrix3D transform = pscene_light->Get_Transform ();
			transform.Translate (Vector3 (0, 0, adjustment));

			// Determine what the distance from the light to the object
			// would be with this new position
			Vector3 light_pos = transform.Get_Translation ();
			Vector3 obj_pos = prender_obj->Get_Position ();
			float distance = (light_pos - obj_pos).Length ();

			// If the new position is acceptable, move the light
			if (distance > m_ViewedSphere.Radius) {
				m_pLightMesh->Set_Transform (transform);
				pscene_light->Set_Transform (transform);
			}
		}

		m_lastPoint = point;
	}
	// Is the mouse button down?
	else if (m_bMouseDown)
	{
		// Get the document to display
		CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();

		// Are we in a valid state?
		if (m_bInitialized && doc->GetScene ())
		{
			RenderObjClass *pCRenderObj = doc->GetDisplayedObject ();
			if (pCRenderObj)
			{
				RECT rect;
				GetClientRect (&rect);

				float midPointX = float(rect.right >> 1);
				float midPointY = float(rect.bottom >> 1);

				float lastPointX = ((float)m_lastPoint.x - midPointX) / midPointX;
				float lastPointY = (midPointY - (float)m_lastPoint.y) / midPointY;

				float pointX = ((float)point.x - midPointX) / midPointX;
				float pointY = (midPointY - (float)point.y) / midPointY;

				// Rotate around the object (orbit) using a 0.00F - 1.00F percentage of
				// the mouse coordinates
				rotation = ::Trackball (lastPointX, lastPointY, pointX, pointY, 0.8F);

				// Do we want to 'lock-out' all rotation except X?
				if (m_allowedCameraRotation == OnlyRotateX)
				{
#ifdef ALLOW_TEMPORARIES
					Matrix3D tempMatrix = Build_Matrix3D (rotation);
#else
					Matrix3D tempMatrix;
					Build_Matrix3D (rotation, tempMatrix);
#endif
					Matrix3D tempMatrix2 (1);

					tempMatrix2.Rotate_X (tempMatrix.Get_X_Rotation ());
					tempMatrix2.Set_Translation (tempMatrix.Get_Translation ());

					rotation = Build_Quaternion (tempMatrix2);
				}
				// Do we want to 'lock-out' all rotation except Y?
				else if (m_allowedCameraRotation == OnlyRotateY)
				{
#ifdef ALLOW_TEMPORARIES
					Matrix3D tempMatrix = Build_Matrix3D (rotation);
#else
					Matrix3D tempMatrix;
					Build_Matrix3D (rotation, tempMatrix);
#endif
					Matrix3D tempMatrix2 (1);

					tempMatrix2.Rotate_Y (tempMatrix.Get_Y_Rotation ());
					tempMatrix2.Set_Translation (tempMatrix.Get_Translation ());

					rotation = Build_Quaternion (tempMatrix2);
				}
				// Do we want to 'lock-out' all rotation except Z?
				else if (m_allowedCameraRotation == OnlyRotateZ)
				{
#ifdef ALLOW_TEMPORARIES
					Matrix3D tempMatrix = Build_Matrix3D (rotation);
#else
					Matrix3D tempMatrix;
					Build_Matrix3D (rotation, tempMatrix);
#endif
					Matrix3D tempMatrix2 (1);

					tempMatrix2.Rotate_Z (tempMatrix.Get_Z_Rotation ());
					tempMatrix2.Set_Translation (tempMatrix.Get_Translation ());

					rotation = Build_Quaternion (tempMatrix2);
				}

				// Get the transformation matrix for the camera and its inverse
				Matrix3D transform = m_pCamera->Get_Transform ();
				Matrix3D inverseMatrix;
				transform.Get_Orthogonal_Inverse (inverseMatrix);

#ifdef ALLOW_TEMPORARIES
				Vector3 to_object = inverseMatrix * sphereCenter;
#else
				Vector3 to_object;
				inverseMatrix.mulVector3 (sphereCenter, to_object);
#endif

				transform.Translate (to_object);

#ifdef ALLOW_TEMPORARIES
				Matrix3D::Multiply (transform, Build_Matrix3D (rotation), &transform);
#else
				Matrix3D rotationMatrix;
				Matrix3D::Multiply (transform, Build_Matrix3D (rotation, rotationMatrix), &transform);
#endif

				transform.Translate (-to_object);

				// Rotate and translate the camera
				m_pCamera->Set_Transform (transform);

				doc->GetBackObjectCamera ()->Set_Transform (transform);
				doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));
			}
		}

		m_lastPoint = point;
	}
	else if (m_bRMouseDown)
	{
		m_lastPoint = point;

		// Get the transformation matrix for the camera and its inverse
		Matrix3D transform = m_pCamera->Get_Transform ();

		Vector3 distanceVectorZ = transform.Get_Z_Vector ();
		if (iDeltaY != 0)
		{

			// Get the bouding rectangle of the main view
			CRect rect;
			GetClientRect (&rect);

			float deltay = (float(iDeltaY))/(float(rect.bottom - rect.top));
			float adjustment = deltay * m_CameraDistance * 3.0F;

			if ((adjustment < minZoomAdjust) && (adjustment >= 0.00F))
			{
				 adjustment = minZoomAdjust;
			}

			if ((adjustment > -minZoomAdjust) && (adjustment <= 0.00F))
			{
				 adjustment = -minZoomAdjust;
			}

			if ((m_CameraDistance + adjustment) > 0.00F)
			{
				m_CameraDistance += adjustment;
				transform.Translate (Vector3 (0.0F, 0.0F, adjustment));

				// Move the camera back to get a good view of the object
				m_pCamera->Set_Transform (transform);

				// Get the main window of our app
				CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
				if (pCMainWnd != NULL)
				{
					// Ensure the background camera matches the main camera
					CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument();
					doc->GetBackObjectCamera ()->Set_Transform (transform);
					doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));

					// Update the current object if necessary
					RenderObjClass *prender_obj = doc->GetDisplayedObject ();
					if (prender_obj != NULL) {

						// Ensure the status bar is updated with the correct poly count
						pCMainWnd->UpdatePolygonCount (prender_obj->Get_Num_Polys ());
					}

					// Ensure the status bar is updated with the correct camera distance
					pCMainWnd->UpdateCameraDistance (m_CameraDistance);
				}

			}
		}

		m_lastPoint = point;
	}

	// Allow the base class to process this message
	CView::OnMouseMove (nFlags, point);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_Camera_To_Display_Emitter
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Reset_Camera_To_Display_Emitter (ParticleEmitterClass &emitter)
{
	// Get some of the emitter settings
	Vector3 velocity = emitter.Get_Start_Velocity ();
	const Vector3 &acceleration = emitter.Get_Acceleration ();
	float lifetime = emitter.Get_Lifetime ();

	// If the velocity is 0, then use the randomizer as the default velocity
	bool use_vel_rand = false;
	if ((velocity.X == 0) && (velocity.Y == 0) && (velocity.Z == 0)) {
		//velocity.Set (emitter.Get_Velocity_Random (), emitter.Get_Velocity_Random (), emitter.Get_Velocity_Random ());
		//use_vel_rand = true;
	}

	// Determine what the max extent covered by a particle will be.
	Vector3 distance = (velocity * lifetime) + ((acceleration * (lifetime * lifetime)) / 2.0F);

	// Do we need to take into account acceleration?
	Vector3 distance_maxima (0, 0, 0);
	if ((acceleration.X != 0) || (acceleration.Y != 0) || (acceleration.Z != 0)) {

		// Determine at what time (for each x,y,z) a maxima will occur.
		Vector3 time_max (0, 0, 0);
		time_max.X = (acceleration.X != 0) ? ((-velocity.X) / acceleration.X) : 0.00F;
		time_max.Y = (acceleration.Y != 0) ? ((-velocity.Y) / acceleration.Y) : 0.00F;
		time_max.Z = (acceleration.Z != 0) ? ((-velocity.Z) / acceleration.Z) : 0.00F;

		// Is there a maxima for the X direction?
		if ((time_max.X >= 0.0F) && (time_max.X < lifetime)) {
			distance_maxima.X = (velocity.X * time_max.X) + ((acceleration.X * (time_max.X * time_max.X)) / 2.0F);
			distance_maxima.X = fabs (distance_maxima.X);
		}

		// Is there a maxima for the Y direction?
		if ((time_max.Y >= 0.0F) && (time_max.Y < lifetime)) {
			distance_maxima.Y = (velocity.Y * time_max.Y) + ((acceleration.Y * (time_max.Y * time_max.Y)) / 2.0F);
			distance_maxima.Y = fabs (distance_maxima.Y);
		}

		// Is there a maxima for the Z direction?
		if ((time_max.Z >= 0.0F) && (time_max.Z < lifetime)) {
			distance_maxima.Z = (velocity.Z * time_max.Z) + ((acceleration.Z * (time_max.Z * time_max.Z)) / 2.0F);
			distance_maxima.Z = fabs (distance_maxima.Z);
		}
	}

	distance.X = fabs (distance.X);
	distance.Y = fabs (distance.Y);
	distance.Z = fabs (distance.Z);

	// Determine what the maximum distance convered in a single direction is
	float max_dist = max (distance.X, distance.Y);
	max_dist = max (max_dist, distance.Z);
	max_dist = max (max_dist, distance_maxima.X);
	max_dist = max (max_dist, distance_maxima.Y);
	max_dist = max (max_dist, distance_maxima.Z);

	Vector3 center = distance / 2.00F;
	center.X = max (center.X, distance_maxima.X / 2.00F);
	center.Y = max (center.Y, distance_maxima.Y / 2.00F);
	center.Z = max (center.Z, distance_maxima.Z / 2.00F);

	if (use_vel_rand) {
		center.Set (0, 0, 0);
	}

	// Build a logical sphere from the emitters settings
	// that should provide a good viewing distance for the emitter.
	SphereClass sphere;
	sphere.Center = center;
	sphere.Radius = max (emitter.Get_Particle_Size () * 5, (max_dist * 3.0F) / 5.0F);

	// View this sphere
	Reset_Camera_To_Display_Sphere (sphere);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_Camera_To_Display_Sphere
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Reset_Camera_To_Display_Sphere (SphereClass &sphere)
{
	// Calculate a default camera distance to view this sphere
	m_CameraDistance = sphere.Radius * 3.00F;
	m_CameraDistance = (m_CameraDistance < 1.0F) ? 1.0F : m_CameraDistance;

	// Calculate a transform that is the appropriate distance
	// from the sphere center and is looking at the center
	Matrix3D transform (1);
	transform.Look_At (sphere.Center + Vector3 (m_CameraDistance, 0, 0), sphere.Center, 0);

	// Record some variables for later use
	sphereCenter	= sphere.Center;
	m_ObjectCenter	= sphereCenter;
	minZoomAdjust	= m_CameraDistance / 190.0F;
	rotation			= Build_Quaternion (transform);

	// Move the camera back to get a good view of the object
	m_pCamera->Set_Transform (transform);

	// Make the same adjustment for the scene light
	CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();
	LightClass *pSceneLight = doc->GetSceneLight ();
	if ((m_pLightMesh != NULL) && (pSceneLight != NULL)) {

		// Reposition the light and its 'mesh' as appropriate
		transform.Make_Identity ();
		transform.Set_Translation (sphereCenter);
		transform.Translate (0, 0, 0.7F * m_CameraDistance);
		pSceneLight->Set_Transform (transform);
		m_pLightMesh->Set_Transform (transform);

		// Scale the light's mesh appropriately
		static float last_scale = 1.0F;
		m_pLightMesh->Scale (m_CameraDistance / (14 * last_scale));

		last_scale = m_CameraDistance / 14;
	}

	float max_dist = m_CameraDistance * 60.0F;
	float min_dist = max (0.2F, minZoomAdjust / 2);

	// Set the clipping planes so objects are clipped correctly
	if (doc->Are_Clip_Planes_Manual () == false) {
		m_pCamera->Set_Clip_Planes (min_dist, max_dist);

		// Adjust the fog near clipping plane to the new value, but
		// leave the far clip plane alone (since it is scene dependant
		// not camera dependant).
		float fog_near, fog_far;
		doc->GetScene()->Get_Fog_Range(&fog_near, &fog_far);
		doc->GetScene()->Set_Fog_Range(min_dist, fog_far);
		doc->GetScene()->Recalculate_Fog_Planes();
	}

	// Reset the background camera to match the main camera
	doc->GetBackObjectCamera ()->Set_Transform (transform);
	doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));

	// Update the camera distance in the status bar
	CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
	if (pCMainWnd != NULL) {
		pCMainWnd->UpdateCameraDistance (m_CameraDistance);
		pCMainWnd->UpdateFrameCount (0, 0, 0);
	}

	// Record the sphere we are viewing for later
	m_ViewedSphere = sphere;
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_Camera_To_Display_Object
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Reset_Camera_To_Display_Object (RenderObjClass &render_object)
{
	// Reset the camera to get a good look at this object's bounding sphere
	SphereClass sp = render_object.Get_Bounding_Sphere ();
	Reset_Camera_To_Display_Sphere (sp);

	// Should we update the camera's position as well?
	int index = render_object.Get_Bone_Index ("CAMERA");
	if (index > 0) {

		// Convert the bone's transform into a camera transform
		Matrix3D	transform = render_object.Get_Bone_Transform (index);
		if (m_CameraBonePosX) {
			Matrix3D tmp = transform;
			Matrix3D cam_transform (Vector3 (0, -1, 0), Vector3 (0, 0, 1), Vector3 (-1, 0, 0), Vector3 (0, 0, 0));
#ifdef ALLOW_TEMPORARIES
			transform = tmp * cam_transform;
#else
			transform.mul(tmp, cam_transform);
#endif
		}

		// Pass the new transform onto the camera
		CameraClass *camera = GetCamera ();
		camera->Set_Transform (transform);
	}

	// Update the polygon count in the main window
	CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
	if (pCMainWnd != NULL) {
		pCMainWnd->UpdatePolygonCount (render_object.Get_Num_Polys ());
	}

	// Load the settings in the default.dat if its in the local directory.
	Load_Default_Dat ();
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Load_Default_Dat
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Load_Default_Dat (void)
{
	// Get the directory where this executable was run from
	TCHAR filename[MAX_PATH];
	::GetModuleFileName (NULL, filename, sizeof (filename));

	// Strip the filename from the path
	LPTSTR ppath = ::strrchr (filename, '\\');
	if (ppath != NULL) {
		ppath[0] = 0;
	}

	// Concat the default.dat filename onto the path
	::strcat (filename, "\\default.dat");

	// Does the file exist in the directory?
	if (::GetFileAttributes (filename) != 0xFFFFFFFF) {

		// Ask the document to load the settings from this data file
		CW3DViewDoc *pCDoc = (CW3DViewDoc *)GetDocument ();
		if (pCDoc != NULL) {
			pCDoc->LoadSettings (filename);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnRButtonUp
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnRButtonUp
(
    UINT nFlags,
    CPoint point
)
{
	// Mouse button is up
	m_bRMouseDown = FALSE;

	if (m_bMouseDown) {
		((CW3DViewDoc *)GetDocument())->Set_Cursor ("orbit.tga");
	} else {
		::SetCursor (::LoadCursor (NULL, MAKEINTRESOURCE (IDC_ARROW)));
		((CW3DViewDoc *)GetDocument())->Set_Cursor ("cursor.tga");
		ReleaseCapture ();
	}

	// Allow the base class to process this message
	CView::OnRButtonUp(nFlags, point);
	return ;
}

////////////////////////////////////////////////////////////////////////////
//
//  OnRButtonDown
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnRButtonDown
(
    UINT nFlags,
    CPoint point
)
{
    // Capture all mouse messages
    SetCapture ();

    // Mouse button is down
    m_bRMouseDown = TRUE;
    m_lastPoint = point;

    if (m_bMouseDown)
    {
        ::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_CURSOR_GRAB)));
		  ((CW3DViewDoc *)GetDocument())->Set_Cursor ("grab.tga");
    }
    else
    {
        ::SetCursor (::LoadCursor (::AfxGetResourceHandle (), MAKEINTRESOURCE (IDC_CURSOR_ZOOM)));
		  ((CW3DViewDoc *)GetDocument())->Set_Cursor ("zoom.tga");
    }

	// Allow the base class to process this message
    CView::OnRButtonDown(nFlags, point);
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetAnimationState
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::SetAnimationState (ANIMATION_STATE animationState)
{
    // Has the state changed?
    if (m_animationState != animationState)
    {
        switch (animationState)
        {
            // We want to stop the animation
            case AnimStopped:
            {
                // Get the document so we can get our current object
                CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument();
                ASSERT_VALID (doc);

                // Get the currently displayed object
                RenderObjClass *pCRenderObj = doc->GetDisplayedObject ();
                if (pCRenderObj)
                {
                    // Reset the animation to frame 0

                    if (doc->GetCurrentAnimation()) {
                    	pCRenderObj->Set_Animation (doc->GetCurrentAnimation (), 0);
                    }
                }

                // Reset the animation to frame 0
                doc->ResetAnimation ();
            }
            break;

            case AnimPlaying:
            {
					CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument ();
					doc->Play_Animation_Sound ();

                // Reset the frame timer
					 m_dwLastFrameUpdate = timeGetTime ();
            }
            break;
        }

        // Save the new state
        m_animationState = animationState;
    }

    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetCameraPos
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::SetCameraPos (CAMERA_POS cameraPos)
{
    // Get the document so we can get our current object
    CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument();
    ASSERT_VALID (doc);

    // Get the currently displayed object
    RenderObjClass *pCRenderObj = doc->GetDisplayedObject ();
    if (pCRenderObj)
    {
        SphereClass sphere = m_ViewedSphere;

        m_CameraDistance = sphere.Radius * 3.00F;
        m_CameraDistance = (m_CameraDistance < 1.0F) ? 1.0F : m_CameraDistance;
        m_CameraDistance = (m_CameraDistance > 400.0F) ? 400.0F : m_CameraDistance;

        Matrix3D transform (1);

        switch (cameraPos)
        {
            case CameraFront:
            {
                transform.Look_At (sphere.Center + Vector3 (m_CameraDistance, 0.00F, 0.00F), sphere.Center, 0);
            }
            break;

            case CameraBack:
            {
                transform.Look_At (sphere.Center + Vector3 (-m_CameraDistance, 0.00F, 0.00F), sphere.Center, 0);
            }
            break;

            case CameraLeft:
            {
                transform.Look_At (sphere.Center + Vector3 (0.00F, -m_CameraDistance, 0.00F), sphere.Center, 0);
            }
            break;

            case CameraRight:
            {
                transform.Look_At (sphere.Center + Vector3 (0.00F, m_CameraDistance, 0.00F), sphere.Center, 0);
            }
            break;

            case CameraTop:
            {
                transform.Look_At (sphere.Center + Vector3 (0.00F, 0.00F, m_CameraDistance), sphere.Center, 3.1415926535F);
            }
            break;

            case CameraBottom:
            {
                transform.Look_At (sphere.Center + Vector3 (0.00F, 0.00F, -m_CameraDistance), sphere.Center, 3.1415926535F);
            }
            break;
        }

	    // Move the camera back to get a good view of the object
        m_pCamera->Set_Transform (transform);

        // Get the main window of our app
        CMainFrame *pCMainWnd = (CMainFrame *)::AfxGetMainWnd ();
        if (pCMainWnd != NULL)
        {
            CW3DViewDoc* doc = (CW3DViewDoc *)GetDocument();

            doc->GetBackObjectCamera ()->Set_Transform (transform);
            doc->GetBackObjectCamera ()->Set_Position (Vector3 (0.00F, 0.00F, 0.00F));

            RenderObjClass *pCRenderObj = doc->GetDisplayedObject ();
            if (pCRenderObj)
            {
                pCMainWnd->UpdatePolygonCount (pCRenderObj->Get_Num_Polys ());
            }

            pCMainWnd->UpdateCameraDistance(m_CameraDistance);
        }
    }

    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  RotateObject
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::RotateObject (OBJECT_ROTATION rotation)
{
    // Is this rotation different?
    if (m_objectRotation != rotation)
    {
        // Save the rotation state
        m_objectRotation = rotation;
    }

    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  SetAllowedCameraRotation
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::SetAllowedCameraRotation (CAMERA_ROTATION cameraRotation)
{
    // Store this for later reference
    m_allowedCameraRotation = cameraRotation;
    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  ResetObject
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::ResetObject (void)
{
    // Get the current document
    CW3DViewDoc *doc = ::GetCurrentDocument ();

    ASSERT (doc);
    if (doc)
    {
        // Get the currently displayed object
        RenderObjClass *pCRenderObj = doc->GetDisplayedObject ();
        if (pCRenderObj)
        {
            // Reset the rotation of the object
            pCRenderObj->Set_Transform (Matrix3D(1));
        }
    }

    return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  OnGetMinMaxInfo
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::OnGetMinMaxInfo (MINMAXINFO FAR* lpMMI)
{
	CView::OnGetMinMaxInfo (lpMMI);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Rotate_Object
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Rotate_Object (void)
{
	// Get the document to display
	CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument();

	// Get the currently displayed object
	RenderObjClass *prender_obj = doc->GetDisplayedObject ();
	if (prender_obj != NULL)
	{
		// Get the current transform for the object
		Matrix3D transform = prender_obj->Get_Transform ();

		if ((m_objectRotation & RotateX) == RotateX) {
			transform.Rotate_X (0.05F);
		} else if ((m_objectRotation & RotateXBack) == RotateXBack) {
			transform.Rotate_X (-0.05F);
		}

		if ((m_objectRotation & RotateY) == RotateY) {
			transform.Rotate_Y (-0.05F);
		} else if ((m_objectRotation & RotateYBack) == RotateYBack) {
			transform.Rotate_Y (0.05F);
		}

		if ((m_objectRotation & RotateZ) == RotateZ) {
			transform.Rotate_Z (0.05F);
		} else if ((m_objectRotation & RotateZBack) == RotateZBack) {
			transform.Rotate_Z (-0.05F);
		}

		if (!transform.Is_Orthogonal()) {
			transform.Re_Orthogonalize();
		}

		// Set the new transform for the object
		prender_obj->Set_Transform (transform);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Rotate_Light
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Rotate_Light (void)
{
	// Get the document to display
	CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument();

	// Get the currently displayed object
	LightClass *pscene_light = doc->GetSceneLight ();
	RenderObjClass *prender_obj = doc->GetDisplayedObject ();
	if ((pscene_light != NULL) && (prender_obj != NULL)) {
		Matrix3D rotation_matrix (1);

		// Build a rotation matrix that contains the x,y,z
		// rotations we want to apply to the light
		if ((m_LightRotation & RotateX) == RotateX) {
			rotation_matrix.Rotate_X (0.05F);
		} else if ((m_LightRotation & RotateXBack) == RotateXBack) {
			rotation_matrix.Rotate_X (-0.05F);
		}

		if ((m_LightRotation & RotateY) == RotateY) {
			rotation_matrix.Rotate_Y (-0.05F);
		} else if ((m_LightRotation & RotateYBack) == RotateYBack) {
			rotation_matrix.Rotate_Y (0.05F);
		}

		if ((m_LightRotation & RotateZ) == RotateZ) {
			rotation_matrix.Rotate_Z (0.05F);
		} else if ((m_LightRotation & RotateZBack) == RotateZBack) {
			rotation_matrix.Rotate_Z (-0.05F);
		}

		//
		//	Now, use the rotation matrix to rotate the
		// light 'around' the displayed object (in its coordinate system)
		//
		Matrix3D coord_inv;
		Matrix3D coord_to_obj;
		Matrix3D coord_system = prender_obj->Get_Transform ();
		coord_system.Get_Orthogonal_Inverse (coord_inv);

		Matrix3D transform = pscene_light->Get_Transform ();
		Matrix3D::Multiply (coord_inv, transform, &coord_to_obj);

		Matrix3D::Multiply (coord_system, rotation_matrix, &transform);
		Matrix3D::Multiply (transform, coord_to_obj, &transform);

		// Ensure the matrix hasn't degenerated
		if (!transform.Is_Orthogonal ()) {
			transform.Re_Orthogonalize ();
		}

		// Pass the new transform onto the light
		m_pLightMesh->Set_Transform (transform);
		pscene_light->Set_Transform (transform);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_FOV
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Set_FOV (double hfov, double vfov, bool force)
{
	CW3DViewDoc *doc = (CW3DViewDoc *)GetDocument();

	if (force || (doc->Is_FOV_Manual () == false)) {
		m_pCamera->Set_View_Plane (hfov, vfov);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Reset_FOV
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Reset_FOV (void)
{
	int cx = 0;
	int cy = 0;

	if (m_iWindowed == 0) {
		cx = g_iWidth;
		cy = g_iHeight;
	} else {
		CRect rect;
		GetClientRect (&rect);
		cx = rect.Width ();
		cy = rect.Height ();
	}

	// update the camera FOV settings
	// take the larger of the two dimensions, give it the
	// full desired FOV, then give the other dimension an
	// FOV proportional to its relative size
	double hfov,vfov;
	if (cy > cx) {

		vfov = (float)DEG_TO_RAD(45.0f);
		hfov = (double)cx / (double)cy * vfov;
	} else  {
		hfov = (float)DEG_TO_RAD(45.0f);
		vfov = (double)cy / (double)cx * hfov;
	}

	// Reset the field of view
	Set_FOV (hfov, vfov);
	return ;
}


////////////////////////////////////////////////////////////////////////////
//
//  Set_Camera_Distance
//
////////////////////////////////////////////////////////////////////////////
void
CGraphicView::Set_Camera_Distance (float dist)
{
	m_CameraDistance = dist;

	//
	//	Reposition the camera
	//
	Matrix3D new_tm(1);
	new_tm.Look_At (m_ViewedSphere.Center + Vector3 (m_CameraDistance, 0.00F, 0.00F), m_ViewedSphere.Center, 0);
	m_pCamera->Set_Transform (new_tm);

	//
	// Update the status bar
	//
	CMainFrame *main_wnd = (CMainFrame *)::AfxGetMainWnd ();
	if (main_wnd != NULL) {
		main_wnd->UpdateCameraDistance (m_CameraDistance);
	}

	return ;
}


