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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/rendobj.h                              $*
 *                                                                                             *
 *                   Org Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 9:23a                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 * 06/27/02 KM Shader system classid addition                                       *
 * 07/01/02 KM Coltype enum change to avoid MAX conflicts									   *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "sphere.h"
#include "coltype.h"
#include "aabox.h"
#include "persist.h"
#include "multilist.h"
#include "robjlist.h"
#include <float.h>

class	Vector3;
class Matrix3D;
class MaterialInfoClass;
class TextureClass;
class SceneClass;
class HTreeClass;
class HAnimClass;
class HAnimComboClass;
class HCompressedAnimClass;
class RayCollisionTestClass;
class AABoxCollisionTestClass;
class OBBoxCollisionTestClass;
class AABoxIntersectionTestClass;
class OBBoxIntersectionTestClass;
class CameraClass;
class SphereClass;
class AABoxClass;
class RenderInfoClass;
class SpecialRenderInfoClass;
class	IntersectionClass;
class	IntersectionResultClass;
class DecalGeneratorClass;
class RenderObjProxyClass;
class StringClass;
template<class T> class DynamicVectorClass;

// "unreferenced formal parameter"
#pragma warning(disable : 4100)

//////////////////////////////////////////////////////////////////////////////////
// RenderObjClass
// This is the interface for all objects that get rendered by WW3D.
//
// Render object RTTI:  If you really need to typecast a render object
//		pointer that you got from the asset manager, the class id mechanism
//		can be used to check what you really have.  User class id's can come
//		after CLASSID_LAST.
//
// Cloning: All RenderObj's need to be able to clone themselves.  This function
//		should create a new and separate RenderObj of the correct type and return a
//		RenderObj pointer to it.  The implementation of this function will be
//		to simply call your copy constructor; its basically a virtual copy constructor.
//
// Rendering: If the render object is in a scene that is rendered and is determined
//		to be visible by that scene, it will receive a Render call.  The argument
//		to the call will contain both the camera being used and the low level rendering
//		interface.  In addition, the Special_Render function is for all "non-normal"
//		types of rendering.  Some examples of this are: G-Buffer rendering (rendering
//		object ID's), shadow rendering (just use black, etc) and whatever else we
//		come up with.  Basically it will be a function with a big switch statement
//		to handle all of these extra operations.  This means the main render code
//		path is not cluttered with these checks while not forcing every object to
//		implement millions of separate special render functions.  (Many objects just
//		pass the render calls onto their sub-objects).
//
//	VertexProcessors: Vertex processors are classes that are not actually 'rendered'
//		They insert into the system an object that performs operations on all of
//		the subsequent vertices that are processed.  Lights and Fogs are types of
//		vertex processors.
//
// "Scene Graph": A scene is organized as a list of render objects.  There is no
//		implied hierarchical structure to a scene.  RenderObjects can contain other
//		render objects (they follow the 'Composite' pattern) which is how hierarchical
//		objects are built.  Hierarchical models are render objects that just
//		contain other render objects and apply hierarchical transforms to them.
//		Hierarchical Models can be inserted inside of other hierarchical models.
//
//	Predictive LOD: The predictive LOD system selects LODs for the visible objects
//		so that the various resources (polys, vertices, etc.) do not pass given
//		budgets - the goal is to achieve a constant frame rate. This interface
//		includes things that are needed for this optimization process. Objects which
//		do not support changing their LOD should report that they have 1 LOD and
//		should report their cost to the LOD optimization system.
//
//	Dependency Generation:  Render objects are composed of one or more W3D and
//		texture files.  This set of interfaces provides access to that dependency list.
//
//////////////////////////////////////////////////////////////////////////////////

// This is a small abstract class which a render object may have a pointer to. If present, its
// Pre_Render and Post_Render calls will be called before and after the Render() call of this
// render object. Applications using WW3D may create concrete classes deriving from this for
// application-specific pre- and post- render processing. (the return value from Pre_Render
// determines whether to perform the Render() call - if false, Render() will not be called).
class RenderHookClass
{
public:
	RenderHookClass(void) { }
	virtual ~RenderHookClass(void) { }
	virtual bool Pre_Render(RenderObjClass *robj, RenderInfoClass &rinfo) = 0;
	virtual void Post_Render(RenderObjClass *robj, RenderInfoClass &rinfo) = 0;
private:
	// Enforce no-copy semantics, for this and derived classes. This is done by making these
	// private and not having definitions for them.
	RenderHookClass(const RenderHookClass & src);
	RenderHookClass & operator=(const RenderHookClass & src);
};

