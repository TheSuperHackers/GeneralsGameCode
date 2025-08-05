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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dappdata.h                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/21/01 9:44a                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef W3DAPPDATA_H
#define W3DAPPDATA_H

#include <max.h>

/*
** The W3DUtilityClass uses several app-data chunks to store user-options for INodes
** in the MAX scene.  Below are the IDs for each app-data chunk type:
*/

#define W3D_APPDATA_0			0
#define W3D_APPDATA_1			1
#define W3D_APPDATA_2			2
#define W3D_DAZZLE_APPDATA		3


/*
**	Classifying INodes for w3d exporting
**
** NOTE: Use these utility functions rather than going straight to the AppData
** structure!
**
**  - There are bits stored in AppData for each node
**  - These bits indicate wether something should be exported as hierarchy,
**    geometry (and if so, what type of geometry: mesh, collision box, bitmap, etc)
**
** When we say something is "Hierarchy" that means its transform should be put
** into any hierarchy tree or motion being created.  In some places I used
** the term "Bone" which means the same thing.
**
** When we say something is "Geometry" that means that the object will become
** some sort of render object (normal mesh, bitmap, collision mesh, etc)
**
*/

/*
** Either or both of these will return true for a given INode
*/
bool	Is_Bone(INode * node);
bool	Is_Geometry(INode * node);

/*
** Geometry Type:
** One of the following will return true if the given INode is
** to have its geometry exported
*/
bool	Is_Normal_Mesh(INode * node);
bool	Is_Camera_Aligned_Mesh(INode * node);
bool	Is_Camera_Oriented_Mesh(INode * node);
bool	Is_Collision_AABox(INode * node);
bool	Is_Collision_OBBox(INode * node);
bool	Is_Skin(INode * node);
bool	Is_Shadow(INode * node);
bool	Is_Null_Object(INode * node);
bool	Is_Dazzle(INode * node);
bool	Is_Aggregate(INode * node);

/*
** Collision Bits, any or all of these may return true for a given INode
*/
bool	Is_Physical_Collision(INode * node);
bool	Is_Projectile_Collision(INode * node);
bool	Is_Vis_Collision(INode * node);
bool	Is_Camera_Collision(INode * node);
bool	Is_Vehicle_Collision(INode * node);

/*
** Miscellaneous settings
*/
bool	Is_Hidden(INode * node);
bool	Is_Two_Sided(INode * node);
bool	Is_ZNormals(INode * node);
bool	Is_Vertex_Alpha(INode * node);
bool	Is_Shatterable(INode * node);
bool	Is_NPatchable(INode * node);

/*
** Proxy support.  If a node has a name which contains a ~ it is considered a
** proxy for an application defined named object.  This overrides all other
** settings (in the future, we shouldn't do things this way!)
*/
inline bool Is_Proxy(INode &node)
{
	return (::strchr (node.GetName (), '~') != NULL);
}



/*
** AJA 9/24/99
** NOTE: Whenever you add a new W3DAppDataStruct, you must add an accessor function
** to the bottom of this file. Then the WWScript.dlx project must be modified. That
** project implements an extension to the MAXScript language in the form of a function
** called "wwCopyAppData". The implementation of this wwCopyAppData function must be
** aware of the new W3DAppDataStruct. The modification is pretty straightfoward.
**
** The wwCopyAppData extension was added to MAXScript so that the data we define
** below can be preserve when a model is cloned (copy/instance/reference). What
** happens without using wwCopyAppData is that a new INode is created that contains
** a copy/instance/reference to the original mesh/bone in the source model. When
** this new INode is created, it has no app data. When the app data is examined later
** on, it gets the default values! That means all information concerning Export
** Geometry/Transform, mesh type, and damage region is lost. wwCopyAppData allows
** a script to duplicate a model INCLUDING all W3D app data, so that this information
** is preserved.
*/


/*
** The W3DAppData0Struct contains a bitfield.  These #defines are
** used to interpret the bits.
** (gth) NOTE: AppData0 is now OBSOLETE!!!  Use W3DAppData2Struct now.
*/
#define EXPORT_TYPE_MASK					0x000F
#define GEOMETRY_TYPE_MASK					0x01F0
#define COLLISION_TYPE_MASK				0xF000

