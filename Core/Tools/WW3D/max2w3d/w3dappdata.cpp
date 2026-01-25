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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dappdata.cpp                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/21/01 9:45a                                               $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Is_Bone -- check if a node is a "bone"                                                    *
 *   Is_Geometry -- Check if an INode is marked for "geometry" export                          *
 *   Is_Normal_Mesh -- check if a node is a mesh                                               *
 *   Is_Camera_Aligned_Mesh -- check if a node is to be a camera aligned mesh                  *
 *   Is_Camera_Oriented_Mesh -- check if a node is to be a camera oriented mesh                *
 *   Is_Collision_AABox -- check if a node is a collision box mesh                             *
 *   Is_Collision_OBBox -- check if a node is a collision box mesh                             *
 *   Is_Skin -- check if a node is a skin                                                      *
 *   Is_Shadow -- check if a node marked to cast shadows                                       *
 *   Is_Damage -- check if a node is a damage version of a mesh                                *
 *   Is_Damage_For_Mesh -- check if a node is a damage version of a particular mesh            *
 *   Is_Physical_Collision -- Is node categorized as "physical collision geometry"             *
 *   Is_Projectile_Collision -- Is node categorized as "projectile collision geometry"?        *
 *   Is_Vis_Collision -- Is node categorized as "vis collision geometry"?                      *
 *   Is_Camera_Collision -- Is node categorized as "camera collision geometry"?                *
 *   Is_Hidden -- checks whether the node should be hidden                                     *
 *   Is_Two_Sided -- Checks whether the node should be two sided                               *
 *   Is_ZNormals -- checks if the node should be exported with 0,0,1 vert normals              *
 *   Is_Dazzle -- check if a node is a dazzle object                                           *
 *   Is_Aggregate -- check if a node is an aggregate object                                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "w3dappdata.h"
#include "w3dutil.h"
#include "util.h"
#include "modstack.h"


/***********************************************************************************************
**
** W3DAppData2Struct Implementation
**
***********************************************************************************************/

/*
** Current version of the W3dAppData2 structure
** History:
** version 0 - initial implementation.
** version 1 - added camera collision option, initialize to match physical collision settting
**             when converting from version 0.
*/
const int W3DAPPDATA2_CURRENT_VERSION = 1;



W3DAppData2Struct::W3DAppData2Struct(void) :
	ExportFlags(0),
	GeometryType(0),
	GeometryFlags(0),
	CollisionFlags(0)
{
	memset(UnUsed,0,sizeof(UnUsed));
	Init_With_Other_Defaults();
}

W3DAppData2Struct::W3DAppData2Struct(W3DAppData0Struct & olddata)	:
	ExportFlags(0),
	GeometryType(0),
	GeometryFlags(0),
	CollisionFlags(0)
{
	memset(UnUsed,0,sizeof(UnUsed));
	Init_From_AppData0(olddata);
}

void W3DAppData2Struct::Init_With_Mesh_Defaults(void)
{
	ExportFlags = (EXPORT_TRANSFORM|EXPORT_GEOMETRY);
	GeometryType = GEO_TYPE_NORMAL_MESH;
	GeometryFlags = 0;
	CollisionFlags = 0;
	memset(UnUsed,0,sizeof(UnUsed));

	Set_Version(W3DAPPDATA2_CURRENT_VERSION);
}

void	W3DAppData2Struct::Init_With_Other_Defaults(void)
{
	ExportFlags = 0;
	GeometryType = 0;
	GeometryFlags = 0;
	CollisionFlags = 0;
	memset(UnUsed,0,sizeof(UnUsed));

	Set_Version(W3DAPPDATA2_CURRENT_VERSION);
}

