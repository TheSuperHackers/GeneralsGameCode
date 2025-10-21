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
 *                     $Archive:: /Commando/Code/ww3d2/decalsys.h                             $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/29/01 11:30a                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef DECALSYS_H
#define DECALSYS_H

#include "always.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "obbox.h"
#include "robjlist.h"
#include "matpass.h"
#include "projector.h"

class DecalGeneratorClass;
class DecalMeshClass;

/**
** DecalSystemClass
** This is a class that manages creation and destruction of decals in the system.  It is
** meant to be over-ridden for game-specific behaviors.
**
** Sample Code:
** 1 - Create the generator.  The system gives it a unique id and gives you a clean decal generator
**
** DecalGeneratorClass * gen = DecalSystem->Lock_Decal_Generator();
** gen->Set_Transform(tm);
** gen->Set_Projection(proj_tm);
** gen->Set_Bounds(OBBox);
**
** 2 - Apply the generator to any objects that you want it to affect.  It wont do anything if it
**     does not overlap any polygons in those objects
**
** Scene->Collect_Objects(gen->Get_Bounding_Box(),objectlist);
** for (iterator.First(objectlist); !iterator.Is_Done(); iterator.Next()) {
**   iterator.Peek_Object()->Create_Decal(gen);
** }
**
** 3 - Release the generator back to the system.  At this point, the system may record which meshes
**     actually generated extra decal polygons for future removal.  All of this information will be
**     tied together with the unique 'decal ID' that was assigned to the generator.
**
** DecalSystem->Unlock_Decal_Generator(gen);
**
*/
class DecalSystemClass
{
public:
	DecalSystemClass(void);
	virtual ~DecalSystemClass(void);

	/*
	**	Create and release DecalGenerators.  Note that this is the point at which the
	** decal system can track "logical" decals.  The generator will keep an internal list
	** of all of the render objects which generated decals which you should copy if you
	** want to track them (e.g. if you want to cap the maximum number of active decals and
	** kill the old ones...)
	*/
	virtual DecalGeneratorClass *		Lock_Decal_Generator(void);
	virtual void							Unlock_Decal_Generator(DecalGeneratorClass * generator);

	/*
	** When a decal-mesh is destroyed, it must inform the DecalSystem.  Otherwise, systems
	** which track decals can get dangling pointers.
	*/
	virtual void							Decal_Mesh_Destroyed(uint32 decal_id,DecalMeshClass * mesh)	{ }

protected:

	/*
	** This generates the decal ID when a generator is created. This decal system reroutes this
	** to Generate_Unique_Global_Decal_Id(), but other decal systems may use a different method.
	*/
	virtual uint32							Generate_Decal_Id(void) { return Generate_Unique_Global_Decal_Id(); }

	/*
	** Unique ID generation for decals.  Not all decal systems have to use
	** this method of generating ids.  Some may wish to assign the id as the
	** array index of the logical id or use some other aritrary method.
	*/
	static uint32							Generate_Unique_Global_Decal_Id(void);

	static uint32							DecalIDGenerator;
};


/**
** DecalGeneratorClass
** This class encapsulates the information needed to generate a decal.   It also tracks
** what meshes actually used it to generate new decal polygons.
*/
class DecalGeneratorClass : public ProjectorClass
{
public:

	/*
	** All meshes that actually generate decal polygons should register themselves in the
	** list.  Then when the decal generation is finished, this list can be copied so that
	** we can come back to those meshes and remove the decals if we want to.
	*/
	void								Add_Mesh(RenderObjClass * mesh);
	NonRefRenderObjListClass &	Get_Mesh_List(void);

	/*
	** Decal generator parameters.
	** see ProjectorClass for control over the coordinate system, projection, etc
	*/
	uint32							Get_Decal_ID(void)								{ return DecalID; }
	DecalSystemClass *			Peek_Decal_System(void)							{ return System; }

	/*
	** Backface rejection thresh-hold.  The dot-product between the projection vector and
	** the normal of each polygon is taken, if the result is greater than this value the polygon
	** is accepted into the decal.  Set it to -1 if you want to accept all polygons.
	*/
	void								Set_Backface_Threshhold(float val)			{ BackfaceVal = val; }
	float								Get_Backface_Threshhold(void)					{ return BackfaceVal; }

	/*
	** Normally, decals are not generated on translucent meshes.  This is due to the "floating
	** decals" that you can get on things like trees.  The user can override this behavior
	** through the following interface.
	*/
	void								Apply_To_Translucent_Meshes(bool onoff)	{ ApplyToTranslucentMeshes = onoff; }
	bool								Is_Applied_To_Translucent_Meshes(void)		{ return ApplyToTranslucentMeshes; }

	/*
	** Material parameters: just grab a pointer the material pass and modify it.
	** Remember to release your ref to it when you are done.
	*/
	MaterialPassClass *			Get_Material(void)								{ WWASSERT(Material != NULL); Material->Add_Ref(); return Material; }

