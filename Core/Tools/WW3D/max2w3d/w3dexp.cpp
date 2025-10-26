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

/* $Header: /Commando/Code/Tools/max2w3d/w3dexp.cpp 78    1/03/01 11:06a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dexp.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/03/01 11:03a                                              $*
 *                                                                                             *
 *                    $Revision:: 78                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   W3dExportClass::W3dExportClass -- constructor                                             *
 *   W3dExportClass::~W3dExportClass -- destructor                                             *
 *   W3dExportClass::Export_Hierarchy -- Export the hierarchy tree                             *
 *   W3dExportClass::Export_Animation -- Export animation data                                 *
 *   W3dExportClass::Export_Damage_Animations -- Exports damage animations for the model       *
 *   W3dExportClass::Export_Geometry -- Export the geometry data                               *
 *   W3dExportClass::get_hierarchy_tree -- get a pointer to the hierarchy tree                 *
 *   W3dExportClass::get_export_options -- get the export options                              *
 *   W3dExportClass::Start_Progress_Bar -- start the MAX progress meter                        *
 *   W3dExportClass::End_Progress_Bar -- end the progress meter                                *
 *   W3dExportClass::get_damage_root_list -- gets the list of damage root nodes                *
 *   W3dExportClass::Export_HLod -- Export an HLOD description                                 *
 *   W3dExportClass::Export_Collection -- exports a collection chunk                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "rawfile.h"
#include "chunkio.h"
#include "w3dexp.h"
#include "w3dutil.h"
#include "nodelist.h"
#include "meshsave.h"
#include "hiersave.h"
#include "hlodsave.h"
#include "meshcon.h"
#include "SnapPoints.h"
#include "w3ddlg.h"
#include "PROGRESS.H"
#include "errclass.h"
#include "motion.h"
#include "util.h"
#include "w3ddesc.h"
#include "colboxsave.h"
#include "nullsave.h"
#include "dazzlesave.h"
#include "maxworldinfo.h"
#include "exportlog.h"

#include "geometryexporttask.h"
#include "geometryexportcontext.h"

#include <direct.h>
#include "TARGA.H"

// Used to communicate from the exporter to the dialog.
char W3dExportClass::CurrentExportPath[_MAX_DRIVE + _MAX_DIR + 1] = { '\000' };


/* local functions */
static DWORD WINAPI progress_callback( LPVOID arg);
static HierarchySaveClass * load_hierarchy_file(char * filename);
static bool dupe_check(const INodeListClass & list);
static bool check_lod_extensions (INodeListClass &list, INode *origin);

/*
** Struct for export info (AppDataChunk hung off the scene pointer)
** This includes the export info struct and some padding.
** NOTE: to avoid file versioning issues, new data should be added after
** existing data in this struct, the padding array should be made smaller so
** the total size remains the same, and the new data should give reasonable
** results with a default content of zeros (which is what it will contain if
** an older file is loaded).
*/
struct ExportInfoAppDataChunkStruct {
	W3dExportOptionsStruct	ExportOptions;
	unsigned char				Padding[89];
};



/************************************************************************************************
**
**	GeometryFilterClass - filters out nodes which are not marked for W3D geometry export
**
************************************************************************************************/
class GeometryFilterClass : public INodeFilterClass
{
public:
	virtual BOOL Accept_Node(INode * node, TimeValue time)
	{
		Object * obj = node->EvalWorldState(time).obj;

		if
		(
			obj
//			&& !Is_Proxy (*node)
			&& !Is_Origin(node)
			&& !node->IsHidden()
			&& Is_Geometry(node)
		)
		{
			return TRUE;
		} else {
			return FALSE;
		}
	}
};




/************************************************************************************************
**
** OriginFilterClass - Filters out nodes which are not "origin" objects. Origins are MAX dummy
** objects whose parents are the scene root, and are named "origin.*" (ie. first 7 characters
** in the name are "origin.". These origin objects will be (0,0,0) for all of its descendants.
** This allows an artist to create multiple models within one MAX scene but still have each
** mesh's coordinates equal without needing to stack all the models on the world origin.
** We iterate through the origin objects when exporting a scene containing multiple LODs of
** one model.
**
************************************************************************************************/
class OriginFilterClass : public INodeFilterClass
{
public:

	virtual BOOL Accept_Node(INode * node, TimeValue time)	{ return Is_Origin(node); }
};


/************************************************************************************************
**
** DamageRootFilterClass - Filters out all nodes which are not "damage root" objects. These nodes
** are MAX dummy objects whose parents are the scene root, and are named "damage.*" (ie. first 7
** characters in the name are "damage.". These damage roots mean that all of its children
** represent a damaged model.
**
************************************************************************************************/
class DamageRootFilterClass : public INodeFilterClass
{
public:

