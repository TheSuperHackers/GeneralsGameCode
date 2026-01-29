/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /VSS_Sync/ww3d2/hlod.cpp                                    $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HLodLoaderClass::Load_W3D -- Loads an HlodDef from a W3D file                             *
 *   HLodPrototypeClass::Create -- Creates an HLod from an HLodDef                             *
 *   HLodDefClass::HLodDefClass -- Constructor                                                 *
 *   HLodDefClass::HLodDefClass -- Copy Constructor                                            *
 *   HLodDefClass::~HLodDefClass -- Destructor                                                 *
 *   HLodDefClass::Free -- releases all resources being used                                   *
 *   HLodDefClass::Initialize -- init this def from an HLod                                    *
 *   HLodDefClass::Save -- save this HLodDef                                                   *
 *   HLodDefClass::Save_Header -- writes the HLodDef header                                    *
 *   HLodDefClass::Save_Lod_Array -- Saves the lod array                                       *
 *   HLodDefClass::Save_Aggregate_Array -- Save the array of aggregate models                  *
 *   HLodDefClass::Load_W3D -- Loads this HLodDef from a W3d File                              *
 *   HLodDefClass::read_header -- loads the HLodDef header from a W3d file                     *
 *   HLodDefClass::read_proxy_array -- load the proxy names                                    *
 *   HLodDefClass::SubObjectArrayClass::SubObjectArrayClass -- LodArray constructor            *
 *   HLodDefClass::SubObjectArrayClass::~SubObjectArrayClass -- LodArray destructor            *
 *   HLodDefClass::SubObjectArrayClass::Save_W3D -- saves a w3d file for this HLodDef          *
 *   HLodDefClass::SubObjectArrayClass::Load_W3D -- LodArray load function                     *
 *   HLodDefClass::SubObjectArrayClass::Reset -- release the contents of this array            *
 *   HLodClass::HLodClass -- Constructor                                                       *
 *   HLodClass::HLodClass -- copy constructor                                                  *
 *   HLodClass::HLodClass -- Constructor                                                       *
 *   HLodClass::HLodClass -- Constructs an HLod from an HLodDef                                *
 *   HLodClass::HLodClass -- Constructs an HLod from an HModelDef                              *
 *   HLodClass::operator -- assignment operator                                                *
 *   HLodClass::~HLodClass -- Destructor                                                       *
 *   HLodClass::Free -- releases all resources                                                 *
 *   HLodClass::Clone -- virtual copy constructor                                              *
 *   HLodClass::Set_Max_Screen_Size -- Set max-screen-size for an LOD                          *
 *   HLodClass::Get_Max_Screen_Size -- get max-screen-size for an LOD                          *
 *   HLodClass::Get_Lod_Count -- returns number of levels of detail                            *
 *   HLodClass::Get_Lod_Model_Count -- number of sub-objs in a given level of detail           *
 *   HLodClass::Peek_Lod_Model -- returns pointer to a model in one of the LODs                *
 *   HLodClass::Get_Lod_Model -- returns a pointer to a model in one of the LODs               *
 *   HLodClass::Get_Lod_Model_Bone -- returns the bone index of a model                        *
 *   HLodClass::Get_Additional_Model_Count -- number of additional sub-objs                    *
 *   HLodClass::Peek_Additional_Model -- returns pointer to an additional model                *
 *   HLodClass::Get_Additional_Model -- returns pointer to an additional model                 *
 *   HLodClass::Get_Additional_Model_Bone -- returns the bone index of an additional model     *
 *   HLodClass::Is_NULL_Lod_Included -- does this HLod have nullptr as its lowest LOD             *
 *   HLodClass::Include_NULL_Lod -- Add nullptr as the lowest LOD                                 *
 *   HLodClass::Get_Num_Polys -- returns polycount of the current LOD                          *
 *   HLodClass::Render -- render this HLod                                                     *
 *   HLodClass::Special_Render -- Special_Render for HLod                                      *
 *   HLodClass::Set_Transform -- Sets the transform                                            *
 *   HLodClass::Set_Position -- Sets the position                                              *
 *   HLodClass::Notify_Added -- callback notifies subobjs that they were added                 *
 *   HLodClass::Notify_Removed -- notifies subobjs that they were removed                      *
 *   HLodClass::Get_Num_Sub_Objects -- returns total number of sub-objects                     *
 *   HLodClass::Get_Sub_Object -- returns a pointer to specified sub-object                    *
 *   HLodClass::Add_Sub_Object -- add a sub-object to this HLod                                *
 *   HLodClass::Remove_Sub_Object -- remove a sub-object from this HLod                        *
 *   HLodClass::Get_Num_Sub_Objects_On_Bone -- returns the number of objects on the given bone *
 *   HLodClass::Get_Sub_Object_On_Bone -- returns obj on the given bone                        *
 *   HLodClass::Get_Sub_Object_Bone_Index -- returns bone index of given object                *
 *   HLodClass::Add_Sub_Object_To_Bone -- adds a sub-object to a bone                          *
 *   HLodClass::Set_Animation -- set animation state to the base-pose                          *
 *   HLodClass::Set_Animation -- set animation state to an animation frame                     *
 *   HLodClass::Set_Animation -- set animation state to a blend of two animations              *
 *   HLodClass::Set_Animation -- set animation state to a combination of anims                 *
 *   HLodClass::Cast_Ray -- cast a ray against this HLod                                       *
 *   HLodClass::Cast_AABox -- Cast a swept AABox against this HLod                             *
 *   HLodClass::Cast_OBBox -- Cast a swept OBBox against this HLod                             *
 *   HLodClass::Intersect_AABox -- Intersect an AABox with this HLod                           *
 *   HLodClass::Intersect_OBBox -- Intersect an OBBox with this HLod                           *
 *   HLodClass::Prepare_LOD -- Prepare for LOD processing                                      *
 *   HLodClass::Recalculate_Static_LOD_Factors -- compute lod factors                          *
 *   HLodClass::Increment_LOD -- move to next lod                                              *
 *   HLodClass::Decrement_LOD -- move to previous lod                                          *
 *   HLodClass::Get_Cost -- returns the cost of this LOD                                       *
 *   HLodClass::Get_Value -- returns the value of this LOD                                     *
 *   HLodClass::Get_Post_Increment_Value -- returns the post increment value                   *
 *   HLodClass::Set_LOD_Level -- set the current lod level                                     *
 *   HLodClass::Get_LOD_Level -- returns the current LOD level                                 *
 *   HLodClass::Get_LOD_Count -- returns the number of levels of detail                        *
 *   HLodClass::Calculate_Cost_Value_Arrays -- computes the cost-value arrays                  *
 *   HLodClass::Get_Current_LOD -- returns a render object which represents the current LOD    *
 *   HLodClass::Set_Texture_Reduction_Factor -- resizeable texture support                     *
 *   HLodClass::Scale -- scale this HLod model                                                 *
 *   HLodClass::Get_Num_Snap_Points -- returns the number of snap points in this model         *
 *   HLodClass::Get_Snap_Point -- returns specified snap-point                                 *
 *   HLodClass::Update_Sub_Object_Transforms -- updates transforms of all sub-objects          *
 *   HLodClass::Update_Obj_Space_Bounding_Volumes -- update object-space bounding volumes      *
 *   HLodClass::Add_Lod_Model -- adds a model to one of the lods                               *
 *   HLodClass::Create_Decal -- create a decal on this HLod                                    *
 *   HLodClass::Delete_Decal -- remove a decal from this HLod                                  *
 *   HLodClass::Set_HTree -- replace the hierarchy tree                                        *
 *   HLodClass::Get_Proxy_Count -- Returns the number of proxy objects                         *
 *   HLodClass::Get_Proxy -- returns the information for the i'th proxy                        *
 * HLodClass::Set_Hidden -- Propogates the hidden bit to particle emitters.                    *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#include "hlod.h"
#include "assetmgr.h"
#include "hmdldef.h"
#include "w3derr.h"
#include "chunkio.h"
#include "predlod.h"
#include "rinfo.h"
#include <win.h>
#include "sphere.h"
#include "boxrobj.h"


/*
** Loader Instance
*/
HLodLoaderClass			_HLodLoader;


/**
** ProxyRecordClass
** This is a structure that contains the data describing a single "proxy" object.  These
** are used for application purposes and simply provide a way for the assets to associate
** a string with a bone index.
*/
class ProxyRecordClass
{
public:
	ProxyRecordClass(void) : BoneIndex(0)
	{
		memset(Name,0,sizeof(Name));
	}

	bool					operator == (const ProxyRecordClass & that) { return false; }
	bool					operator != (const ProxyRecordClass & that) { return !(*this == that); }

	void					Init(const W3dHLodSubObjectStruct & w3d_data)
	{
		BoneIndex = w3d_data.BoneIndex;
		strlcpy(Name,w3d_data.Name,sizeof(Name));
	}

	int					Get_Bone_Index(void)		{ return BoneIndex; }
	const char *		Get_Name(void)				{ return Name; }

protected:

	int		BoneIndex;
	char		Name[2*W3D_NAME_LEN];

};

/**
** ProxyArrayClass
** This is a ref-counted list of proxy objects.  It is generated whenever an HLODdef contains
** proxies.  Each instantiated HLOD simply add-refs a pointer to the single list.
*/
class ProxyArrayClass : public W3DMPO, public VectorClass<ProxyRecordClass>, public RefCountClass
{
	W3DMPO_GLUE(ProxyArrayClass)
public:
	ProxyArrayClass(int size) : VectorClass<ProxyRecordClass>(size)
	{
	}
};



/*
** The HLod Loader Implementation
*/

/***********************************************************************************************
 * HLodLoaderClass::Load_W3D -- Loads an HlodDef from a W3D file                               *
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
PrototypeClass *HLodLoaderClass::Load_W3D( ChunkLoadClass &cload )
{
	HLodDefClass * def = W3DNEW HLodDefClass;

	if (def == nullptr)
	{
		return nullptr;
	}

	if (def->Load_W3D(cload) != WW3D_ERROR_OK) {
		// load failed, delete the model and return an error
		delete def;
		return nullptr;
	} else {
		// ok, accept this model!
		HLodPrototypeClass *proto = W3DNEW HLodPrototypeClass(def);
		return proto;
	}
	return nullptr;
}


/*
** HLod Prototype Implementation
*/

/***********************************************************************************************
 * HLodPrototypeClass::Create -- Creates an HLod from an HLodDef                               *
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
RenderObjClass * HLodPrototypeClass::Create(void)
{
	HLodClass * hlod = NEW_REF( HLodClass , ( *Definition ) );
	return hlod;
}


/*
** HLodDef Implementation
*/

