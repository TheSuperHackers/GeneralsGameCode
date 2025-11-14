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
 *                     $Archive:: /Commando/Code/ww3d2/distlod.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 2/06/01 3:21p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DistLODDefClass -- default constructor for DistLODDefClass                                *
 *   DistLODDefClass::DistLODDefClass -- manual constructor for DistLODDefClass                *
 *   DistLODDefClass::~DistLODDefClass -- destructor for DistLODDefClass                       *
 *   DistLODDefClass::Free -- releases all memory in use by this object                        *
 *   DistLODDefClass::Load -- initialize this object from a W3D file                           *
 *   DistLODDefClass::read_header -- read the header from a W3D file                           *
 *   DistLODDefClass::read_node -- read a model node description from a W3D file               *
 *   DistLODClass::DistLODClass -- constructor                                                 *
 *   DistLODClass::DistLODClass -- copy constructor                                            *
 *   DistLODClass::~DistLODClass -- destructor                                                 *
 *   DistLODClass::Free -- releases memory in use                                              *
 *   DistLODClass::Get_Name -- returns the name of this LOD object                             *
 *   DistLODClass::Get_Num_Polys -- returns the number of polys in this model                  *
 *   DistLODClass::Render -- Render this LOD.                                                  *
 *   DistLODClass::Special_Render -- custom render function                                    *
 *   DistLODCLass::Get_Num_Sub_Objects -- returns the number of subobjects (levels of detail)  *
 *   DistLODClass::Get_Sub_Object -- returns pointer to the specified sub-object (LOD)         *
 *   DistLODCLass::Set_Transform -- sets the transform for this model                          *
 *   DistLODClass::Set_Position -- set the position of this object                             *
 *   DistLODClass::Set_Animation -- set the animation state of this model                      *
 *   DistLODClass::Set_Animation -- set the animation state of this model                      *
 *   DistLODClass::Set_Animation -- set the animation state to a blend of two anims            *
 *   DistLODClass::Set_Animation -- set the animation state to a combination of anims          *
 *   DistLODClass::Get_Num_Bones -- returns the number of bones                                *
 *   DistLODClass::Get_Bone_Name -- returns the name of the specified bone                     *
 *   DistLODClass::Get_Bone_Index -- returns the index of the given bone (if found)            *
 *   DistLODClass::Get_Bone_Transform -- returns the transform of the given bone               *
 *   DistLODClass::Get_Bone_Transform -- returns the transform of the given bone               *
 *   DistLODClass::Capture_Bone -- take control of a bone                                      *
 *   DistLODClass::Release_Bone -- release control of a bone                                   *
 *   DistLODClass::Is_Bone_Captured -- check whether the given bone is captured                *
 *   DistLODClass::Control_Bone -- set the transform for a captured bone                       *
 *   DistLODClass::Cast_Ray -- cast a ray against this model                                   *
 *   DistLODClass::Cast_AABox -- perform an AABox cast against this model                      *
 *   DistLODClass::Cast_OBBox -- perform an OBBox cast against this model                      *
 *   DistLODCLass::Get_Num_Snap_Points -- returns number of snap points in this model          *
 *   DistLODClass::Get_Snap_Point -- returns the i'th snap point                               *
 *   DistLODCLass::Scale -- scale this model; passes on to each LOD                            *
 *   DistLODClass::Scale -- scale this model; passes on to each LOD                            *
 *   DistLODClass::Update_LOD -- adjusts the current LOD based on the distance                 *
 *   DistLODClass::Increment_Lod -- moves to a higher detail LOD                               *
 *   DistLODClass::Decrement_Lod -- moves to a lower detail LOD                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "distlod.h"
#include "nstrdup.h"
#include "ww3d.h"
#include "assetmgr.h"
#include "camera.h"
#include "w3derr.h"
#include "wwdebug.h"
#include "chunkio.h"
#include "hlod.h"
#include "rinfo.h"
#include "coltest.h"
#include "inttest.h"

/*
** Loader Instance
*/
DistLODLoaderClass			_DistLODLoader;


