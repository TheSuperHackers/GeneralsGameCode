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
 *                     $Archive:: /Commando/Code/ww3d2/hlod.h                                 $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/25/01 1:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef ANIMOBJ_H
#include "animobj.h"
#endif

#ifndef VECTOR_H
#include "Vector.H"
#endif

#ifndef SNAPPTS_H
#include "snapPts.h"
#endif

#ifndef PROTO_H
#include "proto.h"
#endif

#ifndef W3DERR_H
#include "w3derr.h"
#endif

#ifndef __PROXY_H
#include "proxy.h"
#endif


class DistLODClass;
class HModelClass;
class HLodDefClass;
class HModelDefClass;
class ProxyArrayClass;



/*

	HLodClass

	This is an hierarchical, animatable level-of-detail model.

*/
class HLodClass : public W3DMPO, public Animatable3DObjClass
{
	W3DMPO_GLUE(HLodClass)
public:

	HLodClass(const HLodClass & src);
	HLodClass(const char * name,RenderObjClass ** lods,int count);
	HLodClass(const HLodDefClass & def);
	HLodClass(const HModelDefClass & def);

	HLodClass & operator = (const HLodClass &);
	virtual ~HLodClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Cloning and Identification
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const										{ return CLASSID_HLOD; }
	virtual int						Get_Num_Polys(void) const;

	/////////////////////////////////////////////////////////////////////////////
	// HLod Interface - Editing and information
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Set_Max_Screen_Size(int lod_index, float size);
	virtual float					Get_Max_Screen_Size(int lod_index) const;

	virtual int						Get_Lod_Count(void) const;
	virtual int						Get_Lod_Model_Count (int lod_index) const;
	virtual RenderObjClass *	Peek_Lod_Model (int lod_index, int model_index) const;
	virtual RenderObjClass *	Get_Lod_Model (int lod_index, int model_index) const;
	virtual int						Get_Lod_Model_Bone (int lod_index, int model_index) const;
	virtual int						Get_Additional_Model_Count(void) const;
	virtual RenderObjClass *	Peek_Additional_Model (int model_index) const;
	virtual RenderObjClass *	Get_Additional_Model (int model_index) const;
	virtual int						Get_Additional_Model_Bone (int model_index) const;
	virtual void					Add_Lod_Model(int lod, RenderObjClass * robj, int boneindex);

	virtual bool					Is_NULL_Lod_Included (void) const;
	virtual void					Include_NULL_Lod (bool include = true);

	/////////////////////////////////////////////////////////////////////////////
	// Proxy interface
	/////////////////////////////////////////////////////////////////////////////
	virtual int						Get_Proxy_Count (void) const;
	virtual bool					Get_Proxy (int index, ProxyClass &proxy) const;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);

	virtual void					Notify_Added(SceneClass * scene);
	virtual void					Notify_Removed(SceneClass * scene);

	virtual int						Get_Num_Sub_Objects(void) const;
	virtual RenderObjClass *	Get_Sub_Object(int index) const;
	virtual int						Add_Sub_Object(RenderObjClass * subobj);
	virtual int						Remove_Sub_Object(RenderObjClass * robj);

	virtual int						Get_Num_Sub_Objects_On_Bone(int boneindex) const;
	virtual RenderObjClass *	Get_Sub_Object_On_Bone(int index,int boneindex) const;
	virtual int						Get_Sub_Object_Bone_Index(RenderObjClass * subobj) const;
	virtual int						Get_Sub_Object_Bone_Index(int LodIndex, int ModelIndex)	const;
	virtual int						Add_Sub_Object_To_Bone(RenderObjClass * subobj,int bone_index);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Hierarchical Animation
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Set_Animation(void);
	virtual void					Set_Animation( HAnimClass * motion,
															float frame, int anim_mode = ANIM_MODE_MANUAL);
	virtual void					Set_Animation( HAnimClass * motion0,
															float frame0,
															HAnimClass * motion1,
															float frame1,
															float percentage);
	virtual void					Set_Animation( HAnimComboClass * anim_combo);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection, Ray Tracing
	/////////////////////////////////////////////////////////////////////////////
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
	virtual bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
	virtual bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Predictive LOD
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Prepare_LOD(CameraClass &camera);
   virtual void					Recalculate_Static_LOD_Factors(void);
	virtual void					Increment_LOD(void);
	virtual void					Decrement_LOD(void);
	virtual float					Get_Cost(void) const;
	virtual float					Get_Value(void) const;
	virtual float					Get_Post_Increment_Value(void) const;
	virtual void					Set_LOD_Level(int lod);
	virtual int						Get_LOD_Level(void) const;
	virtual int						Get_LOD_Count(void) const;
	virtual void					Set_LOD_Bias(float bias);
	virtual int						Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const;
	virtual RenderObjClass *	Get_Current_LOD(void);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual const SphereClass &	Get_Bounding_Sphere(void) const;
	virtual const AABoxClass &		Get_Bounding_Box(void) const;
	virtual void						Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
	virtual void						Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Decals
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void					Create_Decal(DecalGeneratorClass * generator);
	virtual void					Delete_Decal(uint32 decal_id);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
//   virtual void					Set_Texture_Reduction_Factor(float trf);
	virtual void					Scale(float scale);
	virtual void					Scale(float scalex, float scaley, float scalez)	{ }
	virtual int						Get_Num_Snap_Points(void);
	virtual void					Get_Snap_Point(int index,Vector3 * set);
	virtual void					Set_Hidden(int onoff);

	// (gth) TESTING DYNAMICALLY SWAPPING SKELETONS!
	virtual void					Set_HTree(HTreeClass * htree);

