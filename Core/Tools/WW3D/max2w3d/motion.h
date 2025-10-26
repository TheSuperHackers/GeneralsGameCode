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

/* $Header: /Commando/Code/Tools/max2w3d/motion.h 13    10/30/00 6:56p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/motion.h                       $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/30/00 5:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef ALWAYS_H
#include "always.h"
#endif

#include <max.h>

#ifndef HIERSAVE_H
#include "hiersave.h"
#endif

#ifndef PROGRESS_H
#include "PROGRESS.H"
#endif

#ifndef CHUNKIO_H
#include "chunkio.h"
#endif

#ifndef VECTOR_H
#include "Vector.H"
#endif

#ifndef LOGDLG_H
#include "logdlg.h"
#endif

struct W3dExportOptionsStruct;


class MotionClass
{
public:

	MotionClass	(
						IScene * scene,
						INode * rootnode,
						HierarchySaveClass * basepose,
					   W3dExportOptionsStruct & options,
						int	framerate,
						Progress_Meter_Class * meter,
                  HWND	MaxHwnd,
						char * name,
						Matrix3 & offset = Matrix3(1)		// matrix to bring current object space into
																	// base object space (for damage animations)
					);

	MotionClass	(
						IScene * scene,
						INodeListClass * rootlist,
						HierarchySaveClass * basepose,
					   W3dExportOptionsStruct & options,
						int	framerate,
						Progress_Meter_Class * meter,
                  HWND MaxHwnd,
						char * name,
						Matrix3 & offset = Matrix3(1)		// matrix to bring current object space into
																	// base object space (for damage animations)
					);

	~MotionClass(void);

	bool Save(ChunkSaveClass & csave);

private:

	IScene *						Scene;
	INode *						RootNode;
	INodeListClass *			RootList;
	HierarchySaveClass *		BasePose;
	int							StartFrame;
	int							EndFrame;
	int							NumFrames;
	int							FrameRate;

	bool							ReduceAnimation;
	int							ReduceAnimationPercent;

	bool							CompressAnimation;
	int							CompressAnimationFlavor;
	float							CompressAnimationTranslationError;
	float							CompressAnimationRotationError;

	Progress_Meter_Class *	Meter;
	char							Name[W3D_NAME_LEN];
	Matrix3						Offset;

	// 2D array of matrices, one per node per frame
	Matrix3 * *					MotionMatrix;

	// 2D array of euler angles, one per node per frame
	Point3 * *					EulerDelta;

	// Visibility bits, one bit per node per frame
	BooleanVectorClass *		VisData;

	// Movement bits, one bit per node per frame, to designate a movement as interpolated, or not
	BooleanVectorClass *		BinMoveData;

	// flag for each node in the base pose, indicating
	// whether the node actually appeard in the max scene
	// being exported.
	BooleanVectorClass		NodeValidFlags;

	void			compute_frame_motion(int frame);
	void			set_motion_matrix(int node,int frame,const Matrix3 & motion);
	Matrix3		get_motion_matrix(int node,int frame);
	void			set_eulers(int node,int frame,float x,float y,float z);
	Point3		get_eulers(int node,int frame);
	void			set_visibility(int node,int frame,bool visible);
	bool			get_visibility(int node,int frame);
	void			set_binary_movement(int node,int frame,bool visible);
	bool			get_binary_movement(int node,int frame);

	bool			save_header(ChunkSaveClass & csave);
	bool			save_channels(ChunkSaveClass & csave);

	void			init(void);

};
