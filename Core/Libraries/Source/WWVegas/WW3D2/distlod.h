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
 *                     $Archive:: /Commando/Code/ww3d2/distlod.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/25/01 2:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef DISTLOD_H
#define DISTLOD_H

#include "proto.h"
#include "rendobj.h"
#include "composite.h"
#include "w3derr.h"

class		ChunkLoadClass;
struct	DistLODNodeDefStruct;
class		DistLODDefClass;


/*
** DistLODClass
** Distance based LOD switching.  This is a simple LOD object which switches models
** based on the pre-set distances.  Note that the models are stored in order of
** descending resolution; i.e. the highest res model will be in index 0.
** Most functions in this class simply pass the call on to all of the LODs.
**
** When the level of detail is changed, the LOD which was being used must be
** notified that it is being removed from the scene and the new LOD must be
** notified that it is being added.  This allows us to correctly handle lights
** and particle emitters in LODs...
*/
class DistLODClass : public CompositeRenderObjClass
{
public:

	DistLODClass(const DistLODDefClass & desc);
	DistLODClass(const DistLODClass & that);
	virtual ~DistLODClass(void);
	virtual RenderObjClass *	Clone(void) const { return W3DNEW DistLODClass(*this); }
	virtual int						Class_ID(void)	const { return CLASSID_DISTLOD; }
	virtual int						Get_Num_Polys(void) const;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	// Access each LOD individually through Get_Sub_Object.
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);

	virtual int						Get_Num_Sub_Objects(void) const;
	virtual RenderObjClass *	Get_Sub_Object(int index) const;

	virtual int						Add_Sub_Object_To_Bone(RenderObjClass * subobj,int bone_index);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Hierarchical Animation
	// Here again, these functions are passed on to all LODs.  Each LOD is
	// assumed to use the same HTree (or have no HTree, this is so that LODs are
	// animation-compatible) so the bone query functions simply pass to the top
	// LOD.
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Set_Animation( void );
	virtual void					Set_Animation( HAnimClass * motion, float frame, int anim_mode = ANIM_MODE_MANUAL);
	virtual void					Set_Animation( HAnimClass * motion0,float frame0,HAnimClass * motion1,float frame1,float percentage);
	virtual void					Set_Animation( HAnimComboClass * anim_combo);
	virtual HAnimClass *			Peek_Animation( void );
	virtual int						Get_Num_Bones(void);
	virtual const char *			Get_Bone_Name(int bone_index);
	virtual int						Get_Bone_Index(const char * bonename);
	virtual const Matrix3D &	Get_Bone_Transform(const char * bonename);
	virtual const Matrix3D &	Get_Bone_Transform(int boneindex);
	virtual void					Capture_Bone(int bindex);
	virtual void					Release_Bone(int bindex);
	virtual bool					Is_Bone_Captured(int bindex) const;
	virtual void					Control_Bone(int bindex,const Matrix3D & tm,bool world_space_translation = false);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Collision Detection, Ray Tracing
	// Collision tests are performed on the top-level LOD.
	/////////////////////////////////////////////////////////////////////////////
	virtual bool					Cast_Ray(RayCollisionTestClass & raytest);
	virtual bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
	virtual bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Attributes, Options, Properties, etc
	/////////////////////////////////////////////////////////////////////////////
	virtual int						Get_Num_Snap_Points(void);
	virtual void					Get_Snap_Point(int index,Vector3 * set);
	virtual void					Scale(float scale);
	virtual void					Scale(float scalex, float scaley, float scalez);

	/////////////////////////////////////////////////////////////////////////////
	// DistLOD Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual float					Get_Switch_Up_Dist(int index) const { return Lods[index].ResUpDist; }
	virtual float					Get_Switch_Down_Dist(int index) const { return Lods[index].ResDownDist; }
	virtual void					Set_Switch_Up_Dist(int index, float dist) { Lods[index].ResUpDist = dist; }
	virtual void					Set_Switch_Down_Dist(int index, float dist) { Lods[index].ResDownDist = dist; }

private:

	enum { HIGHEST_LOD = 0 };

	void								Free(void);
	void								Update_Lod(const CameraClass & camera);
	void								Increment_Lod(void);
	void								Decrement_Lod(void);

	struct LODNodeClass
	{
		RenderObjClass *			Model;
		float							ResUpDist;
		float							ResDownDist;
	};

	int								LodCount;				// how many models are in this LOD object
	int								CurLod;					// which model is currently used for rendering
	int								VpPushLod;				// which model was used for the vp->push (in CurLod changes before the pop)
	LODNodeClass *					Lods;						// one LODNodeClass for each level
};


/*
** Loaders for DistLODClass
*/
class DistLODLoaderClass : public PrototypeLoaderClass
{
public:

	virtual int						Chunk_Type (void)  { return W3D_CHUNK_LODMODEL; }
	virtual PrototypeClass *	Load_W3D(ChunkLoadClass & cload);
};

/*
** DistLODModelDefStruct
** Describes a single model in a Distance-Based LOD Object
*/
struct DistLODNodeDefStruct
{
	DistLODNodeDefStruct(void) : Name(NULL),ResDownDist(0.0f),ResUpDist(0.0f) {}

	char *	Name;
	float		ResDownDist;
	float		ResUpDist;
};

/*
** DistLODDefClass
** Description of a Distance-Based LOD Object.  These are similar
** to the HModelDef's that are used by the asset manager to construct
** HModels.
*/
class DistLODDefClass
{
public:

	DistLODDefClass(void);
	DistLODDefClass(const char * name,int lodcount,DistLODNodeDefStruct * lods);
	~DistLODDefClass(void);

	WW3DErrorType				Load_W3D(ChunkLoadClass & cload);
	const char *				Get_Name(void) const { return Name; }

private:

	char * 						Name;

	int							LodCount;
	DistLODNodeDefStruct *	Lods;

	void							Free(void);
	bool							read_header(ChunkLoadClass & cload);
	bool							read_node(ChunkLoadClass & cload,DistLODNodeDefStruct * node);

	friend class DistLODClass;
};

/*
** Prototype for Dist-LOD objects
*/
class DistLODPrototypeClass : public W3DMPO, public PrototypeClass
{
	W3DMPO_GLUE(DistLODPrototypeClass)
public:
	DistLODPrototypeClass( DistLODDefClass *def ) { Definition = def; }

	virtual const char *			Get_Name(void) const			{ return Definition->Get_Name(); }
	virtual int								Get_Class_ID(void) const	{ return RenderObjClass::CLASSID_DISTLOD; }
	virtual RenderObjClass *	Create(void);
	virtual void							DeleteSelf()							{ delete this; }

protected:
	virtual ~DistLODPrototypeClass(void) { delete Definition; }

private:
	DistLODDefClass *				Definition;
};

/*
** Instance of the loaders which the asset manager install
*/
extern DistLODLoaderClass			_DistLODLoader;

#endif
