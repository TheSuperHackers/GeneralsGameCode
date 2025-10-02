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

/* $Header: /Commando/Code/ww3d2/hcanim.cpp 3     6/29/01 6:41p Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hcanim.cpp                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 6/27/01 7:50p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   NodeMotionStruct::NodeMotionStruct -- constructor                                         *
 *   NodeMotionStruct::~NodeMotionStruct -- destructor                                         *
 *   HCompressedAnimClass::HCompressedAnimClass -- constructor                                 *
 *   HCompressedAnimClass::~HCompressedAnimClass -- Destructor                                 *
 *   HCompressedAnimClass::Free -- De-allocates all memory in use                              *
 *   HCompressedAnimClass::Load -- Loads hierarchy animation from a file                       *
 *   HCompressedAnimClass::read_channel -- Reads in a single channel of motion                 *
 *   HCompressedAnimClass::add_channel -- Adds a motion channel to the animation               *
 *   HCompressedAnimClass::Get_Translation -- returns the translation vector for the given fram*
 *   HCompressedAnimClass::Get_Orientation -- returns a quaternion for the orientation of the p*
 *   HCompressedAnimClass::read_bit_channel -- read a bit channel from the file                *
 *   HCompressedAnimClass::add_bit_channel -- install a bit channel into the animation         *
 *   HCompressedAnimClass::Get_Visibility -- return visibility state for given pivot/frame     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "hcanim.h"
#include "assetmgr.h"
#include "htree.h"
#include "motchan.h"
#include "chunkio.h"
#include "w3d_file.h"
#include "wwdebug.h"
#include <string.h>
#include <nstrdup.h>


struct NodeCompressedMotionStruct
{
	NodeCompressedMotionStruct();
	~NodeCompressedMotionStruct();

	void SetFlavor(int flavor)  {Flavor = flavor;}

	int Flavor;

	union {
		struct {
			TimeCodedMotionChannelClass *		X;
			TimeCodedMotionChannelClass *		Y;
			TimeCodedMotionChannelClass *		Z;
			TimeCodedMotionChannelClass *		Q;
		} tc;

		struct {
			AdaptiveDeltaMotionChannelClass *		X;
			AdaptiveDeltaMotionChannelClass *		Y;
			AdaptiveDeltaMotionChannelClass *		Z;
			AdaptiveDeltaMotionChannelClass *		Q;

		} ad;

		struct {
			 void * X;
			 void * Y;
			 void * Z;
			 void * Q;
		} vd;
	};


	TimeCodedBitChannelClass *			Vis;
};

/***********************************************************************************************
 * NodeCompressedMotionStruct::NodeCompressedMotionStruct -- constructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
NodeCompressedMotionStruct::NodeCompressedMotionStruct() :
	Vis(NULL)
{
		vd.X = NULL;
		vd.Y = NULL;
		vd.Z = NULL;
		vd.Q = NULL;
}


/***********************************************************************************************
 * NodeCompressedMotionStruct::~NodeCompressedMotionStruct -- destructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/23/98   GTH : Created.                                                                 *
 *   02/02/00   JGA : Compressed                                                               *
 *=============================================================================================*/
NodeCompressedMotionStruct::~NodeCompressedMotionStruct()
{
	// Needs to be changed to call the correct destructors

	switch (Flavor) {
		case ANIM_FLAVOR_TIMECODED:
			delete tc.X;
			delete tc.Y;
			delete tc.Z;
			delete tc.Q;
			break;
		case ANIM_FLAVOR_ADAPTIVE_DELTA:
			delete ad.X;
			delete ad.Y;
			delete ad.Z;
			delete ad.Q;
			break;
		default:
			WWASSERT(0);	// unknown flavor
			break;
	}

	delete Vis;

}