void	W3DAppData2Struct::Init_From_AppData0(W3DAppData0Struct & olddata)
{
	Init_With_Other_Defaults();

	Enable_Export_Transform(olddata.Is_Bone());
	Enable_Export_Geometry(olddata.Is_Geometry());

	if (olddata.Is_Camera_Aligned_Mesh()) {
		Set_Geometry_Type(GEO_TYPE_CAMERA_ALIGNED);
	}
	if (olddata.Is_Camera_Oriented_Mesh()) {
		Set_Geometry_Type(GEO_TYPE_CAMERA_ORIENTED);
	}
	if (olddata.Is_Collision_AABox()) {
		Set_Geometry_Type(GEO_TYPE_AABOX);
	}
	if (olddata.Is_Collision_OBBox()) {
		Set_Geometry_Type(GEO_TYPE_OBBOX);
	}
	if (olddata.Is_Normal_Mesh()) {
		Set_Geometry_Type(GEO_TYPE_NORMAL_MESH);
	}
	if (olddata.Is_Null()) {
		Set_Geometry_Type(GEO_TYPE_NULL);
	}

	Enable_Hidden(olddata.Is_Hidden());
	Enable_Two_Sided(olddata.Is_Two_Sided());
	Enable_Shadow(olddata.Is_Shadow());
	Enable_Vertex_Alpha(olddata.Is_Vertex_Alpha());
	Enable_ZNormals(olddata.Is_ZNormals());
	Enable_Physical_Collision(olddata.Is_Physical_Collision());
	Enable_Projectile_Collision(olddata.Is_Projectile_Collision());
	Enable_Vis_Collision(olddata.Is_Vis_Collision());
	Enable_Camera_Collision(olddata.Is_Physical_Collision());		// make camera setting match physical

	Set_Version(W3DAPPDATA2_CURRENT_VERSION);
}

void W3DAppData2Struct::Update_Version(void)
{
	/*
	** If this is a version 0 struct, We need to initialize the camera collision
	** setting to be equal to the physical collision setting and convert to version 1
	*/
	if (Get_Version() == 0) {
		Enable_Camera_Collision(Is_Physical_Collision_Enabled());
		Set_Version(1);
	}

	/*
	** Add future version conversion code here!
	*/
}

bool	W3DAppData2Struct::operator == (const W3DAppData2Struct & that)
{
	/*
	** Export flags have to match
	*/
	if (ExportFlags != that.ExportFlags) return false;

	/*
	** If geometry is enabled, verify the geometry type and options
	*/
	if (Is_Geometry()) {
		if (Get_Geometry_Type() != that.Get_Geometry_Type()) return false;
		if (GeometryFlags != that.GeometryFlags) return false;
		if (CollisionFlags != that.CollisionFlags) return false;
	}

	return true;
}

bool W3DAppData2Struct::Geometry_Options_Match(const W3DAppData2Struct & that)
{
	if (Get_Geometry_Type() != that.Get_Geometry_Type()) return false;
	if (GeometryFlags != that.GeometryFlags) return false;
	if (CollisionFlags != that.CollisionFlags) return false;
	return true;
}


W3DAppData2Struct * W3DAppData2Struct::Get_App_Data
(
	INode * node,
	bool create_if_missing
)
{
	/*
	** Try to get our AppData which has the export flags
	*/
	W3DAppData2Struct * wdata = nullptr;
	AppDataChunk * appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,W3D_APPDATA_2);

	/*
	** If there wasn't one, look for an AppData0 chunk and convert that.
	** If there was one, get the data from it
	*/
	if (appdata) {

		wdata = (W3DAppData2Struct *)(appdata->data);
		wdata->Update_Version();

	} else {

		/*
		** Create a new W3DAppData2Struct for this node
		*/
		wdata = (W3DAppData2Struct *)malloc(sizeof(W3DAppData2Struct));

		/*
		** If we have the old AppData0 then initialize from it otherwise
		** just initialize to the defaults
		*/
		AppDataChunk * oldappdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,W3D_APPDATA_0);
		if (oldappdata) {

			/*
			** initializing from the old app data 0 chunk;
			*/
			wdata->Init_From_AppData0(*(W3DAppData0Struct*)(oldappdata->data));

			/*
			** Destroy the old obsolete App Data 0 chunk
			*/
			node->RemoveAppDataChunk(W3DUtilityClassID, UTILITY_CLASS_ID, W3D_APPDATA_0);

		} else {

			/*
			** If this object looks like it is going to be a mesh, then default
			** it to have the mesh export flags (export its transform and the
			** triangle mesh).  Otherwise use the default of completely ignoring it!
			*/
			if (Is_Max_Tri_Mesh(node)) {
				wdata->Init_With_Mesh_Defaults();
			} else {
				wdata->Init_With_Other_Defaults();
			}

		}
		node->AddAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,2,sizeof(W3DAppData2Struct),wdata);
		appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,2);
		assert(appdata);

	}

	return wdata;
}