	virtual BOOL Accept_Node(INode * node, TimeValue time)	{ return Is_Damage_Root(node); }
};


/************************************************************************************************
**
** DamageRegionFilterClass - Filters out all node that are not a bone which is part of a certain
** deformation region. Pass the region ID to the constructor.
**
************************************************************************************************/
class DamageRegionFilterClass : public INodeFilterClass
{
public:
	DamageRegionFilterClass(int region_id)						{ RegionId = region_id; }

	virtual BOOL Accept_Node(INode * node, TimeValue time)
	{
		if (!Is_Bone(node)) return FALSE;

		// Check it's damage region ID (if it has one).
		AppDataChunk * appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,1);
		if (!appdata) return FALSE;
		W3DAppData1Struct *wdata = (W3DAppData1Struct*)(appdata->data);
		return wdata->DamageRegion == RegionId;
	}

protected:

	int RegionId;
};


/***********************************************************************************************
 * W3dExportClass::DoExport -- This method is called for the plug-in to perform it's file expo *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  	name - filename to use																						  *
 * 	export - A pointer the plug-in may use to call methods to enumerate the scene 			  *
 * 	max - An interface pointer the plug-in may use to call methods of MAX.						  *
 * 																														  *
 * OUTPUT:                                                                                     *
 *   Nonzero on successful export; otherwise 0.																  *
 * 																														  *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/09/1997 GH  : Created.                                                                 *
 *   10/17/2000 gth : Removed the old export code-path, everything goes through an origin now  *
 *=============================================================================================*/
int W3dExportClass::DoExport
(
	const TCHAR *filename,
	ExpInterface *do_export,
	Interface *max,
	BOOL suppressPrompts,
	DWORD options
)
{
	ExportInterface = do_export;
	MaxInterface = max;
	RootNode = NULL;
	OriginList = NULL;
	DamageRootList = NULL;
	HierarchyTree = NULL;

	try {

		CurTime = MaxInterface->GetTime();
		FrameRate = GetFrameRate();
		FixupType = HierarchySaveClass::MATRIX_FIXUP_TRANS_ROT;

		/*
		** The Animation and the Hierarchy will be named with the root portion of the W3D filename
		** and the path is used by the options dialog
		*/
		char rootname[_MAX_FNAME + 1];
		char drivename[_MAX_DRIVE + 1];
		char dirname[_MAX_DIR + 1];
		_splitpath(filename, drivename, dirname, rootname, NULL);
		sprintf(CurrentExportPath, "%s%s", drivename, dirname);

		/*
		** The batch export process (suppressPrompt == TRUE) needs to know the directory of the
		** MAX file being exported. This is so that it can use the old relative pathname of the
		** W3D file containing the hierarchy.
		*/
		_splitpath(max->GetCurFilePath(), drivename, dirname, NULL, NULL);
		sprintf(CurrentScenePath, "%s%s", drivename, dirname);

		/*
		** Get export options
		*/
		if (!get_export_options(suppressPrompts)) {
			return 1;
		}

		/*
		** If no data is going to be exported just bail
		*/
		if ((!ExportOptions.ExportHierarchy) && (!ExportOptions.ExportAnimation) && (!ExportOptions.ExportGeometry)) {
			return 1;
		}

		/*
		** Initialize the logging system
		*/
		ExportLog::Init(NULL);

		/*
		** Create a chunk saver to write the w3d file with
		*/
		RawFileClass stream(filename);

		if (!stream.Open(FileClass::WRITE)) {
			MessageBox(NULL,"Unable to open file.","Error",MB_OK | MB_SETFOREGROUND);
			return 1;
		}

		ChunkSaveClass csave(&stream);

		/*
		** Export data from the scene.
		**
		** Are we doing an old export (one model/LOD per scene) or a new export (multiple LODs
		** for one model in a scene)?
		*/
		if (get_origin_list())
		{
			DoOriginBasedExport(rootname, csave);
		}

		/*
		** Done!
		*/
		stream.Close();

		if (HierarchyTree != NULL) {
			delete HierarchyTree;
			HierarchyTree = NULL;
		}

		if (OriginList != NULL) {
			delete OriginList;
			OriginList = NULL;
		}

		if (DamageRootList != NULL) {
			delete DamageRootList;
			DamageRootList = NULL;
		}

	} catch (ErrorClass error) {

		MessageBox(NULL,error.error_message,"Error",MB_OK | MB_SETFOREGROUND);
	}

	ExportLog::Shutdown(ExportOptions.ReviewLog);
	MaxInterface->RedrawViews(MaxInterface->GetTime());
	return 1;
}