#define EXPORT_BONE_FLAG					0x0001	// export a bone (transform) for this node
#define EXPORT_GEOMETRY_FLAG				0x0002	// export the geometry for this node
#define EXPORT_HIDDEN_FLAG					0x0004	// mesh should be hidden by default
#define EXPORT_TWO_SIDED_FLAG				0x0008	// mesh should be two sided

#define GEOMETRY_TYPE_CAMERA_ALIGNED	0x0010	// interpret this geometry as a camera-aligned mesh
#define GEOMETRY_TYPE_NORMAL_MESH		0x0020	// this is a normal mesh
#define GEOMETRY_TYPE_OBBOX				0x0030	// this is an oriented box (should have 8 verts, etc)
#define GEOMETRY_TYPE_AABOX				0x0040	// this is an axis aligned box
#define GEOMETRY_TYPE_CAMERA_ORIENTED	0x0050	// interpret this geometry as a camera-oriented mesh
#define GEOMETRY_TYPE_NULL					0x0100	// this is a null (for LOD)

#define EXPORT_CAST_SHADOW_FLAG			0x0200	// this mesh casts a shadow
#define EXPORT_VERTEX_ALPHA_FLAG			0x0400	// convert vertex colors to alpha
#define EXPORT_ZNORMALS_FLAG				0x0800	// force vertex normals to point along +z

#define COLLISION_TYPE_PHYSICAL			0x1000	// runtime engine performs physical collision against this mesh
#define COLLISION_TYPE_PROJECTILE		0x2000	// perform projectile collisions against this mesh
#define COLLISION_TYPE_VIS					0x4000	// perform vis group collisions against this mesh

#define DEFAULT_MESH_EXPORT_FLAGS		(EXPORT_BONE_FLAG | EXPORT_GEOMETRY_FLAG | GEOMETRY_TYPE_NORMAL_MESH)
#define DEFAULT_EXPORT_FLAGS				0


/*
** W3D Utility AppData sub-type 0  (OBSOLETE!)
** ----------------------------------------------------
** The utility provides a right-click menu which allows
** the user to toggle the export flags.
** (gth) NOTE: AppData0 is now OBSOLETE!!!  Use W3DAppData2Struct now.
*/
struct W3DAppData0Struct
{
	W3DAppData0Struct(void) : ExportFlags(DEFAULT_EXPORT_FLAGS) {}

	bool	Is_Bone(void)							{ return (ExportFlags & EXPORT_BONE_FLAG) == EXPORT_BONE_FLAG; }
	bool	Is_Geometry(void)						{ return (ExportFlags & EXPORT_GEOMETRY_FLAG) == EXPORT_GEOMETRY_FLAG; }

	bool	Is_Normal_Mesh(void)					{ return (ExportFlags & GEOMETRY_TYPE_MASK) == GEOMETRY_TYPE_NORMAL_MESH; }
	bool	Is_Camera_Aligned_Mesh(void)		{ return (ExportFlags & GEOMETRY_TYPE_MASK) == GEOMETRY_TYPE_CAMERA_ALIGNED; }
	bool	Is_Camera_Oriented_Mesh(void)		{ return (ExportFlags & GEOMETRY_TYPE_MASK) == GEOMETRY_TYPE_CAMERA_ORIENTED; }
	bool	Is_Collision_AABox(void)			{ return (ExportFlags & GEOMETRY_TYPE_MASK) == GEOMETRY_TYPE_AABOX; }
	bool	Is_Collision_OBBox(void)			{ return (ExportFlags & GEOMETRY_TYPE_MASK) == GEOMETRY_TYPE_OBBOX; }
	bool	Is_Null(void)							{ return (ExportFlags & GEOMETRY_TYPE_MASK) == GEOMETRY_TYPE_NULL; }

	bool	Is_Physical_Collision(void)		{ return (ExportFlags & COLLISION_TYPE_PHYSICAL) == COLLISION_TYPE_PHYSICAL; }
	bool	Is_Projectile_Collision(void)		{ return (ExportFlags & COLLISION_TYPE_PROJECTILE) == COLLISION_TYPE_PROJECTILE; }
	bool	Is_Vis_Collision(void)				{ return (ExportFlags & COLLISION_TYPE_VIS) == COLLISION_TYPE_VIS; }