RenderObjClass * DistLODPrototypeClass::Create(void)
{
	DistLODClass * dist = NEW_REF( DistLODClass , ( *Definition ) );

	// Have to pull each LOD out of the DistLOD, create a copy of the name
	// and destroy the DistLOD so that the models are "containerless".  Also
	// invert the order of the models in the DistLOD
	char * name = nstrdup(dist->Get_Name());
	WWASSERT(name != NULL);

	int count = dist->Get_Num_Sub_Objects();
	RenderObjClass ** robj = W3DNEWARRAY RenderObjClass * [count];
	int i=0;
	for (; i<count; i++) {

		robj[count - 1 - i] = dist->Get_Sub_Object(i);
		WWASSERT(robj[count - 1 - i] != NULL);
	}
	dist->Release_Ref();

	WWDEBUG_SAY(("OBSOLETE Dist-LOD model found! Please re-export %s!",name));
	HLodClass * hlod = NEW_REF(HLodClass , (name,robj,count));

	// Now, release the temporary refs and memory for the name
	for (i=0; i<count; i++) {
		robj[i]->Release_Ref();
	}
	free(name);

	return hlod;
}

/*
** The Prototype Loader
*/
PrototypeClass *DistLODLoaderClass::Load_W3D( ChunkLoadClass &cload )
{
	DistLODDefClass * pCDistLODClass = W3DNEW DistLODDefClass;

	if (pCDistLODClass == NULL)
	{
		return NULL;
	}

	if (pCDistLODClass->Load_W3D(cload) != WW3D_ERROR_OK)
	{
		// load failed, delete the model and return an error
		delete pCDistLODClass;
		return NULL;

	} else {

		// ok, accept this model!
		DistLODPrototypeClass *pCLODProto = W3DNEW DistLODPrototypeClass (pCDistLODClass);
		return pCLODProto;
	}
}


/***********************************************************************************************
 * DistLODDefClass -- default constructor for DistLODDefClass                                  *
 *                                                                                             *
 * DistLODDefClass is a "blueprint" for constructing a DistLODClass                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
DistLODDefClass::DistLODDefClass(void) :
	Name(NULL),
	LodCount(0),
	Lods(NULL)
{
}


/***********************************************************************************************
 * DistLODDefClass::DistLODDefClass -- manual constructor for DistLODDefClass                  *
 *                                                                                             *
 * This constructor allows you to create a DistLODDef Manually (and then use it to create      *
 * the desired DistLODClass).  The array of DistLODModelDefStructs which you pass in will      *
 * only be read from so you are responsible for the memory used by them.                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
DistLODDefClass::DistLODDefClass(const char * name,int lodcount,DistLODNodeDefStruct * modeldefs) :
	Name(NULL),
	LodCount(0),
	Lods(NULL)
{
	assert(name != NULL);
	Name = nstrdup(name);

	LodCount = lodcount;
	Lods = W3DNEWARRAY DistLODNodeDefStruct[LodCount];
	for (int i=0; i<LodCount; i++) {
		Lods[i].Name = nstrdup(modeldefs[i].Name);
		Lods[i].ResDownDist = modeldefs[i].ResDownDist;
		Lods[i].ResUpDist = modeldefs[i].ResUpDist;
	}
}


/***********************************************************************************************
 * DistLODDefClass::~DistLODDefClass -- destructor for DistLODDefClass                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
DistLODDefClass::~DistLODDefClass(void)
{
	Free();
}


/***********************************************************************************************
 * DistLODDefClass::Free -- releases all memory in use by this object                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODDefClass::Free(void)
{
	delete[] Name;
	Name = NULL;

	if (Lods != NULL) {
		for (int i=0; i<LodCount; i++) {
			delete[] Lods[i].Name;
		}
		delete[] Lods;
		Lods = NULL;
	}
	LodCount = 0;
}


/***********************************************************************************************
 * DistLODDefClass::Load -- initialize this object from a W3D file                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WW3DErrorType DistLODDefClass::Load_W3D(ChunkLoadClass & cload)
{
	/*
	** First make sure we release any memory in use
	*/
	Free();

	if (read_header(cload) == false) {
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

		if (cload.Cur_Chunk_ID() != W3D_CHUNK_LOD) {
			// ERROR: Expected LOD struct!
			return WW3D_ERROR_LOAD_FAILED;
		}

		/*
		**	Read the data from the chunk into the LOD struct
		*/
		W3dLODStruct lodStruct;
		if (cload.Read(&lodStruct,sizeof(W3dLODStruct)) != sizeof(W3dLODStruct)) {
			return WW3D_ERROR_LOAD_FAILED;
		}

		// Add the information from the chunk into the LOD array
		Lods[iLOD].Name = nstrdup (lodStruct.RenderObjName);
		Lods[iLOD].ResUpDist = lodStruct.LODMin;
		Lods[iLOD].ResDownDist = lodStruct.LODMax;

		// Close-out the chunk
		cload.Close_Chunk();
	}

	return WW3D_ERROR_OK;
}


