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

/* $Header: /Commando/Code/Tools/max2w3d/motion.cpp 26    10/30/00 6:56p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/motion.cpp                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/30/00 5:25p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 26                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   MotionClass::MotionClass -- constructor                                                   * 
 *   MotionClass::MotionClass -- constructor                                                   * 
 *   MotionClass::init -- initialize                                                           * 
 *   MotionClass::~MotionClass -- destructor                                                   * 
 *   MotionClass::compute_frame_motion -- compute the motion for a specified frame             * 
 *   MotionClass::set_motion_matrix -- save a motin matrix                                     * 
 *   MotionClass::get_motion_matrix -- retrieve a motion matrix                                * 
 *   MotionClass::set_eulers -- store euler angles                                             * 
 *   MotionClass::get_eulers -- retrieve euler angles                                          * 
 *   MotionClass::Save -- save the motion to a W3D file                                        * 
 *   MotionClass::save_header -- save the header                                               * 
 *   MotionClass::save_channels -- save the motion channels                                    * 
 *   MotionClass::set_visibility -- store a visibility bit                                     *
 *   MotionClass::get_visibility -- retrieve the visibility bit for this node:frame            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "motion.h"
#include "w3d_file.h"
#include "vchannel.h"
#include "bchannel.h"
#include "EULER.H"
#include "util.h"
#include "errclass.h"
#include "w3dutil.h"
#include "exportlog.h"



/*********************************************************************************************** 
 * MotionClass::MotionClass -- constructor                                                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
MotionClass::MotionClass
(
	IScene * scene,
	INode * rootnode,
	HierarchySaveClass * basepose,
   W3dExportOptionsStruct & options,
	int framerate,
	Progress_Meter_Class * meter,
   HWND MaxHwnd,
	char * name,
	Matrix3 & offset
):
	BasePose(basepose),
	Scene(scene),
	RootNode(rootnode),
	RootList(NULL),
	StartFrame(options.StartFrame),
	EndFrame(options.EndFrame),
	ReduceAnimation(options.ReduceAnimation),
	ReduceAnimationPercent(options.ReduceAnimationPercent),
	CompressAnimation(options.CompressAnimation),
	CompressAnimationFlavor(options.CompressAnimationFlavor),
	CompressAnimationTranslationError(options.CompressAnimationTranslationError),
	CompressAnimationRotationError(options.CompressAnimationRotationError),
	FrameRate(framerate),
	Meter(meter),
	Offset(offset)
{		
	
	ExportLog::printf("Initializing Capture....\n");
   
	init();

	Set_W3D_Name(Name,name);
}

/*********************************************************************************************** 
 * MotionClass::MotionClass -- constructor                                                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
MotionClass::MotionClass
(
	IScene * scene,
	INodeListClass * rootlist,
	HierarchySaveClass * basepose,
   W3dExportOptionsStruct & options,
	int framerate,
	Progress_Meter_Class * meter,
   HWND MaxHwnd,
	char * name,
	Matrix3 & offset
):
	BasePose(basepose),
	Scene(scene),
	RootNode(NULL),
	RootList(rootlist),
	StartFrame(options.StartFrame),
	EndFrame(options.EndFrame),
	ReduceAnimation(options.ReduceAnimation),
	ReduceAnimationPercent(options.ReduceAnimationPercent),
	CompressAnimation(options.CompressAnimation),
	CompressAnimationFlavor(options.CompressAnimationFlavor),
	CompressAnimationTranslationError(options.CompressAnimationTranslationError),
	CompressAnimationRotationError(options.CompressAnimationRotationError),
	FrameRate(framerate),
	Meter(meter),
	Offset(offset)
{						
	
	ExportLog::printf("Initializing Capture....\n");

	init();
   										
	Set_W3D_Name(Name,name);
}

/*********************************************************************************************** 
 * MotionClass::init -- initialize                                                             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void MotionClass::init(void)
{
	int i,j;

	NumFrames = (EndFrame - StartFrame + 1);

	ExportLog::printf("Extracting %d frames of animation from Max\n", NumFrames);
	ExportLog::printf("\n");

	/*
	** Allocate space for a matrix per frame per node
	** and an XYZEulers per frame per node.
	*/
	MotionMatrix = new Matrix3 * [BasePose->Num_Nodes()];
	if (MotionMatrix == NULL) {
		throw (ErrorClass("Out of Memory"));
	}

	EulerDelta = new Point3 * [BasePose->Num_Nodes()];
	if (EulerDelta == NULL) {
		throw (ErrorClass("Out of Memory"));
	}
	
	for (i=0; i<BasePose->Num_Nodes(); i++) {
		MotionMatrix[i] = new Matrix3[NumFrames];
		if (MotionMatrix[i] == NULL) {
			throw (ErrorClass("Out of Memory"));
		}
		
		/*
		** Initialize all of the matrices to identity.
		*/
		for (j=0; j<NumFrames; j++) {
			MotionMatrix[i][j] = Matrix3(1);
		}
	}

	for (i=0; i<BasePose->Num_Nodes(); i++) {
		EulerDelta[i] = new Point3[NumFrames];
		if (EulerDelta[i] == NULL) {
			throw (ErrorClass("Out of Memory"));
		}

		/*
		** Initialize all of the Euler angles to 0,0,0
		*/
		for (j=0; j<NumFrames; j++) {
			EulerDelta[i][j] = Point3(0,0,0);
		}
	}

	/*
	** allocate boolean vectors for the visiblity data
	*/
	VisData = new BooleanVectorClass[BasePose->Num_Nodes()];

	for (i=0; i<BasePose->Num_Nodes(); i++) {
		VisData[i].Resize(NumFrames);

		/* 
		** initialize to always visible
		*/
		for (j=0; j<NumFrames; j++) {
			VisData[i][j] = true;
		}
	}

	//
	// allocate boolean vectors for movement data
	//
	BinMoveData = new BooleanVectorClass[BasePose->Num_Nodes()];

	for (i=0; i<BasePose->Num_Nodes(); i++) {
		BinMoveData[i].Resize(NumFrames);

		/* 
		** initialize to always interpolate
		*/
		for (j=0; j<NumFrames; j++) {
			BinMoveData[i][j] = false;
		}
	}


	/*
	** Allocate a bit for each node in the base pose.  These
	** bits indicate whether the node actually appeared in the
	** scene.  If the bit is zero after all of the animation
	** has been processed, the node can be ignored.
	*/
	NodeValidFlags.Resize(BasePose->Num_Nodes());

	for (i=0; i<BasePose->Num_Nodes(); i++) {
		NodeValidFlags[i] = 0;
	}

	/*
	** Compute motion data for each frame
	*/
	for (i=0; i < NumFrames; i++) {
		ExportLog::rprintf("( %d ) ", i);
		ExportLog::updatebar(i, NumFrames);
		compute_frame_motion(i);
	}

	ExportLog::updatebar(1, 1);	// 100%
	ExportLog::rprintf("Extraction Complete.\n");
}



