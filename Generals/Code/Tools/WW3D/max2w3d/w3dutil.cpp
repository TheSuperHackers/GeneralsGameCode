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

/* $Header: /Commando/Code/Tools/max2w3d/w3dutil.cpp 45    8/21/01 10:28a Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dutil.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 8/21/01 9:41a                                               $* 
 *                                                                                             * 
 *                    $Revision:: 45                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "w3dutil.h"
#include "w3ddesc.h"
#include "rcmenu.h"
#include "util.h"
#include "nodelist.h"
#include "tchar.h"
#include "gamemtl.h"
#include "notify.h"
#include "gennamesdialog.h"
#include "genmtlnamesdialog.h"
#include "genlodextensiondialog.h"
#include "floaterdialog.h"
#include <stdmat.h>


#define DAZZLE_SETTINGS_FILENAME		"dazzle.ini"
#define DAZZLE_TYPES_SECTION			"Dazzles_List"
#define DAZZLE_SECTION_BUFFERSIZE	32767


static BOOL CALLBACK _settings_form_dlg_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK _w3d_utility_tools_dlg_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static VisibleSelectedINodeFilter _INodeFilter;


/**********************************************************************************************
**
** MaterialReferenceMaker - Class to support Export Utilities in the W3D Tools panel.
**
**********************************************************************************************/
class MaterialReferenceMaker : public ReferenceMaker
{
	public:

		~MaterialReferenceMaker() {DeleteAllRefs();}

		virtual int					NumRefs();
		virtual RefTargetHandle GetReference (int i);
		virtual void				SetReference (int i, RefTargetHandle rtarg);
		RefResult					NotifyRefChanged (Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);

		Mtl		  *MaterialPtr;
		static int  ReferenceCount;
};

int MaterialReferenceMaker::NumRefs()
{
	return (ReferenceCount);
}

RefTargetHandle MaterialReferenceMaker::GetReference (int i)
{
	assert (i < ReferenceCount);
	return (MaterialPtr);
}

void MaterialReferenceMaker::SetReference (int i, RefTargetHandle rtarg)
{
	assert (i < ReferenceCount);
	MaterialPtr = (Mtl*) rtarg;
}

RefResult MaterialReferenceMaker::NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message)
{
	return (REF_SUCCEED); 
}

int MaterialReferenceMaker::ReferenceCount;


/**********************************************************************************************
**
** SettingsFormClass - code for the W3DUTILITY_SETTINGS_DIALOG.  Used in the command panel
** and in the floating settings dialog.
**
**********************************************************************************************/
class SettingsFormClass
{
public:

	SettingsFormClass(HWND hwnd);
	~SettingsFormClass(void);

	bool		Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);
	void		Selection_Changed(void);

	static void	Update_All_Instances(void);

private:

	void		Init(void);
	void		Destroy(void);
	void		Disable_Controls(void);
	void		Update_Controls(INodeListClass * nodelist = NULL);

	HWND						Hwnd;
	ISpinnerControl *		RegionSpin;
	SettingsFormClass *	Next;

	static SettingsFormClass *	ActiveList;

};


/**********************************************************************************************
**
** W3DUtilityClass - Utility plugin which presents windows controls for setting all of
** the W3D export options for the currently selected nodes
**
**********************************************************************************************/
class W3DUtilityClass : public UtilityObj 
{
public:

	W3DUtilityClass();
	~W3DUtilityClass();

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);
	void SelectionSetChanged(Interface *ip,IUtil *iu);
	void DeleteThis() {}

	void Select_Hierarchy(void);
	void Select_Geometry(void);
	void Select_Alpha(void);
	void Select_Physical(void);
	void Select_Projectile(void);
	void Select_Vis(void);
	
public:
	
	Interface *							InterfacePtr;
	FloaterDialogClass				SettingsFloater;
	HWND									SettingsPanelHWND;
	HWND									ToolsPanelHWND;
	bool									UpdateSpinnerValue;

	GenNamesDialogClass::OptionsStruct			NameOptions;
	GenMtlNamesDialogClass::OptionsStruct		MtlNameOptions;
	GenLodExtensionDialogClass::OptionsStruct	LodExtensionOptions;
	
	int											WorkingNameIndex;

	enum {
		NONE = 0,
		HIDE,
		SELECT_GEOM,
		SELECT_HIER,
		SELECT_ALPHA,
		SELECT_PHYSICAL,
		SELECT_PROJECTILE,
		SELECT_VIS,
		GENERATE_NAME,
		GENERATE_MATERIAL_NAME,
		GENERATE_LOD_EXTENSION
	};

	enum MaterialConversionEnum {
		GAME_REFERENCE_COUNT,
		GAME_TO_STANDARD,
		STANDARD_TO_GAME
	};

	struct NodeStatesStruct
	{
		int				ExportHierarchy;
		int				ExportGeometry;
		int				GeometryTwoSided;
		int				GeometryHidden;
		int				GeometryZNormals;
		int				GeometryVertexAlpha;
		int				GeometryCastShadow;
		int				GeometryShatterable;
		int				GeometryNPatch;
		int				CollisionPhysical;
		int				CollisionProjectile;
		int				CollisionVis;
		int				CollisionCamera;
		int				CollisionVehicle;
		bool				GeometryCameraAligned;
		bool				GeometryCameraOriented;
		bool				GeometryNormal;
		bool				GeometryAABox;
		bool				GeometryOBBox;
		bool				GeometryNull;
		bool				GeometryDazzle;
		bool				GeometryAggregate;

		int				DamageRegion;
		int				DazzleCount;
		char				DazzleType[128];
	};

	/*
	** Evaluate the status of nodes in the given list
	*/
	static int		eval_tri_state(int totalcount,int oncount);
	static void		eval_node_states(INodeListClass * node_list,NodeStatesStruct * ns);

	/*
	** Update the controls in any active settings panels
	*/
	static void		update_settings_controls(INodeListClass * node_list = NULL);

	/*
	** Modify the state of all selected nodes
	*/
	static void		set_hierarchy_in_all_selected(INodeListClass * list,bool onoff);
	static void		set_geometry_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_hidden_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_two_sided_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_znormals_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_vertex_alpha_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_shadow_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_shatterable_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_npatches_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_physical_collision_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_projectile_collision_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_vis_collision_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_camera_collision_in_all_selected(INodeListClass * list,bool onoff);
	static void		enable_vehicle_collision_in_all_selected(INodeListClass * list,bool onoff);

	static void		set_geometry_type_in_all_selected(INodeListClass * list,int geotype);
	static void		set_dazzle_type_in_all_selected(INodeListClass * list,char * dazzletype);

	static void		set_region_in_all_selected(INodeListClass * list,char region);

	/*
	** Functions used by the tools rollup in the command panel
	*/
	void		descend_tree(INode * node,int action);
	void		hide_node(INode * node);
	void		select_geometry_node(INode * node);
	void		select_hierarchy_node(INode * node);
	void		select_alpha_node(INode * node);
	void		select_physical_node(INode * node);
	void		select_projectile_node(INode * node);
	void		select_vis_node(INode * node);
	bool		is_alpha_material(Mtl * nodemtl);
	bool		is_alpha_mesh(INode * node,Mtl * nodemtl);

	void		generate_names(void);
	void		generate_node_name(INode * node);
	void		generate_material_names(void);
	void		generate_material_names_for_node(INode * node);
	void		generate_material_names(Mtl * mtl);
	void		generate_lod_extensions(void);
	void		generate_lod_ext(INode * node);
	
	void		create_floater(void);

	void		export_with_standard_materials();
	int		convert_materials (MaterialConversionEnum conversion, MaterialReferenceMaker *gamenodematerials);
	StdMat *	new_standard_material (GameMtl *gamemtl);

	static W3DAppData0Struct *			get_app_data_0(INode * node);
	static W3DAppData1Struct *			get_app_data_1(INode * node);
	static W3DAppData2Struct *			get_app_data_2(INode * node);
	static W3DDazzleAppDataStruct *	get_dazzle_app_data(INode * node);
};

static W3DUtilityClass TheW3DUtility;


/**********************************************************************************************
**
** W3DUtilityClassDesc - Class Descriptor for the W3D Utility
**
**********************************************************************************************/
class W3DUtilityClassDesc:public ClassDesc 
{
public:

	int 				IsPublic()								{ return 1; }
	void *			Create(BOOL loading = FALSE)		{ return &TheW3DUtility; }
	const TCHAR *	ClassName()								{ return Get_String(IDS_W3D_UTILITY_CLASS_NAME); }
	SClass_ID		SuperClassID()							{ return UTILITY_CLASS_ID; }
	Class_ID			ClassID()								{ return W3DUtilityClassID; }
	const TCHAR* 	Category()								{ return Get_String(IDS_W3DMENU_CATEGORY); }
};

static W3DUtilityClassDesc W3DUtilityDesc;

ClassDesc * Get_W3D_Utility_Desc(void) 
{ 
	return &W3DUtilityDesc; 
}

/**********************************************************************************************
**
** W3DUtilityClass Implementation
**
**********************************************************************************************/
W3DUtilityClass::W3DUtilityClass(void)
{
	InterfacePtr = NULL;	
	SettingsPanelHWND = NULL;
	ToolsPanelHWND = NULL;
	UpdateSpinnerValue = true;
}

W3DUtilityClass::~W3DUtilityClass(void)
{
}

void W3DUtilityClass::BeginEditParams(Interface *ip,IUtil *iu) 
{
	InterfacePtr = ip;
	
	SettingsPanelHWND = InterfacePtr->AddRollupPage(
		AppInstance,
		MAKEINTRESOURCE(IDD_W3DUTILITY_SETTINGS_DIALOG),
		_settings_form_dlg_proc,
		Get_String(IDS_W3DUTILITY_SETTINGS),
		0);

	ToolsPanelHWND = InterfacePtr->AddRollupPage(
		AppInstance,
		MAKEINTRESOURCE(IDD_W3DUTILITY_TOOLS_DIALOG),
		_w3d_utility_tools_dlg_proc,
		Get_String(IDS_W3DUTILITY_TOOLS),
		0);


//	TheRCMenu.Bind(TheW3DUtility.InterfacePtr,&TheW3DUtility);
//	RightClickMenuManager *rcm = ip->GetRightClickMenuManager();
//	if (TheRCMenu.Installed!=TRUE) { 
//		rcm->Register(&TheRCMenu);
//	}

	SettingsFormClass::Update_All_Instances();
}
	
void W3DUtilityClass::EndEditParams(Interface *ip,IUtil *iu) 
{
	InterfacePtr = NULL;

	ip->DeleteRollupPage(SettingsPanelHWND);
	ip->DeleteRollupPage(ToolsPanelHWND);
	
	SettingsPanelHWND = NULL;
	ToolsPanelHWND = NULL;
}

void W3DUtilityClass::SelectionSetChanged(Interface *ip,IUtil *iu)
{
	// (gth) the settings panels which need to respond to the selection set changing
	// are now registered directly with MAX and don't need to be updated here
	//	update_dialog();
}

int W3DUtilityClass::eval_tri_state(int totalcount,int oncount)
{
	if (oncount == 0) {
		return 0;
	}
	if (oncount == totalcount) {
		return 1;
	}
	return 2;
}

void W3DUtilityClass::eval_node_states(INodeListClass * list,NodeStatesStruct * ns)
{
	// initialize the counters and booleans
	ns->ExportHierarchy = 0;
	ns->ExportGeometry = 0;
	ns->GeometryHidden = 0;
	ns->GeometryTwoSided = 0;
	ns->GeometryZNormals = 0;
	ns->GeometryVertexAlpha = 0;
	ns->GeometryCastShadow = 0;
	ns->GeometryShatterable = 0;
	ns->GeometryNPatch = 0;
	ns->CollisionPhysical = 0;
	ns->CollisionProjectile = 0;
	ns->CollisionVis = 0;
	ns->CollisionCamera = 0;
	ns->CollisionVehicle = 0;
	
	ns->GeometryCameraAligned = false;
	ns->GeometryCameraOriented = false;
	ns->GeometryNormal = false;
	ns->GeometryAABox = false;
	ns->GeometryOBBox = false;
	ns->GeometryNull = false;
	ns->GeometryDazzle = false;
	ns->GeometryAggregate = false;

	/*
	** ns->DamageRegion will be MAX_DAMAGE_REGIONS if not all
	** of the selected nodes are in the same damage region. If
	** they are, then ns->DamageRegion will be the region they
	** share.
	*/
	if (list->Num_Nodes() > 0)
	{
		// Use the first damage region for comparing to the others.
		W3DAppData1Struct *wdata = get_app_data_1((*list)[0]);
		ns->DamageRegion = wdata->DamageRegion;
	}
	else
		ns->DamageRegion = MAX_DAMAGE_REGIONS;

	/*
	** ns->DazzleType will be DEFAULT if not all of the selected
	** nodes are the same.  If they are the same, it will be the
	** dazzle type that they all share
	*/
	ns->DazzleCount = 0;
	if (list->Num_Nodes() > 0) {
		W3DDazzleAppDataStruct * dazzledata = get_dazzle_app_data((*list)[0]);
		strncpy(ns->DazzleType,dazzledata->DazzleType,sizeof(ns->DazzleType));
	} else {
		strcpy(ns->DazzleType,"DEFAULT");
	}

	/*
	** evaluate each node
	*/
	for (unsigned int ni=0; ni<list->Num_Nodes(); ni++) {
		
		W3DAppData2Struct * wdata = get_app_data_2((*list)[ni]);
		assert(wdata);

		ns->ExportHierarchy +=		(wdata->Is_Bone() ? 1 : 0);
		ns->ExportGeometry +=		(wdata->Is_Geometry()  ? 1 : 0);
		ns->GeometryHidden +=		(wdata->Is_Hidden_Enabled() ? 1 : 0);
		ns->GeometryTwoSided +=		(wdata->Is_Two_Sided_Enabled() ? 1 : 0);
		ns->GeometryZNormals +=		(wdata->Is_ZNormals_Enabled() ? 1 : 0);
		ns->GeometryVertexAlpha +=	(wdata->Is_Vertex_Alpha_Enabled() ? 1 : 0);
		ns->GeometryCastShadow +=	(wdata->Is_Shadow_Enabled() ? 1 : 0);
		ns->GeometryShatterable += (wdata->Is_Shatterable_Enabled() ? 1 : 0);
		ns->GeometryNPatch +=		(wdata->Is_NPatchable_Enabled() ? 1 : 0);
		ns->CollisionPhysical +=	(wdata->Is_Physical_Collision_Enabled() ? 1 : 0);
		ns->CollisionProjectile +=	(wdata->Is_Projectile_Collision_Enabled() ? 1 : 0);				
		ns->CollisionVis +=			(wdata->Is_Vis_Collision_Enabled() ? 1 : 0);
		ns->CollisionCamera +=		(wdata->Is_Camera_Collision_Enabled() ? 1 : 0);
		ns->CollisionVehicle +=		(wdata->Is_Vehicle_Collision_Enabled() ? 1 : 0);

		switch (wdata->Get_Geometry_Type()) {
		case W3DAppData2Struct::GEO_TYPE_CAMERA_ALIGNED:	ns->GeometryCameraAligned = true;	break;
		case W3DAppData2Struct::GEO_TYPE_CAMERA_ORIENTED:	ns->GeometryCameraOriented = true;	break;
		case W3DAppData2Struct::GEO_TYPE_NORMAL_MESH:		ns->GeometryNormal = true;				break;
		case W3DAppData2Struct::GEO_TYPE_AABOX:				ns->GeometryAABox = true;				break;
		case W3DAppData2Struct::GEO_TYPE_OBBOX:				ns->GeometryOBBox = true;				break;
		case W3DAppData2Struct::GEO_TYPE_NULL:					ns->GeometryNull = true;				break;
		case W3DAppData2Struct::GEO_TYPE_DAZZLE:				ns->GeometryDazzle = true;	ns->DazzleCount++; break;
		case W3DAppData2Struct::GEO_TYPE_AGGREGATE:			ns->GeometryAggregate = true;			break;
		}

		// Compare this damage region to our existing one. If it's not the same,
		// use MAX_DAMAGE_REGION (an invalid value) as a sentinel value.
		if (ns->DamageRegion != MAX_DAMAGE_REGIONS)
		{
			W3DAppData1Struct *wdata1 = get_app_data_1((*list)[ni]);
			assert(wdata1);
			if (wdata1->DamageRegion != ns->DamageRegion)
				ns->DamageRegion = MAX_DAMAGE_REGIONS;
		}

		// compare this objects dazzle type to our existing one.  If its not
		// the same, use 'DEFAULT'.
		W3DDazzleAppDataStruct * dazzledata = get_dazzle_app_data((*list)[ni]);
		if (strcmp(ns->DazzleType,dazzledata->DazzleType) != 0) {
			strcpy(ns->DazzleType,"DEFAULT");
		}
	}	

	// If any of the counters are zero, that means none of the objects had that
	// bit set.  If any of them are equal to the number of objects, then they
	// all had that bit set.  Otherwise, there was a mix and we should use the
	// third state for the checkbox (greyed out check).
	int count = list->Num_Nodes();
	ns->ExportHierarchy = eval_tri_state(count, ns->ExportHierarchy);
	ns->ExportGeometry = eval_tri_state(count, ns->ExportGeometry);
	ns->GeometryHidden = eval_tri_state(count, ns->GeometryHidden);
	ns->GeometryTwoSided = eval_tri_state(count, ns->GeometryTwoSided);
	ns->GeometryZNormals = eval_tri_state(count, ns->GeometryZNormals);
	ns->GeometryVertexAlpha = eval_tri_state(count, ns->GeometryVertexAlpha);
	ns->GeometryCastShadow = eval_tri_state(count, ns->GeometryCastShadow);
	ns->GeometryShatterable = eval_tri_state(count, ns->GeometryShatterable);
	ns->GeometryNPatch = eval_tri_state(count, ns->GeometryNPatch);
	ns->CollisionPhysical = eval_tri_state(count, ns->CollisionPhysical);
	ns->CollisionProjectile = eval_tri_state(count, ns->CollisionProjectile);
	ns->CollisionVis = eval_tri_state(count, ns->CollisionVis);
	ns->CollisionCamera = eval_tri_state(count, ns->CollisionCamera);
	ns->CollisionVehicle = eval_tri_state(count, ns->CollisionVehicle);

}