/***********************************************************************************************
 * DistLODDefClass::read_header -- read the header from a W3D file                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool DistLODDefClass::read_header(ChunkLoadClass & cload)
{
	/*
	**	Open the first chunk, it should be the LOD header
	*/
	if (!cload.Open_Chunk()) return false;

	if (cload.Cur_Chunk_ID() != W3D_CHUNK_LODMODEL_HEADER) {
		// ERROR: Expected LOD Header!
		return false;
	}

	W3dLODModelHeaderStruct lodHeader;
	if (cload.Read(&lodHeader,sizeof(W3dLODModelHeaderStruct)) != sizeof(W3dLODModelHeaderStruct)) {
		return false;
	}

	cload.Close_Chunk();

	// Copy the name into our internal variable
	Name = ::nstrdup (lodHeader.Name);
	LodCount = lodHeader.NumLODs;
	Lods = W3DNEWARRAY DistLODNodeDefStruct[LodCount];
	return true;
}


/***********************************************************************************************
 * DistLODDefClass::read_node -- read a model node description from a W3D file                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool DistLODDefClass::read_node(ChunkLoadClass & cload,DistLODNodeDefStruct * node)
{
	return true;
}


/***********************************************************************************************
 * DistLODClass::DistLODClass -- constructor                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
DistLODClass::DistLODClass(const DistLODDefClass & def)
{
	Set_Name(def.Get_Name());
	LodCount = def.LodCount;
	CurLod = 0;
	Lods = W3DNEWARRAY LODNodeClass[LodCount];

	for (int i=0; i<LodCount; i++) {
		// create a render object
		Lods[i].Model = WW3DAssetManager::Get_Instance()->Create_Render_Obj(def.Lods[i].Name);
		assert(Lods[i].Model != NULL);
      Lods[i].Model->Set_Container(this);

		// copy the distances
		Lods[i].ResUpDist = def.Lods[i].ResUpDist;
		Lods[i].ResDownDist = def.Lods[i].ResDownDist;
	}

	Update_Sub_Object_Bits();
	Update_Obj_Space_Bounding_Volumes();
}


/***********************************************************************************************
 * DistLODClass::DistLODClass -- copy constructor                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
DistLODClass::DistLODClass(const DistLODClass & that) :
	CompositeRenderObjClass( that )
{
	LodCount = that.LodCount;
	CurLod = VpPushLod = that.CurLod;

	Lods = W3DNEWARRAY LODNodeClass[LodCount];
	for (int i=0; i<LodCount; i++) {
		// create a render object
		Lods[i].Model = that.Lods[i].Model->Clone();
		assert(Lods[i].Model != NULL);
      Lods[i].Model->Set_Container(this);

		// copy the distances
		Lods[i].ResUpDist = that.Lods[i].ResUpDist;
		Lods[i].ResDownDist = that.Lods[i].ResDownDist;
	}
	Update_Obj_Space_Bounding_Volumes();
}


/***********************************************************************************************
 * DistLODClass::~DistLODClass -- destructor                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
DistLODClass::~DistLODClass(void)
{
	Free();
}


/***********************************************************************************************
 * DistLODClass::Free -- releases memory in use                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Free(void)
{
	if (Lods != NULL) {
		for (int i=0; i<LodCount; i++) {
			if (Lods[i].Model != NULL) {
				Lods[i].Model->Set_Container(NULL);
				Lods[i].Model->Release_Ref();
				Lods[i].Model = NULL;
			}
		}
		delete[] Lods;
		Lods = NULL;
	}
	CurLod = 0;
	LodCount = 0;
}


/***********************************************************************************************
 * DistLODClass::Get_Num_Polys -- returns the number of polys in this model                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * returns the number of polys in the *current* lod.                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
int DistLODClass::Get_Num_Polys(void) const
{
	return Lods[CurLod].Model->Get_Num_Polys();
}


/***********************************************************************************************
 * DistLODClass::Render -- Render this LOD.                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Render(RenderInfoClass & rinfo)
{
	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	Update_Lod(rinfo.Camera);
	Lods[CurLod].Model->Render(rinfo);
}


/***********************************************************************************************
 * DistLODClass::Special_Render -- custom render function                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	Update_Lod(rinfo.Camera);
	Lods[CurLod].Model->Special_Render(rinfo);
}


/***********************************************************************************************
 * DistLODCLass::Get_Num_Sub_Objects -- returns the number of subobjects (levels of detail)    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
int DistLODClass::Get_Num_Sub_Objects(void) const
{
	return LodCount;
}


/***********************************************************************************************
 * DistLODClass::Get_Sub_Object -- returns pointer to the specified sub-object (LOD)           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * DistLODClass::Get_Sub_Object(int index) const
{
	assert(index >= 0);
	assert(index < LodCount);

	if (Lods[index].Model == NULL) {
		return NULL;
	} else {
		Lods[index].Model->Add_Ref();
		return Lods[index].Model;
	}
}

int DistLODClass::Add_Sub_Object_To_Bone(RenderObjClass * subobj,int bone_index)
{
	// NOTE: this is broken code, a render object cannot have two containers...
	if (subobj->Class_ID() == CLASSID_DISTLOD) {
		// Add each lod of the sub object to a cooresponding model of mine
		DistLODClass * sub_lod_obj = (DistLODClass *)subobj;
		for (int i=0; i< LodCount; i++) {
			Lods[i].Model->Add_Sub_Object_To_Bone( sub_lod_obj->Lods[ MIN( i, sub_lod_obj->LodCount-1 ) ].Model, bone_index);
		}
	} else {
		Lods[0].Model->Add_Sub_Object_To_Bone( subobj, bone_index);
	}
	return 0;
}


/***********************************************************************************************
 * DistLODCLass::Set_Transform -- sets the transform for this model                            *
 *                                                                                             *
 * sets the transform in all LODs.                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Set_Transform(const Matrix3D &m)
{
	RenderObjClass::Set_Transform(m);
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Set_Transform(m);
	}
}


/***********************************************************************************************
 * DistLODClass::Set_Position -- set the position of this object                               *
 *                                                                                             *
 * sets the position of all lods                                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Set_Position(const Vector3 &v)
{
	RenderObjClass::Set_Position(v);
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Set_Position(v);
	}
}


/***********************************************************************************************
 * DistLODClass::Set_Animation -- set the animation state of this model                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void	DistLODClass::Set_Animation( void )
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Set_Animation();
	}
}


/***********************************************************************************************
 * DistLODClass::Set_Animation -- set the animation state of this model                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Set_Animation( HAnimClass * motion,float frame,int mode)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Set_Animation(motion,frame,mode);
	}
}


/***********************************************************************************************
 * DistLODClass::Set_Animation -- set the animation state to a blend of two anims              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Set_Animation( HAnimClass * motion0,float frame0,HAnimClass * motion1,float frame1,float percentage)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Set_Animation(motion0,frame0,motion1,frame1,percentage);
	}
}


/***********************************************************************************************
 * DistLODClass::Set_Animation -- set the animation state to a combination of anims            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Set_Animation( HAnimComboClass * anim_combo)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Set_Animation( anim_combo);
	}
}


/***********************************************************************************************
 * DistLODClass::Peek_Animation													                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
HAnimClass *	DistLODClass::Peek_Animation( void )
{
	return Lods[0].Model->Peek_Animation();
}


/***********************************************************************************************
 * DistLODClass::Get_Num_Bones -- returns the number of bones                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
int DistLODClass::Get_Num_Bones(void)
{
	return Lods[0].Model->Get_Num_Bones();
}


/***********************************************************************************************
 * DistLODClass::Get_Bone_Name -- returns the name of the specified bone                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
const char * DistLODClass::Get_Bone_Name(int bone_index)
{
	return Lods[0].Model->Get_Bone_Name(bone_index);
}


/***********************************************************************************************
 * DistLODClass::Get_Bone_Index -- returns the index of the given bone (if found)              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
int DistLODClass::Get_Bone_Index(const char * bonename)
{
   // Highest LOD is used since lowest may be a Null3DObjClass.
	return Lods[0].Model->Get_Bone_Index(bonename);
}


/***********************************************************************************************
 * DistLODClass::Get_Bone_Transform -- returns the transform of the given bone                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
const Matrix3D &	DistLODClass::Get_Bone_Transform(const char * bonename)
{
   // Highest LOD is used since lowest may be a Null3DObjClass.
	return Lods[0].Model->Get_Bone_Transform(bonename);
}


/***********************************************************************************************
 * DistLODClass::Get_Bone_Transform -- returns the transform of the given bone                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
const Matrix3D &	DistLODClass::Get_Bone_Transform(int boneindex)
{
   // Highest LOD is used since lowest may be a Null3DObjClass.
	return Lods[0].Model->Get_Bone_Transform(boneindex);
}


/***********************************************************************************************
 * DistLODClass::Capture_Bone -- take control of a bone                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Capture_Bone(int bindex)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Capture_Bone(bindex);
	}
}


/***********************************************************************************************
 * DistLODClass::Release_Bone -- release control of a bone                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Release_Bone(int bindex)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Release_Bone(bindex);
	}
}


/***********************************************************************************************
 * DistLODClass::Is_Bone_Captured -- check whether the given bone is captured                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool DistLODClass::Is_Bone_Captured(int bindex) const
{
   // Highest LOD is used since lowest may be a Null3DObjClass.
	return Lods[0].Model->Is_Bone_Captured(bindex);
}


/***********************************************************************************************
 * DistLODClass::Control_Bone -- set the transform for a captured bone                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Control_Bone(int bindex,const Matrix3D & tm,bool world_space_translation)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Control_Bone(bindex,tm,world_space_translation);
	}
}

/***********************************************************************************************
 * DistLODClass::Cast_Ray -- cast a ray against this model                                     *
 *                                                                                             *
 * casts the ray against the top-level LOD                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool DistLODClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	if (raytest.CollisionType & Get_Collision_Type()) {
		return Lods[HIGHEST_LOD].Model->Cast_Ray(raytest);
	} else {
		return false;
	}
}


/***********************************************************************************************
 * DistLODClass::Cast_AABox -- perform an AABox cast against this model                        *
 *                                                                                             *
 * tests against the top-level LOD                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/3/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool DistLODClass::Cast_AABox(AABoxCollisionTestClass & boxtest)
{
	if (boxtest.CollisionType & Get_Collision_Type()) {
		return Lods[HIGHEST_LOD].Model->Cast_AABox(boxtest);
	} else {
		return false;
	}
}


/***********************************************************************************************
 * DistLODClass::Cast_OBBox -- perform an OBBox cast against this model                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool DistLODClass::Cast_OBBox(OBBoxCollisionTestClass & boxtest)
{
	if (boxtest.CollisionType & Get_Collision_Type()) {
		return Lods[HIGHEST_LOD].Model->Cast_OBBox(boxtest);
	} else {
		return false;
	}
}


/***********************************************************************************************
 * DistLODCLass::Get_Num_Snap_Points -- returns number of snap points in this model            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
int DistLODClass::Get_Num_Snap_Points(void)
{
	return Lods[0].Model->Get_Num_Snap_Points();
}


/***********************************************************************************************
 * DistLODClass::Get_Snap_Point -- returns the i'th snap point                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Get_Snap_Point(int index,Vector3 * set)
{
	Lods[0].Model->Get_Snap_Point(index,set);
}


/***********************************************************************************************
 * DistLODCLass::Scale -- scale this model; passes on to each LOD                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Scale(float scale)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[CurLod].Model != NULL);
		Lods[CurLod].Model->Scale(scale);
	}
}


/***********************************************************************************************
 * DistLODClass::Scale -- scale this model; passes on to each LOD                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Scale(float scalex, float scaley, float scalez)
{
	for (int i=0; i<LodCount; i++) {
		assert(Lods[i].Model != NULL);
		Lods[i].Model->Scale(scalex,scaley,scalez);
	}
}


/***********************************************************************************************
 * DistLODClass::Update_LOD -- adjusts the current LOD based on the distance                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Update_Lod(const CameraClass & camera)
{
	// evaluate the distance from the camera and select an LOD based on it.
	float dist =	(
							camera.Get_Position() -
							Lods[CurLod].Model->Get_Bounding_Sphere().Center
						).Quick_Length();

	if (dist < Lods[CurLod].ResUpDist) {
		Increment_Lod();
	} else if (dist > Lods[CurLod].ResDownDist) {
		Decrement_Lod();
	}
}


/***********************************************************************************************
 * DistLODClass::Increment_Lod -- moves to a higher detail LOD                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Increment_Lod(void)
{
	// TODO: change the order in which models are stored
	if (CurLod > 0) {
		if (Is_In_Scene()) {
			Lods[CurLod].Model->Notify_Removed(Scene);
		}
		CurLod--;
		if (Is_In_Scene()) {
			Lods[CurLod].Model->Notify_Added(Scene);
		}
	}
}


/***********************************************************************************************
 * DistLODClass::Decrement_Lod -- moves to a lower detail LOD                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/1/99     GTH : Created.                                                                 *
 *=============================================================================================*/
void DistLODClass::Decrement_Lod(void)
{
	if (CurLod < LodCount - 1) {
		if (Is_In_Scene()) {
			Lods[CurLod].Model->Notify_Removed(Scene);
		}
		CurLod++;
		if (Is_In_Scene()) {
			Lods[CurLod].Model->Notify_Added(Scene);
		}
	}
}


