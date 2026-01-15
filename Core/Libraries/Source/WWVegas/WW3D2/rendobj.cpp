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

/* $Header: /Commando/Code/ww3d2/rendobj.cpp 16    12/17/01 8:06p Byon_g $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Engine                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/rendobj.cpp                            $*
 *                                                                                             *
 *                   Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 07/01/02 12:45p                                              $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 * 07/01/02 KM Coltype enum change to avoid MAX conflicts									   *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RenderObjClass::RenderObjClass -- constructor                                             *
 *   RenderObjClass::RenderObjClass -- copy constructor                                        *
 *   RenderObjClass::operator == -- assignment operator                                        *
 *   RenderObjClass::Calculate_Texture_Reduction_Factor -- calculate texture reduction factor  *
 *   RenderObjClass::Set_Texture_Reduction_Factor -- set texture reduction factor.             *
 *   RenderObjClass::Get_Screen_Size -- get normalized area of object.                         *
 *   RenderObjClass::Get_Scene -- returns the (add_ref'd) scene pointer                        *
 *   RenderObjClass::Set_Container -- sets the container pointer                               *
 *   RenderObjClass::Get_Container -- returns the container pointer                            *
 *   RenderObjClass::Set_Transform -- set the transform for this object                        *
 *   RenderObjClass::Set_Position -- Sets the position of this object                          *
 *   RenderObjClass::Get_Transform -- returns the transform for the object                     *
 *   RenderObjClass::Get_Position -- returns the position of this render object                *
 *   RenderObjClass::Validate_Transform -- If the transform is dirty, this causes it to be re- *
 *   RenderObjClass::Get_Sub_Object -- returns pointer to first sub-obj with given name        *
 *   RenderObjClass::Add_Sub_Object_To_Bone -- add an object to a named bone                   *
 *   RenderObjClass::Prepare_LOD -- prepare object for predictive and texture LOD processing.  *
 *   RenderObjClass::Get_Cost -- get object rendering cost for predictive LOD processing.      *
 *   RenderObjClass::Update_Sub_Object_Bits -- updates our bits according to our sub-objects   *
 *   RenderObjClass::Update_Sub_Object_Transforms -- re-evaluate the transforms my sub-objects *
 *   RenderObjClass::Add -- Generic add for render objects                                     *
 *   RenderObjClass::Remove -- Generic Remove for Render Objects                               *
 *   RenderObjClass::Notify_Added -- notifies the object that it is in a scene                 *
 *   RenderObjClass::Notify_Removed -- notifies an object that it has been removed             *
 *   RenderObjClass::Update_Cached_Bounding_Volumes -- default collision sphere.               *
 *   RenderObjClass::Get_Obj_Space_Bounding_Sphere -- default collision sphere.                *
 *   RenderObjClass::Get_Obj_Space_Bounding_Box -- default collision box.                      *
 *   RenderObjClass::Intersect - Returns true if specified intersection object                 *
 *   RenderObjClass::Intersect_Sphere -- tests for intersection with the bounding sphere       *
 *   RenderObjClass::Intersect_Sphere_Quick -- tests for intersection with the bounding sphere *
 *   RenderObjClass::Build_Dependency_List -- Generates a list of files this obj depends on.   *
 *   RenderObjClass::Build_Texture_List -- Builds a list of texture files this obj depends on. *
 *   RenderObjClass::Add_Dependencies_To_List -- Add dependent files to the list.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "rendobj.h"
#include "assetmgr.h"
#include "win.h"
#include "pot.h"
#include "scene.h"
#include "colmath.h"
#include "coltest.h"
#include "inttest.h"
#include "wwdebug.h"
#include "matinfo.h"
#include "htree.h"
#include "predlod.h"
#include "camera.h"
#include "ww3d.h"
#include "chunkio.h"
#include "persistfactory.h"
#include "saveload.h"
#include "ww3dids.h"
#include "intersec.h"



// Definitions of static members:
const float	RenderObjClass::AT_MIN_LOD = FLT_MAX;
const float	RenderObjClass::AT_MAX_LOD = -1.0f;

// Local inline functions
StringClass
Filename_From_Asset_Name (const char *asset_name)
{
	StringClass filename;
	if (asset_name != nullptr) {

		//
		// Copy the model name into a new filename buffer
		//
		::lstrcpy (filename.Get_Buffer (::lstrlen (asset_name) + 5), asset_name);

		//
		// Do we need to strip off the model's suffix?
		//
		char *suffix = ::strchr (filename.Peek_Buffer(), '.');
		if (suffix != nullptr) {
			suffix[0] = 0;
		}

		//
		// Concat the w3d file extension
		//
		filename += ".w3d";
	}

	return filename;
}

static inline bool Check_Is_Transform_Identity(const Matrix3D& m)
{
	const float zero=0.0f;
	const float one=1.0f;

	unsigned d=
		((unsigned&)m[0][0]^(unsigned&)one) |
		((unsigned&)m[0][1]^(unsigned&)zero) |
		((unsigned&)m[0][2]^(unsigned&)zero) |
		((unsigned&)m[0][3]^(unsigned&)zero) |
		((unsigned&)m[1][0]^(unsigned&)zero) |
		((unsigned&)m[1][1]^(unsigned&)one) |
		((unsigned&)m[1][2]^(unsigned&)zero) |
		((unsigned&)m[1][3]^(unsigned&)zero) |
		((unsigned&)m[2][0]^(unsigned&)zero) |
		((unsigned&)m[2][1]^(unsigned&)zero) |
		((unsigned&)m[2][2]^(unsigned&)one) |
		((unsigned&)m[2][3]^(unsigned&)zero);
	return !d;
}


/***********************************************************************************************
 * RenderObjClass::RenderObjClass -- constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/04/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass::RenderObjClass(void) :
	Bits(DEFAULT_BITS),
	Transform(1),
	NativeScreenSize(WW3D::Get_Default_Native_Screen_Size()),
	Scene(nullptr),
	Container(nullptr),
	User_Data(nullptr),
	RenderHook(nullptr),
	ObjectScale(1.0),
	ObjectColor(0),
	CachedBoundingSphere(Vector3(0,0,0),1.0f),
	CachedBoundingBox(Vector3(0,0,0),Vector3(1,1,1)),
	IsTransformIdentity(false)
{
}

/***********************************************************************************************
 * RenderObjClass::RenderObjClass -- copy constructor                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/97    GTH : Created.                                                                 *
 *   1/28/98    EHC : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass::RenderObjClass(const RenderObjClass & src) :
	Bits(src.Bits),
	Transform(src.Transform),
	NativeScreenSize(src.NativeScreenSize),
	Scene(nullptr),
	Container(nullptr),
	User_Data(nullptr),
	RenderHook(nullptr),
	ObjectScale(1.0),
	ObjectColor(0),
	CachedBoundingSphere(src.CachedBoundingSphere),
	CachedBoundingBox(src.CachedBoundingBox),
	IsTransformIdentity(src.IsTransformIdentity)
{
	// Even though we're copying an object which might be in a scene
	// this copy won't be so I'm clearing the scene pointer, same logic
	// follows for things like the Container pointer.
}


/***********************************************************************************************
 * RenderObjClass -- assignment operator                                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/97    GTH : Created.                                                                 *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass & RenderObjClass::operator = (const RenderObjClass & that)
{
	// don't do anything if we're assigning this to this
	if (this != &that) {
		Set_Hidden(that.Is_Hidden());
		Set_Animation_Hidden(that.Is_Animation_Hidden());
		Set_Force_Visible(that.Is_Force_Visible());
		Set_Collision_Type(that.Get_Collision_Type());
		Set_Native_Screen_Size(that.Get_Native_Screen_Size());
		IsTransformIdentity=false;
	}
	return *this;
}


/***********************************************************************************************
 * RenderObjClass::Calculate_Texture_Reduction_Factor -- calculate texture reduction factor.   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *  04/08/99    NH : Created.                                                                  *
 *=============================================================================================*/
