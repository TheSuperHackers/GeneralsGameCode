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
 *                     $Archive:: /Commando/Code/ww3d2/decalsys.cpp                           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DecalSystemClass::DecalSystemClass -- Constructor                                         *
 *   DecalSystemClass::~DecalSystemClass -- Destructor                                         *
 *   DecalSystemClass::Lock_Decal_Generator -- returns a DecalGenerator                        *
 *   DecalSystemClass::Unlock_Decal_Generator -- Destroys the decal generator                  *
 *   DecalSystemClass::Generate_Unique_Global_Decal_Id -- Generates a unique id for a decal    *
 *   DecalGeneratorClass::DecalGeneratorClass -- Constructor                                   *
 *   DecalGeneratorClass::~DecalGeneratorClass -- Destructor                                   *
 *   DecalGeneratorClass::Add_Mesh -- Meshes that generate a decal should add themselves       *
 *   DecalGeneratorClass::Get_Mesh_List -- returns the list of meshes                          *
 *   DecalGeneratorClass::Set_Mesh_Transform -- sets the current mesh coordinate system        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "decalsys.h"
#include "rendobj.h"
#include "mesh.h"
#include "decalmsh.h"
#include "matrixmapper.h"
#include "texture.h"


uint32 DecalSystemClass::DecalIDGenerator = 0;


/*
** DecalSystemClass Implementation
*/

/***********************************************************************************************
 * DecalSystemClass::DecalSystemClass -- Constructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
DecalSystemClass::DecalSystemClass(void)
{
}


/***********************************************************************************************
 * DecalSystemClass::~DecalSystemClass -- Destructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
DecalSystemClass::~DecalSystemClass(void)
{
}


/***********************************************************************************************
 * DecalSystemClass::Lock_Decal_Generator -- returns a DecalGenerator                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
DecalGeneratorClass * DecalSystemClass::Lock_Decal_Generator(void)
{
	DecalGeneratorClass * gen = W3DNEW DecalGeneratorClass(Generate_Decal_Id(), this);
	return gen;
}


/***********************************************************************************************
 * DecalSystemClass::Unlock_Decal_Generator -- Destroys the decal generator                    *
 *                                                                                             *
 * Derived classes may take a record of the results of the generator here.  Then they can      *
 * put a cap on the total number of decals in the system or whatever...                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void DecalSystemClass::Unlock_Decal_Generator(DecalGeneratorClass * generator)
{
	delete generator;
}


/***********************************************************************************************
 * DecalSystemClass::Generate_Unique_Global_Decal_Id -- Generates a unique id for a decal      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Hopefully you won't use more than 4 billion decals at one time...                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
uint32 DecalSystemClass::Generate_Unique_Global_Decal_Id(void)
{
	return DecalIDGenerator++;
}


/*
** DecalGeneratorClass Implementation
*/

/***********************************************************************************************
 * DecalGeneratorClass::DecalGeneratorClass -- Constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
DecalGeneratorClass::DecalGeneratorClass(uint32 id,DecalSystemClass * system) :
	DecalID(id),
	System(system),
	BackfaceVal(0.0f),
	ApplyToTranslucentMeshes(false),
	Material(nullptr)
{
	Material = NEW_REF(MaterialPassClass,());

	WWASSERT(System != nullptr);
	WWASSERT(Material != nullptr);
}


/***********************************************************************************************
 * DecalGeneratorClass::~DecalGeneratorClass -- Destructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
DecalGeneratorClass::~DecalGeneratorClass(void)
{
	REF_PTR_RELEASE(Material);
}


/***********************************************************************************************
 * DecalGeneratorClass::Add_Mesh -- Meshes that generate a decal should add themselves         *
 *                                                                                             *
 * This adds a mesh to the internal list.  All meshes that actually add a decal should add     *
 * themselves to this list so that the DecalSystem can keep track of them.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void DecalGeneratorClass::Add_Mesh(RenderObjClass * mesh)
{
	WWASSERT(mesh->Class_ID() == RenderObjClass::CLASSID_MESH);
	MeshList.Add(mesh);
}


/***********************************************************************************************
 * DecalGeneratorClass::Get_Mesh_List -- returns the list of meshes                            *
 *                                                                                             *
 * This is the list of meshes which generated decals for this "logical decal"                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
NonRefRenderObjListClass & DecalGeneratorClass::Get_Mesh_List(void)
{
	return MeshList;
}


/***********************************************************************************************
 * DecalGeneratorClass::Set_Mesh_Transform -- sets the current mesh coordinate system          *
 *                                                                                             *
 * The coordinate system of the mesh is needed in order to compute texture coordinates for     *
 * the decal polygons.                                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/26/00    gth : Created.                                                                 *
 *=============================================================================================*/