/***********************************************************************************************
 * W3dExportClass::DoOriginBasedExport -- New export codepath. Exports any objects linked to   *
 * an origin object. Assumes origins named "origin.01" and greater represent LODs of the       *
 * original object ("origin.00"). Also assumes "damage.01" and greater represent damaged       *
 * versions of the original object.                                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/13/1999  AJA : Created.                                                                 *
 *   9/21/1999  AJA : Added support for the new animation exporting process (damage-related).  *
 *=============================================================================================*/
void W3dExportClass::DoOriginBasedExport(char *rootname,ChunkSaveClass &csave)
{
	/*
	** Build the damage root list.
	*/
	INodeListClass	*damage_list = get_damage_root_list();
	assert(damage_list != NULL);

	/*
	** Start the progress meter
	*/
	Start_Progress_Bar();
	Progress_Meter_Class meter(MaxInterface,0.0f,100.0f);

	int steps = 0;
	steps++;										// Base Pose
	steps+= OriginList->Num_Nodes();		// n Origins
	steps++;										// Basic Anim OR Damage Anims
	steps++;										// HLOD OR Collection

	meter.Finish_In_Steps(steps);

	/*
	** Find the base object's origin.
	*/
	bool				is_base_object = false;
	INodeListClass	*origin_list = get_origin_list();
	unsigned int	i, count = origin_list->Num_Nodes();
	INode				*base_origin = NULL;

	for (i = 0; i < count; i++)
	{
		INode *node = (*origin_list)[i];
		if (Is_Base_Origin(node))
		{
			base_origin = node;
			break;
		}
	}

	/*
	** Write the Hierarchy Tree (if needed)
	*/
	Progress_Meter_Class treemeter(meter, meter.Increment);
	if (!Export_Hierarchy(rootname, csave, treemeter, base_origin))
	{
		MessageBox(NULL,"Hierarchy Export Failure!","Error",MB_OK | MB_SETFOREGROUND);
		End_Progress_Bar();
		return;
	}
	meter.Add_Increment();

	if (damage_list->Num_Nodes() <= 0)
	{
		/*
		** Write the Base Animation (if needed)
		*/
		Progress_Meter_Class animmeter(meter, meter.Increment);
		if (!Export_Animation(rootname, csave, animmeter, base_origin))
		{
			MessageBox(NULL,"Animation Export Failure!","Error",MB_OK | MB_SETFOREGROUND);
			End_Progress_Bar();
			return;
		}
		meter.Add_Increment();
	}
	else
	{
		/*
		** Write the damage animations.
		*/
		Progress_Meter_Class damagemeter(meter, meter.Increment);
		for (i = 0; i < damage_list->Num_Nodes(); i++)
		{
			if (!Export_Damage_Animations(rootname, csave, damagemeter, (*damage_list)[i]))
			{
				MessageBox(NULL, "Damage Animation Export Failure!", "Error", MB_OK | MB_SETFOREGROUND);
				End_Progress_Bar();
				return;
			}
		}
		meter.Add_Increment();
	}

	/*
	** Create an array of pointers to MeshConnectionsClass objects. These objects
	** will be created below, and will be used to generate the HLOD with the
	** geometry of all models in the scene.
	*/
	MeshConnectionsClass **connections = new MeshConnectionsClass*[count];
	if (!connections)
	{
		MessageBox(NULL, "Memory allocation failure!", "Error", MB_OK | MB_SETFOREGROUND);
		End_Progress_Bar();
		return;
	}
	memset(connections, 0, sizeof(MeshConnectionsClass*) * count);

	/*
	** For each model in the scene, write its animation and geometry (if needed).
	** All models share the above hierarchy tree.
	*/
	int idx = strlen(rootname);
	rootname[idx+1] = '\0';

	/*
	** If we're not exporting a hierarchical model, only export the "origin.00"
	*/
	if (!ExportOptions.LoadHierarchy && !ExportOptions.ExportHierarchy) {
		count = 1;
	}

	for (i = 0; i < count; i++)
	{
		/*
		** Get the current origin.
		*/
		INode *origin = (*origin_list)[i];

		/*
		** Write each mesh (if needed)
		*/
		MeshConnectionsClass *meshcon = NULL;
		Progress_Meter_Class meshmeter(meter, meter.Increment);
		if (!Export_Geometry(rootname, csave, meshmeter, origin, &meshcon))
		{
			MessageBox(NULL, "Geometry Export Failure!", "Error", MB_OK | MB_SETFOREGROUND);
			End_Progress_Bar();
			return;
		}
		meter.Add_Increment();

		/*
		** Put the MeshConnectionsClass object for this model into
		** the array in order of LOD (top-level last).
		*/
		int lod_level = Get_Lod_Level(origin);
		if (lod_level >= count || connections[count - lod_level - 1] != NULL)
		{
			char text[256];
			sprintf(text, "Origin Naming Error! There are %d models defined in this "
				"scene, therefore your origin names should be\n\"Origin.00\" through "
				"\"Origin.%02d\", 00 being the high-poly model and %02d being the "
				"lowest detail LOD.", count, count-1, count-1);
			MessageBox(NULL, text, "Error", MB_OK | MB_SETFOREGROUND);
			End_Progress_Bar();
			return;
		}
		connections[count - lod_level - 1] = meshcon;
	}

	/*
	** Generate the HLOD based on all the mesh connections.
	*/
	if (ExportOptions.LoadHierarchy || ExportOptions.ExportHierarchy) {
		rootname[idx] = '\0';	// remove the trailing character (signifies which lod level)
		HierarchySaveClass *htree = get_hierarchy_tree();
		if (htree)
		{
			Progress_Meter_Class hlod_meter(meter, meter.Increment);
			if (!Export_HLod(rootname, htree->Get_Name(), csave, hlod_meter, connections, count))
			{
				MessageBox(NULL, "HLOD Generation Failure!", "Error", MB_OK | MB_SETFOREGROUND);
				End_Progress_Bar();
				return;
			}
			meter.Add_Increment();
		}
	}

	/*
	** Deallocate the array of mesh connections.
	*/
	for (i = 0; i < count; i++)
	{
		if (connections[i] != NULL)
			delete connections[i];
	}
	delete []connections;

	End_Progress_Bar();
}


