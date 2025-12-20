/*
**	Command & Conquer Generals Zero Hour(tm)
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

/* $Header: /Commando/Code/Tools/max2w3d/w3dexp.h 22    10/23/00 5:34p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dexp.h                       $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/23/00 5:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include <max.h>
#include "dllmain.h"
#include "hiersave.h"
#include "w3dutil.h"
#include "resource.h"


class MeshConnectionsClass;
class GeometryExportTaskClass;

class W3dExportClass : public SceneExport
{

public:

	W3dExportClass() {};
	~W3dExportClass() {};

	int				ExtCount()				{ return 1; };
	const TCHAR *	Ext(int n)				{ return Get_String(IDS_W3D_FILE_EXTEN); };
	const TCHAR *	LongDesc()				{ return Get_String(IDS_W3D_LONG_DESCRIPTION); };
	const TCHAR *	ShortDesc()				{ return Get_String(IDS_W3D_SHORT_DESCRIPTION); };
	const TCHAR *	AuthorName()			{ return Get_String(IDS_AUTHOR_NAME); };
	const TCHAR *	CopyrightMessage()	{ return Get_String(IDS_COPYRIGHT_NOTICE); };
	const TCHAR *	OtherMessage1()		{ return _T(""); };
	const TCHAR *	OtherMessage2()		{ return _T(""); };
	unsigned int	Version()				{ return 100; };

	void				ShowAbout(HWND hWnd)	{};
	int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);	// Export file

protected:

	void				DoOriginBasedExport(char *rootname, ChunkSaveClass &csave);

public:

	friend BOOL CALLBACK ExportOptionsDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

	static char CurrentExportPath[_MAX_DRIVE + _MAX_DIR + 1];	// Used to communicate from the exporter to the dialog.
	char CurrentScenePath[_MAX_DRIVE + _MAX_DIR + 1]; // directory where the current .max file is stored

private:

	ExpInterface *			ExportInterface;
	Interface *				MaxInterface;
	TimeValue				CurTime;
	int						FrameRate;

	W3dExportOptionsStruct	ExportOptions;

	char						HierarchyFilename[_MAX_PATH];
	int						FixupType;
	INode *					RootNode;
	INodeListClass *		RootList;
	INodeListClass *		OriginList;
	INodeListClass *		DamageRootList;

	HierarchySaveClass *	HierarchyTree;

	bool get_export_options(BOOL suppress_prompts = FALSE);
	INodeListClass * get_origin_list(void);
	INodeListClass * get_damage_root_list(void);
	HierarchySaveClass * get_hierarchy_tree(void);

	bool get_base_object_tm(Matrix3 &tm);

	bool Export_Hierarchy(char * name,ChunkSaveClass & csave,Progress_Meter_Class & meter,INode *root);
	bool Export_Animation(char * name,ChunkSaveClass & csave,Progress_Meter_Class & meter,INode *root);
	bool Export_Damage_Animations(char * name,ChunkSaveClass & csave,Progress_Meter_Class &meter,INode *damage_root);
	bool Export_Geometry(char * name,ChunkSaveClass & csave,Progress_Meter_Class & meter,INode *root=NULL, MeshConnectionsClass **out_connection=NULL);
	bool Export_HLod (char *name, const char *htree_name, ChunkSaveClass &csave, Progress_Meter_Class &meter, MeshConnectionsClass **connections, int lod_count);
	bool Export_Collection(const char * name,ChunkSaveClass & csave,DynamicVectorClass<GeometryExportTaskClass *> & objlist,INodeListClass & placeholder_list,INodeListClass & transform_node_list);

	void Start_Progress_Bar(void);
	void End_Progress_Bar(void);

};