// RenderObjClass definition
class RenderObjClass : public RefCountClass , public PersistClass, public MultiListObjectClass
{
public:

 	//Integer flag placed at the start of structure pointed to by
 	//User_Data to signal that it points at custom mesh material settings.
	//Added for 'Generals' - MW
 	enum	{USER_DATA_MATERIAL_OVERRIDE = 0x01234567};

 	//This strucutre is used to pass custom rendering parameters into the W3D
 	//mesh renderer so it can override settings which are usually shared across
 	//all instances of a model - typically material settings like alpha, texture
 	//animation, texture uv scrolling, etc.  Added for 'Generals' -MW
 	struct Material_Override
 	{	Material_Override(void)	: Struct_ID(USER_DATA_MATERIAL_OVERRIDE),customUVOffset(0,0) {}

 		int Struct_ID;	//ID used to identify this structure from a pointer to it.
 		Vector2 customUVOffset;
 	};

	//
	//	Note:  It is very important that these values NEVER CHANGE.  That means
	//	when adding a new class id, it should be added to the end of the enum.
	//
	enum
	{
		CLASSID_UNKNOWN	= 0xFFFFFFFF,
		CLASSID_MESH		= 0,
		CLASSID_HMODEL,
		CLASSID_DISTLOD,
		CLASSID_PREDLODGROUP,
		CLASSID_TILEMAP,
		CLASSID_IMAGE3D,	// Obsolete
		CLASSID_LINE3D,
		CLASSID_BITMAP2D,	// Obsolete
		CLASSID_CAMERA,
		CLASSID_DYNAMESH,
		CLASSID_DYNASCREENMESH,
		CLASSID_TEXTDRAW,
		CLASSID_FOG,
		CLASSID_LAYERFOG,
		CLASSID_LIGHT,
		CLASSID_PARTICLEEMITTER,
		CLASSID_PARTICLEBUFFER,
		CLASSID_SCREENPOINTGROUP,
		CLASSID_VIEWPOINTGROUP,
		CLASSID_WORLDPOINTGROUP,
		CLASSID_TEXT2D,
		CLASSID_TEXT3D,
		CLASSID_NULL,
		CLASSID_COLLECTION,
		CLASSID_FLARE,
		CLASSID_HLOD,
		CLASSID_AABOX,
		CLASSID_OBBOX,
		CLASSID_SEGLINE,
		CLASSID_SPHERE,
		CLASSID_RING,
		CLASSID_BOUNDFOG,
		CLASSID_DAZZLE,
		CLASSID_SOUND,
		CLASSID_SEGLINETRAIL,
		CLASSID_LAND,
		CLASSID_SHDMESH,				// mesh class that uses the scaleable shader system
		CLASSID_LAST		= 0x0000FFFF
	};