void W3DUtilityClass::update_settings_controls(INodeListClass * node_list)
{
	SettingsFormClass::Update_All_Instances();
}

void	W3DUtilityClass::set_hierarchy_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Export_Transform(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::set_geometry_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Export_Geometry(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_hidden_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Hidden(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_two_sided_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Two_Sided(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_znormals_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_ZNormals(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_vertex_alpha_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Vertex_Alpha(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_shadow_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Shadow(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_shatterable_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Shatterable(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_npatches_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_NPatchable(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_physical_collision_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Physical_Collision(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_projectile_collision_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Projectile_Collision(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_vis_collision_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Vis_Collision(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_camera_collision_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Camera_Collision(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::enable_vehicle_collision_in_all_selected(INodeListClass * node_list,bool onoff)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Enable_Vehicle_Collision(onoff);
	}
	update_settings_controls(node_list);
}

void	W3DUtilityClass::set_geometry_type_in_all_selected(INodeListClass * node_list,int geotype)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DAppData2Struct * wdata = get_app_data_2((*node_list)[ni]);
		wdata->Set_Geometry_Type((W3DAppData2Struct::GeometryTypeEnum)geotype);
	}
	update_settings_controls(node_list);
}

void W3DUtilityClass::set_dazzle_type_in_all_selected(INodeListClass * node_list,char * dazzle_type)
{
	for (unsigned int ni=0; ni<node_list->Num_Nodes(); ni++) {
		W3DDazzleAppDataStruct * dazzledata = get_dazzle_app_data((*node_list)[ni]);
		strncpy(dazzledata->DazzleType,dazzle_type,sizeof(dazzledata->DazzleType) - 1);
	}
	update_settings_controls(node_list);
}

void W3DUtilityClass::set_region_in_all_selected(INodeListClass * list,char region)
{
	if (list->Num_Nodes() == 0) return;

	// Damage regions are stored in each node's AppData1.
	for (int i = 0; i < list->Num_Nodes(); i++)
	{
		W3DAppData1Struct *wdata = get_app_data_1((*list)[i]);
		wdata->DamageRegion = region;
	}
	update_settings_controls(list);
}

void W3DUtilityClass::generate_names(void)
{
	GenNamesDialogClass dialog(InterfacePtr);
	bool retval = dialog.Get_Options(&NameOptions);
	WorkingNameIndex = NameOptions.NameIndex;

	if (retval) {
		descend_tree(InterfacePtr->GetRootNode(),GENERATE_NAME);
	}
}

void W3DUtilityClass::generate_material_names(void)
{
	GenMtlNamesDialogClass dialog(InterfacePtr);
	bool retval = dialog.Get_Options(&MtlNameOptions);
	WorkingNameIndex = MtlNameOptions.NameIndex;

	if (retval) {
		descend_tree(InterfacePtr->GetRootNode(),GENERATE_MATERIAL_NAME);
	}
}

void W3DUtilityClass::generate_lod_extensions(void)
{
	GenLodExtensionDialogClass dialog(InterfacePtr);
	bool retval = dialog.Get_Options(&LodExtensionOptions);

	if (retval) {
		descend_tree(InterfacePtr->GetRootNode(),GENERATE_LOD_EXTENSION);
	}
}

void W3DUtilityClass::generate_lod_ext(INode * node)
{
	/*
	** Only works on selected nodes.
	*/
	if (!node->Selected())
		return;

	/*
	** If this node already has an LOD extension, we'll replace it
	** with the new LOD index. Otherwise we'll tack it on the end.
	** Display a error message if the name is too long to append
	** the extension, and skip the node without changing the name.
	*/
	char	msg[256];
	char	newname[W3D_NAME_LEN];
	char	*oldname = node->GetName();
	char	*ext = strrchr(oldname, '.');
	int	old_lod;
	if ( (ext != NULL) && (sscanf(ext, ".%d", &old_lod) == 1) )
	{
		/*
		** An existing LOD index. If it's different than the new
		** one, replace it.
		*/
		if (old_lod == LodExtensionOptions.LodIndex)
			return;	// same lod index

		/*
		** Room for the new extension? (2 because when we export, the extension will,
		** be replaced by a single character [A..Z] to indicate the LOD level.
		** ie. 2==strlen("A")+1)
		*/
		if (ext - oldname + 2 <= W3D_NAME_LEN)
		{
			*ext = '\0';
			sprintf(newname, "%s.%02d", oldname, LodExtensionOptions.LodIndex);
			*ext = '.';
			node->SetName(newname);
		}
		else
		{
			*ext = '\0';
			sprintf(msg, "The maximum W3D object name is %d characters. Adding the LOD "
				"extension to \"%s\" will pass this limit! Please shorten its name.",
				W3D_NAME_LEN - 1, oldname);
			*ext = '.';
			MessageBox(NULL, msg, "Error", MB_OK);
		}
	}
	else
	{
		/*
		** Room for the new extension? (2 because when we export, the extension will,
		** be replaced by a single character [A..Z] to indicate the LOD level.
		** ie. 2==strlen("A")+1)
		*/
		if (strlen(oldname) + 2 <= W3D_NAME_LEN)
		{
			sprintf(newname, "%s.%02d", oldname, LodExtensionOptions.LodIndex);
			node->SetName(newname);
		}
		else
		{
			sprintf(msg, "The maximum W3D object name is %d characters. Adding the LOD "
				"extension to \"%s\" will pass this limit! Please shorten its name.",
				W3D_NAME_LEN - 1, oldname);
			MessageBox(NULL, msg, "Error", MB_OK);
		}
	}
}

void W3DUtilityClass::create_floater(void)
{
	SettingsFloater.Create(InterfacePtr,IDD_W3DUTILITY_SETTINGS_DIALOG,_settings_form_dlg_proc);
	SettingsFormClass::Update_All_Instances();
}

void W3DUtilityClass::export_with_standard_materials()
{
	char *convertingmessage = "Converting materials...";

	// Count the no. of references to game materials.
	MaterialReferenceMaker::ReferenceCount = convert_materials (GAME_REFERENCE_COUNT, NULL);
	
	MaterialReferenceMaker *gamenodematerials = NULL;

	if (MaterialReferenceMaker::ReferenceCount > 0) {
		gamenodematerials = new MaterialReferenceMaker [MaterialReferenceMaker::ReferenceCount];
		assert (gamenodematerials != NULL);
	}

	InterfacePtr->PushPrompt (convertingmessage);
	SetCursor (LoadCursor (NULL, IDC_WAIT));
	convert_materials (GAME_TO_STANDARD, gamenodematerials);
	InterfacePtr->PopPrompt();
	InterfacePtr->FileExport();
	UpdateWindow (InterfacePtr->GetMAXHWnd());
	InterfacePtr->PushPrompt (convertingmessage);
	SetCursor (LoadCursor (NULL, IDC_WAIT));
	convert_materials (STANDARD_TO_GAME, gamenodematerials);
	InterfacePtr->PopPrompt();

	// Clean-up.
	if (gamenodematerials != NULL) delete [] gamenodematerials;
}

int W3DUtilityClass::convert_materials (MaterialConversionEnum conversion, MaterialReferenceMaker *gamenodematerials)
{
	int gamenodematerialindex = 0;
	
	INode *rootnode = InterfacePtr->GetRootNode();
	if (rootnode != NULL) {

		INodeListClass *meshlist = new INodeListClass (rootnode, 0);
		if (meshlist != NULL) {

			for (unsigned nodeindex = 0; nodeindex < meshlist->Num_Nodes(); nodeindex++) {

				Mtl *nodemtl = ((*meshlist) [nodeindex])->GetMtl();

				// Is this a non-null material?
				if (nodemtl != NULL) {
					
					// Is this not a multi-material?
					if (!nodemtl->IsMultiMtl()) {
						
						switch (conversion) {

							case GAME_REFERENCE_COUNT:
								if (nodemtl->ClassID() == GameMaterialClassID) {
									assert (((GameMtl*) nodemtl)->Substitute_Material() == NULL);
								}
								break;

							case GAME_TO_STANDARD:
   								
								if (nodemtl->ClassID() == GameMaterialClassID) {
									
									// Make a reference to the game material to ensure that it is not deleted by the system.
									gamenodematerials [gamenodematerialindex].MakeRefByID (FOREVER, gamenodematerialindex, nodemtl);

									// Does this material already have an equivalent standard material?
									if (((GameMtl*) nodemtl)->Substitute_Material() == NULL) {
										((GameMtl*) nodemtl)->Set_Substitute_Material (new_standard_material ((GameMtl*) nodemtl));
									}
									((*meshlist) [nodeindex])->SetMtl (((GameMtl*) nodemtl)->Substitute_Material());

								} else {
									gamenodematerials [gamenodematerialindex].MaterialPtr = NULL;
								}	
								break;

							case STANDARD_TO_GAME:

								// Change materials to game materials if they were previously game materials before being
								// converted to standard materials.
								if (gamenodematerials [gamenodematerialindex].MaterialPtr != NULL) {
									((*meshlist) [nodeindex])->SetMtl (gamenodematerials [gamenodematerialindex].MaterialPtr);
									((GameMtl*) gamenodematerials [gamenodematerialindex].MaterialPtr)->Set_Substitute_Material (NULL);
								}
								break;
						}
						gamenodematerialindex++;

					} else {

						// For each sub-material...
						for (int materialindex = 0; materialindex < nodemtl->NumSubMtls(); materialindex++) {

							Mtl *submaterial = nodemtl->GetSubMtl (materialindex);
							
							// Is this a non-null submaterial?
							if (submaterial != NULL) {
							
								switch (conversion) {
									
									case GAME_REFERENCE_COUNT:
										if (submaterial->ClassID() == GameMaterialClassID) {
											assert (((GameMtl*) submaterial)->Substitute_Material() == NULL);
										}
										break;

									case GAME_TO_STANDARD:
								
										if (submaterial->ClassID() == GameMaterialClassID) {
									
											// Make a reference to the game material to ensure that it is not deleted by the system.
											gamenodematerials [gamenodematerialindex].MakeRefByID (FOREVER, gamenodematerialindex, submaterial);
										
											// Does this material already have an equivalent standard material?
											if (((GameMtl*) submaterial)->Substitute_Material() == NULL) {
 												((GameMtl*) submaterial)->Set_Substitute_Material (new_standard_material ((GameMtl*) submaterial));
											}
											nodemtl->SetSubMtl (materialindex, ((GameMtl*) submaterial)->Substitute_Material());
										
										} else {
											gamenodematerials [gamenodematerialindex].MaterialPtr = NULL;
										}	
										break;

									case STANDARD_TO_GAME:

										// Change materials to game materials if they were previously game materials before being
										// converted to standard materials.
										if (gamenodematerials [gamenodematerialindex].MaterialPtr != NULL) {
											nodemtl->SetSubMtl (materialindex, gamenodematerials [gamenodematerialindex].MaterialPtr);
											((GameMtl*) gamenodematerials [gamenodematerialindex].MaterialPtr)->Set_Substitute_Material (NULL);
										}
										break;
								}
								gamenodematerialindex++;
							}
						}
					}
				}
			}

			// Clean-up.
			delete meshlist;
		}
	}

	return (gamenodematerialindex);
}

StdMat *W3DUtilityClass::new_standard_material (GameMtl *gamemtl)
{
	Color emissive;

	// Create a new standard material.
	StdMat *stdmtl = NewDefaultStdMat();

	// Set its properties by translating the supplied game material.
	// NOTE 0: Only consider pass 0 in the game material - ignore all other passes.
	// NOTE 1: Use defaults for all standard material attributes that cannot be
	//			  converted from the game material in a meaningful way.
	stdmtl->SetName (gamemtl->GetName());
	stdmtl->SetAmbient (gamemtl->GetAmbient(), 0);
	stdmtl->SetDiffuse (gamemtl->GetDiffuse(), 0);
	stdmtl->SetSpecular (gamemtl->GetSpecular(), 0);
	stdmtl->SetOpacity (gamemtl->Get_Opacity (0, 0), 0);
	stdmtl->SetShininess (gamemtl->Get_Shininess (0, 0), 0);
	stdmtl->SetShinStr (gamemtl->GetShinStr(), 0);
	stdmtl->SetSubTexmap (ID_DI, gamemtl->Get_Texture (0, 0));
	emissive = gamemtl->Get_Emissive (0, 0);
	stdmtl->SetSelfIllum ((emissive.r + emissive.g + emissive.b) / 3.0f, 0);

	return (stdmtl);
}   

void W3DUtilityClass::Select_Hierarchy(void)
{
	InterfacePtr->SelectNode(NULL);
	INode * root = InterfacePtr->GetRootNode();
	descend_tree(root,SELECT_HIER);
	InterfacePtr->ForceCompleteRedraw();
}

void W3DUtilityClass::Select_Geometry(void)
{
	InterfacePtr->SelectNode(NULL);
	INode * root = InterfacePtr->GetRootNode();
	descend_tree(root,SELECT_GEOM);
	InterfacePtr->ForceCompleteRedraw();
}

void W3DUtilityClass::Select_Alpha(void)
{
	InterfacePtr->SelectNode(NULL);
	INode * root = InterfacePtr->GetRootNode();
	descend_tree(root,SELECT_ALPHA);
	InterfacePtr->ForceCompleteRedraw();
}

void W3DUtilityClass::Select_Physical(void)
{
	InterfacePtr->SelectNode(NULL);
	INode * root = InterfacePtr->GetRootNode();
	descend_tree(root,SELECT_PHYSICAL);
	InterfacePtr->ForceCompleteRedraw();
}

void W3DUtilityClass::Select_Projectile(void)
{
	InterfacePtr->SelectNode(NULL);
	INode * root = InterfacePtr->GetRootNode();
	descend_tree(root,SELECT_PROJECTILE);
	InterfacePtr->ForceCompleteRedraw();
}

void W3DUtilityClass::Select_Vis(void)
{
	InterfacePtr->SelectNode(NULL);
	INode * root = InterfacePtr->GetRootNode();
	descend_tree(root,SELECT_VIS);
	InterfacePtr->ForceCompleteRedraw();
}

void W3DUtilityClass::descend_tree(INode * node,int func)
{
	if (!node) return;
	
	switch (func) 
	{
	case HIDE:
		hide_node(node);
		break;

	case SELECT_GEOM:
		select_geometry_node(node);
		break;

	case SELECT_HIER:
		select_hierarchy_node(node);
		break;

	case SELECT_ALPHA:
		select_alpha_node(node);
		break;

	case SELECT_PHYSICAL:
		select_physical_node(node);
		break;

	case SELECT_PROJECTILE:
		select_projectile_node(node);
		break;

	case SELECT_VIS:
		select_vis_node(node);
		break;

	case GENERATE_NAME:
		generate_node_name(node);
		break;

	case GENERATE_MATERIAL_NAME:
		generate_material_names_for_node(node);
		break;

	case GENERATE_LOD_EXTENSION:
		generate_lod_ext(node);
		break;

	default:
		break;
	};

	for (int i=0; i<node->NumberOfChildren(); i++) {
		INode * child = node->GetChildNode(i);
		descend_tree(child,func);
	}
}

void W3DUtilityClass::hide_node(INode * node) 
{
	if (!node->IsHidden()) node->Hide(TRUE);
	InterfacePtr->NodeInvalidateRect(node);
}

void W3DUtilityClass::select_geometry_node(INode * node) 
{
	if (Is_Geometry(node) && !node->IsHidden()) {
		InterfacePtr->SelectNode(node,0);
	}
}

void W3DUtilityClass::select_hierarchy_node(INode * node) 
{
	if (Is_Bone(node) && !node->IsHidden()) {
		InterfacePtr->SelectNode(node,0);
	}
}

void W3DUtilityClass::select_alpha_node(INode * node) 
{
	if (node->IsHidden() || !Is_Geometry(node)) {
		return;
	}

	Mtl * nodemtl = node->GetMtl();
	if (is_alpha_material(nodemtl)) {
		if (is_alpha_mesh(node,nodemtl)) {
			InterfacePtr->SelectNode(node,0);
		}
	}
}

void W3DUtilityClass::select_physical_node(INode * node) 
{
	if (!node->IsHidden() && Is_Geometry(node) && Is_Physical_Collision(node)) {
		InterfacePtr->SelectNode(node,0);
	}
}

void W3DUtilityClass::select_projectile_node(INode * node) 
{
	if (!node->IsHidden() && Is_Geometry(node) && Is_Projectile_Collision(node)) {
		InterfacePtr->SelectNode(node,0);
	}
}

void W3DUtilityClass::select_vis_node(INode * node) 
{
	if (!node->IsHidden() && Is_Geometry(node) && Is_Vis_Collision(node)) {
		InterfacePtr->SelectNode(node,0);
	}
}

bool W3DUtilityClass::is_alpha_material(Mtl * nodemtl)
{
	if (nodemtl == NULL) {
		return false;
	}

	bool is_alpha = false;
	if (nodemtl->IsMultiMtl()) {
		for (int mi=0; mi<nodemtl->NumSubMtls(); mi++) {
			is_alpha |= is_alpha_material(nodemtl->GetSubMtl(mi));
		}
	} else {
		if (nodemtl->ClassID() == GameMaterialClassID) {
			GameMtl * gamemtl = (GameMtl *)nodemtl;

			if (	(gamemtl->Get_Dest_Blend(0) == W3DSHADER_DESTBLENDFUNC_SRC_ALPHA) ||
					(gamemtl->Get_Dest_Blend(0) == W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA) ||
					(gamemtl->Get_Src_Blend(0) == W3DSHADER_SRCBLENDFUNC_SRC_ALPHA) ||
					(gamemtl->Get_Src_Blend(0) == W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA) ||
					(gamemtl->Get_Alpha_Test(0) == W3DSHADER_ALPHATEST_ENABLE)  )
			{
				is_alpha = true;
			}
		}
	}
	return is_alpha;
}

bool W3DUtilityClass::is_alpha_mesh(INode * node,Mtl * nodemtl)
{
	Object *       obj = node->EvalWorldState(0).obj;
	TriObject *    tri = (TriObject *)obj->ConvertToType(0, triObjectClassID);

	if (tri != NULL) {
		Mesh & mesh = tri->mesh;

		int face_index;
		int mat_index;

		if (nodemtl == NULL) {

			return false;

		} else if (nodemtl->NumSubMtls() <= 1) {

			return is_alpha_material(nodemtl);

		} else {
			
			int sub_mtl_count = nodemtl->NumSubMtls();
			bool * sub_mtl_flags = new bool[sub_mtl_count];
			
			// Initialize each sub-material flag to false (indicates that the material is un-used)
			for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
				sub_mtl_flags[mat_index] = false;
			}

			// Set a true for each material actually referenced by the mesh
			for (face_index=0; face_index<mesh.getNumFaces(); face_index++) {
				int max_mat_index = mesh.faces[face_index].getMatID();
				int mat_index = (max_mat_index % sub_mtl_count);
				sub_mtl_flags[mat_index] = true;
			}

			// Loop over the used materials and return true if any are alpha materials
			for (mat_index=0; mat_index<sub_mtl_count; mat_index++) {
				if (sub_mtl_flags[mat_index]) {
					if (is_alpha_material(nodemtl->GetSubMtl(mat_index))) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

void W3DUtilityClass::generate_node_name(INode * node)
{
	TCHAR temp_string[256];

	if (NameOptions.OnlyAffectSelected && !node->Selected()) {
		return;
	}

	if (!Is_Bone(node) && !Is_Geometry(node)) {
		return;
	}

	if (NameOptions.AssignNames) {
		_stprintf(temp_string,"%s%03d",NameOptions.RootName,WorkingNameIndex);
		node->SetName(temp_string);
		WorkingNameIndex++;
	}

	if (NameOptions.AssignPrefix) {
		_stprintf(temp_string,"%s%s",NameOptions.PrefixName,node->GetName());
		node->SetName(temp_string);
	}

	if (NameOptions.AssignSuffix) {
		_stprintf(temp_string,"%s%s",node->GetName(),NameOptions.SuffixName);
		node->SetName(temp_string);
	}

	if (NameOptions.AssignCollisionBits) {
		
		W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
		assert(wdata);
		
		wdata->Enable_Physical_Collision(NameOptions.PhysicalCollision);
		wdata->Enable_Projectile_Collision(NameOptions.ProjectileCollision);
		wdata->Enable_Vis_Collision(NameOptions.VisCollision);
		wdata->Enable_Camera_Collision(NameOptions.CameraCollision);
		wdata->Enable_Vehicle_Collision(NameOptions.VehicleCollision);
	}
}

void W3DUtilityClass::generate_material_names_for_node(INode * node)
{
	if (MtlNameOptions.OnlyAffectSelected && !node->Selected()) {
		return;
	}
	generate_material_names(node->GetMtl());
}

void W3DUtilityClass::generate_material_names(Mtl * mtl)
{
	if (mtl == NULL) {
		return;
	}

	// set the name of this material and increment the index
	TCHAR newname[GenMtlNamesDialogClass::MAX_MATERIAL_NAME_LEN];
	_stprintf(newname,"%s%03d",MtlNameOptions.RootName,WorkingNameIndex);
	mtl->SetName(newname);
	WorkingNameIndex++;

	// recurse into children
	if (mtl->IsMultiMtl()) {
		for (int mi=0; mi<mtl->NumSubMtls(); mi++) {
			generate_material_names(mtl->GetSubMtl(mi));
		}
	}
}


W3DAppData0Struct * W3DUtilityClass::get_app_data_0(INode * node)
{
	/*
	** Try to get our AppData which has the export flags
	*/
	W3DAppData0Struct * wdata = NULL;
	AppDataChunk * appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,0);

	/*
	** If there wasn't one, return NULL since this app data chunk is obsolete now.
	** If there was one, get the data from it
	*/
	if (appdata) {
		wdata = (W3DAppData0Struct *)(appdata->data);
	} 
	
	return wdata;
}


W3DAppData1Struct * W3DUtilityClass::get_app_data_1(INode * node)
{
	// Try to get our AppData which has the damage region
	W3DAppData1Struct * wdata = NULL;
	AppDataChunk * appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,1);

	// If there wasn't one, add one.  If there was one, get the data from it
	if (appdata) {
		wdata = (W3DAppData1Struct *)(appdata->data);
	} else {
		wdata = new W3DAppData1Struct;
		node->AddAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,1,sizeof(W3DAppData1Struct),wdata);

		appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,1);
		assert(appdata);
	}

	return wdata;
}


W3DAppData2Struct * W3DUtilityClass::get_app_data_2(INode * node)
{
	return W3DAppData2Struct::Get_App_Data(node);
}


W3DDazzleAppDataStruct * W3DUtilityClass::get_dazzle_app_data(INode * node)
{
	return W3DDazzleAppDataStruct::Get_App_Data(node);
}


/**********************************************************************************************
**
** Dialog procs for the W3DUtilityClass
**
**********************************************************************************************/
static BOOL CALLBACK _w3d_utility_tools_dlg_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			TheW3DUtility.InterfacePtr->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		case WM_COMMAND:
		{				
			switch (LOWORD(wParam))
			{
				/*
				** Buttons
				*/
				case IDC_SELECT_GEOMETRY:
					TheW3DUtility.Select_Geometry();
					break;

				case IDC_SELECT_HIERARCHY:
					TheW3DUtility.Select_Hierarchy();
					break;

				case IDC_SELECT_ALPHA_MESHES:
					TheW3DUtility.Select_Alpha();
					break;

				case IDC_SELECT_PHYSICAL:
					TheW3DUtility.Select_Physical();
					break;

				case IDC_SELECT_PROJECTILE:
					TheW3DUtility.Select_Projectile();
					break;

				case IDC_SELECT_VIS:
					TheW3DUtility.Select_Vis();
					break;

				case IDC_COLLECTION_NAMES_GENERATE:
					TheW3DUtility.generate_names();
					TheW3DUtility.update_settings_controls();
					break;

				case IDC_MATERIAL_NAMES_GENERATE:
					TheW3DUtility.generate_material_names();
					TheW3DUtility.update_settings_controls();
					break;

				case IDC_LOD_EXTENSION_GENERATE:
					TheW3DUtility.generate_lod_extensions();
					TheW3DUtility.update_settings_controls();
					break;

				case IDC_EXPORT_STANDARD_MATERIALS:
					TheW3DUtility.export_with_standard_materials();
					break;

				case IDC_CREATE_SETTINGS_FLOATER:
					TheW3DUtility.create_floater();
					break;
			}
			return TRUE;
		}

		default:
			return FALSE;
	}
	return TRUE;
}



/**********************************************************************************************
**
** SettingsFormClass Implementation
** NOTE: When you use the _settings_form_dlg_proc, a SettingsFormClass will automatically
** be allocated and attached to the dialog.  You can cause all of the active forms to 
** refresh their status by calling Update_All_Instances.  The forms will be destroyed when
** the window is destroyed.
**
**********************************************************************************************/
SettingsFormClass *	SettingsFormClass::ActiveList = NULL;

BOOL CALLBACK _settings_form_dlg_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_INITDIALOG) {
		SettingsFormClass * form = new SettingsFormClass(hWnd);
		::SetProp(hWnd,"SettingsFormClass",(HANDLE)form);
	}

	SettingsFormClass * form = (SettingsFormClass *)::GetProp(hWnd,"SettingsFormClass");
	if (form) {
		return form->Dialog_Proc(hWnd,msg,wParam,lParam);
	} else {
		return FALSE;
	}
}

static void _settings_form_selection_changed_callback(void * param,NotifyInfo * info)
{
	((SettingsFormClass*)param)->Selection_Changed();
}

SettingsFormClass::SettingsFormClass(HWND hwnd) :
	Hwnd(hwnd),
	RegionSpin(NULL)
{
	/*
	** Link into the active list
	*/
	Next = ActiveList;
	ActiveList = this;

	/*
	** Register with MAX
	*/
	::RegisterNotification(_settings_form_selection_changed_callback, this, NOTIFY_SELECTIONSET_CHANGED);
}

SettingsFormClass::~SettingsFormClass(void)
{
	/*
	** Unregister from MAX
	*/
	::UnRegisterNotification(_settings_form_selection_changed_callback, this, NOTIFY_SELECTIONSET_CHANGED);

	/*
	** Unlink from the active list
	*/
	if (ActiveList == this) {
		ActiveList = Next;
	} else {
		
		SettingsFormClass * prev = ActiveList;
		SettingsFormClass * cur = ActiveList->Next;
	
		while ((cur != this) && (cur != NULL)) {
			cur = cur->Next;
			prev = prev->Next;
		}

		assert(cur == this);
		if (cur == this) {
			prev->Next = cur->Next;
		}
	}

	Hwnd = NULL;
}


void SettingsFormClass::Update_All_Instances(void)
{
	if (ActiveList == NULL) {
		return;
	}

	/*
	** Build a list of the selected nodes
	*/
	INodeListClass node_list(	::GetCOREInterface()->GetRootNode(),
										::GetCOREInterface()->GetTime(),
										&_INodeFilter	);

	/*
	** Update all settings forms
	*/
	SettingsFormClass * form = ActiveList;
	while (form != NULL) {
		form->Update_Controls(&node_list);
		form = form->Next;
	}
}


void SettingsFormClass::Init(void)
{
	// Initialize the contents of the dazzle combo
	// Reset the dazzle combo
	HWND dazzle_combo = GetDlgItem(Hwnd,IDC_DAZZLE_COMBO);
	assert(dazzle_combo != NULL);
	SendMessage(dazzle_combo,CB_RESETCONTENT,0,0);

	// Load the section of Dazzle.INI that defines all of the types.  The windows function
	// that I'm using here, reads in a NULL-terminated string for each entry in the section.  Each
	// string is of the form 'key=value'.  Based on my testing, it appears that windows removes any white
	// space before or after the equal sign as well.
	char dllpath[_MAX_PATH];
	::GetModuleFileName(AppInstance,dllpath,sizeof(dllpath));
	char * last_slash = strrchr(dllpath,'\\');
	last_slash++;
	strcpy(last_slash,DAZZLE_SETTINGS_FILENAME);

	char * dazzle_types_buffer = new char[DAZZLE_SECTION_BUFFERSIZE];	// max size of a section for Win95
	
	::GetPrivateProfileSection( DAZZLE_TYPES_SECTION, dazzle_types_buffer, DAZZLE_SECTION_BUFFERSIZE, dllpath);

	// Now we need to handle each string in the section buffer; skipping the 'key=' and adding
	// the dazzle type name into the combo box.
	char * entry = dazzle_types_buffer;
	if (entry != NULL) {
		while (*entry != NULL) {
			entry = strchr(entry,'=');
			if (entry != NULL) {
				entry++;
				::SendMessage(dazzle_combo,CB_ADDSTRING,0,(LPARAM)entry);
				entry += strlen(entry) + 1;
			}
		}
	} else {
		::SendMessage(dazzle_combo,CB_ADDSTRING,0,(LPARAM)"Default");
	}

	::SendMessage(dazzle_combo,CB_SETCURSEL,(WPARAM)0,0);

	delete dazzle_types_buffer;

	/*
	** Setup the damage region spinner control.
	*/
	RegionSpin = SetupIntSpinner
	(
		Hwnd,
		IDC_DAMREG_INDEX_SPIN,
		IDC_DAMREG_INDEX_EDIT,
		NO_DAMAGE_REGION, MAX_DAMAGE_REGIONS-1, NO_DAMAGE_REGION
	);

}

void SettingsFormClass::Destroy(void)
{
	ReleaseISpinner(RegionSpin);
	RegionSpin = NULL;
}

bool SettingsFormClass::Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	int check;

	switch (message) {
		case WM_INITDIALOG:
			Init();
			break;

		case WM_DESTROY:
			Destroy();
			delete this;
			break;

		case WM_COMMAND:
		{				
			/*
			** handle the tri-state checkboxes.
			** MAKE SURE YOU PUT ANY NEW CHECKBOX ID's IN HERE!!!
			*/
			int control_id = LOWORD(wParam);
			if (	(control_id == IDC_HIERARCHY_CHECK) || 
					(control_id == IDC_GEOMETRY_CHECK) ||
					(control_id == IDC_GEOMETRY_HIDE) ||
					(control_id == IDC_GEOMETRY_TWO_SIDED) ||
					(control_id == IDC_GEOMETRY_ZNORMALS) ||
					(control_id == IDC_GEOMETRY_VERTEX_ALPHA) || 
					(control_id == IDC_GEOMETRY_CAST_SHADOW) ||
					(control_id == IDC_GEOMETRY_SHATTERABLE) ||
					(control_id == IDC_GEOMETRY_NPATCH) ||
					(control_id == IDC_COLLISION_PHYSICAL) ||
					(control_id == IDC_COLLISION_PROJECTILE) ||
					(control_id == IDC_COLLISION_VIS) ||
					(control_id == IDC_COLLISION_CAMERA) ||
					(control_id == IDC_COLLISION_VEHICLE))
 			{
				check = !SendDlgItemMessage(hWnd,LOWORD(wParam),BM_GETCHECK,0,0L);
				SendDlgItemMessage(hWnd,LOWORD(wParam),BM_SETCHECK,check,0L);
			}

			INodeListClass node_list(	::GetCOREInterface()->GetRootNode(),
												::GetCOREInterface()->GetTime(),
												&_INodeFilter	);


			switch (LOWORD(wParam))
			{
				/*
				** Tri-State Checkboxes, make sure that the ID of all checkboxes is present
				** in the 'if' statement above!
				*/
				case IDC_HIERARCHY_CHECK: 
					W3DUtilityClass::set_hierarchy_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_GEOMETRY_CHECK: 
					W3DUtilityClass::set_geometry_in_all_selected(&node_list,check == BST_CHECKED);
					break;
				
				case IDC_GEOMETRY_HIDE:
					W3DUtilityClass::enable_hidden_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_GEOMETRY_TWO_SIDED:
					W3DUtilityClass::enable_two_sided_in_all_selected(&node_list,check == BST_CHECKED);
					break;
				
				case IDC_GEOMETRY_ZNORMALS:
					W3DUtilityClass::enable_znormals_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_GEOMETRY_VERTEX_ALPHA:
					W3DUtilityClass::enable_vertex_alpha_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_GEOMETRY_CAST_SHADOW:
					W3DUtilityClass::enable_shadow_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_GEOMETRY_SHATTERABLE:
					W3DUtilityClass::enable_shatterable_in_all_selected(&node_list,check == BST_CHECKED);
					break;
				
				case IDC_GEOMETRY_NPATCH:
					W3DUtilityClass::enable_npatches_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_COLLISION_PHYSICAL: 
					W3DUtilityClass::enable_physical_collision_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_COLLISION_PROJECTILE:
					W3DUtilityClass::enable_projectile_collision_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_COLLISION_VIS:
					W3DUtilityClass::enable_vis_collision_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_COLLISION_CAMERA:
					W3DUtilityClass::enable_camera_collision_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				case IDC_COLLISION_VEHICLE:
					W3DUtilityClass::enable_vehicle_collision_in_all_selected(&node_list,check == BST_CHECKED);
					break;

				/*
				** Radio buttons for the Geometry type
				*/
				case IDC_GEOMETRY_NORMAL:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_NORMAL_MESH);
					break;
				
				case IDC_GEOMETRY_CAMERA_ALIGNED:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_CAMERA_ALIGNED);
					break;
				
				case IDC_GEOMETRY_CAMERA_ORIENTED:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_CAMERA_ORIENTED);
					break;
				
				case IDC_GEOMETRY_NULL:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_NULL);
					break;

				case IDC_GEOMETRY_AABOX:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_AABOX);
					break;

				case IDC_GEOMETRY_OBBOX:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_OBBOX);
					break;

				case IDC_GEOMETRY_AGGREGATE:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_AGGREGATE);
					break;

				case IDC_GEOMETRY_DAZZLE:
					W3DUtilityClass::set_geometry_type_in_all_selected(&node_list,W3DAppData2Struct::GEO_TYPE_DAZZLE);
					break;

				/*
				** Dazzle type setting.  Whenever the user changes the selected dazzle type, apply
				** the new setting to all selected nodes.
				*/
				case IDC_DAZZLE_COMBO:
					if (HIWORD(wParam) == CBN_SELCHANGE) {

						HWND dazzle_combo = GetDlgItem(hWnd,IDC_DAZZLE_COMBO);
						if (dazzle_combo != NULL) {
		
							char dazzle_type[128];
							int cursel = ::SendMessage(dazzle_combo,CB_GETCURSEL,0,0);
							int len = ::SendMessage(dazzle_combo,CB_GETLBTEXTLEN,cursel,0);
							if (len < 128) {
								::SendMessage(dazzle_combo,CB_GETLBTEXT,(WPARAM)cursel,(LPARAM)dazzle_type);
								W3DUtilityClass::set_dazzle_type_in_all_selected(&node_list,dazzle_type);
							}
						}
					}
					break;

			}
			return TRUE;
		}

		/*
		** Spinners
		*/
		case CC_SPINNER_CHANGE:
			{
				INodeListClass node_list(	::GetCOREInterface()->GetRootNode(),
													::GetCOREInterface()->GetTime(),
													&_INodeFilter	);

				W3DUtilityClass::set_region_in_all_selected(&node_list,RegionSpin->GetIVal());
				break;
			}

		/*
		** Max Custom Edit boxes
		*/
		case WM_CUSTEDIT_ENTER:
			{
				INodeListClass node_list(	::GetCOREInterface()->GetRootNode(),
													::GetCOREInterface()->GetTime(),
													&_INodeFilter	);

				ICustEdit * edit_ctrl = GetICustEdit(GetDlgItem(hWnd,wParam));
				if (wParam == IDC_OBJ_NAME) {
					if (edit_ctrl && node_list.Num_Nodes() == 1) {
						char buffer[64];
						edit_ctrl->GetText(buffer,sizeof(buffer));
						node_list[0]->SetName(buffer);
						Update_All_Instances();
					}
				}
				ReleaseICustEdit(edit_ctrl);
				break;
			}


		default:
			return FALSE;
	}
	return TRUE;
}