/*********************************************************************************************** 
 * MotionClass::~MotionClass -- destructor                                                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
MotionClass::~MotionClass(void)
{
	int i;

	for (i=0; i<BasePose->Num_Nodes(); i++) {
		if (MotionMatrix[i]) delete[] MotionMatrix[i];
	}
	if (MotionMatrix) {
		delete[] MotionMatrix;
	}

	for (i=0; i<BasePose->Num_Nodes(); i++) {
		if (EulerDelta[i]) delete[] EulerDelta[i];
	}
	if (EulerDelta) {
		delete[] EulerDelta;
	}

	if (VisData) {
		delete[] VisData;
	}

	if (BinMoveData) {
		delete[] BinMoveData;
	}		
   
 	ExportLog::printf("Destroy Log..%d,%d,%d,%d, %s..\n",1,2,3,4,"go");

}


/*********************************************************************************************** 
 * MotionClass::compute_frame_motion -- compute the motion for a specified frame               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void MotionClass::compute_frame_motion(int frame)
{
	/*
	** Compute MAX's time value for this frame 
	** NOTE: the frame index passed in is the offset from StartFrame
	** to get the original MAX frame number, we add StartFrame.
	*/
	TimeValue frametime = (StartFrame + frame) * GetTicksPerFrame();
	
	/*
	** Create a hierarchy tree object for the scene at this frame
	*/
	HierarchySaveClass * tree;

	if (RootNode != NULL) {
		tree = new HierarchySaveClass(RootNode,frametime,*Meter,"NoName",false,BasePose);
	} else {
		tree = new HierarchySaveClass(RootList,frametime,*Meter,"NoName",false,BasePose,Offset);
	}
		
	if (tree == NULL) {
		throw (ErrorClass("Out of memory!"));
	}

	/*
	** Loop over each node in this frame's tree
	*/
	for (int tindex=0; tindex<tree->Num_Nodes(); tindex++) {

		/*
		** Find the node in the Base Pose corresponding to this node.
		** If this node is not in the base pose, skip
		*/
		int bindex = BasePose->Find_Named_Node(tree->Get_Node_Name(tindex));

		if (bindex != -1) {

			/*
			** Get the relative transform from the base and from
			** this frame's tree.  Assume that both have already been "fixed";
			** obviously the base pose has... However, the current tree
			** needs to be built by passing the basepose in as the "fixup tree"
			**
			** What are the "fixup" matrices?  These are the transforms which
			** were applied to the base pose when the user wanted to force the
			** base pose to use only matrices with certain properties.  For 
			** example, if we wanted the base pose to use translations only,
			** the fixup transform for each node is a transform which when
			** multiplied by the real node's world transform, yeilds a pure
			** translation matrix.  Fixup matrices also show up in the mesh
			** exporter since all vertices must be transformed by their inverses
			** in order to make things work...
			*/
			Matrix3 basetm = BasePose->Get_Node_Relative_Transform(bindex);
			Matrix3 thistm = tree->Get_Node_Relative_Transform(tindex);
			INode *tree_node = tree->Get_Node(tindex);
			
			Matrix3 motion = thistm * Inverse(basetm);

			motion = Cleanup_Orthogonal_Matrix(motion);

			set_motion_matrix(bindex,frame,motion);

			/*
			** Also, store the Euler angles for this node
			*/
			EulerAnglesClass my_eulers(motion,EulerOrderXYZr);
			float ex = my_eulers.Get_Angle(0);
			float ey = my_eulers.Get_Angle(1);
			float ez = my_eulers.Get_Angle(2);

			set_eulers(bindex,frame,ex,ey,ez);							

			/*
			** Store the visibility bit for this node
			*/
			INode * node = tree->Get_Node(tindex);
			bool vis;
			if (node) {
				vis = (node->GetVisibility(frametime) > 0.0f);
			} else {
				vis = 1;
			}
			set_visibility(bindex,frame,vis);

			//
			// Store out binary move or not
			//
			bool binary_move = false;
			  
         if ((node)&&(vis))  {
         	
	         if (frame != 0) {
					// sample previous frame, and an inbetween time
					// to determine if there's a binary movement

					TimeValue frametime_prev = frametime - GetTicksPerFrame();
					TimeValue frametime_mid	 = (frametime + frametime_prev) / 2;

					// if data at frametime_prev == data at frametime_mid and != data at frametime
					// then we have a binary movement! 
						
               Control *c;
               
               c = node->GetTMController()->GetPositionController();
				
					if (c) {

            		Interval iValid;
               
						Matrix3 smat1;	// sample matrix 1
						Matrix3 smat2;	// sample matrix 2
						Matrix3 smat3; // sample matrix 3
               
						iValid = FOREVER;
						smat1  = node->GetParentTM(frametime_prev);
						c->GetValue(frametime_prev, &smat1, iValid, CTRL_RELATIVE);
               
						iValid = FOREVER;
						smat2  = node->GetParentTM(frametime_mid);
						c->GetValue(frametime_mid, &smat2, iValid, CTRL_RELATIVE);
               
						iValid = FOREVER;
						smat3  = node->GetParentTM(frametime);
						c->GetValue(frametime, &smat3, iValid, CTRL_RELATIVE);
               
						if ((smat1 == smat2) && (!(smat2 == smat3))) {
               		binary_move = true;
							DebugPrint(_T("Binary Move on Translation\n"));
						}	
               
						if (false == binary_move)  {
							c = node->GetTMController()->GetRotationController();   
							 
							if (c) {

								iValid = FOREVER;
								smat1  = node->GetParentTM(frametime_prev);
								c->GetValue(frametime_prev, &smat1, iValid, CTRL_RELATIVE);
								
								iValid = FOREVER;
								smat2  = node->GetParentTM(frametime_mid);
								c->GetValue(frametime_mid, &smat2, iValid, CTRL_RELATIVE);
								
								iValid = FOREVER;
								smat3  = node->GetParentTM(frametime);
								c->GetValue(frametime, &smat3, iValid, CTRL_RELATIVE);
                  
								if ((smat1 == smat2) && (!(smat2 == smat3)))  {
                  			binary_move = true;
									DebugPrint(_T("Binary Move on Rotation\n"));
								}
							}
						}
					}
	         }
         }  
         				  
                       
			set_binary_movement(bindex, frame, binary_move);
         
		} // if(bindex!=-1)
	}

	/*
	** release allocated memory
	*/
	delete tree;
}