	RenderObjClass(void);
	RenderObjClass(const RenderObjClass & src);
	RenderObjClass & operator = (const RenderObjClass &);
	virtual ~RenderObjClass(void)																					{ if (RenderHook) delete RenderHook; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Cloning and Identification
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const																= 0;
	virtual int						Class_ID(void)	const															{ return CLASSID_UNKNOWN; }
	virtual const char *			Get_Name(void) const															{ return "UNNAMED"; }
	virtual void					Set_Name(const char * name)												{ }
	virtual const char *			Get_Base_Model_Name (void) const											{ return NULL; }
	virtual void					Set_Base_Model_Name (const char *name)									{ }
	virtual int						Get_Num_Polys(void) const													{ return 0; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	//
	// Render - this object should render its polygons.  Typically called from a SceneClass
	// Special_Render - all special-case rendering goes here to avoid polluting the main render pipe (e.g. VIS)
	// On_Frame_Update - render objects can register for an On_Frame_Update call; the scene will call this once
	//                   per frame if they do so.
	// Restart - This interface is used to facilitate model recycling.  If a render object is "Restarted" it should
	//           put itself back into a state as if it has never been rendered (e.g. particle emitters
	//           should reset their "emitted particle counts" so they can be re-used.)
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo)											= 0;
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo)						{ }
	virtual void					On_Frame_Update() 														{ }
	virtual void					Restart(void)																	{ }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	// Some of the functions in this group are non-virtual as they are meant
	// to be never overriden or are supposed to be implemented in terms of
	// the other virtual functions.  We want to keep the virtual interface
	// as small as possible
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void					Add(SceneClass * scene);
	virtual bool					Remove(void);
	virtual SceneClass *			Get_Scene(void);
	virtual SceneClass *			Peek_Scene(void)																{ return Scene; }
	virtual void					Set_Container(RenderObjClass * con);
	virtual void					Validate_Transform(void) const;

#define GET_CONTAINER_INLINE
#ifdef GET_CONTAINER_INLINE
	// srj sez: this is called a ton and never overridden, so inline it
	inline RenderObjClass *	Get_Container(void) const { return Container; }
#else
	virtual RenderObjClass *	Get_Container(void) const;
#endif

	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);
	const Matrix3D &				Get_Transform(void) const;
	const Matrix3D &				Get_Transform(bool& is_transform_identity) const;
	const Matrix3D &				Get_Transform_No_Validity_Check(void) const;
	const Matrix3D &				Get_Transform_No_Validity_Check(bool& is_transform_identity) const;
	bool								Is_Transform_Identity() const;
	bool								Is_Transform_Identity_No_Validity_Check() const;
	Vector3							Get_Position(void) const;

	virtual void					Notify_Added(SceneClass * scene);
	virtual void					Notify_Removed(SceneClass * scene);

	virtual int						Get_Num_Sub_Objects(void) const											{ return 0; }
	virtual RenderObjClass *	Get_Sub_Object(int index) const											{ return NULL; }
	virtual int						Add_Sub_Object(RenderObjClass * subobj)								{ return 0; }
	virtual int						Remove_Sub_Object(RenderObjClass * robj)								{ return 0; }
	virtual RenderObjClass *	Get_Sub_Object_By_Name(const char * name, int *index=NULL) const;

	virtual int						Get_Num_Sub_Objects_On_Bone(int boneindex) const					{ return 0; }
	virtual RenderObjClass *	Get_Sub_Object_On_Bone(int index,int boneindex)	const				{ return NULL; }
	virtual int						Get_Sub_Object_Bone_Index(RenderObjClass * subobj)	const 		{ return 0; }
	virtual int						Get_Sub_Object_Bone_Index(int LodIndex, int ModelIndex)	const 		{ return 0; }
	virtual int						Add_Sub_Object_To_Bone(RenderObjClass * subobj,int bone_index)	{ return 0; }
	virtual int						Add_Sub_Object_To_Bone(RenderObjClass * subobj,const char * bname);
	virtual int						Remove_Sub_Objects_From_Bone(int boneindex);
	virtual int						Remove_Sub_Objects_From_Bone(const char * bname);

	// This is public only so objects can recursively call this on their sub-objects
	virtual void					Update_Sub_Object_Transforms(void);


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Hierarchical Animation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	enum AnimMode
	{
		ANIM_MODE_MANUAL		= 0,
		ANIM_MODE_LOOP,
		ANIM_MODE_ONCE,
		ANIM_MODE_LOOP_PINGPONG,
		ANIM_MODE_LOOP_BACKWARDS,	//make sure only backwards playing animations after this one
		ANIM_MODE_ONCE_BACKWARDS,

		ANIM_MODE_COUNT
	};

	virtual void					Set_Animation( void )														{ }
	virtual void					Set_Animation( HAnimClass * motion,
															float frame, int anim_mode = ANIM_MODE_MANUAL)	{ }
	virtual void					Set_Animation( HAnimClass * motion0,
															float frame0,
															HAnimClass * motion1,
															float frame1,
															float percentage)											{ }
	virtual void					Set_Animation( HAnimComboClass * anim_combo)							{ }

	virtual HAnimClass *			Peek_Animation( void )														{ return NULL; }
	virtual int						Get_Num_Bones(void)															{ return 0; }
	virtual const char *			Get_Bone_Name(int bone_index)												{ return NULL; }
	virtual int						Get_Bone_Index(const char * bonename)									{ return 0; }
	virtual const Matrix3D &	Get_Bone_Transform(const char * bonename)    						{ return Get_Transform(); }
	virtual const Matrix3D &	Get_Bone_Transform(int boneindex)      								{ return Get_Transform(); }
	virtual void					Capture_Bone(int bindex)													{ }


	virtual void					Release_Bone(int bindex)													{ }
	virtual bool					Is_Bone_Captured(int bindex) const										{ return false; }
	virtual void					Control_Bone(int bindex,const Matrix3D & objtm,bool world_space_translation = false)						{ }
	virtual const HTreeClass *	Get_HTree(void) const														{ return NULL; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection
	// Cast_Ray - intersects a ray with the render object
	// Cast_AABox - intersects a swept AABox with the render object
	// Cast_OBBox - intersects a swept OBBox with the render object
	// Intersect_AABox - boolean test for intersection between an AABox and the renderobj
	// Intersect_OBBox - boolean test for intersection between an OBBox and the renderobj
	// Intersect - tests a ray for intersection with the render object
	// Intersect_Sphere - tests a ray for intersection with the bounding spheres
	// Intersect_Sphere_Quick - tests a ray for intersection with bounding spheres
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest)								{ return false; }
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest)						{ return false; }
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest)						{ return false; }

	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest)				{ return false; }
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)				{ return false; }

	virtual bool					Intersect(IntersectionClass *Intersection, IntersectionResultClass *Final_Result);
	virtual bool					Intersect_Sphere(IntersectionClass *Intersection, IntersectionResultClass *Final_Result);
	virtual bool					Intersect_Sphere_Quick(IntersectionClass *Intersection, IntersectionResultClass *Final_Result);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual const SphereClass & Get_Bounding_Sphere(void) const;
	virtual const AABoxClass &	Get_Bounding_Box(void) const;
	virtual void		 			Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
	virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;
   virtual void               Update_Obj_Space_Bounding_Volumes(void)								{ };


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Predictive LOD
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Two constants for Value queries, which are returned instead of the
	// current Value in certain cases. They are usually used as sentinels.
	// AT_MIN_LOD is a very large positive number, AT_MAX_LOD is negative.
	static const float	AT_MIN_LOD;
	static const float	AT_MAX_LOD;

	virtual void	Prepare_LOD(CameraClass &camera);
   virtual void   Recalculate_Static_LOD_Factors(void)													{ }
	virtual void	Increment_LOD(void)																			{ }
	virtual void	Decrement_LOD(void)																			{ }
	virtual float	Get_Cost(void) const;
	virtual float	Get_Value(void) const																		{ return AT_MIN_LOD; }
	virtual float	Get_Post_Increment_Value(void) const													{ return AT_MAX_LOD; }
	virtual void	Set_LOD_Level(int lod)																		{ }
	virtual int		Get_LOD_Level(void) const																	{ return 0; }
	virtual int		Get_LOD_Count(void) const																	{ return 1; }
	virtual void	Set_LOD_Bias(float bias)																	{ }
	virtual int	Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const;
	virtual RenderObjClass *	Get_Current_LOD(void)														{ Add_Ref(); return this; }


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Dependency Generation
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//
	//	Note:  The strings contained in these lists need to be freed by the caller.
	// They should be freed using the delete operator.
	//
	//	Be aware, these lists WILL contain duplicate entries.
	//
	virtual bool					Build_Dependency_List (DynamicVectorClass<StringClass> &file_list, bool recursive=true);
	virtual bool					Build_Texture_List (DynamicVectorClass<StringClass> &texture_file_list, bool recursive=true);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Decals
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void					Create_Decal(DecalGeneratorClass * generator)						{ }
	virtual void					Delete_Decal(uint32 decal_id)												{ }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual MaterialInfoClass * Get_Material_Info(void) 													{ return NULL; }
	virtual void					Set_User_Data(void *value, bool recursive = false)					{ User_Data = value; };
	virtual void *					Get_User_Data()																{ return User_Data; };
	virtual int						Get_Num_Snap_Points(void)													{ return 0; }
	virtual void					Get_Snap_Point(int index,Vector3 * set)								{ }