/*
float RenderObjClass::Calculate_Texture_Reduction_Factor(float norm_screensize)
{
	// NOTE: The texture reduction factor represents the number of powers of two that the texture
	// must be reduced by in both dimensions. The reason that such an inherently integral quantity
	// is represented as a float is for the texture reduction algorithms to incorporate hysteresis
	// properly.
	float reduction = sqrt(Get_Native_Screen_Size() / norm_screensize);
	reduction = MAX(1.0f, reduction);

	// We want to calculate the log base 2. Since the standard libraries have no log-base-2
	// function, we use the following: log-base-2(x) = log(x)/log(2) where log is the natural
	// logarithm (which does exist in the stadard libraries).
	// We precalculare 1/log(2) as 1.442695f.
	return  log(reduction) * 1.442695f;
}
*/

/***********************************************************************************************
 * RenderObjClass::Set_Texture_Reduction_Factor -- set texture reduction factor.               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *  04/08/99    NH : Created.                                                                  *
 *=============================================================================================*/
/*
void RenderObjClass::Set_Texture_Reduction_Factor(float trf)
{
	WWASSERT(0);	// Texture reduction system is broken! Don't call!
	MaterialInfoClass *minfo = Get_Material_Info();
	if (minfo) {
		minfo->Set_Texture_Reduction_Factor(trf);
		minfo->Release_Ref();
	} else {
		int num_obj = Get_Num_Sub_Objects();
		RenderObjClass *sub_obj;

		for (int i = 0; i < num_obj; i++) {
			sub_obj = Get_Sub_Object(i);
			if (sub_obj) {
				sub_obj->Set_Texture_Reduction_Factor(trf);
				sub_obj->Release_Ref();
			}
		}
	}
}
*/