/*********************************************************************************************** 
 * MotionClass::set_motion_matrix -- store a motion matrix                                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void MotionClass::set_motion_matrix(int node,int frame,const Matrix3 & motion)
{
	assert(node >= 0);
	assert(frame >= 0);
	assert(node < BasePose->Num_Nodes());
	assert(frame < NumFrames);

	MotionMatrix[node][frame] = motion;
	NodeValidFlags[node] = 1;
}


/*********************************************************************************************** 
 * MotionClass::get_motion_matrix -- retrieve a motion matrix                                  * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
Matrix3 MotionClass::get_motion_matrix(int node,int frame)
{
	assert(node >= 0);
	assert(frame >= 0);
	assert(node < BasePose->Num_Nodes());
	assert(frame < NumFrames);

	return MotionMatrix[node][frame];
}


/*********************************************************************************************** 
 * MotionClass::set_eulers -- store euler angles                                               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void MotionClass::set_eulers(int node,int frame, float x, float y, float z)
{
	/*
	** if we're past the first frame, massage the euler angles to the
	** representation closest to the previous frame.
	*/
	if (frame > 0) {

		/*
		** First, compute equivalent euler angles
		*/
		double x2 = PI + x;
		double y2 = PI - y;
		double z2 = PI + z;
				
 		if (x2 > PI) {
 			x2 = x2 - 2*PI;
 		}

 		if (y2 > PI) {
 			y2 = y2 - 2*PI;
 		}

 		if (z2 > PI) {
 			z2 = z2 - 2*PI;
 		}

		/*
		** load up the previous frame eulers
		*/
		double px,py,pz;
		px = get_eulers(node,frame - 1)[0];
		py = get_eulers(node,frame - 1)[1];
		pz = get_eulers(node,frame - 1)[2];

		// now, pick between the two
		double mag0 = (x - px) * (x - px)    + (y - py) * (y - py)   + (z - pz) * (z - pz);
		double mag1 = (x2 - px) * (x2 - px)  + (y2 - py) * (y2 - py) + (z2 - pz) * (z2 - pz);

		if (mag1 < mag0) {
			x = x2;
			y = y2;
			z = z2;
		}
	}
	
	EulerDelta[node][frame].x = x;
	EulerDelta[node][frame].y = y;
	EulerDelta[node][frame].z = z;
	NodeValidFlags[node] = 1;
}