/***********************************************************************************************
 * W3dExportClass::Export_Hierarchy -- Export the hierarchy tree                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1997 GH  : Created.                                                                 *
 *   13/9/1999  AJA : Split into two calls, one that takes a node list and one that takes a    *
 *                    single root node.                                                        *
 *   10/17/2000 gth : Removed the old code-path, we always use an origin now                   *
 *=============================================================================================*/
bool W3dExportClass::Export_Hierarchy(char *name,ChunkSaveClass & csave,Progress_Meter_Class & meter,
												  INode *root)
{
	if (!ExportOptions.ExportHierarchy) return true;
	HierarchySaveClass::Enable_Terrain_Optimization(ExportOptions.EnableTerrainMode);

	if (root == NULL) return false;

	try {
		HierarchyTree = new HierarchySaveClass(root,CurTime,meter,name,FixupType);
	} catch (ErrorClass err) {
		MessageBox(NULL, err.error_message,"Error!",MB_OK | MB_SETFOREGROUND);
		return false;
	}

	HierarchyTree->Save(csave);

	return true;
}

/***********************************************************************************************
 * W3dExportClass::Export_Animation -- Export animation data                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1997 GH  : Created.                                                                 *
 *   13/9/1999  AJA : Split into two calls, one that takes a node list and one that takes a    *
 *                    single root node.                                                        *
 *   10/17/2000 gth : Removed the old code-path, we always use an origin now                   *
 *=============================================================================================*/
bool W3dExportClass::Export_Animation(char * name,ChunkSaveClass & csave,Progress_Meter_Class & meter,
												  INode *root)
{
	if (!ExportOptions.ExportAnimation) return true;
	HierarchySaveClass * htree = get_hierarchy_tree();

	if ((root == NULL) || (htree == NULL)) {
		return false;
	}

	MotionClass * motion = NULL;

	try {
		motion = new MotionClass(	ExportInterface->theScene,
											root,
											htree,
										   ExportOptions,
											FrameRate,
											&meter,
											MaxInterface->GetMAXHWnd(),
											name);
	} catch (ErrorClass err) {
		MessageBox(NULL,err.error_message,"Error!",MB_OK | MB_SETFOREGROUND);
		return false;
	}

	motion->Save(csave);

	delete motion;

	return true;
}


/***********************************************************************************************
 * W3dExportClass::Export_Damage_Animations -- Exports damage animations for the model         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1999 AJA : Created.                                                                       *
 *=============================================================================================*/