//	virtual float					Calculate_Texture_Reduction_Factor(float norm_screensize);
//	virtual void					Set_Texture_Reduction_Factor(float trf);
	virtual float					Get_Screen_Size(CameraClass &camera);
	virtual void					Scale(float scale) 															{ };
	virtual void					Scale(float scalex, float scaley, float scalez)						{ };
 	virtual void					Set_ObjectScale(float scale) { ObjectScale=scale;}	//set's a scale factor that's factored into transform matrix.									{ScaleFactor=scale; };
	float							Get_ObjectScale( void ) const { return ObjectScale; };
 	void							Set_ObjectColor(unsigned int color) { ObjectColor=color;}	//the color that was used to modify the asset for player team color (for Generals). -MW
	unsigned int					Get_ObjectColor( void ) const { return ObjectColor; };

   virtual int						Get_Sort_Level(void) const													{ return 0; /* SORT_LEVEL_NONE */ }
   virtual void					Set_Sort_Level(int level)													{ }

	virtual int						Is_Really_Visible(void)														{ return ((Bits & IS_REALLY_VISIBLE) == IS_REALLY_VISIBLE); }
	virtual int						Is_Not_Hidden_At_All(void)													{ return ((Bits & IS_NOT_HIDDEN_AT_ALL) == IS_NOT_HIDDEN_AT_ALL); }
	virtual int						Is_Visible(void) const														{ return (Bits & IS_VISIBLE); }
	virtual void					Set_Visible(int onoff)														{ if (onoff) { Bits |= IS_VISIBLE; } else { Bits &= ~IS_VISIBLE; } }