/*********************************************************************************************** 
 * MotionClass::get_eulers -- retrieve euler angles                                            * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
Point3 MotionClass::get_eulers(int node,int frame)
{
	return Point3(
		EulerDelta[node][frame].x,
		EulerDelta[node][frame].y,
		EulerDelta[node][frame].z
	);
}


/***********************************************************************************************
 * MotionClass::set_visibility -- store a visibility bit                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MotionClass::set_visibility(int node,int frame,bool visible)
{
	VisData[node][frame] = visible;
	NodeValidFlags[node] = 1;
}


/***********************************************************************************************
 * MotionClass::get_visibility -- retrieve the visibility bit for this node:frame              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool MotionClass::get_visibility(int node,int frame)
{
	return VisData[node][frame];
}


/***********************************************************************************************
 * MotionClass::set_binary_movement -- store a binary movement bit                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void MotionClass::set_binary_movement(int node,int frame,bool visible)
{
	BinMoveData[node][frame] = visible;
	//NodeValidFlags[node] = 1;
}


/***********************************************************************************************
 * MotionClass::get_visibility -- retrieve the movement bit for this node:frame                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool MotionClass::get_binary_movement(int node,int frame)
{
	return BinMoveData[node][frame];
}



/*********************************************************************************************** 
 * MotionClass::Save -- save the motion to a W3D file                                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool MotionClass::Save(ChunkSaveClass & csave)
{
	uint32 chunk_anim_type = W3D_CHUNK_ANIMATION;

	ExportLog::printf("\nBegin Save Motion Data\n");

	if (CompressAnimation) {
		chunk_anim_type = W3D_CHUNK_COMPRESSED_ANIMATION;
	}

	if (!csave.Begin_Chunk( chunk_anim_type )) {
		return false;
	}

	if (!save_header(csave)) {
		return false;
	}

	if (!save_channels(csave)) {
		return false;
	}

	if (!csave.End_Chunk()) {
		return false;
	}
	
	return true;
}


/*********************************************************************************************** 
 * MotionClass::save_header -- save the header                                                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool MotionClass::save_header(ChunkSaveClass & csave)
{

	ExportLog::printf("Save Header Type: ");

	if (CompressAnimation) {
		// New Compressed Style
		if (!csave.Begin_Chunk(W3D_CHUNK_COMPRESSED_ANIMATION_HEADER)) {
			return false;
		}

		W3dCompressedAnimHeaderStruct aheader;
		aheader.Version = W3D_CURRENT_COMPRESSED_HANIM_VERSION;
		Set_W3D_Name(aheader.Name,Name);
		Set_W3D_Name(aheader.HierarchyName,BasePose->Get_Name());
		aheader.NumFrames = NumFrames;
		aheader.FrameRate = FrameRate;
		aheader.Flavor    = CompressAnimationFlavor;	// for future expansion
		
		switch (CompressAnimationFlavor) {
			
			case ANIM_FLAVOR_TIMECODED:
				ExportLog::printf("TimeCoded\n");
				break;
			case ANIM_FLAVOR_ADAPTIVE_DELTA:
				ExportLog::printf("Adaptive Delta\n");
				break;
			default:
				ExportLog::printf("UNKNOWN\n");
				break;
		}

		if (csave.Write(&aheader,sizeof(aheader)) != sizeof(aheader)) {
			return false;
		}

		if (!csave.End_Chunk()) {
			return false;
		}
	}
	else {
		
		ExportLog::printf("Non-Compressed.\n");

		// Classic Non-Compressed Style
		if (!csave.Begin_Chunk(W3D_CHUNK_ANIMATION_HEADER)) {
			return false;
		}

		W3dAnimHeaderStruct aheader;
		aheader.Version = W3D_CURRENT_HANIM_VERSION;
		Set_W3D_Name(aheader.Name,Name);
		Set_W3D_Name(aheader.HierarchyName,BasePose->Get_Name());
		aheader.NumFrames = NumFrames;
		aheader.FrameRate = FrameRate;
		
		if (csave.Write(&aheader,sizeof(aheader)) != sizeof(aheader)) {
			return false;
		}

		if (!csave.End_Chunk()) {
			return false;
		}
	}

	
	return true;
}

/*********************************************************************************************** 
 * MotionClass::save_channels -- save the motion channels                                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
bool MotionClass::save_channels(ChunkSaveClass & csave)
{

	int NumNodes = BasePose->Num_Nodes();

	ExportLog::printf("\nSaving Channel Data for %d Nodes\n", NumNodes);

	for (int nodeidx = 0; nodeidx < BasePose->Num_Nodes(); nodeidx++) {

		ExportLog::printf("\nnode: %d ", nodeidx);

		/*
		** Just ignore this node if it didn't appear in the max scene.
		*/
		if (NodeValidFlags[nodeidx]) {
			
			float identity[] = { 0.0f,0.0f,0.0f,1.0f };

			VectorChannelClass	xchan (nodeidx, NumFrames, ANIM_CHANNEL_X,  1, identity);
			VectorChannelClass	ychan (nodeidx, NumFrames, ANIM_CHANNEL_Y,  1, identity);
			VectorChannelClass	zchan (nodeidx, NumFrames, ANIM_CHANNEL_Z,  1, identity);
			VectorChannelClass	xrchan(nodeidx, NumFrames, ANIM_CHANNEL_XR, 1, identity);
			VectorChannelClass	yrchan(nodeidx, NumFrames, ANIM_CHANNEL_YR, 1, identity);
			VectorChannelClass	zrchan(nodeidx, NumFrames, ANIM_CHANNEL_ZR, 1, identity);
			VectorChannelClass	qchan (nodeidx, NumFrames, ANIM_CHANNEL_Q,  4, identity);

			xchan.SetSaveOptions(CompressAnimation, CompressAnimationFlavor, CompressAnimationTranslationError, CompressAnimationRotationError, ReduceAnimation, ReduceAnimationPercent);
			ychan.SetSaveOptions(CompressAnimation, CompressAnimationFlavor, CompressAnimationTranslationError, CompressAnimationRotationError, ReduceAnimation, ReduceAnimationPercent);
			zchan.SetSaveOptions(CompressAnimation, CompressAnimationFlavor, CompressAnimationTranslationError, CompressAnimationRotationError, ReduceAnimation, ReduceAnimationPercent);
			xrchan.SetSaveOptions(CompressAnimation, CompressAnimationFlavor, CompressAnimationTranslationError, CompressAnimationRotationError, ReduceAnimation, ReduceAnimationPercent);
			yrchan.SetSaveOptions(CompressAnimation, CompressAnimationFlavor, CompressAnimationTranslationError, CompressAnimationRotationError, ReduceAnimation, ReduceAnimationPercent);
			zrchan.SetSaveOptions(CompressAnimation, CompressAnimationFlavor, CompressAnimationTranslationError, CompressAnimationRotationError, ReduceAnimation, ReduceAnimationPercent);
			qchan.SetSaveOptions(CompressAnimation, CompressAnimationFlavor, CompressAnimationTranslationError, CompressAnimationRotationError, ReduceAnimation, ReduceAnimationPercent);

			BitChannelClass  vischan(nodeidx, NumFrames, BIT_CHANNEL_VIS, 1);
			vischan.Set_Bits(VisData[nodeidx]);

			BitChannelClass binmovechan(nodeidx, NumFrames, 0, 0);
			binmovechan.Set_Bits(BinMoveData[nodeidx]);

			for (int frameidx = 0; frameidx < NumFrames; frameidx++) {

				float		vec[4];
				Matrix3 tm = get_motion_matrix(nodeidx,frameidx);
				Point3 eulers = get_eulers(nodeidx,frameidx);


				Point3 old_tran = tm.GetTrans();
				Quat old_rot(tm);
				
				Point3	tran;
				Point3	scale;
				Quat		rot;

				DecomposeMatrix(tm,tran,rot,scale);

				/*
				** fixup the quaternion - max's quaternions are different than mine(?)
				*/
				rot[0] = -rot[0];
				rot[1] = -rot[1];
				rot[2] = -rot[2];
				rot[3] = rot[3];

				/*
				** Build the x translation channel
				*/
				vec[0] = tran.x;
				xchan.Set_Vector(frameidx,vec);

				/*
				** Build the y translation channel
				*/
				vec[0] = tran.y;
				ychan.Set_Vector(frameidx,vec);

				/*
				** Build the z translation channel
				*/
				vec[0] = tran.z;
				zchan.Set_Vector(frameidx,vec);

				/* 
				** Build the x rotation channel
				*/
				vec[0] = eulers.x;
				xrchan.Set_Vector(frameidx,vec);

				/* 
				** Build the y rotation channel
				*/
				vec[0] = eulers.y;
				yrchan.Set_Vector(frameidx,vec);

				/* 
				** Build the z rotation channel
				*/
				vec[0] = eulers.z;
				zrchan.Set_Vector(frameidx,vec);

				/*
				** Build the quaternion rotation channel
				*/
				vec[0] = rot[0];
				vec[1] = rot[1];
				vec[2] = rot[2];
				vec[3] = rot[3];

				qchan.Set_Vector(frameidx,vec);

				/*
				** build the visibility channel
				*/
				vischan.Set_Bit(frameidx,get_visibility(nodeidx,frameidx));
				//
				// build binarymovement channel
				//
				binmovechan.Set_Bit(frameidx, get_binary_movement(nodeidx, frameidx));
			}
			
			// If objects arn't visible, then the channel data may as well be empty

			if (!vischan.Is_Empty()) {

				if (!xchan.Is_Empty()) xchan.ClearInvisibleData(&vischan);
				if (!ychan.Is_Empty()) ychan.ClearInvisibleData(&vischan);
				if (!zchan.Is_Empty()) zchan.ClearInvisibleData(&vischan);
				if (!qchan.Is_Empty()) qchan.ClearInvisibleData(&vischan);

			}

			if (!xchan.Is_Empty()) {
				ExportLog::printf("x");
				xchan.Save(csave, &binmovechan );
			}
			if (!ychan.Is_Empty()) {
				ExportLog::printf("y");
				ychan.Save(csave, &binmovechan );
			}
			if (!zchan.Is_Empty()) {
				ExportLog::printf("z");
				zchan.Save(csave, &binmovechan );
			}


			// (gth) not saving Euler angles any more since we don't use them
//			if (!xrchan.Is_Empty()) xrchan.Save(csave);
//			if (!yrchan.Is_Empty()) yrchan.Save(csave);
//			if (!zrchan.Is_Empty()) zrchan.Save(csave);

			if (!qchan.Is_Empty()) {
				ExportLog::printf("q");
				qchan.Save(csave, &binmovechan);
			}

			if (!vischan.Is_Empty()) {
				ExportLog::printf("v");
				vischan.Save(csave, CompressAnimation);
			}
		}

		ExportLog::updatebar(nodeidx ,NumNodes);

	}

	ExportLog::updatebar(1,1);

	ExportLog::printf("\n\nSave Channel Data Complete.\n");

	return true;
}

// EOF - motion.cpp