/***********************************************************************************************
 * HCompressedAnimClass::HCompressedAnimClass -- constructor                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HCompressedAnimClass::HCompressedAnimClass(void) :
	NumFrames(0),
	NumNodes(0),
	Flavor(0),
	FrameRate(0),
	NodeMotion(NULL)
{
	memset(Name,0,W3D_NAME_LEN);
	memset(HierarchyName,0,W3D_NAME_LEN);
}


/***********************************************************************************************
 * HCompressedAnimClass::~HCompressedAnimClass -- Destructor                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HCompressedAnimClass::~HCompressedAnimClass(void)
{
	Free();
}


/***********************************************************************************************
 * HCompressedAnimClass::Free -- De-allocates all memory in use                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void HCompressedAnimClass::Free(void)
{
	delete[] NodeMotion;
	NodeMotion = NULL;
}


/***********************************************************************************************
 * HCompressedAnimClass::Load -- Loads hierarchy animation from a file                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int HCompressedAnimClass::Load_W3D(ChunkLoadClass & cload)
{
	int i = 0;
	/*
	** First make sure we release any memory in use
	*/
	Free();

	/*
	**	Open the first chunk, it should be the animation header
	*/
	if (!cload.Open_Chunk()) return LOAD_ERROR;

	if (cload.Cur_Chunk_ID() != W3D_CHUNK_COMPRESSED_ANIMATION_HEADER) {
		// ERROR: Expected Animation Header!
		return LOAD_ERROR;
	}

	W3dCompressedAnimHeaderStruct aheader;
	if (cload.Read(&aheader,sizeof(W3dAnimHeaderStruct)) != sizeof(W3dAnimHeaderStruct)) {
		return LOAD_ERROR;
	}

	cload.Close_Chunk();

	strcpy(Name,aheader.HierarchyName);
	strcat(Name,".");
	strcat(Name,aheader.Name);

	// TSS chasing crash bug 05/26/99
   WWASSERT(HierarchyName != NULL);
   WWASSERT(aheader.HierarchyName != NULL);
   WWASSERT(sizeof(HierarchyName) >= W3D_NAME_LEN);
   strlcpy(HierarchyName,aheader.HierarchyName,W3D_NAME_LEN);

	HTreeClass * base_pose = WW3DAssetManager::Get_Instance()->Get_HTree(HierarchyName);
	if (base_pose == NULL) {
		goto Error;
	}
	NumNodes = base_pose->Num_Pivots();

	NumFrames = aheader.NumFrames;
	FrameRate = aheader.FrameRate;
	Flavor    = aheader.Flavor;

	// Just for now
	WWASSERT((Flavor == ANIM_FLAVOR_TIMECODED)||(Flavor == ANIM_FLAVOR_ADAPTIVE_DELTA));

	NodeMotion = W3DNEWARRAY NodeCompressedMotionStruct[ NumNodes ];
	if (NodeMotion == NULL) {
		goto Error;
	}

	// Initialize Flavor
	for (i=0; i<NumNodes; i++) {
		NodeMotion[i].SetFlavor(Flavor);
	}

	/*
	** Now, read in all of the other chunks (motion channels).
	*/
	TimeCodedMotionChannelClass * tc_chan;
	AdaptiveDeltaMotionChannelClass * ad_chan;
	TimeCodedBitChannelClass * newbitchan;

	while (cload.Open_Chunk()) {

		switch (cload.Cur_Chunk_ID()) {

			case W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL:

				switch ( Flavor ) {

					case ANIM_FLAVOR_TIMECODED:

						if (!read_channel(cload,&tc_chan)) {
							goto Error;
						}
						if (tc_chan->Get_Pivot() < NumNodes) {
							add_channel(tc_chan);
						} else {
							// PWG 12-14-98: we have only allocated space for NumNode pivots.
							// If we have an index thats equal or higher than NumNode we are
							// gonna trash memory.  Boy will we trash memory.
							// GTH 09-25-2000: print a warning and survive this error
							delete tc_chan;
							WWDEBUG_SAY(("ERROR! animation %s indexes a bone not present in the model. Please re-export!",Name));
						}

						break;

					case ANIM_FLAVOR_ADAPTIVE_DELTA:
						if (!read_channel(cload,&ad_chan)) {
							goto Error;
						}
						if (ad_chan->Get_Pivot() < NumNodes) {
							add_channel(ad_chan);
						} else {
							// PWG 12-14-98: we have only allocated space for NumNode pivots.
							// If we have an index thats equal or higher than NumNode we are
							// gonna trash memory.  Boy will we trash memory.
							// GTH 09-25-2000: print a warning and survive this error
							delete ad_chan;
							WWDEBUG_SAY(("ERROR! animation %s indexes a bone not present in the model. Please re-export!",Name));
						}
						break;
				}
				break;

			case W3D_CHUNK_COMPRESSED_BIT_CHANNEL:
				if (!read_bit_channel(cload,&newbitchan)) {
					goto Error;
				}
				if (newbitchan->Get_Pivot() < NumNodes) {
					add_bit_channel(newbitchan);
				} else {
					// PWG 12-14-98: we have only allocated space for NumNode pivots.
					// If we have an index thats equal or higher than NumNode we are
					// gonna trash memory.  Boy will we trash memory.
					// GTH 09-25-2000: print a warning and survive this error
					delete newbitchan;
					WWDEBUG_SAY(("ERROR! animation %s indexes a bone not present in the model. Please re-export!",Name));
				}

				break;

			default:
				break;
		}
		cload.Close_Chunk();
	}

	return OK;