/***********************************************************************************************
 * HLodDefClass::HLodDefClass -- Constructor                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
HLodDefClass::HLodDefClass(void) :
	Name(nullptr),
	HierarchyTreeName(nullptr),
	LodCount(0),
	Lod(nullptr),
	ProxyArray(nullptr)
{
}


/***********************************************************************************************
 * HLodDefClass::HLodDefClass -- Copy Constructor                                              *
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
HLodDefClass::HLodDefClass(HLodClass &src_lod) :
	Name(nullptr),
	HierarchyTreeName(nullptr),
	LodCount(0),
	Lod(nullptr),
	ProxyArray(nullptr)
{
	Initialize (src_lod);
	return ;
}


/***********************************************************************************************
 * HLodDefClass::~HLodDefClass -- Destructor                                                   *
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
HLodDefClass::~HLodDefClass(void)
{
	Free ();
	return ;
}


/***********************************************************************************************
 * HLodDefClass::Free -- releases all resources being used                                     *
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
void HLodDefClass::Free(void)
{
	::free(Name);
	Name = nullptr;

	::free(HierarchyTreeName);
	HierarchyTreeName = nullptr;

	delete[] Lod;
	Lod = nullptr;
	LodCount = 0;

	REF_PTR_RELEASE(ProxyArray);

	return ;
}


/***********************************************************************************************
 * HLodDefClass::Initialize -- init this def from an HLod                                      *
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
void HLodDefClass::Initialize(HLodClass &src_lod)
{
	// Start with a fresh set of data
	Free ();

	// Copy the name and hierarcy name from the source object
	Name = ::strdup (src_lod.Get_Name ());
	const HTreeClass *phtree = src_lod.Get_HTree ();
	if (phtree != nullptr) {
		HierarchyTreeName = ::strdup (phtree->Get_Name ());
	}

	// Determine the number of LODs in the src object
	LodCount = src_lod.Get_LOD_Count ();
	WWASSERT (LodCount > 0);
	if (LodCount > 0) {

		// Allocate an array large enough to hold all the LODs and
		// loop through each LOD.
		Lod = W3DNEWARRAY SubObjectArrayClass[LodCount];
		for (int index = 0; index < LodCount; index ++) {

			// Fill in the maximum screen size for this LOD
			Lod[index].MaxScreenSize = src_lod.Get_Max_Screen_Size (index);
			Lod[index].ModelCount = src_lod.Get_Lod_Model_Count (index);

			// Loop through all the models that compose this LOD and generate a
			// list of model's and the bones they live on
			char **model_names = W3DNEWARRAY char *[Lod[index].ModelCount];
			int *bone_indicies = W3DNEWARRAY int[Lod[index].ModelCount];
			for (int model_index = 0; model_index < Lod[index].ModelCount; model_index ++) {

				// Record information about this model (if possible)
				RenderObjClass *prender_obj = src_lod.Peek_Lod_Model (index, model_index);
				if (prender_obj != nullptr) {
					model_names[model_index] = ::strdup (prender_obj->Get_Name ());
					bone_indicies[model_index] = src_lod.Get_Lod_Model_Bone (index, model_index);
				} else {
					model_names[model_index] = nullptr;
					bone_indicies[model_index] = 0;
				}
			}

			// Pass these arrays of information onto our internal data
			Lod[index].ModelName = model_names;
			Lod[index].BoneIndex = bone_indicies;
		}
	}

	return;
}


/***********************************************************************************************
 * HLodDefClass::Save -- save this HLodDef                                                     *
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
WW3DErrorType HLodDefClass::Save(ChunkSaveClass & csave)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_SAVE_FAILED;

	// Begin a chunk that identifies an aggregate
	if (csave.Begin_Chunk (W3D_CHUNK_HLOD) == TRUE) {

		// Attempt to save the different sections of the aggregate definition
		if ((Save_Header (csave) == WW3D_ERROR_OK) &&
			 (Save_Lod_Array (csave) == WW3D_ERROR_OK)) {

			// Success!
			ret_val = WW3D_ERROR_OK;
		}

		// Close the aggregate chunk
		csave.End_Chunk ();
	}

	// Return the WW3DErrorType return code
	return ret_val;
}


/***********************************************************************************************
 * HLodDefClass::Save_Header -- writes the HLodDef header                                      *
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
WW3DErrorType HLodDefClass::Save_Header(ChunkSaveClass &csave)
{
	// Assume error
	WW3DErrorType ret_val = WW3D_ERROR_SAVE_FAILED;

	// Begin a chunk that identifies the aggregate
	if (csave.Begin_Chunk (W3D_CHUNK_HLOD_HEADER) == TRUE) {

		// Fill the header structure
		W3dHLodHeaderStruct header = { 0 };
		header.Version = W3D_CURRENT_HLOD_VERSION;
		header.LodCount = LodCount;

		// Copy the name to the header
		::lstrcpyn (header.Name, Name, sizeof (header.Name));
		header.Name[sizeof (header.Name) - 1] = 0;

		// Copy the hierarchy tree name to the header
		::lstrcpyn (header.HierarchyName, HierarchyTreeName, sizeof (header.HierarchyName));
		header.HierarchyName[sizeof (header.HierarchyName) - 1] = 0;

		// Write the header out to the chunk
		if (csave.Write (&header, sizeof (header)) == sizeof (header)) {

			// Success!
			ret_val = WW3D_ERROR_OK;
		}

		// End the header chunk
		csave.End_Chunk ();
	}

	// Return the WW3DErrorType return code
	return ret_val;
}


/***********************************************************************************************
 * HLodDefClass::Save_Lod_Array -- Saves the lod array                                         *
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
WW3DErrorType HLodDefClass::Save_Lod_Array(ChunkSaveClass &csave)
{
	// Loop through all the LODs and save their model array to the chunk
	bool success = true;
	for (int lod_index = 0;
		  (lod_index < LodCount) && success;
		  lod_index ++) {
		success = Lod[lod_index].Save_W3D (csave);
	}

	// Return the WW3DErrorType return code
	return success ? WW3D_ERROR_OK : WW3D_ERROR_SAVE_FAILED;
}


/***********************************************************************************************
 * HLodDefClass::Save_Aggregate_Array -- Save the array of aggregate models                    *
 *                                                                                             *
 * aggregate models are ones that are attached as "additional" models.                         *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/25/2000 gth : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType HLodDefClass::Save_Aggregate_Array(ChunkSaveClass & csave)
{
	if (Aggregates.ModelCount > 0) {
		csave.Begin_Chunk(W3D_CHUNK_HLOD_AGGREGATE_ARRAY);
		Aggregates.Save_W3D(csave);
		csave.End_Chunk();
	}
	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * HLodDefClass::Load_W3D -- Loads this HLodDef from a W3d File                                *
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
WW3DErrorType HLodDefClass::Load_W3D(ChunkLoadClass & cload)
{
	/*
	** First make sure we release any memory in use
	*/
	Free();

	if (read_header(cload) == FALSE) {
	  return WW3D_ERROR_LOAD_FAILED;
	}

	/*
	**	Loop through all the LODs and read the info from its chunk
	*/
	for (int iLOD = 0; iLOD < LodCount; iLOD ++) {

		/*
		**	Open the next chunk, it should be a LOD struct
		*/
		if (!cload.Open_Chunk()) return WW3D_ERROR_LOAD_FAILED;

		if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_LOD_ARRAY) {
			// ERROR: Expected LOD struct!
			return WW3D_ERROR_LOAD_FAILED;
		}

		Lod[iLOD].Load_W3D(cload);

		// Close-out the chunk
		cload.Close_Chunk();
	}

	/*
	** Parse the rest of the chunks
	*/
	while (cload.Open_Chunk()) {
		switch(cload.Cur_Chunk_ID())
		{
			case W3D_CHUNK_HLOD_AGGREGATE_ARRAY:
				Aggregates.Load_W3D(cload);
				break;
			case W3D_CHUNK_HLOD_PROXY_ARRAY:
				read_proxy_array(cload);
				break;
		}
		cload.Close_Chunk();
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * HLodDefClass::read_header -- loads the HLodDef header from a W3d file                       *
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
bool HLodDefClass::read_header(ChunkLoadClass & cload)
{
	/*
	**	Open the first chunk, it should be the LOD header
	*/
	if (!cload.Open_Chunk()) return false;

	if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_HEADER) {
		// ERROR: Expected HLOD Header!
		return false;
	}

	W3dHLodHeaderStruct header;
	if (cload.Read(&header,sizeof(header)) != sizeof(header)) {
		return false;
	}
	cload.Close_Chunk();

	// Copy the name into our internal variable
	Name = ::_strdup(header.Name);
	HierarchyTreeName = ::strdup(header.HierarchyName);
	LodCount = header.LodCount;
	Lod = W3DNEWARRAY SubObjectArrayClass[LodCount];
	return true;
}


/***********************************************************************************************
 * HLodDefClass::read_proxy_array -- load the proxy names                                      *
 *                                                                                             *
 *    This function is coded separately from SubObjectArrayClass::Load because we are going    *
 *    to store the proxies in a shared data structure.  Because all of the proxy data is       *
 *    constant, each instanced HLOD can just add-ref a pointer to its proxy array.             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/27/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool HLodDefClass::read_proxy_array(ChunkLoadClass & cload)
{
	REF_PTR_RELEASE(ProxyArray);

	/*
	** Open the first chunk, it should be a Lod Array Header
	*/
	if (!cload.Open_Chunk()) return false;
	if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER) return false;

	W3dHLodArrayHeaderStruct header;
	if (cload.Read(&header,sizeof(header)) != sizeof(header)) return false;

	if (!cload.Close_Chunk()) return false;

	ProxyArray = NEW_REF(ProxyArrayClass,(header.ModelCount));

	/*
	** Read each sub object definition
	*/
	for (int imodel=0; imodel<ProxyArray->Length(); ++imodel) {
		if (!cload.Open_Chunk()) return false;
		if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT) return false;

		W3dHLodSubObjectStruct subobjdef;
		if (cload.Read(&subobjdef,sizeof(subobjdef)) != sizeof(subobjdef)) return false;
		if (!cload.Close_Chunk()) return false;

		(*ProxyArray)[imodel].Init(subobjdef);
	}
	return true;
}