protected:

	HLodClass(void);

	void								Free(void);
	virtual void					Update_Sub_Object_Transforms(void);
	virtual void					Update_Obj_Space_Bounding_Volumes(void);

protected:


	class ModelNodeClass
	{
	public:
		RenderObjClass *			Model;
		int							BoneIndex;
		bool operator == (const ModelNodeClass & that) { return (Model == that.Model) && (BoneIndex == that.BoneIndex); }
		bool operator != (const ModelNodeClass & that) { return !operator == (that); }
	};

	class ModelArrayClass : public DynamicVectorClass<ModelNodeClass>
	{
	public:
		ModelArrayClass(void) : MaxScreenSize(NO_MAX_SCREEN_SIZE), NonPixelCost(0.0f),
			PixelCostPerArea(0.0f), BenefitFactor(0.0f) {}
		float							MaxScreenSize;		// Maximum screen size for this LOD
		float							NonPixelCost;		// Cost heuristics of LODS (w/o per-pixel cost)
		float							PixelCostPerArea;	// PixelCostPerArea * area(normalized) + NonPixelCost = total Cost
		float							BenefitFactor;		// BenefitFactor * area(normalized) = Benefit
	};

	// Lod Render Objects, basically one of the LOD Models will be rendered. Typically
	// each model in an HLodModel will be a mesh or a "simple" HLod (one with a single LOD)
	int								LodCount;
	int								CurLod;
	ModelArrayClass *				Lod;

	//
	//	An animating heirarchy can use a hidden CLASSID_OBBOX mesh to represent its bounding
	// box as it animates.  This is the sub object index of that mesh (if it exists).
	//
	int								BoundingBoxIndex;

	float *							Cost;					// Cost array (recalculated every frame)
	float *							Value;				// Value array (recalculated every frame)

	// Additional Models, these models have been linked to one of the bones in this
	// model.  They are all always rendered.  They can be HLODs themselves in order
	// to implement switching on sub models.
	// NOTE: This uses ModelArrayClass for convenience, but MaxScreenSize,
	// NonPixelCost, PixelCostPerArea, BenefitFactor are not used here.
	ModelArrayClass				AdditionalModels;

	// possible array of snap points.
	SnapPointsClass *				SnapPoints;

	// possible array of proxy objects (names and bone indexes for application defined usage)
	ProxyArrayClass *				ProxyArray;

	// Current LOD Bias (affects recalculation of the Value array)
	float								LODBias;
};


/*
** Loaders for HLodClass
*/
class HLodLoaderClass : public PrototypeLoaderClass
{
public:
	virtual int						Chunk_Type (void)  { return W3D_CHUNK_HLOD; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};


/**
** HLodDefClass
** This description object is generated when reading a W3D_CHUNK_HLOD.  It
** directly describes the contents of an HLod model.
*/
class HLodDefClass : public W3DMPO
{
	W3DMPO_GLUE(HLodDefClass)
public:

	HLodDefClass(void);
	HLodDefClass(HLodClass &src_lod);
	~HLodDefClass(void);

	WW3DErrorType				Load_W3D(ChunkLoadClass & cload);
	WW3DErrorType				Save(ChunkSaveClass & csave);
	const char *				Get_Name(void) const { return Name; }
	void							Initialize(HLodClass &src_lod);

protected:

	/*
	** Serializtion methods
	*/
	WW3DErrorType				Save_Header (ChunkSaveClass &csave);
	WW3DErrorType				Save_Lod_Array (ChunkSaveClass &csave);
	WW3DErrorType				Save_Aggregate_Array(ChunkSaveClass & csave);

private:

	/*
	** SubObjectArrayClass
	** Describes a level-of-detail in an HLod object.  Note that this is
	** a render object which will be exploded when the HLod is constructed (its
	** sub-objects, if any, will be placed into the HLod).
	*/
	class SubObjectArrayClass
	{
	public:
		SubObjectArrayClass(void);
		~SubObjectArrayClass(void);
		void		Reset(void);
		void		operator = (const SubObjectArrayClass & that);

		bool		Load_W3D(ChunkLoadClass & cload);
		bool		Save_W3D(ChunkSaveClass & csave);

		float		MaxScreenSize;
		int		ModelCount;
		char **	ModelName;				// array of model names
		int *		BoneIndex;				// array of bone indices
	};

	char * 						Name;
	char *						HierarchyTreeName;
	int							LodCount;
	SubObjectArrayClass *	Lod;
	SubObjectArrayClass		Aggregates;
	ProxyArrayClass *			ProxyArray;

	void							Free(void);
	bool							read_header(ChunkLoadClass & cload);
	bool							read_proxy_array(ChunkLoadClass & cload);

	friend class HLodClass;
};


/*
** Prototype for HLod objects
*/
class HLodPrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(HLodPrototypeClass)
public:
	HLodPrototypeClass( HLodDefClass *def )					{ Definition = def; }

	virtual const char *			Get_Name(void) const			{ return Definition->Get_Name(); }
	virtual int								Get_Class_ID(void) const	{ return RenderObjClass::CLASSID_HLOD; }
	virtual RenderObjClass *	Create(void);
	virtual void							DeleteSelf()							{ delete this; }

	HLodDefClass *					Get_Definition(void) const	{ return Definition; }

protected:
	virtual ~HLodPrototypeClass(void)							{ delete Definition; }

private:
	HLodDefClass *					Definition;
};

/*
** Instance of the loaders which the asset manager install
*/
extern HLodLoaderClass			_HLodLoader;