/***********************************************************************************************
 * RenderObjClass::Get_Screen_Size -- get normalized area of object.                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/12/99    NH : Created.                                                                  *
 *=============================================================================================*/
float RenderObjClass::Get_Screen_Size(CameraClass &camera)
{
	// Currently this works by projecting the bounding sphere to the screen
	// (as if the object was at the center) - in future this may be made more
	// accurate (perhaps by using the object-space bounding-box)
	Vector3 cam = camera.Get_Position();

	ViewportClass viewport = camera.Get_Viewport();
	Vector2 vpr_min, vpr_max;
	camera.Get_View_Plane(vpr_min, vpr_max);
	float width_factor = viewport.Width() / (vpr_max.X - vpr_min.X);
	float height_factor = viewport.Height() / (vpr_max.Y - vpr_min.Y);

	const SphereClass & sphere = Get_Bounding_Sphere();
	float dist = (sphere.Center - cam).Length();
	float radius = 0.0f;
	if (dist) {
		radius = sphere.Radius / dist;
	}

	// Return area in normalized units.
	return WWMATH_PI * radius * radius * width_factor * height_factor;
}


/***********************************************************************************************
 * RenderObjClass::Get_Scene -- returns the (add_ref'd) scene pointer                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
SceneClass * RenderObjClass::Get_Scene(void)
{
	if (Scene != nullptr) {
		Scene->Add_Ref();
	}
	return Scene;
}


/***********************************************************************************************
 * RenderObjClass::Set_Container -- sets the container pointer                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Set_Container(RenderObjClass * con)
{
	// Either we arent currently in a container or we are clearing our container, otherwise
	// Houston, there is a problem!
	WWASSERT((con == nullptr) || (Container == nullptr));
	Container = con;
}

#ifdef GET_CONTAINER_INLINE
// nothing
#else
/***********************************************************************************************
 * RenderObjClass::Get_Container -- returns the container pointer                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * RenderObjClass::Get_Container(void) const
{
	return Container;
}
#endif

/***********************************************************************************************
 * RenderObjClass::Set_Transform -- set the transform for this object                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Set_Transform(const Matrix3D &m)
{
	Transform = m;
	IsTransformIdentity=Check_Is_Transform_Identity(m);
	Invalidate_Cached_Bounding_Volumes();
}


/***********************************************************************************************
 * RenderObjClass::Set_Position -- Sets the position of this object                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *   07/14/2001 SKB : Add Check_Is_Transform_Identity                                          *
 *=============================================================================================*/
void RenderObjClass::Set_Position(const Vector3 &v)
{
	Transform.Set_Translation(v);
	IsTransformIdentity=Check_Is_Transform_Identity(Transform);
	Invalidate_Cached_Bounding_Volumes();
}


/***********************************************************************************************
 * RenderObjClass::Validate_Transform -- If the transform is dirty, this causes it to be re-ca *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/15/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Validate_Transform(void) const
{
	/*
	** Recurse up the tree to see if any of my parents are saying that their sub-object
	** transforms are dirty
	*/
	RenderObjClass * con = Get_Container();
	bool dirty = false;
	if (con != nullptr)
	{
		dirty = con->Are_Sub_Object_Transforms_Dirty();

		while (con->Get_Container() != nullptr)
		{
			dirty |= con->Are_Sub_Object_Transforms_Dirty();
			con = con->Get_Container();
		}

		/*
		** If the transforms are dirty, update them
		*/
		if (dirty)
		{
			con->Update_Sub_Object_Transforms();
		}
	}
	if (dirty)
		IsTransformIdentity = Check_Is_Transform_Identity(Transform);
}