bool W3dExportClass::Export_Damage_Animations(char *name, ChunkSaveClass &csave,
															 Progress_Meter_Class &meter,
															 INode *damage_root)
{
	if (!ExportOptions.ExportAnimation) return true;
	HierarchySaveClass *htree = get_hierarchy_tree();

	if ((damage_root == NULL) || (htree == NULL))
		return false;

	int damage_state = Get_Damage_State(damage_root);

	/*
	** While exporting damage animations, we need the offset from our origin to the real
	** scene origin.
	*/
	Matrix3 originoffset = Inverse(damage_root->GetNodeTM(CurTime));

   /*
	** For every damage region we find, export an animation.
	*/
	bool			done = false;
	int			current_region = 0;
	int			num_damage_bones = 0;	// number of bones assigned to a damage region
	for (current_region = 0; current_region < MAX_DAMAGE_REGIONS; current_region++)
	{
		DamageRegionFilterClass	region_filter(current_region);
		INodeListClass				bone_list(damage_root, CurTime, &region_filter);

		num_damage_bones += bone_list.Num_Nodes();

		// Move to the next region if there aren't any bones in this one.
		if (bone_list.Num_Nodes() <= 0)
			continue;

		// Put together an animation name for this damage region.
		char anim_name[W3D_NAME_LEN];
		sprintf(anim_name, "damage%d-%d", current_region, damage_state);

		// Export an animation for this damage region.
		MotionClass *motion = NULL;
		try
		{
			motion = new MotionClass(	ExportInterface->theScene,
												&bone_list,
												htree,
												ExportOptions,
												FrameRate,
												&meter,
												MaxInterface->GetMAXHWnd(),
												anim_name,
												originoffset);
		}
		catch (ErrorClass err)
		{
			MessageBox(NULL, err.error_message, "Error!", MB_OK | MB_SETFOREGROUND);
			return false;
		}

		assert(motion != NULL);
		motion->Save(csave);

		delete motion;
	}

	if (num_damage_bones <= 0)
	{
		MessageBox(NULL, "Warning: Your damage bones need to be given damage region numbers. "
			"You can do this in the W3D Tools panel.", name, MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
	}

	return true;
}

#define TEAM_COLOR_PALETTE_SIZE 16
unsigned int houseColorScale[TEAM_COLOR_PALETTE_SIZE]=
{255,239,223,211,195,174,167,151,135,123,107,91,79,63,47,35};

/***********************************************************************************************
 * W3dExportClass::Export_Geometry -- Export the geometry data                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1997 GH  : Created.                                                                 *
 *   13/9/1999  AJA : Added an optional "root" parameter to export geometry of the node's      *
 *                    descendants.                                                             *
 *   10/17/2000 gth : Made the "root" parameter a requirement, just pass in the scene root     *
 *                    if you want to export all geometry in the scene.                         *
 *   10/30/2000 gth : If exporting only geometry, only export the first mesh                   *
 *=============================================================================================*/
bool W3dExportClass::Export_Geometry(char * name,ChunkSaveClass & csave,Progress_Meter_Class & meter,
												 INode *root,MeshConnectionsClass **out_connection)
{
	unsigned int i;

	assert(root != NULL);
	if (!ExportOptions.ExportGeometry) return true;

	/*
	** If we're attaching the meshes to a hierarchy, get the tree
	*/
	HierarchySaveClass * htree = NULL;
	if (ExportOptions.LoadHierarchy || ExportOptions.ExportHierarchy) {
		htree = get_hierarchy_tree();
		if (htree == NULL) {
			return false;
		}
	}

	DynamicVectorClass<GeometryExportTaskClass *> export_tasks;
	INodeListClass *geometry_list = NULL;

	/*
	** Create the lists of nodes that we're going to work with
	*/
	GeometryFilterClass geometryfilter;
	geometry_list = new INodeListClass(root,CurTime,&geometryfilter);
	if (dupe_check(*geometry_list)) {
		return false;
	}

	MaxWorldInfoClass world_info(export_tasks);
	world_info.Allow_Mesh_Smoothing (ExportOptions.SmoothBetweenMeshes);
	unsigned int materialColors[16*16];	///@todo: MW: Fix this to remove solid colors.
	char	materialColorFilename[_MAX_FNAME + 1];
	memset(materialColors,0,sizeof(materialColors));
	for (i=0; i<TEAM_COLOR_PALETTE_SIZE; i++)
	{	//preset the first 16 colors to predefined set of house colors
		materialColors[i]=houseColorScale[i] << 16;
	}

	/*
	** Initialize the context object for exporting geometry
	*/
	GeometryExportContextClass context(	name,
													csave,
													world_info,
													ExportOptions,
													htree,
													root,
													get_origin_list(),
													CurTime,
													materialColors);

	if (ExportOptions.EnableMaterialColorToTextureConversion)
		context.materialColorTexture=materialColorFilename;

	sprintf(materialColorFilename,"%sZMCD_%s.tga",CurrentExportPath,name);

	/*
	** Initialize a list of geometry export tasks containing all nodes marked for geometry export.
	** If we're only exporting geometry, only export the first mesh. (no more collections)
	*/
	int geometry_count = geometry_list->Num_Nodes();
	if ((htree == NULL) && (geometry_list->Num_Nodes() > 1)) {
		geometry_count = MIN(geometry_count,1);
		ExportLog::printf("\nDiscarding extra meshes since we are not exporting a hierarchical model.\n");
	}

	for (i=0; i<geometry_count; i++) {
		GeometryExportTaskClass * export_task = GeometryExportTaskClass::Create_Task((*geometry_list)[i],context);
		if (export_task != NULL) {
			export_tasks.Add(export_task);
		}
	}
	meter.Finish_In_Steps(export_tasks.Count());

	/*
	** Optimize the mesh data if the user desired, modifying the list of geometry export tasks.
	*/
	if (ExportOptions.EnableOptimizeMeshData) {
		GeometryExportTaskClass::Optimize_Geometry(export_tasks,context);
	}

	/*
	** If there is only one piece of geometry to export and no place-holders, and we're not
	** exporting a hierarchical model, then we force the name to match the filename
	*/
	if ((export_tasks.Count() == 1) && (htree == NULL))
	{
		export_tasks[0]->Set_Name(name);
		export_tasks[0]->Set_Container_Name("");
	}

	/*
	** Generate the mesh-connections object to return to the caller
	*/
	MeshConnectionsClass * meshcon = NULL;
	if (htree != NULL) {
		Progress_Meter_Class mcmeter(meter,meter.Increment);
		try {
			meshcon = new MeshConnectionsClass(export_tasks,context);
		} catch (ErrorClass err) {
			MessageBox(NULL,err.error_message,"Error!",MB_OK | MB_SETFOREGROUND);
			return false;
		}
		*out_connection = meshcon;
		meter.Add_Increment();
	}

	/*
	** Export each piece of geometry
	*/
	for (i=0; i<export_tasks.Count(); i++) {
		Progress_Meter_Class meshmeter(meter,meter.Increment);
		context.ProgressMeter = &meshmeter;

		try {
			export_tasks[i]->Export_Geometry(context);
		} catch (ErrorClass err) {
			MessageBox(MaxInterface->GetMAXHWnd(),err.error_message,"Error!",MB_OK | MB_SETFOREGROUND);
			continue;
		}

		meter.Add_Increment();
	}

	//Check if any textures need to be generated
	if (context.numMaterialColors || context.numHouseColors)
	{
		Targa	targ;
		char imageBuffer[16*16*3];
		int px,py,buf_index;
		unsigned int Diffuse;

		//clear to black
		memset(imageBuffer,0,sizeof(imageBuffer));

		for (i=0; i<(16+context.numMaterialColors); i++)
		{
			//get coordinates of this material within texture page
			px=(i%16);	///@todo: MW: Remove hard-coded texture size
			py=(i/16);

			Diffuse=context.materialColors[i];
			buf_index=(px+py*16)*3;

			imageBuffer[buf_index]=(Diffuse>>16)&0xff;
			imageBuffer[buf_index+1]=(Diffuse>>8)&0xff;
			imageBuffer[buf_index+2]=(Diffuse)&0xff;
		}

		memset(&targ.Header,0,sizeof(targ.Header));
		targ.Header.Width=16;
		targ.Header.Height=16;
		targ.Header.PixelDepth=24;
		targ.Header.ImageType=TGA_TRUECOLOR;
		targ.SetImage(imageBuffer);
		targ.YFlip();

		if (context.numHouseColors)
			sprintf(materialColorFilename,"%sZHCD_%s.tga",CurrentExportPath,name);

		targ.Save(materialColorFilename,TGAF_IMAGE,false);
	}

	/*
	** Cleanup
	*/
	for (i=0; i<export_tasks.Count(); i++) {
		delete export_tasks[i];
	}

	export_tasks.Delete_All();
	delete geometry_list;

	return true;
}


/***********************************************************************************************
 * W3dExportClass::Export_HLod -- Export an HLOD description                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool W3dExportClass::Export_HLod( char *name, const char *htree_name, ChunkSaveClass &csave,
											 Progress_Meter_Class &meter, MeshConnectionsClass **connections,
											 int lod_count)
{
	if (!ExportOptions.ExportGeometry) return true;

	HLodSaveClass hlod_save(connections, lod_count, CurTime, name, htree_name, meter, get_origin_list());
	if (!hlod_save.Save(csave))
		return false;
	return true;
}


/***********************************************************************************************
 * W3dExportClass::get_hierarchy_tree -- get a pointer to the hierarchy tree                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HierarchySaveClass * W3dExportClass::get_hierarchy_tree(void)
{
	/*
	** If the hierarchy tree pointer has been initialized, just return it
	*/
	if (HierarchyTree != NULL) return HierarchyTree;

	/*
	** If we are supposed to be loading a hierarchy from disk, then
	** load it
	*/
	if (!ExportOptions.ExportHierarchy) {
		HierarchyTree = load_hierarchy_file(HierarchyFilename);
		if (HierarchyTree) {
			return HierarchyTree;
		} else {
			char buf[256];
			sprintf(buf,"Unable to load hierarchy file: %s\nIf this Max file has been moved, please re-select the hierarchy file.",HierarchyFilename);
			MessageBox(MaxInterface->GetMAXHWnd(),buf,"Error",MB_OK | MB_SETFOREGROUND);
			return NULL;
		}
	}

	/*
	** Should never fall through to here...
	** This would only happen if ExportHierarchy was true and the Export_Hierarchy
	** function failed to create a hierarchy tree for us.
	*/
	assert(0);
	return NULL;
}