	bool	Is_Hidden(void)						{ return (ExportFlags & EXPORT_HIDDEN_FLAG) == EXPORT_HIDDEN_FLAG; }
	bool	Is_Two_Sided(void)					{ return (ExportFlags & EXPORT_TWO_SIDED_FLAG) == EXPORT_TWO_SIDED_FLAG; }
	bool	Is_Vertex_Alpha(void)				{ return (ExportFlags & EXPORT_VERTEX_ALPHA_FLAG) == EXPORT_VERTEX_ALPHA_FLAG; }
	bool	Is_ZNormals(void)						{ return (ExportFlags & EXPORT_ZNORMALS_FLAG) == EXPORT_ZNORMALS_FLAG; }
	bool	Is_Shadow(void)						{ return (ExportFlags & EXPORT_CAST_SHADOW_FLAG) == EXPORT_CAST_SHADOW_FLAG; }

	unsigned short ExportFlags;	// what was I thinking??? (gth)
};



/*
** W3D Utility AppData sub-type 1
** ----------------------------------------------------
** This AppData contains the damage region number for
** the current object.
*/

// Maximum number of damage regions on a model.
#define MAX_DAMAGE_REGIONS		((char)16)
// Value that represents no damage region.
#define NO_DAMAGE_REGION		((char)-1)

struct W3DAppData1Struct
{
	W3DAppData1Struct(void) : DamageRegion(NO_DAMAGE_REGION) { }

	/*
	** NO_DAMAGE_REGION means the object isn't part of
	** any damage region, 0 through MAX_DAMAGE_REGIONS-1
	** are valid damage regions.
	*/
	char DamageRegion;
};



/*
** W3D Utility AppData sub-type 2
** ----------------------------------------------------
** This is an app-data struct that is meant to contain all
** of the w3d export options for an object in MAX.  It
** replaces the old AppData sub-type 0.  Call the static
** member functions in the class to get a pointer to
** the AppData2 struct hanging off any INode and automatically
** create one for you if there isn't already one...
*/

struct W3DAppData2Struct
{
	W3DAppData2Struct(void);
	W3DAppData2Struct(W3DAppData0Struct & olddata);

	void	Init_With_Mesh_Defaults(void);
	void	Init_With_Other_Defaults(void);
	void	Init_From_AppData0(W3DAppData0Struct & olddata);
	void	Update_Version(void);

	enum GeometryTypeEnum
	{
		GEO_TYPE_CAMERA_ALIGNED =	0x00000001,		// Geometry types are mutually exclusive
		GEO_TYPE_NORMAL_MESH =		0x00000002,
		GEO_TYPE_OBBOX =				0x00000003,
		GEO_TYPE_AABOX =				0x00000004,
		GEO_TYPE_CAMERA_ORIENTED =	0x00000005,
		GEO_TYPE_NULL =				0x00000006,
		GEO_TYPE_DAZZLE =				0x00000007,
		GEO_TYPE_AGGREGATE =			0x00000008,
	};

	/*
	** Read Access
	*/
	bool	Is_Bone(void) const							{ return (ExportFlags & EXPORT_TRANSFORM) == EXPORT_TRANSFORM; }
	bool	Is_Geometry(void)	const						{ return (ExportFlags & EXPORT_GEOMETRY) == EXPORT_GEOMETRY; }

	int	Get_Geometry_Type(void)	const				{ return GeometryType; }
	bool	Is_Normal_Mesh(void)	const					{ return GeometryType == GEO_TYPE_NORMAL_MESH; }
	bool	Is_Camera_Aligned_Mesh(void) const		{ return GeometryType == GEO_TYPE_CAMERA_ALIGNED; }
	bool	Is_Camera_Oriented_Mesh(void) const		{ return GeometryType == GEO_TYPE_CAMERA_ORIENTED; }
	bool	Is_Collision_AABox(void) const			{ return GeometryType == GEO_TYPE_AABOX; }
	bool	Is_Collision_OBBox(void) const			{ return GeometryType == GEO_TYPE_OBBOX; }
	bool	Is_Null(void) const							{ return GeometryType == GEO_TYPE_NULL; }
	bool	Is_Dazzle(void) const 						{ return GeometryType == GEO_TYPE_DAZZLE; }