/***********************************************************************************************
 * RenderObjClass::Get_Position -- returns the position of this render object                  *
 *                                                                                             *
 * Similar to Get_Transform but returns only the position.                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
Vector3 RenderObjClass::Get_Position(void) const
{
	Validate_Transform();
	return Transform.Get_Translation();
}


/***********************************************************************************************
 * RenderObjClass::Get_Sub_Object -- returns pointer to first sub-obj with given name          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * RenderObjClass::Get_Sub_Object_By_Name(const char * name, int *index) const
{
	int i;

	// first try the un-altered name
	for (i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		if (robj) {
			if (stricmp(robj->Get_Name(),name) == 0) {
				if (index) *index=i;
				return robj;
			} else {
				robj->Release_Ref();
			}
		}
	}

	// check the given name against the "suffix" names of each sub-object
	for (i=0; i<Get_Num_Sub_Objects(); i++) {
		RenderObjClass * robj = Get_Sub_Object(i);
		if (robj) {
			const char * subobjname = strchr(robj->Get_Name(),'.');
			if (subobjname == nullptr) {
				subobjname = robj->Get_Name();
			} else {
				// skip past the period.
				subobjname = subobjname+1;
			}

			if (stricmp(subobjname,name) == 0) {
				if (index) *index=i;
				return robj;
			} else {
				robj->Release_Ref();
			}
		}
	}

	return nullptr;
}


/***********************************************************************************************
 * RenderObjClass::Add_Sub_Object_To_Bone -- add an object to a named bone                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:	If the bone name is unknown then this function will add the the object to the   *
 *					root transform rather than failing.  This is due to the fact that GetBoneIndex  *
 *					returns the root tranform for unknown bones.												  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
int RenderObjClass::Add_Sub_Object_To_Bone(RenderObjClass * subobj,const char * bname)
{
	int bindex = Get_Bone_Index(bname);
	return Add_Sub_Object_To_Bone(subobj,bindex);
}


/***********************************************************************************************
 * RenderObjClass::Remove_Sub_Objects_From_Bone -- remove all objects from a named bone        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/02     NH : Created.                                                                  *
 *=============================================================================================*/
int RenderObjClass::Remove_Sub_Objects_From_Bone(int boneindex)
{
	int count = Get_Num_Sub_Objects_On_Bone(boneindex);
	int remove_count = 0;
	for (int i = count-1; i >= 0; i--) {
		RenderObjClass *robj = Get_Sub_Object_On_Bone(i, boneindex);
		if ( robj ) {
			remove_count += Remove_Sub_Object(robj);
			robj->Release_Ref();
		}
	}
	return remove_count;
}


/***********************************************************************************************
 * RenderObjClass::Remove_Sub_Objects_From_Bone -- remove all objects from a named bone        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/99     NH : Created.                                                                  *
 *=============================================================================================*/
int RenderObjClass::Remove_Sub_Objects_From_Bone(const char * bname)
{
	return Remove_Sub_Objects_From_Bone(Get_Bone_Index(bname));
}


/***********************************************************************************************
 * RenderObjClass::Prepare_LOD -- prepare object for predictive and texture LOD processing.    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/11/99    NH : Created.                                                                  *
 *=============================================================================================*/
void RenderObjClass::Prepare_LOD(CameraClass &camera)
{
	// Since most RenderObjClass derivatives are not LOD-capable, the default
	// implementation just sets the texture reduction factor and doesn't do any
	// predictive LOD preparation (except for adding the objects' cost to the
	// total static (nonoptimizeable) cost).

	// Find the maximum screen dimension of the object in pixels
//	float norm_area = Get_Screen_Size(camera);

	// Find and set texture reduction factor
	// Jani: Don't set tex reduction, it's broken!
//   Set_Texture_Reduction_Factor(Calculate_Texture_Reduction_Factor(norm_area));

	// Since we are not adding this object to the predictive LOD optimizer,
	// at least add its cost in.
	PredictiveLODOptimizerClass::Add_Cost(Get_Cost());
}


