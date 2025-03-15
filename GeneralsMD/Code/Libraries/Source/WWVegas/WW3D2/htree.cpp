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

/* $Header: /Commando/Code/ww3d2/htree.cpp 14    10/01/01 6:07p Patrick $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D Library                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d2/htree.cpp                              $* 
 *                                                                                             * 
 *                       Author:: Greg_h                                                       * 
 *                                                                                             * 
 *                     $Modtime:: 10/01/01 6:06p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 14                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   HTreeClass::HTreeClass -- constructor                                                     *  
 *   HTreeClass::~HTreeClass -- destructor                                                     * 
 *   HTreeClass::Load -- loads a hierarchy tree from a file                                    * 
 *   HTreeClass::read_pivots -- reads the pivots out of a file                                 * 
 *   HTreeClass::Free -- de-allocate all memory in use                                         * 
 *   HTreeClass::Base_Update -- Computes the base pose transform for each pivot                * 
 *   HTreeClass::Anim_Update -- Computes the transform for each pivot with motion              * 
 *   HTreeClass::Blend_Update -- computes each pivot as a blend of two anims                   *
 *   HTreeClass::Combo_Update -- compute each pivot's transform using an anim combo            *
 *   HTreeClass::Get_Transform -- returns the transformation for the desired pivot             * 
 *   HTreeClass::Find_Bone -- Find a bone by name                                              *
 *   HTreeClass::Get_Bone_Name -- get the name of a bone from its index                        *
 *   HTreeClass::Update_Parent_Need_Bits -- all "needed" children force their parents to be nee*
 *   HTreeClass::HTreeClass -- copy constructor                                                *
 *   HTreeClass::Get_Parent_Index -- returns index of the parent of the given bone             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "htree.h"
#include "hanim.h"
#include "hcanim.h"
#include <string.h>
#include <assert.h>
#include "wwmath.h"
#include "chunkio.h"
#include "w3d_file.h"
#include "wwmemlog.h"
#include "hrawanim.h"
#include "motchan.h"

/*********************************************************************************************** 
 * HTreeClass::HTreeClass -- constructor                                                       * 
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
HTreeClass::HTreeClass(void) :
	NumPivots(0),
	Pivot(NULL),
	ScaleFactor(1.0f)
{
}

void HTreeClass::Init_Default(void)
{
	Free ();

	NumPivots = 1;
	Pivot = MSGW3DNEWARRAY("HTreeClass::Pivot") PivotClass[NumPivots];

	Pivot[0].Index = 0;
	Pivot[0].Parent = NULL;
	Pivot[0].BaseTransform.Make_Identity();
	Pivot[0].Transform.Make_Identity();
	Pivot[0].IsVisible = true;
	strcpy(Pivot[0].Name,"RootTransform");
	//::strcpy (Name, "Default");
	Name[0] = 0;
	return ;



}

/*********************************************************************************************** 
 * HTreeClass::~HTreeClass -- destructor                                                       * 
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
HTreeClass::~HTreeClass(void)
{
	Free();



}


/***********************************************************************************************
 * HTreeClass::HTreeClass -- copy constructor                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/98     GTH : Created.                                                                 *
 *=============================================================================================*/
HTreeClass::HTreeClass(const HTreeClass & src) :
	NumPivots(0),
	Pivot(NULL),
	ScaleFactor(1.0f)
{
	memcpy(&Name,&src.Name,sizeof(Name));

	NumPivots = src.NumPivots;
	if (NumPivots > 0) {
		Pivot = MSGW3DNEWARRAY("HTreeClass::Pivot") PivotClass[NumPivots];
	}

	for (int pi = 0; pi < NumPivots; pi++) {
		Pivot[pi] = src.Pivot[pi];
		
		if (src.Pivot[pi].Parent != NULL) {
			Pivot[pi].Parent = &(Pivot[src.Pivot[pi].Parent->Index]);
		} else {
			Pivot[pi].Parent = NULL;
		}
	}

	ScaleFactor = src.ScaleFactor;
}

