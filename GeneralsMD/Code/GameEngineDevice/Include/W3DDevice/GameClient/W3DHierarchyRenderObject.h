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
 *                     $Archive:: /Commando/Code/ww3d2/HLOD.h                                 $*
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
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
import Graphics.Scene.Models.Factory;

#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
import Assets.ModelAssembly;
import Assets.Cache.Animations;


#include "W3DDevice/GameClient/W3DAnimatedModelRenderObject.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DIntersectionQuery.h"
#include "WWLib/ref_ptr.h"
import Graphics.Scene.Models.Children;
import Graphics.Scene.Models.DetailLevels;
#include "WWLib/Vector.h"


class HModelClass;



/*

	W3DHierarchyRenderObject

	This is an hierarchical, animatable level-of-detail model.

*/
class W3DHierarchyRenderObject : public W3DAnimatedModelRenderObject
{
	W3DMPO_CODE(W3DHierarchyRenderObject)
public:

	W3DHierarchyRenderObject(const W3DHierarchyRenderObject & src);
	W3DHierarchyRenderObject(const char * name,W3DRenderObject ** lods,int count);
	W3DHierarchyRenderObject(const Assets::ModelAssemblyDesc & def);

	W3DHierarchyRenderObject & operator = (const W3DHierarchyRenderObject &);
	virtual ~W3DHierarchyRenderObject() override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Cloning and Identification
	/////////////////////////////////////////////////////////////////////////////
	virtual W3DRenderObject *	Clone() const override;
	virtual int						Class_ID() const override { return CLASSID_HLOD; }
	virtual int						Get_Num_Polys() const override;

	/////////////////////////////////////////////////////////////////////////////
	// HLod Interface - Editing and information
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Set_Max_Screen_Size(int lod_index, float size);
	virtual float					Get_Max_Screen_Size(int lod_index) const;

	virtual int						Get_Lod_Count() const;
	virtual int						Get_Lod_Model_Count (int lod_index) const;
	virtual W3DRenderObject *	Peek_Lod_Model (int lod_index, int model_index) const;
	virtual W3DRenderObject *	Get_Lod_Model (int lod_index, int model_index) const;
	virtual int						Get_Lod_Model_Bone (int lod_index, int model_index) const;
	virtual int						Get_Additional_Model_Count() const;
	virtual W3DRenderObject *	Peek_Additional_Model (int model_index) const;
	virtual W3DRenderObject *	Get_Additional_Model (int model_index) const;
	virtual int						Get_Additional_Model_Bone (int model_index) const;
	virtual void					Add_Lod_Model(int lod, W3DRenderObject * robj, int boneindex);


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(W3DRenderContext & rinfo) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m) override;
	virtual void 					Set_Position(const Vector3 &v) override;

	virtual void					Notify_Added(W3DScene * scene) override;
	virtual void					Notify_Removed(W3DScene * scene) override;

	virtual int						Get_Num_Sub_Objects() const override;
	virtual W3DRenderObject *	Get_Sub_Object(int index) const override;
	virtual int						Add_Sub_Object(W3DRenderObject * subobj) override;
	virtual int						Remove_Sub_Object(W3DRenderObject * robj) override;

	virtual int						Get_Num_Sub_Objects_On_Bone(int boneindex) const override;
	virtual W3DRenderObject *	Get_Sub_Object_On_Bone(int index,int boneindex) const override;
	virtual int						Get_Sub_Object_Bone_Index(W3DRenderObject * subobj) const override;
	virtual int						Get_Sub_Object_Bone_Index(int LodIndex, int ModelIndex)	const override;
	virtual int						Add_Sub_Object_To_Bone(W3DRenderObject * subobj,int bone_index) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Hierarchical Animation
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Set_Animation() override;
	virtual void					Set_Animation( Assets::AnimationAssetHandle motion,
															float frame, int anim_mode = ANIM_MODE_MANUAL) override;
	virtual void					Set_Animation( Assets::AnimationAssetHandle motion0,
															float frame0,
															Assets::AnimationAssetHandle motion1,
															float frame1,
															float percentage) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection, Ray Tracing
	/////////////////////////////////////////////////////////////////////////////
	virtual bool					Cast_Ray(W3DRayCastQuery & raytest) override;
	virtual bool					Cast_AABox(W3DBoxCastQuery & boxtest) override;
	virtual bool					Cast_OBBox(W3DOrientedBoxCastQuery & boxtest) override;
	virtual bool					Intersect_AABox(W3DBoxIntersectionQuery & boxtest) override;
	virtual bool					Intersect_OBBox(W3DOrientedBoxIntersectionQuery & boxtest) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Predictive LOD
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Prepare_LOD(W3DCamera &camera) override;
   virtual void					Recalculate_Static_LOD_Factors() override;
	virtual void					Increment_LOD() override;
	virtual void					Decrement_LOD() override;
	virtual float					Get_Cost() const override;
	virtual float					Get_Value() const override;
	virtual float					Get_Post_Increment_Value() const override;
	virtual void					Set_LOD_Level(int lod) override;
	virtual int						Get_LOD_Level() const override;
	virtual int						Get_LOD_Count() const override;
	virtual void					Set_LOD_Bias(float bias) override;
	virtual int						Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const override;
	virtual W3DRenderObject *	Get_Current_LOD() override;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual const SphereClass &	Get_Bounding_Sphere() const override;
	virtual const AABoxClass &		Get_Bounding_Box() const override;
	virtual void						Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const override;
	virtual void						Get_Obj_Space_Bounding_Box(AABoxClass & box) const override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
//   virtual void					Set_Texture_Reduction_Factor(float trf);
	virtual void					Scale(float scale) override;
	virtual void					Scale(float scalex, float scaley, float scalez) override { }
	virtual int						Get_Num_Snap_Points() override;
	virtual void					Get_Snap_Point(int index,Vector3 * set) override;
	virtual void					Set_Hidden(int onoff) override;

	// (gth) TESTING DYNAMICALLY SWAPPING SKELETONS!

protected:

	W3DHierarchyRenderObject();

	void								Free();
	virtual void					Update_Sub_Object_Transforms() override;
	virtual void					Update_Obj_Space_Bounding_Volumes() override;

protected:


    using ChildOwner = RefCountPtr<W3DRenderObject>;
    using ChildAttachment = Graphics::ModelChildren<ChildOwner>::Attachment;
    Graphics::ModelChildren<ChildOwner> m_children;
    Graphics::ModelDetailLevels m_detail_levels;
    int BoundingBoxIndex;

	// possible array of snap points.
	std::vector<Assets::Vector3f> SnapPoints;

	// possible array of proxy objects (names and bone indexes for application defined usage)
	std::vector<Assets::ModelAttachmentDesc> Proxies;
};


/*
** Loaders for W3DHierarchyRenderObject
*/
Graphics::ModelFactory<W3DRenderObject>* Load_HLod_Factory(ChunkLoadClass& cload);


/*
** Prototype for HLod objects
*/


/*
** Instance of the loaders which the asset manager install
*/

Graphics::ModelFactory<W3DRenderObject>* Load_HModel_Factory(ChunkLoadClass& cload);
Graphics::ModelFactory<W3DRenderObject>* Load_ModelLevels_Factory(ChunkLoadClass& cload);
Graphics::ModelFactory<W3DRenderObject>* Load_Aggregate_Factory(ChunkLoadClass& cload);