/***********************************************************************************************
 * HLodDefClass::SubObjectArrayClass::SubObjectArrayClass -- LodArray constructor              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
HLodDefClass::SubObjectArrayClass::SubObjectArrayClass(void) :
	MaxScreenSize(NO_MAX_SCREEN_SIZE),
	ModelCount(0),
	ModelName(nullptr),
	BoneIndex(nullptr)
{
}


/***********************************************************************************************
 * HLodDefClass::SubObjectArrayClass::~SubObjectArrayClass -- LodArray destructor              *
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
HLodDefClass::SubObjectArrayClass::~SubObjectArrayClass(void)
{
	Reset();
}


/***********************************************************************************************
 * HLodDefClass::SubObjectArrayClass::Reset -- release the contents of this array              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/25/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void HLodDefClass::SubObjectArrayClass::Reset(void)
{
	MaxScreenSize = NO_MAX_SCREEN_SIZE;

	if (ModelName != nullptr) {
		for (int imodel=0; imodel<ModelCount;imodel++) {
			free(ModelName[imodel]);
		}
		delete[] ModelName;
		ModelName = nullptr;
	}

	delete[] BoneIndex;
	BoneIndex = nullptr;

	ModelCount = 0;
}


/***********************************************************************************************
 * HLodDefClass::SubObjectArrayClass::Load_W3D -- LodArray load function                       *
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
bool HLodDefClass::SubObjectArrayClass::Load_W3D(ChunkLoadClass & cload)
{
	/*
	** Open the first chunk, it should be a Lod Array Header
	*/
	if (!cload.Open_Chunk()) return false;
	if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER) return false;

	W3dHLodArrayHeaderStruct header;
	if (cload.Read(&header,sizeof(header)) != sizeof(header)) return false;

	if (!cload.Close_Chunk()) return false;

	ModelCount = header.ModelCount;
	MaxScreenSize = header.MaxScreenSize;

	DEBUG_ASSERTCRASH(ModelName == nullptr, ("HLodDefClass::SubObjectArrayClass::Load_W3D: Leaking ModelName"));
	DEBUG_ASSERTCRASH(BoneIndex == nullptr, ("HLodDefClass::SubObjectArrayClass::Load_W3D: Leaking BoneIndex"));
	ModelName = W3DNEWARRAY char * [ModelCount];
	BoneIndex = W3DNEWARRAY int [ModelCount];

	/*
	** Read each sub object definition
	*/
	for (int imodel=0; imodel<ModelCount; ++imodel) {
		if (!cload.Open_Chunk()) return false;
		if (cload.Cur_Chunk_ID() != W3D_CHUNK_HLOD_SUB_OBJECT) return false;

		W3dHLodSubObjectStruct subobjdef;
		if (cload.Read(&subobjdef,sizeof(subobjdef)) != sizeof(subobjdef)) return false;

		if (!cload.Close_Chunk()) return false;

		ModelName[imodel] = strdup(subobjdef.Name);
		BoneIndex[imodel] = subobjdef.BoneIndex;
	}
	return true;
}

/***********************************************************************************************
 * HLodDefClass::SubObjectArrayClass::Save_W3D -- saves a w3d file for this HLodDef            *
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
bool HLodDefClass::SubObjectArrayClass::Save_W3D(ChunkSaveClass &csave)
{
	// Assume error
	bool ret_val = false;

	// Begin a chunk that identifies the LOD array
	if (csave.Begin_Chunk (W3D_CHUNK_HLOD_LOD_ARRAY) == TRUE) {

		// Begin a chunk that identifies the LOD header
		if (csave.Begin_Chunk (W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER) == TRUE) {

			W3dHLodArrayHeaderStruct header = { 0 };
			header.ModelCount = ModelCount;
			header.MaxScreenSize = MaxScreenSize;

			// Write the LOD header structure out to the chunk
			ret_val = (csave.Write (&header, sizeof (header)) == sizeof (header));

			// End the header chunk
			csave.End_Chunk ();
		}

		if (ret_val) {

			// Write all of this LOD's models to the file
			for (int index = 0;
				  (index < ModelCount) && ret_val;
				  index ++) {

				// Save this LOD sub-obj to the chunk
				ret_val &= (csave.Begin_Chunk (W3D_CHUNK_HLOD_SUB_OBJECT) == TRUE);
				if (ret_val) {

					W3dHLodSubObjectStruct info = { 0 };
					info.BoneIndex = BoneIndex[index];

					// Copy this model name into the structure
					::lstrcpyn (info.Name, ModelName[index], sizeof (info.Name));
					info.Name[sizeof (info.Name) - 1] = 0;

					// Write the LOD sub-obj structure out to the chunk
					ret_val &= (csave.Write (&info, sizeof (info)) == sizeof (info));

					// End the sub-obj chunk
					csave.End_Chunk ();
				}
			}
		}

		// End the HLOD-Array chunk
		csave.End_Chunk ();
	}

	// Return the true/false result code
	return ret_val;
}




/***********************************************************************************************
 * HLodClass::HLodClass -- Constructor                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
HLodClass::HLodClass(void) :
	Animatable3DObjClass(nullptr),
	LodCount(0),
	CurLod(0),
	Lod(nullptr),
	BoundingBoxIndex(-1),
	Cost(nullptr),
	Value(nullptr),
	AdditionalModels(),
	SnapPoints(nullptr),
	ProxyArray(nullptr),
	LODBias(1.0f)
{
}


/***********************************************************************************************
 * HLodClass::HLodClass -- copy constructor                                                    *
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
HLodClass::HLodClass(const HLodClass & src) :
	Animatable3DObjClass(src),
	LodCount(0),
	CurLod(0),
	Lod(nullptr),
	BoundingBoxIndex(-1),
	Cost(nullptr),
	Value(nullptr),
	AdditionalModels(),
	SnapPoints(nullptr),
	ProxyArray(nullptr),
	LODBias(1.0f)
{
	*this = src;
}


/***********************************************************************************************
 * HLodClass::HLodClass -- Constructor                                                         *
 *                                                                                             *
 * Creates an HLodClass from an array of render objects.  Each render object is assumed to     *
 * be an alternate level of detail.                                                            *
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
HLodClass::HLodClass(const char * name,RenderObjClass ** lods,int count) :
	Animatable3DObjClass(nullptr),
	LodCount(0),
	CurLod(0),
	Lod(nullptr),
	BoundingBoxIndex(-1),
	Cost(nullptr),
	Value(nullptr),
	AdditionalModels(),
	SnapPoints(nullptr),
	ProxyArray(nullptr),
	LODBias(1.0f)
{
	// enforce parameters
	WWASSERT(name != nullptr);
	WWASSERT(lods != nullptr);
	WWASSERT((count > 0) && (count < 256));

	// Set the name
	Set_Name(name);

	LodCount = count;
	WWASSERT(LodCount >= 1);
	Lod = W3DNEWARRAY ModelArrayClass[LodCount];
	WWASSERT(Lod);
	Cost = W3DNEWARRAY float[LodCount];
	WWASSERT(Cost);
	// Value has LodCount + 1 entries so PostIncrementValue can always use
	// Value[CurLod + 1] (the last entry wil be AT_MAX_LOD).
	Value = W3DNEWARRAY float[LodCount + 1];
	WWASSERT(Value);

	// Create our HTree from the highest LOD if it is an HModel
	// Otherwise, create a single node tree
	const HTreeClass * tree = lods[count-1]->Get_HTree();
	if (tree != nullptr) {
		HTree = W3DNEW HTreeClass(*tree);
	} else {
		HTree = W3DNEW HTreeClass();
		HTree->Init_Default();
	}

	// Ok, now suck the sub-objects out of each LOD model and place them into this HLOD.
	for (int lod_index=0; lod_index < LodCount; lod_index++) {
		RenderObjClass * lod_obj = lods[lod_index];
		WWASSERT(lod_obj);

		if (	(lod_obj->Class_ID() == RenderObjClass::CLASSID_HMODEL) ||
				(lod_obj->Class_ID() == RenderObjClass::CLASSID_HLOD) ||
				(lod_obj->Get_Num_Sub_Objects() > 1) ) {

			// here we insert all sub-objects of this render object into the current LOD array
			while (lod_obj->Get_Num_Sub_Objects() > 0) {

				RenderObjClass * sub_obj = lod_obj->Get_Sub_Object(0);
				int boneindex = lod_obj->Get_Sub_Object_Bone_Index(sub_obj);
				lod_obj->Remove_Sub_Object(sub_obj);

				Add_Lod_Model(lod_index,sub_obj,boneindex);

				sub_obj->Release_Ref();
			}

		} else {

			// just insert the render object as the sole member of the current LOD array.  This
			// case happens if this level of detail is a simple object such as a mesh or NullRenderObj
			Add_Lod_Model(lod_index,lod_obj,0);
		}
	}

	Recalculate_Static_LOD_Factors();

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its cost and value arrays
	// according to a screen area of 1 pixel.
	int minlod = Calculate_Cost_Value_Arrays(1.0f, Value, Cost);

	// Ensure lod is no less than minimum allowed
	if (CurLod < minlod) Set_LOD_Level(minlod);

	// Flag our sub-objects as having dirty transforms
	Set_Sub_Object_Transforms_Dirty(true);

	// Normal render object processing whenever sub-objects are added or removed:
	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
}


/***********************************************************************************************
 * HLodClass::HLodClass -- Constructs an HLod from an HLodDef                                  *
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
HLodClass::HLodClass(const HLodDefClass & def) :
	Animatable3DObjClass(def.HierarchyTreeName),
	LodCount(0),
	CurLod(0),
	Lod(nullptr),
	BoundingBoxIndex(-1),
	Cost(nullptr),
	Value(nullptr),
	AdditionalModels(),
	SnapPoints(nullptr),
	ProxyArray(nullptr),
	LODBias(1.0f)
{
	// Set the name
	Set_Name(def.Get_Name());


	// Number of LODs comes from the distlod
	LodCount = def.LodCount;
	WWASSERT(LodCount >= 1);
	Lod = W3DNEWARRAY ModelArrayClass[LodCount];
	WWASSERT(Lod);
	Cost = W3DNEWARRAY float[LodCount];
	WWASSERT(Cost);
	// Value has LodCount + 1 entries so PostIncrementValue can always use
	// Value[CurLod + 1] (the last entry wil be AT_MAX_LOD).
	Value = W3DNEWARRAY float[LodCount + 1];
	WWASSERT(Value);

	// Add Models to the ModelArrays
	for (int ilod=0; ilod < def.LodCount; ilod++) {

		Lod[ilod].MaxScreenSize = def.Lod[ilod].MaxScreenSize;

		for (int imodel=0; imodel < def.Lod[ilod].ModelCount; imodel++) {

			RenderObjClass * robj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(def.Lod[ilod].ModelName[imodel]);
			int boneindex = def.Lod[ilod].BoneIndex[imodel];
			if (robj != nullptr) {
				Add_Lod_Model(ilod,robj,boneindex);
				robj->Release_Ref();
			}
		}
	}

	Recalculate_Static_LOD_Factors();

	// Add aggregates to this model
	for (int iagg=0; iagg<def.Aggregates.ModelCount; iagg++) {
		RenderObjClass * robj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(def.Aggregates.ModelName[iagg]);
		int boneindex = def.Aggregates.BoneIndex[iagg];
		if (robj != nullptr) {
			Add_Sub_Object_To_Bone(robj,boneindex);
			robj->Release_Ref();
		}
	}

	// Add a reference to the proxy array
	REF_PTR_SET(ProxyArray,def.ProxyArray);

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its cost and value arrays
	// according to a screen area of 1 pixel.
	int minlod = Calculate_Cost_Value_Arrays(1.0f, Value, Cost);

	// Ensure lod is no less than minimum allowed
	if (CurLod < minlod) Set_LOD_Level(minlod);

	// Flag our sub-objects as having dirty transforms
	Set_Sub_Object_Transforms_Dirty(true);

	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
	return ;
}


/***********************************************************************************************
 * HLodClass::HLodClass -- Constructs an HLod from an HModelDef                                *
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
HLodClass::HLodClass(const HModelDefClass & def) :
	Animatable3DObjClass(def.BasePoseName),
	LodCount(0),
	CurLod(0),
	Lod(nullptr),
	BoundingBoxIndex(-1),
	Cost(nullptr),
	Value(nullptr),
	AdditionalModels(),
	SnapPoints(nullptr),
	ProxyArray(nullptr),
	LODBias(1.0f)
{
	// Set the name
	Set_Name(def.Get_Name());

	// This is a "simple" HLod, only one LOD
	LodCount = 1;
	Lod = W3DNEWARRAY ModelArrayClass[1];
	WWASSERT(Lod);
	Cost = W3DNEWARRAY float[1];
	WWASSERT(Cost);
	// Value has LodCount + 1 entries so PostIncrementValue can always use
	// Value[CurLod + 1] (the last entry wil be AT_MAX_LOD).
	Value = W3DNEWARRAY float[2];
	WWASSERT(Value);

	// no lod size clamping
	Lod[0].MaxScreenSize = NO_MAX_SCREEN_SIZE;

	// create the sub-objects
	int imodel;
	for (imodel=0; imodel < def.SubObjectCount; ++imodel) {
		RenderObjClass * robj = WW3DAssetManager::Get_Instance()->Create_Render_Obj(def.SubObjects[imodel].RenderObjName);
		if (robj) {
			int boneindex = def.SubObjects[imodel].PivotID;
			Add_Lod_Model(0,robj,boneindex);
			robj->Release_Ref();
		}
	}

	Recalculate_Static_LOD_Factors();

	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its cost and value arrays
	// according to a screen area of 1 pixel.
	int minlod = Calculate_Cost_Value_Arrays(1.0f, Value, Cost);

	// Ensure lod is no less than minimum allowed
	if (CurLod < minlod) Set_LOD_Level(minlod);

	// Flag our sub-objects as having dirty transforms
	Set_Sub_Object_Transforms_Dirty(true);

	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
	return ;
}


/***********************************************************************************************
 * HLodClass::operator -- assignment operator                                                  *
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
HLodClass & HLodClass::operator = (const HLodClass & that)
{
	int lod,model;

	if (this != &that) {
		Free();
		Animatable3DObjClass::operator = (that);
		BoundingBoxIndex = that.BoundingBoxIndex;

		LodCount = that.LodCount;
		WWASSERT(LodCount >= 1);

		Lod = W3DNEWARRAY ModelArrayClass[LodCount];
		WWASSERT(Lod != nullptr);
		Cost = W3DNEWARRAY float[LodCount];
		WWASSERT(Cost);
		// Value has LodCount + 1 entries so PostIncrementValue can always use
		// Value[CurLod + 1] (the last entry wil be AT_MAX_LOD).
		Value = W3DNEWARRAY float[LodCount + 1];
		WWASSERT(Value);

		for (lod=0; lod<LodCount;lod++) {
			Lod[lod].Resize(that.Lod[lod].Count());
			Lod[lod].MaxScreenSize = that.Lod[lod].MaxScreenSize;

			for (model = 0; model < that.Lod[lod].Count(); model++) {

				ModelNodeClass newnode;
				newnode.Model = that.Lod[lod][model].Model->Clone();
				newnode.BoneIndex = that.Lod[lod][model].BoneIndex;
				newnode.Model->Set_Container(this);
				if (Is_In_Scene()) {
					newnode.Model->Notify_Added(Scene);
				}

				Lod[lod].Add(newnode);
			}
		}

		AdditionalModels.Resize(that.AdditionalModels.Count());
		for (model = 0; model < that.AdditionalModels.Count(); model++) {

			ModelNodeClass newnode;
			newnode.Model = that.AdditionalModels[model].Model->Clone();
			newnode.BoneIndex = that.AdditionalModels[model].BoneIndex;
			newnode.Model->Set_Container(this);
			if (Is_In_Scene()) {
				newnode.Model->Notify_Added(Scene);
			}

			AdditionalModels.Add(newnode);
		}

		LODBias = that.LODBias;
	}

	Recalculate_Static_LOD_Factors();


	// So that the object is ready for use after construction, we will
	// complete its initialization by initializing its cost and value arrays
	// according to a screen area of 1 pixel.
	int minlod = Calculate_Cost_Value_Arrays(1.0f, Value, Cost);

	// Ensure lod is no less than minimum allowed
	if (CurLod < minlod) Set_LOD_Level(minlod);

	// Flag our sub-objects as having dirty transforms
	Set_Sub_Object_Transforms_Dirty(true);

	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
	return *this;
}


/***********************************************************************************************
 * HLodClass::~HLodClass -- Destructor                                                         *
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
HLodClass::~HLodClass(void)
{
	Free();
}


/***********************************************************************************************
 * HLodClass::Free -- releases all resources                                                   *
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
void HLodClass::Free(void)
{
	int lod,model;

	for (lod = 0; lod < LodCount; lod++) {
		for (model = 0; model < Lod[lod].Count(); model++) {

			RenderObjClass * robj = Lod[lod][model].Model;
			Lod[lod][model].Model = nullptr;

			WWASSERT(robj);
			robj->Set_Container(nullptr);
			robj->Release_Ref();

		}

		Lod[lod].Delete_All();
	}

	delete[] Lod;
	Lod = nullptr;
	LodCount = 0;

	delete[] Cost;
	Cost = nullptr;

	delete[] Value;
	Value = nullptr;

	for (model = 0; model < AdditionalModels.Count(); model++) {
		RenderObjClass * robj = AdditionalModels[model].Model;
		AdditionalModels[model].Model = nullptr;

		WWASSERT(robj);
		robj->Set_Container(nullptr);
		robj->Release_Ref();
	}
	AdditionalModels.Delete_All();

	REF_PTR_RELEASE(SnapPoints);
	REF_PTR_RELEASE(ProxyArray);
}


/***********************************************************************************************
 * HLodClass::Clone -- virtual copy constructor                                                *
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
RenderObjClass * HLodClass::Clone(void) const
{
	return W3DNEW HLodClass(*this);
}


/***********************************************************************************************
 * HLodClass::Get_Obj_Space_Bounding_Box -- Return the bounding box mesh if we have one.		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/13/00    pds : Created.                                                                 *
 *=============================================================================================*/
void HLodClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	//
	//	Do we have a bounding box mesh?
	//
	int count = Lod[LodCount - 1].Count ();
	if (BoundingBoxIndex >= 0 && BoundingBoxIndex < count) {

		RenderObjClass *mesh = Lod[LodCount - 1][BoundingBoxIndex].Model;
		if (mesh != nullptr && mesh->Class_ID () == RenderObjClass::CLASSID_OBBOX) {

			OBBoxRenderObjClass *obbox_mesh = (OBBoxRenderObjClass *)mesh;

			//
			//	Determine what the box's transform 'should' be this frame.
			// Note:  We do this because some animation types don't update
			// unless they are visible.
			//
			Matrix3D box_tm;
			Simple_Evaluate_Bone (Lod[LodCount - 1][BoundingBoxIndex].BoneIndex, &box_tm);

			//
			//	Convert the OBBox from its coordinate system to the coordinate
			// system of the HLOD.
			//
			Matrix3D world_to_hlod_tm;
			Matrix3D box_to_hlod_tm;

			Get_Transform ().Get_Orthogonal_Inverse (world_to_hlod_tm);
			Matrix3D::Multiply(world_to_hlod_tm,box_tm,&box_to_hlod_tm);

			box_to_hlod_tm.Transform_Center_Extent_AABox(	obbox_mesh->Get_Local_Center(),
																			obbox_mesh->Get_Local_Extent(),
																			&box.Center,&box.Extent);
		}

	} else {
		Animatable3DObjClass::Get_Obj_Space_Bounding_Box (box);
	}
}


/***********************************************************************************************
 * HLodClass::Get_Obj_Space_Bounding_Sphere -- Use the bounding box mesh to calculate a sphere.*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/13/00    pds : Created.                                                                 *
 *=============================================================================================*/
void HLodClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	AABoxClass box;
	Get_Obj_Space_Bounding_Box(box);
	sphere.Center = box.Center;
	sphere.Radius = box.Extent.Length();
}


/***********************************************************************************************
 * HLodClass::Get_Obj_Space_Bounding_Sphere -- Use the bounding box mesh to calculate a sphere.*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/13/00    pds : Created.                                                                 *
 *=============================================================================================*/
const SphereClass &HLodClass::Get_Bounding_Sphere(void) const
{
	if (BoundingBoxIndex >= 0) {
		//
		//	Get the bounding sphere in local coordinates
		//
		SphereClass sphere;
		Get_Obj_Space_Bounding_Sphere (sphere);

		//
		//	Transform the sphere into world coords and return the sphere
		//
#ifdef ALLOW_TEMPORARIES
		CachedBoundingSphere.Center = Get_Transform () * sphere.Center;
#else
		Get_Transform().mulVector3(sphere.Center, CachedBoundingSphere.Center);
#endif
		CachedBoundingSphere.Radius = sphere.Radius;
	} else {
		Animatable3DObjClass::Get_Bounding_Sphere ();
	}

	return CachedBoundingSphere;
}


/***********************************************************************************************
 * HLodClass::Get_Obj_Space_Bounding_Box -- Return the bounding box mesh if we have one.		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/13/00    pds : Created.                                                                 *
 *=============================================================================================*/
const AABoxClass &HLodClass::Get_Bounding_Box(void) const
{
	if (BoundingBoxIndex >= 0) {

		//
		//	Get the bounding box in local coordinates
		//
		AABoxClass box;
		Get_Obj_Space_Bounding_Box (box);

		//
		//	Transform the bounding box to world coordinates
		//
		Get_Transform().Transform_Center_Extent_AABox(	box.Center,
																		box.Extent,
																		&CachedBoundingBox.Center,
																		&CachedBoundingBox.Extent	);
	} else {
		Animatable3DObjClass::Get_Bounding_Box ();
	}

	return CachedBoundingBox;
}