Error:

	Free();
	return LOAD_ERROR;

}

/***********************************************************************************************
 * HCompressedAnimClass::read_channel -- Reads in a single channel of motion                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool HCompressedAnimClass::read_channel(ChunkLoadClass & cload,TimeCodedMotionChannelClass * * newchan)
{
	*newchan = W3DNEW TimeCodedMotionChannelClass;
	bool result = (*newchan)->Load_W3D(cload);

	return result;

}

bool HCompressedAnimClass::read_channel(ChunkLoadClass & cload,AdaptiveDeltaMotionChannelClass * * newchan)
{
	*newchan = W3DNEW AdaptiveDeltaMotionChannelClass;
	bool result = (*newchan)->Load_W3D(cload);

	return result;

}


/***********************************************************************************************
 * HCompressedAnimClass::add_channel -- Adds a motion channel to the animation                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void HCompressedAnimClass::add_channel(TimeCodedMotionChannelClass * newchan)
{
	int idx = newchan->Get_Pivot();

	switch (newchan->Get_Type())
	{
		case ANIM_CHANNEL_X:
			NodeMotion[idx].tc.X = newchan;
			break;

		case ANIM_CHANNEL_Y:
			NodeMotion[idx].tc.Y = newchan;
			break;

		case ANIM_CHANNEL_Z:
			NodeMotion[idx].tc.Z = newchan;
			break;

		case ANIM_CHANNEL_Q:
			NodeMotion[idx].tc.Q = newchan;
			break;
	}

}

void HCompressedAnimClass::add_channel(AdaptiveDeltaMotionChannelClass * newchan)
{
	int idx = newchan->Get_Pivot();

	switch (newchan->Get_Type())
	{
		case ANIM_CHANNEL_X:
			NodeMotion[idx].ad.X = newchan;
			break;

		case ANIM_CHANNEL_Y:
			NodeMotion[idx].ad.Y = newchan;
			break;

		case ANIM_CHANNEL_Z:
			NodeMotion[idx].ad.Z = newchan;
			break;

		case ANIM_CHANNEL_Q:
			NodeMotion[idx].ad.Q = newchan;
			break;
	}

}




/***********************************************************************************************
 * HCompressedAnimClass::read_bit_channel -- read a bit channel from the file                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool HCompressedAnimClass::read_bit_channel(ChunkLoadClass & cload,TimeCodedBitChannelClass * * newchan)
{
	*newchan = W3DNEW TimeCodedBitChannelClass;
	bool result = (*newchan)->Load_W3D(cload);

	return result;

}


/***********************************************************************************************
 * HCompressedAnimClass::add_bit_channel -- install a bit channel into the animation           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void HCompressedAnimClass::add_bit_channel(TimeCodedBitChannelClass * newchan)
{
	int idx = newchan->Get_Pivot();

	switch (newchan->Get_Type())
	{
		case BIT_CHANNEL_VIS:
			NodeMotion[idx].Vis = newchan;
			break;
	}
}

/***********************************************************************************************
 * HCompressedAnimClass::Get_Translation -- returns the translation vector for the given frame *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void HCompressedAnimClass::Get_Translation( Vector3& trans, int pividx, float frame ) const
{
	struct NodeCompressedMotionStruct * motion = &NodeMotion[pividx];

	trans=Vector3(0,0,0);

	switch(Flavor) {
		case ANIM_FLAVOR_TIMECODED:
			if (motion->tc.X) motion->tc.X->Get_Vector(frame, &(trans[0]));
			if (motion->tc.Y) motion->tc.Y->Get_Vector(frame, &(trans[1]));
			if (motion->tc.Z) motion->tc.Z->Get_Vector(frame, &(trans[2]));
			break;
		case ANIM_FLAVOR_ADAPTIVE_DELTA:
			if (motion->ad.X) motion->ad.X->Get_Vector(frame, &(trans[0]));
			if (motion->ad.Y) motion->ad.Y->Get_Vector(frame, &(trans[1]));
			if (motion->ad.Z) motion->ad.Z->Get_Vector(frame, &(trans[2]));
			break;
		default:
			WWASSERT(0);	// unknown flavor
			break;
	}
}

/***********************************************************************************************
 * HCompressedAnimClass::Get_Orientation -- returns a quaternion for the orientation of the    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void HCompressedAnimClass::Get_Orientation(Quaternion& q, int pividx,float frame) const
{
	switch(Flavor) {
		case ANIM_FLAVOR_TIMECODED:
			if (NodeMotion[pividx].tc.Q) q = NodeMotion[pividx].tc.Q->Get_QuatVector(frame);
			else q.Make_Identity();
			break;
		case ANIM_FLAVOR_ADAPTIVE_DELTA:
			if (NodeMotion[pividx].ad.Q) q = NodeMotion[pividx].ad.Q->Get_QuatVector(frame);
			else q.Make_Identity();
			break;
		default:
			WWASSERT(0); // unknown flavor
			break;
	}
}

/***********************************************************************************************
 * HCompressedAnimClass::Get_Transform -- returns the transform matrix for the given frame	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void HCompressedAnimClass::Get_Transform( Matrix3D& mtx, int pividx, float frame ) const
{
	struct NodeCompressedMotionStruct * motion = &NodeMotion[pividx];

	switch(Flavor) {
		case ANIM_FLAVOR_TIMECODED:
			if (NodeMotion[pividx].tc.Q) {
				Quaternion q;
				q = NodeMotion[pividx].tc.Q->Get_QuatVector(frame);
				::Build_Matrix3D(q,mtx);
			}
			else mtx.Make_Identity();
			if (motion->tc.X) motion->tc.X->Get_Vector(frame, &(mtx[0][3]));
			if (motion->tc.Y) motion->tc.Y->Get_Vector(frame, &(mtx[1][3]));
			if (motion->tc.Z) motion->tc.Z->Get_Vector(frame, &(mtx[2][3]));
			break;
		case ANIM_FLAVOR_ADAPTIVE_DELTA:
			if (NodeMotion[pividx].ad.Q) {
				Quaternion q;
				q = NodeMotion[pividx].ad.Q->Get_QuatVector(frame);
				::Build_Matrix3D(q,mtx);
			}
			else mtx.Make_Identity();

			if (motion->ad.X) motion->ad.X->Get_Vector(frame, &(mtx[0][3]));
			if (motion->ad.Y) motion->ad.Y->Get_Vector(frame, &(mtx[1][3]));
			if (motion->ad.Z) motion->ad.Z->Get_Vector(frame, &(mtx[2][3]));
			break;
		default:
			WWASSERT(0);	// unknown flavor
			break;
	}
}

/***********************************************************************************************
 * HCompressedAnimClass::Get_Visibility -- return visibility state for given pivot/frame       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/19/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool HCompressedAnimClass::Get_Visibility(int pividx,float frame)
{

	if (NodeMotion[pividx].Vis != NULL) {
		return (NodeMotion[pividx].Vis->Get_Bit((int)frame) == 1);
	}


	// default to always visible...
	return 1;
}



/***********************************************************************************************
 * HAnimClass::Is_Node_Motion_Present -- return true if there is motion defined for this frame *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/23/99    EHC : Created.                                                                 *
 *=============================================================================================*/