	bool	Is_Hidden_Enabled(void) const				{ return (GeometryFlags & GEOMETRY_FLAG_HIDDEN) == GEOMETRY_FLAG_HIDDEN; }
	bool	Is_Two_Sided_Enabled(void) const			{ return (GeometryFlags & GEOMETRY_FLAG_TWO_SIDED) == GEOMETRY_FLAG_TWO_SIDED; }
	bool	Is_Vertex_Alpha_Enabled(void) const		{ return (GeometryFlags & GEOMETRY_FLAG_VERTEX_ALPHA) == GEOMETRY_FLAG_VERTEX_ALPHA; }
	bool	Is_ZNormals_Enabled(void) const			{ return (GeometryFlags & GEOMETRY_FLAG_ZNORMALS) == GEOMETRY_FLAG_ZNORMALS; }
	bool	Is_Shadow_Enabled(void) const				{ return (GeometryFlags & GEOMETRY_FLAG_CAST_SHADOW) == GEOMETRY_FLAG_CAST_SHADOW; }
	bool	Is_Shatterable_Enabled(void) const		{ return (GeometryFlags & GEOMETRY_FLAG_SHATTERABLE) == GEOMETRY_FLAG_SHATTERABLE; }
	bool	Is_NPatchable_Enabled(void) const		{ return (GeometryFlags & GEOMETRY_FLAG_NPATCHABLE) == GEOMETRY_FLAG_NPATCHABLE; }

	bool	Is_Physical_Collision_Enabled(void) const		{ return (CollisionFlags & COLLISION_FLAG_PHYSICAL) == COLLISION_FLAG_PHYSICAL; }
	bool	Is_Projectile_Collision_Enabled(void) const	{ return (CollisionFlags & COLLISION_FLAG_PROJECTILE) == COLLISION_FLAG_PROJECTILE; }
	bool	Is_Vis_Collision_Enabled(void) const			{ return (CollisionFlags & COLLISION_FLAG_VIS) == COLLISION_FLAG_VIS; }
	bool	Is_Camera_Collision_Enabled(void) const		{ return (CollisionFlags & COLLISION_FLAG_CAMERA) == COLLISION_FLAG_CAMERA; }
	bool	Is_Vehicle_Collision_Enabled(void) const		{ return (CollisionFlags & COLLISION_FLAG_VEHICLE) == COLLISION_FLAG_VEHICLE; }

	/*
	** Write Access
	*/
	void	Enable_Export_Transform(bool onoff)			{ if (onoff) { ExportFlags |= EXPORT_TRANSFORM; } else { ExportFlags &= ~EXPORT_TRANSFORM; } }
	void	Enable_Export_Geometry(bool onoff)			{ if (onoff) { ExportFlags |= EXPORT_GEOMETRY; } else { ExportFlags &= ~EXPORT_GEOMETRY; } }

	void	Set_Geometry_Type(GeometryTypeEnum type)	{ GeometryType = (unsigned int)type; }

	void	Enable_Hidden(bool onoff)						{ if (onoff) { GeometryFlags |= GEOMETRY_FLAG_HIDDEN; } else { GeometryFlags &= ~GEOMETRY_FLAG_HIDDEN; } }
	void	Enable_Two_Sided(bool onoff)					{ if (onoff) { GeometryFlags |= GEOMETRY_FLAG_TWO_SIDED; } else { GeometryFlags &= ~GEOMETRY_FLAG_TWO_SIDED; } }
	void	Enable_Shadow(bool onoff)						{ if (onoff) { GeometryFlags |= GEOMETRY_FLAG_CAST_SHADOW; } else { GeometryFlags &= ~GEOMETRY_FLAG_CAST_SHADOW; } }
	void	Enable_Vertex_Alpha(bool onoff)				{ if (onoff) { GeometryFlags |= GEOMETRY_FLAG_VERTEX_ALPHA; } else { GeometryFlags &= ~GEOMETRY_FLAG_VERTEX_ALPHA; } }
	void	Enable_ZNormals(bool onoff)					{ if (onoff) { GeometryFlags |= GEOMETRY_FLAG_ZNORMALS; } else { GeometryFlags &= ~GEOMETRY_FLAG_ZNORMALS; } }
	void	Enable_Shatterable(bool onoff)				{ if (onoff) { GeometryFlags |= GEOMETRY_FLAG_SHATTERABLE; } else { GeometryFlags &= ~GEOMETRY_FLAG_SHATTERABLE; } }
	void	Enable_NPatchable(bool onoff)					{ if (onoff) { GeometryFlags |= GEOMETRY_FLAG_NPATCHABLE; } else { GeometryFlags &= ~GEOMETRY_FLAG_NPATCHABLE; } }