/***********************************************************************************************
 * HLodClass::Set_Max_Screen_Size -- Set max-screen-size for an LOD                            *
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
void HLodClass::Set_Max_Screen_Size(int lod_index, float size)
{
	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < LodCount);
	if ((lod_index >= 0) && (lod_index < LodCount)) {

		// Set the new screen size for this LOD
		Lod[lod_index].MaxScreenSize = size;

		Recalculate_Static_LOD_Factors();

		// So that the object is ready for use after construction, we will
		// complete its initialization by initializing its cost and value arrays
		// according to a screen area of 1 pixel.
		int minlod = Calculate_Cost_Value_Arrays(1.0f, Value, Cost);

		// Ensure lod is no less than minimum allowed
		if (CurLod < minlod) Set_LOD_Level(minlod);
	}

	return ;
}


/***********************************************************************************************
 * HLodClass::Get_Max_Screen_Size -- get max-screen-size for an LOD                            *
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
float HLodClass::Get_Max_Screen_Size(int lod_index) const
{
	float size = NO_MAX_SCREEN_SIZE;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < LodCount);
	if ((lod_index >= 0) && (lod_index < LodCount)) {

		// Get the screen size for this LOD
		size = Lod[lod_index].MaxScreenSize;
	}

	// Return the LOD's screen size to the caller
	return size;
}


/***********************************************************************************************
 * HLodClass::Get_Lod_Count -- returns number of levels of detail                              *
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
int HLodClass::Get_Lod_Count(void) const
{
	return LodCount;
}


/***********************************************************************************************
 * HLodClass::Set_LOD_Bias --  sets LOD bias                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/08/00    nh : Created.                                                                  *
 *=============================================================================================*/
void HLodClass::Set_LOD_Bias(float bias)
{
	assert(bias > 0.0f);
	bias = MAX(bias, 0.0f);
	LODBias = bias;

	int additional_count = AdditionalModels.Count();
	for (int i = 0; i < additional_count; i++) {
		AdditionalModels[i].Model->Set_LOD_Bias(bias);
	}
}


/***********************************************************************************************
 * HLodClass::Get_Lod_Model_Count -- number of sub-objs in a given level of detail             *
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
int HLodClass::Get_Lod_Model_Count(int lod_index) const
{
	int count = 0;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < LodCount);
	if ((lod_index >= 0) && (lod_index < LodCount)) {

		// Get the number of models in this Lod
		count = Lod[lod_index].Count ();
	}

	// Return the number of models that compose this Lod
	return count;
}


/***********************************************************************************************
 * HLodClass::Peek_Lod_Model -- returns pointer to a model in one of the LODs                  *
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
RenderObjClass *HLodClass::Peek_Lod_Model(int lod_index, int model_index) const
{
	RenderObjClass *pmodel = nullptr;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < LodCount);
	if ((lod_index >= 0) &&
		 (lod_index < LodCount) &&
		 (model_index < Lod[lod_index].Count ())) {

		// Get a pointer to the requested model
		pmodel = Lod[lod_index][model_index].Model;
	}

	// Return a pointer to the requested model
	return pmodel;
}


/***********************************************************************************************
 * HLodClass::Get_Lod_Model -- returns a pointer to a model in one of the LODs                 *
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
RenderObjClass *HLodClass::Get_Lod_Model(int lod_index, int model_index) const
{
	RenderObjClass *pmodel = nullptr;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < LodCount);
	if ((lod_index >= 0) &&
		 (lod_index < LodCount) &&
		 (model_index < Lod[lod_index].Count ())) {

		// Get a pointer to the requested model
		pmodel = Lod[lod_index][model_index].Model;
		if (pmodel != nullptr) {
			pmodel->Add_Ref ();
		}
	}

	// Return the number of models that compose this Lod
	return pmodel;
}


/***********************************************************************************************
 * HLodClass::Get_Lod_Model_Bone -- returns the bone index of a model                          *
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
int HLodClass::Get_Lod_Model_Bone(int lod_index, int model_index) const
{
	int bone_index = 0;

	// Params valid?
	WWASSERT(lod_index >= 0);
	WWASSERT(lod_index < LodCount);
	if ((lod_index >= 0) &&
		 (lod_index < LodCount) &&
		 (model_index < Lod[lod_index].Count ())) {

		// Get the bone that this model resides on
		bone_index = Lod[lod_index][model_index].BoneIndex;
	}

	// Return the bone that this model resides on
	return bone_index;
}


/***********************************************************************************************
 * HLodClass::Get_Additional_Model_Count -- number of additional sub-objs                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/23/00    NH : Created.                                                                  *
 *=============================================================================================*/
int HLodClass::Get_Additional_Model_Count(void) const
{
	return AdditionalModels.Count();
}


/***********************************************************************************************
 * HLodClass::Peek_Additional_Model -- returns pointer to an additional model                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/23/00    NH : Created.                                                                  *
 *=============================================================================================*/
RenderObjClass * HLodClass::Peek_Additional_Model (int model_index) const
{
	RenderObjClass *pmodel = nullptr;

	// Param valid?
	WWASSERT(model_index >= 0);
	WWASSERT(model_index < AdditionalModels.Count());
	if ((model_index >= 0) &&
		 (model_index < AdditionalModels.Count())) {

		// Get a pointer to the requested model
		pmodel = AdditionalModels[model_index].Model;
	}

	// Return a pointer to the requested model
	return pmodel;
}


/***********************************************************************************************
 * HLodClass::Get_Additional_Model -- returns pointer to an additional model                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/23/00    NH : Created.                                                                  *
 *=============================================================================================*/
RenderObjClass * HLodClass::Get_Additional_Model (int model_index) const
{
	RenderObjClass *pmodel = nullptr;

	// Param valid?
	WWASSERT(model_index >= 0);
	WWASSERT(model_index < AdditionalModels.Count());
	if ((model_index >= 0) &&
		 (model_index < AdditionalModels.Count())) {

		// Get a pointer to the requested model
		pmodel = AdditionalModels[model_index].Model;
		if (pmodel != nullptr) {
			pmodel->Add_Ref ();
		}
	}

	// Return a pointer to the requested model
	return pmodel;
}


/***********************************************************************************************
 * HLodClass::Get_Additional_Model_Bone -- returns the bone index of an additional model       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/23/00    NH : Created.                                                                  *
 *=============================================================================================*/
int HLodClass::Get_Additional_Model_Bone (int model_index) const
{
	int bone_index = 0;

	// Params valid?
	WWASSERT(model_index >= 0);
	WWASSERT(model_index < AdditionalModels.Count());
	if ((model_index >= 0) &&
		 (model_index < AdditionalModels.Count())) {

		// Get the bone that this model resides on
		bone_index = AdditionalModels[model_index].BoneIndex;
	}

	// Return the bone that this model resides on
	return bone_index;
}


/***********************************************************************************************
 * HLodClass::Is_NULL_Lod_Included -- does this HLod have nullptr as its lowest LOD               *
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
bool HLodClass::Is_NULL_Lod_Included(void) const
{
	bool included = false;

	// Determine if the lowest-level LOD is the null render object or not...
	if ((LodCount > 0) && (Lod[0][0].Model != nullptr)) {
		included = (Lod[0][0].Model->Class_ID () == RenderObjClass::CLASSID_NULL);
	}

	// Return the true/false result code
	return included;
}


/***********************************************************************************************
 * HLodClass::Include_NULL_Lod -- Add nullptr as the lowest LOD                                   *
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
void HLodClass::Include_NULL_Lod(bool include)
{
	if ((include == false) && Is_NULL_Lod_Included ()) {

		// Free the 'nullptr' object's stored information
		int index = 0;
		for (int model = 0; model < Lod[index].Count (); model++) {

			RenderObjClass * robj = Lod[index][model].Model;
			Lod[index][model].Model = nullptr;

			WWASSERT(robj);
			robj->Set_Container (nullptr);
			robj->Release_Ref ();
		}

		// Resize the lod array
		LodCount -= 1;
		Lod[index].Delete_All ();
		ModelArrayClass *temp_lods = W3DNEWARRAY ModelArrayClass[LodCount];
		for (index = 0; index < LodCount; index ++) {
			temp_lods[index] = Lod[index + 1];
		}

		// Now resize the value and cost arrays
		float *temp_cost = W3DNEWARRAY float[LodCount];
		float *temp_value = W3DNEWARRAY float[LodCount + 1];
		::memcpy (temp_cost, &Cost[1], sizeof (float) * LodCount);
		::memcpy (temp_value, &Value[1], sizeof (float) * (LodCount + 1));

		delete [] Lod;
		delete [] Value;
		delete [] Cost;
		Lod = temp_lods;
		Value = temp_value;
		Cost = temp_cost;
		CurLod = (CurLod >= LodCount) ? (LodCount - 1) : CurLod;

	} else if (include && (Is_NULL_Lod_Included () == false)) {

		// Tag the nullptr render object onto the end
		RenderObjClass *null_object = WW3DAssetManager::Get_Instance ()->Create_Render_Obj ("NULL");
		WWASSERT (null_object != nullptr);
		if (null_object != nullptr) {

			// Resize the lod array
			ModelArrayClass *temp_lods = W3DNEWARRAY ModelArrayClass[LodCount + 1];
			for (int index = 0; index < LodCount; index ++) {
				temp_lods[index + 1] = Lod[index];
			}

			// Now resize the value and cost arrays
			float *temp_cost = W3DNEWARRAY float[LodCount + 1];
			float *temp_value = W3DNEWARRAY float[LodCount + 2];
			::memcpy (&temp_cost[1], Cost, sizeof (float) * LodCount);
			::memcpy (&temp_value[1], Value, sizeof (float) * (LodCount + 1));

			delete [] Lod;
			delete [] Value;
			delete [] Cost;
			Lod = temp_lods;
			Value = temp_value;
			Cost = temp_cost;
			LodCount ++;

			// Add this null object to the start of the lod list
			Add_Lod_Model (0, null_object, 0);
			null_object->Release_Ref ();
		}
	}

	// recalculate cost/value arrays and ensure the current LOD is still valid.
	int minlod = Calculate_Cost_Value_Arrays(1.0f, Value, Cost);

	// Ensure lod is no less than minimum allowed
	if (CurLod < minlod) Set_LOD_Level(minlod);

	return ;
}


/***********************************************************************************************
 * HLodClass::Get_Proxy_Count -- Returns the number of proxy records                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/27/2000 gth : Created.                                                                 *
 *=============================================================================================*/
int HLodClass::Get_Proxy_Count(void) const
{
	if (ProxyArray != nullptr) {
		return ProxyArray->Length();
	} else {
		return 0;
	}
}