/***********************************************************************************************
 * RenderObjClass::Get_Cost -- get object rendering cost for predictive LOD processing.        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/11/99    NH : Created.                                                                  *
 *=============================================================================================*/
float RenderObjClass::Get_Cost(void) const
{
	int polycount = Get_Num_Polys();
	// If polycount is zero set Cost to a small nonzero amount to avoid divisions by zero.
	float cost = (polycount != 0)? polycount : 0.000001f;
	return cost;
}


/***********************************************************************************************
 * RenderObjClass::Calculate_Cost_Value_Arrays -- calc arrays for predictive LOD processing.   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNING:	This member function assumes that the arrays passed are large enough (# LODs for   *
 *				the cost array and # LODs + 1 for the value array) to contain the desired data.    *
 *                                                                                             *
 * NOTE:	This member function is usually only called internally inside LOD objects for         *
 *			initialization purposes, or externally by specialized LOD objects like the G          *
 *			predictive LOD PIP proxy.                                                             *
 *                                                                                             *
 * NOTE:	The screen area used is in normalized units.                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/11/99    NH : Created.                                                                  *
 *=============================================================================================*/
int RenderObjClass::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
	values[0] = AT_MIN_LOD;
	values[1] = AT_MAX_LOD;
	costs[0] = Get_Cost();

	return 0;
}


/***********************************************************************************************
 * RenderObjClass::Update_Sub_Object_Bits -- updates our bits according to our sub-objects     *
 *                                                                                             *
 * This should be called by any object that contains other objects whenever a sub object is    *
 * added or removed.  It updates the status of the attribute bits which are supposed to be     *
 * the union of all of the sub-objects attributes.  (I.e. if one of our sub-objects is         *
 * translucent, then we should be marked as translucent).                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Update_Sub_Object_Bits(void)
{
	// this doesn't do anything for non-composite objects
	if (Get_Num_Sub_Objects() == 0) return;

	// go through all of our sub-objects
	int coltype = 0;
	int istrans = 0;
	int isalpha = 0;
	int isadditive = 0;

	for (int ni = 0; ni < Get_Num_Sub_Objects(); ni++) {
		RenderObjClass * robj = Get_Sub_Object(ni);
		coltype |= robj->Get_Collision_Type();
		istrans |= robj->Is_Translucent();
		isalpha |= robj->Is_Alpha();
		isadditive |= robj->Is_Additive();
		robj->Release_Ref();
	}

	Set_Collision_Type(coltype);
	Set_Translucent(istrans);
	Set_Alpha(isalpha);
	Set_Additive(isadditive);

	// if we are a sub-object, tell our container to do this
	if (Container) {
		Container->Update_Sub_Object_Bits();
	}
}


/***********************************************************************************************
 * RenderObjClass::Update_Sub_Object_Transforms -- re-evaluate the transforms my sub-objects   *
 *                                                                                             *
 * The default implementation is empty, derived classes which have sub-objects should          *
 * implement it to update the transforms of their sub-objects                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Update_Sub_Object_Transforms(void)
{
}


/***********************************************************************************************
 * RenderObjClass::Add -- Generic add for render objects                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/04/1997 GH  : Created.                                                                 *
 *   2/25/99    GTH : Moved to the base RenderObjClass                                         *
 *=============================================================================================*/
void RenderObjClass::Add(SceneClass * scene)
{
	WWASSERT(scene);
	WWASSERT(Container == nullptr);
	Scene = scene;
	Scene->Add_Render_Object(this);
}

/***********************************************************************************************
 * RenderObjClass::Remove -- Generic Remove for Render Objects                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/04/1997 GH  : Created.                                                                 *
 *   2/25/99    GTH : moved to the base RenderObjClass                                         *
 *=============================================================================================*/
bool RenderObjClass::Remove(void)
{
	// All render objects have their scene pointers set.  To check if this is a "top level"
	// object, (i.e. directly in the scene) you see if its Container pointer is null.
#if 1
	if (Container == nullptr) {
		if (Scene != nullptr) {
			Scene->Remove_Render_Object(this);
			return true;
		}
		return false;
	} else {
		Container->Remove_Sub_Object(this);
		return true;
	}
#else
	if (!Scene)
		return false;
	Scene->Remove_Render_Object(this);
	Scene = nullptr;
	return true;
#endif
}