/***********************************************************************************************
 * W3dExportClass::get_damage_root_list -- gets the list of damage root nodes                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/17/2000 gth : Created.                                                                 *
 *=============================================================================================*/
INodeListClass * W3dExportClass::get_damage_root_list(void)
{
	if (DamageRootList != NULL) return DamageRootList;

	/*
	** Create a list of all damage root objects in the scene.
	*/
	DamageRootFilterClass nodefilter;
	DamageRootList = new INodeListClass(ExportInterface->theScene, CurTime, &nodefilter);
	return DamageRootList;
}


/***********************************************************************************************
 * get_origin_list -- get the list of origin nodes                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/13/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
INodeListClass * W3dExportClass::get_origin_list(void)
{
	if (OriginList != NULL) return OriginList;

	/*
	** Create a list of all origins in the scene.
	*/
	static OriginFilterClass originfilter;
	OriginList = new INodeListClass (ExportInterface->theScene, CurTime, &originfilter);

	/*
	** If we didn't find any origins, add the scene root as an origin.
	** NOTE: it would also be a problem if the origin list contained both the scene root
	** and the user placed origins.  Thats not happening now because the OriginList
	** does not collect the scene root... were that to change we'd have to update this
	** code as well.
	*/
	if (OriginList->Num_Nodes() == 0) {
		OriginList->Insert(MaxInterface->GetRootNode());
	}

	return OriginList;
}