void DecalGeneratorClass::Set_Mesh_Transform(const Matrix3D & mesh_transform)
{
	/*
	** Mobj-texture = Projection * Mwrld-texture * Mobj-wrld
	*/
	Matrix3D world_to_texture;
	Matrix3D tmp;
	Matrix4x4  mesh_to_texture;

	Transform.Get_Orthogonal_Inverse(world_to_texture);
	Matrix3D::Multiply(world_to_texture,mesh_transform,&tmp);
	Matrix4x4::Multiply(Projection,tmp,&mesh_to_texture);

	/*
	** Plug the matrix and texture size into the mapper
	*/
	if (WW3D::Is_Texturing_Enabled()) {
		float texsize = 64.0f;
		TextureClass * tex = Material->Peek_Texture();
		WWASSERT(tex != nullptr);
		if (tex) {
//			SurfaceClass::SurfaceDescription surface_desc;
//			tex->Get_Level_Description(surface_desc);
//			texsize = surface_desc.Width;
			texsize = tex->Get_Width();
		}

		Mapper->Set_Texture_Transform(mesh_to_texture,texsize);
	}
}


/*
** MultiFixedPoolDecalSystemClass implementation
*/

MultiFixedPoolDecalSystemClass::MultiFixedPoolDecalSystemClass(uint32 num_pools, const uint32 *pool_sizes) :
	Pools(nullptr),
	PoolCount(num_pools),
	Generator_PoolID(0),
	Generator_SlotID(0)
{
	if (PoolCount)
	{
		WWASSERT(pool_sizes);
		Pools = W3DNEWARRAY LogicalDecalPoolClass [PoolCount];
	}
	for (uint32 i = 0; i < PoolCount; i++) {
		assert(pool_sizes[i]);
		Pools[i].Initialize(pool_sizes[i]);
	}
}

MultiFixedPoolDecalSystemClass::MultiFixedPoolDecalSystemClass(const MultiFixedPoolDecalSystemClass & that) :
	Pools(nullptr),
	PoolCount(that.PoolCount),
	Generator_PoolID(that.Generator_PoolID),
	Generator_SlotID(that.Generator_SlotID)
{
	uint32 i;

	// Allocate arrays (we dont' copy array contents because those are mesh-specific and will be
	// filled when the state is set anyway)
	if (PoolCount) Pools = W3DNEWARRAY LogicalDecalPoolClass [PoolCount];
	for (i = 0; i < PoolCount; i++) {
		Pools[i].Initialize(that.Pools[i].Size);
	}
}

MultiFixedPoolDecalSystemClass::~MultiFixedPoolDecalSystemClass(void)
{
	delete [] Pools;
	Pools = nullptr;
}

// This clears the slot in addition to locking the generator, thus preventing any decal id
// collisions (since any decal previously in that slot will have the same id as the new one).
DecalGeneratorClass * MultiFixedPoolDecalSystemClass::Lock_Decal_Generator(void)
{
	Clear_Decal_Slot(Generator_PoolID, Generator_SlotID);
	return DecalSystemClass::Lock_Decal_Generator();
}

// This will register the decal in the system in the appropriate pool and slot (determined by
// the generator's pool and slot ids), removing any decal which may have been there before.
void MultiFixedPoolDecalSystemClass::Unlock_Decal_Generator(DecalGeneratorClass * generator)
{
	find_logical_decal(generator->Get_Decal_ID()).Set(generator);
	DecalSystemClass::Unlock_Decal_Generator(generator);
}

// This notifies the system that a mesh which has decals on it was destroyed - therefore we
// need to remove the mesh from our list to avoid dangling pointers.
void MultiFixedPoolDecalSystemClass::Decal_Mesh_Destroyed(uint32 decal_id,DecalMeshClass * mesh)
{
	// We must remove this mesh from all lists where it is present. The method is: for each
	// decal id present in the decal mesh, find the logical decal and remove the decal mesh from
	// its list.
	assert(mesh);
	MeshClass *parent = mesh->Peek_Parent();
	find_logical_decal(decal_id).MeshList.Remove(parent);
}