/***********************************************************************************************
 * RenderObjClass::Notify_Added -- notifies the object that it is in a scene                   *
 *                                                                                             *
 * This function will be called whenever an object is directly or indirectly added to a scene  *
 * An example of "indirect" addition would be if you were added as a sub-object to an HModel   *
 * that was already in a scene.                                                                *
 *                                                                                             *
 * Override this function if you want to register your object for per-frame-updating or        *
 * as a VertexProcessor.  (See the Register method of SceneClass, ParticleBufferClass, etc)    *
 *                                                                                             *
 * Container objects must forward this notification to their sub objects.                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Due to implementation details of some derived scenes, the SceneClass calls this function.   *
 * Please dont move the call to this to RenderObjClass::Add                                    *
 * Derived classes should also call the base class to ensure that the scene pointer is set     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Notify_Added(SceneClass * scene)
{
	Scene = scene;
}


/***********************************************************************************************
 * RenderObjClass::Notify_Removed -- notifies an object that it has been removed               *
 *                                                                                             *
 * Works similar to the Notify_Added function.  You can override and Unregister yourself from  *
 * any scene based special processing.  Container objects must recurse to their sub objects.   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Derived classes should also call the base class to ensure that the scene pointer is cleared *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Notify_Removed(SceneClass * scene)
{
	Scene = nullptr;
}


/***********************************************************************************************
 * RenderObjClass::Update_Cached_Bounding_Volumes -- default collision sphere.                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/7/97    GTH : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Update_Cached_Bounding_Volumes(void) const
{
	Get_Obj_Space_Bounding_Box(CachedBoundingBox);
	Get_Obj_Space_Bounding_Sphere(CachedBoundingSphere);

#ifdef ALLOW_TEMPORARIES
	CachedBoundingSphere.Center = Get_Transform() * CachedBoundingSphere.Center;
#else
	Get_Transform().mulVector3(CachedBoundingSphere.Center);
#endif
 	CachedBoundingSphere.Radius = ObjectScale * CachedBoundingSphere.Radius;
	CachedBoundingBox.Transform(Get_Transform());

	Validate_Cached_Bounding_Volumes();
}


/***********************************************************************************************
 * RenderObjClass::Get_Obj_Space_Bounding_Sphere -- default collision sphere.                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   28/8/97    NH : Created.                                                                  *
 *   2/25/99    GTH : Moved into RenderObjClass                                                *
 *=============================================================================================*/
void RenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Center.Set(0,0,0);
	sphere.Radius = 1.0f;
}


/***********************************************************************************************
 * RenderObjClass::Get_Obj_Space_Bounding_Box -- default collision box.                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   28/8/97    NH : Created.                                                                  *
 *   2/25/99    GTH : Moved into RenderObjClass                                                *
 *=============================================================================================*/
void RenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Center.Set(0,0,0);
	box.Extent.Set(0,0,0);
}

/***********************************************************************************************
 *  RenderObjClass::Intersect - Returns true if specified intersection object                  *
 *                                intersects this renderobject                                 *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT: A properly configured Intersection object specifying ray direction & vector          *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Moved into RenderObjClass                                                *
 *=============================================================================================*/
bool RenderObjClass::Intersect(IntersectionClass *Intersection, IntersectionResultClass *Final_Result)
{

	// do the quick sphere test just to make sure it is worth the more expensive intersection test
	if (Intersect_Sphere_Quick(Intersection, Final_Result)) {

		CastResultStruct castresult;
		LineSegClass lineseg;

		Vector3 end = *Intersection->RayLocation + *Intersection->RayDirection * Intersection->MaxDistance;
		lineseg.Set(* Intersection->RayLocation, end);

		RayCollisionTestClass ray(lineseg, &castresult);
		ray.CollisionType = COLL_TYPE_ALL;

		if (Cast_Ray(ray)) {
			lineseg.Compute_Point(ray.Result->Fraction,&(Final_Result->Intersection));
			Final_Result->Intersects = true;
			Final_Result->IntersectionType = IntersectionResultClass::GENERIC;
			if (Intersection->IntersectionNormal)
				* Intersection->IntersectionNormal = castresult.Normal;
			Final_Result->IntersectedRenderObject = this;
			Final_Result->ModelMatrix = Transform;
			return true;
		}
	}
	Final_Result->Intersects = false;
	return false;
}