/*********************************************************************************************** 
 * HTreeClass::Load -- loads a hierarchy tree from a file                                      * 
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
int HTreeClass::Load_W3D(ChunkLoadClass & cload)
{
	Free();

	/*
	**	Read the first chunk, it should be the hierarchy header
	*/
	if (!cload.Open_Chunk()) return LOAD_ERROR;

	if (cload.Cur_Chunk_ID() != W3D_CHUNK_HIERARCHY_HEADER) {
		// ERROR: Expected Hierarchy Header
		return LOAD_ERROR;
	}
	
	W3dHierarchyStruct header;
	if (cload.Read(&header,sizeof(W3dHierarchyStruct)) != sizeof(W3dHierarchyStruct)) {
		return LOAD_ERROR;
	}

	cload.Close_Chunk();

	/*
	** Check the version, if < 3.0 add a root node for everything
	** to attach to.  The load_pivots function will also have to be
	** notified of this.
	*/
	bool pre30 = false;
	if (header.Version < W3D_MAKE_VERSION(3,0)) {
		header.NumPivots ++;
		pre30 = true;
	}
	
	/*
	** Allocate the array of pivots
	*/
	memcpy(Name,header.Name,W3D_NAME_LEN);
	NumPivots = header.NumPivots;
	if (NumPivots > 0) {
		Pivot = MSGW3DNEWARRAY("HTreeClass::Pivot") PivotClass[NumPivots];
	}

	/*
	** Now, read in all of the other chunks for this hierarchy.
	*/

	while (cload.Open_Chunk()) {

		switch (cload.Cur_Chunk_ID()) {

			case W3D_CHUNK_PIVOTS:
				if (!read_pivots(cload,pre30)) {
					goto Error;
				}			
				break;

			default:
				// ERROR: expected W3D_CHUNK_PIVOTS!
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
 * HTreeClass::read_pivots -- reads the pivots out of a file                                   * 
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
bool HTreeClass::read_pivots(ChunkLoadClass & cload,bool pre30)
{
	W3dPivotStruct piv;
	Matrix3D mtx;
	
	int first_piv = 0;

	/*
	** At (w3d file format) version 3.0, I added a node for the root.  Pre-3.0 htrees didn't have
	** this so we just put one in.
	*/
	if (pre30) {
		Pivot[0].Index = 0;
		Pivot[0].Parent = NULL;
		Pivot[0].BaseTransform.Make_Identity();
		Pivot[0].Transform.Make_Identity();
		Pivot[0].IsVisible = true;
		strcpy(Pivot[0].Name,"RootTransform");
		first_piv++;
	}

	for (int pidx=first_piv; pidx < NumPivots; pidx++) {

		if (cload.Read(&piv,sizeof(W3dPivotStruct)) != sizeof(W3dPivotStruct)) {
			return false;
		}

		memcpy(Pivot[pidx].Name,piv.Name,W3D_NAME_LEN);
		Pivot[pidx].Index = pidx;		

		Pivot[pidx].BaseTransform.Make_Identity();
		Pivot[pidx].BaseTransform.Translate(Vector3(piv.Translation.X,piv.Translation.Y,piv.Translation.Z));

#ifdef ALLOW_TEMPORARIES
		Pivot[pidx].BaseTransform = 
			Pivot[pidx].BaseTransform * 
			Build_Matrix3D( 
				Quaternion(
							piv.Rotation.Q[0],
							piv.Rotation.Q[1],
							piv.Rotation.Q[2],
							piv.Rotation.Q[3]
				),
				mtx
			);
#else
		Pivot[pidx].BaseTransform.postMul( 
			Build_Matrix3D( 
				Quaternion(
							piv.Rotation.Q[0],
							piv.Rotation.Q[1],
							piv.Rotation.Q[2],
							piv.Rotation.Q[3]
				),
				mtx
			)
		);
#endif

		/*
		** At version 3.0 a root node was added, this "fixes up" pre-3.0 files
		** to have that root node
		*/
		if (pre30) {
			piv.ParentIdx += 1;
		}

		/*
		** Set the parent pointer.  The first pivot will have a parent index
		** of -1 (in post-3.0 files) so set its parent to NULL.
		*/
		if (piv.ParentIdx == -1) {
			Pivot[pidx].Parent = NULL;
			assert(pidx == 0);
		} else {
			Pivot[pidx].Parent = &(Pivot[piv.ParentIdx]);
		}

	}

	Pivot[0].Transform.Make_Identity();
	Pivot[0].IsVisible = true;

	return true;
}


/*********************************************************************************************** 
 * HTreeClass::Free -- de-allocate all memory in use                                           * 
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
void HTreeClass::Free(void)
{
	if (Pivot != NULL) {
		delete[] Pivot;
		Pivot = NULL;
	}
	NumPivots = 0;

	// Also clean up other members:
	ScaleFactor = 1.0f;
}


/*********************************************************************************************** 
 * HTreeClass::Simple_Evaluate_Pivot -- Returns the transform of a pivot at the given frame.	  *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/13/2000 PDS  : Created.                                                                * 
 *=============================================================================================*/
bool HTreeClass::Simple_Evaluate_Pivot
(	
	HAnimClass *		motion,
	int					pivot_index,
	float					frame,
	const Matrix3D &	obj_tm,
	Matrix3D *			end_tm
) const
{
	bool retval = false;
	end_tm->Make_Identity ();

	if (	motion != NULL &&
			end_tm != NULL &&
			pivot_index >= 0 &&
			pivot_index < NumPivots)
	{
		//
		//	Loop over the hierarchy of pivots that this pivot is
		// attached to and transform each.
		//
		for (	PivotClass *pivot = &Pivot[pivot_index];
				pivot != NULL && pivot->Parent != NULL;
				pivot = pivot->Parent)
		{
			//
			//	Build a matrix that represents the animation for this pivot
			//

			Matrix3D anim_tm;
			motion->Get_Transform(anim_tm, pivot->Index, frame);

//			Quaternion q;
//			motion->Get_Orientation (q, pivot->Index, frame);
//			Matrix3D anim_tm = ::Build_Matrix3D(q);

			Vector3 trans;
			anim_tm.Get_Translation(&trans);
			anim_tm.Set_Translation(trans * ScaleFactor);

			//
			//	Transform the animation transform by the 'relative-to-parent' transform.
			//
			Matrix3D curr_tm;
			Matrix3D::Multiply(pivot->BaseTransform, anim_tm, &curr_tm);

			//
			//	Transform the return value by this transform
			//
#ifdef ALLOW_TEMPORARIES
			Matrix3D::Multiply (curr_tm, *end_tm, end_tm);
#else
			end_tm->preMul(curr_tm);
#endif
		}

		//
		//	Transform the return value by the object's transform
		//
#ifdef ALLOW_TEMPORARIES
		Matrix3D::Multiply (obj_tm, *end_tm, end_tm);		
#else
			end_tm->preMul(obj_tm);
#endif

		// Success!
		retval = true;
	}	

	return retval;
}



/*********************************************************************************************** 
 * HTreeClass::Simple_Evaluate_Pivot -- Returns the transform of a pivot at the given frame.	  *
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/13/2000 PDS  : Created.                                                                * 
 *=============================================================================================*/
bool HTreeClass::Simple_Evaluate_Pivot
(	
	int					pivot_index,
	const Matrix3D &	obj_tm,
	Matrix3D *			end_tm
) const
{
	bool retval = false;
	end_tm->Make_Identity ();

	if (	end_tm != NULL &&
			pivot_index >= 0 &&
			pivot_index < NumPivots)
	{
		//
		//	Loop over the hierarchy of pivots that this pivot is
		// attached to and transform each.
		//
		for (	PivotClass *pivot = &Pivot[pivot_index];
				pivot != NULL && pivot->Parent != NULL;
				pivot = pivot->Parent)
		{
			//
			//	Build a matrix that represents the animation for this pivot
			//
			Matrix3D anim_tm (1);

			//
			//	Transform the animation transform by the 'relative-to-parent' transform.
			//
			Matrix3D curr_tm;
			Matrix3D::Multiply (pivot->BaseTransform, anim_tm, &curr_tm);

			//
			//	Transform the return value by this transform
			//
			Matrix3D::Multiply (curr_tm, *end_tm, end_tm);
		}

		//
		//	Transform the return value by the object's transform
		//
		Matrix3D::Multiply (obj_tm, *end_tm, end_tm);		
		retval = true;
	}	

	return retval;
}


/*********************************************************************************************** 
 * HTreeClass::Base_Update -- Computes the base pose transform for each pivot                  * 
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
void HTreeClass::Base_Update(const Matrix3D & root)
{
	PivotClass *pivot;

	Pivot[0].Transform = root;
	Pivot[0].IsVisible = true;

	for (int piv_idx=1; piv_idx < NumPivots; piv_idx++) {
		
		pivot = &Pivot[piv_idx];
		
		assert(pivot->Parent != NULL);
		Matrix3D::Multiply(pivot->Parent->Transform, pivot->BaseTransform, &(pivot->Transform));
		pivot->IsVisible = 1;

		if (pivot->Is_Captured()) pivot->Capture_Update();
	}
}

/*********************************************************************************************** 
 * HTreeClass::Anim_Update -- Computes the transform for each pivot with motion                * 
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
void HTreeClass::Anim_Update(const Matrix3D & root,HAnimClass * motion,float frame)
{
	PivotClass *pivot;
	Matrix3D mtx;

	Pivot[0].Transform = root;
	Pivot[0].IsVisible = true;

	int num_anim_pivots = motion->Get_Num_Pivots ();

	for (int piv_idx=1; piv_idx < NumPivots; piv_idx++) {
		pivot = &Pivot[piv_idx];

		// base pose
		assert(pivot->Parent != NULL);
		Matrix3D::Multiply(pivot->Parent->Transform, pivot->BaseTransform, &(pivot->Transform));
			
		// Don't update this pivot if the HTree doesn't have animation data for it...
		if (piv_idx < num_anim_pivots) {
			
			// animation
			Vector3 trans;
			motion->Get_Translation(trans,piv_idx,frame);
			pivot->Transform.Translate(trans * ScaleFactor);

			Quaternion q;
			motion->Get_Orientation(q,piv_idx,frame);
			::Build_Matrix3D(q,mtx);

#ifdef ALLOW_TEMPORARIES
			pivot->Transform = pivot->Transform * mtx;
#else
			pivot->Transform.postMul(mtx);
#endif

			// visibility
			pivot->IsVisible = motion->Get_Visibility(piv_idx,frame);
		}

		if (pivot->Is_Captured()) 
		{ 
			pivot->Capture_Update();
			pivot->IsVisible = true;
		} 
	}
}
								
/*Customized version of the above which excludes interpolation and assumes HRawAnimClass
For use by 'Generals' -MW*/
void HTreeClass::Anim_Update(const Matrix3D & root,HRawAnimClass * motion,float frame)
{
	PivotClass *pivot,*endpivot,*lastAnimPivot;

	Pivot[0].Transform = root;
	Pivot[0].IsVisible = true;


	int num_anim_pivots = motion->Get_Num_Pivots ();

	//Get integer frame
	int iframe=WWMath::Float_To_Long(frame);
	if (iframe >= motion->Get_Num_Frames()) 
		iframe = 0;

	Vector3 trans;
	Quaternion q;
	Matrix3D mtx;

	struct NodeMotionStruct * nodeMotion = ((HRawAnimClass*)motion)->Get_Node_Motion_Array();
	nodeMotion += 1;	//skip the root node

	pivot = &Pivot[1];
	endpivot=pivot+(NumPivots-1);
	lastAnimPivot = &Pivot[num_anim_pivots];

	for (int piv_idx=1; pivot < endpivot; pivot++,nodeMotion++) {

		// base pose
		assert(pivot->Parent != NULL);
		Matrix3D::Multiply(pivot->Parent->Transform, pivot->BaseTransform, &(pivot->Transform));
			
		// Don't update this pivot if the HTree doesn't have animation data for it...
		if (pivot < lastAnimPivot)
		{
			
			// animation
			trans.Set(0.0f,0.0f,0.0f);
			Matrix3D *xform=&pivot->Transform;

			if (nodeMotion->X != NULL)
				nodeMotion->X->Get_Vector(iframe,&(trans[0]));
			if (nodeMotion->Y != NULL)
				nodeMotion->Y->Get_Vector(iframe,&(trans[1]));
			if (nodeMotion->Z != NULL)
				nodeMotion->Z->Get_Vector(iframe,&(trans[2]));

			if (ScaleFactor == 1.0f)
				xform->Translate(trans);
			else
				xform->Translate(trans*ScaleFactor);

			if (nodeMotion->Q != NULL) 
			{	nodeMotion->Q->Get_Vector_As_Quat(iframe, q);
#ifdef ALLOW_TEMPORARIES
				*xform = *xform * ::Build_Matrix3D(q,mtx);
#else
				xform->postMul(::Build_Matrix3D(q,mtx));
#endif
			}

			// visibility
			if (nodeMotion->Vis != NULL)
				pivot->IsVisible=(nodeMotion->Vis->Get_Bit(iframe) == 1);
			else
				pivot->IsVisible=1;
		}

		if (pivot->Is_Captured()) 
		{ 
			pivot->Capture_Update();
			pivot->IsVisible = true;
		} 
	}
}								


/***********************************************************************************************
 * HTreeClass::Blend_Update -- computes each pivot as a blend of two anims                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void HTreeClass::Blend_Update
(
	const Matrix3D &					root,
	HAnimClass *						motion0,
	float									frame0,
	HAnimClass *						motion1,
	float									frame1,
	float									percentage		// 0.0 = motion0.  1.0 = motion1
)
{
	PivotClass *pivot;
	Matrix3D mtx;

	Pivot[0].Transform = root;
	Pivot[0].IsVisible = true;

	int num_anim_pivots = MIN( motion0->Get_Num_Pivots (), motion1->Get_Num_Pivots () );

	for (int piv_idx=1; piv_idx < NumPivots; piv_idx++) {

		pivot = &Pivot[piv_idx];

		assert(pivot->Parent != NULL);
		Matrix3D::Multiply(pivot->Parent->Transform,pivot->BaseTransform,&(pivot->Transform));

		if (piv_idx < num_anim_pivots) {
			// interpolated translation
			Vector3 trans0;
			motion0->Get_Translation(trans0,piv_idx,frame0);
			Vector3 trans1;
			motion1->Get_Translation(trans1,piv_idx,frame1);
			Vector3 lerped = (1.0 - percentage) * trans0 + (percentage) * trans1;
			pivot->Transform.Translate(lerped * ScaleFactor);

			// interpolated rotation
			Quaternion q0;
			motion0->Get_Orientation(q0,piv_idx,frame0);
			Quaternion q1;
			motion1->Get_Orientation(q1,piv_idx,frame1);
			Quaternion q;
			Fast_Slerp(q,q0,q1,percentage);
#ifdef ALLOW_TEMPORARIES
			pivot->Transform = pivot->Transform * Build_Matrix3D(q);
#else
			pivot->Transform.postMul(Build_Matrix3D(q,mtx));
#endif

			pivot->IsVisible = (motion0->Get_Visibility(piv_idx,frame0) || motion1->Get_Visibility(piv_idx,frame1));
		}

		if (pivot->Is_Captured()) 
		{
			pivot->Capture_Update();
			pivot->IsVisible = true;
		} 
	}
}																							



/***********************************************************************************************
 * HTreeClass::Combo_Update -- compute each pivot's transform using an anim combo              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/4/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void HTreeClass::Combo_Update
(
	const Matrix3D & root,
	HAnimComboClass *anim
)
{
	PivotClass *pivot;
	Matrix3D mtx;

	Pivot[0].Transform = root;
	Pivot[0].IsVisible = true;
	
	int num_anim_pivots = 100000;
	for ( int anim_num = 0; anim_num < anim->Get_Num_Anims(); anim_num++ ) {
		num_anim_pivots = MIN( num_anim_pivots, anim->Peek_Motion( anim_num )->Get_Num_Pivots() );
	}
	if ( num_anim_pivots == 100000 ) {
		num_anim_pivots = 0;
	}

	for (int piv_idx=1; piv_idx < NumPivots; piv_idx++) {
		
		pivot = &Pivot[piv_idx];
		assert(pivot->Parent != NULL);
		Matrix3D::Multiply(pivot->Parent->Transform,pivot->BaseTransform,&(pivot->Transform));
		
		if (piv_idx < num_anim_pivots) {

#define	ASSUME_NORMALIZED_ANIM_COMBO_WEIGHTS

			Vector3 trans(0,0,0);
			Quaternion q0;
			Quaternion q1;
#ifndef ASSUME_NORMALIZED_ANIM_COMBO_WEIGHTS
			float	last_weight = 0;
#endif
			float	weight_total = 0;
			int wcount = 0;

			int anim_num = 0;
			for ( ; anim_num < anim->Get_Num_Anims(); anim_num++ ) {

				HAnimClass *motion = anim->Get_Motion( anim_num );

				if ( motion != NULL ) {

					float frame_num = anim->Get_Frame( anim_num );

					PivotMapClass * pivot_map = anim->Get_Pivot_Weight_Map( anim_num );

					//float	*pivot_map = anim->Get_Pivot_Weight_Map( anim_num );

					float	weight = anim->Get_Weight( anim_num );

					if ( pivot_map != NULL ) {
						weight *= (*pivot_map)[piv_idx];
						// GREG - Pivot maps are ref counted so shouldn't we
						// release the rivot map here?
						pivot_map->Release_Ref();
					}

					if ( weight != 0.0 ) {

						wcount++;
						Vector3 temp_trans;
						motion->Get_Translation( temp_trans, piv_idx, frame_num );
						trans += weight * ScaleFactor * temp_trans;
						weight_total += weight;

#ifdef ASSUME_NORMALIZED_ANIM_COMBO_WEIGHTS
						motion->Get_Orientation(q1,piv_idx, frame_num );
						if ( wcount == 1 ) {
							q0 = q1;
						} else {
							Fast_Slerp(q0, q0, q1, weight / weight_total );
						}
#else
						q0 = q1;	
						motion->Get_Orientation(q1, piv_idx, frame_num );
						last_weight = weight;
#endif
					}

					motion->Release_Ref();

				}
			}

#ifdef ASSUME_NORMALIZED_ANIM_COMBO_WEIGHTS

			if (weight_total != 0.0f ) {
				// SKB: Removed assert because I have a case where I don't want normalization.
				// 	  One anim moves X, the other moves Y.  Assert was just in to warn programmers.	
//				WWASSERT(WWMath::Fabs( weight_total - 1.0 ) < WWMATH_EPSILON);

				pivot->Transform.Translate(trans);
#ifdef ALLOW_TEMPORARIES
				pivot->Transform = pivot->Transform * Build_Matrix3D(q0);
#else
				pivot->Transform.postMul(Build_Matrix3D(q0,mtx));
#endif
			}
#else
			if (( weight_total != 0.0f ) && (wcount >= 2)) {
			
				pivot->Transform.Translate( trans / weight_total );
				Quaternion q = Slerp_( q0, q1, last_weight / weight_total );
				pivot->Transform = pivot->Transform * Build_Matrix3D(q);

			} else if (weight_total != 0.0f) {

				pivot->Transform.Translate( trans / weight_total );
				pivot->Transform = pivot->Transform * Build_Matrix3D(q1);
			}
#endif

			pivot->IsVisible = false;

			for ( anim_num = 0; (anim_num < anim->Get_Num_Anims()) && (!pivot->IsVisible); anim_num++ ) {
				HAnimClass *motion = anim->Get_Motion( anim_num );
				if ( motion != NULL ) {
					float frame_num = anim->Get_Frame( anim_num );

					pivot->IsVisible |= motion->Get_Visibility(piv_idx,frame_num);

					motion->Release_Ref();
				}
			}
		}

		if (pivot->Is_Captured()) 
		{
			pivot->Capture_Update();
			pivot->IsVisible = true;
		}
	}
}						 


/***********************************************************************************************
 * HTreeClass::Find_Bone -- Find a bone by name                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/4/97    GTH : Created.                                                                 *
 *=============================================================================================*/
int HTreeClass::Get_Bone_Index(const char * name) const
{
	for (int i=0; i < NumPivots; i++) {
		if (stricmp(Pivot[i].Name,name) == 0) {
			return i;
		}
	}
	return 0;
}


/***********************************************************************************************
 * HTreeClass::Get_Bone_Name -- get the name of a bone from its index                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/4/97    GTH : Created.                                                                 *
 *=============================================================================================*/
const char * HTreeClass::Get_Bone_Name(int boneidx) const
{
	assert(boneidx >= 0);
	assert(boneidx < NumPivots);

	return Pivot[boneidx].Name;
}


/***********************************************************************************************
 * HTreeClass::Get_Parent_Index -- returns index of the parent of the given bone               *
 *                                                                                             *
 * INPUT:                                                                                      *
 * boneidx - the bone you are interested in                                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * the index of that bone's parent                                                             *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/12/2000  gth : Created.                                                                 *
 *=============================================================================================*/
int HTreeClass::Get_Parent_Index(int boneidx) const
{
	assert(boneidx >= 0);
	assert(boneidx < NumPivots);

	if (Pivot[boneidx].Parent != NULL) {
		return Pivot[boneidx].Parent->Index;
	} else {
		return 0;
	}
}


// Scale this HTree by a constant factor:
void HTreeClass::Scale(float factor)
{
	if (factor == 1.0f) return;

	// Scale pivot translations
	for (int i = 0; i < NumPivots; i++) {
		Matrix3D &pivot_transform = Pivot[i].BaseTransform;
		Vector3 pivot_translation;
		pivot_transform.Get_Translation(&pivot_translation);
		pivot_translation *= factor;
		pivot_transform.Set_Translation(pivot_translation);
	}

	// Set state used later to scale animations:
	ScaleFactor *= factor;
}



void HTreeClass::Capture_Bone(int boneindex)
{
	assert(boneindex >= 0);
	assert(boneindex < NumPivots);
#ifdef LAZY_CAP_MTX_ALLOC
	if (Pivot[boneindex].CapTransformPtr == NULL) 
	{
		Pivot[boneindex].CapTransformPtr = MSGW3DNEW("PivotClassCaptureBoneMtx") DynamicMatrix3D;
		Pivot[boneindex].CapTransformPtr->Mat.Make_Identity();
	}
#else
	Pivot[boneindex].IsCaptured = true;
#endif
}

void HTreeClass::Release_Bone(int boneindex)
{
	assert(boneindex >= 0);
	assert(boneindex < NumPivots);
#ifdef LAZY_CAP_MTX_ALLOC
	if (Pivot[boneindex].CapTransformPtr) 
	{
		delete Pivot[boneindex].CapTransformPtr;
		Pivot[boneindex].CapTransformPtr = NULL;
	}
#else
	Pivot[boneindex].IsCaptured = false;
#endif
}

bool HTreeClass::Is_Bone_Captured(int boneindex) const
{
	assert(boneindex >= 0);
	assert(boneindex < NumPivots);
	return Pivot[boneindex].Is_Captured();
}

void HTreeClass::Control_Bone(int boneindex,const Matrix3D & relative_tm,bool world_space_translation)
{
	assert(boneindex >= 0);
	assert(boneindex < NumPivots);
	assert(Pivot[boneindex].Is_Captured());

#ifdef LAZY_CAP_MTX_ALLOC
	if (Pivot[boneindex].CapTransformPtr == NULL)
		return;
	Pivot[boneindex].WorldSpaceTranslation = world_space_translation;
	Pivot[boneindex].CapTransformPtr->Mat = relative_tm;
#else
	Pivot[boneindex].WorldSpaceTranslation = world_space_translation;
	Pivot[boneindex].CapTransform = relative_tm;
#endif
}

void HTreeClass::Get_Bone_Control(int boneindex, Matrix3D & relative_tm) const
{
	assert(boneindex >= 0);
	assert(boneindex < NumPivots);

	//
	//	Return the bone's control transform to the caller
	//
	if (Pivot[boneindex].IsCaptured) {
		relative_tm = Pivot[boneindex].CapTransform;
	} else {
		relative_tm.Make_Identity ();
	}

	return ;
}

HTreeClass * HTreeClass::Alter_Avatar_HTree( const HTreeClass *tree, Vector3 &scale)
{
	// This is a specific list of pivot names used in the avatar meshes that we need to special case for scaling
	// The reason is due to the fact that we want to scale the avatar's bone structure with differing amount for
	// each axis, and with the T-pos of the avatar skeleton, undesirable results are caused due to the arms and hands
	// being stretched out on the Y-axis instead of the Z-axis like the rest of the bodies. Hence, the list of pivots
	// below are ones that I will special case and scale them based on the Z-axis scaling factor instead of the Y-axis
	// scaling factor.
	const char * flip_list[] = { " RIGHTFOREARM", " RIGHTHAND", " LEFTFOREARM", " LEFTHAND", "RIGHTINDEX", "RIGHTFINGERS", "RIGHTTHUMB", "LEFTINDEX", "LEFTFINGERS", "LEFTTHUMB", 0 };
		
	// Clone the new tree with the tree that is passed in
	HTreeClass * new_tree = new HTreeClass( *tree );

	// Go through each of the pivots and calculate and transform the pivots to match the desired scaling factor
	for(int pi = 0; pi < new_tree->NumPivots; ++pi) {
		PivotClass piv = tree->Pivot[pi];
		Vector3 adjusted_scale = scale;
		
		// If there is no parent, skip
		if(!piv.Parent) continue;

		// If the pivot is on the flip list, use the Z scale to scale both the Z & Y axis 
		int index = 0;
		while(true) {
			if(!flip_list[index]) {
				break;
			} else if(strcmp(piv.Name, flip_list[index]) == 0) {
				adjusted_scale.Y = scale.Z;
				break;
			}
			++index;
		}

		// Get the positions of the pivot and the pivot's parent in worldspace & apply the altering scale to it
		Vector3 pivot_pos = piv.Transform.Get_Translation();
		Vector3 pivot_parent_pos = piv.Parent->Transform.Get_Translation();
		pivot_pos.Scale(adjusted_scale);
		pivot_parent_pos.Scale(adjusted_scale);

		// Get the pivot's parent's inverse transform
		Matrix3D parent_inverse_transform;
		piv.Parent->Transform.Get_Inverse(parent_inverse_transform);

		// Get the new desired vector in worldspace
		Vector3 new_relative_vector = pivot_pos - pivot_parent_pos;

		// Rotate the new vector by the pivot's parent's inverse transform to put it in local space 
		new_relative_vector = parent_inverse_transform.Rotate_Vector(new_relative_vector);

		// Store the final result in the new HTree
		new_tree->Pivot[pi].BaseTransform.Set_Translation( new_relative_vector );
	}

	return new_tree;
}

// Morph the bones on the HTree using weights from a number of other HTrees
HTreeClass * HTreeClass::Create_Morphed(	int num_morph_sources,
														const float morph_weights[],
														const HTreeClass *tree_array[] )
{
	int i;
	assert(num_morph_sources>0);
	for(i=0;i<num_morph_sources;i++) {
		assert( tree_array[i] );
		assert( morph_weights[i]>=0.0f && morph_weights[i]<=1.0f );
	}
	for(i=0;i<num_morph_sources-1;i++) {
		assert( tree_array[i]->NumPivots == tree_array[i+1]->NumPivots );
	}

	// Clone the first one,
	HTreeClass * new_tree = W3DNEWARRAY HTreeClass( *tree_array[0] );

	// Then interpolate all the pivots translations
	for (int pi = 0; pi < new_tree->NumPivots; pi++) {

		Vector3 pos(0.0f,0.0f,0.0f);
		for(int nm = 0; nm < num_morph_sources; nm++) {
			pos.X += tree_array[nm]->Pivot[pi].BaseTransform.Get_Translation().X*morph_weights[nm];
			pos.Y += tree_array[nm]->Pivot[pi].BaseTransform.Get_Translation().Y*morph_weights[nm];
			pos.Z += tree_array[nm]->Pivot[pi].BaseTransform.Get_Translation().Z*morph_weights[nm];
		}

		new_tree->Pivot[pi].BaseTransform.Set_Translation( pos );
	}

	return new_tree;
}

// Create an HTree by Interpolating between others
HTreeClass * HTreeClass::Create_Interpolated(	const HTreeClass * tree_a0_b0, 
																const HTreeClass * tree_a0_b1, 
																const HTreeClass * tree_a1_b0, 
																const HTreeClass * tree_a1_b1, 
																float lerp_a, float lerp_b )
{
	assert( tree_a0_b0->NumPivots == tree_a0_b1->NumPivots );
	assert( tree_a0_b0->NumPivots == tree_a1_b0->NumPivots );
	assert( tree_a0_b0->NumPivots == tree_a1_b1->NumPivots );

	// Clone the first one,
	HTreeClass * new_tree = W3DNEW HTreeClass( *tree_a0_b0 );

	// Then interpolate all the pivots translations
	Vector3 pos_a0, pos_a1, pos;
	for (int pi = 0; pi < new_tree->NumPivots; pi++) {

		Vector3::Lerp( tree_a0_b0->Pivot[pi].BaseTransform.Get_Translation(),
							  		  tree_a0_b1->Pivot[pi].BaseTransform.Get_Translation(),
									  lerp_b, &pos_a0 );
		Vector3::Lerp( tree_a1_b0->Pivot[pi].BaseTransform.Get_Translation(),
									  tree_a1_b1->Pivot[pi].BaseTransform.Get_Translation(),
									  lerp_b, &pos_a1 );
		Vector3::Lerp( pos_a0, pos_a1, lerp_a, &pos );

		new_tree->Pivot[pi].BaseTransform.Set_Translation( pos );
	}

	return new_tree;
}

// Create an HTree by Interpolating between others
HTreeClass * HTreeClass::Create_Interpolated(const HTreeClass * tree_base, 
														   const HTreeClass * tree_a, 
														   const HTreeClass * tree_b, 
														   float a_scale, float b_scale )
{
	WWMEMLOG(MEM_ANIMATION);
	assert( tree_base->NumPivots == tree_a->NumPivots );
	assert( tree_base->NumPivots == tree_b->NumPivots );

	// Clone the first one,
	HTreeClass * new_tree = W3DNEW HTreeClass( *tree_base );

	float	a_scale_abs = WWMath::Fabs( a_scale );
	float	b_scale_abs = WWMath::Fabs( b_scale );

	if ( a_scale_abs + b_scale_abs > 0 ) {

		// Then interpolate all the pivots translations
		Vector3 pos_a, pos_b, pos;
		for (int pi = 0; pi < new_tree->NumPivots; pi++) {

			Vector3::Lerp( tree_base->Pivot[pi].BaseTransform.Get_Translation(),
							  			 tree_a->Pivot[pi].BaseTransform.Get_Translation(),
										 a_scale, &pos_a );
			Vector3::Lerp( tree_base->Pivot[pi].BaseTransform.Get_Translation(),
										 tree_b->Pivot[pi].BaseTransform.Get_Translation(),
										 b_scale, &pos_b );

			pos   = (pos_a * a_scale_abs + pos_b * b_scale_abs ) / ( a_scale_abs + b_scale_abs );

			new_tree->Pivot[pi].BaseTransform.Set_Translation( pos );
		}
	}

	return new_tree;
}