bool HCompressedAnimClass::Is_Node_Motion_Present(int pividx)
{
	WWASSERT((pividx >= 0) && (pividx < NumNodes));

	if (NodeMotion[pividx].vd.X != NULL)	return true;
	if (NodeMotion[pividx].vd.Y != NULL)	return true;
	if (NodeMotion[pividx].vd.Z != NULL)	return true;
	if (NodeMotion[pividx].vd.Q  != NULL)	return true;
	if (NodeMotion[pividx].Vis != NULL)		return true;

	return false;
}

bool HCompressedAnimClass::Has_X_Translation (int pividx)
{
	WWASSERT((pividx >= 0) && (pividx < NumNodes));
	return NodeMotion[pividx].vd.X != NULL;
}

bool HCompressedAnimClass::Has_Y_Translation (int pividx)
{
	WWASSERT((pividx >= 0) && (pividx < NumNodes));
	return NodeMotion[pividx].vd.Y != NULL;
}

bool HCompressedAnimClass::Has_Z_Translation (int pividx)
{
	WWASSERT((pividx >= 0) && (pividx < NumNodes));
	return NodeMotion[pividx].vd.Z != NULL;
}

bool HCompressedAnimClass::Has_Rotation (int pividx)
{
	WWASSERT((pividx >= 0) && (pividx < NumNodes));
	return NodeMotion[pividx].vd.Q != NULL;
}

bool HCompressedAnimClass::Has_Visibility (int pividx)
{
	WWASSERT((pividx >= 0) && (pividx < NumNodes));
	return NodeMotion[pividx].Vis != NULL;
}


// eof - hcanim.cpp