/***********************************************************************************************
 * RenderObjClass::Intersect_Sphere -- tests for intersection with the bounding sphere         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool RenderObjClass::Intersect_Sphere(IntersectionClass *Intersection, IntersectionResultClass *Final_Result)
{
	SphereClass sphere = Get_Bounding_Sphere();
	return Intersection->Intersect_Sphere(sphere, Final_Result);
}


/***********************************************************************************************
 * RenderObjClass::Intersect_Sphere_Quick -- tests for intersection with the bounding sphere   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
bool RenderObjClass::Intersect_Sphere_Quick(IntersectionClass *Intersection, IntersectionResultClass *Final_Result)
{
	SphereClass sphere = Get_Bounding_Sphere();
	return Intersection->Intersect_Sphere_Quick(sphere, Final_Result);
}

/***********************************************************************************************
 * RenderObjClass::Build_Dependency_List -- Generates a list of files this obj depends on.     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/18/99    PDS : Created.                                                                 *
 *=============================================================================================*/
bool RenderObjClass::Build_Dependency_List (DynamicVectorClass<StringClass> &file_list, bool recursive)
{
	if (recursive)
	{
		// Loop through all this object's subobj's
		int subobj_count = Get_Num_Sub_Objects ();
		for (int index = 0; index < subobj_count; index ++) {

			// Ask this subobj to add all of its file dependencies to the list
			RenderObjClass *psub_obj = Get_Sub_Object (index);
			if (psub_obj != nullptr) {
				psub_obj->Build_Dependency_List (file_list);
				psub_obj->Release_Ref ();
			}
		}
	}

	// Now add all of this object's dependencies to the list
	Add_Dependencies_To_List (file_list);

	// Return the true/false result code
	return (file_list.Count () > 0);
}


/***********************************************************************************************
 * RenderObjClass::Build_Texture_List -- Builds a list of texture files this obj depends on.   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/18/99    PDS : Created.                                                                 *
 *=============================================================================================*/
bool RenderObjClass::Build_Texture_List
(
	DynamicVectorClass<StringClass> &	texture_file_list,
	bool											recursive
)
{
	if (recursive) {

		//
		// Loop through all this object's subobj's
		//
		int subobj_count = Get_Num_Sub_Objects ();
		for (int index = 0; index < subobj_count; index ++) {

			//
			// Ask this subobj to add all of its texture file dependencies to the list
			//
			RenderObjClass *sub_obj = Get_Sub_Object (index);
			if (sub_obj != nullptr) {
				sub_obj->Build_Texture_List (texture_file_list);
				sub_obj->Release_Ref ();
			}
		}
	}

	//
	// Now add all of this object's texture dependencies to the list
	//
	Add_Dependencies_To_List (texture_file_list, true);

	// Return the true/false result code
	return (texture_file_list.Count () > 0);
}

/***********************************************************************************************
 * RenderObjClass::Add_Dependencies_To_List -- Add dependent files to the list.                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/18/99    PDS : Created.                                                                 *
 *=============================================================================================*/
void RenderObjClass::Add_Dependencies_To_List
(
	DynamicVectorClass<StringClass> &file_list,
	bool										textures_only
)
{
	//
	// Should we add W3D files to the list?
	//
	if (textures_only == false) {

		//
		// Main W3D file
		//
		const char *model_name = Get_Name ();
		file_list.Add (::Filename_From_Asset_Name (model_name));

		//
		// External hierarchy file
		//
		const HTreeClass *phtree = Get_HTree ();
		if (phtree != nullptr) {
			const char *htree_name = phtree->Get_Name ();
			if (::lstrcmpi (htree_name, model_name) != 0) {

				//
				// Add this file to the list
				//
				file_list.Add (::Filename_From_Asset_Name (htree_name));
			}
		}

		//
		// Original W3D file (if an aggregate)
		//
		const char *base_model_name = Get_Base_Model_Name ();
		if (base_model_name != nullptr) {

			//
			// Add this file to the list
			//
			file_list.Add (::Filename_From_Asset_Name (base_model_name));
		}
	}

	return;
}



/****************************************************************************************


	RenderObjClass - Persistant object support.

	NOTE: For now, the render obj PersistFactory is going to cheat by simply storing
	the name of the render object that was saved.  At load time, it will ask the
	asset manager for that object again.  If the asset manager fails to re-create the
	object,


****************************************************************************************/

class RenderObjPersistFactoryClass : public PersistFactoryClass
{
	virtual uint32				Chunk_ID(void) const;
	virtual PersistClass *	Load(ChunkLoadClass & cload) const;
	virtual void				Save(ChunkSaveClass & csave,PersistClass * obj)	const;