/***********************************************************************************************
 * HLodClass::Get_Proxy -- returns the information for the i'th proxy                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/27/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool HLodClass::Get_Proxy (int index, ProxyClass &proxy) const
{
	bool retval = false;

	if (ProxyArray != nullptr) {

		//
		//	Lookup the proxy's transform
		//
		HTree->Base_Update(Get_Transform());
		Matrix3D transform = HTree->Get_Transform((*ProxyArray)[index].Get_Bone_Index());
		Set_Hierarchy_Valid(false);

		//
		//	Pass the data onto the proxy object
		//
		proxy.Set_Transform(transform);
		proxy.Set_Name((*ProxyArray)[index].Get_Name());
		retval = true;

	} else {
		proxy.Set_Name ("");
		proxy.Set_Transform (Matrix3D (1));
	}

	return retval;
}


/***********************************************************************************************
 * HLodClass::Get_Num_Polys -- returns polycount of the current LOD                            *
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
int HLodClass::Get_Num_Polys(void) const
{
	int polycount = 0;
	int i;
	int model_count = Lod[CurLod].Count();
	for (i = 0; i < model_count; i++) {
		if (Lod[CurLod][i].Model->Is_Not_Hidden_At_All()) {
			polycount += Lod[CurLod][i].Model->Get_Num_Polys();
		}
	}

	int additional_count = AdditionalModels.Count();
	for (i = 0; i < additional_count; i++) {
		if (AdditionalModels[i].Model->Is_Not_Hidden_At_All()) {
			polycount += AdditionalModels[i].Model->Get_Num_Polys();
		}
	}

	return polycount;
}


/***********************************************************************************************
 * HLodClass::Render -- render this HLod                                                       *
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
void HLodClass::Render(RenderInfoClass & rinfo)
{
	int i;

	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	Animatable3DObjClass::Render(rinfo);

	for (i = 0; i < Lod[CurLod].Count(); i++) {
		if (Lod[CurLod][i].Model->Class_ID() != CLASSID_OBBOX)	///We have no use for these - MW
			Lod[CurLod][i].Model->Render(rinfo);
	}

	if (Is_Sub_Objects_Match_LOD_Enabled()) {
		for (i = 0; i < AdditionalModels.Count(); i++) {
			AdditionalModels[i].Model->Set_LOD_Level(Get_LOD_Level());
			AdditionalModels[i].Model->Render(rinfo);
		}
	} else {
		for (i = 0; i < AdditionalModels.Count(); i++) {
			AdditionalModels[i].Model->Render(rinfo);
		}
	}
}


/***********************************************************************************************
 * HLodClass::Special_Render -- Special_Render for HLod                                        *
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
void HLodClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	int i;
	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	Animatable3DObjClass::Special_Render(rinfo);

	int lod_index = CurLod;
	if (rinfo.RenderType == SpecialRenderInfoClass::RENDER_SHADOW) {			// (gth) HACK HACK! yikes
		lod_index = LodCount-1;
	}

	for (i = 0; i < Lod[lod_index].Count(); i++) {
		Lod[lod_index][i].Model->Special_Render(rinfo);
	}

	for (i = 0; i < AdditionalModels.Count(); i++) {
		AdditionalModels[i].Model->Special_Render(rinfo);
	}
}


/***********************************************************************************************
 * HLodClass::Set_Transform -- Sets the transform                                              *
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
void HLodClass::Set_Transform(const Matrix3D &m)
{
	Animatable3DObjClass::Set_Transform(m);
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * HLodClass::Set_Position -- Sets the position                                                *
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
void HLodClass::Set_Position(const Vector3 &v)
{
	Animatable3DObjClass::Set_Position(v);
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * HLodClass::Notify_Added -- callback notifies subobjs that they were added                   *
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
void HLodClass::Notify_Added(SceneClass * scene)
{
	RenderObjClass::Notify_Added(scene);
	int i;
	int model_count = Lod[CurLod].Count();
	for (i = 0; i < model_count; i++) {
		Lod[CurLod][i].Model->Notify_Added(scene);
	}

	int additional_count = AdditionalModels.Count();
	for (i = 0; i < additional_count; i++) {
		AdditionalModels[i].Model->Notify_Added(scene);
	}
}


/***********************************************************************************************
 * HLodClass::Notify_Removed -- notifies subobjs that they were removed                        *
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
void HLodClass::Notify_Removed(SceneClass * scene)
{
	int i;
	int model_count = Lod[CurLod].Count();
	for (i = 0; i < model_count; i++) {
		Lod[CurLod][i].Model->Notify_Removed(scene);
	}

	int additional_count = AdditionalModels.Count();
	for (i = 0; i < additional_count; i++) {
		AdditionalModels[i].Model->Notify_Removed(scene);
	}
	RenderObjClass::Notify_Removed(scene);
}


/***********************************************************************************************
 * HLodClass::Get_Num_Sub_Objects -- returns total number of sub-objects                       *
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
int HLodClass::Get_Num_Sub_Objects(void) const
{
	int count = 0;
	for (int lod=0; lod<LodCount;lod++) {
		count += Lod[lod].Count();
	}
	count += AdditionalModels.Count();
	return count;
}


/***********************************************************************************************
 * HLodClass::Get_Sub_Object -- returns a pointer to specified sub-object                      *
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
RenderObjClass * HLodClass::Get_Sub_Object(int index) const
{
	WWASSERT(index >= 0);
	for (int lod=0; lod<LodCount; lod++) {
		if (index < Lod[lod].Count()) {
			Lod[lod][index].Model->Add_Ref();
			return Lod[lod][index].Model;
		}
		index -= Lod[lod].Count();
	}
	WWASSERT(index < AdditionalModels.Count());
	AdditionalModels[index].Model->Add_Ref();
	return AdditionalModels[index].Model;
}


/***********************************************************************************************
 * HLodClass::Add_Sub_Object -- add a sub-object to this HLod                                  *
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
int HLodClass::Add_Sub_Object(RenderObjClass * subobj)
{
	return Add_Sub_Object_To_Bone(subobj,0);
}


/***********************************************************************************************
 * HLodClass::Remove_Sub_Object -- remove a sub-object from this HLod                          *
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
int HLodClass::Remove_Sub_Object(RenderObjClass * removeme)
{
	// no object given?
	if (removeme == nullptr) {
		return 0;
	}

	// find the sub-object
	bool found = false;
	bool iscurrent = false;

	for (int lod = 0; (lod < LodCount) && (!found); lod++) {
		for (int model = 0; (model < Lod[lod].Count()) && (!found); model++) {

			if (Lod[lod][model].Model == removeme) {

				// remove the model from the array.
				Lod[lod].Delete(model);

				// record that we found it
				found = true;

				if (lod == CurLod) {
					iscurrent = true;
				}
			}
		}
	}

	for (int model = 0; (model < AdditionalModels.Count()) && (!found); model++) {
		if (AdditionalModels[model].Model == removeme) {
			AdditionalModels.Delete(model);
			found = true;
			iscurrent = true;
		}
	}

	if (found) {

		// clear the object's container pointer
		removeme->Set_Container(nullptr);

		// let him know in case he is removed from the scene as a result of this
		// this is the combination of this HLod being in the scene and and this model
		// either being in the current LOD or being in the additional model list...
		if (iscurrent && Is_In_Scene()) {
			removeme->Notify_Removed(Scene);
		}

		// release our reference to this render object
		// object may delete itself here...
		removeme->Release_Ref();

		Update_Sub_Object_Bits();
		Update_Obj_Space_Bounding_Volumes();

		return 1;
	}

	return 0;
}


/***********************************************************************************************
 * HLodClass::Get_Num_Sub_Objects_On_Bone -- returns the number of objects on the given bone   *
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
int HLodClass::Get_Num_Sub_Objects_On_Bone(int boneindex) const
{
	int count = 0;

	for (int lod = 0; lod < LodCount; lod++) {
		for (int model = 0; model < Lod[lod].Count(); model++) {
			if (Lod[lod][model].BoneIndex == boneindex) count++;
		}
	}
	for (int model = 0; model < AdditionalModels.Count(); model++) {
		if (AdditionalModels[model].BoneIndex == boneindex) count++;
	}
	return count;
}


/***********************************************************************************************
 * HLodClass::Get_Sub_Object_On_Bone -- returns obj on the given bone                          *
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
RenderObjClass * HLodClass::Get_Sub_Object_On_Bone(int index,int boneindex) const
{
	int count = 0;
	for (int lod = 0; lod < LodCount; lod++) {
		for (int model = 0; model < Lod[lod].Count(); model++) {
			if (Lod[lod][model].BoneIndex == boneindex) {
				if (count == index) {
					Lod[lod][model].Model->Add_Ref();
					return Lod[lod][model].Model;
				}
				count++;
			}
		}
	}
	for (int model = 0; model < AdditionalModels.Count(); model++) {
		if (AdditionalModels[model].BoneIndex == boneindex) {
			if (count == index) {
				AdditionalModels[model].Model->Add_Ref();
				return AdditionalModels[model].Model;
			}
			count++;
		}
	}
	return nullptr;
}


/***********************************************************************************************
 * HLodClass::Get_Sub_Object_Bone_Index -- returns bone index of given object                  *
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
int HLodClass::Get_Sub_Object_Bone_Index(RenderObjClass * subobj) const
{
	for (int lod = 0; lod < LodCount; lod++) {
		for (int model = 0; model < Lod[lod].Count(); model++) {
			if (Lod[lod][model].Model == subobj) {
				return Lod[lod][model].BoneIndex;
			}
		}
	}
	for (int model = 0; model < AdditionalModels.Count(); model++) {
		if (AdditionalModels[model].Model == subobj) {
			return AdditionalModels[model].BoneIndex;
		}
	}
	return 0;
}

//Custom version of above function for cases where we know the lod/model index. -MW
int HLodClass::Get_Sub_Object_Bone_Index(int LodIndex, int ModelIndex)	const
{
	return Lod[LodIndex][ModelIndex].BoneIndex;
}

/***********************************************************************************************
 * HLodClass::Add_Sub_Object_To_Bone -- adds a sub-object to a bone                            *
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
int HLodClass::Add_Sub_Object_To_Bone(RenderObjClass * subobj,int boneindex)
{
	WWASSERT(subobj);
	if ((boneindex < 0) || (boneindex >= HTree->Num_Pivots())) return 0;

	subobj->Set_LOD_Bias(LODBias);

	ModelNodeClass newnode;
	newnode.Model = subobj;
	newnode.Model->Add_Ref();
	newnode.Model->Set_Container(this);
	newnode.Model->Set_Animation_Hidden(HTree->Get_Visibility (boneindex) == false);
	newnode.BoneIndex = boneindex;

	int result = AdditionalModels.Add(newnode);

	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
	Set_Hierarchy_Valid (false);
	Set_Sub_Object_Transforms_Dirty(true);

	if (Is_In_Scene()) {
		subobj->Notify_Added(Scene);
	}

	return result;
}


/***********************************************************************************************
 * HLodClass::Set_Animation -- set animation state to the base-pose                            *
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
void HLodClass::Set_Animation(void)
{
	Animatable3DObjClass::Set_Animation();
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * HLodClass::Set_Animation -- set animation state to an animation frame                       *
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
void HLodClass::Set_Animation(HAnimClass * motion,float frame,int mode)
{
	Animatable3DObjClass::Set_Animation(motion,frame,mode);
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * HLodClass::Set_Animation -- set animation state to a blend of two animations                *
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
void HLodClass::Set_Animation
(
	HAnimClass * motion0,
	float frame0,
	HAnimClass * motion1,
	float frame1,
	float percentage
)
{
	Animatable3DObjClass::Set_Animation(motion0,frame0,motion1,frame1,percentage);
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * HLodClass::Set_Animation -- set animation state to a combination of anims                   *
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
void HLodClass::Set_Animation(HAnimComboClass * anim_combo)
{
	Animatable3DObjClass::Set_Animation(anim_combo);
	Set_Sub_Object_Transforms_Dirty(true);
}


/***********************************************************************************************
 * HLodClass::Cast_Ray -- cast a ray against this HLod                                         *
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
bool HLodClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	if (Are_Sub_Object_Transforms_Dirty ()) {
		Update_Sub_Object_Transforms ();
	}

	bool res = false;
	int i;

	// collide against the top LOD
	int top = LodCount-1;
	for (i = 0; i < Lod[top].Count(); i++) {
		res |= Lod[top][i].Model->Cast_Ray(raytest);
	}

	for (i = 0; i < AdditionalModels.Count(); i++) {
		res |= AdditionalModels[i].Model->Cast_Ray(raytest);
	}

	return res;
}


/***********************************************************************************************
 * HLodClass::Cast_AABox -- Cast a swept AABox against this HLod                               *
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
bool HLodClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	if (Are_Sub_Object_Transforms_Dirty ()) {
		Update_Sub_Object_Transforms ();
	}

	bool res = false;
	int i;

	// collide against the top LOD
	int top = LodCount-1;
	for (i = 0; i < Lod[top].Count(); i++) {
		res |= Lod[top][i].Model->Cast_AABox(boxtest);
	}

	for (i = 0; i < AdditionalModels.Count(); i++) {
		res |= AdditionalModels[i].Model->Cast_AABox(boxtest);
	}

	return res;
}


/***********************************************************************************************
 * HLodClass::Cast_OBBox -- Cast a swept OBBox against this HLod                               *
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
bool HLodClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	if (Are_Sub_Object_Transforms_Dirty ()) {
		Update_Sub_Object_Transforms ();
	}

	bool res = false;
	int i;

	// collide against the top LOD
	int top = LodCount-1;
	for (i = 0; i < Lod[top].Count(); i++) {
		res |= Lod[top][i].Model->Cast_OBBox(boxtest);
	}

	for (i = 0; i < AdditionalModels.Count(); i++) {
		res |= AdditionalModels[i].Model->Cast_OBBox(boxtest);
	}

	return res;
}


/***********************************************************************************************
 * HLodClass::Intersect_AABox -- Intersect an AABox with this HLod                             *
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
bool HLodClass::Intersect_AABox(AABoxIntersectionTestClass & boxtest)
{
	if (Are_Sub_Object_Transforms_Dirty ()) {
		Update_Sub_Object_Transforms ();
	}

	bool res = false;
	int i;

	// collide against the top LOD
	int top = LodCount-1;
	for (i = 0; i < Lod[top].Count(); i++) {
		res |= Lod[top][i].Model->Intersect_AABox(boxtest);
	}

	for (i = 0; i < AdditionalModels.Count(); i++) {
		res |= AdditionalModels[i].Model->Intersect_AABox(boxtest);
	}

	return res;
}


/***********************************************************************************************
 * HLodClass::Intersect_OBBox -- Intersect an OBBox with this HLod                             *
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
bool HLodClass::Intersect_OBBox(OBBoxIntersectionTestClass & boxtest)
{
	if (Are_Sub_Object_Transforms_Dirty ()) {
		Update_Sub_Object_Transforms ();
	}

	bool res = false;
	int i;

	// collide against the top LOD
	int top = LodCount-1;
	for (i = 0; i < Lod[top].Count(); i++) {
		res |= Lod[top][i].Model->Intersect_OBBox(boxtest);
	}

	for (i = 0; i < AdditionalModels.Count(); i++) {
		res |= AdditionalModels[i].Model->Intersect_OBBox(boxtest);
	}

	return res;
}


/***********************************************************************************************
 * HLodClass::Prepare_LOD -- Prepare for LOD processing                                        *
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
void HLodClass::Prepare_LOD(CameraClass &camera)
{
	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	// Find the maximum screen dimension of the object in pixels
	float norm_area = Get_Screen_Size(camera);

	/*
	** Set texture reduction factor for the (non-additional) subobjects:
	*/