// The cheatSpy has been put on ice until later... perhaps the next patch? - M Lorenzen
  //	virtual int						Is_VisibleWithCheatSpy(void) const								{ return ((Bits&=~0x80) & (IS_VISIBLE); }
//	virtual void					Set_VisibleWithCheatSpy(int onoff)								{ if (onoff) { Bits |= IS_VISIBLE|0x80; } else { Bits &= ~IS_VISIBLE; } }

	virtual int						Is_Hidden(void) const														{ return !(Bits & IS_NOT_HIDDEN); }
	virtual void					Set_Hidden(int onoff)														{ if (onoff) { Bits &= ~IS_NOT_HIDDEN; } else { Bits |= IS_NOT_HIDDEN; } }
	virtual int						Is_Animation_Hidden(void) const											{ return !(Bits & IS_NOT_ANIMATION_HIDDEN); }
	virtual void					Set_Animation_Hidden(int onoff)											{ if (onoff) { Bits &= ~IS_NOT_ANIMATION_HIDDEN; } else { Bits |= IS_NOT_ANIMATION_HIDDEN; } }
	virtual int						Is_Force_Visible(void) const												{ return Bits & IS_FORCE_VISIBLE; }
	virtual void					Set_Force_Visible(int onoff)          									{ if (onoff) { Bits |= IS_FORCE_VISIBLE; } else { Bits &= ~IS_FORCE_VISIBLE; } }

	virtual int						Is_Translucent(void) const													{ return Bits & IS_TRANSLUCENT; }
	virtual void					Set_Translucent(int onoff)													{ if (onoff) { Bits |= IS_TRANSLUCENT; } else { Bits &= ~IS_TRANSLUCENT; } }
	virtual int						Is_Alpha(void) const													{ return Bits & IS_ALPHA; }
	virtual void					Set_Alpha(int onoff)													{ if (onoff) { Bits |= IS_ALPHA; } else { Bits &= ~IS_ALPHA; } }
	virtual int						Is_Additive(void) const													{ return Bits & IS_ADDITIVE; }
	virtual void					Set_Additive(int onoff)													{ if (onoff) { Bits |= IS_ADDITIVE; } else { Bits &= ~IS_ADDITIVE; } }
	virtual int						Get_Collision_Type(void) const											{ return (Bits & COLL_TYPE_MASK); }
	virtual void					Set_Collision_Type(int type)												{ Bits &= ~COLL_TYPE_MASK; Bits |= (type & COLL_TYPE_MASK) | COLL_TYPE_ALL; }
   virtual bool					Is_Complete(void)																{ return false; }
	virtual bool					Is_In_Scene(void)																{ return Scene != NULL; }
	virtual float					Get_Native_Screen_Size(void) const										{ return NativeScreenSize; }
	virtual void					Set_Native_Screen_Size(float screensize)								{ NativeScreenSize = screensize; }

	void								Set_Sub_Objects_Match_LOD(int onoff)									{ if (onoff) { Bits |= SUBOBJS_MATCH_LOD; } else { Bits &= ~SUBOBJS_MATCH_LOD; } }
	int								Is_Sub_Objects_Match_LOD_Enabled(void)									{ return Bits & SUBOBJS_MATCH_LOD; }

	void								Set_Sub_Object_Transforms_Dirty(bool onoff)							{ if (onoff) { Bits |= SUBOBJ_TRANSFORMS_DIRTY; } else { Bits &= ~SUBOBJ_TRANSFORMS_DIRTY; } }
	bool								Are_Sub_Object_Transforms_Dirty(void)									{ return (Bits & SUBOBJ_TRANSFORMS_DIRTY) != 0; }

	void								Set_Ignore_LOD_Cost(bool onoff)											{ if (onoff) { Bits |= IGNORE_LOD_COST; } else { Bits &= ~IGNORE_LOD_COST; } }
	bool								Is_Ignoring_LOD_Cost(void)													{ return (Bits & IGNORE_LOD_COST) != 0; }

	void								Set_Is_Self_Shadowed()														{ Bits|=IS_SELF_SHADOWED; }
	void								Unset_Is_Self_Shadowed()													{ Bits&=~IS_SELF_SHADOWED; }
	int								Is_Self_Shadowed() const													{ return (Bits&IS_SELF_SHADOWED); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Persistant object save-load interface
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual const PersistFactoryClass &	Get_Factory (void) const;
	virtual bool					Save (ChunkSaveClass &csave);
	virtual bool					Load (ChunkLoadClass &cload);

	// Application-specific render hook:
	RenderHookClass *				Get_Render_Hook(void) { return RenderHook; }
	void								Set_Render_Hook(RenderHookClass *hook) { if (RenderHook) delete RenderHook; RenderHook = hook; }

protected:

	virtual void					Add_Dependencies_To_List (DynamicVectorClass<StringClass> &file_list, bool textures_only = false);

	virtual void					Update_Cached_Bounding_Volumes(void) const;
	virtual void					Update_Sub_Object_Bits(void);

	bool								Bounding_Volumes_Valid(void) const										{ return (Bits & BOUNDING_VOLUMES_VALID) != 0; }
	void								Invalidate_Cached_Bounding_Volumes(void) const						{ Bits &= ~BOUNDING_VOLUMES_VALID; }
	void								Validate_Cached_Bounding_Volumes(void)	const							{ Bits |= BOUNDING_VOLUMES_VALID; }

	enum
	{
		COLL_TYPE_MASK =		0x000000FF,

		IS_VISIBLE =					0x00000100,
		IS_NOT_HIDDEN =				0x00000200,
		IS_NOT_ANIMATION_HIDDEN =	0x00000400,
		IS_FORCE_VISIBLE =			0x00000800,
		BOUNDING_VOLUMES_VALID =	0x00002000,
		IS_TRANSLUCENT =				0x00004000,			// is additive or alpha blended on any poly
		IGNORE_LOD_COST =				0x00008000,			// used to define if we should ignore object from LOD calculations
		SUBOBJS_MATCH_LOD =			0x00010000,			// force sub-objects to have same LOD level
		SUBOBJ_TRANSFORMS_DIRTY =	0x00020000,			// my sub-objects need me to update their transform
		IS_ALPHA = 0x00040000,	// added for Generals so we can default these meshes not to cast shadows. -MW
		IS_ADDITIVE = 0x00100000,	//added for Generals so we quickly determine what type of blending is on the mesh. -MW
		IS_SELF_SHADOWED =			0x00080000,			// the mesh is self shadowed
		IS_CHEATER =            0x00100000,// the new cheat spy code uses these bits, since nothing else now does
		IS_REALLY_VISIBLE =			IS_VISIBLE | IS_NOT_HIDDEN | IS_NOT_ANIMATION_HIDDEN,
      IS_NOT_HIDDEN_AT_ALL =     IS_NOT_HIDDEN | IS_NOT_ANIMATION_HIDDEN,
		DEFAULT_BITS =					COLL_TYPE_ALL | IS_NOT_HIDDEN | IS_NOT_ANIMATION_HIDDEN,
	};

	mutable unsigned long		Bits;
	Matrix3D							Transform;
 	float						ObjectScale;					//user applied scaling factor inside Transform matrix.
	unsigned int				ObjectColor;					//user applied coloring to the asset/prototype used to make this robj. - For Generals -MW
	mutable SphereClass			CachedBoundingSphere;
	mutable AABoxClass			CachedBoundingBox;
	float								NativeScreenSize;		// The screen size at which the object was designed to be viewed (used in texture resizing).
	mutable bool					IsTransformIdentity;

	SceneClass *					Scene;
	RenderObjClass *				Container;
	void *							User_Data;

	RenderHookClass *				RenderHook;

	friend class SceneClass;
	friend class RenderObjProxyClass;
};