/***********************************************************************************************
 * W3dExportClass::get_export_options -- get the export options                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1997 GH  : Created.                                                                 *
 *   9/30/1999  AJA : Added support for the MAX suppress_prompts flag.                         *
 *=============================================================================================*/
bool W3dExportClass::get_export_options(BOOL suppress_prompts)
{
	int ticksperframe = GetTicksPerFrame();

	// Get the last export settings from the AppDataChunk attached to the
	// scene pointer. If there is no such AppDataChunk create one and set it
	// to default values.

	W3dExportOptionsStruct *options = NULL;

	AppDataChunk * appdata = MaxInterface->GetScenePointer()->GetAppDataChunk(W3D_EXPORTER_CLASS_ID,SCENE_EXPORT_CLASS_ID,0);

	if (appdata) {
		options = &(((ExportInfoAppDataChunkStruct *)(appdata->data))->ExportOptions);
	} else {

		ExportInfoAppDataChunkStruct *appdata_struct =
			(ExportInfoAppDataChunkStruct *)malloc(sizeof(ExportInfoAppDataChunkStruct));

		options = &(appdata_struct->ExportOptions);

		options->ExportHierarchy	= true;
		options->LoadHierarchy		= false;
		options->ExportAnimation	= true;
		options->EnableTerrainMode	= false;

		options->ReduceAnimation 	= false;
		options->ReduceAnimationPercent = 50;

		options->CompressAnimation							= false;
		options->CompressAnimationFlavor					= ANIM_FLAVOR_TIMECODED;
		options->CompressAnimationTranslationError	= 0.001f;	//DEFAULT_LOSSY_ERROR_TOLERANCE;
		options->CompressAnimationRotationError		= 0.050f;	//DEFAULT_LOSSY_ERROR_TOLERANCE;
		options->ReviewLog									= false;

		options->ExportGeometry			= true;
		options->TranslationOnly		= false;
		options->SmoothBetweenMeshes	= true;

		strcpy(options->HierarchyFilename,"");
		strcpy(options->RelativeHierarchyFilename,"");
		options->StartFrame = MaxInterface->GetAnimRange().Start() / ticksperframe;
		options->EndFrame = MaxInterface->GetAnimRange().End() / ticksperframe;
		options->UseVoxelizer = false;
		options->DisableExportAABTrees = true;
		options->EnableOptimizeMeshData = false;
		options->EnableMaterialColorToTextureConversion = false;

		memset(&(appdata_struct->Padding), 0, sizeof(appdata_struct->Padding));

		MaxInterface->GetScenePointer()->AddAppDataChunk(W3D_EXPORTER_CLASS_ID,
			SCENE_EXPORT_CLASS_ID, 0, sizeof(ExportInfoAppDataChunkStruct),
			appdata_struct);

	}

	// (gth) disabling the 'optimize mesh data' feature due to problems with external tools
	options->EnableOptimizeMeshData = false;

	bool retval = true;
	if (suppress_prompts == FALSE)
	{
		W3dOptionsDialogClass dialog(MaxInterface,ExportInterface);
		retval = dialog.Get_Export_Options(options);
	}

	if (suppress_prompts || retval) {

		ExportOptions = *options;

		if ( (suppress_prompts == TRUE) && (options->RelativeHierarchyFilename[0] != 0) )
		{
			// Use the relative pathname WRT the max scene's directory to
			// figure out the absolute directory where the hierarchy file
			// is stored.
			char curdir[_MAX_DRIVE + _MAX_DIR + 1];
			assert(_getcwd(curdir, sizeof(curdir)));
			assert(_chdir(CurrentScenePath) != -1);
			assert(_fullpath(HierarchyFilename, options->RelativeHierarchyFilename,
				sizeof(HierarchyFilename)));
			assert(_chdir(curdir) != -1);
		}
		else
			strcpy(HierarchyFilename,options->HierarchyFilename);

		if (ExportOptions.TranslationOnly) {
			FixupType = HierarchySaveClass::MATRIX_FIXUP_TRANS;
		} else {
			FixupType = HierarchySaveClass::MATRIX_FIXUP_TRANS_ROT;
		}
	}

	return retval;
}