/***********************************************************************************************
**
** W3DDazzleAppDataStruct Implementation
**
***********************************************************************************************/
W3DDazzleAppDataStruct::W3DDazzleAppDataStruct(void)
{
	UnUsed[0] = 0;
	UnUsed[1] = 0;
	UnUsed[2] = 0;
	UnUsed[3] = 0;
	memset(DazzleType,0,sizeof(DazzleType));
	strcpy(DazzleType,"DEFAULT");
}


W3DDazzleAppDataStruct * W3DDazzleAppDataStruct::Get_App_Data(INode * node,bool create_if_missing)
{
	/*
	** Try to get the existing AppData chunk
	*/
	W3DDazzleAppDataStruct * dazzledata = nullptr;
	AppDataChunk * appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,W3D_DAZZLE_APPDATA);

	if (appdata) {

		/*
		** Found it, get the pointer to the dazzle data
		*/
		dazzledata = (W3DDazzleAppDataStruct *)(appdata->data);

	} else if (create_if_missing) {

		/*
		** Create a new W3DDazzleAppDataStruct for this node
		*/
		dazzledata = new W3DDazzleAppDataStruct;
		node->AddAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,W3D_DAZZLE_APPDATA,sizeof(W3DDazzleAppDataStruct),dazzledata);

		appdata = node->GetAppDataChunk(W3DUtilityClassID,UTILITY_CLASS_ID,W3D_DAZZLE_APPDATA);
		assert(appdata);
	}
	return dazzledata;
}





/***********************************************************************************************
**
**	Utility functions for reading the W3D options that are set for a given INode
**
***********************************************************************************************/

static int get_geometry_type(INode * node)
{
	assert(node != nullptr);
	return W3DAppData2Struct::Get_App_Data(node)->Get_Geometry_Type();
}

/***********************************************************************************************
 * Is_Bone -- check if a node is a "bone"                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool Is_Bone(INode * node)
{
	/*
	** regardless of the bits, skins should not have their transform exported.
	*/
	if (Is_Skin(node)) return false;
	if (Is_Origin(node)) return false;
	//if (Is_Proxy(*node)) return false;
	else return (W3DAppData2Struct::Get_App_Data(node)->Is_Bone());
}

/***********************************************************************************************
 * Is_Geometry -- Check if an INode is marked for "geometry" export                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/13/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool Is_Geometry(INode * node)
{
	return (W3DAppData2Struct::Get_App_Data(node)->Is_Geometry());
}

/***********************************************************************************************
 * Is_Mesh -- check if a node is a mesh                                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool Is_Normal_Mesh(INode * node)
{
 	if (!Is_Geometry(node)) return false;
	if (Is_Skin(node)) return false;
	if (Is_Proxy(*node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_NORMAL_MESH);
}


/***********************************************************************************************
 * Is_Camera_Aligned_Mesh -- check if a node is to be a camera aligned mesh                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/5/98     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Is_Camera_Aligned_Mesh(INode * node)
{
	if (!Is_Geometry(node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_CAMERA_ALIGNED);
}

/***********************************************************************************************
 * Is_Camera_Oriented_Mesh -- check if a node is to be a camera oriented mesh                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/99     NH : Created.                                                                  *
 *=============================================================================================*/