	void	Enable_Physical_Collision(bool onoff)		{ if (onoff) { CollisionFlags |= COLLISION_FLAG_PHYSICAL; } else { CollisionFlags &= ~COLLISION_FLAG_PHYSICAL; } }
	void	Enable_Projectile_Collision(bool onoff)	{ if (onoff) { CollisionFlags |= COLLISION_FLAG_PROJECTILE; } else { CollisionFlags &= ~COLLISION_FLAG_PROJECTILE; } }
	void	Enable_Vis_Collision(bool onoff)				{ if (onoff) { CollisionFlags |= COLLISION_FLAG_VIS; } else { CollisionFlags &= ~COLLISION_FLAG_VIS; } }
	void	Enable_Camera_Collision(bool onoff)			{ if (onoff) { CollisionFlags |= COLLISION_FLAG_CAMERA; } else { CollisionFlags &= ~COLLISION_FLAG_CAMERA; } }
	void	Enable_Vehicle_Collision(bool onoff)		{ if (onoff) { CollisionFlags |= COLLISION_FLAG_VEHICLE; } else { CollisionFlags &= ~COLLISION_FLAG_VEHICLE; } }

	/*
	** Comparison
	*/
	bool	operator == (const W3DAppData2Struct & that);
	bool	operator != (const W3DAppData2Struct & that)	{ return !(*this == that); }
	bool	Geometry_Options_Match(const W3DAppData2Struct & that);

	/*
	** Get the W3DAppData2Struct for a given INode and create one if
	** there isn't already one.
	*/
	static W3DAppData2Struct * Get_App_Data(INode * node,bool create_if_missing = true);

protected:

	void	Set_Version(int ver)								{ ExportFlags &= ~VERSION_MASK; ExportFlags |= (ver<<VERSION_SHIFT); }
	int	Get_Version(void)									{ return (ExportFlags & VERSION_MASK)>>VERSION_SHIFT; }

	enum ExportFlagsEnum
	{
		EXPORT_TRANSFORM =					0x00000001,		// Export flags bit-field
		EXPORT_GEOMETRY =						0x00000002,

		VERSION_MASK =							0xFFFF0000,		// upper 16bits is version number.
		VERSION_SHIFT =						16,
	};

	enum GeometryFlagsEnum
	{
		GEOMETRY_FLAG_HIDDEN =				0x00000001,		// Geometry Flags bitfield
		GEOMETRY_FLAG_TWO_SIDED =			0x00000002,
		GEOMETRY_FLAG_CAST_SHADOW =		0x00000004,
		GEOMETRY_FLAG_VERTEX_ALPHA =		0x00000008,
		GEOMETRY_FLAG_ZNORMALS =			0x00000010,
		GEOMETRY_FLAG_SHATTERABLE =		0x00000020,
		GEOMETRY_FLAG_NPATCHABLE =			0x00000040,
	};

	enum CollisionFlagsEnum
	{
		COLLISION_FLAG_PHYSICAL =			0x00000001,
		COLLISION_FLAG_PROJECTILE =		0x00000002,
		COLLISION_FLAG_VIS =					0x00000004,
		COLLISION_FLAG_CAMERA =				0x00000008,
		COLLISION_FLAG_VEHICLE =			0x00000010,
	};

	unsigned int ExportFlags;
	unsigned int GeometryType;
	unsigned int GeometryFlags;
	unsigned int CollisionFlags;

	// future expansion, initialized to zeros
	unsigned int UnUsed[4];
};



/*
** W3D Utility Dazzle App Data
** ----------------------------------------------------
** This app-data struct is used to contain parameters
** specific to dazzle render objects.  It currently only
** contains a type name which is limited to 128 characters
** and some padding variables for future use.
*/

struct W3DDazzleAppDataStruct
{
	/*
	** Constructor, zeros everything, then initializes DazzleType to "DEFAULT"
	*/
	W3DDazzleAppDataStruct(void);

	/*
	** Get the W3DAppData2Struct for a given INode and create one if
	** there isn't already one.
	*/
	static W3DDazzleAppDataStruct * Get_App_Data(INode * node,bool create_if_missing = true);

	/*
	** Members
	*/
	unsigned int	UnUsed[4];
	char				DazzleType[128];
};


#endif