/***********************************************************************************************
 * W3dExportClass::Start_Progress_Bar -- start the MAX progress meter                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void W3dExportClass::Start_Progress_Bar(void)
{
	MaxInterface->ProgressStart(
		"Processing Triangle Mesh",
		TRUE,
		progress_callback,
		NULL);
}

/***********************************************************************************************
 * W3dExportClass::End_Progress_Bar -- end the progress meter                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/16/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void W3dExportClass::End_Progress_Bar(void)
{
	MaxInterface->ProgressUpdate( 100);
	MaxInterface->ProgressEnd();
}

static bool dupe_check(const INodeListClass & list)
{
	for (unsigned i=0; i<list.Num_Nodes(); i++) {

		/*
		** Don't check aggregate objects, they are allowed to have the same name
		*/
		if (!Is_Aggregate(list[i])) {
			for (unsigned j = i+1; j<list.Num_Nodes(); j++) {
				if (stricmp(list[i]->GetName(),list[j]->GetName()) == 0) {
					char buf[256];
					sprintf(buf,"Geometry Nodes with duplicated names found!\nDuplicated Name: %s\n",list[i]->GetName());
					MessageBox(NULL,buf,"Error",MB_OK | MB_SETFOREGROUND);
					return true;
				}
			}
		}
	}
	return false;
}

static bool check_lod_extensions (INodeListClass &list, INode *origin)
{
	/*
	** Assumptions:
	** - If origin == NULL, then we're just exporting a single model and don't need to
	** worry about lod extensions at all.
	** - If origin is the root of the scene, then we're just exporting a single model as well.
	** - Otherwise origin actually points to an Origin and not just any INode.
	*/
	if (origin == NULL) return true;
	if (origin->IsRootNode()) return true;

	char	*extension = strrchr(origin->GetName(), '.');
	int	ext_len = strlen(extension);
	for (unsigned i = 0; i < list.Num_Nodes(); i++)
	{
		char *this_ext = strrchr(list[i]->GetName(), '.');

		// Check for the existance of an extension in this node.
		if (this_ext == NULL)
			return false;

		// Check that the extensions are the same.
		if (strcmp(this_ext, extension) != 0)
			return false;
	}

	return true;
}


bool W3dExportClass::get_base_object_tm (Matrix3 &tm)
{
	INodeListClass	*origin_list = get_origin_list();
	if (!origin_list)
		return false;

	unsigned int	i, count = origin_list->Num_Nodes();
	INode				*base_origin = NULL;
	for (i = 0; i < count; i++)
	{
		INode *node = (*origin_list)[i];
		if (Is_Base_Origin(node))
		{
			// we found origin.00, fall through
			base_origin = node;
			break;
		}
	}
	if (!base_origin)
		return false;

	tm = base_origin->GetNodeTM(CurTime);
	return true;
}

static DWORD WINAPI progress_callback( LPVOID arg )
{
	return 0;
}


static HierarchySaveClass * load_hierarchy_file(char * filename)
{
	HierarchySaveClass * hier = NULL;

	RawFileClass file(filename);

	if (!file.Open()) {
		return NULL;
	}
	ChunkLoadClass cload(&file);

	cload.Open_Chunk();
	if (cload.Cur_Chunk_ID() == W3D_CHUNK_HIERARCHY) {
		hier = new HierarchySaveClass();
		hier->Load(cload);
	} else {
		hier = NULL;
		file.Close();
		return NULL;
	}

	cload.Close_Chunk();
	file.Close();

	return hier;
}

