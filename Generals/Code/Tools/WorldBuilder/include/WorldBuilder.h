/*
**	Command & Conquer Generals(tm)
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

// WorldBuilder.h : main header file for the WORLDBUILDER application
//

#if !defined(AFX_WORLDBUILDER_H__FBA41345_2826_11D5_8CE0_00010297BBAC__INCLUDED_)
#define AFX_WORLDBUILDER_H__FBA41345_2826_11D5_8CE0_00010297BBAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "Common/STLTypedefs.h"

#include "BrushTool.h"
#include "TileTool.h"
#include "FeatherTool.h"
#include "AutoEdgeOutTool.h"
#include "FloodFillTool.h"
#include "MoundTool.h"
#include "EyedropperTool.h"
#include "ObjectTool.h"
#include "FenceTool.h"
#include "PointerTool.h"
#include "BlendEdgeTool.h"
#include "BorderTool.h"
#include "GroveTool.h"
#include "HandScrollTool.h"
#include "RoadTool.h"
#include "MeshMoldTool.h"
#include "WaypointTool.h"
#include "PolygonTool.h"
#include "WaterTool.h"
#include "BuildListTool.h"
#include "RampTool.h"
#include "ScorchTool.h"
#include "Common/Debug.h"

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderApp:
// See WorldBuilder.cpp for the implementation of this class
//

// Force maps into a directory structure.
#define DO_MAPS_IN_DIRECTORIES 1

#define NONE_STRING "<none>"


enum {THREE_D_VIEW_WIDTH=800, THREE_D_VIEW_HEIGHT=600};
enum {MAX_OBJECTS_IN_MAP = 3000};

class CWorldBuilderApp : public CWinApp
{
public:
	CWorldBuilderApp();
	~CWorldBuilderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldBuilderApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CWorldBuilderApp)
	afx_msg void OnAppAbout();
	afx_msg void OnResetWindows();
	afx_msg void OnFileOpen();
	afx_msg void OnTexturesizingMapclifftextures();
	afx_msg void OnUpdateTexturesizingMapclifftextures(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

	enum {NUM_VIEW_TOOLS=24};

	Tool							*m_tools[NUM_VIEW_TOOLS]; ///< array of tool pointers.
	Tool							*m_curTool;   ///< Currently active tool.
	Tool							*m_selTool;   ///< Normal tool.  If we hit alt, curTool turns to eyedropper.
	BrushTool					m_brushTool;				///< Height brush tool.
	TileTool					m_tileTool;					///< Single texture tile tool.
	BigTileTool				m_bigTileTool;			///< Wide texture tile tool.
	FeatherTool				m_featherTool;			///< Feather the height values tool.
	AutoEdgeOutTool		m_autoEdgeOutTool;	///< Auto blend texture edges out tool.
	FloodFillTool			m_floodFillTool;		///< Flood fill tool.
	MoundTool					m_moundTool;				///< Add height to height values tool.
	DigTool						m_digTool;					///< Remove height from height values tool.
	EyedropperTool		m_eyedropperTool;	  ///< Eyedropper tool.
	ObjectTool				m_objectTool;				///< Add and orient object tool.
	PointerTool				m_pointerTool;			///< Select and move/rotate tool.
	BlendEdgeTool			m_blendEdgeTool;		///< Blend a single edge tool.
	GroveTool					m_groveTool;				///< Plant a grove of trees tool.
	HandScrollTool		m_handScrollTool;		///< Scroll tool.
	RoadTool					m_roadTool;					///< Road tool.
	MeshMoldTool			m_meshMoldTool;			///< Mesh shaping mold tool.
	WaypointTool			m_waypointTool;			///< Waypoint tool.
	PolygonTool				m_polygonTool;			///< Polygon tool.
	WaterTool					m_waterTool;				///< Water tool.
	BuildListTool			m_buildListTool;		///< Build List tool.
	FenceTool					m_fenceTool;				///< Fence tool.
	RampTool					m_rampTool;					///< Ramp tool.
	ScorchTool				m_scorchTool;				///< Scorch tool.
	BorderTool				m_borderTool;				///< Border tool.

	Int								m_lockCurTool;

	AsciiString				m_currentDirectory; ///< Current directory for open file.

	CDocTemplate			*m_3dtemplate;

	MapObject					*m_pasteMapObjList;	///< List of copied/cut map objects.

protected:
	void deletePasteObjList(void)
	{
		if (m_pasteMapObjList)
			deleteInstance(m_pasteMapObjList);
		m_pasteMapObjList = NULL;
	};

public:

	CDocTemplate *Get3dTemplate() { return m_3dtemplate; }

	/// Set the brush tool as the active tool.
	void selectBrushTool(void) { setActiveTool(&m_brushTool); }

	/// Set the pointer tool as the active tool.
	void selectPointerTool(void);

	/// Set the hand tool as the cur tool (but not active tool)
	void selectHandToolTemp(void) { m_curTool = &m_handScrollTool; }

	/// Set the tool that will be active.
	void setActiveTool(Tool *newTool);

	/// Sets the current directry for file opens.
	void setCurrentDirectory(AsciiString dir) {m_currentDirectory = dir;};

	Tool *getCurTool() { return m_curTool; }

	/// Check to see if any keyboard overrides are changing the current tool.
	void updateCurTool(Bool forceHand);

	/// Switch to the poly tool if we aren't already.
	void setPolyTool(void){ setActiveTool(&m_polygonTool); };

	/// Return true if the hand scroll tool is active.
	Bool isHandScroll(void) {return m_curTool == &m_handScrollTool; }

	void lockCurTool()		{ DEBUG_ASSERTCRASH(!m_lockCurTool,("already locked")); m_lockCurTool = 1; }
	void unlockCurTool()	{ m_lockCurTool = 0; }
	Bool isCurToolLocked()	{ return m_lockCurTool != 0; }

	/// Note - read only data - make yourself a copy.
	MapObject *getMapObjPasteList(void) { return(m_pasteMapObjList);};

	/// Note - the app owns this, and will delete it on close.
	void setMapObjPasteList(MapObject *list) { deletePasteObjList(); m_pasteMapObjList = list; };


	/// Handles command messages.
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
						  AFX_CMDHANDLERINFO* pHandlerInfo);
};

inline CWorldBuilderApp *WbApp() { return (CWorldBuilderApp*)::AfxGetApp(); }


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORLDBUILDER_H__FBA41345_2826_11D5_8CE0_00010297BBAC__INCLUDED_)
