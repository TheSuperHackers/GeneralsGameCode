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

// W3DViewDoc.h : interface of the CW3DViewDoc class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "scene.h"
#include "chunkio.h"
#include "hanim.h"
#include "hcanim.h"
#include "dynamesh.h"
#include "rendobj.h"
#include "LODDefs.h"


///////////////////////////////////////////////////////////
//
//  Constants
//
const DWORD SAVE_SETTINGS_LIGHT     = 0x00000001;
const DWORD SAVE_SETTINGS_BACK      = 0x00000002;
const DWORD SAVE_SETTINGS_CAMERA    = 0x00000004;


// Forward declarations
class ParticleEmitterClass;
class CameraClass;
class SceneClass;
class LightClass;
class RenderObjClass;
class HAnimClass;
class CGraphicView;
class CDataTreeView;
class DistLODClass;
class Bitmap2DObjClass;
class AssetInfoClass;
class HLodPrototypeClass;
class HLodClass;
class ViewerSceneClass;
class EmitterInstanceListClass;
class ScreenCursorClass;
class DazzleLayerClass;


/////////////////////////////////////////////////////////////////////
//
//  CW3DViewDoc
//
/////////////////////////////////////////////////////////////////////
class CW3DViewDoc : public CDocument
{
protected: // create from serialization only
	CW3DViewDoc();
	DECLARE_DYNCREATE(CW3DViewDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CW3DViewDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CW3DViewDoc();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CW3DViewDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void SetChannelQCompression(bool bCompress){	m_bCompress_channel_Q = bCompress;}
	//
	//  Accessors
	//
	CameraClass *			Get2DCamera (void) const				{ return m_pC2DCamera; }
	CameraClass *			GetBackObjectCamera (void) const		{ return m_pCBackObjectCamera; }
	SceneClass *			Get2DScene (void) const					{ return m_pC2DScene; }
	SceneClass *			GetCursorScene (void) const			{ return m_pCursorScene; }
	ViewerSceneClass *	GetScene (void) const					{ return m_pCScene; }
	SceneClass *			GetBackObjectScene (void) const		{ return m_pCBackObjectScene; }
	LightClass *			GetSceneLight (void) const				{ return m_pCSceneLight; }
	RenderObjClass *		GetDisplayedObject (void) const		{ return m_pCRenderObj; }
	HAnimClass *			GetCurrentAnimation (void) const		{ return m_pCAnimation; }
	const HTreeClass *	Get_Current_HTree (void) const;

	//
	// Creation/destruction methods
	//
	void					InitScene (void);
	void					LoadAssetsFromFile (LPCTSTR lpszPathName);
	HLodPrototypeClass *GenerateLOD (LPCTSTR pszLODBaseName, LOD_NAMING_TYPE type);
	void					CleanupResources (void);
	bool					Is_Initialized (void)	{ return m_IsInitialized; }

	void					Reload_Displayed_Object (void);
	void					Display_Emitter (ParticleEmitterClass *pemitter = nullptr, bool use_global_reset_flag = true, bool allow_reset = true);
	void					DisplayObject (RenderObjClass *pCModel = nullptr, bool use_global_reset_flag = true, bool allow_reset = true, bool add_ghost = false);
	BOOL					SaveSettings (LPCTSTR pszFilename, DWORD dwSettingsMask);
	BOOL					LoadSettings (LPCTSTR pszFileName);
	CGraphicView *		GetGraphicView (void);
	CDataTreeView *	GetDataTreeView (void);

	void					Build_Emitter_List (EmitterInstanceListClass *emitter_list, LPCTSTR emitter_name, RenderObjClass *render_obj = nullptr);

	//
	//  Animation methods
	//
	void					Make_Movie (void);
	void					ResetAnimation (void);
	void					StepAnimation (int frame_inc = 1);
	void					PlayAnimation (RenderObjClass *pobj, LPCTSTR panim_name = nullptr, bool use_global_reset_flag = true, bool allow_reset = true);
	void					PlayAnimation (RenderObjClass *pobj, HAnimComboClass *pcombo, bool use_global_reset_flag = true, bool allow_reset = true);
	void					UpdateFrame (float time_slice);
	void					SetAnimationBlend (BOOL bBlend)	{ m_bAnimBlend = bBlend; }
	bool					GetChannelQCompression(){ return m_bCompress_channel_Q;}
	int					GetChannelQnBytes(){return m_nChannelQnBytes;}
	void					SetChannelQnBytes(int n_bytes){m_nChannelQnBytes = n_bytes;}
	BOOL					GetAnimationBlend (void) const	{ return m_bAnimBlend; }
	bool					Is_Camera_Animated (void) const	{ return m_bAnimateCamera; }
	void					Animate_Camera (bool banimate);
	void					Import_Facial_Animation (const CString &hierarchy_name, const CString &filename);
	void					Play_Animation_Sound (void);

	//
	//	Camera methods
	//
	bool					Is_Camera_Auto_Reset_On (void) const	{ return m_bAutoCameraReset; }
	void					Turn_Camera_Auto_Reset_On (bool onoff) { m_bAutoCameraReset = onoff; }

	//
	//  Background color methods
	//
	const Vector3 &	GetBackgroundColor (void) const								{ return m_backgroundColor; }
	void					SetBackgroundColor (const Vector3 &backgroundColor);

	//
	//  Background BMP methods
	//
	const CString &	GetBackgroundBMP  (void) const						{ return m_stringBackgroundBMP; }
	void					SetBackgroundBMP  (LPCTSTR pszBackgroundBMP);

	//
	//  Background Object methods
	//
	const CString &	GetBackgroundObjectName (void) const				{ return m_stringBackgroundObject; }
	void					SetBackgroundObject (LPCTSTR pszBackgroundObjectName);

	//
	//  Fogging methods
	//
	bool					IsFogEnabled (void) const								{ return m_bFogEnabled; }
	void					EnableFog (bool enable=true);

	//
	//	Scene methods
	//
	void					Remove_Object_From_Scene (RenderObjClass *prender_obj = nullptr);

	//
	//	Emitter serialization methods
	//
	bool					Save_Selected_Emitter (void);
	bool					Save_Current_Emitter (const CString &filename);

	//
	//	Primitive serialization methods
	//
	bool					Save_Selected_Primitive (void);
	bool					Save_Current_Sphere (const CString &filename);
	bool					Save_Current_Ring (const CString &filename);

	//
	//	Aggregate methods
	//
	void					Auto_Assign_Bones (void);
	bool					Save_Selected_Aggregate (void);
	bool					Save_Current_Aggregate (const CString &filename);

	//
	//	Sound object methods
	//
	bool					Save_Selected_Sound_Object (void);
	bool					Save_Current_Sound_Object (const CString &filename);

	//
	//  LOD methods
	//
	bool					Save_Current_LOD (const CString &filename);
	bool					Save_Selected_LOD (void);
	void					Switch_LOD (int increment = 1, RenderObjClass *render_obj = nullptr);

	//
	// Alternate Material interface.
	//
	void					Toggle_Alternate_Materials(RenderObjClass * obj = nullptr);

	//
	//	Prototype methods
	//
	void					Update_Aggregate_Prototype (RenderObjClass &render_obj);
	void					Update_LOD_Prototype (HLodClass &hlod);

	//
	//	Cursor management
	//
	void					Show_Cursor (bool onoff);
	void					Set_Cursor (LPCTSTR resource_name);
	bool					Is_Cursor_Shown (void) const;
	void					Create_Cursor (void);

	//
	//	Particle methods
	//
	int					Count_Particles (RenderObjClass *render_obj = nullptr);
	void					Update_Particle_Count (void);

	//
	//	Manual settings
	//
	void					Set_Manual_FOV (bool manual)			{ m_ManualFOV = manual; }
	void					Set_Manul_Clip_Planes  (bool manual){ m_ManualClipPlanes = manual; }
	bool					Is_FOV_Manual (void) const				{ return m_ManualFOV; }
	bool					Are_Clip_Planes_Manual (void) const	{ return m_ManualClipPlanes; }

	void					Update_Camera (void);
	void					Save_Camera_Settings (void);
	void					Load_Camera_Settings (void);

	//
	//	File methods
	//
	void					Copy_Assets_To_Dir (LPCTSTR directory);
	bool					Lookup_Path (LPCTSTR asset_name, CString &path);
	const char *		Get_Last_Path (void) const { return (m_LastPath.IsEmpty () ? nullptr : (const char *)m_LastPath); }

	//
	//	Texture search paths
	//
	const CString &	Get_Texture_Path1 (void) const { return m_TexturePath1; }
	const CString &	Get_Texture_Path2 (void) const { return m_TexturePath2; }

	void					Set_Texture_Path1 (LPCTSTR path);
	void					Set_Texture_Path2 (LPCTSTR path);

	//
	// Dazzle rendering support
	//
	void					Render_Dazzles(CameraClass * camera);

private:

	//////////////////////////////////////////////////////////////////
	//  Private member data
	//////////////////////////////////////////////////////////////////
	ViewerSceneClass *	m_pCScene;
	SceneClass *			m_pC2DScene;
	SceneClass *			m_pCursorScene;
	SceneClass *			m_pCBackObjectScene;
	DazzleLayerClass *	m_pDazzleLayer;
	RenderObjClass *		m_pCRenderObj;
	RenderObjClass *		m_pCBackgroundObject;
	HAnimClass *			m_pCAnimation;
	HAnimComboClass *		m_pCAnimCombo;
	LightClass *			m_pCSceneLight;
	Bitmap2DObjClass *	m_pCBackgroundBMP;
	CameraClass *			m_pC2DCamera;
	CameraClass *			m_pCBackObjectCamera;
	ScreenCursorClass *	m_pCursor;
	Vector3					m_backgroundColor;
	CString					m_stringBackgroundBMP;
	CString					m_stringBackgroundObject;

	bool						m_bCompress_channel_Q;
	int						m_nChannelQnBytes;
	float						m_CurrentFrame;
	float						m_animTime;
	BOOL						m_bAnimBlend;
	bool						m_bAnimateCamera;
	bool						m_bAutoCameraReset;
	bool						m_bOneTimeReset;
	bool						m_ManualFOV;
	bool						m_ManualClipPlanes;
	bool						m_IsInitialized;
	bool						m_bFogEnabled;

	CString					m_TexturePath1;
	CString					m_TexturePath2;

	CString					m_LastPath;

	DynamicVectorClass<CString>	m_LoadList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