	enum
	{
		RENDOBJFACTORY_CHUNKID_VARIABLES		= 0x00555040,
		RENDOBJFACTORY_VARIABLE_OBJPOINTER	= 0x00,
		RENDOBJFACTORY_VARIABLE_NAME,
		RENDOBJFACTORY_VARIABLE_TRANSFORM
	};
};

static RenderObjPersistFactoryClass _RenderObjPersistFactory;

uint32 RenderObjPersistFactoryClass::Chunk_ID(void) const
{
	return WW3D_PERSIST_CHUNKID_RENDEROBJ;
}

PersistClass *	RenderObjPersistFactoryClass::Load(ChunkLoadClass & cload) const
{
	RenderObjClass * old_obj = nullptr;
	Matrix3D tm(1);
	char name[64];
	name[0] = '\0';

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {

			case RENDOBJFACTORY_CHUNKID_VARIABLES:

				while (cload.Open_Micro_Chunk()) {
					switch(cload.Cur_Micro_Chunk_ID()) {
						READ_MICRO_CHUNK(cload,RENDOBJFACTORY_VARIABLE_OBJPOINTER,old_obj);
						READ_MICRO_CHUNK(cload,RENDOBJFACTORY_VARIABLE_TRANSFORM,tm);
						READ_MICRO_CHUNK_STRING(cload,RENDOBJFACTORY_VARIABLE_NAME,name,sizeof(name));
					}
					cload.Close_Micro_Chunk();
				}
				break;

			default:
				WWDEBUG_SAY(("Unhandled Chunk: 0x%X File: %s Line: %d",__FILE__,__LINE__));
				break;
		};
		cload.Close_Chunk();
	}

	// if the object we saved didn't have a name, replace it with null
	if (strlen(name) == 0) {
		static int count = 0;
		if ( count++ < 10 ) {
			WWDEBUG_SAY(("RenderObjPersistFactory attempted to load an un-named render object!"));
			WWDEBUG_SAY(("Replacing it with a null render object!"));
		}
		strcpy(name,"NULL");
	}

	RenderObjClass * new_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(name);

	if (new_obj == nullptr) {
		static int count = 0;
		if ( count++ < 10 ) {
			WWDEBUG_SAY(("RenderObjPersistFactory failed to create object: %s!!",name));
			WWDEBUG_SAY(("Either the asset for this object is gone or you tried to save a procedural object."));
			WWDEBUG_SAY(("Replacing it with a null render object!"));
		}
		strcpy(name,"NULL");
		new_obj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(name);
	}

	WWASSERT(new_obj != nullptr);
	if (new_obj) {
		new_obj->Set_Transform(tm);
	}

	SaveLoadSystemClass::Register_Pointer(old_obj,new_obj);
	return new_obj;
}

void RenderObjPersistFactoryClass::Save(ChunkSaveClass & csave,PersistClass * obj)	const
{
	RenderObjClass * robj = (RenderObjClass *)obj;
	const char * name = robj->Get_Name();
	Matrix3D tm = robj->Get_Transform();

	csave.Begin_Chunk(RENDOBJFACTORY_CHUNKID_VARIABLES);
	WRITE_MICRO_CHUNK(csave,RENDOBJFACTORY_VARIABLE_OBJPOINTER,robj);
	WRITE_MICRO_CHUNK_STRING(csave,RENDOBJFACTORY_VARIABLE_NAME,name);
	WRITE_MICRO_CHUNK(csave,RENDOBJFACTORY_VARIABLE_TRANSFORM,tm);
	csave.End_Chunk();
}


/*
** RenderObj save-load.
*/
const PersistFactoryClass & RenderObjClass::Get_Factory (void) const
{
	return _RenderObjPersistFactory;
}

bool RenderObjClass::Save (ChunkSaveClass &csave)
{
	// This should never hit with the persist factory we're using...
	// Yes this looks like a design flaw but the way we're saving render objects is
	// a "shortcut".  We specifically designed this capability into the persistant
	// object system so that we could avoid making all render object's save and
	// load themselves if possible.
	WWASSERT(0);
	return true;
}

bool RenderObjClass::Load (ChunkLoadClass &cload)
{
	WWASSERT(0); // this should never hit with the persist factory we're using.
	return true;
}