void SettingsFormClass::Selection_Changed(void)
{
	INodeListClass node_list(		::GetCOREInterface()->GetRootNode(),
											::GetCOREInterface()->GetTime(),
											&_INodeFilter	);

	Update_Controls(&node_list);
}

void SettingsFormClass::Update_Controls(INodeListClass * node_list)
{
	/*
	** Update name of currently selected object 
	** "Multiple" if more than one, "None" if no selected objs...
	*/
	ICustEdit * edit_ctrl = GetICustEdit(GetDlgItem(Hwnd,IDC_OBJ_NAME));
	if (edit_ctrl != NULL) {
		if (node_list->Num_Nodes() == 0) {
			edit_ctrl->Enable(FALSE);
			edit_ctrl->SetText(Get_String(IDS_NO_OBJECT));
		} else if (node_list->Num_Nodes() == 1) {
			edit_ctrl->Enable(TRUE);
			edit_ctrl->SetText((*node_list)[0]->GetName());
		} else {
			edit_ctrl->Enable(FALSE);
			edit_ctrl->SetText(Get_String(IDS_MULTIPLE_OBJECTS));
		}

		ReleaseICustEdit(edit_ctrl);
	}

	if (node_list->Num_Nodes() == 0) {
		Disable_Controls();
		return;
	}

	W3DUtilityClass::NodeStatesStruct ns;
	W3DUtilityClass::eval_node_states(node_list,&ns);

	/*
	** Enable hierarchy and geometry checks since they are always available
	*/
	EnableWindow(GetDlgItem(Hwnd,IDC_HIERARCHY_CHECK),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CHECK),TRUE);

	/*
	** Enable/Disable the geometry controls
	*/
	if (ns.ExportGeometry == 1) {

		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAMERA_ALIGNED),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAMERA_ORIENTED),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NORMAL),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NULL),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_AABOX),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_OBBOX),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_AGGREGATE),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_DAZZLE),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_HIDE),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_TWO_SIDED),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_ZNORMALS),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_VERTEX_ALPHA),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAST_SHADOW),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_SHATTERABLE),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NPATCH),TRUE);

		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PHYSICAL),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PROJECTILE),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VIS),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_CAMERA),TRUE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VEHICLE),TRUE);

	} else {

		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAMERA_ALIGNED),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAMERA_ORIENTED),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NORMAL),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NULL),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_AABOX),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_OBBOX),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_AGGREGATE),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_DAZZLE),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_HIDE),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_TWO_SIDED),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_ZNORMALS),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_VERTEX_ALPHA),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAST_SHADOW),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_SHATTERABLE),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NPATCH),FALSE);

		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PHYSICAL),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PROJECTILE),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VIS),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_CAMERA),FALSE);
		EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VEHICLE),FALSE);
	}

	/*
	** Set the checks based on the nodes states:
	** no check - none of the selected nodes had this setting
	** check - all of the selected nodes had this setting
	** grey check - some of the selected nodes had this setting
	*/
	SendDlgItemMessage(Hwnd,IDC_HIERARCHY_CHECK,BM_SETCHECK,ns.ExportHierarchy,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_CHECK,BM_SETCHECK,ns.ExportGeometry,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_HIDE,BM_SETCHECK,ns.GeometryHidden,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_TWO_SIDED,BM_SETCHECK,ns.GeometryTwoSided,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_ZNORMALS,BM_SETCHECK,ns.GeometryZNormals,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_VERTEX_ALPHA,BM_SETCHECK,ns.GeometryVertexAlpha,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_CAST_SHADOW,BM_SETCHECK,ns.GeometryCastShadow,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_SHATTERABLE,BM_SETCHECK,ns.GeometryShatterable,0L);
	SendDlgItemMessage(Hwnd,IDC_GEOMETRY_NPATCH,BM_SETCHECK,ns.GeometryNPatch,0L);
	SendDlgItemMessage(Hwnd,IDC_COLLISION_PHYSICAL,BM_SETCHECK,ns.CollisionPhysical,0L);
	SendDlgItemMessage(Hwnd,IDC_COLLISION_PROJECTILE,BM_SETCHECK,ns.CollisionProjectile,0L);
	SendDlgItemMessage(Hwnd,IDC_COLLISION_VIS,BM_SETCHECK,ns.CollisionVis,0L);
	SendDlgItemMessage(Hwnd,IDC_COLLISION_CAMERA,BM_SETCHECK,ns.CollisionCamera,0L);
	SendDlgItemMessage(Hwnd,IDC_COLLISION_VEHICLE,BM_SETCHECK,ns.CollisionVehicle,0L);

	/*
	** The damage region spinner should only be enabled if
	** Export Hierarchy is checked for all selected nodes.
	*/
	BOOL spinner_enable = false;
	if (ns.ExportHierarchy == 1)
	{
		if (ns.DamageRegion != MAX_DAMAGE_REGIONS)
		{
			// Show the damage region in the spinner.
			RegionSpin->SetIndeterminate(FALSE);
			RegionSpin->SetValue(ns.DamageRegion, FALSE);
		}
		else
		{
			// The selected objects aren't all in the same region.
			RegionSpin->SetIndeterminate(TRUE);
		}

		spinner_enable = true;
	}
	EnableWindow(GetDlgItem(Hwnd,IDC_DAMREG_INDEX_EDIT),spinner_enable);
	EnableWindow(GetDlgItem(Hwnd,IDC_DAMREG_INDEX_SPIN),spinner_enable);

	/*
	** The dazzle combo box should only be enabled if 
	** Export Geometry, and geometry type dazzle is set for all 
	** selected nodes.
	*/
	bool dazzle_combo_enable = false;
	if (ns.ExportGeometry == 1) {
		if (ns.DazzleCount == node_list->Num_Nodes()) {
			dazzle_combo_enable = true;
		}
	}
	HWND dazzle_combo = GetDlgItem(Hwnd,IDC_DAZZLE_COMBO);
	EnableWindow(dazzle_combo,dazzle_combo_enable);
	int selindex = ::SendMessage(dazzle_combo,CB_FINDSTRING,(WPARAM)0,(LPARAM)ns.DazzleType);
	if (selindex != CB_ERR) {
		::SendMessage(dazzle_combo,CB_SETCURSEL,(WPARAM)selindex,(LPARAM)0);
	} else {
		::SendMessage(dazzle_combo,CB_SETCURSEL,(WPARAM)0,(LPARAM)0);
	}

	/*
	** Set any radio buttons present
	*/
	CheckDlgButton(Hwnd,IDC_GEOMETRY_CAMERA_ALIGNED,(ns.GeometryCameraAligned ? BST_CHECKED : BST_UNCHECKED));
	CheckDlgButton(Hwnd,IDC_GEOMETRY_CAMERA_ORIENTED,(ns.GeometryCameraOriented ? BST_CHECKED : BST_UNCHECKED));
	CheckDlgButton(Hwnd,IDC_GEOMETRY_NORMAL,(ns.GeometryNormal ? BST_CHECKED : BST_UNCHECKED));
	CheckDlgButton(Hwnd,IDC_GEOMETRY_NULL,(ns.GeometryNull ? BST_CHECKED : BST_UNCHECKED));
	CheckDlgButton(Hwnd,IDC_GEOMETRY_AABOX,(ns.GeometryAABox ? BST_CHECKED : BST_UNCHECKED));
	CheckDlgButton(Hwnd,IDC_GEOMETRY_OBBOX,(ns.GeometryOBBox ? BST_CHECKED : BST_UNCHECKED));
	CheckDlgButton(Hwnd,IDC_GEOMETRY_AGGREGATE,(ns.GeometryAggregate ? BST_CHECKED : BST_UNCHECKED));
	CheckDlgButton(Hwnd,IDC_GEOMETRY_DAZZLE,(ns.GeometryDazzle ? BST_CHECKED : BST_UNCHECKED));
}