	/*
	** Decal generation support.  Call Set_Mesh_Transform for the mesh you want to add
	** a decal to.  Then for each vertex, you can call 'Compute_Texture_Coordinate'
	*/
	void								Set_Mesh_Transform(const Matrix3D & tm);

protected:

	DecalGeneratorClass(uint32 id,DecalSystemClass * system);
	~DecalGeneratorClass(void);

	/*
	** Logical Decal ID, DecalSystem that this generator is tied to
	*/
	DecalSystemClass *			System;
	uint32							DecalID;								// unique ID generated by the DecalSystem

	/*
	** Backface Threshhold, Translucent mesh decal enable
	*/
	float								BackfaceVal;
	bool								ApplyToTranslucentMeshes;

	/*
	** Material settings
	*/
	MaterialPassClass *			Material;							// material settings for the decal

	/*
	** Results, list of the meshes which actually generated decal polygons for this logical decal.
	*/
	NonRefRenderObjListClass	MeshList;

	friend class DecalSystemClass;
};


/**
** MultiFixedPoolDecalSystemClass: This is for decal systems which use several fixed-size decal
** pools (If more than one pool is used, it is usually to have separate pools for several
** different classes of decals). Note that here the decal IDs are a combination of the pool and
** slot ids. (decal ids only have to be unique within a given decal system)
*/

class MultiFixedPoolDecalSystemClass : public DecalSystemClass
{

public:

	MultiFixedPoolDecalSystemClass(uint32 num_pools, const uint32 *pool_sizes);
	MultiFixedPoolDecalSystemClass(const MultiFixedPoolDecalSystemClass & that);
	virtual ~MultiFixedPoolDecalSystemClass(void);

	// This clears the slot in addition to locking the generator, thus preventing any decal id
	// collisions (since any decal previously in that slot will have the same id as the new one).
	virtual DecalGeneratorClass *			Lock_Decal_Generator(void);

	// This will register the decal in the system in the appropriate pool and slot (determined by
	// the generator's pool and slot ids), removing any decal which may have been there before.
	virtual void								Unlock_Decal_Generator(DecalGeneratorClass * generator);

	// This notifies the system that a mesh which has decals on it was destroyed - therefore we
	// need to remove the mesh from our list to avoid dangling pointers.
	virtual void								Decal_Mesh_Destroyed(uint32 id,DecalMeshClass * mesh);

	// Not part of the DecalSystemClass interface - this function removes any decal currently in
	// the given slot in the given pool.
	void											Clear_Decal_Slot(uint32 pool_id, uint32 slot_id);

	// This one removes all decals in a given pool.
	void											Clear_Pool(uint32 pool_id);

	// And this one removes all decals in the system.
	void											Clear_All_Decals(void);

protected:

	/*
	** This generates the decal ID when a generator is created. This decal system generates the
	** decal ID from a pool ID and slot ID which are part of the state of the system so someone
	** can set them before calling Lock_Decal_Generator() (which is where this function is called).
	** We do it this way to avoid needing to override Lock_Decal_Generator().
	*/
	virtual	uint32	Generate_Decal_Id(void) { return encode_decal_id(Generator_PoolID, Generator_SlotID); }
	uint32	Generator_PoolID;		// These should be set before calling Lock_Decal_Generator()
	uint32	Generator_SlotID;		// These should be set before calling Lock_Decal_Generator()

	class LogicalDecalClass;

	// Get a reference to the logical decal at the given pool and slot id (performs range chacking)
	LogicalDecalClass & find_logical_decal(uint32 pool_id, uint32 slot_id);

	// Get a reference to the logical decal with the given decal id
	LogicalDecalClass & find_logical_decal(uint32 decal_id);

	// The decal ids are formed so that the upper 16 bits are equal to the pool id and the lower
	// 16 bits are equal to the slot id.
	static uint32 encode_decal_id(uint32 pool_id, uint32 slot_id) { return (slot_id & 0xFFFF) | (pool_id << 16); }
	static void decode_decal_id(uint32 decal_id, uint32 & pool_id, uint32 & slot_id) { slot_id = decal_id & 0xFFFF; pool_id = decal_id >> 16; }

	// A class to store the meshes to which the decal has been applied (so that they can be removed when needed)
	class LogicalDecalClass
	{
	public:
		LogicalDecalClass(void);
		~LogicalDecalClass(void);

		void											Set(DecalGeneratorClass * generator);

		// Just clears any existing logical decal information, leaving the decal empty.
		void											Clear(uint32 decal_id);

		NonRefRenderObjListClass	MeshList;
	};

	class LogicalDecalPoolClass
	{
	public:
		LogicalDecalPoolClass(void);
		~LogicalDecalPoolClass(void);

		void Initialize(uint32 size);

		LogicalDecalClass *		Array;
		uint32						Size;
	};

	LogicalDecalPoolClass *						Pools;
	uint32											PoolCount;

};




#endif //DECALSYS_H