// Texture reduction system broken, don't call!
//	Set_Texture_Reduction_Factor(Calculate_Texture_Reduction_Factor(norm_area));

	// Prepare LOD processing if this object has more than one LOD:
	if (LodCount > 1) {
		/*
		** Prepare cost and value arrays (and ensure current LOD doesn't violate clamping):
		*/
		int minlod = Calculate_Cost_Value_Arrays(norm_area, Value, Cost);
		if (CurLod < minlod) Set_LOD_Level(minlod);


		/*
		** Add myself to the LOD optimizer:
		*/
		PredictiveLODOptimizerClass::Add_Object(this);

	} else {

		// Not added to optimizer, need to add cost
		PredictiveLODOptimizerClass::Add_Cost(Get_Cost());

	}

	/*
	** Recursively call for the additional objects:
	*/
	int additional_count = AdditionalModels.Count();
	for (int i = 0; i < additional_count; i++) {
		if (AdditionalModels[i].Model->Is_Not_Hidden_At_All()) {
			AdditionalModels[i].Model->Prepare_LOD(camera);
		}
	}

}


/***********************************************************************************************
 * HLodClass::Recalculate_Static_LOD_Factors -- compute lod factors                            *
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
void HLodClass::Recalculate_Static_LOD_Factors(void)
{
	/*
	** Calculate NonPixelCost, PixelCostPerArea, BenefitFactor for all LOD
	** levels.
	** NOTE: for now we are using vastly simplified Cost and Benefit metrics.
	** (these will be improved after initial experimentation).
	** the Cost metric is simply the number of polygons, and the Benefit
	** Metric is 1 - 0.5 / #polygons^2.
	*/

	for (int i = 0; i < LodCount; i++) {

		// Currently there are no pixel-related costs taken into account
		Lod[i].PixelCostPerArea = 0.0f;

		// Sum polycount over all non-hidden models in array
		int model_count = Lod[i].Count();
		int polycount = 0;
		for (int j = 0; j < model_count; j++) {
			if (Lod[i][j].Model->Is_Not_Hidden_At_All()) {
				polycount += Lod[i][j].Model->Get_Num_Polys();
			}
		}
		// If polycount is zero set Cost to a small nonzero amount to avoid divisions by zero.
		Lod[i].NonPixelCost = (polycount != 0)? polycount : 0.000001f;

		// A polycount of zero yields a benefit factor of zero: otherwise apply formula.
		Lod[i].BenefitFactor = (polycount != 0) ? (1 - (0.5f / (polycount * polycount))) : 0.0f;
	}

}


/***********************************************************************************************
 * HLodClass::Increment_LOD -- move to next lod                                                *
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
void HLodClass::Increment_LOD(void)
{
	if (CurLod >= (LodCount-1)) return;

	if (Is_In_Scene()) {
		int model_count = Lod[CurLod].Count();
		for (int i = 0; i < model_count; i++) {
			Lod[CurLod][i].Model->Notify_Removed(Scene);
		}
	}

	CurLod++;

	if (Is_In_Scene()) {
		int model_count = Lod[CurLod].Count();
		for (int i = 0; i < model_count; i++) {
			Lod[CurLod][i].Model->Notify_Added(Scene);
		}
	}
}


/***********************************************************************************************
 * HLodClass::Decrement_LOD -- move to previous lod                                            *
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
void HLodClass::Decrement_LOD(void)
{
	if (CurLod < 1) return;

	if (Is_In_Scene()) {
		int model_count = Lod[CurLod].Count();
		for (int i = 0; i < model_count; i++) {
			Lod[CurLod][i].Model->Notify_Removed(Scene);
		}
	}

	CurLod--;

	if (Is_In_Scene()) {
		int model_count = Lod[CurLod].Count();
		for (int i = 0; i < model_count; i++) {
			Lod[CurLod][i].Model->Notify_Added(Scene);
		}
	}
}


/***********************************************************************************************
 * HLodClass::Get_Cost -- returns the cost of this LOD                                         *
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
float HLodClass::Get_Cost(void) const
{
	return(Cost[CurLod]);
}


/***********************************************************************************************
 * HLodClass::Get_Value -- returns the value of this LOD                                       *
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
float HLodClass::Get_Value(void) const
{
	return(Value[CurLod]);
}


/***********************************************************************************************
 * HLodClass::Get_Post_Increment_Value -- returns the post increment value                     *
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
float HLodClass::Get_Post_Increment_Value(void) const
{
	return(Value[CurLod + 1]);
}


/***********************************************************************************************
 * HLodClass::Set_LOD_Level -- set the current lod level                                       *
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
void HLodClass::Set_LOD_Level(int lod)
{
	lod = MAX(0, lod);
	lod = MIN(lod, (LodCount - 1));

	if (lod == CurLod) return;


	if (Is_In_Scene()) {
		int model_count = Lod[CurLod].Count();
		for (int i = 0; i < model_count; i++) {
			Lod[CurLod][i].Model->Notify_Removed(Scene);
		}
	}

	CurLod = lod;

	if (Is_In_Scene()) {
		int model_count = Lod[CurLod].Count();
		for (int i = 0; i < model_count; i++) {
			Lod[CurLod][i].Model->Notify_Added(Scene);
		}
	}
}


/***********************************************************************************************
 * HLodClass::Get_LOD_Level -- returns the current LOD level                                   *
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
int HLodClass::Get_LOD_Level(void) const
{
	return CurLod;
}


/***********************************************************************************************
 * HLodClass::Get_LOD_Count -- returns the number of levels of detail                          *
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
int HLodClass::Get_LOD_Count(void) const
{
	return LodCount;
}


/***********************************************************************************************
 * HLodClass::Calculate_Cost_Value_Arrays -- computes the cost-value arrays                    *
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
int HLodClass::Calculate_Cost_Value_Arrays(float screen_area, float *values, float *costs) const
{
	int lod = 0;

	// Calculate Cost heuristic for each LOD based on normalized screen area:
	for (lod = 0; lod < LodCount; lod++) {
		costs[lod] = Lod[lod].NonPixelCost + Lod[lod].PixelCostPerArea * screen_area;
	}

	// Calculate Value heuristic. First, all LOD levels for which
	// MaxScreenSize is smaller than screen_area have their Value set to
	// AT_MIN_LOD, as well as the first LOD after that (unless there are no
	// other LODs):
	for (lod = 0;  lod < LodCount && Lod[lod].MaxScreenSize < screen_area; lod++) {
		values[lod] = AT_MIN_LOD;
	}

	if (lod >= LodCount) {
		lod = LodCount - 1;
	} else {
		values[lod] = AT_MIN_LOD;
	}

	// Now lod is the lowest allowed - return this value.
	int minlod = lod;

	// Calculate Value heuristic for any remaining LODs based on normalized screen area:
	lod++;
	for (; lod < LodCount; lod++) {
		values[lod] = (Lod[lod].BenefitFactor * screen_area * LODBias) / costs[lod];
	}
	values[LodCount] = AT_MAX_LOD; 	// Post-inc value will flag max LOD.

	return minlod;
}


/***********************************************************************************************
 * HLodClass::Get_Current_LOD -- returns a render object which represents the current LOD      *
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
RenderObjClass * HLodClass::Get_Current_LOD(void)
{
	int count = Get_Lod_Model_Count(CurLod);

	if(!count)
		return nullptr;

	return Get_Lod_Model(CurLod, 0);
}


/***********************************************************************************************
 * HLodClass::Set_Texture_Reduction_Factor -- resizeable texture support                       *
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
/*
void HLodClass::Set_Texture_Reduction_Factor(float trf)
{
	WWASSERT(0);	// don't call to tex reduction system, it's broken!
	// We don't touch the additional subobjects: they will get Prepare_LOD
	// called on them individually which is where texture reduction will be
	// set also.
	for (int lod = 0; lod < LodCount; lod++) {
		int model_count = Lod[lod].Count();
		for (int model_id = 0; model_id < model_count; model_id++) {
			Lod[lod][model_id].Model->Set_Texture_Reduction_Factor(trf);
		}
	}
}
*/