// Not part of the DecalSystemClass interface - this function removes any decal currently in
// the given slot in the given pool.
void MultiFixedPoolDecalSystemClass::Clear_Decal_Slot(uint32 pool_id, uint32 slot_id)
{
	find_logical_decal(pool_id, slot_id).Clear(encode_decal_id(pool_id, slot_id));
}

// This one removes all decals in a given pool.
void MultiFixedPoolDecalSystemClass::Clear_Pool(uint32 pool_id)
{
	LogicalDecalPoolClass & pool = Pools[pool_id];
	uint32 pool_size = pool.Size;
	for (uint32 slot_id = 0; slot_id < pool_size; slot_id++) {
		pool.Array[slot_id].Clear(encode_decal_id(pool_id, slot_id));
	}
}

// And this one removes all decals in the system.
void MultiFixedPoolDecalSystemClass::Clear_All_Decals(void)
{
	for (uint32 pool_id = 0; pool_id < PoolCount; pool_id++) {
		LogicalDecalPoolClass & pool = Pools[pool_id];
		uint32 pool_size = pool.Size;
		for (uint32 slot_id = 0; slot_id < pool_size; slot_id++) {
			pool.Array[slot_id].Clear(encode_decal_id(pool_id, slot_id));
		}
	}
}

// Get a reference to the logical decal at the given pool and slot id (performs range checking)
MultiFixedPoolDecalSystemClass::LogicalDecalClass & MultiFixedPoolDecalSystemClass::find_logical_decal(uint32 pool_id, uint32 slot_id)
{
	assert(pool_id < PoolCount);
	pool_id = MIN(pool_id, PoolCount);
	LogicalDecalPoolClass & pool = Pools[pool_id];
	assert(slot_id < pool.Size);
	slot_id = MIN(slot_id, pool.Size);
	return pool.Array[slot_id];
}

// Get a reference to the logical decal with the given decal id
MultiFixedPoolDecalSystemClass::LogicalDecalClass & MultiFixedPoolDecalSystemClass::find_logical_decal(uint32 decal_id)
{
	uint32 pool_id, slot_id;
	decode_decal_id(decal_id, pool_id, slot_id);
	return find_logical_decal(pool_id, slot_id);
}


/*
** MultiFixedPoolDecalSystemClass::LogicalDecalClass implementation
*/

MultiFixedPoolDecalSystemClass::LogicalDecalClass::LogicalDecalClass(void)
{
}

MultiFixedPoolDecalSystemClass::LogicalDecalClass::~LogicalDecalClass(void)
{
	// We don't need to do anything here since the mesh list will get removed automatically and
	// the decals themselves don't neccessarily need to be removed because the logical decal is.
	// (we don't hold references to the meshes in the mesh list)
}

// Sets the logical decal to the one specified by the given generator (clearing any existing
// decal information)
void MultiFixedPoolDecalSystemClass::LogicalDecalClass::Set(DecalGeneratorClass * generator)
{
	Clear(generator->Get_Decal_ID());

	/*
	** Copy the generators' mesh list into our mesh list
	*/
	NonRefRenderObjListIterator gen_mesh_it(&(generator->Get_Mesh_List()));
	for (gen_mesh_it.First(); !gen_mesh_it.Is_Done(); gen_mesh_it.Next()) {
		MeshList.Add(gen_mesh_it.Get_Obj());
	}
}

// Just clears any existing logical decal information, leaving the decal empty.
void MultiFixedPoolDecalSystemClass::LogicalDecalClass::Clear(uint32 decal_id)
{
	// Remove the decal with this ID from all meshes where it appears
	NonRefRenderObjListIterator it(&MeshList);
	for (it.First(); !it.Is_Done(); it.Next()) {
		it.Get_Obj()->Delete_Decal(decal_id);
	}

	// Delete list
	while (!MeshList.Is_Empty()) {
		MeshList.Remove_Head();
	}
}


/*
** MultiFixedPoolDecalSystemClass::LogicalDecalPoolClass implementation
*/

MultiFixedPoolDecalSystemClass::LogicalDecalPoolClass::LogicalDecalPoolClass(void) :
	Array(nullptr),
	Size(0)
{
}

MultiFixedPoolDecalSystemClass::LogicalDecalPoolClass::~LogicalDecalPoolClass(void)
{
	delete [] Array;
	Array = nullptr;
}

void MultiFixedPoolDecalSystemClass::LogicalDecalPoolClass::Initialize(uint32 size)
{
	delete [] Array;
	Array = nullptr;

	Size = size;
	assert(Size);
	Array = W3DNEWARRAY LogicalDecalClass[Size];
}