bool Is_Camera_Oriented_Mesh(INode * node)
{
	if (!Is_Geometry(node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_CAMERA_ORIENTED);
}

/***********************************************************************************************
 * Is_Collision_AABox -- check if a node is a collision box                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool Is_Collision_AABox(INode * node)
{
	if (!Is_Geometry(node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_AABOX);
}


/***********************************************************************************************
 * Is_Collision_OBBox -- check if a node is a collision box                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/17/1998 GH  : Created.                                                                 *
 *=============================================================================================*/
bool Is_Collision_OBBox(INode * node)
{
	if (!Is_Geometry(node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_OBBOX);
}

/***********************************************************************************************
 * Is_Skin -- check if a node is a skin                                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool Is_Skin(INode * node)
{
	if (!Is_Geometry(node)) return false;
	if (get_geometry_type(node) != W3DAppData2Struct::GEO_TYPE_NORMAL_MESH) return false;

	for (int i = 0; i < node->NumRefs(); i++) {

		ReferenceTarget *refTarg = node->GetReference(i);

		if (refTarg != nullptr && refTarg->ClassID() == Class_ID(WSM_DERIVOB_CLASS_ID,0)) {

			IDerivedObject * wsm_der_obj = (IDerivedObject *)refTarg;
			//MessageBox(nullptr, "WSM found", _T("WSM"), MB_OK);

			for (int j = 0; j < wsm_der_obj->NumModifiers(); j++) {
				Modifier * mod = wsm_der_obj->GetModifier(j);
				if (mod->ClassID() == SKIN_MOD_CLASS_ID) {
					return true;
				}
			}
		}
	}

	return false;
}

/***********************************************************************************************
 * Is_Shadow -- check if a node marked to cast shadows                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool Is_Shadow(INode * node)
{
	if (!Is_Geometry(node)) return false;
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Shadow_Enabled());
}

/***********************************************************************************************
 * Is_Null_Object -- check if a node is a null object                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool Is_Null_Object(INode * node)
{
	if (!Is_Geometry(node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_NULL);
}


/***********************************************************************************************
 * Is_Dazzle -- check if a node is a dazzle object                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/5/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool	Is_Dazzle(INode * node)
{
	if (!Is_Geometry(node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_DAZZLE);
}


/***********************************************************************************************
 * Is_Aggregate -- check if a node is an aggregate object                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/24/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool Is_Aggregate(INode * node)
{
	if (!Is_Geometry(node)) return false;
	return (get_geometry_type(node) == W3DAppData2Struct::GEO_TYPE_AGGREGATE);
}

/***********************************************************************************************
 * Is_Physical_Collision -- Is node categorized as "physical collision geometry"               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/23/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool Is_Physical_Collision(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Physical_Collision_Enabled());
}


/***********************************************************************************************
 * Is_Projectile_Collision -- Is node categorized as "projectile collision geometry"?          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/23/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool Is_Projectile_Collision(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Projectile_Collision_Enabled());
}


/***********************************************************************************************
 * Is_Projectile_Collision -- Is node categorized as "vis collision geometry"?                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/23/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool Is_Vis_Collision(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Vis_Collision_Enabled());
}


/***********************************************************************************************
 * Is_Camera_Collision -- Is node categorized as "camera collision geometry"?                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/4/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool Is_Camera_Collision(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Camera_Collision_Enabled());
}

/***********************************************************************************************
 * Is_Vehicle_Collision -- Is node categorized as "Vehicle collision geometry"?                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/4/2000   gth : Created.                                                                 *
 *=============================================================================================*/
bool Is_Vehicle_Collision(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Vehicle_Collision_Enabled());
}

/***********************************************************************************************
 * Is_Hidden -- checks whether the node should be hidden                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This has nothing to do with its hidden status inside of max.  Things hidden in max are      *
 * ignored by the exporter.  (artist request way back...weird huh?)                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/18/98   GTH : Created.                                                                 *
 *=============================================================================================*/
bool	Is_Hidden(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Hidden_Enabled());
}


/***********************************************************************************************
 * Is_Two_Sided -- Checks whether the node should be two sided                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/18/98   GTH : Created.                                                                 *
 *=============================================================================================*/
bool	Is_Two_Sided(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Two_Sided_Enabled());
}


/***********************************************************************************************
 * Is_ZNormals -- checks if the node should be exported with 0,0,1 vert normals                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/9/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool	Is_ZNormals(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_ZNormals_Enabled());
}


bool	Is_Vertex_Alpha(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Vertex_Alpha_Enabled());
}

bool	Is_Shatterable(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_Shatterable_Enabled());
}

bool	Is_NPatchable(INode * node)
{
	W3DAppData2Struct * wdata = W3DAppData2Struct::Get_App_Data(node);
	return (wdata->Is_NPatchable_Enabled());
}