WWINLINE const SphereClass & RenderObjClass::Get_Bounding_Sphere(void) const
{
	if (!(Bits & BOUNDING_VOLUMES_VALID)) {
		Update_Cached_Bounding_Volumes();
	}
	return CachedBoundingSphere;
}

WWINLINE const AABoxClass & RenderObjClass::Get_Bounding_Box(void) const
{
	if (!(Bits & BOUNDING_VOLUMES_VALID)) {
		Update_Cached_Bounding_Volumes();
	}
	return CachedBoundingBox;
}

/**************************************************************************
 * Bound_Degrees -- Bounds a degree value between 0 and 360.              *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   09/22/1997 PWG : Created.                                            *
 *========================================================================*/
WWINLINE float Bound_Degrees(float angle)
{
	while (angle > 359) angle -= 360;
	while (angle < 0) angle += 360;
	return angle;
}

/***********************************************************************************************
 * RenderObjClass::Get_Transform -- returns the transform for the object                       *
 *                                                                                             *
 * If the transform is invalid (a container has been moved or animated) then the transform     *
 * will be recalculated.                                                                       *
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
WWINLINE const Matrix3D & RenderObjClass::Get_Transform(void) const
{
	Validate_Transform();
	return Transform;
}

WWINLINE const Matrix3D & RenderObjClass::Get_Transform(bool &is_transform_identity) const
{
	Validate_Transform();
	is_transform_identity=IsTransformIdentity;
	return Transform;
}

WWINLINE bool RenderObjClass::Is_Transform_Identity() const
{
	Validate_Transform();
	return IsTransformIdentity;
}

// Warning: Be sure to call this function only if the transform is known to be valid!
WWINLINE const Matrix3D & RenderObjClass::Get_Transform_No_Validity_Check(void) const
{
	return Transform;
}

// Warning: Be sure to call this function only if the transform is known to be valid!
WWINLINE const Matrix3D & RenderObjClass::Get_Transform_No_Validity_Check(bool& is_transform_identity) const
{
	is_transform_identity=IsTransformIdentity;
	return Transform;
}

// Warning: Be sure to call this function only if the transform is known to be valid!
WWINLINE bool RenderObjClass::Is_Transform_Identity_No_Validity_Check() const
{
	return IsTransformIdentity;
}


#ifdef DEFINE_W3DANIMMODE_NAMES
static const char* const TheAnimModeNames[] =
{
	"MANUAL",
	"LOOP",
	"ONCE",
	"LOOP_PINGPONG",
	"LOOP_BACKWARDS",
	"ONCE_BACKWARDS",
	NULL
};
static_assert(ARRAY_SIZE(TheAnimModeNames) == RenderObjClass::ANIM_MODE_COUNT + 1, "Incorrect array size");
#endif