void SettingsFormClass::Disable_Controls(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_OBJ_NAME),FALSE);

	EnableWindow(GetDlgItem(Hwnd,IDC_HIERARCHY_CHECK),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CHECK),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_DAMREG_INDEX_EDIT),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_DAMREG_INDEX_SPIN),FALSE);
	
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NORMAL),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAMERA_ALIGNED),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAMERA_ORIENTED),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_AABOX),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_OBBOX),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NULL),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_AGGREGATE),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_DAZZLE),FALSE);

	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_HIDE),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_TWO_SIDED),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_ZNORMALS),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_VERTEX_ALPHA),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_CAST_SHADOW),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_SHATTERABLE),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_GEOMETRY_NPATCH),FALSE);

	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PHYSICAL),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PROJECTILE),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VIS),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_CAMERA),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VEHICLE),FALSE);

	EnableWindow(GetDlgItem(Hwnd,IDC_DAZZLE_COMBO),FALSE);

	CheckDlgButton(Hwnd,IDC_HIERARCHY_CHECK,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_CHECK,BST_UNCHECKED);
	
	CheckDlgButton(Hwnd,IDC_GEOMETRY_CAMERA_ALIGNED,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_CAMERA_ORIENTED,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_NORMAL,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_AABOX,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_OBBOX,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_NULL,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_AGGREGATE,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_DAZZLE,BST_UNCHECKED);

	CheckDlgButton(Hwnd,IDC_GEOMETRY_HIDE,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_TWO_SIDED,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_ZNORMALS,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_VERTEX_ALPHA,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_CAST_SHADOW,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_SHATTERABLE,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_GEOMETRY_NPATCH,BST_UNCHECKED);

	CheckDlgButton(Hwnd,IDC_COLLISION_PHYSICAL,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_COLLISION_PROJECTILE,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_COLLISION_VIS,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_COLLISION_CAMERA,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_COLLISION_VEHICLE,BST_UNCHECKED);
}


/*
** Functions to access the W3D AppData of any INode.
*/
W3DAppData0Struct * GetW3DAppData0 (INode *node)
{
	return TheW3DUtility.get_app_data_0(node);
}

W3DAppData1Struct * GetW3DAppData1 (INode *node)
{
	return TheW3DUtility.get_app_data_1(node);
}

W3DAppData2Struct * GetW3DAppData2 (INode *node)
{
	return TheW3DUtility.get_app_data_2(node);
}

W3DDazzleAppDataStruct * GetW3DDazzleAppData(INode *node)
{
	return TheW3DUtility.get_dazzle_app_data(node);
}