/***********************************************************************************************
 * HLodClass::Scale -- scale this HLod model                                                   *
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
void HLodClass::Scale(float scale)
{
	if (scale==1.0f) return;

	int lod;
	int model;

	//. Scale all subobjects.
	for (lod = 0; lod < LodCount; lod++) {
		for (model = 0; model < Lod[lod].Count(); model++) {
			Lod[lod][model].Model->Scale(scale);
		}
	}

	for (model = 0; model < AdditionalModels.Count(); model++) {
		AdditionalModels[model].Model->Scale(scale);
	}

	// Scale HTree:
	HTree->Scale(scale);

	// Invalidate hierarchy
	Set_Hierarchy_Valid(false);

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


/***********************************************************************************************
 * HLodClass::Get_Num_Snap_Points -- returns the number of snap points in this model           *
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
int HLodClass::Get_Num_Snap_Points(void)
{
	if (SnapPoints) {
		return SnapPoints->Count();
	} else {
		return 0;
	}
}


/***********************************************************************************************
 * HLodClass::Get_Snap_Point -- returns specified snap-point                                   *
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
void HLodClass::Get_Snap_Point(int index,Vector3 * set)
{
	WWASSERT(set != nullptr);
	if (SnapPoints) {
		*set = (*SnapPoints)[index];
	} else {
		set->X = set->Y = set->Z = 0;
	}
}


/***********************************************************************************************
 * HLodClass::Update_Sub_Object_Transforms -- updates transforms of all sub-objects            *
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
void HLodClass::Update_Sub_Object_Transforms(void)
{
	/*
	** Update the animation transforms, recurse up to the
	** top of the tree...
	*/
	Animatable3DObjClass::Update_Sub_Object_Transforms();

	/*
	** Put the computed transforms into our sub objects.
	*/
	int lod,model;

	for (lod = 0; lod < LodCount; lod++) {
		for (model = 0; model < Lod[lod].Count(); model++) {

			RenderObjClass * robj = Lod[lod][model].Model;
			int bone = Lod[lod][model].BoneIndex;

			robj->Set_Transform(HTree->Get_Transform(bone));
			robj->Set_Animation_Hidden(!HTree->Get_Visibility(bone));
			robj->Update_Sub_Object_Transforms();
		}
	}

	for (model = 0; model < AdditionalModels.Count(); model++) {

		RenderObjClass * robj = AdditionalModels[model].Model;
		int bone = AdditionalModels[model].BoneIndex;

		robj->Set_Transform(HTree->Get_Transform(bone));
		robj->Set_Animation_Hidden(!HTree->Get_Visibility(bone));
		robj->Update_Sub_Object_Transforms();
	}

	Set_Sub_Object_Transforms_Dirty(false);
}


/***********************************************************************************************
 * HLodClass::Update_Obj_Space_Bounding_Volumes -- update object-space bounding volumes        *
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
void HLodClass::Update_Obj_Space_Bounding_Volumes(void)
{
	//
	//	Do we still have a valid bounding box index?
	//
	ModelArrayClass &high_lod = Lod[LodCount - 1];
	int count = high_lod.Count ();
	if (	BoundingBoxIndex < 0 ||
			BoundingBoxIndex >= count ||
			high_lod[BoundingBoxIndex].Model->Class_ID () != RenderObjClass::CLASSID_OBBOX)
	{
		BoundingBoxIndex = -1;
	}

	//
	//	Attempt to find an OBBox mesh inside the hierarchy
	//
	int index = high_lod.Count ();
	while (index -- && BoundingBoxIndex == -1) {
		RenderObjClass *model = high_lod[index].Model;

		//
		//	Is this an OBBox mesh?
		//
		if (model->Class_ID () == RenderObjClass::CLASSID_OBBOX)
		{
			const char *name = model->Get_Name ();
			const char *name_seg = ::strchr (name, '.');
			if (name_seg != nullptr) {
				name = name_seg + 1;
			}

			//
			//	Does the name match the designator we are looking for?
			//
			if (::stricmp (name, "BOUNDINGBOX") == 0) {
				BoundingBoxIndex = index;
			}
		}
	}


	int i;
	RenderObjClass * robj = nullptr;

	// if we don't have any sub objects, just set default bounds
	if (Get_Num_Sub_Objects() <= 0) {
		ObjSphere.Init(Vector3(0,0,0),0);
		ObjBox.Center.Set(0,0,0);
		ObjBox.Extent.Set(0,0,0);
		return;
	}

	// loop through all sub-objects, combining their object-space bounding spheres and boxes.
	// Put our HTree in its base pose at the origin.
	SphereClass sphere;
	AABoxClass obj_aabox;
	MinMaxAABoxClass box;

	HTree->Base_Update(Matrix3D(1));

	robj = Get_Sub_Object(0);
	WWASSERT(robj);

	const Matrix3D & bonetm = HTree->Get_Transform(Get_Sub_Object_Bone_Index(robj));
	robj->Get_Obj_Space_Bounding_Sphere(sphere);
	sphere.Transform(bonetm);
	robj->Get_Obj_Space_Bounding_Box(obj_aabox);

	box.Init(obj_aabox);
	box.Transform(bonetm);

	robj->Release_Ref();

	for (i=1; i<Get_Num_Sub_Objects(); i++) {
		robj = Get_Sub_Object(i);
		WWASSERT(robj);

		const Matrix3D & bonetm = HTree->Get_Transform(Get_Sub_Object_Bone_Index(robj));

		SphereClass tmpsphere;
		robj->Get_Obj_Space_Bounding_Sphere(tmpsphere);
		tmpsphere.Transform(bonetm);
		sphere.Add_Sphere(tmpsphere);

		AABoxClass tmpbox;
		robj->Get_Obj_Space_Bounding_Box(tmpbox);
		tmpbox.Transform(bonetm);
		box.Add_Box(tmpbox);

		robj->Release_Ref();
	}

	ObjSphere = sphere;
	ObjBox = box;

   Invalidate_Cached_Bounding_Volumes();
	Set_Hierarchy_Valid(false);

   // Now update the object space bounding volumes of this object's container:
   RenderObjClass *container = Get_Container();
   if (container) container->Update_Obj_Space_Bounding_Volumes();
}


/***********************************************************************************************
 * HLodClass::Add_Lod_Model -- adds a model to one of the lods                                 *
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
void HLodClass::Add_Lod_Model(int lod, RenderObjClass * robj, int boneindex)
{
	WWASSERT(robj != nullptr);

	ModelNodeClass newnode;
	newnode.Model = robj;
	newnode.Model->Add_Ref();
	newnode.BoneIndex = boneindex;
	newnode.Model->Set_Container(this);
	newnode.Model->Set_Transform(HTree->Get_Transform(boneindex));

	if (Is_In_Scene() && lod == CurLod) {
		newnode.Model->Notify_Added(Scene);
	}
	Lod[lod].Add(newnode);
}


/***********************************************************************************************
 * HLodClass::Create_Decal -- create a decal on this HLod                                      *
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
void HLodClass::Create_Decal(DecalGeneratorClass * generator)
{
	for (int lod=0; lod<LodCount; lod++) {
		for (int model=0; model<Lod[lod].Count(); model++) {
			Lod[lod][model].Model->Create_Decal(generator);
		}
	}

	for (int model=0; model<AdditionalModels.Count(); model++) {
		AdditionalModels[model].Model->Create_Decal(generator);
	}
}


/***********************************************************************************************
 * HLodClass::Delete_Decal -- remove a decal from this HLod                                    *
 *                                                                                             *
 *    The decal_id is the ID which was assigned to the DecalGeneratorClass when you created    *
 *    the decal.                                                                               *
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
void HLodClass::Delete_Decal(uint32 decal_id)
{
	for (int lod=0; lod<LodCount; lod++) {
		for (int model=0; model<Lod[lod].Count(); model++) {
			Lod[lod][model].Model->Delete_Decal(decal_id);
		}
	}

	for (int model=0; model<AdditionalModels.Count(); model++) {
		AdditionalModels[model].Model->Delete_Decal(decal_id);
	}
}


/***********************************************************************************************
 * HLodClass::Set_HTree -- replace the hierarchy tree                                          *
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
void HLodClass::Set_HTree(HTreeClass * htree)
{
	Animatable3DObjClass::Set_HTree(htree);
}


/***********************************************************************************************
 * HLodClass::Set_Hidden -- Propogates the hidden bit to particle emitters.                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/19/01    pds : Created.                                                                 *
 *=============================================================================================*/
void HLodClass::Set_Hidden(int onoff)
{
	//
	//	Loop over all attached models
	//
	int additional_count = AdditionalModels.Count();
	for (int index = 0; index < additional_count; index ++) {

		//
		//	Is this a particle emitter?
		//
		RenderObjClass *model = AdditionalModels[index].Model;
		if (model->Class_ID () == RenderObjClass::CLASSID_PARTICLEEMITTER) {

			//
			//	Pass the hidden bit onto the emitter
			//
			model->Set_Hidden(onoff);
		}
	}

	Animatable3DObjClass::Set_Hidden(onoff);
	return ;
}

